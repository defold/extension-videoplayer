
# https://groups.google.com/a/webmproject.org/forum/#!topic/webm-discuss/zpy_lgS6K-8

ANDROID_SDK=$ANDROID_HOME/android-ndk-r10e

set -e

mkdir -p build-armv7-android
pushd build-armv7-android

make clean
#../configure --prefix=armv7-android --target=armv7-android-gcc --disable-runtime-cpu-detect --disable-neon --disable-neon-asm --disable-examples --sdk-path=/Users/mathiaswesterdahl/android/android-ndk-r12b
CFLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=vfp" ../configure --prefix=armv7-android --target=armv7-android-gcc --disable-neon --disable-neon-asm --disable-examples --disable-unit-tests --disable-docs --disable-tools --sdk-path=$ANDROID_SDK
make -j8
#make install

popd
