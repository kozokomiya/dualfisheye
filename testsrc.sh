#!/bin/bash

export GST_PLUGIN_PATH="./build:${GST_PLUGIN_PATH}"
export GST_DEBUG="1,dualfisheye:4"
# DEBUG_LEVEL  1=ERROR, 2=WARNING, 3=FIXME, 4=INFO, 5=DEBUG, 6=LOG, 7=TRACE

# force cache clear
rm -rf ~/.cache/gstreamer-1.0

# Launch dualfisheye conversion
gst-launch-1.0 -v \
  videotestsrc !\
  "video/x-raw,format=RGBx,width=1280,height=720,framerate=15/1" !\
  videoconvert !\
  dualfisheye !\
  videoconvert !\
  autovideosink
