#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=pjproject-2.3.tar.bz2
DEFINE_PACKAGE_NAME=pjproject
DEFINE_PACKAGE_VERSION=2.3
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=sip
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="pjsip library."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS=" --host=mips-elf-linux --prefix=${DEFINE_BINARY} --enable-video --disable-sdl  --disable-ffmpeg --disable-v4l2 --disable-openh264 --disable-libyuv CC=/var/mips-4.3_h1/bin/mips-linux-gnu-gcc"

. ${DEFINE_LUPC}/build_rule.sh
