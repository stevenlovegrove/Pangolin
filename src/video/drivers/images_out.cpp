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

#include <pangolin/factory/factory_registry.h>
#include <pangolin/video/drivers/images_out.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/image/image_io.h>

namespace pangolin {

ImagesVideoOutput::ImagesVideoOutput(const std::string& image_folder, const std::string& json_file_out)
    : json_frames(picojson::array_type,true),
      image_index(0), image_folder( PathExpand(image_folder) )
{
    if(!json_file_out.empty()) {
        file.open(json_file_out);
        if(!file.is_open()) {
            throw std::runtime_error("Unable to open json file for writing, " + json_file_out);
        }
    }
}

ImagesVideoOutput::~ImagesVideoOutput()
{
    if(file.is_open())
    {
        std::string video_uri = "images://" + image_folder + "/image_*_[0";
        for(size_t s=1; s < streams.size(); ++s) {
            video_uri += pangolin::FormatString(",%", s);
        }
        video_uri += "].png";

        picojson::value json_file;
        json_file["device_properties"] = device_properties;
        json_file["frames"] = json_frames;
        json_file["input_uri"] = input_uri;
        json_file["video_uri"] = video_uri;

        // Serialize json to file.
        file << json_file.serialize(true);
    }
}

const std::vector<StreamInfo>& ImagesVideoOutput::Streams() const
{
    return streams;
}

void ImagesVideoOutput::SetStreams(const std::vector<StreamInfo>& streams, const std::string& uri, const picojson::value& device_properties)
{
    this->streams = streams;
    this->input_uri = uri;
    this->device_properties = device_properties;
}

int ImagesVideoOutput::WriteStreams(const unsigned char* data, const picojson::value& frame_properties)
{
    picojson::value json_filenames(picojson::array_type, true);

    // Write each stream image to file.
    for(size_t s=0; s < streams.size(); ++s) {
        const pangolin::StreamInfo& si = streams[s];
        const std::string filename = pangolin::FormatString("image_%_%.png", image_index, s);
        json_filenames.push_back(filename);
        const Image<unsigned char> img = si.StreamImage(data);
        pangolin::SaveImage(img, si.PixFormat(), filename);
    }

    // Add frame_properties to json file.
    picojson::value json_frame;
    json_frame["frame_properties"] = frame_properties;
    json_frame["stream_files"] = json_filenames;
    json_frames.push_back(json_frame);

    ++image_index;
    return 0;
}

bool ImagesVideoOutput::IsPipe() const
{
    return false;
}

PANGOLIN_REGISTER_FACTORY(ImagesVideoOutput)
{
    struct ImagesVideoFactory : public FactoryInterface<VideoOutputInterface> {
        std::unique_ptr<VideoOutputInterface> Open(const Uri& uri) override {
            const std::string images_folder = PathExpand(uri.url);
            const std::string json_filename = images_folder + "/archive.json";

            return std::unique_ptr<VideoOutputInterface>(
                new ImagesVideoOutput(images_folder, json_filename)
            );
        }
    };

    auto factory = std::make_shared<ImagesVideoFactory>();
    FactoryRegistry<VideoOutputInterface>::I().RegisterFactory(factory, 10, "images");
}

}

