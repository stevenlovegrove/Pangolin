#include <pangolin/drawable/drawn_primitives.h>
#include <pangolin/drawable/mascot.h>
#include <pangolin/image/image_io.h>
#include <pangolin/utils/for_all_t.h>

extern const unsigned char pango_ply[];
extern const unsigned pango_ply_size;

extern const unsigned char matcap_png[];
extern const unsigned matcap_png_size;

namespace pangolin
{

// https://stackoverflow.com/questions/34186389/istream-for-char-buffer
struct membuf : std::streambuf {
  membuf(char* begin, char* end) { this->setg(begin, begin, end); }
};
class imemstream : private virtual membuf, public std::istream
{
  public:
  imemstream(char* begin, char* end) :
      membuf(begin, end),
      std::ios(static_cast<std::streambuf*>(this)),
      std::istream(static_cast<std::streambuf*>(this))
  {
  }
};

Shared<Drawable> makePangolin()
{
  auto mesh = DrawnGroup::Load(pango_ply, pango_ply_size, {});
  auto material_image = DeviceTexture::Create({});
  imemstream stream((char*)matcap_png, (char*)matcap_png + matcap_png_size);
  material_image->update(LoadImage(stream, ImageFileTypePng));
  forAllT<DrawnPrimitives>(mesh->children, [&](DrawnPrimitives& p) {
    p.material_image = material_image;
    p.pose.parent_from_drawable.setScale(0.1);
  });
  return mesh;
}

}  // namespace pangolin
