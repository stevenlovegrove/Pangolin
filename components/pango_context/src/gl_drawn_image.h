#pragma once

namespace pangolin
{

struct DrawnImageProgram
{
    // This program renders an image on the z=0 plane in a discrete pixel
    // convention such that x=0,y=0 is the center of the top-left pixel.
    // As such the vertex extremes for the texture map from (-0.5,-0.5) to
    // (w-0.5,h-0.5).
    // TODO: Add a flag to take convention as configuration.

    void draw(
        const DrawnImage& drawn_image,
        const sophus::CameraModel& camera,
        const sophus::Se3F64& cam_from_world,
        MinMax<double> near_far
    ) {
        // ensure we're synced
        drawn_image.image->sync();

        PANGO_GL(glActiveTexture(GL_TEXTURE0));
        auto bind_im = drawn_image.image->bind();
        auto bind_prog = prog->bind();
        auto bind_vao = vao.bind();

        u_image_size = Eigen::Vector2f(camera.imageSize().width, camera.imageSize().height);
        u_intrinsics = linearClipFromCamera(camera, near_far).cast<float>();
        u_cam_from_world = cam_from_world.cast<float>().matrix();

        // TODO: load from DrawnImage param if specified.
        if(drawn_image.image->pixelType().num_channels == 1) {
            u_color_transform = (Eigen::Matrix4f() << 1,0,0,0,  1,0,0,0,  1,0,0,0, 0,0,0,1).finished();
        }else{
            u_color_transform = Eigen::Matrix4f::Identity();
        }

        PANGO_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }
private:
    const Shared<GlSlProgram> prog = GlSlProgram::Create({
        .sources = {{ .origin="/components/pango_opengl/shaders/main_image.glsl" }}
    });
    GlVertexArrayObject vao = {};
    const GlUniform<int> texture_unit = {"image"};
    const GlUniform<Eigen::Matrix4f> u_intrinsics = {"proj"};
    const GlUniform<Eigen::Matrix4f> u_cam_from_world = {"cam_from_world"};
    const GlUniform<Eigen::Vector2f> u_image_size = {"image_size"};
    const GlUniform<Eigen::Matrix4f> u_color_transform = {"color_transform"};
};

}
