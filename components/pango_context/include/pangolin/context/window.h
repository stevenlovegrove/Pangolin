#pragma once

namespace pangolin
{

struct Window : std::enable_shared_from_this<Window>
{
    using Size = Eigen::Vector2i;

    virtual void resize(const Size& window_size) = 0;
};

}