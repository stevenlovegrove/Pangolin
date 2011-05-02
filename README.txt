== What is Pangolin ==

Pangolin is a Lightweight collection of utilities for rapid vision prototyping.
At its heart is a simple OpenGl display library which can help manage viewports
and offers an advanced but intuitive 3D navigation handler. Pangolin also
provides a mechanism for manipulating program variables through config files and
ui integration, and offers a simple mechanism for lightweight video input.

== Required Dependencies ==

* OpenGL

* Boost
  (win) http://www.boost.org/users/download/
  (deb) sudo apt-get install libboost-dev libboost-thread-dev

* CMake (for build environment)
  (win) http://www.cmake.org/cmake/resources/software.html
  (deb) sudo apt-get install cmake

== Optional Dependencies ==

* FreeGlut / GLU / Glew (Recommended for examples)
  (win) http://www.transmissionzero.co.uk/software/freeglut-devel/
  (deb) sudo apt-get install freeglut3-dev libglu-dev libglew-dev

* FFMPEG (For video decoding and image rescaling)
  (deb) sudo apt-get install ffmpeg libavcodec-dev libavutil-dev libavformat-dev libswscale-dev

* DC1394 (For firewire input)
  (deb) sudo apt-get install libdc1394-22-dev libraw1394-dev

== Very Optional Dependencies ==

* CUDA Toolkit (>= 3.2)
  http://developer.nvidia.com/object/cuda_3_2_downloads.html

* Cg Library (some small Cg utils)
  (deb) sudo apt-get install nvidia-cg-toolkit
