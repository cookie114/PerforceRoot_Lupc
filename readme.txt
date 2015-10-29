Lupc: Linux user-space package collection.

Part 1: Top View

	This directory used for the exteranl open source packages.


Part 2: Directory

	It's hiberarchy layout:

	.Top layers
	Three types of folder:
		/media: for all multi-media related packages, include audio/video/contener, etc.
		/network: for all network related packages, include tcpip/web/ssh/bt, etc.
		/public: other public packages.

	.Sub layers
	Open source project folders.

	Some project release many packages, please just put them into one folder.


Part 3: Build software

	We inplemented some bash script for the package building in the sub layers folders,
	it nameing as: build_[package_name].sh.

	Currently it support auto decompress package/patch, configure, compiling, install, 
	uninstall, clean, pkgconfig and merge to compile cmmands.

	If you didn't kown how to use, please enter sub folder and excute:
	./build_[package_name].sh help
	Then it will print out help.


Part 4:  Development
	
	Please just submit tar ball file into the file server, and use patch file
	to release your modifications.
	By use default bash script, it will automatic de-compress tar ball and
	install all binary into [package_name]_binary directory.
	To make compiler link with those binary library, it also auto merge
	installed binary into compiler directory.
	Some file need be generated:
		.build_[package_name].sh: package command file used to config/compiling/install/etc.
			Please refference other packages build file.
		.[package_name][version].patch: package source code patch file.
			Use "diff -Nru [old_file/path] [new_file/path] > [package_name][version].patch"
			to generate patch file. This file will automatice used by script when generate
			package source code tree.

Part 5: Status
	Public packages:
		curl: OK
		zlib: OK
		liboil: OK
		libevent: OK
		glib: OK
		ipkg: OK

		No need: gpg: OK (patch)
		No need: pth: OK
		No need: gpgme: Fail for need gpgsm???
		No need: opkg: Fail host GPGME >= 1.0.0
		No need: intltool: OK (No need for transmission used local library)

	Network packages:
		openssl: OK
		libxml2: OK
		openssl: OK
		openssh: Work. Need root to crate /var/empty, and will fail when key_gen (mips arch program running in x86)
		pure-ftpd: OK
		transmission: OK (Need host intltool >= 0.35.0)
		ppp: OK
		rp-pppoe: Work. Just ignore the install to /etc/ fail issue.

	Media packages:
		alsa-lib: OK (patch)
		libid3tag: OK
		libogg: OK
		libvorbis: OK
		libmad: OK (patch)
		gstreamer: OK

NOTE:
	If the package use the libtool when compile, you must set sysroot when
	configure.
