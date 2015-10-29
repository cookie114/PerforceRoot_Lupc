#!/bin/bash

if [ $# -gt 0 ]; then
	TMP_INCLUDE_PATH=$1/gpe_base.h
	if [ ! -f ${TMP_INCLUDE_PATH} ]; then
		echo "${TMP_INCLUDE_PATH} not exists!"	
		exit 1
	else
		TMP_INCLUDE_PATH=$1
	fi
else
	echo "need gpe_base.h's path as parameter 1"
	echo "like:"
	echo " xxx/Linux/2.6.27/include/asm-mips/mach-huaya"
	echo " xxx/Linux/3.0/arch/mips/include/asm/mach-nba"
	exit 1
fi
shift

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=DirectFB-1.4.17.tar.gz
DEFINE_PACKAGE_NAME=DirectFB
DEFINE_PACKAGE_VERSION=1.4.17
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Misc
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="compress/decompress libraries."

DEFINE_COMPILER_CFLAGS="-I${TMP_INCLUDE_PATH} -fPIC"
DEFINE_COMPILER_CXXFLAGS="-I${TMP_INCLUDE_PATH} -fPIC"

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--build=i386-linux --host=${DEFINE_HOST} --disable-x11 --with-gfxdrivers=huaya --with-inputdrivers=linuxinput"

#PKG_CONFIG_PATH=${PKG_CONFIG_LIBDIR}
#echo ${PKG_CONFIG_LIBDIR}
#pkg-config --cflags "freetype2"
#exit 0
. ${DEFINE_LUPC}/build_rule.sh
