---
layout: default
title: Building Pangolin
permalink: /:basename/
---

Building Pangolin
====================================
Find the latest version on [Github](http://github.com/stevenlovegrove/Pangolin):

```bash
git clone https://github.com/stevenlovegrove/Pangolin.git
```

## Dependencies ##

Optional dependencies are enabled when found, otherwise they are silently disabled.
Check the CMake configure output for details.

### Required Dependencies ###

* C++11

* OpenGL (Desktop / ES / ES2)
  * (lin) `sudo apt install libgl1-mesa-dev`

* Glew
  * (win) built automatically (assuming git is on your path)
  * (deb) `sudo apt install libglew-dev`
  * (mac) `sudo port install glew`

* CMake (for build environment)
  * (win) http://www.cmake.org/cmake/resources/software.html
  * (deb) `sudo apt install cmake`
  * (mac) `sudo port install cmake`

### Recommended Dependencies ###

* Python2 / Python3, for drop-down interactive console
  * (win) http://www.python.org/downloads/windows
  * (deb) `sudo apt install libpython2.7-dev`
  * (mac) preinstalled with osx
  * (for pybind11) `git submodule init && git submodule update`
  * (useful modules) `sudo python -mpip install numpy pyopengl Pillow pybind11`

* Wayland
  * pkg-config: `sudo apt install pkg-config`
  * Wayland and EGL:`sudo apt install libegl1-mesa-dev libwayland-dev libxkbcommon-dev wayland-protocols`

### Optional Dependencies for video input ###

* FFMPEG (For video decoding and image rescaling)
  * (deb) `sudo apt install ffmpeg libavcodec-dev libavutil-dev libavformat-dev libswscale-dev libavdevice-dev`

* DC1394 (For firewire input)
  * (deb) `sudo apt install libdc1394-22-dev libraw1394-dev`

* libuvc (For cross-platform webcam video input via libusb)
  * git://github.com/ktossell/libuvc.git

* libjpeg, libpng, libtiff, libopenexr (For reading still-image sequences)
  * (deb) `sudo apt install libjpeg-dev libpng12-dev libtiff5-dev libopenexr-dev`

* OpenNI / OpenNI2 (For Kinect / Xtrion / Primesense capture)

* DepthSense SDK

### Very Optional Dependencies ###

* Eigen / TooN (These matrix types supported in the Pangolin API.)

* CUDA Toolkit >= 3.2 (Some CUDA header-only interop utilities included)
  * http://developer.nvidia.com/cuda-downloads

* Doxygen for generating html / pdf documentation.

## Build Natively ##

Pangolin uses the CMake portable pre-build tool. To checkout and build pangolin in the
directory 'build', execute the following at a shell (or the equivelent using a GUI):

```bash
git clone https://github.com/stevenlovegrove/Pangolin.git
cd Pangolin
mkdir build && cd build
cmake ..
cmake --build .
```

If you would like to build the documentation and you have Doxygen installed, you
can execute:

```bash
cmake --build . --target pangolin_doc
```

**On Windows**, Pangolin will attempt to download and build *glew*, *libjpeg*, *libpng* and *zlib* automatically. It does so assuming that git is available on the path - this assumption may be wrong for windows users who have downloaded Pangolin via a zip file on github. You will instead need to download and compile the dependencies manually, and set the BUILD_EXTERN_(lib) options to false for these libraries. The alternate and recommended approach is to install [gitbash](https://git-scm.com/downloads) and work from within their provided console.

## Build for the Web (with Emscripten) ##

Emscripten is a neat c++ compiler which can output javascript executable code. That's right, your Pangolin programs can run on the web, too!

Follow Emscriptens instructions to install the SDK (summerized below):

```bash
mkdir ~/tools && cd ~/tools
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest
```

To build Pangolin with the Emscripten toolchain, create a new build directory, make sure Emscripten is on your `PATH` (by sourcing the utility as shown below), and use the `emcmake` utility to run CMake with the custom toolchain settings. Note that any dependencies of Pangolin must also be compiled emscripten - minimally that could just be Eigen.

```bash
source ~/tools/emsdk/emsdk_env.sh
git clone https://github.com/stevenlovegrove/Pangolin.git
cd Pangolin
mkdir build-em && cd build-em
emcmake cmake .. -DEigen3_DIR=PATH_TO_EIGEN/build-em
cmake --build .
```

## Issues ##

Please visit [Github Issues](https://github.com/stevenlovegrove/Pangolin/issues) to view and report problems with Pangolin. Issues and pull requests should be raised against the master branch which contains the current development version.

Please note; most Pangolin dependencies are optional - to disable a dependency which may be causing trouble on your machine, set the BUILD_PANGOLIN_(option) variable to false with a cmake configuration tool (e.g. ccmake or cmake-gui).

## Contributions and Continuous Integration ##

To contribute to Pangolin, I would appreciate pull requests against the master branch. This will trigger CI builds for your changes automatically, and help me to merge with confidence.



