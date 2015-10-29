#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=gstreamer-0.10.35.tar.bz2
DEFINE_PACKAGE_NAME=gstreamer
DEFINE_PACKAGE_VERSION=0.10.35
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Multimedia
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="gstreamer core."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--build=i686-pc-linux-gnu --host=${DEFINE_HOST}  --disable-nls --disable-loadsave --with-html-dir=/tmp/dump ac_cv_func_register_printf_function=no ac_cv_func_register_printf_specifier=no --with-sysroot=$DEFINE_GCC_SYSROOT --enable-shared"

. ${DEFINE_LUPC}/build_rule.sh
