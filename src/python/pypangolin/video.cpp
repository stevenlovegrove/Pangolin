/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) Andrey Mnatsakanov
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

#include "video.hpp"
#include <pangolin/video/video_interface.h>
#include <pangolin/video/video_input.h>

#include <pybind11/numpy.h>
#include <pybind11/stl.h>

namespace py_pangolin {

    class PyVideoInterface: public pangolin::VideoInterface{
    public:
      using pangolin::VideoInterface::VideoInterface;

      size_t SizeBytes() const override {
        PYBIND11_OVERLOAD_PURE(
                               size_t,
                               pangolin::VideoInterface,
                               SizeBytes);
      }

      const std::vector<pangolin::StreamInfo>& Streams() const override {
        PYBIND11_OVERLOAD_PURE(
                               const std::vector<pangolin::StreamInfo>&,
                               pangolin::VideoInterface,
                               Streams);
      }

      void Start() override {
        PYBIND11_OVERLOAD_PURE(
                               void,
                               pangolin::VideoInterface,
                               Start);
      }

      void Stop() override {
        PYBIND11_OVERLOAD_PURE(
                               void,
                               pangolin::VideoInterface,
                               Stop);
      }

      bool GrabNext(unsigned char* image, bool wait = true) override {
        PYBIND11_OVERLOAD_PURE(
                               bool,
                               pangolin::VideoInterface,
                               GrabNext,
                               image,
                               wait);
      }

      bool GrabNewest(unsigned char* image, bool wait = true) override {
        PYBIND11_OVERLOAD_PURE(
                               bool,
                               pangolin::VideoInterface,
                               GrabNewest,
                               image,
                               wait);
      }
    };

    class PyGenicamVideoInterface: public pangolin::GenicamVideoInterface{
    public:
      using pangolin::GenicamVideoInterface::GenicamVideoInterface;

      std::string GetParameter(const std::string& name) override {
        PYBIND11_OVERLOAD_PURE(
                               std::string,
                               pangolin::GenicamVideoInterface,
                               GetParameter,
                               name);
      }

      void SetParameter(const std::string& name, const std::string& value) override {
        PYBIND11_OVERLOAD_PURE(
                               void,
                               pangolin::GenicamVideoInterface,
                               SetParameter,
                               name,
                               value);
      }
    };

    class PyBufferAwareVideoInterface: public pangolin::BufferAwareVideoInterface{
    public:
      using pangolin::BufferAwareVideoInterface::BufferAwareVideoInterface;

      uint32_t AvailableFrames() const override {
        PYBIND11_OVERLOAD_PURE(
                               uint32_t,
                               pangolin::BufferAwareVideoInterface,
                               AvailableFrames);
      }

      bool DropNFrames(uint32_t n) override {
        PYBIND11_OVERLOAD_PURE(
                               bool,
                               pangolin::BufferAwareVideoInterface,
                               DropNFrames,
                               n);
      }
    };

    class PyVideoPropertiesInterface: public pangolin::VideoPropertiesInterface{
    public:
      using pangolin::VideoPropertiesInterface::VideoPropertiesInterface;

      const picojson::value& DeviceProperties() const override {
        PYBIND11_OVERLOAD_PURE(
                               const picojson::value&,
                               pangolin::VideoPropertiesInterface,
                               DeviceProperties);
      }

      const picojson::value& FrameProperties() const override {
        PYBIND11_OVERLOAD_PURE(
                               const picojson::value&,
                               pangolin::VideoPropertiesInterface,
                               FrameProperties);
      }
    };  

    class PyVideoFilterInterface: public pangolin::VideoFilterInterface{
    public:
      using pangolin::VideoFilterInterface::VideoFilterInterface;

      // template <typename T>
      // std::vector<T*> FindMatchingStreams() override {
      //   PYBIND11_OVERLOAD(
      //                     std::vector<T*>,
      //                     pangolin::VideoFilterInterface,
      //                     FindMatchingStreams);
      // }

      std::vector<pangolin::VideoInterface*>& InputStreams() override {
        PYBIND11_OVERLOAD_PURE(
                               std::vector<pangolin::VideoInterface*>&,
                               pangolin::VideoFilterInterface,
                               InputStreams);
      }
    };  
    
    class PyVideoUvcInterface: public pangolin::VideoUvcInterface{
    public:
      using pangolin::VideoUvcInterface::VideoUvcInterface;

      int IoCtrl(uint8_t unit, uint8_t ctrl, unsigned char* data, int len, pangolin::UvcRequestCode req_code) override {
        PYBIND11_OVERLOAD_PURE(
                               int,
                               pangolin::VideoUvcInterface,
                               IoCtrl,
                               unit,
                               ctrl,
                               data,
                               len,
                               req_code);
      }

      bool GetExposure(int& exp_us) override {
        PYBIND11_OVERLOAD_PURE(
                               bool,
                               pangolin::VideoUvcInterface,
                               GetExposure,
                               exp_us);
      }

      bool SetExposure(int exp_us) override {
        PYBIND11_OVERLOAD_PURE(
                               bool,
                               pangolin::VideoUvcInterface,
                               SetExposure,
                               exp_us);
      }

      bool GetGain(float& gain) override {
        PYBIND11_OVERLOAD_PURE(
                               bool,
                               pangolin::VideoUvcInterface,
                               GetGain,
                               gain);
      }

      bool SetGain(float gain) override {
        PYBIND11_OVERLOAD_PURE(
                               bool,
                               pangolin::VideoUvcInterface,
                               SetGain,
                               gain);
      }
    };
  
    class PyVideoPlaybackInterface: public pangolin::VideoPlaybackInterface{
    public:
      using pangolin::VideoPlaybackInterface::VideoPlaybackInterface;
    
      size_t GetCurrentFrameId() const override {
        PYBIND11_OVERLOAD_PURE(
                               size_t,
                               pangolin::VideoPlaybackInterface,
                               GetCurrentFrameId);
      }

      size_t GetTotalFrames() const override {
        PYBIND11_OVERLOAD_PURE(
                               size_t,
                               pangolin::VideoPlaybackInterface,
                               GetTotalFrames);
      }

      size_t Seek(size_t frameid) override {
        PYBIND11_OVERLOAD_PURE(
                               size_t,
                               pangolin::VideoPlaybackInterface,
                               Seek,
                               frameid);
      }
    };

  class PyVideoOutputInterface: public pangolin::VideoOutputInterface{
  public:
    using pangolin::VideoOutputInterface::VideoOutputInterface;
    
    const std::vector<pangolin::StreamInfo>& Streams() const override {
      PYBIND11_OVERLOAD_PURE(
                             const std::vector<pangolin::StreamInfo>&,
                             pangolin::VideoOutputInterface,
                             Streams);
    }

    void SetStreams(const std::vector<pangolin::StreamInfo>& streams, const std::string& uri, const picojson::value& properties) override {
      PYBIND11_OVERLOAD_PURE(
                             void,
                             pangolin::VideoOutputInterface,
                             SetStreams,
                             streams,
                             uri,
                             properties);
    }

    int WriteStreams(const unsigned char* data, const picojson::value& frame_properties) override {
      PYBIND11_OVERLOAD_PURE(
                             int,
                             pangolin::VideoOutputInterface,
                             WriteStreams,
                             data,
                             frame_properties);
    }

    bool IsPipe() const override {
      PYBIND11_OVERLOAD_PURE(
                             bool,
                             pangolin::VideoOutputInterface,
                             IsPipe);
    }

  };

  void bind_video(pybind11::module& m){
        pybind11::class_<pangolin::VideoInterface, PyVideoInterface > video_interface(m, "VideoInterface");
    video_interface
      .def(pybind11::init<>())
      .def("SizeBytes", &pangolin::VideoInterface::SizeBytes)
      .def("Streams", &pangolin::VideoInterface::Streams)
      .def("Start", &pangolin::VideoInterface::Start)
      .def("Stop", &pangolin::VideoInterface::Stop)
      .def("GrabNext", &pangolin::VideoInterface::GrabNext)
      .def("GrabNewest", &pangolin::VideoInterface::GrabNewest);

    pybind11::class_<pangolin::GenicamVideoInterface, PyGenicamVideoInterface > genicam_video_interface(m, "GenicamVideoInterface");
    genicam_video_interface
      .def(pybind11::init<>())
      .def("GetParameter", &pangolin::GenicamVideoInterface::GetParameter)
      .def("SetParameter", &pangolin::GenicamVideoInterface::SetParameter);

    pybind11::class_<pangolin::BufferAwareVideoInterface, PyBufferAwareVideoInterface > buffer_aware_video_interface(m, "BufferAwareVideoInterface");
    buffer_aware_video_interface
      .def(pybind11::init<>())
      .def("AvailableFrames", &pangolin::BufferAwareVideoInterface::AvailableFrames)
      .def("DropNFrames", &pangolin::BufferAwareVideoInterface::DropNFrames);

    pybind11::class_<pangolin::VideoPropertiesInterface, PyVideoPropertiesInterface > video_properties_interface(m, "VideoPropertiesInterface");
    video_properties_interface
      .def(pybind11::init<>())
      .def("DeviceProperties", &pangolin::VideoPropertiesInterface::DeviceProperties)
      .def("FrameProperties", &pangolin::VideoPropertiesInterface::FrameProperties);

    pybind11::class_<pangolin::VideoFilterInterface, PyVideoFilterInterface > video_filter_interface(m, "VideoFilterInterface");
    video_filter_interface
      .def(pybind11::init<>())
      //      .def("FindMatchingStreams", &pangolin::VideoFilterInterface::FindMatchingStreams)
      .def("InputStreams", &pangolin::VideoFilterInterface::InputStreams);
    
    pybind11::class_<pangolin::VideoUvcInterface, PyVideoUvcInterface > video_uvc_interface(m, "VideoUvcInterface");
    video_uvc_interface
      .def(pybind11::init<>())
      .def("IoCtrl", &pangolin::VideoUvcInterface::IoCtrl)
      .def("GetExposure", &pangolin::VideoUvcInterface::GetExposure)
      .def("SetExposure", &pangolin::VideoUvcInterface::SetExposure)
      .def("GetGain", &pangolin::VideoUvcInterface::GetGain)
      .def("SetGain", &pangolin::VideoUvcInterface::SetGain);

    pybind11::class_<pangolin::VideoPlaybackInterface, PyVideoPlaybackInterface > video_playback_interface(m, "VideoPlaybackInterface");
    video_playback_interface
      .def(pybind11::init<>())
      .def("GetCurrentFrameId", &pangolin::VideoPlaybackInterface::GetCurrentFrameId)
      .def("GetTotalFrames", &pangolin::VideoPlaybackInterface::GetTotalFrames)
      .def("Seek", &pangolin::VideoPlaybackInterface::Seek);

    pybind11::class_<pangolin::VideoOutputInterface, PyVideoOutputInterface > video_output_interface(m, "VideoOutputInterface");
    video_output_interface
      .def(pybind11::init<>())
      .def("Streams", &pangolin::VideoOutputInterface::Streams)
      .def("SetStreams", &pangolin::VideoOutputInterface::SetStreams) 
      .def("WriteStreams", &pangolin::VideoOutputInterface::WriteStreams)
      .def("IsPipe", &pangolin::VideoOutputInterface::IsPipe);

    pybind11::enum_<pangolin::UvcRequestCode>(m, "UvcRequestCode")
      .value("UVC_RC_UNDEFINED", pangolin::UvcRequestCode::UVC_RC_UNDEFINED)
      .value("UVC_SET_CUR", pangolin::UvcRequestCode::UVC_SET_CUR)
      .value("UVC_GET_CUR", pangolin::UvcRequestCode::UVC_GET_CUR)
      .value("UVC_GET_MIN", pangolin::UvcRequestCode::UVC_GET_MIN)
      .value("UVC_GET_MAX", pangolin::UvcRequestCode::UVC_GET_MAX)
      .value("UVC_GET_RES", pangolin::UvcRequestCode::UVC_GET_RES)
      .value("UVC_GET_LEN", pangolin::UvcRequestCode::UVC_GET_LEN)
      .value("UVC_GET_INFO", pangolin::UvcRequestCode::UVC_GET_INFO)
      .value("UVC_GET_DEF", pangolin::UvcRequestCode::UVC_GET_DEF)
      .export_values();

    pybind11::class_<pangolin::VideoInput>(m, "VideoInput", video_interface)
      .def(pybind11::init<>())
      .def(pybind11::init<const std::string&, const std::string&>(), pybind11::arg("input_uri"), pybind11::arg("output_uri")="pango:[buffer_size_mb=100]//video_log.pango")      
      .def("SizeBytes", &pangolin::VideoInput::SizeBytes)
      .def("Streams", &pangolin::VideoInput::Streams)
      .def("Start", &pangolin::VideoInput::Start)
      .def("Stop", &pangolin::VideoInput::Stop)
      .def("GrabNext", &pangolin::VideoInput::GrabNext)
      .def("GrabNewest", &pangolin::VideoInput::GrabNewest)
      .def("InputStreams", &pangolin::VideoInput::InputStreams)
      .def("Open", &pangolin::VideoInput::Open, pybind11::arg("input_uri"), pybind11::arg("output_uri")="pango:[buffer_size_mb=100]//video_log.pango")      
      .def("Close", &pangolin::VideoInput::Close)
//      .def("Grab", &pangolin::VideoInput::Grab, pybind11::arg("buffer"), pybind11::arg("images"), pybind11::arg("wait")=true, pybind11::arg("newest")=false)
      .def("Grab", [](pangolin::VideoInput& vi, bool wait, bool newest){
            unsigned char *buffer = new unsigned char[vi.SizeBytes()];

            std::vector<pangolin::Image<unsigned char>> imgs;
            pybind11::list imgsList;

            if(vi.Grab(buffer,imgs,wait,newest)) {
                for(size_t s=0; s < vi.Streams().size(); ++s) {
                    // Let's just return the first stream for the moment
                    const pangolin::StreamInfo& si = vi.Streams()[s];
                    const pangolin::Image<uint8_t> img = si.StreamImage(buffer);

                    const int c = si.PixFormat().channels;
                    const int Bpp = si.PixFormat().bpp / (8);
                    const int Bpc = Bpp / c;
                    const int bpc = si.PixFormat().bpp / c;
                    PANGO_ASSERT(bpc == 8 || bpc == 16, "only support 8 or 16 bits channel");

                    pangolin::Image<uint8_t> dstImage(
                        new unsigned char[img.h * img.w * Bpp],
                        img.w, img.h, img.w*Bpp
                    );

                    pangolin::PitchedCopy((char*)dstImage.ptr, dstImage.pitch, (char*)img.ptr, img.pitch, img.w * Bpp, img.h);

                    // Create a Python object that will free the allocated memory
                    pybind11::capsule free_when_done(dstImage.ptr,[](void* f) {
                      unsigned char* buffer = (unsigned char*)f;
                      delete[] buffer;
                    });

                    if (bpc == 8){
                        imgsList.append(
                            pybind11::array_t<uint8_t>(
                            {(int)dstImage.h, (int)dstImage.w, c },
                            {(int)dstImage.pitch, Bpp, Bpc},
                            (uint8_t*)dstImage.ptr,
                            free_when_done)
                            );
                    }
                    else if (bpc == 16){
                        imgsList.append(
                            pybind11::array_t<uint16_t>(
                            {(int)dstImage.h, (int)dstImage.w, c },
                            {(int)dstImage.pitch, Bpp, Bpc},
                            (uint16_t*)dstImage.ptr,
                            free_when_done)
                            );
                    }
                    else{
                        PANGO_ASSERT(false, "incompatible bpc");
                    }
                }
            }
            delete[] buffer;
            return imgsList;

      }, pybind11::arg("wait")=true, pybind11::arg("newest")=false )
      .def("Width", &pangolin::VideoInput::Width)
      .def("Height", &pangolin::VideoInput::Height)
      .def("PixFormat", &pangolin::VideoInput::PixFormat)
      .def("VideoUri", &pangolin::VideoInput::VideoUri)
      .def("Reset", &pangolin::VideoInput::Reset)
      .def("LogFilename", (const std::string& (pangolin::VideoInput::*)() const)&pangolin::VideoInput::LogFilename)
      .def("LogFilename", (std::string& (pangolin::VideoInput::*)())&pangolin::VideoInput::LogFilename)
      .def("Record", &pangolin::VideoInput::Record)
      .def("RecordOneFrame", &pangolin::VideoInput::RecordOneFrame)
      .def("SetTimelapse", &pangolin::VideoInput::SetTimelapse)
      .def("IsRecording", &pangolin::VideoInput::IsRecording);    

    pybind11::class_<pangolin::VideoOutput>(m, "VideoOutput", video_output_interface)
      .def(pybind11::init<>())
      .def(pybind11::init<const std::string&>())
      .def("IsOpen", &pangolin::VideoOutput::IsOpen)
      .def("Open", &pangolin::VideoOutput::Open)
      .def("Close", &pangolin::VideoOutput::Close)
      .def("Streams", &pangolin::VideoOutput::Streams)
//      .def("SetStreams", (void (pangolin::VideoOutput::*)(const std::vector<pangolin::StreamInfo>&, const std::string&, const picojson::value&))&pangolin::VideoOutput::SetStreams, pybind11::arg("streams"), pybind11::arg("uri")="", pybind11::arg("properties") = picojson::value())
//      .def("SetStreams", (void (pangolin::VideoOutput::*)(const std::string&, const picojson::value&))&pangolin::VideoOutput::SetStreams, pybind11::arg("uri")="", pybind11::arg("properties") = picojson::value())
//      .def("WriteStreams", &pangolin::VideoOutput::WriteStreams, pybind11::arg("data"), pybind11::arg("frame_properties") = picojson::value())
      .def("WriteStreams", [](pangolin::VideoOutput& vo, pybind11::list images){
        if(vo.SizeBytes()==0) {
            // Setup stream info
            for(auto& i : images) {
                // num bits per channel
                auto arr = pybind11::array::ensure(i);
                int bpc;
                if(pybind11::isinstance<pybind11::array_t<std::uint8_t>>(arr)){
                   bpc = 8;
                } else if (pybind11::isinstance<pybind11::array_t<std::uint16_t>>(arr)){
                   bpc = 16;
                } else {
                   PANGO_ASSERT(false, "numpy dtype must be either uint8_t or uint16_t");
                }

                // num channels
                PANGO_ASSERT(arr.ndim() == 2 || arr.ndim() == 3, "Method only accepts ndarrays of 2 or 3 dimensions.");
                const size_t channels = (arr.ndim() == 3) ? arr.shape(2) : 1;
                PANGO_ASSERT(channels==1 || channels==3);

                //format string
                std::string fmtStr;
                if (bpc == 8 && channels == 3){
                    fmtStr = "RGB24";
                } else if (bpc == 8 && channels == 1){
                    fmtStr = "GRAY8";
                } else if (bpc == 16 && channels ==1){
                    fmtStr = "GRAY16LE";
                } else {
                    PANGO_ASSERT(false, "format is not supported");
                }
                vo.AddStream(pangolin::PixelFormatFromString(fmtStr), arr.shape(1), arr.shape(0));
            }
            vo.SetStreams("python://");
        }

        std::unique_ptr<uint8_t[]> buffer(new uint8_t[vo.SizeBytes()]);
        std::vector<pangolin::Image<unsigned char>> bimgs = vo.GetOutputImages(buffer.get());
        PANGO_ASSERT(bimgs.size() == images.size(), "length of input streams not consistent");
        for(size_t i=0; i < images.size(); ++i) {
            const pangolin::StreamInfo& so = vo.Streams()[i];
            const unsigned int bpc = so.PixFormat().channel_bit_depth;
            const unsigned int Bpp = so.PixFormat().bpp / (8);
            PANGO_ASSERT(bpc == 8 || bpc == 16, "only support 8 or 16 bit depth");
            if (bpc == 8){
                pybind11::array_t<unsigned char> arr = images[i].cast<pybind11::array_t<unsigned char>>();
                pangolin::Image<uint8_t> srcImg(arr.mutable_data(0,0), bimgs[i].w, bimgs[i].h, bimgs[i].w * Bpp);
                pangolin::PitchedCopy((char*)bimgs[i].ptr, bimgs[i].pitch, (char*)srcImg.ptr,
                                      srcImg.pitch, srcImg.pitch, srcImg.h);
            }else if (bpc == 16){
                pybind11::array_t<uint16_t> arr = images[i].cast<pybind11::array_t<uint16_t>>();
                pangolin::Image<uint16_t> srcImg(arr.mutable_data(0,0), bimgs[i].w, bimgs[i].h, bimgs[i].w * Bpp);
                pangolin::PitchedCopy((char*)bimgs[i].ptr, bimgs[i].pitch, (char*)srcImg.ptr,
                                      srcImg.pitch, srcImg.pitch, srcImg.h);
            }else{
                PANGO_ASSERT(false, "format is not supported");
            }

        }
        vo.WriteStreams(buffer.get());
      }, pybind11::arg("images"))
      .def("IsPipe", &pangolin::VideoOutput::IsPipe)
      .def("AddStream", (void (pangolin::VideoOutput::*)(const pangolin::PixelFormat&, size_t,size_t,size_t))&pangolin::VideoOutput::AddStream)
      .def("AddStream", (void (pangolin::VideoOutput::*)(const pangolin::PixelFormat&, size_t,size_t))&pangolin::VideoOutput::AddStream)
      .def("SizeBytes", &pangolin::VideoOutput::SizeBytes)
      .def("GetOutputImages", (std::vector<pangolin::Image<unsigned char>> (pangolin::VideoOutput::*)(unsigned char*) const)&pangolin::VideoOutput::GetOutputImages)    
      .def("GetOutputImages", (std::vector<pangolin::Image<unsigned char>> (pangolin::VideoOutput::*)(std::vector<unsigned char>&) const)&pangolin::VideoOutput::GetOutputImages);   
    
  }
}  // py_pangolin
