What is Pangolin
====================================

Pangolin is a set of lightweight and portable utility libraries for prototyping 3D, numeric or video based programs and algorithms. It is used quite widely in the field of Computer Vision as a means to remove platform-specific boilerplate and make it easy to visualize data.

The general ethos of Pangolin is to minimize boilerplate and maximize portability and flexibility through simple interfaces and factories over things like windowing and video. It also offers a suite of utilities for interactive debugging, such as 3D manipulation, plotters, tweak variables, and a drop-down Quake-like console for python scripting and live tweaking.



## Main features

* Cross Platform Windowing
  * Build for **Windows**, **Linux**, **OSX** and the **Web** (with [Emscripten](https://emscripten.org/))
  * Support different windowing implementations including off-screen buffers
* Viewport Management and Interaction
  * Simple and performant viewport management
  * Intuitive 3D navigation and handlers
  * Utilities to work with Computer Vision & Robotics camera and coordinate conventions
* Video Input and Output
  * Extensive video input/output wrappers for ordinary and machine-vision cameras and media formats
  * Flexible filter interface for easily post-processing video channels and formats, etc.
* Tweak Variables
  * There are 101 widgeting libraries and several 'tweak' var libraries - Pangolin offers another implementation with a few pros and cons
  * One-line definitions and extensible types
* Drop-down Console
  * Extensible for different shells, but currently supports Python live console
  * Easy access to introspect tweak variables



## Code ##

Find the latest version on [Github](http://github.com/stevenlovegrove/Pangolin):

```bash
# Clone Pangolin along with it's submodules
git clone --recursive https://github.com/stevenlovegrove/Pangolin.git
```



## Dependencies ##

*Pangolin* is split into a few *components* so you can include just what you need. Most dependencies are *optional* so you can pick and mix for your needs. Rather than enforcing a particular package manager, you can use a simple [script](https://github.com/stevenlovegrove/Pangolin/blob/master/scripts/install_prerequisites.sh) to generate a list of (**required**, **recommended** or **all**) packages for installation for that manager (e.g. apt, port, brew, dnf, vcpkg):

```bash
# See what package manager and packages are recommended
./scripts/install_prerequisites.sh --dry-run recommended

# Override the package manager choice and install all packages
./scripts/install_prerequisites.sh -m brew all
```

You'll see the dependencies are generally

* Build system (cmake, **required**)
* Windowing system (X11, Cocoa, Win, Web, ...)
* Rendering (OpenGL, GLES)
* Video and Image loading (DC1394, ffmpeg, jpeg, png, ...)
* Wrappers / Cross Compilers (Python3, pybind, Emscripten, ...)

Pangolin does it's best to build something with what it gets, so dependencies which are not found will be silently ignored. If you need a particular feature, check the output of the `cmake ..` stage and look for the *Found and Enabled* lines.



## Building ##

Pangolin uses the CMake portable pre-build tool. To checkout and build pangolin in the
directory 'build', execute the following at a shell (or the equivelent using a GUI):

```bash
# Get Pangolin
cd ~/your_fav_code_directory
git clone --recursive https://github.com/stevenlovegrove/Pangolin.git
cd Pangolin

# Install dependencies (as described above, or your preferred method)
./scripts/install_prerequisites.sh recommended

# Configure and build
cmake -B build
cmake --build build

# with Ninja for faster builds (sudo apt install ninja-build)
cmake -B build -GNinja
cmake --build build

# GIVEME THE PYTHON STUFF!!!! (Check the output to verify selected python version)
cmake --build build -t pypangolin_pip_install

# Run me some tests! (Requires Catch2 which must be manually installed on Ubuntu.)
ctest
```

![Master Build](https://github.com/stevenlovegrove/pangolin/actions/workflows/build.yml/badge.svg?branch=master) p.s. The *master* branch is a development branch. Choose a [stable tag](https://github.com/stevenlovegrove/Pangolin/tags) if you prefer.



#### On Windows

 I'd recommend building natively with the [Build Tools for Visual Studio 2019](https://visualstudio.microsoft.com/downloads/) toolchain (and not mingw or WSL etc which is unsupported). I recommend [gitbash](https://git-scm.com/downloads) for executing the bash snippets on this page and cloning Pangolin. You can work from within their provided console or the fancy new [Windows Terminal](https://devblogs.microsoft.com/commandline/introducing-windows-terminal/) which is a huge improvement to the developer experience on Windows.

#### With Python

You have to be careful about what python version Pangolin has found and is attempting to link against. It will tell you during the `cmake ..` step and you can change it by explicitly telling it the python executable with `cmake -DPYTHON_EXECUTABLE=/path/to/python ..`or ``cmake -DPYTHON_EXECUTABLE=`which python3` `` to use the python accessed through the `python3` alias.

If python is found, the pypangolin module will be built with the default `all` target. A Python wheel can be built manually using the `pypangolin_wheel` target, and the wheel can be installed / uninstalled with `pypangolin_pip_install` and `pypangolin_pip_uninstall` targets.

**NOTE** The python wheel and install targets are only currently working on MacOS and Linux. On Windows, you're out of luck right now. Help appreciated!

#### On the Web

See [**Emscripten (Compile for web)**](#emscripten-compile-for-web) below.


## Examples

Pangolin is mostly documented through it's simple examples which you can find in the [*examples*](https://github.com/stevenlovegrove/Pangolin/tree/master/examples) folder and [*examples/PythonExamples*](https://github.com/stevenlovegrove/Pangolin/tree/master/examples/PythonExamples) for python versions.

Browse some [**online**](https://stevenlovegrove.github.io/Pangolin/examples) and even run them in your browser, such as [**SimplePlot**](https://stevenlovegrove.github.io/Pangolin/emscripten/examples/SimplePlot), [**SimpleDisplay**](https://stevenlovegrove.github.io/Pangolin/emscripten/examples/SimpleDisplay/) and [**SimpleMultiDisplay**](https://stevenlovegrove.github.io/Pangolin/emscripten/examples/SimpleMultiDisplay/).

## Issues ##

Please visit [Github Issues](https://github.com/stevenlovegrove/Pangolin/issues) to view and report problems with Pangolin. Issues and pull requests should be raised against the master branch which contains the current development version.

Please note; most Pangolin dependencies are optional - to disable a dependency which may be causing trouble on your machine, set the BUILD_PANGOLIN_(option) variable to false with a cmake configuration tool (e.g. ccmake or cmake-gui).

#### Common Runtime Problems

<u>Framebuffer with requested attributes not available</u>: You're building against an old or unaccelerated OpenGL version. Either your graphics drivers are not correctly installed or you are on an unusual platform (such as within Docker, in a VM, working over an X forwarding SSH session) which has limited graphics acceleration. For the former case, you need to make sure your system is using appropriate drivers (you can test glxgears / glxinfo on linux for instance). There isn't much you can do about the latter except changing your setup (e.g. finding a VM which can better accelerate your graphics, or perhaps using the [nvidia-docker](https://github.com/NVIDIA/nvidia-docker) wrapper around docker to correctly pass through the graphics driver)

#### Common Python Problems

<u>error: unknown target 'pypangolin_wheels'</u> : cmake didn't find your python. Go back and adjust your cmake variables (such as the tip above) until you see something like "Selected Python: '/opt/local/bin/python3.9'" in the output.

<u>ModuleNotFoundError: No module named 'pypangolin'</u>: Did you install the wheel (see the bash comment under build)? Are you running using the same Python as Pangolin found during the `cmake ..` step?



## Contributions and Continuous Integration ##

For CI, Pangolin uses [Github Actions](https://github.com/stevenlovegrove/Pangolin/actions) for Windows, MacOS, Linux and Emscripten (Web).

To contribute to Pangolin, I would appreciate pull requests against the master branch. If you raise an issue, please include your environment (compiler, operating system, etc).



## Extensibility & Factories

Pangolin uses an extensible factory mechanism for modularising video drivers, windowing backends and console interpreters. Concrete instances are instantiated from a particular factory using a URI string which identifies which factory to use and what parameters it should use. As strings, URI's are a useful mechanism for providing and validating configuration from an end user. The URI form is:
`module_name:[option1=value1,option2=value2,...]//module_resource_to_open`.

#### Video URI's

The *VideoViewer* tool takes these URI's directly in order to specify what images or video to load and show:

```bash
VideoViewer test://
VideoViewer uvc:[size=640x480]///dev/video0
VideoViewer flip://debayer:[tile=rggb,method=downsample]//file://~/somefile.pango
```

Notice that for video, some modules support chaining to construct a simple filter graph.

#### Window URI's

Windowing in Pangolin is also backed by a factory. When an application calls CreateWindowAndBind() or similar, the default:// URI is specified. This can be overriden to use different windowing options by setting the`PANGOLIN_WINDOW_URI` environment variable. e.g.


```bash
# Use the X11 Window Driver and override the default double buffered option
PANGOLIN_WINDOW_URI="x11:[double_buffered=false]//" ./some_pangolin_app

# Create a 'virtual' window using a framebuffer
PANGOLIN_WINDOW_URI="headless://" ./another_pangolin_app
```

Some window parameters that may be interesting to override are `DISPLAYNAME`, `DOUBLEBUFFER`, `SAMPLE_BUFFERS`, `SAMPLES`, `HIGHRES`. Window modules currently include `x11`, `winapi`, `cocoa`.

#### Fonts

Additionally, there are a couple of special parameters that can be passed through the `PANGOLIN_WINDOW_URI` environment variable to change the default font or adjust the default font size:

```bash
PANGOLIN_WINDOW_URI="default:[default_font_size=20]//" ./some_pangolin_app

PANGOLIN_WINDOW_URI="default:[default_font=my_awesome_font.ttf,default_font_size=20]//" ./some_pangolin_app
```

To use Pangolin in your applications whilst being conciencious of chaning fonts, you can query how long fonts or text are with:

```C++
#include <pangolin/display/default_font.h>
int func()
{
    // E.g. Choose sensible size for Pangolin Panel
    const int PANEL_WIDTH = 20* pangolin::default_font().MaxWidth();
    ...
    // E.g. Work out some column width etc.
    const int COL_WIDTH = pangolin::default_font().Text("How big is this text?").Width();
}
```



## Emscripten (Compile for web) ##

Emscripten is a neat c++ compiler which can output javascript executable code. That's right, your Pangolin programs can run on the web, too!

Follow Emscriptens instructions to install the SDK (summerized below):

```bash
mkdir ~/tools && cd ~/tools
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest
```

To build Pangolin with the Emscripten toolchain, create a new build directory, make sure Emscripten is on your PATH, and use the `emconfigure` utility to run CMake with the custom toolchain settings:

```bash
cd ~/code/Pangolin
mkdir build-em && cd build-em
source ~/tools/emsdk/emsdk_env.sh
emcmake cmake ..
```

## Acknowledgements ##

I'd like to thank the growing number of kind contributors to Pangolin for helping to make it more stable and feature rich. Many features of Pangolin have been influenced by other projects such as GFlags, GLConsole, and libcvd in particular. I'd also like to thank the FOSS projects on which Pangolin depends.

For a summary of those who have made code contributions, execute:

```bash
git shortlog -sne
```
