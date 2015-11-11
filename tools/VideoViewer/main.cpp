#include <pangolin/pangolin.h>
#include <pangolin/video/video_record_repeat.h>
#include <pangolin/gl/gltexturecache.h>
#include <pangolin/gl/glpixformat.h>
#include <pangolin/handler/handler_image.h>
#include <pangolin/utils/file_utils.h>

template<typename T>
std::pair<float,float> GetOffsetScale(const pangolin::Image<T>& img, float type_max, float format_max)
{
    std::pair<float,float> mm(std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

    for(size_t y=0; y < img.h; ++y) {
        T* pix = (T*)((char*)img.ptr + y*img.pitch);
        for(size_t x=0; x < img.w; ++x) {
            const T val = *(pix++);
            if(val != 0) {
                if(val < mm.first) mm.first = val;
                if(val > mm.second) mm.second = val;
            }
        }
    }

    const float type_scale = format_max / type_max;
    const float offset = -type_scale* mm.first;
    const float scale = type_max / (mm.second - mm.first);
    return std::pair<float,float>(offset, scale);
}

template<typename T>
pangolin::Image<T> ImageRoi( pangolin::Image<T> img, const pangolin::XYRangei& roi )
{
    const int xmin = std::min(roi.x.min,roi.x.max);
    const int ymin = std::min(roi.y.min,roi.y.max);
    return pangolin::Image<T>(
        roi.x.AbsSize(), roi.y.AbsSize(),
        img.pitch, img.RowPtr(ymin) + xmin
    );
}

void VideoViewer(const std::string& input_uri, const std::string& output_uri)
{
    int frame = 0;
    pangolin::Var<int>  end_frame("viewer.end_frame", std::numeric_limits<int>::max() );

    // Open Video by URI
    pangolin::VideoRecordRepeat video(input_uri, output_uri);
    const size_t num_streams = video.Streams().size();
    int total_frames = std::numeric_limits<int>::max();

    if(num_streams == 0) {
        pango_print_error("No video streams from device.\n");
        return;
    }

    // Check if video supports VideoPlaybackInterface
    pangolin::VideoPlaybackInterface* video_playback = video.Cast<pangolin::VideoPlaybackInterface>();
    if( video_playback ) {
        total_frames = video_playback->GetTotalFrames();
        if(total_frames < std::numeric_limits<int>::max() ) {
            std::cout << "Video length: " << total_frames << " frames" << std::endl;
            end_frame = 1;
        }
    }

    std::vector<unsigned char> buffer;
    buffer.resize(video.SizeBytes()+1);

    // Create OpenGL window - guess sensible dimensions
    pangolin::CreateWindowAndBind( "VideoViewer",
        video.Width() * num_streams, video.Height()
    );

    // Assume packed OpenGL data unless otherwise specified
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // Setup resizable views for video streams
    std::vector<pangolin::GlPixFormat> glfmt;
    std::vector<std::pair<float,float> > gloffsetscale;
    std::vector<pangolin::ImageViewHandler> handlers;
    handlers.reserve(num_streams);

    pangolin::View& container = pangolin::Display("streams");
    container.SetLayout(pangolin::LayoutEqual);
    for(unsigned int d=0; d < num_streams; ++d) {
        const pangolin::StreamInfo& si = video.Streams()[d];
        pangolin::View& view = pangolin::CreateDisplay().SetAspect(si.Aspect());
        container.AddDisplay(view);
        glfmt.push_back(pangolin::GlPixFormat(si.PixFormat()));
        gloffsetscale.push_back(std::pair<float,float>(0.0f, 1.0f) );
        handlers.push_back( pangolin::ImageViewHandler(si.Width(), si.Height()) );
        view.SetHandler(&handlers.back());
    }

    std::vector<pangolin::Image<unsigned char> > images;

#ifdef CALLEE_HAS_CPP11
    const int FRAME_SKIP = 30;
    const char show_hide_keys[]  = {'1','2','3','4','5','6','7','8','9'};
    const char screenshot_keys[] = {'!','"','#','$','%','^','&','*','('};

    // Show/hide streams
    for(size_t v=0; v < container.NumChildren() && v < 9; v++) {
        pangolin::RegisterKeyPressCallback(show_hide_keys[v], [v,&container](){
            container[v].ToggleShow();
        } );
        pangolin::RegisterKeyPressCallback(screenshot_keys[v], [v,&images,&video](){
            if(v < images.size() && images[v].ptr) {
                try{
                    pangolin::SaveImage(
                        images[v], video.Streams()[v].PixFormat(),
                        pangolin::MakeUniqueFilename("capture.png")
                    );
                }catch(std::exception e){
                    pango_print_error("Unable to save frame: %s\n", e.what());
                }
            }
        } );
    }

    pangolin::RegisterKeyPressCallback('r', [&](){
        if(!video.IsRecording()) {
            video.Record();
            pango_print_info("Started Recording.\n");
        }else{
            video.Stop();
            pango_print_info("Finished recording.\n");
        }
        fflush(stdout);
    });
    pangolin::RegisterKeyPressCallback('p', [&](){
        video.Play();
        end_frame = std::numeric_limits<int>::max();
        pango_print_info("Playing from file log.\n");
        fflush(stdout);
    });
    pangolin::RegisterKeyPressCallback('s', [&](){
        video.Source();
        end_frame = std::numeric_limits<int>::max();
        pango_print_info("Playing from source input.\n");
        fflush(stdout);
    });
    pangolin::RegisterKeyPressCallback(' ', [&](){
        end_frame = (frame < end_frame) ? frame : std::numeric_limits<int>::max();
    });
    pangolin::RegisterKeyPressCallback(',', [&](){
        if(video_playback) {
            const int frame = std::min(video_playback->GetCurrentFrameId()-FRAME_SKIP, video_playback->GetTotalFrames()-1);
            video_playback->Seek(frame);
        }else{
            // We can't go backwards
        }
    });
    pangolin::RegisterKeyPressCallback('.', [&](){
        if(video_playback) {
            const int frame = std::max(video_playback->GetCurrentFrameId()+FRAME_SKIP, 0);
            video_playback->Seek(frame);
        }else{
            // Pause at this frame
            end_frame = frame+1;
        }
    });

    pangolin::RegisterKeyPressCallback('a', [&](){
        // Adapt scale
        for(unsigned int i=0; i<images.size(); ++i) {
            pangolin::Image<unsigned char>& img = images[i];
            pangolin::ImageViewHandler& ivh = handlers[i];

            // round to pixels, clamp to image border.
            const bool have_selection = std::isfinite(ivh.GetSelection().Area()) && std::abs(ivh.GetSelection().Area()) >= 4;
            pangolin::XYRangef froi = have_selection ? ivh.GetSelection() : ivh.GetViewToRender();
            pangolin::XYRangei iroi = froi.Cast<int>();
            iroi.Clamp(0, images[i].w-1, 0, img.h-1 );

            if(container[i].HasFocus()) {
                std::pair<float,float> os(0.0f, 1.0f);
                if(glfmt[i].gltype == GL_UNSIGNED_BYTE) {
                    os = GetOffsetScale(ImageRoi(img.Reinterpret<unsigned char>(),iroi), 255.0f, 1.0f);
                }else if(glfmt[i].gltype == GL_UNSIGNED_SHORT) {
                    os = GetOffsetScale(ImageRoi(img.Reinterpret<unsigned short>(),iroi), 65535.0f, 1.0f);
                }else if(glfmt[i].gltype == GL_FLOAT) {
                    os = GetOffsetScale(ImageRoi(img.Reinterpret<float>(),iroi), 1.0f, 1.0f);
                }
                gloffsetscale[i] = os;
            }
        }
    });

    pangolin::RegisterKeyPressCallback('g', [&](){

        std::pair<float,float> os_default(0.0f, 1.0f);

        //get the scale and offset from the container that has focus.
        for(unsigned int i=0; i<images.size(); ++i) {
            pangolin::Image<unsigned char>& img = images[i];
            pangolin::ImageViewHandler& ivh = handlers[i];

            // round to pixels, clamp to image border.
            const bool have_selection = std::isfinite(ivh.GetSelection().Area()) && std::abs(ivh.GetSelection().Area()) >= 4;
            pangolin::XYRangef froi = have_selection ? ivh.GetSelection() : ivh.GetViewToRender();
            pangolin::XYRangei iroi = froi.Cast<int>();
            iroi.Clamp(0, images[i].w-1, 0, img.h-1 );

            if(container[i].HasFocus()) {
                pangolin::Image<unsigned char>& img = images[i];
                pangolin::ImageViewHandler& ivh = handlers[i];

                // round to pixels, clamp to image border.
                const bool have_selection = std::isfinite(ivh.GetSelection().Area()) && std::abs(ivh.GetSelection().Area()) >= 4;
                pangolin::XYRangef froi = have_selection ? ivh.GetSelection() : ivh.GetViewToRender();
                pangolin::XYRangei iroi = froi.Cast<int>();
                iroi.Clamp(0, images[i].w-1, 0, img.h-1 );

                if(glfmt[i].gltype == GL_UNSIGNED_BYTE) {
                    os_default = GetOffsetScale(ImageRoi(img.Reinterpret<unsigned char>(),iroi), 255.0f, 1.0f);
                }else if(glfmt[i].gltype == GL_UNSIGNED_SHORT) {
                    os_default = GetOffsetScale(ImageRoi(img.Reinterpret<unsigned short>(),iroi), 65535.0f, 1.0f);
                }else if(glfmt[i].gltype == GL_FLOAT) {
                    os_default = GetOffsetScale(ImageRoi(img.Reinterpret<float>(),iroi), 1.0f, 1.0f);
                }

            }
        }

        //Adapt scale for all images equally
        //TODO : we're assuming the type of all the containers images' are the same.
        for(unsigned int i=0; i<images.size(); ++i) {
                gloffsetscale[i] = os_default;
        }

    });

#endif


    // Stream and display video
    while(!pangolin::ShouldQuit())
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glColor3f(1.0f, 1.0f, 1.0f);

        if (frame == 0 || frame < end_frame) {
            if (video.Grab(&buffer[0], images) ){
                ++frame;
            }
        }

        // Setup to render in normalised coordinates.
        glMatrixMode(GL_PROJECTION); glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);  glLoadIdentity();
        glEnableClientState(GL_VERTEX_ARRAY);
        glLineWidth(1.5f);
        glDisable(GL_DEPTH_TEST);

        for(unsigned int i=0; i<images.size(); ++i)
        {
            if(container[i].IsShown()) {
                container[i].Activate();
                pangolin::Image<unsigned char>& image = images[i];
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();

                handlers[i].UpdateView();
                const pangolin::XYRangef& xy = handlers[i].GetViewToRender();
                glOrtho(xy.x.min, xy.x.max, xy.y.min, xy.y.max, -1, 1);

                const GLfloat l = xy.x.min; const GLfloat r = xy.x.max;
                const GLfloat b = xy.y.min; const GLfloat t = xy.y.max;
                const GLfloat sq_vert[]  = { l,t,  r,t,  r,b,  l,b };
                const pangolin::GlPixFormat& fmt = glfmt[i];

                const std::pair<float,float> os = gloffsetscale[i];
                pangolin::GlTexture& tex = pangolin::TextureCache::I().GlTex(image.w, image.h, fmt.scalable_internal_format, fmt.glformat, fmt.gltype);
                tex.Bind();
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, handlers[i].UseNN() ? GL_NEAREST : GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, handlers[i].UseNN() ? GL_NEAREST : GL_LINEAR);
                tex.Upload(image.ptr,0,0, image.w, image.h, fmt.glformat, fmt.gltype);

                const GLfloat ln = l / (float)(image.w);
                const GLfloat bn = b / (float)(image.h);
                const GLfloat rn = r / (float)(image.w);
                const GLfloat tn = t / (float)(image.h);
                const GLfloat sq_tex[]  = { ln,tn,  rn,tn,  rn,bn,  ln,bn };

                pangolin::GlSlUtilities::OffsetAndScale(os.first, os.second);
                glEnable(GL_TEXTURE_2D);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(2, GL_FLOAT, 0, sq_tex);
                glVertexPointer(2, GL_FLOAT, 0, sq_vert);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
                glDisable(GL_TEXTURE_2D);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                pangolin::GlSlUtilities::UseNone();

                const pangolin::XYRangef& selxy = handlers[i].GetSelection();
                const GLfloat sq_select[] = {
                    selxy.x.min, selxy.y.min,
                    selxy.x.max, selxy.y.min,
                    selxy.x.max, selxy.y.max,
                    selxy.x.min, selxy.y.max
                };
                glColor4f(1.0,0.0,0.0,1.0);
                glVertexPointer(2, GL_FLOAT, 0, sq_select);
                glDrawArrays(GL_LINE_LOOP, 0, 4);
            }
        }

        glMatrixMode(GL_MODELVIEW);
        glDisableClientState(GL_VERTEX_ARRAY);

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
        } catch (pangolin::VideoException e) {
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
