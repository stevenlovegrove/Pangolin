/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
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

#pragma once

#include <pangolin/utils/uri.h>
#include <pangolin/video/video_exception.h>
#include <pangolin/video/video_interface.h>
#include <pangolin/video/video_output_interface.h>

namespace pangolin
{

//! Open Video Interface from string specification (as described in this files header)
PANGOLIN_EXPORT
std::unique_ptr<VideoInterface> OpenVideo(const std::string& uri);

//! Open Video Interface from Uri specification
PANGOLIN_EXPORT
std::unique_ptr<VideoInterface> OpenVideo(const Uri& uri);

//! Open VideoOutput Interface from string specification (as described in this files header)
PANGOLIN_EXPORT
std::unique_ptr<VideoOutputInterface> OpenVideoOutput(const std::string& str_uri);

//! Open VideoOutput Interface from Uri specification
PANGOLIN_EXPORT
std::unique_ptr<VideoOutputInterface> OpenVideoOutput(const Uri& uri);

//! Create vector of matching interfaces either through direct cast or filter interface.
template<typename T>
std::vector<T*> FindMatchingVideoInterfaces( VideoInterface& video )
{
    std::vector<T*> matches;

    T* vid = dynamic_cast<T*>(&video);
    if(vid) {
        matches.push_back(vid);
    }

    VideoFilterInterface* vidf = dynamic_cast<VideoFilterInterface*>(&video);
    if(vidf) {
        std::vector<T*> fmatches = vidf->FindMatchingStreams<T>();
        matches.insert(matches.begin(), fmatches.begin(), fmatches.end());
    }

    return matches;
}

template<typename T>
T* FindFirstMatchingVideoInterface( VideoInterface& video )
{
    T* vid = dynamic_cast<T*>(&video);
    if(vid) {
        return vid;
    }

    VideoFilterInterface* vidf = dynamic_cast<VideoFilterInterface*>(&video);
    if(vidf) {
        std::vector<T*> fmatches = vidf->FindMatchingStreams<T>();
        if(fmatches.size()) {
            return fmatches[0];
        }
    }

    return 0;
}

inline
picojson::value GetVideoFrameProperties(VideoInterface* video)
{
    VideoPropertiesInterface* pi = dynamic_cast<VideoPropertiesInterface*>(video);
    VideoFilterInterface* fi = dynamic_cast<VideoFilterInterface*>(video);

    if(pi) {
        return pi->FrameProperties();
    }else if(fi){
        if(fi->InputStreams().size() == 1) {
            return GetVideoFrameProperties(fi->InputStreams()[0]);
        }else if(fi->InputStreams().size() > 0){
            picojson::value streams;

            for(size_t i=0; i< fi->InputStreams().size(); ++i) {
                const picojson::value dev_props = GetVideoFrameProperties(fi->InputStreams()[i]);
                if(dev_props.contains("streams")) {
                    const picojson::value& dev_streams = dev_props["streams"];
                    for(size_t j=0; j < dev_streams.size(); ++j) {
                        streams.push_back(dev_streams[j]);
                    }
                }else{
                    streams.push_back(dev_props);
                }
            }

            if(streams.size() > 1) {
                picojson::value json = streams[0];
                json["streams"] = streams;
                return json;
            }else{
                return streams[0];
            }
        }
    }
    return picojson::value();
}

inline
picojson::value GetVideoDeviceProperties(VideoInterface* video)
{
    VideoPropertiesInterface* pi = dynamic_cast<VideoPropertiesInterface*>(video);
    VideoFilterInterface* fi = dynamic_cast<VideoFilterInterface*>(video);

    if(pi) {
        return pi->DeviceProperties();
    }else if(fi){
        if(fi->InputStreams().size() == 1) {
            return GetVideoDeviceProperties(fi->InputStreams()[0]);
        }else if(fi->InputStreams().size() > 0){
            picojson::value streams;

            for(size_t i=0; i< fi->InputStreams().size(); ++i) {
                const picojson::value dev_props = GetVideoDeviceProperties(fi->InputStreams()[i]);
                if(dev_props.contains("streams")) {
                    const picojson::value& dev_streams = dev_props["streams"];
                    for(size_t j=0; j < dev_streams.size(); ++j) {
                        streams.push_back(dev_streams[j]);
                    }
                }else{
                    streams.push_back(dev_props);
                }
            }

            if(streams.size() > 1) {
                picojson::value json = streams[0];
                json["streams"] = streams;
                return json;
            }else{
                return streams[0];
            }
        }
    }
    return picojson::value();
}

}
