
#!/usr/bin/env bash

# https://groups.google.com/a/webmproject.org/forum/#!topic/webm-discuss/zpy_lgS6K-8

set -euo pipefail

SDK_PATH="${ANDROID_NDK_HOME:-${ANDROID_SDK_PATH:-${ANDROID_HOME:-}}}"
if [[ -z "${SDK_PATH}" ]]; then
    echo "Set ANDROID_NDK_HOME or ANDROID_SDK_PATH to the Android NDK root" >&2
    exit 1
fi

BUILD_DIR="build-armv7-android"

mkdir -p "${BUILD_DIR}"
pushd "${BUILD_DIR}" >/dev/null

if [[ -f Makefile ]]; then
    make clean
fi

CFLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=vfp" \
../configure \
    --prefix=armv7-android \
    --target=armv7-android-gcc \
    --disable-neon \
    --disable-neon-asm \
    --disable-examples \
    --disable-unit-tests \
    --disable-docs \
    --disable-tools \
    --sdk-path="${SDK_PATH}"

make -j"$(nproc)"

popd >/dev/null
