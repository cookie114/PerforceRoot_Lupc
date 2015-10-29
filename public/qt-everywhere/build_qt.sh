#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=qt-everywhere-opensource-src-4.7.2.tar.gz
DEFINE_PACKAGE_NAME=qt-everywhere-opensource-src
DEFINE_PACKAGE_VERSION=4.7.2
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Communications
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="QT application framwork."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="-embedded mips -xplatform qws/linux-mips-g++ -little-endian -qt-gfx-linuxfb -no-opengl -no-multimedia -no-qt3support -no-script no-scripttools -no-audio-backend -no-openssl -no-sql-sqlite -no-largefile"

. ${DEFINE_LUPC}/build_rule.sh
