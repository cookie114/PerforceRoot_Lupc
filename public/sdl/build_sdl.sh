#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=SDL-1.2.14.tar.gz
DEFINE_PACKAGE_NAME=SDL
DEFINE_PACKAGE_VERSION=1.2.14
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Communications
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="SDL - Simple Directmedia Laye."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--host=${DEFINE_HOST}  --disable-joystick --disable-cdrom --disable-esd --disable-arts --disable-nas --disable-diskaudio --disable-dummyaudio --disable-mintaudio --disable-nasm --disable-altivec --disable-video-dga --disable-video-photon --disable-video-cocoa --disable-video-ps2gs --disable-video-ps3 --disable-video-svga --disable-video-vgl --disable-video-wscons --disable-video-xbios --disable-video-gem --disable-video-dummy"

. ${DEFINE_LUPC}/build_rule.sh
