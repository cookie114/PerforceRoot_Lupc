#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=php-5.2.17.tar.gz
DEFINE_PACKAGE_NAME=php
DEFINE_PACKAGE_VERSION=5.2.17
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Communications
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="A PHP script."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--host=${DEFINE_HOST} --prefix=${DEFINE_BINARY} --exec-prefix=${DEFINE_BINARY}  --disable-all --enable-pdo --with-sqlite3 --with-sqlite --with-pdo-sqlite --with-zlib --without-iconv"

. ${DEFINE_LUPC}/build_rule.sh
