#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=gst-ffmpeg-0.10.11.tar.bz2
DEFINE_PACKAGE_NAME=gst-ffmpeg
DEFINE_PACKAGE_VERSION=0.10.11
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Multimedia
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="gstreamer ffmpeg plugins."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--host=${DEFINE_HOST} --target=${DEFINE_HOST} --with-ffmpeg-extra-configure="--target-os=linux"  --with-html-dir=/tmp/dump"

. ${DEFINE_LUPC}/build_rule.sh
