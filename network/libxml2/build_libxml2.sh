#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=libxml2-sources-2.7.7.tar.gz
DEFINE_PACKAGE_NAME=libxml2
DEFINE_PACKAGE_VERSION=2.7.7
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Communications
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="xml2 libraries."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--host=${DEFINE_HOST} "

. ${DEFINE_LUPC}/build_rule.sh
