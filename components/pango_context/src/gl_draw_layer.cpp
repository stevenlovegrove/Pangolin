#include <pangolin/gui/draw_layer.h>
#include <pangolin/context/factory.h>
#include <pangolin/handler/handler.h>
#include <pangolin/gl/glsl.h>
#include <pangolin/gl/uniform.h>

#include "glutils.h"

#include <unordered_map>

namespace pangolin
{



// #PANGO_GL_DO_WARN(FUNC) \
//     do {
//         FUNC;
//         if(GLenum err = glGetError() != GL_NO_ERROR) {

//         }
//     }while(false);




struct RenderableImageProgram
{
    GlSlProgram prog;
    const GlUniform<Eigen::Vector4f> param = {"test"};
};

// void test()
// {
//     glUniform<int>(0, 10);
//     RenderableImageProgram test;
//     PANGO_GL(test.prog.Link());

//     int t1 = 9;
//     int t2[3];
//     glUniformArray<int,1>(0, &t1);
//     glUniformArray<int,3>(0, t2);

//     glUniform<int>(0, 0);
//     glUniform<float>(0, 1.0f);
//     glUniform<Eigen::Vector2f>(0, {1.0f, 2.0f});

//     // auto block = GlSlUniformBlock(
//     //     Named<float>("test"),
//     //     Named<int>("foo")
//     // );

//     // using  SomeBlockType = GlSlUniformBlock<
//     //     Named<float,"test">,
//     //     Named<int,"foo">
//     // >;

//     // SomeBlockType block;
// }

struct DrawLayerImpl : public DrawLayer {
    Eigen::Array3f debug_random_color;

    DrawLayerImpl(const DrawLayerImpl::Params& p)
        : size_hint_(p.size_hint),
        handler_(p.handler),
        cam_from_world_(p.cam_from_world),
        intrinsic_k_(p.intrinsic_k),
        non_linear_(p.non_linear),
        objects_(p.objects)
    {
        debug_random_color = (Eigen::Array3f::Random() + 1.0f) / 2.0;
    }

    void renderIntoRegion(const RenderParams& p) override {
        ScopedGlEnable en_scissor(GL_SCISSOR_TEST);
        setGlViewport(p.region);
        setGlScissor(p.region);
        glClearColor(debug_random_color[0], debug_random_color[2], debug_random_color[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // for(auto& obj : objects_) {
        //     if(auto* im = dynamic_cast<RenderableImage*>(obj.ptr())) {
        //     }
        // }
    }

    Size sizeHint() const override {
        return size_hint_;
    }

    void setProjection(
        const Eigen::Matrix4d& intrinsic_k,
        const NonLinearMethod non_linear = {},
        double duration_seconds = 0.0) override {

    }

    void setCamFromWorld(
        const Eigen::Matrix4d& cam_from_world,
        double duration_seconds = 0.0) override {

    }

    void setHandler(const std::shared_ptr<Handler>& handler) override {
        handler_ = handler;
    }

    MinMax<Eigen::Vector3d> getSceneBoundsInWorld() const override {
        return bounds_;
    }

    void add(const Shared<Renderable>& r) override {
        objects_.push_back(r);
    }

    void remove(const Shared<Renderable>& r) override {
        throw std::runtime_error("Not implemented yet...");
    }

    void clear() override {
        objects_.clear();
    }

    Size size_hint_;
    MinMax<Eigen::Vector3d> bounds_;
    std::shared_ptr<Handler> handler_;

    Eigen::Matrix4d cam_from_world_;
    Eigen::Matrix4d intrinsic_k_;
    NonLinearMethod non_linear_;

    std::vector<Shared<Renderable>> objects_;
};

PANGO_CREATE(DrawLayer) {
    return Shared<DrawLayerImpl>::make(p);
}

}
