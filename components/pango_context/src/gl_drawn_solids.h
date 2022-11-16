#pragma once

namespace pangolin
{

struct DrawnSolidsProgram
{
    void draw(
        const DrawnSolids& primitives,
        const sophus::CameraModel& camera,
        const sophus::Se3F64& cam_from_world,
        MinMax<double> near_far
    ){
        auto bind_prog = prog->bind();
        auto bind_vao = vao.bind();

        u_image_size = Eigen::Vector2f(camera.imageSize().width, camera.imageSize().height);
        u_kinv = linearCameraFromImage(camera).cast<float>();
        u_world_from_cam = cam_from_world.inverse().cast<float>().matrix();
        u_znear_zfar = Eigen::Vector2f(near_far.min(), near_far.max());

        PANGO_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }

private:
    const Shared<GlSlProgram> prog = GlSlProgram::Create({
        .sources = {{ .origin="/components/pango_opengl/shaders/main_plane.glsl" }}
    });
    GlVertexArrayObject vao = {};
    const GlUniform<Eigen::Matrix3f> u_kinv = {"kinv"};
    const GlUniform<Eigen::Matrix4f> u_world_from_cam = {"world_from_cam"};
    const GlUniform<Eigen::Vector2f> u_image_size = {"image_size"};
    const GlUniform<Eigen::Vector2f> u_znear_zfar = {"znear_zfar"};
};

}
