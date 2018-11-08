/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) Andrey Mnatsakanov
 * Copyright (c) Qi Hang
 * Copyright (c) Adel Ahmadyan
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

#include "gl_draw.hpp"
#include <pangolin/gl/gldraw.h>
#include <pybind11/stl.h>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py_pangolin {

  void DrawPoints(pybind11::array_t<double> points) {
      auto r = points.unchecked<2>();
      glBegin(GL_POINTS);
      for (ssize_t i = 0; i < r.shape(0); ++i) {
          glVertex3d(r(i, 0), r(i, 1), r(i, 2));
      } 
      glEnd();
  }

  void DrawPoints(pybind11::array_t<double> points, pybind11::array_t<double> colors) {
      auto r = points.unchecked<2>();
      auto rc = colors.unchecked<2>();

      glBegin(GL_POINTS);
      for (ssize_t i = 0; i < r.shape(0); ++i) {
          glColor3f(rc(i, 0), rc(i, 1), rc(i, 2));
          glVertex3d(r(i, 0), r(i, 1), r(i, 2));
      }
      glEnd();
  }


  void DrawCameras(pybind11::array_t<double> cameras, float w=1.0, float h_ratio=0.75, float z_ratio=0.6) {
      auto r = cameras.unchecked<3>();

      float h = w * h_ratio;
      float z = w * z_ratio;

      for (ssize_t i = 0; i < r.shape(0); ++i) {
          glPushMatrix();
          // glMultMatrixd(r.data(i, 0, 0));
          glMultTransposeMatrixd(r.data(i, 0, 0));

          glBegin(GL_LINES);
          glVertex3f(0,0,0);
          glVertex3f(w,h,z);
          glVertex3f(0,0,0);
          glVertex3f(w,-h,z);
          glVertex3f(0,0,0);
          glVertex3f(-w,-h,z);
          glVertex3f(0,0,0);
          glVertex3f(-w,h,z);

          glVertex3f(w,h,z);
          glVertex3f(w,-h,z);

          glVertex3f(-w,h,z);
          glVertex3f(-w,-h,z);

          glVertex3f(-w,h,z);
          glVertex3f(w,h,z);

          glVertex3f(-w,-h,z);
          glVertex3f(w,-h,z);
          glEnd();

          glPopMatrix();
      }
  }

  void DrawCamera(pybind11::array_t<double> camera, float w=1.0, float h_ratio=0.75, float z_ratio=0.6) {
      auto r = camera.unchecked<2>();

      float h = w * h_ratio;
      float z = w * z_ratio;

      glPushMatrix();
      // glMultMatrixd(r.data(0, 0));
      glMultTransposeMatrixd(r.data(0, 0));

      glBegin(GL_LINES);
      glVertex3f(0,0,0);
      glVertex3f(w,h,z);
      glVertex3f(0,0,0);
      glVertex3f(w,-h,z);
      glVertex3f(0,0,0);
      glVertex3f(-w,-h,z);
      glVertex3f(0,0,0);
      glVertex3f(-w,h,z);

      glVertex3f(w,h,z);
      glVertex3f(w,-h,z);

      glVertex3f(-w,h,z);
      glVertex3f(-w,-h,z);

      glVertex3f(-w,h,z);
      glVertex3f(w,h,z);

      glVertex3f(-w,-h,z);
      glVertex3f(w,-h,z);
      glEnd();

      glPopMatrix();
  }


  void DrawLine(pybind11::array_t<double> points, float point_size=0) {
      auto r = points.unchecked<2>();
      glBegin(GL_LINES);
      for (ssize_t i = 0; i < r.shape(0)-1; ++i) {
          glVertex3d(r(i, 0), r(i, 1), r(i, 2));
          glVertex3d(r(i+1, 0), r(i+1, 1), r(i+1, 2));
      }
      glEnd();

      if(point_size > 0) {
          glPointSize(point_size);
          glBegin(GL_POINTS);
          for (ssize_t i = 0; i < r.shape(0); ++i) {
              glVertex3d(r(i, 0), r(i, 1), r(i, 2));
          }
          glEnd();
      }
  }



  void DrawLines(pybind11::array_t<double> points, float point_size=0) {
      auto r = points.unchecked<2>();

      glBegin(GL_LINES);
      for (ssize_t i = 0; i < r.shape(0); ++i) {
          glVertex3d(r(i, 0), r(i, 1), r(i, 2));
          glVertex3d(r(i, 3), r(i, 4), r(i, 5));
      }
      glEnd();

      if(point_size > 0) {
          glPointSize(point_size);
          glBegin(GL_POINTS);
          for (ssize_t i = 0; i < r.shape(0); ++i) {
              glVertex3d(r(i, 0), r(i, 1), r(i, 2));
              glVertex3d(r(i, 3), r(i, 4), r(i, 5));
          }
          glEnd();
      }
  }


  void DrawLines(pybind11::array_t<double> points, pybind11::array_t<double> points2, float point_size=0) {
      auto r = points.unchecked<2>();
      auto r2 = points2.unchecked<2>();
      glBegin(GL_LINES);
      for (ssize_t i = 0; i < std::min(r.shape(0), r2.shape(0)); ++i) {
          glVertex3d(r(i, 0), r(i, 1), r(i, 2));
          glVertex3d(r2(i, 0), r2(i, 1), r2(i, 2));
      }
      glEnd();

      if(point_size > 0) {
          glPointSize(point_size);
          glBegin(GL_POINTS);
          for (ssize_t i = 0; i < std::min(r.shape(0), r2.shape(0)); ++i) {
              glVertex3d(r(i, 0), r(i, 1), r(i, 2));
              glVertex3d(r2(i, 0), r2(i, 1), r2(i, 2));
          }
          glEnd();
      }
  }


  void DrawBoxes(pybind11::array_t<double> cameras, pybind11::array_t<double> sizes) {
      auto r = cameras.unchecked<3>();
      auto rs = sizes.unchecked<2>();

      for (ssize_t i = 0; i < r.shape(0); ++i) {
          glPushMatrix();
          // glMultMatrixd(r.data(i, 0, 0));
          glMultTransposeMatrixd(r.data(i, 0, 0));

          float w = *rs.data(i, 0) / 2.0;  // w/2
          float h = *rs.data(i, 1) / 2.0;
          float z = *rs.data(i, 2) / 2.0;

          glBegin(GL_LINES);
          glVertex3f(-w, -h, -z);
          glVertex3f(w, -h, -z);
          glVertex3f(-w, -h, -z);
          glVertex3f(-w, h, -z);
          glVertex3f(-w, -h, -z);
          glVertex3f(-w, -h, z);

          glVertex3f(w, h, -z);
          glVertex3f(-w, h, -z);
          glVertex3f(w, h, -z);
          glVertex3f(w, -h, -z);
          glVertex3f(w, h, -z);
          glVertex3f(w, h, z);

          glVertex3f(-w, h, z);
          glVertex3f(w, h, z);
          glVertex3f(-w, h, z);
          glVertex3f(-w, -h, z);
          glVertex3f(-w, h, z);
          glVertex3f(-w, h, -z);

          glVertex3f(w, -h, z);
          glVertex3f(-w, -h, z);
          glVertex3f(w, -h, z);
          glVertex3f(w, h, z);
          glVertex3f(w, -h, z);
          glVertex3f(w, -h, -z);
          glEnd();

          glPopMatrix();
      }
  }


  void bind_gl_draw(pybind11::module &m){
    m.def("glDrawColouredCube",
          &pangolin::glDrawColouredCube,
          pybind11::arg("axis_min") = -0.5f,
          pybind11::arg("axis_max") = +0.5f);
    
    m.def("DrawPoints", 
          (void (*) (pybind11::array_t<double>)) &DrawPoints, 
          pybind11::arg("points"));

    m.def("DrawPoints", 
          (void (*) (pybind11::array_t<double>, pybind11::array_t<double>)) &DrawPoints, 
          pybind11::arg("points"), 
          pybind11::arg("colors"));

    m.def("DrawLine", 
        (void (*) (pybind11::array_t<double>, float)) &DrawLine,
        pybind11::arg("points"), 
        pybind11::arg("point_size") = 0);

    m.def("DrawLines", 
        (void (*) (pybind11::array_t<double>, float)) &DrawLines,
        pybind11::arg("points"), 
        pybind11::arg("point_size") = 0);

    m.def("DrawLines", 
        (void (*) (pybind11::array_t<double>, pybind11::array_t<double>, float)) &DrawLines,
        pybind11::arg("points"), 
        pybind11::arg("points2"), 
        pybind11::arg("point_size") = 0);

    m.def("DrawCameras", 
          &DrawCameras, 
          pybind11::arg("poses"), 
          pybind11::arg("w") = 1.0f, 
          pybind11::arg("h_ratio") = 0.75f, 
          pybind11::arg("z_ratio") = 0.6);

    m.def("DrawCamera", 
          &DrawCamera, 
          pybind11::arg("poses"), 
          pybind11::arg("w") = 1.0f, 
          pybind11::arg("h_ratio") = 0.75f, 
          pybind11::arg("z_ratio") = 0.6f);

    m.def("DrawBoxes",
          &DrawBoxes, 
          pybind11::arg("cameras"), 
          pybind11::arg("sizes"));

  }

}  // py_pangolin
