#include <iostream>

#include <GL/glew.h>

#include <pangolin/glcuda.h>
#include <pangolin/pangolin.h>

#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <vector_types.h>

#ifdef USE_CUTIL
  #include <cutil_inline.h>
#endif // USE_CUTIL

using namespace pangolin;
using namespace std;

// Mesh size
const int mesh_width=256;
const int mesh_height=256;

extern "C" void launch_kernel(float4* dVertexArray, uchar4* dColourArray, unsigned int width, unsigned int height, float time);

int main( int /*argc*/, char* argv[] )
{
#ifdef USE_CUTIL
    cudaGLSetGLDevice(cutGetMaxGflopsDeviceId());
#else
    cudaGLSetGLDevice(0);
#endif

  pangolin::CreateGlutWindowAndBind("Main",640,480);
  glewInit();
  
  // 3D Mouse handler requires depth testing to be enabled  
  glEnable(GL_DEPTH_TEST);  

  // Create vertex and colour buffer objects and register them with CUDA
  GlBufferCudaPtr vertex_array(
      GlArrayBuffer, mesh_width*mesh_height*sizeof(float4),
      cudaGraphicsMapFlagsWriteDiscard, GL_STREAM_DRAW
  );
  GlBufferCudaPtr colour_array(
      GlArrayBuffer, mesh_width*mesh_height*sizeof(uchar4),
      cudaGraphicsMapFlagsWriteDiscard, GL_STREAM_DRAW
  );

  // Define Camera Render Object (for view / scene browsing)
  pangolin::OpenGlRenderState s_cam(
    ProjectionMatrix(640,480,420,420,320,240,0.1,1000),
    ModelViewLookAt(-0,2,-2, 0,0,0, AxisY)
  );
  const int UI_WIDTH = 180;

  // Add named OpenGL viewport to window and provide 3D Handler
  View& d_cam = pangolin::Display("cam")
    .SetBounds(0.0, 1.0, Attach::Pix(UI_WIDTH), 1.0, -640.0f/480.0f)
    .SetHandler(new Handler3D(s_cam));

  // Add named Panel and bind to variables beginning 'ui'
  // A Panel is just a View with a default layout and input handling
  View& d_panel = pangolin::CreatePanel("ui")
      .SetBounds(0.0, 1.0, 0.0, Attach::Pix(UI_WIDTH));

#ifdef USE_CUTIL
  // Apply timer as used by CUDA samples
  // The fps measure they use is actually completely incorrect!
  unsigned int timer = 0;
  cutCreateTimer(&timer);
#endif

  // Default hooks for exiting (Esc) and fullscreen (tab).
  for(int frame=0; !pangolin::ShouldQuit(); ++frame)
  {
    static double time = 0;
    static Var<double> delta("ui.time delta", 0.001, 0, 0.005);

#ifdef USE_CUTIL
    static Var<double> fps("ui.fps");
    cutStartTimer(timer);
#endif

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    d_cam.Activate(s_cam);
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
    if(!(frame%100))
    {
#ifdef USE_CUTIL
      fps = 1000.0 / cutGetAverageTimerValue(timer);
      cutResetTimer(timer);
#endif
    }

    // Swap frames and Process Events
    pangolin::FinishGlutFrame();

#ifdef USE_CUTIL
    cutStopTimer(timer);
#endif
  }

#ifdef USE_CUTIL
  cutilCheckError( cutDeleteTimer( timer));
#endif

  return 0;
}
