#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=libevent-1.4.14b-stable.tar.gz
DEFINE_PACKAGE_NAME=libevent
DEFINE_PACKAGE_VERSION=1.4.14b-stable
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Misc
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="event process libraries."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--host=${DEFINE_HOST}  --enable-static=no"

. ${DEFINE_LUPC}/build_rule.sh
