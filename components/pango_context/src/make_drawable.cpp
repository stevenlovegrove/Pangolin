
#include <pangolin/gui/make_drawable.h>
#include <pangolin/gui/drawn_solids.h>

namespace pangolin{

Shared<Drawable> DrawableConversionTraits<Draw::Shape>::makeDrawable(const Draw::Shape& x) {
    auto prims = DrawnPrimitives::Create({
        .element_type=DrawnPrimitives::Type::shapes,
        .default_size = x.size,
    });
    prims->vertices->update(std::vector<Eigen::Vector3f>{ x.pos.cast<float>() }, {});
    prims->shapes->update(std::vector<uint16_t>{ static_cast<uint16_t>(x.type) }, {});
    prims->colors->update(std::vector<Eigen::Vector4f>{ x.color.cast<float>() }, {});
    return prims;
}

Shared<Drawable> DrawableConversionTraits<Draw::Cube>::makeDrawable(const Draw::Cube& cube) {
 auto prims = DrawnPrimitives::Create({
        .element_type=DrawnPrimitives::Type::triangles,
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

  prims->vertices->update( vertices, {});
  prims->colors->update(colors, {});

  return prims;
}

Shared<Drawable> DrawableConversionTraits<Draw::Icosphere>::makeDrawable(const Draw::Icosphere& sphere){
    auto prims = DrawnPrimitives::Create({
        .element_type=DrawnPrimitives::Type::triangles,
    });
 using FaceT = Eigen::Matrix<uint32_t, 3, 1>;
  std::vector<FaceT> faces;

  // http://www.songho.ca/opengl/gl_sphere.html#icosphere
  static float constexpr kHAngle = M_PI / 180 * 72;
  float const k_v_angle = std::atan(1.0f / 2);

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
    for (FaceT const& t : source_faces) {
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

  prims->vertices->update( vertices, {});
  prims->indices->update(faces, {});
 
  return prims;
}

Shared<Drawable> DrawableConversionTraits<Draw::CheckerPlane>::makeDrawable(const Draw::CheckerPlane& ) {
    auto board = DrawnSolids::Create({
        .object_type=DrawnSolids::Type::checkerboard,
    });
    return board;
}

Shared<Drawable> DrawableConversionTraits<Draw::Axes>::makeDrawable(const Draw::Axes& axes) {
    auto prims = DrawnPrimitives::Create({
        .element_type=DrawnPrimitives::Type::axes,
    });
    prims->vertices->update(axes.drawable_from_axis_poses, {});
    prims->default_size = axes.scale;
    return prims;
}
}