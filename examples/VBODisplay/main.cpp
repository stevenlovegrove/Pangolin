#include <iostream>

#include <GL/glew.h>

#include <pangolin/glcuda.h>
#include <pangolin/pangolin.h>

#include <cuda_runtime.h>
#include <cutil_inline.h>
#include <cutil_gl_inline.h>
#include <cutil_gl_error.h>
#include <cuda_gl_interop.h>
#include <vector_types.h>

using namespace pangolin;
using namespace std;

// Mesh size
const int mesh_width=256;
const int mesh_height=256;

extern "C" void launch_kernel(float4* dVertexArray, uchar4* dColourArray, unsigned int width, unsigned int height, float time);

int main( int /*argc*/, char* argv[] )
{
  cudaGLSetGLDevice(cutGetMaxGflopsDeviceId());
  pangolin::CreateGlutWindowAndBind("Main",640,480);
  glewInit();

  // Create vertex and colour buffer objects and register them with CUDA
  GlBufferCudaPtr vertex_array(
      GlArrayBuffer, mesh_width * mesh_height * sizeof(float4),
      cudaGraphicsMapFlagsWriteDiscard, GL_STREAM_DRAW
  );
  GlBufferCudaPtr colour_array(
      GlArrayBuffer, mesh_width * mesh_height * sizeof(uchar4),
      cudaGraphicsMapFlagsWriteDiscard, GL_STREAM_DRAW
  );

  // Define Camera Render Object (for view / scene browsing)
  pangolin::OpenGlRenderState s_cam;
  s_cam.Set(ProjectionMatrix(640,480,420,420,320,240,0.1,1000));
  s_cam.Set(IdentityMatrix(GlModelViewStack));

  // Add named OpenGL viewport to window and provide 3D Handler
  View& d_cam = pangolin::Display("cam")
    .SetBounds(1.0, 0.0, 150, 1.0, -640.0f/480.0f)
    .SetHandler(new Handler3D(s_cam));

  // Add named Panel and bind to variables beginning 'ui'
  // A Panel is just a View with a default layout and input handling
  View& d_panel = pangolin::CreatePanel("ui")
      .SetBounds(1.0, 0.0, 0, 150);

  // Apply timer as used by CUDA samples
  // The fps measure they use is actually completely incorrect!
  unsigned int timer = 0;
  cutCreateTimer(&timer);

  // Default hooks for exiting (Esc) and fullscreen (tab).
  for(int frame=0; !pangolin::ShouldQuit(); ++frame)
  {
    static double time = 0;
    static Var<double> fps("ui.fps");
    static Var<double> delta("ui.time delta", 0.001, 0, 0.005);

    cutStartTimer(timer);

    if(HasResized())
      DisplayBase().ActivateScissorAndClear();

    d_cam.ActivateScissorAndClear(s_cam);
    glEnable(GL_DEPTH_TEST);
    glColor3f(1.0,1.0,1.0);

    {
      CudaScopedMappedPtr var(vertex_array);
      CudaScopedMappedPtr car(colour_array);
      launch_kernel((float4*)*var,(uchar4*)*car,mesh_width,mesh_height,time);
      time += delta;
    }

    vertex_array.Bind();
    glVertexPointer(4, GL_FLOAT, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);

    colour_array.Bind();
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
    glEnableClientState(GL_COLOR_ARRAY);

    glDrawArrays(GL_POINTS, 0, mesh_width * mesh_height);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    // Render our UI panel when we receive input
    if(HadInput() | !(frame%1000))
    {
      fps = 1000.0 / cutGetAverageTimerValue(timer);
      d_panel.Render();
      cutResetTimer(timer);
    }

    // Swap frames and Process Events
    glutSwapBuffers();
    glutMainLoopEvent();

    cutStopTimer(timer);
  }

  cutilCheckError( cutDeleteTimer( timer));

  return 0;
}
