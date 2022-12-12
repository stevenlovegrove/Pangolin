#pragma once

#include <sophus/image/runtime_image.h>
#include <pangolin/maths/conventions.h>
#include <pangolin/render/device_texture.h>
#include <pangolin/render/colormap.h>

#include <pangolin/gui/draw_layer.h>

namespace pangolin
{

// Just renders a fixed checker as a background for image layers to help
// distinguish background from image
struct DrawnPlotBackground : public Drawable
{
    struct Params {
        // Plot background color
        Eigen::Vector4f color_background = {0.95, 0.95,0.95, 1.0};

        // The ammount to scale the background color for each overlapping tick
        // > 1.0 will lighten the graph
        // < 1.0 will darken the graph
        Eigen::Vector4f tick_color_scale = {0.90, 0.90, 1.0, 1.0};

        // The scale from one tick octave to the next
        float tick_to_tick = 5.0;
    };
    static Shared<DrawnPlotBackground> Create(Params p);
};

}
