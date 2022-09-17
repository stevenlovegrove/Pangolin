/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2013 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <pangolin/utils/file_extension.h>

#include <algorithm>
#include <fstream>
#include <string.h>

namespace pangolin
{

std::string ImageFileTypeToName(ImageFileType t)
{
    switch(t)
    {
    case ImageFileTypePpm:
        return "ppm";
    case ImageFileTypeTga:
        return "tga";
    case ImageFileTypePng:
        return "png";
    case ImageFileTypeJpg:
        return "jpg";
    case ImageFileTypeTiff:
        return "tiff";
    case ImageFileTypeGif:
        return "gif";
    case ImageFileTypeExr:
        return "exr";
    case ImageFileTypeBmp:
        return "bmp";
    case ImageFileTypePango:
        return "pango";
    case ImageFileTypePvn:
        return "pvn";
    case ImageFileTypeVrs:
        return "vrs";
    case ImageFileTypePly:
        return "ply";
    case ImageFileTypeObj:
        return "obj";
    case ImageFileTypeArw:
        return "arw";
    case ImageFileTypeUnknown:
    default:
        return "unknown";
    }
}

ImageFileType NameToImageFileType(const std::string& name)
{
    if ("ppm" == name)
        return ImageFileTypePpm;
    else if ("pgm" == name)
        return ImageFileTypePpm;
    else if ("tga" == name)
        return ImageFileTypeTga;
    else if ("png" == name)
        return ImageFileTypePng;
    else if ("jpg" == name)
        return ImageFileTypeJpg;
    else if ("jpeg" == name)
        return ImageFileTypeJpg;
    else if ("tiff" == name)
        return ImageFileTypeTiff;
    else if ("gif" == name)
        return ImageFileTypeGif;
    else if ("exr" == name)
        return ImageFileTypeExr;
    else if ("bmp" == name)
        return ImageFileTypeBmp;
    else if ("pango" == name)
        return ImageFileTypePango;
    else if ("pvn" == name)
        return ImageFileTypePvn;
    else if ("vrs" == name)
        return ImageFileTypeVrs;
    else if ("zstd" == name)
        return ImageFileTypeZstd;
    else if ("lzf" == name)
        return ImageFileTypeLz4;
    else if ("p12b" == name)
        return ImageFileTypeP12b;
    else if ("ply" == name)
        return ImageFileTypePly;
    else if ("obj" == name)
        return ImageFileTypeObj;
    else if ("arw" == name)
        return ImageFileTypeArw;

    return ImageFileTypeUnknown;
}

ImageFileType FileTypeExtension(const std::string& ext)
{
    if( ext == ".png" ) {
        return ImageFileTypePng;
    } else if( ext == ".tga" || ext == ".targa") {
        return ImageFileTypeTga;
    } else if( ext == ".jpg" || ext == ".jpeg" ) {
        return ImageFileTypeJpg;
    } else if( ext == ".gif" ) {
        return ImageFileTypeGif;
    } else if( ext == ".tif" || ext == ".tiff" ) {
        return ImageFileTypeTiff;
    } else if( ext == ".exr"  ) {
        return ImageFileTypeExr;
    } else if( ext == ".bmp"  ) {
        return ImageFileTypeBmp;
    } else if( ext == ".ppm" || ext == ".pgm" || ext == ".pbm" || ext == ".pxm" || ext == ".pdm" ) {
        return ImageFileTypePpm;
    } else if( ext == ".pvn"  ) {
        return ImageFileTypePvn;
    } else if( ext == ".vrs"  ) {
        return ImageFileTypeVrs;
    } else if( ext == ".pango"  ) {
        return ImageFileTypePango;
    } else if( ext == ".zstd"  ) {
        return ImageFileTypeZstd;
    } else if( ext == ".lzf"  ) {
        return ImageFileTypeLz4;
    } else if( ext == ".p12b"  ) {
        return ImageFileTypeP12b;
    } else if( ext == ".ply"  ) {
        return ImageFileTypePly;
    } else if( ext == ".obj"  ) {
        return ImageFileTypeObj;
    } else if( ext == ".ARW"  ) {
        return ImageFileTypeArw;
    } else {
        return ImageFileTypeUnknown;
    }
}

std::string FileLowercaseExtention(const std::string& filename)
{
    size_t pos = filename.find_last_of('.');
    if(pos != std::string::npos) {
        std::string ext = filename.substr(pos);
        std::transform( ext.begin(), ext.end(), ext.begin(), tolower );
        return ext;
    }else{
        return "";
    }
}

ImageFileType FileTypeMagic(const unsigned char data[], size_t bytes)
{
    // Check we wont go over bounds when comparing.
    if(bytes >= 8) {
        const unsigned char magic_png[]  = "\211PNG\r\n\032\n";
        const unsigned char magic_jpg1[] = "\xFF\xD8";
        const unsigned char magic_jpg2[] = "\xFF\xD9";
        const unsigned char magic_gif1[] = "GIF87a";
        const unsigned char magic_gif2[] = "GIF89a";
        const unsigned char magic_tiff1[] = "\x49\x49\x2A\x00";
        const unsigned char magic_tiff2[] = "\x4D\x4D\x00\x2A";
        const unsigned char magic_exr[]   = "\x76\x2F\x31\x01";
        const unsigned char magic_bmp[]   = "BM";
        const unsigned char magic_pango[] = "PANGO";
        const unsigned char magic_pango_zstd[] = "ZSTD";
        const unsigned char magic_pango_lz4[] = "LZ4";
        const unsigned char magic_pango_p12b[] = "P12B";
        const unsigned char magic_vrs[] = "VisionR";
        const unsigned char magic_ply[]   = "ply";

        if( !strncmp((char*)data, (char*)magic_png, 8) ) {
            return ImageFileTypePng;
        }else if( !strncmp( (char*)data, (char*)magic_jpg1, 2)
                  || !strncmp( (char*)data, (char*)magic_jpg2, 2) ) {
            return ImageFileTypeJpg;
        }else if( !strncmp((char*)data, (char*)magic_gif1,6)
                  || !strncmp((char*)data, (char*)magic_gif2,6) ) {
            return ImageFileTypeGif;
        }else if( !strncmp((char*)data, (char*)magic_tiff1,4)
                  || !strncmp((char*)data, (char*)magic_tiff2,4) ) {
            return ImageFileTypeTiff;
        }else if( !strncmp((char*)data, (char*)magic_exr,4) ) {
            return ImageFileTypeExr;
        }else if( !strncmp((char*)data, (char*)magic_bmp,2) ) {
            return ImageFileTypeBmp;
        }else if( !strncmp((char*)data, (char*)magic_pango,5) ) {
            return ImageFileTypePango;
        }else if( !strncmp((char*)data, (char*)magic_vrs,7) ) {
            return ImageFileTypeVrs;
        }else if( !strncmp((char*)data, (char*)magic_pango_zstd,4) ) {
            return ImageFileTypeZstd;
        }else if( !strncmp((char*)data, (char*)magic_pango_lz4,3) ) {
            return ImageFileTypeLz4;
        }else if( !strncmp((char*)data, (char*)magic_pango_p12b,4) ) {
            return ImageFileTypeP12b;
        }else if( !strncmp((char*)data, (char*)magic_ply, 3) ) {
            return ImageFileTypePly;
        }else if( data[0] == 'P' && '0' < data[1] && data[1] < '9') {
            return ImageFileTypePpm;
        }
    }
    return ImageFileTypeUnknown;
}

ImageFileType FileType(const std::string& filename)
{
    // file extension incase we need that as a hint
    const std::string ext = FileLowercaseExtention(filename);

    // Check magic number of file...
    std::ifstream f(filename.c_str(), std::ios::binary );
    if(f.is_open()) {
        const size_t magic_bytes = 8;
        unsigned char magic[magic_bytes];
        f.read((char*)magic, magic_bytes);
        if(f.good()) {
            ImageFileType magic_type = FileTypeMagic(magic, magic_bytes);
            if(magic_type != ImageFileTypeUnknown) {
                if(magic_type == ImageFileTypeTiff && ext == ".arw") {
                    // Special case where we use the extension as well as magic string
                    magic_type = ImageFileTypeArw;
                }

                return magic_type;
            }
        }
    }

    // Fallback on using extension...
    return FileTypeExtension(ext);
}

}
