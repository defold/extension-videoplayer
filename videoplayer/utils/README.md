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

## WebM

The webm library is such a small part, it makes perfect sense to compile it yourself with a custom shell script

You'll find examples of such scripts in the `WebM` folder

The scripts are expected to be run from the libvpx/third_party/libwebm folder.
