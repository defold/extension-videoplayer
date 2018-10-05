
# https://groups.google.com/a/webmproject.org/forum/#!topic/webm-discuss/zpy_lgS6K-8

set -e

mkdir -p build-x86-osx
pushd build-x86-osx

if [-e Makefile]
then
	make clean
fi

CFLAGS="-m32" ../configure --prefix=x86-osx --target=x86-darwin15-gcc --disable-examples --disable-unit-tests --disable-docs --disable-tools
make -j8
#make install

popd
