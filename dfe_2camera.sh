#!/bin/bash


# force cache clear
rm -rf ~/.cache/gstreamer-1.0

# set ENV
export GST_PLUGIN_PATH=".:${GST_PLUGIN_PATH}"
export GST_DEBUG="1,dualfisheye:6"
# DEBUG_LEVEL  1=ERROR, 2=WARNING, 3=FIXME, 4=INFO, 5=DEBUG, 6=LOG, 7=TRACE


# Launch dualfisheye test
gst-launch-1.0 -v \
  videomixer background=1 name=mix \
    sink_0::xpos=0   sink_0::ypos=0 sink_0::zorder=0 \
    sink_1::xpos=640 sink_1::ypos=0 sink_1::zorder=1 ! \
  videoconvert ! \
  dualfisheye ! \
  videoconvert ! \
  omxh264enc ! \
  rtph264pay pt=96 ! \
  udpsink host=192.168.11.26 port=9001 \
  v4l2src device=/dev/video0 ! \
  videoscale ! \
  video/x-raw,width=960,height=720,framerate=15/1 ! \
  videocrop right=320 ! \
  mix.sink_0 \
  v4l2src device=/dev/video2 ! \
  videorate ! \
  image/jpeg,width=1280,height=720,framerate=15/1 ! \
  omxmjpegdec ! \
  videocrop left=640 ! \
  mix.sink_1
