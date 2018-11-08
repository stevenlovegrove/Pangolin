/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
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

#include "gl.hpp"
#include <pangolin/gl/gl.h>
#include <pybind11/stl.h>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "colour.hpp"
#include <pangolin/image/image_io.h>


namespace py_pangolin {

    void bind_gl(pybind11::module & m) {

        pybind11::class_<pangolin::GlTexture, std::shared_ptr<pangolin::GlTexture>>(m, "GlTexture")
            .def(pybind11::init<>(), "Default constructor, represents 'no texture'")
            .def(pybind11::init<GLint, GLint, GLint, bool, int, GLenum, GLenum, GLvoid*>(),
                pybind11::arg("width"), 
                pybind11::arg("height"), 
                pybind11::arg("internal_format") = GL_RGBA8, 
                pybind11::arg("sampling_linear") = true, 
                pybind11::arg("border") = 0,
                pybind11::arg("glformat") = GL_RGBA, 
                pybind11::arg("gltype") = GL_UNSIGNED_BYTE, 
                pybind11::arg("data") = nullptr,
                "internal_format normally one of GL_RGBA8, GL_LUMINANCE8, GL_INTENSITY16")

            .def("Reinitialise", 
                &pangolin::GlTexture::Reinitialise, 
                pybind11::arg("width"), 
                pybind11::arg("height"), 
                pybind11::arg("internal_format") = GL_RGBA8, 
                pybind11::arg("sampling_linear") = true, 
                pybind11::arg("border") = 0, 
                pybind11::arg("glformat") = GL_RGBA, 
                pybind11::arg("gltype") = GL_UNSIGNED_BYTE, 
                pybind11::arg("data") = nullptr, 
                "Reinitialise teture width / height / format")  // virtual function

            .def("IsValid", 
                &pangolin::GlTexture::IsValid)

            .def("Delete", 
                &pangolin::GlTexture::Delete, 
                "Delete OpenGL resources and fall back to representing 'no texture'")

            .def("Bind", 
                &pangolin::GlTexture::Bind)
                
            .def("Unbind", 
                &pangolin::GlTexture::Unbind)

            .def("Upload", 
                (void (pangolin::GlTexture::*) (const void*, GLenum, GLenum)) 
                &pangolin::GlTexture::Upload,
                pybind11::arg("image"), 
                pybind11::arg("data_format") = GL_LUMINANCE, 
                pybind11::arg("data_type") = GL_FLOAT,
                "data_layout normally one of GL_LUMINANCE, GL_RGB, ...\n"
                "data_type normally one of GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_FLOAT")

            .def("Upload", 
                (void (pangolin::GlTexture::*) (const void*, GLsizei, GLsizei, GLsizei, GLsizei, GLenum, GLenum))
                &pangolin::GlTexture::Upload, 
                pybind11::arg("data"), 
                pybind11::arg("tex_x_offset"),  
                pybind11::arg("tex_y_offset"), 
                pybind11::arg("data_w"), 
                pybind11::arg("data_h"),
                pybind11::arg("data_format"), 
                pybind11::arg("data_type"), 
                "Upload data to texture, overwriting a sub-region of it.\n"
                "data ptr contains packed data_w x data_h of pixel data.")

            .def("Download", 
                (void (pangolin::GlTexture::*)(void*, GLenum, GLenum) const) 
                &pangolin::GlTexture::Download, 
                pybind11::arg("image"), 
                pybind11::arg("data_layout") = GL_LUMINANCE, 
                pybind11::arg("data_type") = GL_FLOAT)

            .def("Download", 
                (void (pangolin::GlTexture::*)(pangolin::TypedImage&) const) 
                &pangolin::GlTexture::Download, 
                pybind11::arg("image"))

            .def("Upload", 
                [](pangolin::GlTexture &t, pybind11::array_t<unsigned char> img, GLenum data_format, GLenum data_type) {
                    auto buf = img.request();
                    unsigned char* data = (unsigned char*) buf.ptr;
                    t.Upload(data, data_format, data_type);
                },
                pybind11::arg("image"), 
                pybind11::arg("data_format") = GL_RGB, 
                pybind11::arg("data_type") = GL_UNSIGNED_BYTE)

            .def("Load", 
                &pangolin::GlTexture::Load, 
                pybind11::arg("image"), 
                pybind11::arg("sampling_linear") = true)

            .def("LoadFromFile", 
                &pangolin::GlTexture::LoadFromFile, 
                pybind11::arg("filename"), 
                pybind11::arg("sampling_linear"))

            .def("Save", 
                &pangolin::GlTexture::Save, 
                pybind11::arg("filename"), 
                pybind11::arg("top_line_first") = true)

            .def("SetLinear", 
                &pangolin::GlTexture::SetLinear)

            .def("SetNearestNeighbour", 
                &pangolin::GlTexture::SetNearestNeighbour)

            .def("RenderToViewport", 
                (void (pangolin::GlTexture::*) () const) &pangolin::GlTexture::RenderToViewport)

            .def("RenderToViewport", 
                (void (pangolin::GlTexture::*) (pangolin::Viewport, bool, bool) const) &pangolin::GlTexture::RenderToViewport,
                pybind11::arg("tex_vp"), 
                pybind11::arg("flipx") = false, 
                pybind11::arg("flipy") = false)

            .def("RenderToViewport", 
                (void (pangolin::GlTexture::*) (const bool) const) &pangolin::GlTexture::RenderToViewport, 
                pybind11::arg("flip"))
                
            .def("RenderToViewportFlipY", 
                &pangolin::GlTexture::RenderToViewportFlipY)

            .def("RenderToViewportFlipXFlipY", 
                &pangolin::GlTexture::RenderToViewportFlipXFlipY)

            .def_readwrite("internal_format", &pangolin::GlTexture::internal_format)

            .def_readwrite("tid", &pangolin::GlTexture::tid)

            .def_readwrite("width", &pangolin::GlTexture::width)

            .def_readwrite("height", &pangolin::GlTexture::height)

            // move constructor
            // move assignment
            // private copy constructor
        
        ;

        // GlRenderBuffer
        // GlFramebuffer
        // GlBufferType
        // GlBuffer
        // GlSizeableBuffer
        // 
    }

}  // py_pangolin
