#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=gst-plugins-base-0.10.35.tar.bz2
DEFINE_PACKAGE_NAME=gst-plugins-base
DEFINE_PACKAGE_VERSION=0.10.35
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Multimedia
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="gstreamer base plugins."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--host=${DEFINE_HOST}  --disable-nls --disable-static --with-html-dir=/tmp/dump \
--disable-x --disable-aalib --disable-esd --disable-shout2 --disable-sdl --disable-theora --disable-pango --with-audioresample-format=int \
--disable-xvideo --disable-xshm --disable-gnome_vfs --disable-libvisual --disable-examples"
. ${DEFINE_LUPC}/build_rule.sh
