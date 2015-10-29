#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=libffi-3.0.11.tar.gz
DEFINE_PACKAGE_NAME=libffi
DEFINE_PACKAGE_VERSION=3.0.11
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Misc
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="foreign function interface library."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--build=i386-linux --host=${DEFINE_HOST} --prefix=${DEFINE_BINARY} --program-prefix=${DEFINE_PREFIX}"
# echo "$1"
. ${DEFINE_LUPC}/build_rule.sh 
# $1
