#!/usr/bin/env python3
import sys
sys.path.append('../build/src')

from OpenGL.GL import *
import pypangolin as pango
import math

def main():
    win = pango.CreateWindowAndBind("main py_pangolin", 640, 480)
    log = pango.DataLog()
    log.SetLabels(["sin(t)", "cos(t)", "sin(t)+cos(t)"])

    t=0;
    tinc=0.01

    plotter = pango.Plotter(log,0,4*math.pi/tinc,-2,2,math.pi/(4*tinc),0.5);
    plotter.Track("$i")
    plotter.AddMarker(pango.Marker.Vertical, -1000, pango.Marker.LessThan, pango.Colour.Blue().WithAlpha(0.2))
    plotter.AddMarker(pango.Marker.Horizontal, 100, pango.Marker.GreaterThan, pango.Colour.Red().WithAlpha(0.2))
    plotter.AddMarker(pango.Marker.Horizontal,  10, pango.Marker.Equal, pango.Colour.Green().WithAlpha(0.2))
    plotter.SetBounds(pango.Attach(0), pango.Attach(1),
                      pango.Attach(0), pango.Attach(1))

    pango.DisplayBase().AddDisplay(plotter)

    while not pango.ShouldQuit():
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        log.Log(math.sin(t), math.cos(t), math.sin(t)+math.cos(t))
        t+=tinc

        pango.FinishFrame()

if __name__ == "__main__":
    main()
