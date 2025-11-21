#!/usr/bin/env bash

# Build libvpx for macOS. Run from a libvpx checkout:
#   bash path/to/videoplayer/utils/vpx/build_osx.sh <arch>
# Supported arches: x86_64, arm64

set -euo pipefail

usage() {
    echo "Usage: $0 <arch>" >&2
    echo "  arch: x86_64 | arm64" >&2
    exit 1
}

ARCH="${1:-}"

if [[ -z "${ARCH}" ]]; then
    usage
fi

case "${ARCH}" in
    x86_64)
        TARGET_TRIPLE="x86_64-darwin20-gcc"
        MACOSX_VERSION="-mmacosx-version-min=10.13"
        ;;
    x86)
        TARGET_TRIPLE="x86-darwin20-gcc"
        MACOSX_VERSION="-mmacosx-version-min=10.7"
        ;;
    arm64)
        TARGET_TRIPLE="arm64-darwin21-gcc"
        MACOSX_VERSION="-mmacosx-version-min=11.0"
        ;;
    *)
        echo "Unknown arch: ${ARCH}" >&2
        usage
        ;;
esac

CFLAGS_ARCH="-arch ${ARCH} ${MACOSX_VERSION}"
BUILD_DIR="build-${ARCH}-osx"

mkdir -p "${BUILD_DIR}"
pushd "${BUILD_DIR}" >/dev/null

if [[ -f Makefile ]]; then
    make clean
fi

CFLAGS="${CFLAGS_ARCH}" \
LDFLAGS="${CFLAGS_ARCH}" \
../configure \
    --target="${TARGET_TRIPLE}" \
    --enable-pic \
    --disable-examples \
    --disable-unit-tests \
    --disable-docs \
    --disable-tools

make -j"$(sysctl -n hw.ncpu 2>/dev/null || getconf _NPROCESSORS_ONLN)"

popd >/dev/null
