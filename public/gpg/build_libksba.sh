#!/bin/bash

DEFINE_PACKAGE_CROSS=no
DEFINE_PACKAGE_TAR=libksba-1.1.0.tar.bz2
DEFINE_PACKAGE_NAME=libksba
DEFINE_PACKAGE_VERSION=1.1.0
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Misc
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="gnu gpg comunication secrurity libraries."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--host=${DEFINE_HOST} "

. ${DEFINE_LUPC}/build_rule.sh
