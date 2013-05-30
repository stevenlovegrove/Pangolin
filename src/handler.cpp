/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2013 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <pangolin/handler.h>
#include <pangolin/display_internal.h>
#include <pangolin/view.h>

namespace pangolin
{

// Pointer to context defined in display.cpp
extern __thread PangolinGl* context;

void Handler::Keyboard(View& d, unsigned char key, int x, int y, bool pressed)
{
    View* child = d.FindChild(x,y);
    if( child)
    {
        context->activeDisplay = child;
        if( child->handler)
            child->handler->Keyboard(*child,key,x,y,pressed);
    }
}

void Handler::Mouse(View& d, MouseButton button, int x, int y, bool pressed, int button_state)
{
    View* child = d.FindChild(x,y);
    if( child )
    {
        context->activeDisplay = child;
        if( child->handler)
            child->handler->Mouse(*child,button,x,y,pressed,button_state);
    }
}

void Handler::MouseMotion(View& d, int x, int y, int button_state)
{
    View* child = d.FindChild(x,y);
    if( child )
    {
        context->activeDisplay = child;
        if( child->handler)
            child->handler->MouseMotion(*child,x,y,button_state);
    }
}

void Handler::PassiveMouseMotion(View& d, int x, int y, int button_state)
{
    View* child = d.FindChild(x,y);
    if( child )
    {
        if( child->handler)
            child->handler->PassiveMouseMotion(*child,x,y,button_state);
    }
}

void Handler::Special(View& d, InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state)
{
    View* child = d.FindChild(x,y);
    if( child )
    {
        context->activeDisplay = child;
        if( child->handler)
            child->handler->Special(*child,inType, x,y, p1, p2, p3, p4, button_state);
    }
}

void HandlerScroll::Mouse(View& d, MouseButton button, int x, int y, bool pressed, int button_state)
{
    if( button == button_state && (button == MouseWheelUp || button == MouseWheelDown) )
    {
        if( button == MouseWheelUp) d.scroll_offset   -= 1;
        if( button == MouseWheelDown) d.scroll_offset += 1;
        d.scroll_offset = std::max(0, std::min(d.scroll_offset, (int)d.views.size()) );
        d.ResizeChildren();
    }else{
        Handler::Mouse(d,button,x,y,pressed,button_state);
    }
}

void HandlerScroll::Special(View& d, InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state)
{
    if( inType == InputSpecialScroll )
    {
        d.scroll_offset -= p2 / abs(p2);
        d.scroll_offset = std::max(0, std::min(d.scroll_offset, (int)d.views.size()) );
        d.ResizeChildren();
    }else{
        Handler::Special(d,inType,x,y,p1,p2,p3,p4,button_state);
    }
}

Handler3D::Handler3D(OpenGlRenderState& cam_state, AxisDirection enforce_up, float trans_scale, float zoom_fraction)
    : cam_state(&cam_state), enforce_up(enforce_up), tf(trans_scale), zf(zoom_fraction), cameraspec(CameraSpecOpenGl), last_z(1.0)
{
    SetZero<3,1>(rot_center);
}

void Handler3D::Keyboard(View&, unsigned char key, int x, int y, bool pressed)
{
    // TODO: hooks for reset / changing mode (perspective / ortho etc)
}

void Handler3D::GetPosNormal(pangolin::View& view, int x, int y, double p[3], double Pw[3], double Pc[3], double n[3], double default_z)
{
    // TODO: Get to work on android    
#ifndef _ANDROID_    
    const GLint viewport[4] = {view.v.l,view.v.b,view.v.w,view.v.h};
    const pangolin::OpenGlMatrix proj = cam_state->GetProjectionMatrix();
    const pangolin::OpenGlMatrix mv = cam_state->GetModelViewMatrix();
    //      const pangolin::OpenGlMatrix id = IdentityMatrix();
    
    glReadBuffer(GL_FRONT);
    const int zl = (hwin*2+1);
    const int zsize = zl*zl;
    GLfloat zs[zsize];
    glReadPixels(x-hwin,y-hwin,zl,zl,GL_DEPTH_COMPONENT,GL_FLOAT,zs);
    GLfloat mindepth = *(std::min_element(zs,zs+zsize));
    if(mindepth == 1) mindepth = default_z;
    
    p[0] = x; p[1] = y; p[2] = mindepth;
    gluUnProject(x, y, mindepth, mv.m, proj.m, viewport, &Pw[0], &Pw[1], &Pw[2]);
    //      gluUnProject(x, y, mindepth, id.m, proj.m, viewport, &Pc[0], &Pc[1], &Pc[2]);
    LieApplySE34x4vec3(Pc, mv.m, Pw);
    
    double Pl[3]; double Pr[3]; double Pb[3]; double Pt[3];
    gluUnProject(x-hwin, y, zs[hwin*zl + 0],    mv.m, proj.m, viewport, &Pl[0], &Pl[1], &Pl[2]);
    gluUnProject(x+hwin, y, zs[hwin*zl + zl-1], mv.m, proj.m, viewport, &Pr[0], &Pr[1], &Pr[2]);
    gluUnProject(x, y-hwin, zs[hwin+1],         mv.m, proj.m, viewport, &Pb[0], &Pb[1], &Pb[2]);
    gluUnProject(x, y+hwin, zs[zsize-(hwin+1)], mv.m, proj.m, viewport, &Pt[0], &Pt[1], &Pt[2]);
    
    //      n = ((Pr-Pl).cross(Pt-Pb)).normalized();
    double PrmPl[3]; double PtmPb[3];
    MatSub<3,1>(PrmPl,Pr,Pl);
    MatSub<3,1>(PtmPb,Pt,Pb);
    CrossProduct(n, PrmPl, PtmPb);
    Normalise<3>(n);
#endif
}

void Handler3D::Mouse(View& display, MouseButton button, int x, int y, bool pressed, int button_state)
{
    // mouse down
    last_pos[0] = x;
    last_pos[1] = y;
    
    GLdouble T_nc[3*4];
    LieSetIdentity(T_nc);
    
    if( pressed ) {
        GetPosNormal(display,x,y,p,Pw,Pc,n,last_z);
        if(p[2] < 1.0) {
            last_z = p[2];
            std::copy(Pc,Pc+3,rot_center);
        }
        
        if( button == MouseWheelUp || button == MouseWheelDown)
        {
            LieSetIdentity(T_nc);
            const GLdouble t[] = { 0,0,(button==MouseWheelUp?1:-1)*100*tf};
            LieSetTranslation<>(T_nc,t);
            if( !(button_state & MouseButtonRight) && !(rot_center[0]==0 && rot_center[1]==0 && rot_center[2]==0) )
            {
                LieSetTranslation<>(T_nc,rot_center);
                const GLdouble s = (button==MouseWheelUp?-1.0:1.0) * zf;
                MatMul<3,1>(T_nc+(3*3), s);
            }
            OpenGlMatrix& spec = cam_state->GetModelViewMatrix();
            LieMul4x4bySE3<>(spec.m,T_nc,spec.m);
        }
    }
}

void Handler3D::MouseMotion(View& display, int x, int y, int button_state)
{
    const GLdouble rf = 0.01;
    const int delta[2] = {(x-last_pos[0]),(y-last_pos[1])};
    const float mag = delta[0]*delta[0] + delta[1]*delta[1];
    
    // TODO: convert delta to degrees based of fov
    // TODO: make transformation with respect to cam spec
    
    if( mag < 50*50 )
    {
        OpenGlMatrix& mv = cam_state->GetModelViewMatrix();
        const GLdouble* up = AxisDirectionVector[enforce_up];
        GLdouble T_nc[3*4];
        LieSetIdentity(T_nc);
        bool rotation_changed = false;
        
        if( button_state == MouseButtonMiddle )
        {
            // Middle Drag: in plane translate
            Rotation<>(T_nc,-delta[1]*rf, -delta[0]*rf, (GLdouble)0.0);
        }else if( button_state == MouseButtonLeft )
        {
            // Left Drag: in plane translate
            if( last_z != 1 )
            {
                GLdouble np[3];
                display.GetCamCoordinates(*cam_state,x,y,last_z, np[0], np[1], np[2]);
                const GLdouble t[] = { np[0] - rot_center[0], np[1] - rot_center[1], 0};
                LieSetTranslation<>(T_nc,t);
                std::copy(np,np+3,rot_center);
            }else{
                const GLdouble t[] = { -10*delta[0]*tf, 10*delta[1]*tf, 0};
                LieSetTranslation<>(T_nc,t);
            }
        }else if( button_state == (MouseButtonLeft | MouseButtonRight) )
        {
            // Left and Right Drag: in plane rotate about object
            //        Rotation<>(T_nc,0.0,0.0, delta[0]*0.01);
            
            GLdouble T_2c[3*4];
            Rotation<>(T_2c, (GLdouble)0.0, (GLdouble)0.0, delta[0]*rf);
            GLdouble mrotc[3];
            MatMul<3,1>(mrotc, rot_center, (GLdouble)-1.0);
            LieApplySO3<>(T_2c+(3*3),T_2c,mrotc);
            GLdouble T_n2[3*4];
            LieSetIdentity<>(T_n2);
            LieSetTranslation<>(T_n2,rot_center);
            LieMulSE3(T_nc, T_n2, T_2c );
            rotation_changed = true;
        }else if( button_state == MouseButtonRight)
        {
            // Correct for OpenGL Camera.
            GLdouble aboutx = -rf * delta[1];
            GLdouble abouty =  rf * delta[0];
            
            // Try to correct for different coordinate conventions.
            OpenGlMatrix& pm = cam_state->GetProjectionMatrix();
            abouty *= -pm.m[2*4+3];
            
            if(enforce_up) {
                // Special case if view direction is parallel to up vector
                const GLdouble updotz = mv.m[2]*up[0] + mv.m[6]*up[1] + mv.m[10]*up[2];
                if(updotz > 0.98) aboutx = std::min(aboutx, (GLdouble)0.0);
                if(updotz <-0.98) aboutx = std::max(aboutx, (GLdouble)0.0);
                // Module rotation around y so we don't spin too fast!
                abouty *= (1-0.6*abs(updotz));
            }
            
            // Right Drag: object centric rotation
            GLdouble T_2c[3*4];
            Rotation<>(T_2c, aboutx, abouty, (GLdouble)0.0);
            GLdouble mrotc[3];
            MatMul<3,1>(mrotc, rot_center, (GLdouble)-1.0);
            LieApplySO3<>(T_2c+(3*3),T_2c,mrotc);
            GLdouble T_n2[3*4];
            LieSetIdentity<>(T_n2);
            LieSetTranslation<>(T_n2,rot_center);
            LieMulSE3(T_nc, T_n2, T_2c );
            rotation_changed = true;
        }
        
        LieMul4x4bySE3<>(mv.m,T_nc,mv.m);
        
        if(enforce_up != AxisNone && rotation_changed) {
            EnforceUpT_cw(mv.m, up);
        }
    }
    
    last_pos[0] = x;
    last_pos[1] = y;
}

void Handler3D::Special(View& display, InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state)
{
    if( !(inType == InputSpecialScroll || inType == InputSpecialRotate) )
        return;
    
    // mouse down
    last_pos[0] = x;
    last_pos[1] = y;
    
    GLdouble T_nc[3*4];
    LieSetIdentity(T_nc);
    
    GetPosNormal(display,x,y,p,Pw,Pc,n,last_z);
    if(p[2] < 1.0) {
        last_z = p[2];
        std::copy(Pc,Pc+3,rot_center);
    }
    
    if( inType == InputSpecialScroll ) {
        if(button_state & KeyModifierCmd) {
            const GLdouble rx = -p2 / 1000;
            const GLdouble ry = -p1 / 1000;
            
            Rotation<>(T_nc,rx, ry, (GLdouble)0.0);
            OpenGlMatrix& spec = cam_state->GetModelViewMatrix();
            LieMul4x4bySE3<>(spec.m,T_nc,spec.m);
        }else{
            const GLdouble scrolly = p2/10;
            
            LieSetIdentity(T_nc);
            const GLdouble t[] = { 0,0, -scrolly*100*tf};
            LieSetTranslation<>(T_nc,t);
            if( !(button_state & MouseButtonRight) && !(rot_center[0]==0 && rot_center[1]==0 && rot_center[2]==0) )
            {
                LieSetTranslation<>(T_nc,rot_center);
                MatMul<3,1>(T_nc+(3*3), -scrolly * zf);
            }
            OpenGlMatrix& spec = cam_state->GetModelViewMatrix();
            LieMul4x4bySE3<>(spec.m,T_nc,spec.m);
        }
    }else if(inType == InputSpecialRotate) {
        const GLdouble r = p1 / 20;
        
        GLdouble T_2c[3*4];
        Rotation<>(T_2c, (GLdouble)0.0, (GLdouble)0.0, r);
        GLdouble mrotc[3];
        MatMul<3,1>(mrotc, rot_center, (GLdouble)-1.0);
        LieApplySO3<>(T_2c+(3*3),T_2c,mrotc);
        GLdouble T_n2[3*4];
        LieSetIdentity<>(T_n2);
        LieSetTranslation<>(T_n2,rot_center);
        LieMulSE3(T_nc, T_n2, T_2c );
        OpenGlMatrix& spec = cam_state->GetModelViewMatrix();
        LieMul4x4bySE3<>(spec.m,T_nc,spec.m);
    }
    
}

}
