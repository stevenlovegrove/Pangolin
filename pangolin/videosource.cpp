#include "videosource.h"
#ifdef HAVE_DC1394

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

using namespace std;

void FirewireVideo::init_camera(
  uint64_t guid, int dma_frames,
  dc1394speed_t iso_speed,
  dc1394video_mode_t video_mode,
  dc1394framerate_t framerate
) {
  camera = dc1394_camera_new (d, guid);
  if (!camera)
    throw VideoException("Failed to initialize camera");

  dc1394_camera_free_list (list);

  cout << "Using camera with GUID " << camera->guid << endl;

  /*-----------------------------------------------------------------------
   *  setup capture
   *-----------------------------------------------------------------------*/

  err=dc1394_video_set_iso_speed(camera, iso_speed);
  if( err != DC1394_SUCCESS )
    throw VideoException("Could not set iso speed");

  err=dc1394_video_set_mode(camera, video_mode);
  if( err != DC1394_SUCCESS )
    throw VideoException("Could not set video mode");

  err=dc1394_video_set_framerate(camera, framerate);
  if( err != DC1394_SUCCESS )
    throw VideoException("Could not set framerate");

  err=dc1394_capture_setup(camera,dma_frames, DC1394_CAPTURE_FLAGS_DEFAULT);
  if( err != DC1394_SUCCESS )
    throw VideoException("Could not setup camera - check settings");

  /*-----------------------------------------------------------------------
   *  initialise width and height from mode
   *-----------------------------------------------------------------------*/
  dc1394_get_image_size_from_video_mode(camera, video_mode, &width, &height);

  Start();
}

void FirewireVideo::Start()
{
  if( !running )
  {
    err=dc1394_video_set_transmission(camera, DC1394_ON);
    if( err != DC1394_SUCCESS )
      throw VideoException("Could not start camera iso transmission");
    running = true;
  }
}

void FirewireVideo::Stop()
{
  if( running )
  {
    // Stop transmission
    err=dc1394_video_set_transmission(camera,DC1394_OFF);
    if( err != DC1394_SUCCESS )
      throw VideoException("Could not stop the camera");
    running = false;
  }
}

FirewireVideo::FirewireVideo(uint64_t guid)
  :running(false)
{
  d = dc1394_new ();
  if (!d)
    throw VideoException("");

  init_camera(guid,50,DC1394_ISO_SPEED_400,DC1394_VIDEO_MODE_640x480_RGB8,DC1394_FRAMERATE_30);
}

FirewireVideo::FirewireVideo(unsigned deviceid)
  :running(false)
{
  d = dc1394_new ();
  if (!d)
    throw VideoException("");

  err=dc1394_camera_enumerate (d, &list);
  if( err != DC1394_SUCCESS )
    throw VideoException("Failed to enumerate cameras");

  if (list->num == 0)
    throw VideoException("No cameras found");

  if( deviceid >= list->num )
    throw VideoException("Invalid camera index");

  const uint64_t guid = list->ids[deviceid].guid;
  init_camera(guid,50,DC1394_ISO_SPEED_400,DC1394_VIDEO_MODE_640x480_RGB8,DC1394_FRAMERATE_30);
}

void FirewireVideo::GrabNextWait( unsigned char* image )
{
  dc1394video_frame_t *frame;
  err=dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame);
  if( err != DC1394_SUCCESS )
    throw VideoException("Could not dequeue a frame");

  memcpy(image,frame->image,frame->image_bytes);

  err=dc1394_capture_enqueue(camera,frame);
  if( err != DC1394_SUCCESS )
    throw VideoException("Could not enqueue a frame");
}

bool FirewireVideo::GrabNextPoll( unsigned char* image )
{
  dc1394video_frame_t *frame;
  err=dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_POLL, &frame);
  if( frame )
  {
    if( err != DC1394_SUCCESS )
      throw VideoException("Could not dequeue a frame");

    memcpy(image,frame->image,frame->image_bytes);

    err=dc1394_capture_enqueue(camera,frame);
    if( err != DC1394_SUCCESS )
      throw VideoException("Could not enqueue a frame");
    return true;
  }
  return false;
}

bool FirewireVideo::GrabNewestPoll( unsigned char* image )
{
  dc1394video_frame_t *f;
  dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_POLL, &f);

  if( f )
  {
    while( true )
    {
      dc1394video_frame_t *nf;
      dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_POLL, &nf);
      if( nf )
      {
        err=dc1394_capture_enqueue(camera,f);
        f = nf;
      }else{
        break;
      }
    }
    memcpy(image,f->image,f->image_bytes);
    err=dc1394_capture_enqueue(camera,f);
    return true;
  }

  return false;
}

void FirewireVideo::GrabNewestWait( unsigned char* image )
{
  if( !GrabNewestPoll(image) )
    GrabNextWait(image);
}

FirewireVideo::~FirewireVideo()
{
  Stop();

  // Close camera
  dc1394_video_set_transmission(camera, DC1394_OFF);
  dc1394_capture_stop(camera);
  dc1394_camera_free(camera);
  dc1394_free (d);
}

#endif
