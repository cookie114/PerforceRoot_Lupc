#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=ppp-2.4.5.tar.gz
DEFINE_PACKAGE_NAME=ppp
DEFINE_PACKAGE_VERSION=2.4.5
DEFINE_PACKAGE_SOURCE=

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# Prepare ipkg files
echo "${DEFINE_LOG_TITLE}remove: ${DEFINE_BINARY}/include"
rm -Rf ${DEFINE_BINARY}/include
echo "${DEFINE_LOG_TITLE}remove: ${DEFINE_BINARY}/share"
rm -Rf ${DEFINE_BINARY}/share
