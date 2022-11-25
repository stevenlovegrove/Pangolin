#include <pangolin/gui/drawn_primitives.h>

#include <pangolin/context/factory.h>
#include <pangolin/render/gl_vao.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>

#include "gl_utils.h"
#include "camera_utils.h"

namespace pangolin
{

constexpr GLenum toGlEnum (DrawnPrimitives::Type type) {
    switch(type) {
        case DrawnPrimitives::Type::points: return GL_POINTS;
        case DrawnPrimitives::Type::lines: return GL_LINES;
        case DrawnPrimitives::Type::line_strip: return GL_LINE_STRIP;
        case DrawnPrimitives::Type::line_loop: return GL_LINE_LOOP;
        case DrawnPrimitives::Type::triangles: return GL_TRIANGLES;
        case DrawnPrimitives::Type::triangle_strip: return GL_TRIANGLE_STRIP;
        default: return 0;
    }
}

struct GlDrawnPrimitives : public DrawnPrimitives
{
    void draw( const DrawLayer::ViewParams& params) override {

        switch(element_type) {
            case DrawnPrimitives::Type::axes:
                drawAxes(params);
                break;
            default:
                drawPointsLinesTriangles(params);
                break;
        }
    }

    MinMax<Eigen::Vector3d> boundsInParent() const override {
        return MinMax<Eigen::Vector3d>();
    }

    void drawAxes( const DrawLayer::ViewParams& params ){
        vertices->sync();
        if(!vertices->empty()) {
            auto bind_prog = prog_axes->bind();
            auto bind_vao = vao.bind();

            u_intrinsics = (params.clip_from_image * params.image_from_camera).cast<float>();
            u_cam_from_world = (params.camera_from_world.matrix() * parent_from_drawable).cast<float>();
            u_color = default_color.cast<float>();
            u_length = default_radius;

            auto bind_bo = vertices->bind();
            PANGO_ENSURE(vertices->dataType().is<sophus::Se3<float>>() );
            // xyzw quaternion
            PANGO_GL(glVertexAttribPointer(0, 4, GL_FLOAT, false, 8*sizeof(float), 0));
            // translation + uninitialized padding
            PANGO_GL(glVertexAttribPointer(1, 4, GL_FLOAT, false, 8*sizeof(float), (uint8_t*)(4*sizeof(float))) );
            PANGO_GL(glEnableVertexAttribArray(0));
            PANGO_GL(glEnableVertexAttribArray(1));

            PANGO_GL(glPointSize(5.0));
            PANGO_GL(glDrawArrays(GL_POINTS, 0, vertices->numElements()));
        }
    }

    void drawPointsLinesTriangles( const DrawLayer::ViewParams& params ) {
        vertices->sync();
        if(!vertices->empty()) {
            auto bind_prog = prog->bind();
            auto bind_vao = vao.bind();

            u_intrinsics = (params.clip_from_image * params.image_from_camera).cast<float>();
            u_cam_from_world = (params.camera_from_world.matrix() * parent_from_drawable).cast<float>();
            u_color = default_color.cast<float>();
            vao.addVertexAttrib(0, *vertices);
            PANGO_GL(glPointSize(default_radius*2.0f));
            PANGO_GL(glDrawArrays(toGlEnum(element_type), 0, vertices->numElements()));
        }
    }

private:
    const Shared<GlSlProgram> prog = GlSlProgram::Create({
        .sources = {{ .origin="/components/pango_opengl/shaders/main_primitives_points.glsl" }}
    });
    const Shared<GlSlProgram> prog_axes = GlSlProgram::Create({
        .sources = {{ .origin="/components/pango_opengl/shaders/main_axes.glsl" }}
    });
    GlVertexArrayObject vao = {};
    const GlUniform<Eigen::Matrix4f> u_intrinsics = {"proj"};
    const GlUniform<Eigen::Matrix4f> u_cam_from_world = {"cam_from_world"};
    const GlUniform<Eigen::Vector4f> u_color = {"color"};
    const GlUniform<float> u_length = {"length"};
};

PANGO_CREATE(DrawnPrimitives) {
    auto r = Shared<GlDrawnPrimitives>::make();
    r->element_type = p.element_type;
    r->parent_from_drawable = p.parent_from_drawable;
    r->default_color = p.default_color;
    r->default_radius = p.default_radius;
    return r;
}

}
