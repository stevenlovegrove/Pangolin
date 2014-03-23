/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2014 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <pangolin/video/depthsense.h>

namespace pangolin
{

const size_t ROGUE_ADDR = 0x01;

DepthSenseContext& DepthSenseContext::I()
{
    static DepthSenseContext s;
    return s;
}

DepthSense::Context& DepthSenseContext::Context()
{
    return g_context;
}

void DepthSenseContext::NewDeviceRunning()
{
    running_devices++;
    if(running_devices == 1) {
        StartNodes();
    }
}

void DepthSenseContext::DeviceClosing()
{
    running_devices--;
    if(running_devices == 0) {
        StopNodes();

        // Force destruction of current context
        g_context = DepthSense::Context();
    }
}

DepthSenseVideo* DepthSenseContext::GetDepthSenseVideo(int device_num)
{
    if(running_devices == 0) {
        // Initialise SDK
        g_context = DepthSense::Context::create("localhost");
    }

    // Get the list of currently connected devices
    std::vector<DepthSense::Device> da = g_context.getDevices();

    if( da.size() > device_num )
    {
        return new DepthSenseVideo(da[device_num]);
    }

    throw VideoException("DepthSense device not connected.");
}

DepthSenseContext::DepthSenseContext()
    : is_running(false), running_devices(0)
{
}

DepthSenseContext::~DepthSenseContext()
{
}


void DepthSenseContext::StartNodes()
{
    if(!is_running) {
        // Launch EventLoop thread
        event_thread = boostd::thread(&DepthSenseContext::EventLoop, this );
    }
}

void DepthSenseContext::StopNodes()
{
    if(is_running) {
        g_context.quit();
        event_thread.join();
    }
}

void DepthSenseContext::EventLoop()
{
    is_running = true;
    g_context.startNodes();
    g_context.run();
    g_context.stopNodes();
    is_running = false;
}

DepthSenseVideo::DepthSenseVideo(DepthSense::Device device)
    : device(device), fill_image(0)
{
    device.nodeAddedEvent().connect(this, &DepthSenseVideo::onNodeConnected);
    device.nodeRemovedEvent().connect(this, &DepthSenseVideo::onNodeDisconnected);

    std::vector<DepthSense::Node> nodes = device.getNodes();
    for (int n = 0; n < (int)nodes.size();n++)
        ConfigureNode(nodes[n]);

    // Just return depth image for the time being
    const int n = 1;
    const int w = 320;
    const int h = 240;

    const VideoPixelFormat pfmt = VideoFormatFromString("GRAY16LE");
    
    size_bytes = 0;
    
    for(size_t c=0; c < n; ++c) {
        const StreamInfo stream_info(pfmt, w, h, (w*pfmt.bpp)/8, 0);
        streams.push_back(stream_info);        
        size_bytes += w*h*(pfmt.bpp)/8;
    }

    DepthSenseContext::I().NewDeviceRunning();
}

DepthSenseVideo::~DepthSenseVideo()
{
    if (g_cnode.isSet()) DepthSenseContext::I().Context().unregisterNode(g_cnode);
    if (g_dnode.isSet()) DepthSenseContext::I().Context().unregisterNode(g_dnode);
    
    fill_image = (unsigned char*)ROGUE_ADDR;
    cond_image_requested.notify_all();

    DepthSenseContext::I().DeviceClosing();
}

void DepthSenseVideo::onNodeConnected(DepthSense::Device device, DepthSense::Device::NodeAddedData data)
{
    ConfigureNode(data.node);
}

void DepthSenseVideo::onNodeDisconnected(DepthSense::Device device, DepthSense::Device::NodeRemovedData data)
{
    if (data.node.is<DepthSense::ColorNode>() && (data.node.as<DepthSense::ColorNode>() == g_cnode))
        g_cnode.unset();
    if (data.node.is<DepthSense::DepthNode>() && (data.node.as<DepthSense::DepthNode>() == g_dnode))
        g_dnode.unset();
}

void DepthSenseVideo::ConfigureNode(DepthSense::Node node)
{
    if ((node.is<DepthSense::DepthNode>())&&(!g_dnode.isSet()))
    {
        g_dnode = node.as<DepthSense::DepthNode>();
        ConfigureDepthNode();
        DepthSenseContext::I().Context().registerNode(node);
    }

//    if ((node.is<DepthSense::ColorNode>())&&(!g_cnode.isSet()))
//    {
//        g_cnode = node.as<DepthSense::ColorNode>();
//        ConfigureColorNode();
//        DepthSenseContext::I().Context().registerNode(node);
//    }
}

void DepthSenseVideo::ConfigureDepthNode()
{
    g_dnode.newSampleReceivedEvent().connect(this, &DepthSenseVideo::onNewDepthSample);

    DepthSense::DepthNode::Configuration config = g_dnode.getConfiguration();
    config.frameFormat = DepthSense::FRAME_FORMAT_QVGA;
    config.framerate = 25;
    config.mode = DepthSense::DepthNode::CAMERA_MODE_CLOSE_MODE;
    config.saturation = true;

    //g_dnode.setEnableVertices(true);
    //g_dnode.setEnableDepthMapFloatingPoint(true);
    g_dnode.setEnableDepthMap(true);

    try 
    {
        DepthSenseContext::I().Context().requestControl(g_dnode,0);

        g_dnode.setConfiguration(config);
    }
    catch (DepthSense::ArgumentException& e)
    {
        printf("Argument Exception: %s\n",e.what());
    }
    catch (DepthSense::UnauthorizedAccessException& e)
    {
        printf("Unauthorized Access Exception: %s\n",e.what());
    }
    catch (DepthSense::IOException& e)
    {
        printf("IO Exception: %s\n",e.what());
    }
    catch (DepthSense::InvalidOperationException& e)
    {
        printf("Invalid Operation Exception: %s\n",e.what());
    }
    catch (DepthSense::ConfigurationException& e)
    {
        printf("Configuration Exception: %s\n",e.what());
    }
    catch (DepthSense::StreamingException& e)
    {
        printf("Streaming Exception: %s\n",e.what());
    }
    catch (DepthSense::TimeoutException&)
    {
        printf("TimeoutException\n");
    }

}

void DepthSenseVideo::ConfigureColorNode()
{
    // connect new color sample handler
    g_cnode.newSampleReceivedEvent().connect(this, &DepthSenseVideo::onNewColorSample);

    DepthSense::ColorNode::Configuration config = g_cnode.getConfiguration();
    config.frameFormat = DepthSense::FRAME_FORMAT_VGA;
    config.compression = DepthSense::COMPRESSION_TYPE_MJPEG;
    config.powerLineFrequency = DepthSense::POWER_LINE_FREQUENCY_50HZ;
    config.framerate = 25;

    g_cnode.setEnableColorMap(true);

    try 
    {
        DepthSenseContext::I().Context().requestControl(g_cnode,0);
        g_cnode.setConfiguration(config);
    }
    catch (DepthSense::ArgumentException& e)
    {
        printf("Argument Exception: %s\n",e.what());
    }
    catch (DepthSense::UnauthorizedAccessException& e)
    {
        printf("Unauthorized Access Exception: %s\n",e.what());
    }
    catch (DepthSense::IOException& e)
    {
        printf("IO Exception: %s\n",e.what());
    }
    catch (DepthSense::InvalidOperationException& e)
    {
        printf("Invalid Operation Exception: %s\n",e.what());
    }
    catch (DepthSense::ConfigurationException& e)
    {
        printf("Configuration Exception: %s\n",e.what());
    }
    catch (DepthSense::StreamingException& e)
    {
        printf("Streaming Exception: %s\n",e.what());
    }
    catch (DepthSense::TimeoutException&)
    {
        printf("TimeoutException\n");
    }
}

void DepthSenseVideo::onNewColorSample(DepthSense::ColorNode node, DepthSense::ColorNode::NewSampleReceivedData data)
{

}

void DepthSenseVideo::onNewDepthSample(DepthSense::DepthNode node, DepthSense::DepthNode::NewSampleReceivedData data)
{
    // Our data is stored in a float array:
    // data.depthMapFloatingPoint

    {
        boostd::unique_lock<boostd::mutex> lock(update_mutex);

        // Wait for fill request
        while(!fill_image) {
            cond_image_requested.wait(lock);
        }

        if(fill_image != (unsigned char*)ROGUE_ADDR) {
            // Fill with data
            memcpy(fill_image, data.depthMap, SizeBytes());
            fill_image = 0;
        }
    }

    cond_image_filled.notify_one();
}

void DepthSenseVideo::Start()
{
}

void DepthSenseVideo::Stop()
{
}

size_t DepthSenseVideo::SizeBytes() const
{
    return size_bytes;
}

const std::vector<StreamInfo>& DepthSenseVideo::Streams() const
{
    return streams;
}

bool DepthSenseVideo::GrabNext( unsigned char* image, bool wait )
{
    if(fill_image) {
        throw std::runtime_error("GrabNext Cannot be called concurrently");
    }

    // Request that image is filled with data
    fill_image = image;
    cond_image_requested.notify_one();

    // Wait until it has been filled successfully. 
    {
        boostd::unique_lock<boostd::mutex> lock(update_mutex);
        while( fill_image ) {
            cond_image_filled.wait(lock);
        }
    }

    return true;
}

bool DepthSenseVideo::GrabNewest( unsigned char* image, bool wait )
{
    return GrabNext(image,wait);
}

}
