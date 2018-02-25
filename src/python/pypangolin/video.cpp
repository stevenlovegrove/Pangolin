
//
// Copyright (c) Andrey Mnatsakanov
//

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
            vi.Grab(buffer,imgs,wait,newest);

            // Create a Python object that will free the allocated memory
            pybind11::capsule free_when_done(buffer,[](void* f) {
                unsigned char* buffer = (unsigned char*)f;
                delete[] buffer;
            });

            // Let's just return the first stream for the moment
            const pangolin::StreamInfo& si = vi.Streams()[0];
            const int c = si.PixFormat().channels;
            const int Bpc = si.PixFormat().bpp / (8*c);

            return pybind11::array_t<unsigned char>(
                {(int)si.Height(), (int)si.Width(), c },
                {(int)si.Pitch(), c, Bpc},
                buffer,
                free_when_done);
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
      .def("IsPipe", &pangolin::VideoOutput::IsPipe)
      .def("AddStream", (void (pangolin::VideoOutput::*)(const pangolin::PixelFormat&, size_t,size_t,size_t))&pangolin::VideoOutput::AddStream)
      .def("AddStream", (void (pangolin::VideoOutput::*)(const pangolin::PixelFormat&, size_t,size_t))&pangolin::VideoOutput::AddStream)
      .def("SizeBytes", &pangolin::VideoOutput::SizeBytes)
      .def("GetOutputImages", (std::vector<pangolin::Image<unsigned char>> (pangolin::VideoOutput::*)(unsigned char*) const)&pangolin::VideoOutput::GetOutputImages)    
      .def("GetOutputImages", (std::vector<pangolin::Image<unsigned char>> (pangolin::VideoOutput::*)(std::vector<unsigned char>&) const)&pangolin::VideoOutput::GetOutputImages);   
    
  }
}  // py_pangolin
