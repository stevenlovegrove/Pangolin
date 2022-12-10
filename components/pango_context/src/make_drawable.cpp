
#include <pangolin/gui/make_drawable.h>

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

    prims->

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

Shared<Drawable> DrawableConversionTraits<sophus::Se3F32>::makeDrawable(const sophus::Se3F32& x) {
    auto prims = DrawnPrimitives::Create({
        .element_type=DrawnPrimitives::Type::axes,
    });
    prims->vertices->update(std::vector<sophus::Se3<float>>{x}, {});
    return prims;
}
}