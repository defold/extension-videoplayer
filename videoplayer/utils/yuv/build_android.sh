
# put this in libvpx/third_party/libwbm and cd there

function terminate_usage {
	echo "USAGE: ./build_android.sh <arch>"
	exit 1
}

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

ARCH_FLAGS="-mfloat-abi=softfp -mfpu=neon"

CPP_FLAGS="$ARCH_FLAGS -O2 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -Wno-extern-c-compat -I../../.."

INCLUDES="-I../../../include -I$NDK_ROOT/platforms/android-14/arch-arm/usr/include -I$NDK_ROOT/sources/cxx-stl/gnu-libstdc++/4.8/include -I$NDK_ROOT/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a/include"

OUTDIR=build/android/$ARCH

mkdir -p $OUTDIR
pushd $OUTDIR

for f in source/convert.cc source/convert_argb.cc source/convert_from.cc source/convert_from_argb.cc source/convert_jpeg.cc source/convert_to_argb.cc source/convert_to_i420.cc source/cpu_id.cc source/mjpeg_decoder.cc source/mjpeg_validate.cc source/planar_functions.cc source/rotate.cc source/rotate_any.cc source/rotate_argb.cc source/rotate_common.cc source/rotate_gcc.cc source/row_any.cc source/row_common.cc source/row_gcc.cc source/scale.cc source/scale_any.cc source/scale_argb.cc source/scale_common.cc source/scale_gcc.cc source/video_common.cc source/scale_neon.cc source/rotate_neon.cc source/row_neon.cc
do
	OUT=$(basename $f).cpp_0.o
	echo "Writing" $OUT
    $GCC -c ../../../$f --sysroot=$SYSROOT -o $OUT $CPP_FLAGS $INCLUDES
done

$AR rcs libyuv.a convert.cc.cpp_0.o convert_argb.cc.cpp_0.o convert_from.cc.cpp_0.o convert_from_argb.cc.cpp_0.o convert_jpeg.cc.cpp_0.o convert_to_argb.cc.cpp_0.o convert_to_i420.cc.cpp_0.o cpu_id.cc.cpp_0.o mjpeg_decoder.cc.cpp_0.o mjpeg_validate.cc.cpp_0.o planar_functions.cc.cpp_0.o rotate.cc.cpp_0.o rotate_any.cc.cpp_0.o rotate_argb.cc.cpp_0.o rotate_common.cc.cpp_0.o rotate_gcc.cc.cpp_0.o row_any.cc.cpp_0.o row_common.cc.cpp_0.o row_gcc.cc.cpp_0.o scale.cc.cpp_0.o scale_any.cc.cpp_0.o scale_argb.cc.cpp_0.o scale_common.cc.cpp_0.o scale_gcc.cc.cpp_0.o video_common.cc.cpp_0.o scale_neon.cc.cpp_0.o rotate_neon.cc.cpp_0.o row_neon.cc.cpp_0.o

ls -la libyuv.a

popd
