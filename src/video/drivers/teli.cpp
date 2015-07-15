/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2015 Steven Lovegrove
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

#include <pangolin/video/drivers/teli.h>

namespace pangolin
{

TeliVideo::TeliVideo()
    : cam(0), strm(0), hStrmCmpEvt(0)
{
	Teli::CAM_API_STATUS uiStatus = Teli::Sys_Initialize();
	if (uiStatus != Teli::CAM_API_STS_SUCCESS)
		throw pangolin::VideoException("Unable to initialise TeliSDK.");

	uint32_t num_cams = 0;
	uiStatus = Teli::Sys_GetNumOfCameras(&num_cams);
	if (uiStatus != Teli::CAM_API_STS_SUCCESS)
		throw pangolin::VideoException("Unable to enumerate TeliSDK cameras.");

	if (num_cams == 0)
		throw pangolin::VideoException("No TeliSDK Cameras available.");

	uiStatus = Teli::Cam_Open(0, &cam, 0, false, 0);
	if (uiStatus != Teli::CAM_API_STS_SUCCESS)
		throw pangolin::VideoException("TeliSDK: Error opening camera");

	uint32_t width = 0;
	uint32_t height = 0;
	uiStatus = Teli::GetCamSensorWidth(cam, &width);
	if (uiStatus != Teli::CAM_API_STS_SUCCESS)
		throw pangolin::VideoException("Unable to get TeliSDK Camera dimensions");

	uiStatus = Teli::GetCamSensorHeight(cam, &height);
	if (uiStatus != Teli::CAM_API_STS_SUCCESS)
		throw pangolin::VideoException("Unable to get TeliSDK Camera dimensions");

	// Create completion event object for stream.
	hStrmCmpEvt = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hStrmCmpEvt == NULL)
		throw pangolin::VideoException("TeliSDK: Error creating event.");

	uint32_t uiPyldSize = 0;
	uiStatus = Teli::Strm_OpenSimple(cam, &strm, &uiPyldSize, hStrmCmpEvt);
	if (uiStatus != Teli::CAM_API_STS_SUCCESS)
		throw pangolin::VideoException("TeliSDK: Error opening camera stream.");

	Start();
	
    const VideoPixelFormat pfmt = VideoFormatFromString("GRAY8");
    
    size_bytes = 0;
    
	const int n = 1;
    for(size_t c=0; c < n; ++c) {
		const StreamInfo stream_info(pfmt, width, height, (width*pfmt.bpp) / 8, 0);
        streams.push_back(stream_info);        
		size_bytes += uiPyldSize;
    }

}

TeliVideo::~TeliVideo()
{
	Teli::CAM_API_STATUS uiStatus = Teli::Strm_Close(strm);
	if (uiStatus != Teli::CAM_API_STS_SUCCESS)
		throw pangolin::VideoException("TeliSDK: Error closing camera stream.");

	uiStatus = Teli::Cam_Close(cam);
	if (uiStatus != Teli::CAM_API_STS_SUCCESS)
		throw pangolin::VideoException("TeliSDK: Error closing camera.");

	uiStatus = Teli::Sys_Terminate();
	if (uiStatus != Teli::CAM_API_STS_SUCCESS)
		throw pangolin::VideoException("TeliSDK: Error uninitialising.");
}

//! Implement VideoInput::Start()
void TeliVideo::Start()
{
	Teli::CAM_API_STATUS uiStatus = Teli::Strm_Start(strm);
	if (uiStatus != Teli::CAM_API_STS_SUCCESS)
		throw pangolin::VideoException("TeliSDK: Error starting stream.");
}

//! Implement VideoInput::Stop()
void TeliVideo::Stop()
{
	Teli::CAM_API_STATUS uiStatus = Teli::Strm_Stop(strm);
	if (uiStatus != Teli::CAM_API_STS_SUCCESS)
		throw pangolin::VideoException("TeliSDK: Error stopping stream.");
}

//! Implement VideoInput::SizeBytes()
size_t TeliVideo::SizeBytes() const
{
    return size_bytes;
}

//! Implement VideoInput::Streams()
const std::vector<StreamInfo>& TeliVideo::Streams() const
{
    return streams;
}

//! Implement VideoInput::GrabNext()
bool TeliVideo::GrabNext(unsigned char* image, bool wait)
{
	unsigned int uiRet = WaitForSingleObject(hStrmCmpEvt, 2000);
	if (uiRet == WAIT_OBJECT_0) {
		Teli::CAM_IMAGE_INFO sImageInfo;
		uint32_t uiPyldSize = size_bytes;
		Teli::CAM_API_STATUS uiStatus = Teli::Strm_ReadCurrentImage(strm, image, &uiPyldSize, &sImageInfo);
		return (uiStatus == Teli::CAM_API_STS_SUCCESS);
	}

    return false;
}

//! Implement VideoInput::GrabNewest()
bool TeliVideo::GrabNewest(unsigned char* image, bool wait)
{
    return GrabNext(image,wait);
}

}
