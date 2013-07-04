/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2013 Steven Lovegrove,
 *                    Gabe Sibley
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

#include <pangolin/image_load.h>

#include <algorithm>
#include <stdexcept>

#ifdef HAVE_PNG
#include <png.h>
#endif

namespace pangolin {

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

ImageFileType FileType(const unsigned char data[], size_t bytes)
{
    // Check we wont go over bounds when comparing.
    if(bytes >= 8) {
        const unsigned char magic_png[]  = "\211PNG\r\n\032\n";
        const unsigned char magic_jpg1[] = "\xFF\xD8";
        const unsigned char magic_jpg2[] = "\xFF\xD9";
        const unsigned char magic_gif1[] = "GIF87a";
        const unsigned char magic_gif2[] = "GIF89a";
        
        if( !strncmp((char*)data, (char*)magic_png, 8) ) {
            return ImageFileTypePng;
        }else if( !strncmp( (char*)data, (char*)magic_jpg1, 2)
                  || !strncmp( (char*)data, (char*)magic_jpg2, 2) ) {
            return ImageFileTypeJpg;
        }else if( !strncmp((char*)data, (char*)magic_gif1,6)
                  || !strncmp((char*)data, (char*)magic_gif2,6) ) {
            return ImageFileTypeGif;
        }
    }
    
    return ImageFileTypeUnknown;
}

ImageFileType FileType(const std::string& filename)
{
    // Check magic number of file...    
    FILE *fp = fopen(filename.c_str(), "rb");
    const size_t magic_bytes = 8;    
    unsigned char magic[magic_bytes];    
    size_t num_read = fread( (void *)magic, 1, magic_bytes, fp);
    ImageFileType magic_type = FileType(magic, num_read);
    fclose(fp);
    if(magic_type != ImageFileTypeUnknown) {
        return magic_type;
    }    

    // Fallback on using extension...
    const std::string ext = FileLowercaseExtention(filename);
    if( ext == ".png" ) {
        return ImageFileTypePng;
    } else if( ext == ".tga" || ext == ".targa") {
        return ImageFileTypeTga;
    } else if( ext == ".jpg" || ext == ".jpeg" ) {
        return ImageFileTypeJpg;
    } else if( ext == ".gif" ) {
        return ImageFileTypeGif;
    } else {
        return ImageFileTypeUnknown;
    }    
}

VideoPixelFormat TgaFormat(int depth, int color_type, int color_map)
{
    if(color_map == 0) {
        if(color_type == 2) {
            // Colour
            if(depth == 24) {
                return VideoFormatFromString("RGB24");
            }else if(depth == 32) {
                return VideoFormatFromString("RGBA");
            }
        }else if(color_type == 3){
            // Greyscale
            if(depth == 8) {
                return VideoFormatFromString("GRAY8");
            }else if(depth == 16) {
                return VideoFormatFromString("Y400A");
            }
        }
    }
    throw std::runtime_error("Unsupported TGA format");    
}

TypedImage LoadTga(const std::string& filename)
{
    FILE *file;
    unsigned char type[4];
    unsigned char info[6];

    file = fopen(filename.c_str(), "rb");

    if(file) {
        fread( &type, sizeof (char), 3, file );
        fseek( file, 12, SEEK_SET );
        fread( &info, sizeof (char), 6, file );
        
        const int width  = info[0] + (info[1] * 256);
        const int height = info[2] + (info[3] * 256);
        
        TypedImage img;
        img.fmt = TgaFormat(info[4], type[2], type[1]);
        img.Alloc(width, height, width*img.fmt.bpp / 8);
            
        //read in image data
        fread(img.ptr, sizeof(unsigned char), img.w * img.pitch, file);
        fclose(file);
        
        return img;
    }

    throw std::runtime_error("Unable to load TGA file, '" + filename + "'");    
}

#ifdef HAVE_PNG
VideoPixelFormat PngFormat(png_structp png_ptr, png_infop info_ptr )
{
    png_byte colour = png_get_color_type(png_ptr, info_ptr);
    
    if( colour & PNG_COLOR_MASK_COLOR) {
        if( colour & PNG_COLOR_MASK_ALPHA) {
            return VideoFormatFromString("RGBA");
        } else {
            return VideoFormatFromString("RGB24");
        }
    } else {
        if( colour & PNG_COLOR_MASK_ALPHA) {
            return VideoFormatFromString("Y400A");
        } else {
            return VideoFormatFromString("GRAY8");
        }
    }
    throw std::runtime_error("Unsupported PNG format");
}
#endif

TypedImage LoadPng(const std::string& filename)
{
#ifdef HAVE_PNG
    FILE *in = fopen(filename.c_str(), "rb");

    if( in )  {
        //check the header
        int nBytes = 8;
        png_byte header[nBytes];
        fread(header, 1, nBytes, in);
        int nIsPNG = png_sig_cmp(header, 0, nBytes);
    
        if ( nIsPNG != 0 )  {
            throw std::runtime_error( filename + " is not a PNG file" );
        }
    
        //set up initial png structs
        png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);
        if (!png_ptr) {
            throw std::runtime_error( "PNG Init error 1" );
        }
    
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)  {
            png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
            throw std::runtime_error( "PNG Init error 2" );
        }
    
        png_infop end_info = png_create_info_struct(png_ptr);
        if (!end_info) {
            png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
            throw std::runtime_error( "PNG Init error 3" );
        }
    
        png_init_io(png_ptr, in);
        png_set_sig_bytes(png_ptr, nBytes);
    
        //read the file
        png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

        if( png_get_bit_depth(png_ptr, info_ptr) == 1)  {
            //Unpack bools to bytes to ease loading.
            png_set_packing(png_ptr);
        } else if( png_get_bit_depth(png_ptr, info_ptr) < 8) {
            //Expand nonbool colour depths up to 8bpp
            png_set_expand_gray_1_2_4_to_8(png_ptr);
        }
                
        //Get rid of palette, by transforming it to RGB
        if(png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_PALETTE) {
            png_set_palette_to_rgb(png_ptr);
        }
    
        if( png_get_interlace_type(png_ptr,info_ptr) != PNG_INTERLACE_NONE) {
            throw std::runtime_error( "Interlace not yet supported" );
        }
        
        const size_t w = png_get_image_width(png_ptr,info_ptr);
        const size_t h = png_get_image_height(png_ptr,info_ptr);
        const size_t pitch = png_get_rowbytes(png_ptr, info_ptr);
    
        TypedImage img;
        img.fmt = PngFormat(png_ptr, info_ptr);
        img.Alloc( w, h, pitch );
        
        png_bytepp rows = png_get_rows(png_ptr, info_ptr);
        for( unsigned int r = 0; r < h; r++) {
            memcpy( img.ptr + pitch*r, rows[r], pitch );
        }
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    
        fclose(in);
        return img;
    }

    throw std::runtime_error("Unable to load PNG file, '" + filename + "'");    

#else
    throw std::runtime_error("PNG Support not enabled. Please rebuild Pangolin.");
#endif
}

TypedImage LoadJpg(const std::string& filename)
{
    throw std::runtime_error("JPEG Support not enabled. Please rebuild Pangolin.");
}

TypedImage LoadGif(const std::string& filename)
{
    throw std::runtime_error("GIF Support not enabled. Please rebuild Pangolin.");
}

TypedImage LoadImage(const std::string& filename, ImageFileType file_type)
{
    switch (file_type) {
    case ImageFileTypeTga:
        return LoadTga(filename);
    case ImageFileTypePng:
        return LoadPng(filename);
    case ImageFileTypeJpg:
        return LoadJpg(filename);
    case ImageFileTypeGif:
        return LoadGif(filename);
    default:
        throw std::runtime_error("Unsupported image file type, '" + filename + "'");
    }
}

TypedImage LoadImage(const std::string& filename)
{
    ImageFileType file_type = FileType(filename);
    return LoadImage( filename, file_type );
}

void FreeImage(TypedImage img)
{
    img.Dealloc();
}

}
