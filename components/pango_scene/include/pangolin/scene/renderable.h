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

#pragma once

#include <memory>
#include <map>
#include <random>

#include <pangolin/gl/opengl_render_state.h>
#include <pangolin/scene/interactive.h>

namespace pangolin {

class Renderable
{
public:
    using guid_t = GLuint;

    static guid_t UniqueGuid();

    Renderable(const std::weak_ptr<Renderable>& parent = std::weak_ptr<Renderable>());

    virtual ~Renderable();

    // Default implementation simply renders children.
    virtual void Render(const RenderParams& params = RenderParams());

    void RenderChildren(const RenderParams& params);

    std::shared_ptr<Renderable> FindChild(guid_t guid);

    Renderable& Add(const std::shared_ptr<Renderable>& child);

    void Remove(const std::shared_ptr<Renderable>& child)
    {
        if(child) {
            children.erase(child->guid);
        }
    }

    // Renderable properties
    const guid_t guid;
    std::weak_ptr<Renderable> parent;
    pangolin::OpenGlMatrix T_pc;
    bool should_show;

    // Children
    std::map<guid_t, std::shared_ptr<Renderable>> children;

    // Manipulator (handler, thing)
    std::shared_ptr<Manipulator> manipulator;
};

}
