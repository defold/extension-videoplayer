#!/usr/bin/env bash

# Since the make command doesn't listen to ARFLAGS
# We need to archive the lib ourselves

pushd build-js-web

FILES=`find . -iname "*.o"`

$EMSCRIPTEN/emar crs libvpx.a $FILES

popd
