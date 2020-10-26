#!/usr/bin/env python3
import sys

sys.path.append("../build/src")

import pypangolin as pango
from OpenGL.GL import *


def a_callback():
    print("a pressed")


def main():
    win = pango.CreateWindowAndBind("pySimpleDisplay", 640, 480)
    glEnable(GL_DEPTH_TEST)

    pm = pango.ProjectionMatrix(640, 480, 420, 420, 320, 240, 0.1, 1000)
    mv = pango.ModelViewLookAt(-0, 0.5, -3, 0, 0, 0, pango.AxisY)
    s_cam = pango.OpenGlRenderState(pm, mv)

    ui_width = 180

    handler = pango.Handler3D(s_cam)
    d_cam = (
        pango.CreateDisplay()
        .SetBounds(
            pango.Attach(0),
            pango.Attach(1),
            pango.Attach.Pix(ui_width),
            pango.Attach(1),
            -640.0 / 480.0,
        )
        .SetHandler(handler)
    )

    pango.CreatePanel("ui").SetBounds(
        pango.Attach(0), pango.Attach(1), pango.Attach(0), pango.Attach.Pix(ui_width)
    )
    var_ui = pango.Var("ui")
    var_ui.a_Button = False
    var_ui.a_double = (0.0, pango.VarMeta(0, 5))
    var_ui.an_int = (2, pango.VarMeta(0, 5))
    var_ui.a_double_log = (3.0, pango.VarMeta(1, 1e4, logscale=True))
    var_ui.a_checkbox = (False, pango.VarMeta(toggle=True))
    var_ui.an_int_no_input = 2
    var_ui.a_str = "sss"

    ctrl = -96
    pango.RegisterKeyPressCallback(ctrl + ord("a"), a_callback)

    while not pango.ShouldQuit():
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        if var_ui.a_checkbox:
            var_ui.an_int = var_ui.a_double

        var_ui.an_int_no_input = var_ui.an_int

        d_cam.Activate(s_cam)
        pango.glDrawColouredCube()
        pango.FinishFrame()


if __name__ == "__main__":
    main()
