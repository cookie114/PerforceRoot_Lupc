#!/bin/bash

if [ ! -d $2/bin ]
then
	mkdir $2/bin
else
	rm -f $2/bin/*
fi
if [ ! -d $2/lib ]
then
	mkdir $2/lib
else
	rm -f $2/lib/*
fi

cp -l $1/src/.libs/ntfs-3g* $2/bin
cp -l $1/libntfs-3g/.libs/libntfs-3g.so* $2/lib

