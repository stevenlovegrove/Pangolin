import sys
import os
import argparse
import numpy as np
import time
import json

# If this example doesn't work, it's probably because this path is wrong...
sys.path.append('../build/src')

import pypangolin as pango

def main(flags):
    vid_uri = flags.pango
    vout_uri = flags.pangoOut

    vid = pango.VideoInput(vid_uri)
    vout = pango.VideoOutput(vout_uri) if vout_uri else None

    device_properties = vid.DeviceProperties()

    # print metadata
    print("Opened video uri: '{}' with {} x {} dimensions".format( vid_uri,vid.Width(),vid.Height()))

    # user specified initial frame
    vid.Seek(flags.startFrame)

    # show each frame
    streamsBitDepth = vid.GetStreamsBitDepth()

    for frame in vid:
        if vout:
            vout.WriteStreams(frame, streamsBitDepth, vid.FrameProperties(), device_properties);

        # frame is a list of Images! One per stream
        # process(frame)

        # printing
        sys.stdout.write('\rframe: {} / {}'.format(vid.GetCurrentFrameId(), vid.GetTotalFrames()))

    print('\nDONE')

if __name__ == "__main__":
    # input flags
    parser = argparse.ArgumentParser('Read a .pango file frame by frame. Optionally stream to another video output.')
    parser.add_argument(
        '--pango', type=str,
        help='path to the input pango file.')
    parser.add_argument(
        '--startFrame', type=int, default=0,
        help='index of the start frame (inclusive)')
    parser.add_argument(
        '--pangoOut', type=str, default=None,
        help='path to the output pango file.')

    # main function
    main(parser.parse_args())
