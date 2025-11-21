
#!/usr/bin/env bash

# Put this in libvpx/third_party/libwebm and cd there.

set -euo pipefail

usage() {
    echo "USAGE: ./build_osx.sh <arch>" >&2
    echo "  arch: x86_64 | arm64" >&2
    exit 1
}

ARCH="${1:-}" && [[ -n "${ARCH}" ]] || usage

case "${ARCH}" in
    x86_64)
        ARCH_FLAGS="-arch ${ARCH} -m64"
        MACOSX_VERSION="-mmacosx-version-min=10.13"
        ;;
    arm64)
        ARCH_FLAGS="-arch ${ARCH}"
        MACOSX_VERSION="-mmacosx-version-min=11.0"
        ;;
    *)
        echo "Unknown arch: ${ARCH}" >&2
        usage
        ;;
esac

CPP_FLAGS="${ARCH_FLAGS} -std=c++11 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -Wno-extern-c-compat ${MACOSX_VERSION} -I../../.."

OUTDIR="build/osx/${ARCH}"

mkdir -p "${OUTDIR}"
pushd "${OUTDIR}" >/dev/null

for f in ../../common/file_util.cc ../../common/hdr_util.cc ../../mkvparser/mkvparser.cc ../../mkvparser/mkvreader.cc ../../mkvmuxer/mkvmuxer.cc ../../mkvmuxer/mkvmuxerutil.cc ../../mkvmuxer/mkvwriter.cc
do
    clang++ -c "../${f}" -o "$(basename "${f}").cpp_0.o" ${CPP_FLAGS}
done

ar rcs libwebm.a file_util.cc.cpp_0.o hdr_util.cc.cpp_0.o mkvparser.cc.cpp_0.o mkvreader.cc.cpp_0.o mkvmuxer.cc.cpp_0.o mkvmuxerutil.cc.cpp_0.o mkvwriter.cc.cpp_0.o

ls -la libwebm.a

popd >/dev/null
