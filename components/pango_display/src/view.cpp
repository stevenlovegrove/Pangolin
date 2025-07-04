/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
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

#include <stdexcept>
#include <pangolin/display/display.h>
#include <pangolin/display/view.h>
#include <pangolin/gl/gl.h>
#include <pangolin/gl/viewport.h>
#include <pangolin/gl/opengl_render_state.h>
#include <pangolin/platform.h>

#include "pangolin_gl.h"

namespace pangolin
{

const int panal_v_margin = 6;

int AttachAbs( int low, int high, Attach a)
{
    if( a.unit == Pixel ) return low + (int)a.p;
    if( a.unit == ReversePixel ) return high - (int)a.p;
    return (int)(low + a.p * (high - low));
}

double AspectAreaWithinTarget(double target, double test)
{
    if( test < target )
        return test / target;
    else
        return target / test;
}

void View::Resize(const Viewport& p)
{
    // Compute Bounds based on specification
    v.l = AttachAbs(p.l,p.r(),left);
    v.b = AttachAbs(p.b,p.t(),bottom);
    int r = AttachAbs(p.l,p.r(),right);
    int t = AttachAbs(p.b,p.t(),top);

    // Make sure left and right, top and bottom are correct order
    if( t < v.b ) std::swap(t,v.b);
    if( r < v.l ) std::swap(r,v.l);

    v.w = r - v.l;
    v.h = t - v.b;

    vp = v;

    // Adjust based on aspect requirements
    if( aspect != 0 )
    {
        const float current_aspect = (float)v.w / (float)v.h;
        if( aspect > 0 )
        {
            // Fit to space
            if( current_aspect < aspect )
            {
                //Adjust height
                const int nh = (int)(v.w / aspect);
                v.b += vlock == LockBottom ? 0 : (vlock == LockCenter ? (v.h-nh)/2 : (v.h-nh) );
                v.h = nh;
            }else if( current_aspect > aspect )
            {
                //Adjust width
                const int nw = (int)(v.h * aspect);
                v.l += hlock == LockLeft? 0 : (hlock == LockCenter ? (v.w-nw)/2 : (v.w-nw) );
                v.w = nw;
            }
        }else{
            // Overfit
            double true_aspect = -aspect;
            if( current_aspect < true_aspect )
            {
                //Adjust width
                const int nw = (int)(v.h * true_aspect);
                v.l += hlock == LockLeft? 0 : (hlock == LockCenter ? (v.w-nw)/2 : (v.w-nw) );
                v.w = nw;
            }else if( current_aspect > true_aspect )
            {
                //Adjust height
                const int nh = (int)(v.w / true_aspect);
                v.b += vlock == LockBottom ? 0 : (vlock == LockCenter ? (v.h-nh)/2 : (v.h-nh) );
                v.h = nh;
            }
        }
    }

    ResizeChildren();
}

inline int zcompare(const View* lhs, const View* rhs)
{
    return lhs->zorder < rhs->zorder;
}

void View::ResizeChildren()
{
    if( layout == LayoutOverlay )
    {
        // Sort children into z-order
        std::sort(views.begin(), views.end(), zcompare);

        for(std::vector<View*>::iterator iv = views.begin(); iv != views.end(); ++iv ) {
            (*iv)->Resize(v);
        }
    }else if( layout == LayoutVertical )
    {
        // Allocate space incrementally
        Viewport space = v.Inset(panal_v_margin);
        int num_children = 0;
        for(std::vector<View*>::iterator iv = views.begin(); iv != views.end(); ++iv )
        {
            if (!(*iv)->show) continue;
            num_children++;
            if(scroll_offset >= num_children ) {
                (*iv)->scroll_show = false;
            }else{
                (*iv)->scroll_show = true;
                (*iv)->Resize(space);
                space.h = (*iv)->v.b - panal_v_margin - space.b;
            }
        }
    }else if(layout == LayoutHorizontal )
    {
        // Allocate space incrementally
        const int margin = 8;
        Viewport space = v.Inset(margin);
        for(std::vector<View*>::iterator iv = views.begin(); iv != views.end(); ++iv )
        {
            (*iv)->Resize(space);
            space.w = (*iv)->v.l + margin + space.l;
        }
    }else if(layout == LayoutEqualVertical )
    {
        // Allocate vertical space equally
        const size_t visiblechildren = NumVisibleChildren();
        const float height = (float)v.h / (float)visiblechildren;

        for( size_t i=0; i < visiblechildren; ++i) {
            Viewport space(v.l, (GLint)(v.b+(visiblechildren-1-i)*height), v.w, (GLint)(height) );
            VisibleChild(i).Resize(space);
        }
    }else if(layout == LayoutEqualHorizontal )
    {
        // Allocate vertical space equally
        const size_t visiblechildren = NumVisibleChildren();
        const float width = (float)v.w / (float)visiblechildren;

        for( size_t i=0; i < visiblechildren; ++i) {
            Viewport space( (GLint)(v.l+i*width), v.b, (GLint)width, v.h);
            VisibleChild(i).Resize(space);
        }
    }else if(layout == LayoutEqual )
    {
        const size_t visiblechildren = NumVisibleChildren();
        // TODO: Make this neater, and make fewer assumptions!
        if( visiblechildren > 0 )
        {
            // This containers aspect
            const double this_a = std::fabs(v.aspect());

            // Use first child with fixed aspect for all children
            double child_a = std::fabs(VisibleChild(0).aspect);
            for(size_t i=1; (child_a==0) && i < visiblechildren; ++i ) {
                child_a = std::fabs(VisibleChild(i).aspect);
            }

            if(child_a == 0) {
                child_a = 1;
            }

            double a = visiblechildren*child_a;
            double area = AspectAreaWithinTarget(this_a, a);

            size_t cols = visiblechildren-1;
            for(; cols > 0; --cols)
            {
                const size_t rows = visiblechildren / cols + (visiblechildren % cols == 0 ? 0 : 1);
                const double na = cols * child_a / rows;
                const double new_area = visiblechildren*AspectAreaWithinTarget(this_a,na)/(rows*cols);
                if( new_area <= area )
                    break;
                area = new_area;
                a = na;
            }

            cols++;
            const size_t rows = visiblechildren / cols + (visiblechildren % cols == 0 ? 0 : 1);
            size_t cw, ch;
            if( a > this_a )
            {
                cw = v.w / cols;
                ch = (int)(cw / child_a); //v.h / rows;
            }else{
                ch = v.h / rows;
                cw = (int)(ch * child_a);
            }

            for(size_t i=0; i< visiblechildren; ++i )
            {
                size_t c = i % cols;
                size_t r = i / cols;
                Viewport space( GLint(v.l + c*cw), GLint(v.t() - (r+1)*ch), GLint(cw), GLint(ch) );
                VisibleChild(i).Resize(space);
            }
        }
    }

}

void View::Render()
{
    if(extern_draw_function && show && scroll_show) {
        extern_draw_function(*this);
    }
    RenderChildren();
}

void View::RenderChildren()
{
    for(std::vector<View*>::iterator iv = views.begin(); iv != views.end(); ++iv )
    {
        if((*iv)->show && (*iv)->scroll_show) (*iv)->Render();
    }
}

void View::Activate() const
{
    v.Activate();
}

void View::ActivateAndScissor() const
{
    vp.Scissor();
    v.Activate();
}

void View::ActivateScissorAndClear() const
{
    vp.Scissor();
    v.Activate();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void View::Activate(const OpenGlRenderState& state ) const
{
    v.Activate();
    state.Apply();
}

void View::ActivateAndScissor(const OpenGlRenderState& state) const
{
    vp.Scissor();
    v.Activate();
    state.Apply();
}

void View::ActivateScissorAndClear(const OpenGlRenderState& state ) const
{
    vp.Scissor();
    v.Activate();
    state.Apply();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void View::ActivatePixelOrthographic() const
{
    v.ActivatePixelOrthographic();
}

void View::ActivateIdentity() const
{
    v.ActivateIdentity();
}

GLfloat View::GetClosestDepth([[maybe_unused]] int x, [[maybe_unused]] int y, int radius) const
{
    // TODO: Get to work on android

    const int zl = (radius*2+1);

    std::vector<GLfloat> zs(zl*zl);

#ifndef HAVE_GLES
    glReadBuffer(GL_FRONT);
    glReadPixels(x-radius,y-radius,zl,zl,GL_DEPTH_COMPONENT,GL_FLOAT,zs.data());
#else
    std::fill(zs.begin(), zs.end(), 0.8);
#endif

    const GLfloat mindepth = *(std::min_element(zs.begin(),zs.end()));
    return mindepth;
}

void View::GetObjectCoordinates(const OpenGlRenderState& cam_state, double winx, double winy, double winzdepth, GLdouble& x, GLdouble& y, GLdouble& z) const
{
    const GLint viewport[4] = {v.l,v.b,v.w,v.h};
    const OpenGlMatrix proj = cam_state.GetProjectionMatrix();
    const OpenGlMatrix mv = cam_state.GetModelViewMatrix();
    glUnProject(winx, winy, winzdepth, mv.m, proj.m, viewport, &x, &y, &z);
}

void View::GetCamCoordinates(const OpenGlRenderState& cam_state, double winx, double winy, double winzdepth, GLdouble& x, GLdouble& y, GLdouble& z) const
{
    const GLint viewport[4] = {v.l, v.b, v.w, v.h};
    const OpenGlMatrix proj = cam_state.GetProjectionMatrix();
#ifndef HAVE_GLES
    glUnProject(winx, winy, winzdepth, Identity4d, proj.m, viewport, &x, &y, &z);
#else
    glUnProject(winx, winy, winzdepth, Identity4f, proj.m, viewport, &x, &y, &z);
#endif
}

View& View::SetFocus()
{
    GetCurrentContext()->activeDisplay = this;
    return *this;
}

bool View::HasFocus() const
{
    return GetCurrentContext()->activeDisplay == this;
}

View& View::SetBounds(Attach bottom, Attach top, Attach left, Attach right)
{
    this->left = left;
    this->top = top;
    this->right = right;
    this->bottom = bottom;
    GetCurrentContext()->base.ResizeChildren();
    return *this;
}

View& View::SetBounds(Attach bottom, Attach top,  Attach left, Attach right, bool keep_aspect)
{
    aspect = keep_aspect ? v.aspect() : 0;
    SetBounds(top,bottom,left,right);
    return *this;
}

View& View::SetBounds(Attach bottom, Attach top,  Attach left, Attach right, double aspect)
{
    this->aspect = aspect;
    SetBounds(top,bottom,left,right);
    return *this;
}

View& View::SetAspect(double aspect)
{
    this->aspect = aspect;
    GetCurrentContext()->base.ResizeChildren();
    return *this;
}

View& View::SetLock(Lock horizontal, Lock vertical )
{
    vlock = vertical;
    hlock = horizontal;
    return *this;
}

View& View::SetLayout(Layout l)
{
    layout = l;
    return *this;
}


View& View::AddDisplay(View& child)
{
    // detach child from any other view, and add to this
    std::vector<View*>::iterator f = std::find(
                GetCurrentContext()->base.views.begin(), GetCurrentContext()->base.views.end(), &child
                );

    if( f != GetCurrentContext()->base.views.end() )
        GetCurrentContext()->base.views.erase(f);

    views.push_back(&child);
    GetCurrentContext()->base.ResizeChildren();
    return *this;
}

View& View::Show(bool show)
{
    this->show = show;
    GetCurrentContext()->base.ResizeChildren();
    return *this;
}

void View::ToggleShow()
{
    Show(!show);
}

bool View::IsShown() const
{
    return show;
}

Viewport View::GetBounds() const
{
    return Viewport( std::max(v.l, vp.l), std::max(v.b, vp.b), std::min(v.w, vp.w), std::min(v.h, vp.h) );
}

void View::SaveOnRender(const std::string& filename_hint)
{
    SaveWindowOnRender(filename_hint, this->v.Intersect(this->vp));
}

void View::SaveRenderNow(const std::string& filename_hint)
{
    SaveWindowNow(filename_hint, this->v);
}

View& View::operator[](size_t i)
{
    return *views[i];
}

size_t View::NumChildren() const
{
    return views.size();
}

size_t View::NumVisibleChildren() const
{
    int numvis = 0;
    for(std::vector<View*>::const_iterator i=views.begin(); i!=views.end(); ++i)
    {
        if((*i)->show) {
            numvis++;
        }
    }
    return numvis;
}

View& View::VisibleChild(size_t i)
{
    size_t numvis = 0;
    for(size_t v=0; v < views.size(); ++v ) {
        if(views[v]->show) {
            if( i == numvis ) {
                return *views[v];
            }
            numvis++;
        }
    }
    // Shouldn't get here
    throw std::out_of_range("No such child.");
}

View* View::FindChild(int x, int y)
{
    // Find in reverse order to mirror draw order
    for( std::vector<View*>::const_reverse_iterator i = views.rbegin(); i != views.rend(); ++i )
        if( (*i)->show && (*i)->GetBounds().Contains(x,y) )
            return (*i);
    return 0;
}

View& View::SetHandler(Handler* h)
{
    handler = h;
    return *this;
}

View& View::SetDrawFunction(const std::function<void(View&)>& drawFunc)
{
    extern_draw_function = drawFunc;
    return *this;
}

}
