#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=ffmpeg-2.4.2.tar.bz2
DEFINE_PACKAGE_NAME=ffmpeg
DEFINE_PACKAGE_VERSION=2.4.2
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Multimedia
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="ffmpeg library."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure 
#not use --disable-avcodec
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--arch=mips --disable-static --enable-shared --disable-ffplay --disable-ffprobe --disable-ffserver --disable-avdevice --disable-swresample --disable-swscale --disable-encoders --disable-decoders   --disable-muxers --disable-filters --disable-hwaccels  --disable-mipsfpu  --prefix=${DEFINE_BINARY} --enable-cross-compile --cc=/var/mips-4.3_h1/bin/mips-linux-gnu-gcc --strip=/var/mips-4.3_h1/bin/mips-linux-gnu-strip --target-os=linux"

. ${DEFINE_LUPC}/build_rule.sh
