#!/bin/sh

if [ -e antlr-3.5.jar ]; then
	true
else
	echo "Downloading ANTLR 3.5 JAR from http://www.antlr.org/download.html"
	wget -c http://www.antlr3.org/download/antlr-3.5.jar || \
	(echo "Failed to download ANTLR. Aborting."; exit 1)
fi
if [ -e antlr-3.5/lib/libantlr3c.a -o -e libantlr3c-3.5.tar.gz ]; then
	true
else
	echo "Downloading ANTLR 3.5 C Runtime from http://www.antlr.org/download/C"
	wget -c http://www.antlr3.org/download/C/libantlr3c-3.4.tar.gz || \
	(echo "Failed to download ANTLR. Aborting."; exit 1)
fi
if [ -e antlr-3.5/lib/libantlr3c.a ]; then
	true
else
	MYPWD="$PWD"
        FLAGS64=
        if uname -m | grep x86_64; then
            FLAGS64=--enable-64bit ;
        fi
	tar -zxf libantlr3c-3.4.tar.gz && \
	cd libantlr3c-3.4 && \
	./configure --prefix="$MYPWD"/antlr-3.5 $FLAGS64 --disable-shared && \
	make && \
	make install && \
	cd "$MYPWD"
fi
if [ -e antlr-3.5.jar -o -e antlr-3.5/lib/libantlr3c.a ]; then
	true
else
	echo "Compile finished, but couldn't find all output files."; exit 1
fi

echo "Type 'make' to compile javascript files."
