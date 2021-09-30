# Basic OpenGL Tutorial with Pangolin

By popular request, here is a set of basic OpenGL intros for people wanting to get going quickly with Pangolin.

#### gl_intro_triangle

Drawing a triangle is the *HelloWorld* of OpenGL and a good place for us to start. 

`1_gl_intro_classic_triangle.cpp`: In the **classic** version, Pangolin's main use is creating a window and OpenGL context that we can use without platform specific code. [GLFW](https://www.glfw.org/) would be a good alternative if this is your only aim. Notice drawing the triangle takes 3 function calls and there is a matched Enable... / Disable... call that we must take care to pair.

`1_gl_intro_pango_triangle.cpp`: This is the **pango** version where we make use of a simple utilitiy method to accomplish the same as before. Not much to write home about, but you can see the general ethos of Pangolin is to reduce OpenGL boilerplate without getting too fancy.



#### gl_intro_triangle_vbo

`2_gl_intro_classic_triangle_vbo.cpp`: In the previous example, the vertices of the triangle live in your machines main memory. Your poor graphics card is being drip fed the vertices each time the image is redrawn every frame. This VBO (Vertex Buffer Object) example instead copies the vertex data to your GPU's memory once, and then only references it later. Your graphics card then gets to keep it handy (in probably faster memory) without repeated transfers over a shared bus. The **classic** version uses only the regular OpenGL API. You'll notice it is a C API, and we're notably missing C++ idioms like [RAII](https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization) to make sure we don't leak resources such as the VBO buffer.

`2_gl_intro_pango_triangle_vbo.cpp`: The **pango** version gets a bit more compact where we make use of the `pangolin::GlBuffer` wrapper to manage the GL object lifetime.



#### gl_intro_triangle_vbo_shader

`3_gl_intro_classic_triangle_vbo_shader.cpp`: Okay, so I lied - people don't really recommend using the 'fixed pipeline' for rendering stuff in OpenGL anymore - you're meant to use *GLSL shaders*, code that can run on your graphics hardware to more explicitly outline how you want things to appear. Most of what we had before was fine, but `glColor3f` is normally replaced by shader output, plus a bunch of other stuff that we've not introduced anyway, so don't panic. Personally, the **classic** version gives me shudders - just look at all the boilerplate! 

`3_gl_intro_pango_triangle_vbo_shader.cpp`: The **pango** version shows how to use some of its C++ wrappers that you'll hopefully agree improve readability. Pangolin also has a few utilities to help with writing shaders, most notable here for those familiar with GLSL is Pangolins preprocessor which allows you to `#include`, `#expect` and `#define` things, as well as load multiple shaders from one file with the `@start` annotations.



#### gl_intro_viewport

Okay, I've finished trying to outline how much boilerplate is needed with the regular OpenGL API. Here is a simple intro on how to use Pangolin to organize sub-views of the main window. Pangolin will take care of resizing these views as the window is resized which you would otherwise do manually. Notice we've ditched the shaders again because unless you really care about performance, I wont tell people you're using the deprecated pipeline if you dont. It's really your choice - you can use Pangolin or not either way.



#### gl_intro_view_transforms

In this example we've moved from Flatland into 3D! Doing so requires us to get acquinted with a few important transforms that map from different coordinate systems. In the previous examples, our trusty triangle was getting defined on the XY plane within a box from bottom-left (-1.0,-1.0) to top-right (1.0,1.0) which is then mapped to pixels by the specified viewport. This coordinate system is called *normalized device coordinates*, so named because it is independent of final render resolution but represents the final 2D mapping of render primitives. There are some other important coordinate systems in OpenGL (namely clip coordinates), but what really matters is that whatever coordinate system we care about can somehow get converted into one OpenGL cares about. For this, we talk about coordinate transforms! To make like easy, we try to use linear transforms. Since perspective projection isn't linear, we use homogeneous coordinates to cheat. OpenGL's *fixed pipeline* has several built-in capabilities for working with these 4x4 homogeneous transforms, but the modern OpenGL way is to handle all that stuff ourselves. I'd recommend learning a bit more from the excellent resource [songho](https://www.songho.ca/opengl/gl_transform.html), but you can probably pick up a lot as a practitioner from the code.
