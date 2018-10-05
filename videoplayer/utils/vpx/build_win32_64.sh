#!/usr/bin/env bash

# Run from a  Msys2 prompt (https://www.msys2.org/)

# pacman -S make
# pacman -S yasm
# pacman -S perl

set -e

mkdir -p build-x86_64-win32
pushd build-x86_64-win32

if [-e Makefile]
then
	make clean
fi

# needed if you have the perl lib in a non standard location (this path is from mingw itself though)
#export PERL5LIB=/mingw/msys/1.0/lib/perl5/5.8/

#export PATH=$PATH:/c/Program\ Files\ \(x86\)/Microsoft\ Visual\ Studio\ 14.0/Common7/IDE

#export PATH=$PATH:/c/Program\ Files\ \(x86\)/MSBuild/14.0/Bin/amd64

../configure --prefix=x86_64-win32 --target=x86_64-win64-vs14 --enable-static-msvcrt --disable-examples --disable-unit-tests --disable-docs
make -j8
#make install

popd
