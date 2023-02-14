
#include <pangolin/gui/drawn_solids.h>
#include <pangolin/gui/make_drawable.h>

namespace pangolin
{

Shared<Drawable> DrawableConversionTraits<draw::Shape>::makeDrawable(
    const draw::Shape& x)
{
  auto prims = DrawnPrimitives::Create({
      .element_type = DrawnPrimitives::Type::shapes,
      .default_size = x.size,
  });
  prims->vertices->queueUpdate(
      std::vector<Eigen::Vector3f>{x.pos.cast<float>()});
  prims->shapes->queueUpdate(
      std::vector<uint16_t>{static_cast<uint16_t>(x.type)});
  prims->colors->queueUpdate(
      std::vector<Eigen::Vector4f>{x.color.cast<float>()});
  return prims;
}

Shared<Drawable> DrawableConversionTraits<draw::Cube>::makeDrawable(
    const draw::Cube& cube)
{
  auto prims = DrawnPrimitives::Create({
      .element_type = DrawnPrimitives::Type::triangles,
  });

  std::vector<Eigen::Vector3f> vertices;
  std::vector<Color> colors;
  float s = 0.5f * cube.size;
  Eigen::Vector3f ppp(+s, +s, +s);
  Eigen::Vector3f ppm(+s, +s, -s);
  Eigen::Vector3f pmp(+s, -s, +s);
  Eigen::Vector3f pmm(+s, -s, -s);

  Eigen::Vector3f mpp(-s, +s, +s);
  Eigen::Vector3f mpm(-s, +s, -s);
  Eigen::Vector3f mmp(-s, -s, +s);
  Eigen::Vector3f mmm(-s, -s, -s);

  // right
  vertices.push_back(ppp);
  vertices.push_back(pmp);
  vertices.push_back(ppm);
  colors.push_back(cube.colors[0]);
  colors.push_back(cube.colors[0]);
  colors.push_back(cube.colors[0]);

  vertices.push_back(pmm);
  vertices.push_back(ppm);
  vertices.push_back(pmp);
  colors.push_back(cube.colors[0]);
  colors.push_back(cube.colors[0]);
  colors.push_back(cube.colors[0]);

  // left
  vertices.push_back(mpp);
  vertices.push_back(mpm);
  vertices.push_back(mmp);
  colors.push_back(cube.colors[3]);
  colors.push_back(cube.colors[3]);
  colors.push_back(cube.colors[3]);

  vertices.push_back(mmm);
  vertices.push_back(mmp);
  vertices.push_back(mpm);
  colors.push_back(cube.colors[3]);
  colors.push_back(cube.colors[3]);
  colors.push_back(cube.colors[3]);

  // up
  vertices.push_back(ppp);
  vertices.push_back(ppm);
  vertices.push_back(mpp);
  colors.push_back(cube.colors[1]);
  colors.push_back(cube.colors[1]);
  colors.push_back(cube.colors[1]);

  vertices.push_back(mpm);
  vertices.push_back(mpp);
  vertices.push_back(ppm);
  colors.push_back(cube.colors[1]);
  colors.push_back(cube.colors[1]);
  colors.push_back(cube.colors[1]);

  // down
  vertices.push_back(pmp);
  vertices.push_back(mmp);
  vertices.push_back(pmm);
  colors.push_back(cube.colors[4]);
  colors.push_back(cube.colors[4]);
  colors.push_back(cube.colors[4]);

  vertices.push_back(mmm);
  vertices.push_back(pmm);
  vertices.push_back(mmp);
  colors.push_back(cube.colors[4]);
  colors.push_back(cube.colors[4]);
  colors.push_back(cube.colors[4]);

  // front
  vertices.push_back(ppp);
  vertices.push_back(mpp);
  vertices.push_back(pmp);
  colors.push_back(cube.colors[2]);
  colors.push_back(cube.colors[2]);
  colors.push_back(cube.colors[2]);

  vertices.push_back(mmp);
  vertices.push_back(pmp);
  vertices.push_back(mpp);
  colors.push_back(cube.colors[2]);
  colors.push_back(cube.colors[2]);
  colors.push_back(cube.colors[2]);

  // back
  vertices.push_back(ppm);
  vertices.push_back(pmm);
  vertices.push_back(mpm);
  colors.push_back(cube.colors[5]);
  colors.push_back(cube.colors[5]);
  colors.push_back(cube.colors[5]);

  vertices.push_back(mmm);
  vertices.push_back(mpm);
  vertices.push_back(pmm);
  colors.push_back(cube.colors[5]);
  colors.push_back(cube.colors[5]);
  colors.push_back(cube.colors[5]);

  prims->vertices->queueUpdate(std::move(vertices));
  prims->colors->queueUpdate(std::move(colors));

  return prims;
}

Shared<Drawable> DrawableConversionTraits<draw::Icosphere>::makeDrawable(
    const draw::Icosphere& sphere)
{
  auto prims = DrawnPrimitives::Create({
      .element_type = DrawnPrimitives::Type::triangles,
  });
  using FaceT = Eigen::Matrix<uint32_t, 3, 1>;
  std::vector<FaceT> faces;

  // http://www.songho.ca/opengl/gl_sphere.html#icosphere
  static float constexpr kHAngle = M_PI / 180 * 72;
  const float k_v_angle = std::atan(1.0f / 2);

  float h_angle1 = -M_PI / 2 - kHAngle / 2;  // start from -126 deg at 1st row
  float h_angle2 = -M_PI / 2;                // start from -90 deg at 2nd row

  std::vector<Eigen::Vector3f> vertices;

  vertices.push_back({0, 0, sphere.radius});

  for (int i = 0; i < 5; ++i) {
    float z = sphere.radius * std::sin(k_v_angle);
    float xy = sphere.radius * std::cos(k_v_angle);

    vertices.push_back(
        Eigen::Vector3f(xy * std::cos(h_angle1), xy * std::sin(h_angle1), z));

    vertices.push_back(
        Eigen::Vector3f(xy * std::cos(h_angle2), xy * std::sin(h_angle2), -z));

    h_angle1 += kHAngle;
    h_angle2 += kHAngle;
  }

  vertices.push_back({0, 0, -sphere.radius});

  faces.push_back(FaceT(0, 1, 3));
  faces.push_back(FaceT(0, 3, 5));
  faces.push_back(FaceT(0, 5, 7));
  faces.push_back(FaceT(0, 7, 9));
  faces.push_back(FaceT(0, 9, 1));

  faces.push_back(FaceT(11, 4, 2));
  faces.push_back(FaceT(11, 6, 4));
  faces.push_back(FaceT(11, 8, 6));
  faces.push_back(FaceT(11, 10, 8));
  faces.push_back(FaceT(11, 2, 10));

  faces.push_back(FaceT(1, 2, 3));
  faces.push_back(FaceT(2, 4, 3));
  faces.push_back(FaceT(3, 4, 5));
  faces.push_back(FaceT(4, 6, 5));
  faces.push_back(FaceT(5, 6, 7));
  faces.push_back(FaceT(6, 8, 7));
  faces.push_back(FaceT(7, 8, 9));
  faces.push_back(FaceT(8, 10, 9));
  faces.push_back(FaceT(9, 10, 1));
  faces.push_back(FaceT(10, 2, 1));

  for (size_t i = 0; i < sphere.num_subdivisions; ++i) {
    std::vector<FaceT> source_faces = faces;
    faces.clear();
    for (const FaceT& t : source_faces) {
      Eigen::Vector3f p0_p1 =
          (vertices[t.x()] + vertices[t.y()]).normalized() * sphere.radius;
      int p0_p1_id = vertices.size();
      vertices.push_back(p0_p1);

      Eigen::Vector3f p0_p2 =
          (vertices[t.x()] + vertices[t.z()]).normalized() * sphere.radius;
      int p0_p2_id = vertices.size();
      vertices.push_back(p0_p2);

      Eigen::Vector3f p1_p2 =
          (vertices[t.y()] + vertices[t.z()]).normalized() * sphere.radius;
      int p1_p2_id = vertices.size();
      vertices.push_back(p1_p2);

      faces.push_back(FaceT(t.x(), p0_p1_id, p0_p2_id));
      faces.push_back(FaceT(t.y(), p1_p2_id, p0_p1_id));
      faces.push_back(FaceT(t.z(), p0_p2_id, p1_p2_id));
      faces.push_back(FaceT(p0_p1_id, p1_p2_id, p0_p2_id));
    }
  }

  prims->vertices->queueUpdate(std::move(vertices));
  prims->indices->queueUpdate(std::move(faces));

  return prims;
}

Shared<Drawable> DrawableConversionTraits<draw::CheckerPlane>::makeDrawable(
    const draw::CheckerPlane&)
{
  auto board = DrawnSolids::Create({
      .object_type = DrawnSolids::Type::checkerboard,
  });
  return board;
}

Shared<Drawable> DrawableConversionTraits<draw::Axes>::makeDrawable(
    const draw::Axes& axes)
{
  auto prims = DrawnPrimitives::Create({
      .element_type = DrawnPrimitives::Type::axes,
  });
  prims->vertices->queueUpdate(axes.drawable_from_axis_poses);
  prims->default_size = axes.scale;
  return prims;
}

Shared<Drawable> DrawableConversionTraits<draw::Points3f>::makeDrawable(
    const draw::Points3f& points)
{
  auto prims = DrawnPrimitives::Create(
      {.element_type = DrawnPrimitives::Type::points,
       .default_color = points.color});

  prims->vertices->queueUpdate(points.points);
  return prims;
}

Shared<Drawable> DrawableConversionTraits<draw::Points3d>::makeDrawable(
    const draw::Points3d& points)
{
  std::vector<Eigen::Vector3f> vertices;

  auto prims = DrawnPrimitives::Create(
      {.element_type = DrawnPrimitives::Type::points,
       .default_color = points.color});

  for (const auto& p : points.points) {
    vertices.push_back(p.cast<float>());
  }
  prims->vertices->queueUpdate(std::move(vertices));
  return prims;
}

Shared<Drawable>
DrawableConversionTraits<std::vector<draw::Line3>>::makeDrawable(
    const std::vector<draw::Line3>& lines)
{
  std::vector<Eigen::Vector3f> vertices;
  std::vector<Color> colors;

  auto prims = DrawnPrimitives::Create({
      .element_type = DrawnPrimitives::Type::lines,
      .default_size = 3,
  });

  for (const draw::Line3& line : lines) {
    vertices.push_back(line.a);
    vertices.push_back(line.b);
    colors.push_back(line.color);
    colors.push_back(line.color);
  }
  prims->vertices->queueUpdate(std::move(vertices));
  prims->colors->queueUpdate(std::move(colors));

  return prims;
}

Shared<Drawable>
DrawableConversionTraits<std::vector<draw::Line2>>::makeDrawable(
    const std::vector<draw::Line2>& lines)
{
  std::vector<Eigen::Vector3f> vertices;
  std::vector<Color> colors;

  auto prims = DrawnPrimitives::Create({
      .element_type = DrawnPrimitives::Type::lines,
      .default_size = 3,
  });

  for (const draw::Line2& line : lines) {
    vertices.push_back(line.toXy0A());
    vertices.push_back(line.toXy0B());
    colors.push_back(line.color);
    colors.push_back(line.color);
  }
  prims->vertices->queueUpdate(std::move(vertices));
  prims->colors->queueUpdate(std::move(colors));
  return prims;
}

Shared<Drawable> DrawableConversionTraits<draw::CameraFrustum>::makeDrawable(
    const draw::CameraFrustum& frustum)
{
  std::vector<draw::Line3> lines;

  Eigen::Vector2d left(-0.5, 0.5 * frustum.camera.imageSize().height - 0.25);
  Eigen::Vector2d right(
      frustum.camera.imageSize().width - 0.5,
      0.5 * frustum.camera.imageSize().height - 0.25);
  Eigen::Vector2d top(0.5 * frustum.camera.imageSize().width - 0.25, -0.5);
  Eigen::Vector2d bottom(
      0.5 * frustum.camera.imageSize().width - 0.25,
      frustum.camera.imageSize().height - 0.5);

  Eigen::Vector3d top_near = frustum.near * frustum.camera.camUnproj(top, 1.0);
  Eigen::Vector3d top_far = frustum.far * frustum.camera.camUnproj(top, 1.0);

  Eigen::Vector3d right_near =
      frustum.near * frustum.camera.camUnproj(right, 1.0);
  Eigen::Vector3d right_far =
      frustum.far * frustum.camera.camUnproj(right, 1.0);

  Eigen::Vector3d left_near =
      frustum.near * frustum.camera.camUnproj(left, 1.0);
  Eigen::Vector3d left_far = frustum.far * frustum.camera.camUnproj(left, 1.0);

  Eigen::Vector3d bottom_near =
      frustum.near * frustum.camera.camUnproj(bottom, 1.0);
  Eigen::Vector3d bottom_far =
      frustum.far * frustum.camera.camUnproj(bottom, 1.0);

  Eigen::Vector3d top_right_near = 0.5 * top_near + 0.5 * right_near;
  Eigen::Vector3d top_left_near = 0.5 * top_near + 0.5 * left_near;
  Eigen::Vector3d bottom_right_near = 0.5 * bottom_near + 0.5 * right_near;
  Eigen::Vector3d bottom_left_near = 0.5 * bottom_near + 0.5 * left_near;

  Eigen::Vector3d top_right_far = 0.5 * top_far + 0.5 * right_far;
  Eigen::Vector3d top_left_far = 0.5 * top_far + 0.5 * left_far;
  Eigen::Vector3d bottom_right_far = 0.5 * bottom_far + 0.5 * right_far;
  Eigen::Vector3d bottom_left_far = 0.5 * bottom_far + 0.5 * left_far;

  lines.emplace_back(top_right_near, top_right_far, frustum.color);
  lines.emplace_back(top_left_near, top_left_far, frustum.color);
  lines.emplace_back(bottom_right_near, bottom_right_far, frustum.color);
  lines.emplace_back(bottom_left_near, bottom_left_far, frustum.color);

  lines.emplace_back(top_left_near, top_right_near, frustum.color);
  lines.emplace_back(top_right_near, bottom_right_near, frustum.color);
  lines.emplace_back(bottom_right_near, bottom_left_near, frustum.color);
  lines.emplace_back(bottom_left_near, top_left_near, frustum.color);

  lines.emplace_back(top_left_far, top_right_far, frustum.color);
  lines.emplace_back(top_right_far, bottom_right_far, frustum.color);
  lines.emplace_back(bottom_right_far, bottom_left_far, frustum.color);
  lines.emplace_back(bottom_left_far, top_left_far, frustum.color);

  return DrawableConversionTraits<std::vector<draw::Line3>>::makeDrawable(
      lines);
}

Shared<Drawable> DrawableConversionTraits<draw::Circle3>::makeDrawable(
    const draw::Circle3& circle)
{
  auto prims = DrawnPrimitives::Create(
      {.element_type = DrawnPrimitives::Type::lines,
       .default_color = circle.color});
  size_t num_vertices = 3.f * std::ceil(circle.radius);

  std::vector<Eigen::Vector3f> vertices;
  vertices.reserve(num_vertices);
  for (size_t i = 0; i < num_vertices; ++i) {
    const auto theta = (2.0 * M_PI) / num_vertices * i;
    vertices.push_back(Eigen::Vector3d{
        circle.center + circle.radius * std::cos(theta) * circle.a +
        circle.radius * std::sin(theta) * circle.b}
                           .cast<float>());
  }

  prims->vertices->queueUpdate(std::move(vertices));

  return prims;
}
}  // namespace pangolin
