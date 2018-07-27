import sys
from pathlib import Path
import os
from matplotlib import pyplot as plt
import argparse
import numpy as np
import time


# add pangolin to PYTHONPATH
homeDir = str(Path.home())
sys.path.append(os.path.join(homeDir, 'ws/Pangolin/build/src'))

# import pypangolin
import pypangolin as pango

def main(flags):
	vid_uri = flags.pango
	vout_uri = flags.pangoOut
	stream = flags.stream
	if vout_uri is None:
		vout_uri = "pango://demo.pango"

	vid = pango.VideoInput(vid_uri)
	vout = pango.VideoOutput(vout_uri)

    # print metadata
	print("Opened video uri: '{}' with {} x {} dimensions".format(
		vid_uri,vid.Width(),vid.Height()))
	fmt = vid.PixFormat()
	print("format: channels: {}, channel_bits: {}, planar: {}".format(
		fmt.channels, fmt.bpp, fmt.planar))


	# user specified initial frame
	vid.Seek(flags.startFrame)
	if flags.endFrame is None:
		flags.endFrame = vid.GetTotalFrames()

    # initialize display
	numStreams = vid.GetNumStreams()
	streams = [stream] if stream else list(range(numStreams))
	assert streams[-1] < numStreams, 'specified stream {} is out of bnd'.format(stream)

	fig, axes = plt.subplots(len(streams), 1, figsize=(12, 12*len(streams)), squeeze=False)
	fig.show()

    # show each frame
	while (vid.GetCurrentFrameId() < flags.endFrame):
		# grab the next frame
		allStreams = vid.Grab()

		vout.WriteStreams(allStreams);
		for i, s in enumerate(streams):
			arr = allStreams[s]
			# print(arr.shape)
			axes[i,0].cla()
			if arr.shape[-1] == 1:
				axes[i,0].imshow(np.squeeze(arr), cmap='Greys')
			else:
				axes[i,0].imshow(arr)

		# update figures
		fig.canvas.draw()

		# printing
		if vid.GetCurrentFrameId() % 10 == 0:
			print('frame: {} / {}'.format(
				vid.GetCurrentFrameId(), vid.GetTotalFrames()))

if __name__ == "__main__":
    # input flags
    parser = argparse.ArgumentParser(
        'Read a .pango file frame by frame.')
    parser.add_argument(
        '--pango', type=str,
        help='path to the input pango file.')
    parser.add_argument(
        '--startFrame', type=int, default=0,
        help='path to the input pango file.')
    parser.add_argument(
		'--endFrame', type=int, default=None,
		help='path to the input pango file.')
    parser.add_argument(
        '--pangoOut', type=str, default=None,
        help='path to the output pango file.')
    parser.add_argument(
        '--stream', type=int, default=None,
        help='stream to open.')
    FLAGS = parser.parse_args()

    # main function
    main(FLAGS)
