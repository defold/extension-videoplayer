#!/usr/bin/env bash

# Run from a Msys2 prompt (https://www.msys2.org/)

# pacman -S make
# pacman -S yasm
# pacman -S perl

set -e

mkdir -p build-x86-win32
pushd build-x86-win32

if [-e Makefile]
then
	make clean
fi

export PATH=$PATH:/c/Program\ Files\ \(x86\)/MSBuild/14.0/Bin/amd64

../configure --prefix=x86-win32 --target=x86-win32-vs14 --enable-static-msvcrt --disable-examples --disable-unit-tests --disable-docs
make -j8
#make install

popd
