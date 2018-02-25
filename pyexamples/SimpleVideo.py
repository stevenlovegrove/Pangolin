import sys
sys.path.append('../build/src')

import sys
import numpy as np
import pypangolin as pango
from PIL import Image

def main(vid_uri):
    vid = pango.VideoInput(vid_uri)

    print("Opened video uri: '{}' with {} x {} dimensions".format(vid_uri,vid.Width(),vid.Height()))

    # Just show first frame for now
    for i in range(0,1):
        arr = vid.Grab()
        Image.fromarray(arr).show()

if __name__ == "__main__":
    vid_uri = 'test://' if (len(sys.argv) == 1) else sys.argv[1]
    main(vid_uri)
