#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include <pangolin/platform.h>
#include <pangolin/display.h>
#include <pangolin/display_internal.h>
#include <pangolin/view.h>
#include <pangolin/viewport.h>
//#include <pangolin/gl.h>
#include <pangolin/glinclude.h>

#ifdef HAVE_BOOST_GIL
    #include <boost/gil/gil_all.hpp>
    #ifdef HAVE_PNG
    #define png_infopp_NULL (png_infopp)NULL
    #define int_p_NULL (int*)NULL
    #include <boost/gil/extension/io/png_io.hpp>
    #endif // HAVE_PNG
    #ifdef HAVE_JPEG
    #include <boost/gil/extension/io/jpeg_io.hpp>
    #endif // HAVE_JPEG
    #ifdef HAVE_TIFF
    #include <boost/gil/extension/io/tiff_io.hpp>
    #endif // HAVE_TIFF
#endif // HAVE_BOOST_GIL

namespace pangolin
{

// Pointer to context defined in display.cpp
extern __thread PangolinGl* context;

const int panal_v_margin = 6;

int AttachAbs( int low, int high, Attach a)
{
    if( a.unit == Pixel ) return low + a.p;
    if( a.unit == ReversePixel ) return high - a.p;
    return low + a.p * (high - low);
}

float AspectAreaWithinTarget(double target, double test)
{
    if( test < target )
        return test / target;
    else
        return target / test;
}

void SaveViewFromFbo(std::string prefix, View& view, float scale)
{
#ifndef _ANDROID_
    const Viewport orig = view.v;
    view.v.l = 0;
    view.v.b = 0;
    view.v.w *= scale;
    view.v.h *= scale;
    
    const int w = view.v.w;
    const int h = view.v.h;
    
    float origLineWidth;
    glGetFloatv(GL_LINE_WIDTH, &origLineWidth);
    glLineWidth(origLineWidth * scale);
    
    float origPointSize;
    glGetFloatv(GL_POINT_SIZE, &origPointSize);
    glPointSize(origPointSize * scale);
    
    // Create FBO
    GlTexture color(w,h);
    GlRenderBuffer depth(w,h);
    GlFramebuffer fbo(color, depth);
    
    // Render into FBO
    fbo.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    view.Render();
    glFlush();
    
#ifdef HAVE_BOOST_GIL
    // Read buffer and write file
    boost::gil::rgba8_image_t img(w, h);
    glReadPixels(0,0,w,h, GL_RGBA, GL_UNSIGNED_BYTE, boost::gil::interleaved_view_get_raw_data( boost::gil::view( img ) ) );
#ifdef HAVE_PNG
    boost::gil::png_write_view(prefix + ".png", flipped_up_down_view( boost::gil::const_view(img)) );
#endif // HAVE_PNG
#endif // HAVE_BOOST_GIL
    
    // unbind FBO
    fbo.Unbind();
    
    // restore viewport / line width
    view.v = orig;
    glLineWidth(origLineWidth);
    glPointSize(origPointSize); 
#endif // _ANDROID_
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

void View::ResizeChildren()
{
    if( layout == LayoutOverlay )
    {
        foreach(View* i, views)
            i->Resize(v);
    }else if( layout == LayoutVertical )
    {
        // Allocate space incrementally
        Viewport space = v.Inset(panal_v_margin);
        int num_children = 0;
        foreach(View* i, views )
        {
            num_children++;
            if(scroll_offset > num_children ) {
                i->show = false;
            }else{
                i->show = true;
                i->Resize(space);
                space.h = i->v.b - panal_v_margin - space.b;
            }
        }
    }else if(layout == LayoutHorizontal )
    {
        // Allocate space incrementally
        const int margin = 8;
        Viewport space = v.Inset(margin);
        foreach(View* i, views )
        {
            i->Resize(space);
            space.w = i->v.l + margin + space.l;
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
                std::cerr << "LayoutEqual requires that each child has same aspect, but no child with fixed aspect found. Using 1:1." << std::endl;
                child_a = 1;
            }
            
            double a = visiblechildren*child_a;
            double area = AspectAreaWithinTarget(this_a, a);
            
            int cols = visiblechildren-1;
            for(; cols > 0; --cols)
            {
                const int rows = visiblechildren / cols + (visiblechildren % cols == 0 ? 0 : 1);
                const double na = cols * child_a / rows;
                const double new_area = visiblechildren*AspectAreaWithinTarget(this_a,na)/(rows*cols);
                if( new_area <= area )
                    break;
                area = new_area;
                a = na;
            }
            
            cols++;
            const int rows = visiblechildren / cols + (visiblechildren % cols == 0 ? 0 : 1);
            int cw,ch;
            if( a > this_a )
            {
                cw = v.w / cols;
                ch = cw / child_a; //v.h / rows;
            }else{
                ch = v.h / rows;
                cw = ch * child_a;
            }
            
            for( unsigned int i=0; i< visiblechildren; ++i )
            {
                int c = i % cols;
                int r = i / cols;
                Viewport space(v.l + c*cw, v.t() - (r+1)*ch, cw,ch);
                VisibleChild(i).Resize(space);
            }
        }
    }
    
}

void View::Render()
{
    if(!extern_draw_function.empty() && show) {
        extern_draw_function(*this);
    }
    RenderChildren();
}

void View::RenderChildren()
{
    foreach(View* v, views)
        if(v->show) v->Render();
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

GLfloat View::GetClosestDepth(int x, int y, int radius) const
{
    // TODO: Get to work on android    
    const int zl = (radius*2+1);
    const int zsize = zl*zl;
    GLfloat zs[zsize];
    
#ifndef _ANDROID_
    glReadBuffer(GL_FRONT);
    glReadPixels(x-radius,y-radius,zl,zl,GL_DEPTH_COMPONENT,GL_FLOAT,zs);
#else
    std::fill(zs,zs+zsize, 0.8);    
#endif
    
    const GLfloat mindepth = *(std::min_element(zs,zs+zsize));
    return mindepth;
}

void View::GetObjectCoordinates(const OpenGlRenderState& cam_state, double winx, double winy, double winzdepth, GLdouble& x, GLdouble& y, GLdouble& z) const
{
    const GLint viewport[4] = {v.l,v.b,v.w,v.h};
    const OpenGlMatrix proj = cam_state.GetProjectionMatrix();
    const OpenGlMatrix mv = cam_state.GetModelViewMatrix();
    gluUnProject(winx, winy, winzdepth, mv.m, proj.m, viewport, &x, &y, &z);
}

void View::GetCamCoordinates(const OpenGlRenderState& cam_state, double winx, double winy, double winzdepth, GLdouble& x, GLdouble& y, GLdouble& z) const
{
    const GLint viewport[4] = {v.l,v.b,v.w,v.h};
    const OpenGlMatrix proj = cam_state.GetProjectionMatrix();
#ifndef _ANDROID_    
    gluUnProject(winx, winy, winzdepth, Identity4d, proj.m, viewport, &x, &y, &z);
#else
    gluUnProject(winx, winy, winzdepth, Identity4f, proj.m, viewport, &x, &y, &z);
#endif
}

View& View::SetFocus()
{
    context->activeDisplay = this;
    return *this;
}

View& View::SetBounds(Attach bottom, Attach top,  Attach left, Attach right, bool keep_aspect)
{
    SetBounds(top,bottom,left,right,0.0);
    aspect = keep_aspect ? v.aspect() : 0;
    return *this;
}

View& View::SetBounds(Attach bottom, Attach top,  Attach left, Attach right, double aspect)
{
    this->left = left;
    this->top = top;
    this->right = right;
    this->bottom = bottom;
    this->aspect = aspect;
    context->base.ResizeChildren();
    return *this;
}

View& View::SetAspect(double aspect)
{
    this->aspect = aspect;
    context->base.ResizeChildren();
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
                context->base.views.begin(), context->base.views.end(), &child
                );
    
    if( f != context->base.views.end() )
        context->base.views.erase(f);
    
    views.push_back(&child);
    context->base.ResizeChildren();
    return *this;
}

View& View::Show(bool show)
{
    this->show = show;
    context->base.ResizeChildren();
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

void View::SaveOnRender(const std::string& filename_prefix)
{
    const Viewport tosave = this->v.Intersect(this->vp);
    context->screen_capture.push(std::pair<std::string,Viewport>(filename_prefix,tosave ) );
}

void View::RecordOnRender(const std::string& record_uri)
{
#ifdef BUILD_PANGOLIN_VIDEO
    if(!context->recorder.IsOpen()) {
        Viewport area = GetBounds();
        context->record_view = this;
        context->recorder.Open(record_uri);
        context->recorder.AddStream(area.w, area.h, "YUV420P");
    }else{
        context->recorder.Reset();
    }
#else
    std::cerr << "Error: Video Support hasn't been built into this library." << std::endl;
#endif // BUILD_PANGOLIN_VIDEO
}

void View::SaveRenderNow(const std::string& filename_prefix, float scale)
{
    SaveViewFromFbo(filename_prefix, *this, scale);
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
    assert(0);
    return *this;
}

View* View::FindChild(int x, int y)
{
    // Find in reverse order to mirror draw order
    for( std::vector<View*>::const_reverse_iterator i = views.rbegin(); i != views.rend(); ++i )
        if( (*i)->show && (*i)->v.Contains(x,y) )
            return (*i);
    return 0;    
}

View& View::SetHandler(Handler* h)
{
    handler = h;
    return *this;
}

View& View::SetDrawFunction(const boost::function<void(View&)>& drawFunc)
{
    extern_draw_function = drawFunc;
    return *this;
}

}
