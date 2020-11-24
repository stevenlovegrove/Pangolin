---
layout: default
title: Pangolin
---

What is Pangolin?
====================================
Pangolin is a set of lightweight and portable utility libraries for prototyping 3D, numeric or video based programs and algorithms. It is used quite widely in the field of Computer Vision as a means to remove platform-specific boilerplate and make it easy to visualize data.

The general ethos of Pangolin is to minimize boilerplate and maximize portability and flexibility through simple interfaces and factories over things like windowing and video. It also offers a suite of utilities for interactive debugging, such as 3D manipulation, plotters, tweak variables, and a drop-down Quake-like console for python scripting and live tweaking.

### Main features

* Cross Platform Windowing
  * Build for **Windows**, **Linux**, **OSX** and the **Web** (with [Emscripten](https://emscripten.org/))
  * Support different windowing implementations including off-screen buffers
* Viewport Management and Interaction
  * Simple and performant viewport management
  * Intuitive 3D navigation and handlers
  * Utilities to work with Computer Vision & Robotics camera and coordinate conventions
* Video Input
  * Extensive video input wrappers for ordinary and machine-vision cameras and media formats
  * Flexible filter interface for easily manipulating video channels and formats, etc.
* Tweak Variables
  * There are 101 widgeting libraries and several 'tweak' var libraries - Pangolin offers another implementation with a few pros and cons
  * One-line definitions and extensible types
* Drop-down Console
  * Extensible for different shells, but currently supports Python live console
  * Easy access to introspect tweak variables

## Code ##

Find the latest version on [Github](http://github.com/stevenlovegrove/Pangolin):

```bash
git clone https://github.com/stevenlovegrove/Pangolin.git
```

See [Pangolin Build](build) Instructions for details of how to get up and running

### How Simple Are we Talking?

```
<script src="http://gist-it.appspot.com/github/stevenlovegrove/Pangolin/blob/master/examples/SimpleDisplay/main.cpp"></script>
```

See [Pangolin Examples](examples) for more examples that'll run live in your browser!

