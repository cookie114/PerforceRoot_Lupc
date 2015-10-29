#!/bin/bash

cd public/zlib && ./build_zlib-1.2.7.sh && cd ../..
cd public/libffi && ./build_libffi-3.0.11.sh && cd ../..
cd public/glib && ./build_glib-2.34.2.sh && cd ../..
cd network/libxml2 && ./build_libxml2-2.9.0.sh && cd ../..
cd network/libsoup && ./build_libsoup-2.40.3.sh && cd ../..
# cd media/alsa-lib && ./build_alsa.sh && cd ../..
# cd media/libid3tag && ./build_libid3tag.sh && cd ../..
# cd media/libogg && ./build_libogg.sh && cd ../..
# cd media/libvorbis && ./build_libvorbis.sh && cd ../..
# cd media/libmad && ./build_libmad.sh && cd ../..
cd media/gstreamer-1.0 && ./build_gstreamer-1.0.7.sh && cd ../..
cd media/gstreamer-1.0 && ./build_gst-plugins-base-1.0.7.sh && cd ../..
cd media/gstreamer-1.0 && ./build_gst-plugins-good-1.0.7.sh && cd ../..
cd media/gstreamer-1.0 && ./build_gst-plugins-bad-1.0.7.sh && cd ../..
cd media/gstreamer-1.0 && ./build_gst-plugins-ugly-1.0.7.sh && cd ../..
# cd media/gstreamer-1.0 && ./build_gst-plugins-loks-1.0.0.sh && cd ../..

