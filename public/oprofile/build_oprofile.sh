#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=oprofile-0.9.7-rc1.tar.gz
DEFINE_PACKAGE_NAME=oprofile
DEFINE_PACKAGE_VERSION=0.9.7-rc1
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Misc
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="compress/decompress libraries."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--host=${DEFINE_HOST}  --with-linux=${DEFINE_LUPC}/../froyo-kernel --with-kernel-support --with-extra-includes=${DEFINE_GCC_PREFIX}/include --with-extra-libs=${DEFINE_GCC_PREFIX}/lib"

cp ./binutils/*.h ${DEFINE_GCC_ROOT}/mips-linux-gnu/include
cp ./binutils/libbfd.a ${DEFINE_GCC_ROOT}/mips-linux-gnu/lib
cp ./binutils/libiberty.a ${DEFINE_GCC_ROOT}/mips-linux-gnu/lib/soft-float/el/

. ${DEFINE_LUPC}/build_rule.sh
