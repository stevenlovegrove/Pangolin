#include <iostream>

#include <GL/glew.h>

#include <pangolin/pangolin.h>

#include <cuda_runtime.h>
#include <cutil_inline.h>
#include <cutil_gl_inline.h>
#include <cutil_gl_error.h>
#include <cuda_gl_interop.h>
#include <vector_types.h>

using namespace pangolin;
using namespace std;

extern "C"
void launch_kernel(float4* pos, unsigned int mesh_width, unsigned int mesh_height, float time);

GLuint vbo = 0;
unsigned int timer = 0;
struct cudaGraphicsResource* vbo_cuda_res;
const int width=640;
const int height=480;

void UpdateVBO()
{
  cudaGraphicsMapResources(1, &vbo_cuda_res, 0);
  size_t num_bytes;
  float4* positions;
  cudaGraphicsResourceGetMappedPointer((void**)&positions, &num_bytes, vbo_cuda_res);
  static float time = 0.0;
  launch_kernel(positions,width,height,time);
  time += 0.002;
  cudaGraphicsUnmapResources(1, &vbo_cuda_res, 0);
}

void CleanUp()
{
  if (vbo) {
    cudaGraphicsUnregisterResource(vbo_cuda_res);
    glBindBuffer(1, vbo);
    glDeleteBuffers(1, &vbo);
  }
  cutilCheckError( cutDeleteTimer( timer));
}

int main( int /*argc*/, char* argv[] )
{
  cudaGLSetGLDevice(cutGetMaxGflopsDeviceId());
  pangolin::CreateGlutWindowAndBind("Main",640,480);
  atexit(&CleanUp);
  glewInit();

  // Create buffer object and register it with CUDA
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  const unsigned int size = width * height * 4 * sizeof(float);
  glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  cudaGraphicsGLRegisterBuffer(
    &vbo_cuda_res, vbo,
    cudaGraphicsMapFlagsWriteDiscard
  );

  // Define Camera Render Object (for view / scene browsing)
  pangolin::OpenGlRenderState s_cam;
  s_cam.Set(ProjectionMatrix(640,480,420,420,320,240,0.1,1000));
  s_cam.Set(IdentityMatrix(GlModelViewStack));

  // Add named OpenGL viewport to window and provide 3D Handler
  View& d_cam = pangolin::Display("cam")
    .SetBounds(1.0, 0.0, 100, 1.0, -640.0f/480.0f)
    .SetHandler(new Handler3D(s_cam));

  // Add named Panal and bind to variables beginning 'ui'
  // A Panal is just a View with a default layout and input handling
  View& d_panal = pangolin::CreatePanal("ui")
      .SetBounds(1.0, 0.0, 0, 100);

  cutCreateTimer(&timer);

  // Default hooks for exiting (Esc) and fullscreen (tab).
  while(!pangolin::ShouldQuit())
  {
    static Var<double> fps("ui.fps");

    cutStartTimer(timer);

    if(HasResized())
      DisplayBase().ActivateScissorAndClear();

    d_cam.ActivateScissorAndClear(s_cam);
    glEnable(GL_DEPTH_TEST);
    glColor3f(1.0,1.0,1.0);

    UpdateVBO();
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexPointer(4, GL_FLOAT, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_POINTS, 0, width * height);
    glDisableClientState(GL_VERTEX_ARRAY);

    // Render our UI panal when we receive input
//    if(HadInput())
      d_panal.Render();

    // Swap frames and Process Events
    glutSwapBuffers();
    glutMainLoopEvent();

    cutStopTimer(timer);
    fps = 1000.0 / cutGetAverageTimerValue(timer);
    cutResetTimer(timer);
  }

  CleanUp();

  return 0;
}
