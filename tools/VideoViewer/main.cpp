#include <pangolin/pangolin.h>
#include <pangolin/gl/gltexturecache.h>

struct GlFormat
{
    GlFormat() {}

    GlFormat(const pangolin::VideoPixelFormat& fmt)
    {
        switch( fmt.channels) {
        case 1: glformat = GL_LUMINANCE; break;
        case 3: glformat = GL_RGB; break;
        case 4: glformat = GL_RGBA; break;
        default: throw std::runtime_error("Unable to display video format");
        }

        switch (fmt.channel_bits[0]) {
        case 8: gltype = GL_UNSIGNED_BYTE; break;
        case 16: gltype = GL_UNSIGNED_SHORT; break;
        case 32: gltype = GL_FLOAT; break;
        default: throw std::runtime_error("Unknown channel format");
        }
    }

    GLint glformat;
    GLenum gltype;
};

void RenderToViewport(
    pangolin::Image<unsigned char>& image,
    const GlFormat& fmt, bool flipx=false, bool flipy=false
) {
    pangolin::GlTexture& tex = pangolin::TextureCache::I().GlTex(image.w, image.h, fmt.glformat, fmt.glformat, fmt.gltype);
    tex.Upload(image.ptr,0,0, image.w, image.h, fmt.glformat, fmt.gltype);
    tex.RenderToViewport(pangolin::Viewport(0,0,image.w, image.h), flipx, flipy);
}

void VideoViewer(const std::string& input_uri, const std::string& output_uri)
{
    // Open Video by URI
    pangolin::VideoInput video(input_uri);

    if(video.Streams().size() == 0) {
        pango_print_error("No video streams from device.\n");
        return;
    }

    std::vector<unsigned char> buffer;
    buffer.resize(video.SizeBytes()+1);

    // Video Output device
    pangolin::VideoOutput video_out;

    // Create OpenGL window - guess sensible dimensions
    pangolin::CreateWindowAndBind( "VideoViewer",
        video.Width() * video.Streams().size(), video.Height()
    );

    // Setup resizable views for video streams
    std::vector<GlFormat> glfmt;
    pangolin::DisplayBase().SetLayout(pangolin::LayoutEqual);
    for(unsigned int d=0; d < video.Streams().size(); ++d) {
        pangolin::View& view = pangolin::CreateDisplay().SetAspect(video.Streams()[d].Aspect());
        pangolin::DisplayBase().AddDisplay(view);
        glfmt.push_back(GlFormat(video.Streams()[d].PixFormat()));
    }

    int frame = 0;
    pangolin::Var<int> max_frame("max_frame", std::numeric_limits<int>::max() );

#ifdef CALLEE_HAS_CPP11
    pangolin::RegisterKeyPressCallback('r', [&](){
        if(!video_out.IsOpen()) {
            pango_print_info("Recording...\n");
            video_out.Open(output_uri);
            video_out.AddStreams(video.Streams());
        }else{
            video_out.Close();
            pango_print_info("Finished recording.\n");
        }
        fflush(stdout);
    });
    pangolin::RegisterKeyPressCallback(' ', [&](){
        max_frame = (frame < max_frame) ? frame : std::numeric_limits<int>::max();
    });
    pangolin::RegisterKeyPressCallback(pangolin::PANGO_SPECIAL + pangolin::PANGO_KEY_RIGHT, [&](){
        max_frame = frame+1;
    });
#endif

    std::vector<pangolin::Image<unsigned char> > images;

    // Stream and display video
    while(!pangolin::ShouldQuit())
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glColor3f(1.0f, 1.0f, 1.0f);

        if (frame == 0 || frame < max_frame) {
            images.clear();
            if (video.Grab(&buffer[0], images) ){
                if (video_out.IsOpen()) {
                    video_out.WriteStreams(&buffer[0]);
                }
                ++frame;
            }
        }

        for(unsigned int i=0; i<images.size(); ++i)
        {
            pangolin::DisplayBase()[i].Activate();
            if(glfmt[i].gltype == GL_UNSIGNED_SHORT) {
                pangolin::GlSlUtilities::Scale(20.0f, 0.0f);
                RenderToViewport(images[i], glfmt[i],false,true);
                pangolin::GlSlUtilities::UseNone();
            }else{
                RenderToViewport(images[i], glfmt[i],false,true);
            }
        }

        pangolin::FinishFrame();
    }
}


int main( int argc, char* argv[] )
{
    const std::string dflt_output_uri = "pango://video.pango";

    if( argc > 1 ) {
        const std::string input_uri = std::string(argv[1]);
        const std::string output_uri = (argc > 2) ? std::string(argv[2]) : dflt_output_uri;
        try{
            VideoViewer(input_uri, output_uri);
        } catch (std::exception e) {
            std::cout << e.what() << std::endl;
        }
    }else{
        const std::string input_uris[] = {
            "dc1394:[fps=30,dma=10,size=640x480,iso=400]//0",
            "convert:[fmt=RGB24]//v4l:///dev/video0",
            "convert:[fmt=RGB24]//v4l:///dev/video1",
            "openni:[img1=rgb]//",
            "test:[size=160x120,n=1,fmt=RGB24]//"
            ""
        };

        std::cout << "Usage  : VideoViewer [video-uri]" << std::endl << std::endl;
        std::cout << "Where video-uri describes a stream or file resource, e.g." << std::endl;
        std::cout << "\tfile:[realtime=1]///home/user/video/movie.pvn" << std::endl;
        std::cout << "\tfile:///home/user/video/movie.avi" << std::endl;
        std::cout << "\tfiles:///home/user/seqiemce/foo%03d.jpeg" << std::endl;
        std::cout << "\tdc1394:[fmt=RGB24,size=640x480,fps=30,iso=400,dma=10]//0" << std::endl;
        std::cout << "\tdc1394:[fmt=FORMAT7_1,size=640x480,pos=2+2,iso=400,dma=10]//0" << std::endl;
        std::cout << "\tv4l:///dev/video0" << std::endl;
        std::cout << "\tconvert:[fmt=RGB24]//v4l:///dev/video0" << std::endl;
        std::cout << "\tmjpeg://http://127.0.0.1/?action=stream" << std::endl;
        std::cout << "\topenni:[img1=rgb]//" << std::endl;
        std::cout << std::endl;

        // Try to open some video device
        for(int i=0; !input_uris[i].empty(); ++i )
        {
            try{
                pango_print_info("Trying: %s\n", input_uris[i].c_str());
                VideoViewer(input_uris[i], dflt_output_uri);
                return 0;
            }catch(pangolin::VideoException) { }
        }
    }

    return 0;
}
