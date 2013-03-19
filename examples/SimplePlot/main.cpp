#include <iostream>
#include <boost/thread.hpp>
#include <pangolin/pangolin.h>

using namespace pangolin;
using namespace std;

#include <pangolin/display_internal.h>

int main( int /*argc*/, char* argv[] )
{
  // Create OpenGL window in single line thanks to GLUT
  pangolin::CreateGlutWindowAndBind("Main",640,480);

  // Data logger object
  DataLog log;
  const double tinc = 0.01;

  // OpenGL 'view' of data. We might have many views of the same data.
  Plotter plotter(&log,0,M_PI/tinc,-2,2,M_PI/(4*tinc),0.5);
  plotter.SetBounds(0.0, 1.0, 0.0, 1.0);
  DisplayBase().AddDisplay(plotter);

  double t = 0;

  // Optionally add named labels
  vector<std::string> labels;
  labels.push_back(std::string("sin(t)"));
  labels.push_back(std::string("cos(t)"));
  labels.push_back(std::string("tan(t)"));
  labels.push_back(std::string("sin(t)+cos(t)"));
  log.SetLabels(labels);

  // Default hooks for exiting (Esc) and fullscreen (tab).
  while( !pangolin::ShouldQuit() )
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    log.Log(sin(t),cos(t),tan(t),sin(t)+cos(t));
    t += tinc;

    // Render graph, Swap frames and Process Events
    pangolin::FinishGlutFrame();

    boost::this_thread::sleep(boost::posix_time::milliseconds(10));
  }

  return 0;
}
