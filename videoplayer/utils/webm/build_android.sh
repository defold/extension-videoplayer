
# put this in libvpx/third_party/libwbm and cd there

function terminate_usage {
	echo "USAGE: ./build_android.sh <arch>"
	exit 1
}


ARM_DARWIN_ROOT='/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer'

#android/android-ndk-r10e/toolchains/arm-linux-androideabi-4.8/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-g++


NDK_ROOT=~/android/android-ndk-r10e

SYSROOT=$NDK_ROOT/platforms/android-14/arch-arm

echo $SYSROOT

GCC=$NDK_ROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-g++
AR=$NDK_ROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-ar

echo $SYSROOT

NDK_VERSION="14"

ARCH="${1:-}" && [ ! -z "${ARCH}" ] || terminate_usage

# if [ "$ARCH" == "arm64" ]
# then
# 	CPP_FLAGS="-m64 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -Wno-extern-c-compat"
# else
# 	CPP_FLAGS="-m32 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -Wno-extern-c-compat"
# fi

ARCH_FLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=vfp"

CPP_FLAGS="$ARCH_FLAGS -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -Wno-extern-c-compat -I../../.."

OUTDIR=build/android/$ARCH

mkdir -p $OUTDIR
pushd $OUTDIR

for f in ../../common/file_util.cc ../../common/hdr_util.cc ../../mkvparser/mkvparser.cc ../../mkvparser/mkvreader.cc ../../mkvmuxer/mkvmuxer.cc ../../mkvmuxer/mkvmuxerutil.cc ../../mkvmuxer/mkvwriter.cc
do
    $GCC -c ../$f --sysroot=$SYSROOT -o $(basename $f).cpp_0.o $CPP_FLAGS -I../../ -I$NDK_ROOT/platforms/android-14/arch-arm/usr/include -I$NDK_ROOT/sources/cxx-stl/gnu-libstdc++/4.8/include -I$NDK_ROOT/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a/include
done

$AR rcs libwebm.a file_util.cc.cpp_0.o hdr_util.cc.cpp_0.o mkvparser.cc.cpp_0.o mkvreader.cc.cpp_0.o mkvmuxer.cc.cpp_0.o mkvmuxerutil.cc.cpp_0.o mkvwriter.cc.cpp_0.o

ls -la libwebm.a

popd
