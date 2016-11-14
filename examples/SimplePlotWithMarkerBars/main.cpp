#include <iostream>
#include <chrono>
#include <thread>

#include <pangolin/pangolin.h>

int main(/*int argc, char* argv[]*/)
{
  // Create OpenGL window in single line
  pangolin::CreateWindowAndBind("Main",640,480);


  const float tinc = 0.01f;


  pangolin::DataLog log;
  pangolin::Plotter plotter(&log);

  plotter.ClearSeries();
  plotter.AddSeries("$0", "$1", pangolin::DrawingModeLine, pangolin::Colour::Unspecified(), "sin(t)");
  plotter.AddSeries("$0", "$2", pangolin::DrawingModeLine, pangolin::Colour::Unspecified(), "cos(t)");
  plotter.AddSeries("$0", "$3", pangolin::DrawingModeLine, pangolin::Colour::Unspecified(), "sin(t)+cos(t)");
  plotter.SetAspect(640.0/480.0);
  plotter.SetTicks(M_PI, 0.5);
  pangolin::DisplayBase().AddDisplay(plotter);

  float t = 0;

  // Default hooks for exiting (Esc) and fullscreen (tab).

  int sin_counter = 0;
  int cos_counter = 0;
  while( !pangolin::ShouldQuit() )
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    plotter.SetView(pangolin::XYRangef(t-10*M_PI, t, -1, 1));
    log.Log(t, sin(t),cos(t),sin(t)+cos(t));

    t += tinc;


    if (t > sin_counter*2*M_PI) {
        plotter.AddMarker(pangolin::Marker::Direction::Vertical,
                                sin_counter*2*M_PI, pangolin::Marker::GreaterThan,
                                pangolin::Colour(0.0, 0, 1.0, 0.5),
                                std::unique_ptr<pangolin::Rangef>(new pangolin::Rangef(0, 0.1)),
                                std::unique_ptr<float>(new float(M_PI)));
        ++sin_counter;
    }
    if (t > cos_counter*2*M_PI + M_PI_2) {
        plotter.AddMarker(pangolin::Marker::Direction::Vertical,
                                cos_counter*2*M_PI + M_PI, pangolin::Marker::GreaterThan,
                                pangolin::Colour(1.0, 0, 0.0, 0.5),
                                std::unique_ptr<pangolin::Rangef>(new pangolin::Rangef(0.1, 0.2)),
                                std::unique_ptr<float>(new float(M_PI)));
        ++cos_counter;
    }


    //
    pangolin::FinishFrame();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  return 0;
}
