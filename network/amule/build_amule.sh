#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=aMule-2.2.6.tar.bz2
DEFINE_PACKAGE_NAME=aMule
DEFINE_PACKAGE_VERSION=2.2.6
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS="zlib wxWidgets cryptopp"
DEFINE_PACKAGE_SECTION=Communications
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="a emule download tool."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--host=${DEFINE_HOST}  \
--disable-monolithic \
--disable-static \
--enable-amule-daemon \
--enable-amulecmd \
--disable-debug \
--enable-optimize \
--with-wx-config=/root/mips/wxBase/bin/wx-config \
--with-toolkit=base"

. ${DEFINE_LUPC}/build_rule.sh
