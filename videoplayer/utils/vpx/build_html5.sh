#!/usr/bin/env bash


set -e

mkdir -p build-js-web
pushd build-js-web

if [-e Makefile]
then
	make distclean
fi

ARFLAGS=crs $EMSCRIPTEN/emconfigure ../configure --prefix=js-web --disable-examples --disable-unit-tests --disable-docs --disable-tools
make -j8
#make install

popd



