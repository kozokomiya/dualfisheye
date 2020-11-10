#!/bin/bash


# force cache clear
rm -rf ~/.cache/gstreamer-1.0

# set ENV
#export GST_PLUGIN_PATH=".:${GST_PLUGIN_PATH}"
#export GST_DEBUG="1,dualfisheye:6"
# DEBUG_LEVEL  1=ERROR, 2=WARNING, 3=FIXME, 4=INFO, 5=DEBUG, 6=LOG, 7=TRACE


# Launch dualfisheye test
gst-launch-1.0 -v \
  v4l2src device=/dev/video0 ! \
  video/x-raw,width=640,height=480,framerate=30/1 ! \
  videoconvert ! \
  omxh264enc ! \
  rtph264pay pt=96 ! \
  udpsink host=192.168.11.26 port=9001

