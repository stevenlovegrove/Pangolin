#include <fstream>
#include <pangolin/image/typed_image.h>

namespace pangolin {

PixelFormat PpmFormat(const std::string& strType, int num_colours)
{
    if(strType == "P5") {
        if(num_colours < 256) {
            return PixelFormatFromString("GRAY8");
        } else {
            return PixelFormatFromString("GRAY16LE");
        }
    }else if(strType == "P6") {
        return PixelFormatFromString("RGB24");
    }else{
        throw std::runtime_error("Unsupported PPM/PGM format");
    }
}

void PpmConsumeWhitespaceAndComments(std::ifstream& bFile)
{
    // TODO: Make a little more general / more efficient
    while( bFile.peek() == ' ' )  bFile.get();
    while( bFile.peek() == '\n' ) bFile.get();
    while( bFile.peek() == '#' )  bFile.ignore(4096, '\n');
}

TypedImage LoadPpm(std::ifstream& bFile)
{
    // Parse header
    std::string ppm_type = "";
    int num_colors = 0;
    int w = 0;
    int h = 0;

    bFile >> ppm_type;
    PpmConsumeWhitespaceAndComments(bFile);
    bFile >> w;
    PpmConsumeWhitespaceAndComments(bFile);
    bFile >> h;
    PpmConsumeWhitespaceAndComments(bFile);
    bFile >> num_colors;
    bFile.ignore(1,'\n');

    if(!bFile.fail() && w > 0 && h > 0) {
        TypedImage img(w, h, PpmFormat(ppm_type, num_colors) );

        // Read in data
        for(size_t r=0; r<img.h; ++r) {
            bFile.read( (char*)img.ptr + r*img.pitch, img.pitch );
        }
        if(!bFile.fail()) {
            return img;
        }
    }

    throw std::runtime_error("Unable to load PPM file.");
}

TypedImage LoadPpm(const std::string& filename)
{
    std::ifstream bFile( filename.c_str(), std::ios::in | std::ios::binary );
    return LoadPpm(bFile);
}

void SavePpm(const Image<unsigned char>& image, const pangolin::PixelFormat& fmt, const std::string& filename, bool top_line_first)
{
    std::ofstream bFile( filename.c_str(), std::ios::out | std::ios::binary );

    // Setup header variables
    std::string ppm_type = "";
    int num_colors = 0;
    int w = (int)image.w;
    int h = (int)image.h;

    if(fmt.format == "GRAY8") {
        ppm_type = "P5";
        num_colors = 255;
    }else if(fmt.format == "GRAY16LE") {
        ppm_type = "P5";
        num_colors = 65535;
    }else if(fmt.format == "RGB24") {
        ppm_type = "P6";
        num_colors = 255;
    }else{
        throw std::runtime_error("Unsupported pixel format");
    }

    // Write header
    bFile << ppm_type;
    bFile << " ";
    bFile << w;
    bFile << " ";
    bFile << h;
    bFile << " ";
    bFile << num_colors;
    bFile << "\n";

    // Write out data
    if(top_line_first) {
        for(size_t r=0; r<image.h; ++r) {
            bFile.write( (char*)image.ptr + r*image.pitch, image.pitch );
        }
    }else{
        for(size_t r=0; r<image.h; ++r) {
            bFile.write( (char*)image.ptr + (image.h-1-r)*image.pitch, image.pitch );
        }
    }
}

}
