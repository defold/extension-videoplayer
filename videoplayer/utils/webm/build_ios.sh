#!/usr/bin/env bash

# Build libwebm for iOS. Run from libvpx/third_party/libwebm:
#   bash path/to/videoplayer/utils/webm/build_ios.sh <arch>
# Supported arches: arm64

set -euo pipefail

usage() {
    echo "USAGE: ./build_ios.sh <arch>" >&2
    echo "  arch: arm64" >&2
    exit 1
}

ARCH="${1:-}" && [[ -n "${ARCH}" ]] || usage

IOS_SDK=$(xcrun --sdk iphoneos --show-sdk-path)
MIN_IOS_VERSION="${MIN_IOS_VERSION:-12.0}"

if [[ "${ARCH}" != "arm64" ]]; then
    echo "Unknown arch: ${ARCH}" >&2
    usage
fi

ARCH_FLAGS="-arch ${ARCH}"

CPP_FLAGS="${ARCH_FLAGS} -std=c++11 -fembed-bitcode -miphoneos-version-min=${MIN_IOS_VERSION} -isysroot ${IOS_SDK} -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -Wno-extern-c-compat -I../../.."

OUTDIR="build/ios/${ARCH}"

mkdir -p "${OUTDIR}"
pushd "${OUTDIR}" >/dev/null

for f in ../../common/file_util.cc ../../common/hdr_util.cc ../../mkvparser/mkvparser.cc ../../mkvparser/mkvreader.cc ../../mkvmuxer/mkvmuxer.cc ../../mkvmuxer/mkvmuxerutil.cc ../../mkvmuxer/mkvwriter.cc
do
    clang++ -c "../${f}" -o "$(basename "${f}").cpp_0.o" ${CPP_FLAGS}
done

ar rcs libwebm.a file_util.cc.cpp_0.o hdr_util.cc.cpp_0.o mkvparser.cc.cpp_0.o mkvreader.cc.cpp_0.o mkvmuxer.cc.cpp_0.o mkvmuxerutil.cc.cpp_0.o mkvwriter.cc.cpp_0.o

ls -la libwebm.a

popd >/dev/null
