
#!/usr/bin/env bash

# https://groups.google.com/a/webmproject.org/forum/#!topic/webm-discuss/zpy_lgS6K-8

set -euo pipefail

usage() {
    echo "Usage: $0 <arch>" >&2
    echo "  arch: armv7 | arm64" >&2
    exit 1
}

ARCH="${1:-}"
[[ -n "${ARCH}" ]] || usage

SDK_PATH="${ANDROID_NDK_HOME:-${ANDROID_SDK_PATH:-${ANDROID_HOME:-}}}"
if [[ -z "${SDK_PATH}" ]]; then
    echo "Set ANDROID_NDK_HOME or ANDROID_SDK_PATH to the Android NDK root" >&2
    exit 1
fi

HOST_UNAME=$(uname -s | tr '[:upper:]' '[:lower:]')
HOST_ARCH=$(uname -m)
case "${HOST_ARCH}" in
    x86_64|amd64) HOST_ARCH="x86_64" ;;
    arm64|aarch64) HOST_ARCH="arm64" ;;
esac
HOST_TAG="${HOST_UNAME}-${HOST_ARCH}"
TOOLCHAIN="${SDK_PATH}/toolchains/llvm/prebuilt/${HOST_TAG}"
if [[ ! -d "${TOOLCHAIN}" ]]; then
    echo "NDK toolchain not found at ${TOOLCHAIN}" >&2
    exit 1
fi
export PATH="${TOOLCHAIN}/bin:${PATH}"
SYSROOT="${TOOLCHAIN}/sysroot"

case "${ARCH}" in
    armv7)
        TARGET_TRIPLE="armv7a-linux-androideabi21"
        CONFIG_TARGET="armv7-android-gcc"
        CONFIG_PREFIX="armv7-android"
        CFLAGS_ARCH="-march=armv7-a -mfloat-abi=softfp -mfpu=vfp -D__ANDROID_API__=21"
        ;;
    arm64)
        TARGET_TRIPLE="aarch64-linux-android21"
        CONFIG_TARGET="arm64-android-gcc"
        CONFIG_PREFIX="arm64-android"
        CFLAGS_ARCH="-march=armv8-a -D__ANDROID_API__=21"
        ;;
    *)
        echo "Unknown arch: ${ARCH}" >&2
        usage
        ;;
esac

BUILD_DIR="build-${ARCH}-android"

mkdir -p "${BUILD_DIR}"
pushd "${BUILD_DIR}" >/dev/null

if [[ -f Makefile ]]; then
    make clean
fi

CC="${TOOLCHAIN}/bin/${TARGET_TRIPLE}-clang" \
CXX="${TOOLCHAIN}/bin/${TARGET_TRIPLE}-clang++" \
AR="${TOOLCHAIN}/bin/llvm-ar" \
NM="${TOOLCHAIN}/bin/llvm-nm" \
STRIP="${TOOLCHAIN}/bin/llvm-strip" \
CFLAGS="--sysroot=${SYSROOT} --target=${TARGET_TRIPLE} ${CFLAGS_ARCH}" \
LDFLAGS="--sysroot=${SYSROOT} --target=${TARGET_TRIPLE}" \
EXTRA_CFLAGS="--sysroot=${SYSROOT} --target=${TARGET_TRIPLE}" \
../configure \
    --prefix="${CONFIG_PREFIX}" \
    --target="${CONFIG_TARGET}" \
    --disable-neon \
    --disable-neon-asm \
    --disable-examples \
    --disable-unit-tests \
    --disable-docs \
    --disable-tools \
    --enable-pic

make -j"$(nproc)"

popd >/dev/null
