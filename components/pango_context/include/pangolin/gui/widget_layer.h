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
    struct Params {
        std::string name = "";
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

template<typename T>
struct LayerConversionTraits<Var<T>> {
    static Shared<Layer> makeLayer(const Var<T>& var) {
        return WidgetLayer::Create({
            .name=var.Meta().full_name, .size_hint={Parts{1},Pixels{50}}
        });
    }
};

}
