
#!/usr/bin/env bash

# https://groups.google.com/a/webmproject.org/forum/#!topic/webm-discuss/zpy_lgS6K-8

set -euo pipefail

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
WRAPPER_DIR="$(pwd)/toolwrap"
mkdir -p "${WRAPPER_DIR}"
cat > "${WRAPPER_DIR}/arm-linux-androideabi-gcc" <<'EOF'
#!/usr/bin/env bash
exec armv7a-linux-androideabi21-clang "$@"
EOF
cat > "${WRAPPER_DIR}/arm-linux-androideabi-g++" <<'EOF'
#!/usr/bin/env bash
exec armv7a-linux-androideabi21-clang++ "$@"
EOF
chmod +x "${WRAPPER_DIR}/arm-linux-androideabi-gcc" "${WRAPPER_DIR}/arm-linux-androideabi-g++"
export PATH="${WRAPPER_DIR}:${PATH}"

BUILD_DIR="build-armv7-android"

mkdir -p "${BUILD_DIR}"
pushd "${BUILD_DIR}" >/dev/null

if [[ -f Makefile ]]; then
    make clean
fi

CC="${TOOLCHAIN}/bin/armv7a-linux-androideabi21-clang" \
CXX="${TOOLCHAIN}/bin/armv7a-linux-androideabi21-clang++" \
AR="${TOOLCHAIN}/bin/llvm-ar" \
NM="${TOOLCHAIN}/bin/llvm-nm" \
STRIP="${TOOLCHAIN}/bin/llvm-strip" \
CFLAGS="--sysroot=${SYSROOT} --target=armv7a-linux-androideabi21 -march=armv7-a -mfloat-abi=softfp -mfpu=vfp -D__ANDROID_API__=21" \
LDFLAGS="--sysroot=${SYSROOT} --target=armv7a-linux-androideabi21" \
EXTRA_CFLAGS="--sysroot=${SYSROOT} --target=armv7a-linux-androideabi21" \
../configure \
    --prefix=armv7-android \
    --target=armv7-android-gcc \
    --disable-neon \
    --disable-neon-asm \
    --disable-examples \
    --disable-unit-tests \
    --disable-docs \
    --disable-tools \
    --enable-pic

make -j"$(nproc)"

popd >/dev/null
