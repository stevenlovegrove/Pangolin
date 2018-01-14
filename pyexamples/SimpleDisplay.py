import sys
sys.path.append('../build/src')

import pypangolin

def a_callback():
    print("a pressed")

def main():
    window_a=pypangolin.CreateWindowAndBind("main py_pangolin", 640, 480)
    pypangolin.glEnableDepthTest()
    pm = pypangolin.ProjectionMatrix(640,480,420,420,320,240,0.1,1000);
    mv = pypangolin.ModelViewLookAt(-0, 0.5, -3, 0, 0, 0, pypangolin.AxisY)
    s_cam = pypangolin.OpenGlRenderState(pm, mv)

    ui_width = 180

    handler=pypangolin.Handler3D(s_cam)
    d_cam = pypangolin.CreateDisplay().SetBounds(pypangolin.Attach(0),
                                                 pypangolin.Attach(1),
                                                 pypangolin.Attach.Pix(ui_width),
                                                 pypangolin.Attach(1),
                                                 -640.0/480.0).SetHandler(handler)

    pypangolin.CreatePanel("ui").SetBounds(pypangolin.Attach(0),
                                           pypangolin.Attach(1),
                                           pypangolin.Attach(0),
                                           pypangolin.Attach.Pix(ui_width))
    var_ui=pypangolin.Var("ui")
    var_ui.A_Button=False
    var_ui.B_Button=True
    var_ui.B_Double=1
    var_ui.B_Str="sss"
    

    ctrl=-96
    pypangolin.RegisterKeyPressCallback(ctrl+ord('a'), a_callback)
    
    while not pypangolin.ShouldQuit():
        pypangolin.glClearColorBifferBitOrDepthBufferBit()
        d_cam.Activate(s_cam)
        pypangolin.glColor3f(1,1,1)
        pypangolin.glDrawColouredCube()   
        pypangolin.FinishFrame()


if __name__ == "__main__":
    main()
