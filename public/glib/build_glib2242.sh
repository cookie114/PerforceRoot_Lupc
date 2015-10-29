#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=glib-2.24.2.tar.bz2
DEFINE_PACKAGE_NAME=glib
DEFINE_PACKAGE_VERSION=2.24.2
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Misc
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="glib libraries."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--build=i386-linux --host=${DEFINE_HOST}  --with-html-dir=/tmp/dump glib_cv_stack_grows=no glib_cv_uscore=no ac_cv_func_posix_getgrgid_r=no ac_cv_func_posix_getpwuid_r=no"
echo "$1"
. ${DEFINE_LUPC}/build_rule.sh $1