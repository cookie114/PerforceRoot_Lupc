#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=gdb-7.0.tar.bz2
DEFINE_PACKAGE_NAME=gdb
DEFINE_PACKAGE_VERSION=7.0
DEFINE_PACKAGE_SOURCE=gdb/gdbserver

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# Prepare ipkg files
echo "${DEFINE_LOG_TITLE}remove: ${DEFINE_BINARY}/share"
rm -Rf ${DEFINE_BINARY}/share
