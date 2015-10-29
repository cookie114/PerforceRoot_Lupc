#!/bin/bash

DEFINE_PACKAGE_CROSS=yes
DEFINE_PACKAGE_TAR=alsa-lib-1.0.23.tar.bz2
DEFINE_PACKAGE_NAME=alsa-lib
DEFINE_PACKAGE_VERSION=1.0.23
DEFINE_PACKAGE_SOURCE=

# For ipkg
DEFINE_PACKAGE_STABLE=yes
DEFINE_PACKAGE_PRIORITY=optional
DEFINE_PACKAGE_DEPENDS=
DEFINE_PACKAGE_SECTION=Multimedia
DEFINE_PACKAGE_MAINTAINER="HuayaMicro Software Team"
DEFINE_PACKAGE_DESCRIPTION="alsa support libraries."

DEFINE_LUPC=../..
. ${DEFINE_LUPC}/build_setting.sh

# For configure
DEFINE_CONFIGURE_ENV=
DEFINE_CONFIGURE_CMD="./configure"
DEFINE_CONFIGURE_FLAGS="--host=${DEFINE_HOST}  --disable-python --with-softfloat --with-configdir=/usr/local/share/alsa --with-plugindir=/usr/local/share/alsa/plugins --with-alsa-devdir=/dev"

if [ -d /usr/local/share/alsa ]
then
	sudo rm -rf /usr/local/share/alsa
fi

sudo mkdir -p /usr/local/share/alsa
sudo mkdir -p /usr/local/share/alsa/cards
sudo chmod -R og+w /usr/local/share/alsa

. ${DEFINE_LUPC}/build_rule.sh

#mkdir -p ${DEFINE_BINARY}/share/alsa
#cp -rf /usr/local/share/alsa/alsa.conf ${DEFINE_BINARY}/share/alsa

