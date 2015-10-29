#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=qt-everywhere-opensource-src-4.7.4.tar.gz
DEFINE_PACKAGE_NAME=qt-everywhere-opensource-src
DEFINE_PACKAGE_VERSION=4.7.4
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
#export QMK_CFLAGS="${DEFINE_CFLAGS}" 
#export QMK_CXXFLAGS="${DEFINE_CXXFLAGS}"
#export QMK_LFLAGS="${DEFINE_LDFLAGS}"


chmod 777 ./qt-everywhere-opensource-src-4.7.4.patch

sed -i "s#\${QMK_CFLAGS}#\"${DEFINE_CFLAGS}\"#g" ./qt-everywhere-opensource-src-4.7.4.patch
sed -i "s#\${QMK_CXXFLAGS}#\"${DEFINE_CXXFLAGS}\"#g" ./qt-everywhere-opensource-src-4.7.4.patch
sed -i "s#\${QMK_LFLAGS}#\"${DEFINE_LDFLAGS}\"#g" ./qt-everywhere-opensource-src-4.7.4.patch

#DEFINE_CONFIGURE_CMD="configure"
DEFINE_CONFIGURE_FLAGS="-opensource -confirm-license -embedded mips -xplatform qws/linux-mips-g++ -little-endian -qt-gfx-linuxfb -plugin-gfx-directfb -no-qt3support -no-scripttools -no-declarative -no-declarative-debug -no-accessibility -no-stl -no-cups -no-nis -no-gtkstyle -no-opengl -no-openvg -nomake demos -nomake examples -nomake docs -no-libpng -system-libpng"
DEFINE_INSTALL_CMD="make INSTALL_ROOT=${DEFINE_BINARY} install"

. ${DEFINE_LUPC}/build_rule.sh

#to get qmake work
#1. need to add 'QMAKEDIR' to enviroment varaible
# export QMAKEDIR=/xxxx/cross_tools/mips-4.3/mips-linux-gnu/libc/el/usr/bin
#2. need to add qmake path to PATH
. ./qt_setting.sh


