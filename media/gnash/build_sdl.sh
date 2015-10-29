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
DEFINE_PACKAGE_SECTION=Multimedia
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="Simple Directmedia Library."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--host=${DEFINE_HOST}  --enable-video-fbcon --disable-video-qtopia --disable-video-dummy --disable-video-opengl --disable-dga --disable-video-dga --disable-video-x11-dgamouse --disable-video-x11-vm --disable-video-x11 --disable-arts --disable-cdrom --disable-nasm "

. ${DEFINE_LUPC}/build_rule.sh
