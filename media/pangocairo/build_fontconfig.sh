#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=fontconfig-2.8.0.tar.gz
DEFINE_PACKAGE_NAME=fontconfig
DEFINE_PACKAGE_VERSION=2.8.0
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Multimedia
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="Fontconfig library."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--host=${DEFINE_HOST}  --with-arch=mips --sysconfdir=/usr/local/etc/fonts --with-freetype-config=${DEFINE_GCC_PREFIX}/bin/freetype-config"

. ${DEFINE_LUPC}/build_rule.sh
