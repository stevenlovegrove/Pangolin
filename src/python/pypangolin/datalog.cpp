
//
// Copyright (c) Andrey Mnatsakanov
//

#include "datalog.hpp"

#include <pybind11/stl.h>
#include <pangolin/plot/datalog.h>

namespace py_pangolin {

  void bind_datalog(pybind11::module& m){
    pybind11::class_<pangolin::DimensionStats >(m, "DimensionStats")
      .def(pybind11::init<>())
      .def("Reset", &pangolin::DimensionStats::Reset)
      .def("Add", &pangolin::DimensionStats::Add)
      .def_readwrite("isMonotonic", &pangolin::DimensionStats::isMonotonic)
      .def_readwrite("sum", &pangolin::DimensionStats::sum)
      .def_readwrite("sum_sq", &pangolin::DimensionStats::sum_sq)
      .def_readwrite("min", &pangolin::DimensionStats::min)
      .def_readwrite("max", &pangolin::DimensionStats::max);

    pybind11::class_<pangolin::DataLogBlock>(m, "DataLogBlock")
      .def(pybind11::init<size_t,size_t,size_t>())
      .def("Samples", &pangolin::DataLogBlock::Samples)
      .def("MaxSamples", &pangolin::DataLogBlock::MaxSamples)
      .def("SampleSpaceLeft", &pangolin::DataLogBlock::SampleSpaceLeft)
      .def("IsFull", &pangolin::DataLogBlock::IsFull)
      .def("AddSamples", &pangolin::DataLogBlock::AddSamples)
      .def("ClearLinked", &pangolin::DataLogBlock::ClearLinked)
      .def("NextBlock", &pangolin::DataLogBlock::NextBlock)
      .def("StartId", &pangolin::DataLogBlock::StartId)
      .def("DimData", &pangolin::DataLogBlock::DimData)
      .def("Dimensions", &pangolin::DataLogBlock::Dimensions)
      .def("Sample", &pangolin::DataLogBlock::Sample)
      .def("StartId", &pangolin::DataLogBlock::StartId);    

    pybind11::class_<pangolin::DataLog>(m, "DataLog")
      .def(pybind11::init<unsigned int>(), pybind11::arg("block_samples_alloc")=10000)
      .def("SetLabels", &pangolin::DataLog::SetLabels)
      .def("Labels", &pangolin::DataLog::Labels)
      .def("Log", (void (pangolin::DataLog::*)(size_t, const float*, unsigned int))&pangolin::DataLog::Log, pybind11::arg("dimension"), pybind11::arg("vals"), pybind11::arg("samples")=1)
      .def("Log", (void (pangolin::DataLog::*)(float))&pangolin::DataLog::Log)
      .def("Log", (void (pangolin::DataLog::*)(float, float))&pangolin::DataLog::Log)
      .def("Log", (void (pangolin::DataLog::*)(float, float, float))&pangolin::DataLog::Log)
      .def("Log", (void (pangolin::DataLog::*)(float, float, float, float))&pangolin::DataLog::Log)
      .def("Log", (void (pangolin::DataLog::*)(float, float, float, float, float))&pangolin::DataLog::Log)
      .def("Log", (void (pangolin::DataLog::*)(float, float, float, float, float, float))&pangolin::DataLog::Log)
      .def("Log", (void (pangolin::DataLog::*)(float, float, float, float, float, float, float))&pangolin::DataLog::Log)
      .def("Log", (void (pangolin::DataLog::*)(float, float, float, float, float, float, float, float))&pangolin::DataLog::Log)
      .def("Log", (void (pangolin::DataLog::*)(float, float, float, float, float, float, float, float, float))&pangolin::DataLog::Log)
      .def("Log", (void (pangolin::DataLog::*)(float, float, float, float, float, float, float, float, float, float))&pangolin::DataLog::Log)
      .def("Log", (void (pangolin::DataLog::*)(const std::vector<float>&))&pangolin::DataLog::Log)
      .def("Clear", &pangolin::DataLog::Clear)
      .def("Save", &pangolin::DataLog::Save)
      .def("FirstBlock", &pangolin::DataLog::FirstBlock)
      .def("LastBlock", &pangolin::DataLog::LastBlock)
      .def("Samples", &pangolin::DataLog::Samples)
      .def("Sample", &pangolin::DataLog::Sample)
      .def("Stats", &pangolin::DataLog::Stats);
  }

}  // py_pangolin
