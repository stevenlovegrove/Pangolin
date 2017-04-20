What is Pangolin {#mainpage}
====================================

Pangolin is a lightweight portable rapid development library for managing OpenGL
display / interaction and abstracting video input. At its heart is a simple
OpenGl viewport manager which can help to modularise 3D visualisation without
adding to its complexity, and offers an advanced but intuitive 3D navigation
handler. Pangolin also provides a mechanism for manipulating program variables
through config files and ui integration, and has a flexible real-time plotter
for visualising graphical data.

The ethos of Pangolin is to reduce the boilerplate code that normally
gets written to visualise and interact with (typically image and 3D
based) systems, without compromising performance. It also enables write-once
code for a number of platforms, currently including Windows, Linux, OSX, Android
and IOS.

## Code ##

Find the latest version on [Github](http://github.com/stevenlovegrove/Pangolin):

```
git clone https://github.com/stevenlovegrove/Pangolin.git
```

## Dependencies ##

Optional dependencies are enabled when found, otherwise they are silently disabled.
Check the CMake configure output for details.

### Required Dependencies ###

* C++11

* OpenGL (Desktop / ES / ES2)

* Glew
 * (win) built automatically (assuming git is on your path)
 * (deb) sudo apt-get install libglew-dev
 * (mac) sudo port install glew

* CMake (for build environment)
 * (win) http://www.cmake.org/cmake/resources/software.html
 * (deb) sudo apt-get install cmake
 * (mac) sudo port install cmake

### Recommended Dependencies ###

* Python2 / Python3, for drop-down interactive console
 * (win) http://www.python.org/downloads/windows
 * (deb) sudo apt-get install libpython2.7-dev
 * (mac) preinstalled with osx

### Optional Dependencies for video input ###

* FFMPEG (For video decoding and image rescaling)
 * (deb) sudo apt-get install ffmpeg libavcodec-dev libavutil-dev libavformat-dev libswscale-dev

* DC1394 (For firewire input)
 * (deb) sudo apt-get install libdc1394-22-dev libraw1394-dev

* libuvc (For cross-platform webcam video input via libusb)
 * git://github.com/ktossell/libuvc.git

* libjpeg, libpng, libtiff, libopenexr (For reading still-image sequences)
 * (deb) sudo apt-get install libjpeg-dev libpng12-dev libtiff5-dev libopenexr-dev

* OpenNI / OpenNI2 (For Kinect / Xtrion / Primesense capture)

* DepthSense SDK

### Very Optional Dependencies ###

* Eigen / TooN (These matrix types supported in the Pangolin API.)

* CUDA Toolkit >= 3.2 (Some CUDA header-only interop utilities included)
 * http://developer.nvidia.com/cuda-downloads

* Doxygen for generating html / pdf documentation.

## Building ##

Pangolin uses the CMake portable pre-build tool. To checkout and build pangolin in the
directory 'build', execute the following at a shell (or the equivelent using a GUI):

```
git clone https://github.com/stevenlovegrove/Pangolin.git
cd Pangolin
mkdir build
cd build
cmake ..
make -j
sudo make install
```

If you would like to build the documentation and you have Doxygen installed, you
can execute:

```
make doc
```

**On Windows**, Pangolin will attempt to download and build *glew*, *libjpeg*, *libpng* and *zlib* automatically. It does so assuming that git is available on the path - this assumption may be wrong for windows users who have downloaded Pangolin via a zip file on github. You will instead need to download and compile the dependencies manually, and set the BUILD_EXTERN_(lib) options to false for these libraries. The alternate and recommended approach is to install [gitbash](https://git-scm.com/downloads) and work from within their provided console.

## Issues ##

Please visit [Github Issues](https://github.com/stevenlovegrove/Pangolin/issues) to view and report problems with Pangolin. Issues and pull requests should be raised against the master branch which contains the current development version.

Please note; most Pangolin dependencies are optional - to disable a dependency which may be causing trouble on your machine, set the BUILD_PANGOLIN_(option) variable to false with a cmake configuration tool (e.g. ccmake or cmake-gui).

## Contributions and Continuous Integration ##

For CI, Pangolin uses [travis-ci.org](https://travis-ci.org/stevenlovegrove/Pangolin) for Ubuntu, OSX and [ci.appveyor.com](https://ci.appveyor.com/project/stevenlovegrove/pangolin) for Windows.

To contribute to Pangolin, I would appreciate pull requests against the master branch. This will trigger CI builds for your changes automatically, and help me to merge with confidence.

## Binaries ##

Binaries are available for Windows x64, as output by the Windows CI server: [Appveyor Artifacts](https://ci.appveyor.com/project/stevenlovegrove/pangolin/build/artifacts).

## Acknowledgements ##

I'd like to thank the growing number of kind contributors to Pangolin for helping to make it more stable and feature rich. Many features of Pangolin have been influenced by other projects such as GFlags, GLConsole, and libcvd in particular. I'd also like to thank the FOSS projects on which Pangolin depends.

For a summary of those who have made code contributions, execute:

```
git shortlog -sne
```
