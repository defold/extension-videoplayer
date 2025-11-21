#!/usr/bin/env bash

# Build libvpx for iOS. Run from a libvpx checkout:
#   bash path/to/videoplayer/utils/vpx/build_ios.sh <arch>
# Supported arches: arm64

set -euo pipefail

usage() {
    echo "Usage: $0 <arch>" >&2
    echo "  arch: arm64" >&2
    exit 1
}

ARCH="${1:-}"
[[ -n "${ARCH}" ]] || usage

IOS_SDK=$(xcrun --sdk iphoneos --show-sdk-path)
MIN_IOS_VERSION="${MIN_IOS_VERSION:-12.0}"

if [[ "${ARCH}" != "arm64" ]]; then
    echo "Unknown arch: ${ARCH}" >&2
    usage
fi

TARGET_TRIPLE="arm64-darwin-gcc"

BUILD_DIR="build-${ARCH}-ios"
CLANG_FLAGS="-arch ${ARCH} -fembed-bitcode -miphoneos-version-min=${MIN_IOS_VERSION}"

mkdir -p "${BUILD_DIR}"
pushd "${BUILD_DIR}" >/dev/null

if [[ -f Makefile ]]; then
    make clean
fi

CFLAGS="${CLANG_FLAGS}" \
LDFLAGS="${CLANG_FLAGS}" \
../configure \
    --target="${TARGET_TRIPLE}" \
    --sdk-path="${IOS_SDK}" \
    --enable-pic \
    --disable-examples \
    --disable-unit-tests \
    --disable-docs \
    --disable-tools

make -j"$(sysctl -n hw.ncpu 2>/dev/null || getconf _NPROCESSORS_ONLN)"

popd >/dev/null
