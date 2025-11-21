#!/usr/bin/env bash

set -euo pipefail

BUILD_DIR="build-js-web"

mkdir -p "${BUILD_DIR}"
pushd "${BUILD_DIR}" >/dev/null

if [[ -f Makefile ]]; then
    make distclean
fi

emconfigure ../configure \
    --prefix=js-web \
    --target=generic-gnu \
    --disable-runtime-cpu-detect \
    --disable-examples \
    --disable-unit-tests \
    --disable-docs \
    --disable-tools \
    --enable-pic

STRIP=true emmake make -j"$(nproc)"

popd >/dev/null
