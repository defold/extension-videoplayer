#!/usr/bin/env bash

set -euo pipefail

BUILD_DIR="build-js-web"

mkdir -p "${BUILD_DIR}"
pushd "${BUILD_DIR}" >/dev/null

if [[ -f Makefile ]]; then
    make distclean
fi

export EMCC_CFLAGS="-s WASM=0"
export EMXX_CFLAGS="-s WASM=0"
export EMCC_CXXFLAGS="-s WASM=0"
export LDFLAGS="-s WASM=0"
export STRIP=echo

emconfigure ../configure \
    --prefix=js-web \
    --target=generic-gnu \
    --disable-runtime-cpu-detect \
    --disable-examples \
    --disable-unit-tests \
    --disable-docs \
    --disable-tools \
    --enable-pic

emmake make -j"$(nproc)"

popd >/dev/null
