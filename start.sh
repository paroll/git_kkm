#!/bin/bash

sudo modprobe bcm2835-v4l2
export STREAMER_PATH=$HOME/workspace/camera/mjpg-streamer/mjpg-streamer-experimental
export LD_LIBRARY_PATH=$STREAMER_PATH
$STREAMER_PATH/mjpg_streamer -i "$STREAMER_PATH/plugins/input_raspicam/input_raspicam.so -ex backlight -q 90 -x 1280 -y 720 -d /dev/video0" -o "$STREAMER_PATH/plugins/output_http/output_http.so -w $STREAMER_PATH/www"
