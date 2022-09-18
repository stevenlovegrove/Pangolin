#pragma once

#include <pangolin/image/managed_image.h>
#include <pangolin/gl/glfont.h>
#include <pangolin/gl/glsl.h>

#include <locale>
#include <codecvt>

namespace pangolin
{

inline pangolin::ManagedImage<Eigen::Vector4f> MakeFontLookupImage(const pangolin::GlFont& font)
{
    pangolin::ManagedImage<Eigen::Vector4f> img(font.chardata.size(), 2);

    for(const auto& cp_char : font.chardata) {
        // font offset
        img(cp_char.second.AtlasIndex(), 0) = {
            cp_char.second.GetVert(0).tu,
            cp_char.second.GetVert(0).tv,
            cp_char.second.GetVert(2).tu - cp_char.second.GetVert(0).tu, // w
            cp_char.second.GetVert(2).tv - cp_char.second.GetVert(0).tv  // h
        };
        // screen offset
        img(cp_char.second.AtlasIndex(), 1) = {
            cp_char.second.GetVert(0).x,
            -cp_char.second.GetVert(0).y,
            cp_char.second.GetVert(2).x - cp_char.second.GetVert(0).x, // w
            cp_char.second.GetVert(0).y - cp_char.second.GetVert(2).y  // h
        };
    }

    return img;
}

inline std::u16string to_index_string(const pangolin::GlFont& font, const std::u32string& utf32)
{
    std::u16string index16(utf32.size(), '\0');
    for(size_t i=0; i < index16.size(); ++i) {
        const auto it = font.chardata.find(utf32[i]);
        if(it != font.chardata.end()) {
            index16[i] = it->second.AtlasIndex();
        }
    }
    return index16;
}

inline std::u16string to_index_string(pangolin::GlFont& font, const std::string& utf8)
{
    const std::u32string utf32 = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.from_bytes(utf8);
    return to_index_string(font, utf32);
}

inline pangolin::ManagedImage<uint16_t> MakeFontIndexImage(pangolin::GlFont& font, const std::string& utf8)
{
    const std::u32string utf32 = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.from_bytes(utf8);

    pangolin::ManagedImage<uint16_t> img(utf32.size(), 1);

    for(size_t i=0; i < utf32.size(); ++i) {
        const auto& ch = font.chardata[utf32[i]];
        img(i,0) = ch.AtlasIndex();
    }

    return img;
}

template<typename T>
pangolin::GlTexture TextureFromImage(const pangolin::Image<T>& img)
{
    pangolin::GlTexture tex;
    using Fmt = pangolin::GlFormatTraits<T>;
    CheckGlDieOnError();
    tex.Reinitialise(img.w, img.h, Fmt::glinternalformat, false, 0, Fmt::glformat, Fmt::gltype, img.ptr);
    CheckGlDieOnError();
    return tex;
}

}
