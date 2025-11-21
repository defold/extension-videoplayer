# How to build the libraries

Download the source from the [repository](https://github.com/webmproject/libvpx)
either as a zip or `git clone`

# Version

The libvpx version used is currently `v1.6.0-720-g186dc40`

# Build scripts

The scripts are here as a guideline, and are not guaranteed to work.
Feel free to contribute with improved versions!

## VPX

Most part of libvpx should be compilable using `configure` + `make -j8` and changing the `target` triplet accordingly.
Some example scripts can be found in the `vpx` folder.
The scripts are expected to be run from the libvpx folder.
- macOS universal: `bash videoplayer/utils/vpx/build_osx.sh x86_64` and `bash videoplayer/utils/vpx/build_osx.sh arm64`
- iOS (run from a libvpx checkout): `bash videoplayer/utils/vpx/build_ios.sh arm64`

## WebM

The webm library is such a small part, it makes perfect sense to compile it yourself with a custom shell script

You'll find examples of such scripts in the `WebM` folder

- macOS universal: from `third_party/libwebm`, run `bash videoplayer/utils/webm/build_osx.sh x86_64` and `bash videoplayer/utils/webm/build_osx.sh arm64`
- iOS: from `third_party/libwebm`, run `bash videoplayer/utils/webm/build_ios.sh arm64`

The scripts are expected to be run from the libvpx/third_party/libwebm folder.
