
# put this in libvpx/third_party/libwbm and cd there

function terminate_usage {
	echo "USAGE: ./build_ios.sh <arch>"
	exit 1
}


ARM_DARWIN_ROOT='/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer'
IOS_SDK_VERSION="9.3"

ARCH="${1:-}" && [ ! -z "${ARCH}" ] || terminate_usage

if [ "$ARCH" == "arm64" ]
then
	CPP_FLAGS="-m64 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -Wno-extern-c-compat -miphoneos-version-min=5.1"
else
	CPP_FLAGS="-m32 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -Wno-extern-c-compat -miphoneos-version-min=5.1"
fi

CPP_FLAGS="$ARCH_FLAGS -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -Wno-extern-c-compat -mmacosx-version-min=10.7  -I../../.."

OUTDIR=build/ios/$ARCH

mkdir -p $OUTDIR
pushd $OUTDIR

for f in ../../common/file_util.cc ../../common/hdr_util.cc ../../mkvparser/mkvparser.cc ../../mkvparser/mkvreader.cc ../../mkvmuxer/mkvmuxer.cc ../../mkvmuxer/mkvmuxerutil.cc ../../mkvmuxer/mkvwriter.cc
do
    clang++ -arch $ARCH -isysroot $ARM_DARWIN_ROOT/SDKs/iPhoneOS$IOS_SDK_VERSION.sdk -c ../$f -o $(basename $f).cpp_0.o $CPP_FLAGS -I../../
done

ar rcs libwebm.a file_util.cc.cpp_0.o hdr_util.cc.cpp_0.o mkvparser.cc.cpp_0.o mkvreader.cc.cpp_0.o mkvmuxer.cc.cpp_0.o mkvmuxerutil.cc.cpp_0.o mkvwriter.cc.cpp_0.o

ls -la libwebm.a

popd
