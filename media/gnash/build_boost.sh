#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=boost_1_45_0.tar.bz2
DEFINE_PACKAGE_NAME=boost
DEFINE_PACKAGE_VERSION=1.45.0
DEFINE_PACKAGE_SOURCE=boost_1_45_0

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Multimedia
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="Boost library."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--host=${DEFINE_HOST} "

. ${DEFINE_LUPC}/build_rule.sh source

# rename source to ${DEFINE_SOURCE}
mv ${DEFINE_PACKAGE_SOURCE} ${DEFINE_SOURCE}

# patch project-config.jam
patch -p1 < ${DEFINE_SOURCE}.patch

cd ${DEFINE_SOURCE}

# create bjam
./bootstrap.sh

./bjam  --exec-prefix=${DEFINE_BINARY} --libdir=${DEFINE_BINARY}/lib --includedir=${DEFINE_BINARY}/include

./bjam install
