#include <pangolin/handler/handler.h>
#include <pangolin/plot/range.h>

namespace pangolin
{

class ImageViewHandler : public Handler
{
public:
    ImageViewHandler(size_t w, size_t h)
        : linked_view_handler(0), rview_default(0,w,0,h),
          rview(rview_default), target(rview)
    {
    }

    void UpdateView()
    {
        const float sf = 1.0f / 20.0f;

        if( linked_view_handler ) {
            // Synchronise rview and target with linked plotter
            rview = linked_view_handler->rview;
            target = linked_view_handler->target;
        }else{
            // Animate view window toward target
            pangolin::XYRange d = target - rview;
            rview += d * sf;
        }
    }

    pangolin::XYRange& GetView()
    {
        return target;
    }

    pangolin::XYRange& GetDefaultView()
    {
        return rview_default;
    }

    pangolin::XYRange& GetSelection()
    {
        return selection;
    }

    void SetView(const pangolin::XYRange& range)
    {
        ImageViewHandler& tv = linked_view_handler ? *linked_view_handler : *this;
        tv.rview = range;
    }

    void SetViewSmooth(const pangolin::XYRange& range)
    {
        ImageViewHandler& tv = linked_view_handler ? *linked_view_handler : *this;
        tv.target = range;
    }

    void ScrollView(float x, float y)
    {
        ImageViewHandler& tv = linked_view_handler ? *linked_view_handler : *this;
        tv.target.x += x;
        tv.target.y += y;
        tv.rview.x += x;
        tv.rview.y += y;
    }

    void ScrollViewSmooth(float x, float y)
    {
        ImageViewHandler& tv = linked_view_handler ? *linked_view_handler : *this;
        tv.target.x += x;
        tv.target.y += y;
    }

    void ScaleView(float x, float y, float cx, float cy)
    {
        ImageViewHandler& tv = linked_view_handler ? *linked_view_handler : *this;
        tv.target.x.Scale(x,cx);
        tv.target.y.Scale(y,cy);
        tv.rview.x.Scale(x,cx);
        tv.rview.y.Scale(y,cy);
    }

    void ScaleViewSmooth(float x, float y, float cx, float cy)
    {
        ImageViewHandler& tv = linked_view_handler ? *linked_view_handler : *this;
        tv.target.x.Scale(x,cx);
        tv.target.y.Scale(y,cy);
    }

    void ResetView()
    {
        ImageViewHandler& tv = linked_view_handler ? *linked_view_handler : *this;
        tv.target = tv.rview_default;
    }

    ///////////////////////////////////////////////////////
    /// pangolin::Handler
    ///////////////////////////////////////////////////////

    void Keyboard(View&, unsigned char key, int x, int y, bool pressed) override
    {
        const float mvfactor = 1.0f / 10.0f;
        const float c[2] = { rview.x.Mid(), rview.y.Mid() };

        if(pressed) {
            if(key == ' ') {
                if( selection.Area() > 0.0f) {
                    // Set view to equal selection
                    SetViewSmooth(selection);

                    // Reset selection
                    selection.x.max = selection.x.min;
                    selection.y.max = selection.y.min;
                }
            }else if(key == PANGO_SPECIAL + PANGO_KEY_LEFT) {
                const float w = rview.x.Size();
                const float dx = mvfactor*w;
                ScrollViewSmooth(-dx, 0);
            }else if(key == PANGO_SPECIAL + PANGO_KEY_RIGHT) {
                const float w = rview.x.Size();
                const float dx = mvfactor*w;
                ScrollViewSmooth(+dx, 0);
            }else if(key == PANGO_SPECIAL + PANGO_KEY_DOWN) {
                const float h = target.y.Size();
                const float dy = mvfactor*h;
                ScrollViewSmooth(0, -dy);
            }else if(key == PANGO_SPECIAL + PANGO_KEY_UP) {
                const float h = target.y.Size();
                const float dy = mvfactor*h;
                ScrollViewSmooth(0, +dy);
            }else if(key == '=') {
                ScaleViewSmooth(0.5, 0.5, c[0], c[1]);
            }else if(key == '-') {
                ScaleViewSmooth(2.0, 2.0, c[0], c[1]);
            }else if(key == 'r') {
                ResetView();
            }
        }
    }

    void Mouse(View&, pangolin::MouseButton button, int x, int y, bool pressed, int button_state) override
    {
    }

    void MouseMotion(View&, int x, int y, int button_state) override
    {
    }

    void PassiveMouseMotion(View&, int x, int y, int button_state) override
    {
    }

    void Special(View&, pangolin::InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state) override
    {
    }

protected:
    ImageViewHandler* linked_view_handler;

    pangolin::XYRange rview_default;
    pangolin::XYRange rview;
    pangolin::XYRange target;
    pangolin::XYRange selection;
};

}
