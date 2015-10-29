#!/bin/bash

# Default setting
. ${DEFINE_LUPC}/build_setting.sh

# Set error output
set -e

# Usage
func_usage() {
cat << EOF
Common script for build Lupc packages.
Usage:
$(basename $0) [help|source|config|all|clean|insall|uninstall|pkgconfig|ipkg]
	help:		Print this message.
	source:		Check source tree, else de-compress from tar ball.
	config:		Configure package.
	all:		Compile package.
	install:	Install package.
	uninstall:	Uninstall package.
	clean:		Clean package.
	ipkg:		Generate ipkg file and put to package release directory.

Don't specify any command param and it will go though source, binary, config, all, install, clean and pkgconfig.
EOF
}

# De-compress tar ball to generate source directory
func_decompress() {
	local extension=${1##*.}
	case "${extension}" in
		"gz")
			tar -xvf ${1}
			;;
		"bz2")
			tar -jxvf ${1}
			;;
		"zip")
			unzip ${1} -d ${DEFINE_PACKAGE_NAME}-${DEFINE_PACKAGE_VERSION}
			;;
		"tgz")
			tar -zxvf ${1}
			;;
		"xz")
			tar -xJkf ${1}
			;;
		*)
			echo "${DEFINE_LOG_TITLE}Un-supported tar ball: ${1} -> ${extension}!"
			exit 0;
			;;
	esac
}

# Check source directory
func_check_source() {
	if  [ ! -d ${DEFINE_SOURCE} ]
	then
		echo "${DEFINE_LOG_TITLE}Create package source directory: ${DEFINE_SOURCE}..."
		func_decompress ${DEFINE_PACKAGE_TAR} ${DEFINE_SOURCE}
		if [ -f ${DEFINE_SOURCE}.patch ]
		then
			echo "${DEFINE_LOG_TITLE}Patch source code..."
			patch -p1 < ${DEFINE_SOURCE}.patch
		fi

		echo "${DEFINE_LOG_TITLE}Done"
	fi
}

# Check binary directory
func_check_binary() {
	if  [ ! -d ${DEFINE_BINARY} ]
	then
		echo "${DEFINE_LOG_TITLE}Create package release directory: ${DEFINE_RELEASE}..."
		mkdir -p ${DEFINE_BINARY}
		echo "${DEFINE_LOG_TITLE}Done"
	fi
}

# Configure
func_configure() {

	# make build dir
	if [ ! -z "${DEFINE_PACKAGE_BUILDDIR}" ]
	then
		if  [ ! -d ${DEFINE_PACKAGE_BUILD} ]
		then
			echo "${DEFINE_LOG_TITLE}Create build directory..."
			mkdir ${DEFINE_PACKAGE_BUILDDIR}
			echo "${DEFINE_LOG_TITLE}Done"
		fi
		cd ${DEFINE_PACKAGE_BUILDDIR}
		FUNC_CONF_CMD_PREFIX=../ 
	else
		FUNC_CONF_CMD_PREFIX=./ 
	fi

	if [ -e Makefile ]
	then
		#make clean
		echo
	fi

	if [ ! -z "${DEFINE_CONFIGURE_CMD}" ]
	then
		echo "${DEFINE_LOG_TITLE}Cross configure..."
		if [ ${DEFINE_PACKAGE_NAME} = fontconfig ]
		then
			CC="${DEFINE_HOST}-gcc ${DEFINE_CFLAGS}" LDFLAGS=${DEFINE_LDFLAGS} ${FUNC_CONF_CMD_PREFIX}/${DEFINE_CONFIGURE_CMD} ${DEFINE_CONFIGURE_FLAGS} ${LT_SYSROOT_FLAG}
		else
			CC="${DEFINE_HOST}-gcc ${DEFINE_CFLAGS}" CXX="${DEFINE_HOST}-g++ ${DEFINE_CFLAGS}" CFLAGS=${DEFINE_CFLAGS} CXXFLAGS=${DEFINE_CXXFLAGS} LDFLAGS=${DEFINE_LDFLAGS} ${FUNC_CONF_CMD_PREFIX}/${DEFINE_CONFIGURE_CMD} ${DEFINE_CONFIGURE_FLAGS} --prefix=/usr ${LT_SYSROOT_FLAG}
		fi
	else
		echo "${DEFINE_LOG_TITLE}Local configure..."
		${FUNC_CONF_CMD_PREFIX}/configure ${DEFINE_CONFIGURE_FLAGS} --prefix=/usr ${LT_SYSROOT_FLAG}
	fi
	echo "${DEFINE_LOG_TITLE}Done"
}

# Compile
func_compile() {
	if [ ! -z "${DEFINE_MAKEFILE_CMD}" ]
	then
		echo "${DEFINE_LOG_TITLE}Customer compiling..."
		CC="${DEFINE_HOST}-gcc ${DEFINE_CFLAGS}" LDFLAGS=${DEFINE_LDFLAGS} ${DEFINE_MAKEFILE_CMD} ${DEFINE_MAKEFILE_FLAGS}
	else
		echo "${DEFINE_LOG_TITLE}Default compiling..."
		make
	fi
	echo "${DEFINE_LOG_TITLE}Done"
}

# Install
func_install() {
	# stage install, firstly intall in ${DEFINE_BINARY}
	# then strip, copy into ${DEFINE_CGG_SYSROOT}
	func_check_binary
	echo "${DEFINE_LOG_TITLE}Install binary..."
	if [ ! -z "${DEFINE_INSTALL_CMD}" ]
	then
		${DEFINE_INSTALL_CMD} ${DEFINE_INSTALL_FLAGS}
	else
		make DESTDIR=${DEFINE_BINARY} install
	fi

	#export LD_LIBRARY_PATH=${DEFINE_BINARY}/lib:${LD_LIBRARY_PATH}

	# Here strip executable files
	if [ -d ${DEFINE_BINARY}/usr/bin ]
	then
		${DEFINE_HOST}-strip ${DEFINE_BINARY}/usr/bin/* || true
	fi
	if [ -d ${DEFINE_BINARY}/usr/sbin ]
	then
		${DEFINE_HOST}-strip ${DEFINE_BINARY}/usr/sbin/* || true
	fi
	if [ -d ${DEFINE_BINARY}/usr/lib ]
	then
		find ${DEFINE_BINARY}/usr/lib -name "*.so*" -exec ${DEFINE_HOST}-strip {} \;
		find ${DEFINE_BINARY}/usr/lib -type f -name "*.la" -exec sed -i -e '/dependency_libs=/ s% \(/usr/lib\)% =\1%g' {} \; -print
	fi

	FUNC_INSTALL_PWD=`pwd`
	cd ${DEFINE_BINARY}
	tar cvf inst.tmp.tar usr
	cd ${DEFINE_GCC_SYSROOT}
	sudo tar xvf ${DEFINE_BINARY}/inst.tmp.tar
	# rm -rf ${DEFINE_BINARY}
	cd ${FUNC_INSTALL_PWD}

	echo "${DEFINE_LOG_TITLE}Done"
}

# Un-install
func_uninstall() {
	echo "${DEFINE_LOG_TITLE}Uninstall binary..."

	if [ ${DEFINE_PACKAGE_CROSS} = 'yes' ]
	then
		make DESTDIR=${DEFINE_GCC_SYSROOT} uninstall
		rm -R ${DEFINE_BINARY}
	else
		make uninstall
	fi
	echo "${DEFINE_LOG_TITLE}Done"
}

# Clean up
func_clean() {
	echo "${DEFINE_LOG_TITLE}Clean source..."
	make clean
	echo "${DEFINE_LOG_TITLE}Done"
}


# Generate ipkg file and put to package release directory
func_ipkg() {
	if [ -d ${DEFINE_PKG_BINARY} ]
	then
		echo "${DEFINE_LOG_TITLE}Generate ipkg file, then generate release package..."
		# Prepare ipkg
		if [ -f ${DEFINE_PACKAGE_NAME}-ipkg-pre.sh ]
		then
			chmod 777 ${DEFINE_PACKAGE_NAME}-ipkg-pre.sh
			./${DEFINE_PACKAGE_NAME}-ipkg-pre.sh
		fi
		# Generate control file
		if [ ! -d ${DEFINE_PKG_BINARY}/CONTROL ]
		then
			mkdir ${DEFINE_PKG_BINARY}/CONTROL
		fi
		echo "Package: ${DEFINE_PACKAGE_NAME}"				>  ${DEFINE_PKG_BINARY}/CONTROL/control
		echo "Version: ${DEFINE_PACKAGE_VERSION}"			>> ${DEFINE_PKG_BINARY}/CONTROL/control
		echo "Priority: ${DEFINE_PACKAGE_PRIORITY}"			>> ${DEFINE_PKG_BINARY}/CONTROL/control
		echo "Depends: ${DEFINE_PACKAGE_DEPENDS}"			>> ${DEFINE_PKG_BINARY}/CONTROL/control
		echo "Section: ${DEFINE_PACKAGE_SECTION}"			>> ${DEFINE_PKG_BINARY}/CONTROL/control
		echo "Architecture: mipsel"							>> ${DEFINE_PKG_BINARY}/CONTROL/control
		echo "Maintainer: ${DEFINE_PACKAGE_MAINTAINER}" 	>> ${DEFINE_PKG_BINARY}/CONTROL/control
		echo "Source: ${DEFINE_PACKAGE_TAR}" 				>> ${DEFINE_PKG_BINARY}/CONTROL/control
		echo "Description: ${DEFINE_PACKAGE_DESCRIPTION}"	>> ${DEFINE_PKG_BINARY}/CONTROL/control

		if [ ${DEFINE_PACKAGE_STABLE} = "no" ]
		then
			pkg_dir=${DEFINE_PKG_ROOT}/unstable
		else
			pkg_dir=${DEFINE_PKG_ROOT}/stable
		fi
		# Filter out old section from Packages file
		sed "/Package: ${DEFINE_PACKAGE_NAME}$/, +12d" ${pkg_dir}/Packages > ${pkg_dir}/Packages.tmp
		mv ${pkg_dir}/Packages.tmp ${pkg_dir}/Packages
		# Call standard ipkg-build.sh
		DEFINE_PKG_ROOT=${pkg_dir} DEFINE_PACKAGE_NAME=${DEFINE_PACKAGE_NAME} ${DEFINE_LUPC}/build_package.sh ${DEFINE_PKG_BINARY}
		echo "${DEFINE_LOG_TITLE}Done"
	fi
}

# patch ltmain.sh
func_patch_lt_sysroot() {
	if [ -f ltmain.sh ]
	then
		echo "${DEFINE_LOG_TITLE} add lt_sysroot"
		LT_SYSROOT_FLAG=" --with-sysroot=${DEFINE_GCC_SYSROOT}"

		#if [ -f lt_sysroot.patch ]
		#then
		#	echo "${DEFINE_LOG_TITLE}Patch libtool (sysroot)..."
		#	patch -p1 < lt_sysroot.patch
		#fi
	else
		LT_SYSROOT_FLAG=""
	fi
}

# Main
if [ $# -lt 1 ]
then
	func_check_source

	cd ${DEFINE_SOURCE}/${DEFINE_PACKAGE_SOURCE}

	func_patch_lt_sysroot

	func_configure
	func_compile
	func_install

#	func_uninstall
#	func_clean
else
	case "${1}" in
		"help")
			func_usage
			;;
		"source")
			func_check_source
			;;
		"config")
			cd ${DEFINE_SOURCE}/${DEFINE_PACKAGE_SOURCE}
			func_configure
			;;
		"all")
			cd ${DEFINE_SOURCE}/${DEFINE_PACKAGE_SOURCE}
			func_compile
			;;
		"install")
			cd ${DEFINE_SOURCE}/${DEFINE_PACKAGE_SOURCE}
			func_install
			;;
		"pkgconfig")
			func_pkgconfig
			;;
		"uninstall")
			cd ${DEFINE_SOURCE}/${DEFINE_PACKAGE_SOURCE}
			func_uninstall
			;;
		"clean")
			cd ${DEFINE_SOURCE}/${DEFINE_PACKAGE_SOURCE}
			func_clean
			;;
		"ipkg")
			func_ipkg
			;;
		*)
			;;
	esac
fi
cd ${DEFINE_ROOT}

