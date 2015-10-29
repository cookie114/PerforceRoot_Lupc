#!/bin/bash

cd public/zlib && ./build_zlib-1.2.7.sh && cd ../..
cd public/glib && ./build_glib-2.34.2.sh && cd ../..
cd public/libjpeg && ./build_jpegsrc-v6b.sh && cd ../..

cd public/libsoup && ./build_libsoup-2.40.3.sh && cd ../..

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

