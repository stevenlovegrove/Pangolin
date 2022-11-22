#pragma once

#include <algorithm>
#include <vector>

#include <Eigen/Core>
#include <pangolin/utils/logging.h>
#include <pangolin/gui/layer.h>

namespace pangolin
{

////////////////////////////////////////////////////////////////////
/// Represents a (possibly nested) arrangement of Panels on screen
///
struct LayerGroup
{
    enum class Grouping
    {
        stacked,    // layers blended over one another
        tabbed,     // one layer shown at a time, with user selecting current
        horizontal, // layers share client area horizontally
        vertical,   // layers share client area vertically
        flex        // layers are arranged in a dynamic group which fills the
                    // available space. Requires common aspect for each layer.
    };

    LayerGroup() = default;
    LayerGroup(Shared<Layer> layer) : layer(layer){}

    // return true if this layer and its children should be rendered
    inline bool isShown() const {return show_;}

    // show or hide this whole layer
    inline void show(bool visible=true) {
        show_ = visible;
    }

    // show or hide a layer in this LayerGroup tree
    inline void show(std::shared_ptr<Layer>& layer_to_show, bool visible=true) {
        if(layer == layer_to_show) {
            show(visible);
        }else{
            for(auto& lg : children) {
                lg.show(layer_to_show, visible);
            }
        }
    }

    Grouping grouping = Grouping::horizontal;
    std::vector<LayerGroup> children = {};
    std::shared_ptr<Layer> layer = nullptr;
    size_t selected_tab = 0;
    bool show_ = true;
    double width_over_height = 1.0;

    struct LayoutInfo
    {
        Eigen::Array2i min_pix = {0,0};
        Eigen::Array2d parts = {0.0f, 0.0f};
        MinMax<Eigen::Array2i> region;
    };

    mutable LayoutInfo cached_;
};

}

#include <pangolin/gui/layer_group_operators.h>
