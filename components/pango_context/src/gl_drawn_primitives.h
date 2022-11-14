#pragma once

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

struct DrawnPrimitivesProgram
{
    void draw(
        const DrawnPrimitives& primitives,
        const sophus::CameraModel& camera,
        const sophus::Se3F64& cam_from_world,
        MinMax<double> near_far
    ){
        switch(primitives.element_type) {
            case DrawnPrimitives::Type::axes:
                drawAxes(primitives, camera, cam_from_world, near_far);
                break;
            default:
                drawPointsLinesTriangles(primitives, camera, cam_from_world, near_far);
                break;
        }
    }

    void drawAxes(
        const DrawnPrimitives& primitives,
        const sophus::CameraModel& camera,
        const sophus::Se3F64& cam_from_world,
        MinMax<double> near_far
    ){
        primitives.vertices->sync();
        if(!primitives.vertices->empty()) {
            auto bind_prog = prog_axes->bind();
            auto bind_vao = vao.bind();
            auto enable_depth = (primitives.enable_visibility_testing) ?
                ScopedGlDisable::noOp() : ScopedGlDisable(GL_DEPTH_TEST);

            u_intrinsics = linearClipFromCamera(camera, near_far).cast<float>();
            u_cam_from_world = (cam_from_world.matrix() * primitives.world_from_drawable).cast<float>();
            u_color = primitives.default_color.cast<float>();
            u_length = primitives.default_radius;

            auto bind_bo = primitives.vertices->bind();
            PANGO_ENSURE(primitives.vertices->dataType().is<sophus::Se3<float>>() );
            // xyzw quaternion
            PANGO_GL(glVertexAttribPointer(0, 4, GL_FLOAT, false, 8*sizeof(float), 0));
            // translation + uninitialized padding
            PANGO_GL(glVertexAttribPointer(1, 4, GL_FLOAT, false, 8*sizeof(float), (uint8_t*)(4*sizeof(float))) );
            PANGO_GL(glEnableVertexAttribArray(0));
            PANGO_GL(glEnableVertexAttribArray(1));

            PANGO_GL(glPointSize(5.0));
            PANGO_GL(glDrawArrays(GL_POINTS, 0, primitives.vertices->numElements()));
        }
    }

    void drawPointsLinesTriangles(
        const DrawnPrimitives& primitives,
        const sophus::CameraModel& camera,
        const sophus::Se3F64& cam_from_world,
        MinMax<double> near_far
    ) {
        primitives.vertices->sync();
        if(!primitives.vertices->empty()) {
            auto bind_prog = prog->bind();
            auto bind_vao = vao.bind();
            auto enable_depth = (primitives.enable_visibility_testing) ?
                ScopedGlDisable::noOp() : ScopedGlDisable(GL_DEPTH_TEST);

            u_intrinsics = linearClipFromCamera(camera, near_far).cast<float>();
            u_cam_from_world = (cam_from_world.matrix() * primitives.world_from_drawable).cast<float>();
            u_color = primitives.default_color.cast<float>();
            vao.addVertexAttrib(0, *primitives.vertices);
            PANGO_GL(glPointSize(primitives.default_radius*2.0f));
            PANGO_GL(glDrawArrays(toGlEnum(primitives.element_type), 0, primitives.vertices->numElements()));
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

}
