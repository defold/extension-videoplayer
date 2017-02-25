

## libwebm



### iOS

Put this in libvpx/third_party/libwbm and cd there

    ARM_DARWIN_ROOT='/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer'
    IOS_SDK_VERSION="9.3"

    CPP_FLAGS="-m64 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -Wno-extern-c-compat -miphoneos-version-min=5.1"

    mkdir build
    pushd build

    for f in common/file_util.cc common/hdr_util.cc mkvparser/mkvparser.cc mkvparser/mkvreader.cc mkvmuxer/mkvmuxer.cc mkvmuxer/mkvmuxerutil.cc mkvmuxer/mkvwriter.cc
    do
        clang++ -arch arm64 -isysroot $ARM_DARWIN_ROOT/SDKs/iPhoneOS$IOS_SDK_VERSION.sdk -c ../$f -o $(basename $f).cpp_0.o $CPP_FLAGS -I../
    done

    ar rcs libwebm.a file_util.cc.cpp_0.o hdr_util.cc.cpp_0.o mkvparser.cc.cpp_0.o mkvreader.cc.cpp_0.o mkvmuxer.cc.cpp_0.o mkvmuxerutil.cc.cpp_0.o mkvwriter.cc.cpp_0.o

    ls -la libwebm.a

    popd


### macOS



### Building library

    CPP_FLAGS="-D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -Wno-extern-c-compat"

    mkdir build
    pushd build

    for f in common/file_util.cc common/hdr_util.cc mkvparser/mkvparser.cc mkvparser/mkvreader.cc mkvmuxer/mkvmuxer.cc mkvmuxer/mkvmuxerutil.cc mkvmuxer/mkvwriter.cc
    do
        clang++ -c ../$f -o $(basename $f).cpp_0.o $CPP_FLAGS -I../
    done

    ar rcs libwebm.a file_util.cc.cpp_0.o hdr_util.cc.cpp_0.o mkvparser.cc.cpp_0.o mkvreader.cc.cpp_0.o mkvmuxer.cc.cpp_0.o mkvmuxerutil.cc.cpp_0.o mkvwriter.cc.cpp_0.o

    popd





