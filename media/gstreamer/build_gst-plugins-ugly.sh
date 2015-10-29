#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=gst-plugins-ugly-0.10.18.tar.bz2
DEFINE_PACKAGE_NAME=gst-plugins-ugly
DEFINE_PACKAGE_VERSION=0.10.18
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Multimedia
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="gstreamer ugly plugins."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--host=${DEFINE_HOST}  --disable-nls --disable-static --with-html-dir=/tmp/dump"

. ${DEFINE_LUPC}/build_rule.sh
