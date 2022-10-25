#include "pypangoio.h"

namespace py_pangolin
{

PyPangoIO::PyPangoIO(std::queue<InterpreterLine>& line_queue, InterpreterLineType line_type)
    : line_queue(line_queue), line_type(line_type)
{
}

void PyPangoIO::write(const std::string& text)
{
    buffer += text;
    size_t nl = buffer.find_first_of('\n');
    while(nl != std::string::npos) {
        const std::string line = buffer.substr(0,nl);
        line_queue.push(InterpreterLine(line,line_type));
        buffer = buffer.substr(nl+1);
        nl = buffer.find_first_of('\n');
    }
}

void PyPangoIO::flush()
{
    // Do nothing
}

void bind_pango_write_object(pybind11::module& m)
{
    pybind11::class_<PyPangoIO>(m, "PyPangoIO")
      .def("write", &PyPangoIO::write)
      .def("flush", &PyPangoIO::flush);
}

}
