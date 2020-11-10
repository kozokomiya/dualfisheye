#!/bin/bash

export JANUS_SERVER=aiit-act-2020.work  # Janus Server Address
export V_PORT=8013                      # Video UDP PORT
export A_PORT=8003                      # Audio UDP PORT

# force cache clear
rm -rf ~/.cache/gstreamer-1.0

# set ENV
export GST_PLUGIN_PATH="./build:${GST_PLUGIN_PATH}"
export GST_DEBUG="1,dualfisheye:6"
# DEBUG_LEVEL  1=ERROR, 2=WARNING, 3=FIXME, 4=INFO, 5=DEBUG, 6=LOG, 7=TRACE


# Launch dualfisheye test
gst-launch-1.0 -v \
  udpsrc port=18011 ! \
  "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264" ! \
  rtph264depay ! \
  avdec_h264 ! \
  videoconvert ! \
  dualfisheye ! \
  queue ! \
  videoconvert ! \
  vp8enc ! \
  rtpvp8pay pt=96 ! \
  udpsink host=$JANUS_SERVER port=$V_PORT
