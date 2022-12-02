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
            case DrawnPrimitives::Type::shapes:
                drawShapes(params);
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
        if(!prog) {
            prog = GlSlProgram::Create({
                .sources = {{ .origin="/components/pango_opengl/shaders/main_axes.glsl" }}
            });
        }

        vertices->sync();
        if(!vertices->empty()) {
            auto bind_prog = prog->bind();
            auto bind_vao = vao.bind();

            u_intrinsics = (params.clip_from_image * params.image_from_camera).cast<float>();
            u_cam_from_world = (params.camera_from_world.matrix() * parent_from_drawable).cast<float>();
            u_color = default_color.cast<float>();
            u_size = default_size;

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

    void drawShapes( const DrawLayer::ViewParams& params ) {
        if(!prog) {
            prog = GlSlProgram::Create({
                .sources = {{ .origin="/components/pango_opengl/shaders/main_shapes.glsl" }}
            });
        }

        vertices->sync();
        colors->sync();
        shapes->sync();

        if(!vertices->empty()) {
            auto bind_prog = prog->bind();
            auto bind_vao = vao.bind();

            u_intrinsics = (params.clip_from_image * params.image_from_camera).cast<float>();
            u_cam_from_world = (params.camera_from_world.matrix() * parent_from_drawable).cast<float>();
            u_size = default_size;
            vao.addVertexAttrib(0, *vertices);
            vao.addVertexAttrib(1, *colors);
            vao.addVertexAttrib(2, *shapes);
            PANGO_GL(glDrawArrays(GL_POINTS, 0, vertices->numElements()));
        }
    }

    void drawPointsLinesTriangles( const DrawLayer::ViewParams& params ) {
        constexpr int location_vertex = 0;
        constexpr int location_colors = 1;

        indices->sync();
        vertices->sync();
        colors->sync();

        if(!prog) {
            GlSlProgram::Defines defines;
            defines["VERTEX_COLORS"] = std::to_string(!colors->empty());

            prog = GlSlProgram::Create({
                .sources = {{ .origin="/components/pango_opengl/shaders/main_primitives_points.glsl" }},
                .program_defines = defines
            });
        }


        if(!vertices->empty()) {
            auto bind_prog = prog->bind();
            auto bind_vao = vao.bind();

            u_intrinsics = (params.clip_from_image * params.image_from_camera).cast<float>();
            u_cam_from_world = (params.camera_from_world.matrix() * parent_from_drawable).cast<float>();
            vao.addVertexAttrib(location_vertex, *vertices);
            if(!colors->empty()) {
                vao.addVertexAttrib(location_colors, *colors);
            }else{
                u_color = default_color.cast<float>();
            }

            PANGO_GL(glPointSize(default_size));

            if(indices->empty()) {
                PANGO_GL(glDrawArrays(toGlEnum(element_type), 0, vertices->numElements()));
            }else{
                const GlFormatInfo gl_fmt = glTypeInfo(indices->dataType());
                auto bind_ibo = indices->bind();
                PANGO_GL(glDrawElements(toGlEnum(element_type), indices->numElements(), gl_fmt.gl_type, 0));
            }

        }
    }

private:
    std::shared_ptr<GlSlProgram> prog;

    GlVertexArrayObject vao = {};
    const GlUniform<Eigen::Matrix4f> u_intrinsics = {"proj"};
    const GlUniform<Eigen::Matrix4f> u_cam_from_world = {"cam_from_world"};
    const GlUniform<Eigen::Vector4f> u_color = {"color"};
    const GlUniform<float> u_size = {"size"};
};

PANGO_CREATE(DrawnPrimitives) {
    auto r = Shared<GlDrawnPrimitives>::make();
    r->element_type = p.element_type;
    r->parent_from_drawable = p.parent_from_drawable;
    r->default_color = p.default_color;
    r->default_size = p.default_size;
    return r;
}

}
