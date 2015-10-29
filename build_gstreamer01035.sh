#!/bin/bash

cd public/zlib && ./build_zlib126.sh && cd ../..
cd public/glib && ./build_glib.sh && cd ../..

cd media/alsa-lib && ./build_alsa.sh && cd ../..
cd media/libid3tag && ./build_libid3tag.sh && cd ../..
cd media/libogg && ./build_libogg.sh && cd ../..
cd media/libvorbis && ./build_libvorbis.sh && cd ../..
cd media/libmad && ./build_libmad.sh && cd ../..
cd media/gstreamer && ./build_gstreamer.sh && cd ../..
cd media/gstreamer && ./build_gst-plugins-base.sh && cd ../..
cd media/gstreamer && ./build_gst-plugins-good.sh && cd ../..
cd media/gstreamer && ./build_gst-plugins-bad.sh && cd ../..
cd media/gstreamer && ./build_gst-plugins-ugly.sh && cd ../..
cd media/gstreamer && ./build_gst-plugins-loks.sh && cd ../..

