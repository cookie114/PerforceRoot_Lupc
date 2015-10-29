#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=rp-pppoe-3.10.tar.gz
DEFINE_PACKAGE_NAME=rp-pppoe
DEFINE_PACKAGE_VERSION=3.10
DEFINE_PACKAGE_SOURCE=src

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# Prepare ipkg files
echo "${DEFINE_LOG_TITLE}remove: ${DEFINE_BINARY}/share"
rm -Rf ${DEFINE_BINARY}/share
echo "${DEFINE_LOG_TITLE}create: ${DEFINE_BINARY}/etc"
mkdir -p ${DEFINE_PKG_BINARY}/etc/ppp
cp ${DEFINE_SOURCE}/configs/* ${DEFINE_PKG_BINARY}/etc/ppp
