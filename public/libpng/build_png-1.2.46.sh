#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=libpng-1.2.46.tar.gz
DEFINE_PACKAGE_NAME=libpng
DEFINE_PACKAGE_VERSION=1.2.46
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
DEFINE_CONFIGURE_FLAGS="--build=i386-linux --host=${DEFINE_HOST}"

. ${DEFINE_LUPC}/build_rule.sh