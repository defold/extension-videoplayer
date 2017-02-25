
# put this in libvpx/third_party/libwbm and cd there

function terminate_usage {
	echo "USAGE: ./build_osx.sh <arch>"
	exit 1
}

ARCH="${1:-}" && [ ! -z "${ARCH}" ] || terminate_usage

if [ "$ARCH" == "x86_64" ]
then
	ARCH_FLAGS="-arch $ARCH -m64"
else
	ARCH_FLAGS="-arch i386 -m32"
fi


CPP_FLAGS="$ARCH_FLAGS -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -Wno-extern-c-compat -mmacosx-version-min=10.7  -I../../.."

OUTDIR=build/osx/$ARCH

mkdir -p $OUTDIR
pushd $OUTDIR

for f in ../../common/file_util.cc ../../common/hdr_util.cc ../../mkvparser/mkvparser.cc ../../mkvparser/mkvreader.cc ../../mkvmuxer/mkvmuxer.cc ../../mkvmuxer/mkvmuxerutil.cc ../../mkvmuxer/mkvwriter.cc
do
    clang++ -c ../$f -o $(basename $f).cpp_0.o $CPP_FLAGS
done

ar rcs libwebm.a file_util.cc.cpp_0.o hdr_util.cc.cpp_0.o mkvparser.cc.cpp_0.o mkvreader.cc.cpp_0.o mkvmuxer.cc.cpp_0.o mkvmuxerutil.cc.cpp_0.o mkvwriter.cc.cpp_0.o

ls -la libwebm.a

popd
