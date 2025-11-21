#!/usr/bin/env bash

set -euo pipefail

usage() {
    echo "Usage: $0 <mode>" >&2
    echo "  mode: js | wasm" >&2
    exit 1
}

MODE="${1:-js}"

case "${MODE}" in
    js)
        WASM_FLAG=0
        BUILD_DIR="build-js-web"
        PREFIX="js-web"
        SUPPORT_LONGJMP=0
        ;;
    wasm)
        WASM_FLAG=1
        BUILD_DIR="build-wasm-web"
        PREFIX="wasm-web"
        SUPPORT_LONGJMP=1
        ;;
    *)
        echo "Unknown mode: ${MODE}" >&2
        usage
        ;;
esac

mkdir -p "${BUILD_DIR}"
pushd "${BUILD_DIR}" >/dev/null

if [[ -f Makefile ]]; then
    make distclean
fi

EXTRA_FLAGS=""
if [[ "${SUPPORT_LONGJMP}" == "1" ]]; then
    EXTRA_FLAGS="-s SUPPORT_LONGJMP=1"
fi

export EMCC_CFLAGS="-s WASM=${WASM_FLAG} ${EXTRA_FLAGS}"
export EMXX_CFLAGS="-s WASM=${WASM_FLAG} ${EXTRA_FLAGS}"
export EMCC_CXXFLAGS="-s WASM=${WASM_FLAG} ${EXTRA_FLAGS}"
export LDFLAGS="-s WASM=${WASM_FLAG} ${EXTRA_FLAGS}"
export STRIP=echo

emconfigure ../configure \
    --prefix="${PREFIX}" \
    --target=generic-gnu \
    --disable-runtime-cpu-detect \
    --disable-examples \
    --disable-unit-tests \
    --disable-docs \
    --disable-tools \
    --enable-pic

emmake make -j"$(nproc)"

popd >/dev/null
