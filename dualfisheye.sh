#!/bin/bash


# force cache clear
rm -rf ~/.cache/gstreamer-1.0

# set ENV
export GST_PLUGIN_PATH=".:${GST_PLUGIN_PATH}"
export GST_DEBUG="1,dualfisheye:6"
# DEBUG_LEVEL  1=ERROR, 2=WARNING, 3=FIXME, 4=INFO, 5=DEBUG, 6=LOG, 7=TRACE


# Launch dualfisheye test
gst-launch-1.0 -v \
  udpsrc port=18011 ! \
  "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264" ! \
  rtph264depay ! \
  avdec_h264 ! \
  videoconvert ! \
  queue ! \
  vp8enc ! \
  rtpvp8pay pt=96 ! \
  udpsink host=10.0.1.11 port=8013

  #openh264dec ! \
  #dualfisheye dispsize=TRUE borderwidth=30 bordercolor="#00FF00" equirectangular=FALSE ! \
  #videoconvert ! \
