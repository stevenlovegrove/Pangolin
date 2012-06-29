== What is Pangolin ==

Pangolin is a lightweight rapid development library for managing OpenGL
display / interaction and video input. At its heart is a simple OpenGl
viewport manager which can help to modularise 3D visualisation without
adding to its complexity, and offers an advanced but intuitive 3D
navigation handler. Pangolin also provides a mechanism for manipulating
program variables through config files and ui integration.

The ethos of Pangolin is to reduce the boilerplate code that normally
gets written to visualise and interact with (typically image and 3D
based) systems, without compromising performance.

== Required Dependencies ==

* OpenGL

* Boost
  (win) http://www.boost.org/users/download/
  (deb) sudo apt-get install libboost-dev libboost-thread-dev
  (mac) sudo port install boost

* CMake (for build environment)
  (win) http://www.cmake.org/cmake/resources/software.html
  (deb) sudo apt-get install cmake
  (mac) sudo port install cmake

* FreeGlut / GLU / Glew (Required for drawing text and windowing)
  (win) http://www.transmissionzero.co.uk/software/freeglut-devel/
  (deb) sudo apt-get install freeglut3-dev libglu-dev libglew-dev
  (mac) sudo port install freeglut glew

== Optional Dependencies ==

* FFMPEG (For video decoding and image rescaling)
  (deb) sudo apt-get install ffmpeg libavcodec-dev libavutil-dev libavformat-dev libswscale-dev

* DC1394 (For firewire input)
  (deb) sudo apt-get install libdc1394-22-dev libraw1394-dev

== Very Optional Dependencies ==

* CUDA Toolkit (>= 3.2)
  http://developer.nvidia.com/object/cuda_3_2_downloads.html

* Cg Library (some small Cg utils)
  (deb) sudo apt-get install nvidia-cg-toolkit
