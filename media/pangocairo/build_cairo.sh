#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=cairo-1.10.2.tar.gz
DEFINE_PACKAGE_NAME=cairo
DEFINE_PACKAGE_VERSION=1.10.2
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Multimedia
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="Cairo library."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--host=${DEFINE_HOST}  --enable-xlib=no --enable-xlib-xrender=no --enable-xcb=no --enable-xlib-xcb=no --enable-xcb-shm=no --enable-win32=no"

. ${DEFINE_LUPC}/build_rule.sh
