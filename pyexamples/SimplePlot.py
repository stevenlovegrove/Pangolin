import sys
sys.path.append('../build/src')

import math
import pypangolin

def a_callback():
    print("a pressed")

window_a=pypangolin.CreateWindowAndBind("main py_pangolin", 640, 480)
log=pypangolin.DataLog()
log.SetLabels(["sin(t)", "cos(t)", "sin(t)+cos(t)"])

tinc=0.01
plotter=pypangolin.Plotter(log,0,4*math.pi/tinc,-2,2,math.pi/(4*tinc),0.5);
plotter.SetBounds(pypangolin.Attach(0),
                  pypangolin.Attach(1),
                  pypangolin.Attach(0),
                  pypangolin.Attach(1))
plotter.Track("$i")
plotter.AddMarker(pypangolin.Marker.Vertical, -1000, pypangolin.Marker.LessThan, pypangolin.Colour.Blue().WithAlpha(0.2))
plotter.AddMarker(pypangolin.Marker.Horizontal, 100, pypangolin.Marker.GreaterThan, pypangolin.Colour.Red().WithAlpha(0.2))
plotter.AddMarker(pypangolin.Marker.Horizontal,  10, pypangolin.Marker.Equal, pypangolin.Colour.Green().WithAlpha(0.2))

plotter.AddToDisplay(pypangolin.DisplayBase())

t=0;

while not pypangolin.ShouldQuit():
    pypangolin.glClearColorBifferBitOrDepthBufferBit()

    log.Log(math.sin(t), math.cos(t), math.sin(t)+math.cos(t))
    t+=tinc
    
    pypangolin.FinishFrame()
