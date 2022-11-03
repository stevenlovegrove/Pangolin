#pragma once

#include <pangolin/gui/layer.h>

namespace pangolin
{

// Forward declarations
struct Widget;

////////////////////////////////////////////////////////////////////
/// Supports displaying a 2D tweak-var style interface
///
struct WidgetLayer : public Layer
{
    // virtual void add(SharedVector<Widget> widgets) = 0;

    struct Params {
        std::string title = "";
        Size size_hint = {Parts{1}, Parts{1}};

        // Font to use for widget text. Empty will use the
        // built-in font.
        std::string font_path = "";

        float widget_height_pix = 50.0;
        float widget_padding_pix = 8.0;
        float widget_font_height_pix = 32.0;

        // Proportionally scale all elements up or down
        // as a ratio of those above
        float scale = 1.0;
    };
    static Shared<WidgetLayer> Create(Params p);
};

// ////////////////////////////////////////////////////////////////////
// /// Represents a simple user interface element
// ///
// struct Widget : std::enable_shared_from_this<Widget>
// {
// };

// struct Seperator : public Widget
// {
//     struct Params {
//         std::string label = "";
//     };

//     static Shared<Seperator> Create(Params p);
// };

// struct Slider : public Widget
// {
//     struct Params {
//         std::string label = "";
//         double min=0.0;
//         double max=1.0;
//     };

//     static Shared<Slider> Create(Params p);
// };

// template<DerivedFrom<WidgetLayer> T>
// void operator+=(const Shared<T>& g1, const SharedVector<Widget>& g2)
// {
// }

}
