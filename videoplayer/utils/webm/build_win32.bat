
@echo off

rem run from Visual Studio command prompt (32 or 64 bit)
rem put this in libvpx/third_party/libwbm and cd there

rem ARCH="${1:-}" && [ ! -z "${ARCH}" ] || terminate_usage

rem if [ "$ARCH" == "x86_64" ]
rem then
rem 	ARCH_FLAGS="-arch $ARCH -m64"
rem else
rem 	# i386
rem 	if [ "$ARCH" == "i386" ]
rem 	then
rem 		ARCH_FLAGS="-arch $ARCH -m32"
rem 	else
rem 		echo Unknown arch: $ARCH
rem 		exit 1
rem 	fi
rem fi


rem CPP_FLAGS="$ARCH_FLAGS -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -Wno-extern-c-compat -mmacosx-version-min=10.7  -I../../.."

IF NOT DEFINED Platform SET Platform=X86

SET OUTDIR=build-%Platform%

mkdir %OUTDIR%
cd %OUTDIR%

cl.exe /c /O2 /nologo /I.. /Z7 /MT  ../common/file_util.cc
cl.exe /c /O2 /nologo /I.. /Z7 /MT  ../common/hdr_util.cc
cl.exe /c /O2 /nologo /I.. /Z7 /MT  ../mkvparser/mkvparser.cc
cl.exe /c /O2 /nologo /I.. /Z7 /MT  ../mkvparser/mkvreader.cc
cl.exe /c /O2 /nologo /I.. /Z7 /MT  ../mkvmuxer/mkvmuxer.cc
cl.exe /c /O2 /nologo /I.. /Z7 /MT  ../mkvmuxer/mkvmuxerutil.cc
cl.exe /c /O2 /nologo /I.. /Z7 /MT  ../mkvmuxer/mkvwriter.cc

lib.exe /out:webm.lib *.obj

cd ..
