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

#include <pangolin/display/view.h>
#include <pangolin/handler/handler.h>
#include <pangolin/display/display.h>

#include "pangolin_gl.h"

namespace pangolin
{

void Handler::Keyboard(View& d, unsigned char key, int x, int y, bool pressed)
{
    View* child = d.FindChild(x,y);
    if( child)
    {
        GetCurrentContext()->activeDisplay = child;
        if( child->handler)
            child->handler->Keyboard(*child,key,x,y,pressed);
    }
}

void Handler::Mouse(View& d, MouseButton button, int x, int y, bool pressed, int button_state)
{
    View* child = d.FindChild(x,y);
    if( child )
    {
        GetCurrentContext()->activeDisplay = child;
        if( child->handler)
            child->handler->Mouse(*child,button,x,y,pressed,button_state);
    }
}

void Handler::MouseMotion(View& d, int x, int y, int button_state)
{
    View* child = d.FindChild(x,y);
    if( child )
    {
        GetCurrentContext()->activeDisplay = child;
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
    View* child = d.FindChild( (int)x, (int)y);
    if( child )
    {
        GetCurrentContext()->activeDisplay = child;
        if( child->handler)
            child->handler->Special(*child,inType, x,y, p1, p2, p3, p4, button_state);
    }
}

void HandlerScroll::Mouse(View& d, MouseButton button, int x, int y, bool pressed, int button_state)
{
    if( pressed && (button == MouseWheelUp || button == MouseWheelDown) )
    {
        if( button == MouseWheelUp) d.scroll_offset   -= 1;
        if( button == MouseWheelDown) d.scroll_offset += 1;
        d.scroll_offset = std::max(0, std::min(d.scroll_offset, (int)d.NumVisibleChildren()-1) );
        d.ResizeChildren();
    }else{
        Handler::Mouse(d,button,x,y,pressed,button_state);
    }
}

void HandlerScroll::Special(View& d, InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state)
{
    if( inType == InputSpecialScroll )
    {
        d.scroll_offset -= (int)(p2 / fabs(p2));
        d.scroll_offset = std::max(0, std::min(d.scroll_offset, (int)d.NumVisibleChildren()-1) );
        d.ResizeChildren();
    }else{
        Handler::Special(d,inType,x,y,p1,p2,p3,p4,button_state);
    }
}

HandlerBase3D::HandlerBase3D(OpenGlRenderState& cam_state, AxisDirection enforce_up, float trans_scale, float zoom_fraction)
    : cam_state(&cam_state), enforce_up(enforce_up), tf(trans_scale), zf(zoom_fraction), cameraspec(CameraSpecOpenGl), last_z(0.8)
{
    SetZero<3,1>(rot_center);
}

void HandlerBase3D::Keyboard(View&, unsigned char /*key*/, int /*x*/, int /*y*/, bool /*pressed*/)
{
    // TODO: hooks for reset / changing mode (perspective / ortho etc)
}

bool HandlerBase3D::ValidWinDepth(GLprecision depth)
{
    return depth != 1;
}

void HandlerBase3D::PixelUnproject( View& view, GLprecision winx, GLprecision winy, GLprecision winz, GLprecision Pc[3])
{
    const GLint viewport[4] = {view.v.l,view.v.b,view.v.w,view.v.h};
    const pangolin::OpenGlMatrix proj = cam_state->GetProjectionMatrix();
    glUnProject(winx, winy, winz, IdentityMatrix().m, proj.m, viewport, &Pc[0], &Pc[1], &Pc[2]);
}

void HandlerBase3D::GetPosNormalImpl(pangolin::View& view, int winx, int winy, GLprecision p[3], GLprecision Pw[3], GLprecision Pc[3], GLprecision nw[3], GLprecision default_z, float* zs)
{
    const int zl = (hwin*2+1);
    const int zsize = zl*zl;

    GLfloat mindepth = *(std::min_element(zs,zs+zsize));
    if(mindepth == 1) mindepth = (GLfloat)default_z;

    p[0] = winx; p[1] = winy; p[2] = mindepth;
    PixelUnproject(view, winx, winy, mindepth, Pc);

    const pangolin::OpenGlMatrix mv = cam_state->GetModelViewMatrix();

    GLprecision T_wc[3*4];
    LieSE3from4x4(T_wc, mv.Inverse().m );
    LieApplySE3vec(Pw, T_wc, Pc);

    // Neighboring points in camera coordinates
    GLprecision Pl[3]; GLprecision Pr[3]; GLprecision Pb[3]; GLprecision Pt[3];
    PixelUnproject(view, winx-hwin, winy, zs[hwin*zl + 0],    Pl );
    PixelUnproject(view, winx+hwin, winy, zs[hwin*zl + zl-1], Pr );
    PixelUnproject(view, winx, winy-hwin, zs[hwin+1],         Pb );
    PixelUnproject(view, winx, winy+hwin, zs[zsize-(hwin+1)], Pt );

    // n = ((Pr-Pl).cross(Pt-Pb)).normalized();
    GLprecision PrmPl[3]; GLprecision PtmPb[3];
    MatSub<3,1>(PrmPl,Pr,Pl);
    MatSub<3,1>(PtmPb,Pt,Pb);

    GLprecision nc[3];
    CrossProduct(nc, PrmPl, PtmPb);
    Normalise<3>(nc);

    // T_wc is col major, so the rotation component is first.
    LieApplySO3(nw,T_wc,nc);
}

void HandlerBase3D::GetPosNormal(pangolin::View& view, int winx, int winy, GLprecision p[3], GLprecision Pw[3], GLprecision Pc[3], GLprecision nw[3], GLprecision default_z)
{
    const int zl = (hwin*2+1);
    const int zsize = zl*zl;
    GLfloat zs[zsize];

    GLint readid = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readid);
    // Choose attachment based on the bound framebuffer since GL_FRONT doesn't exist for FBOs
    glReadBuffer(readid == 0 ? GL_FRONT : GL_COLOR_ATTACHMENT0);
    glReadPixels(winx-hwin,winy-hwin,zl,zl,GL_DEPTH_COMPONENT,GL_FLOAT,zs);
    GetPosNormalImpl(view, winx, winy, p, Pw, Pc, nw, default_z, zs);
}

void HandlerBase3D::Mouse(View& display, MouseButton button, int x, int y, bool pressed, int button_state)
{
    // mouse down
    last_pos[0] = (float)x;
    last_pos[1] = (float)y;
    
    GLprecision T_nc[3*4];
    LieSetIdentity(T_nc);

    funcKeyState = 0;
    if( pressed )
    {
        GetPosNormal(display,x,y,p,Pw,Pc,n,last_z);
        if( ValidWinDepth(p[2]) )
        {
            last_z = p[2];
            std::copy(Pc,Pc+3,rot_center);
        }
        
        if( button == MouseWheelUp || button == MouseWheelDown)
        {
            LieSetIdentity(T_nc);
            const GLprecision t[3] = { 0,0,(button==MouseWheelUp?1:-1)*100*tf};
            LieSetTranslation<>(T_nc,t);
            if( !(button_state & MouseButtonRight) && !(rot_center[0]==0 && rot_center[1]==0 && rot_center[2]==0) )
            {
                LieSetTranslation<>(T_nc,rot_center);
                const GLprecision s = (button==MouseWheelUp?-1.0:1.0) * zf;
                MatMul<3,1>(T_nc+(3*3), s);
            }
            OpenGlMatrix& spec = cam_state->GetModelViewMatrix();
            LieMul4x4bySE3<>(spec.m,T_nc,spec.m);
        }

        funcKeyState = button_state;
    }
}

void HandlerBase3D::MouseMotion(View& display, int x, int y, int button_state)
{
    const GLprecision rf = 0.01;
    const float delta[2] = { (float)x - last_pos[0], (float)y - last_pos[1] };
    const float mag = delta[0]*delta[0] + delta[1]*delta[1];

    if((button_state & KeyModifierCtrl) && (button_state & KeyModifierShift))
    {
        GLprecision T_nc[3 * 4];
        LieSetIdentity(T_nc);

        GetPosNormal(display, x, y, p, Pw, Pc, n, last_z);
        if(ValidWinDepth(p[2]))
        {
            last_z = p[2];
            std::copy(Pc, Pc + 3, rot_center);
        }

        funcKeyState = button_state;
    }
    else
    {
        funcKeyState = 0;
    }

    // TODO: convert delta to degrees based of fov
    // TODO: make transformation with respect to cam spec
    if( mag < 50.0f*50.0f )
    {
        OpenGlMatrix& mv = cam_state->GetModelViewMatrix();
        const GLprecision* up = AxisDirectionVector[enforce_up];
        GLprecision T_nc[3*4];
        LieSetIdentity(T_nc);
        bool rotation_changed = false;
        
        if( button_state == MouseButtonMiddle )
        {
            // Middle Drag: Rotate around view

            // Try to correct for different coordinate conventions.
            GLprecision aboutx = -rf * delta[1];
            GLprecision abouty = rf * delta[0];
            OpenGlMatrix& pm = cam_state->GetProjectionMatrix();
            abouty *= -pm.m[2 * 4 + 3];

            Rotation<>(T_nc, aboutx, abouty, (GLprecision)0.0);
        }else if( button_state == MouseButtonLeft )
        {
            // Left Drag: in plane translate
            if( ValidWinDepth(last_z) )
            {
                GLprecision np[3];
                PixelUnproject(display, x, y, last_z, np);
                const GLprecision t[] = { np[0] - rot_center[0], np[1] - rot_center[1], 0};
                LieSetTranslation<>(T_nc,t);
                std::copy(np,np+3,rot_center);
            }else{
                const GLprecision t[] = { -10*delta[0]*tf, 10*delta[1]*tf, 0};
                LieSetTranslation<>(T_nc,t);
            }
        }else if( button_state == (MouseButtonLeft | MouseButtonRight) )
        {
            // Left and Right Drag: in plane rotate about object
            //        Rotation<>(T_nc,0.0,0.0, delta[0]*0.01);
            
            GLprecision T_2c[3*4];
            Rotation<>(T_2c, (GLprecision)0.0, (GLprecision)0.0, delta[0]*rf);
            GLprecision mrotc[3];
            MatMul<3,1>(mrotc, rot_center, (GLprecision)-1.0);
            LieApplySO3<>(T_2c+(3*3),T_2c,mrotc);
            GLprecision T_n2[3*4];
            LieSetIdentity<>(T_n2);
            LieSetTranslation<>(T_n2,rot_center);
            LieMulSE3(T_nc, T_n2, T_2c );
            rotation_changed = true;
        }else if( button_state == MouseButtonRight)
        {
            GLprecision aboutx = -rf * delta[1];
            GLprecision abouty = -rf * delta[0];

            // Try to correct for different coordinate conventions.
            if(cam_state->GetProjectionMatrix().m[2*4+3] <= 0) {
                abouty *= -1;
            }
            
            if(enforce_up) {
                // Special case if view direction is parallel to up vector
                const GLprecision updotz = mv.m[2]*up[0] + mv.m[6]*up[1] + mv.m[10]*up[2];
                if(updotz > 0.98) aboutx = std::min(aboutx, (GLprecision)0.0);
                if(updotz <-0.98) aboutx = std::max(aboutx, (GLprecision)0.0);
                // Module rotation around y so we don't spin too fast!
                abouty *= (1-0.6*fabs(updotz));
            }
            
            // Right Drag: object centric rotation
            GLprecision T_2c[3*4];
            Rotation<>(T_2c, aboutx, abouty, (GLprecision)0.0);
            GLprecision mrotc[3];
            MatMul<3,1>(mrotc, rot_center, (GLprecision)-1.0);
            LieApplySO3<>(T_2c+(3*3),T_2c,mrotc);
            GLprecision T_n2[3*4];
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
    
    last_pos[0] = (float)x;
    last_pos[1] = (float)y;
}

void HandlerBase3D::Special(View& display, InputSpecial inType, float x, float y, float p1, float p2, float /*p3*/, float /*p4*/, int button_state)
{
    if( !(inType == InputSpecialScroll || inType == InputSpecialRotate) )
        return;
    
    // mouse down
    last_pos[0] = x;
    last_pos[1] = y;
    
    GLprecision T_nc[3*4];
    LieSetIdentity(T_nc);
    
    GetPosNormal(display, (int)x, (int)y, p, Pw, Pc, n, last_z);
    if(p[2] < 1.0) {
        last_z = p[2];
        std::copy(Pc,Pc+3,rot_center);
    }
    
    if( inType == InputSpecialScroll ) {
        if(button_state & KeyModifierCmd) {
            const GLprecision rx = -p2 / 1000;
            const GLprecision ry = -p1 / 1000;
            
            Rotation<>(T_nc,rx, ry, (GLprecision)0.0);
            OpenGlMatrix& spec = cam_state->GetModelViewMatrix();
            LieMul4x4bySE3<>(spec.m,T_nc,spec.m);
        }else{
            const GLprecision scrolly = p2/10;
            
            LieSetIdentity(T_nc);
            const GLprecision t[] = { 0,0, -scrolly*100*tf};
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
        const GLprecision r = p1 / 20;
        
        GLprecision T_2c[3*4];
        Rotation<>(T_2c, (GLprecision)0.0, (GLprecision)0.0, r);
        GLprecision mrotc[3];
        MatMul<3,1>(mrotc, rot_center, (GLprecision)-1.0);
        LieApplySO3<>(T_2c+(3*3),T_2c,mrotc);
        GLprecision T_n2[3*4];
        LieSetIdentity<>(T_n2);
        LieSetTranslation<>(T_n2,rot_center);
        LieMulSE3(T_nc, T_n2, T_2c );
        OpenGlMatrix& spec = cam_state->GetModelViewMatrix();
        LieMul4x4bySE3<>(spec.m,T_nc,spec.m);
    }
    
}

#ifdef HAVE_GLES
Handler3DBlitCopy::Handler3DBlitCopy(pangolin::OpenGlRenderState& cam_state, pangolin::AxisDirection enforce_up, float trans_scale)
    : pangolin::HandlerBase3D(cam_state,enforce_up, trans_scale)
{
}

void Handler3DBlitCopy::GetPosNormal(pangolin::View& view, int winx, int winy, GLprecision p[3], GLprecision Pw[3], GLprecision Pc[3], GLprecision n[3], GLprecision default_z)
{
    // Go around the houses so we can get depth data...
    // With GLES3 / WebGL 2.0, we can't read from the depth buffer or download ROI's of GL textures
    // The process below is the simplest I could make work that avoids a large GPU -> CPU download.

    const int w = pangolin::DisplayBase().v.w;
    const int h = pangolin::DisplayBase().v.h;

    GLint num_bits;
    glGetIntegerv(GL_DEPTH_BITS, &num_bits);

    // Make sure our buffers are the right size for the current screen buffer
    if(!depth_blit.IsValid() || depth_blit.width != w || depth_blit.height != h)
    {
        // Reinitialize all buffers
        fb_blit.Reinitialise();
        rgb_blit.Reinitialise(w,h,GL_RGB, true, 0, GL_RGB, GL_UNSIGNED_BYTE);
        depth_blit.Reinitialise(w,h,GL_DEPTH24_STENCIL8, false, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
        fb_blit.AttachColour(rgb_blit);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb_blit.fbid);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depth_blit.tid, 0);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

        fb.Reinitialise();
        rgb.Reinitialise(w,h,GL_R32F, true, 0, GL_RED, GL_FLOAT);
        depth.Reinitialise(w,h,GL_DEPTH24_STENCIL8, false, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
        fb.AttachColour(rgb);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb.fbid);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depth.tid, 0);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    }

    // Copy screens depth buffer to our blit framebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb_blit.fbid);
    glBlitFramebuffer(0,0,w,h, 0,0,w,h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Render the depth buffer to our regular buffer (so depth ends up in color buffer)
    fb.Bind();
    glViewport(0,0,w,h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor4f(1.0,1.0,1.0,1.0);
    depth_blit.RenderToViewport();
    depth_blit.Unbind();

    const int zl = (hwin*2+1);
    const int zsize = zl*zl;
    GLfloat zs[4*zsize];
    GLfloat zsr[zsize];

    // GLES3 only supports ReadPixels with GL_RGBA
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(winx-hwin,winy-hwin,zl,zl,GL_RGBA,GL_FLOAT,zs);
    for(size_t i=0; i < zsize; ++i) zsr[i] = zs[4*i];
    fb.Unbind();

    HandlerBase3D::GetPosNormalImpl(view,winx,winy,p,Pw,Pc,n,default_z,zsr);
}
#endif

}
