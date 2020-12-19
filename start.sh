#!/bin/bash

if [ "$(uname)" == 'Darwin' ]; then   # Mac Case
  export CAMERA_SRC='avfvideosrc device-index=0 !'
elif [ "$(expr substr $(uname -s) 1 5)" == 'Linux' ]; then  # Linux Case
  if [[ $(uname -a) == *raspberry* ]]; then   # Raspberry Pi case
    export CAMERA_SRC='v4l2src device=/dev/video0 ! omxmjpegdec !'
  else                                        # Linux PC 
    export CAMERA_SRC='v4l2src device=/dev/video0 ! jpegdec !'
  fi
else
  echo 'Your platform is not supported. Only for linux or Mac.'
  exit 1
fi

export GST_PLUGIN_PATH="./build:${GST_PLUGIN_PATH}"
export GST_DEBUG="1,dualfisheye:4"
# DEBUG_LEVEL  1=ERROR, 2=WARNING, 3=FIXME, 4=INFO, 5=DEBUG, 6=LOG, 7=TRACE

# force cache clear
rm -rf ~/.cache/gstreamer-1.0

# Launch dualfisheye conversion
gst-launch-1.0 -v \
  $CAMERA_SRC \
  videoconvert !\
  dualfisheye !\
  videoconvert !\
  autovideosink
