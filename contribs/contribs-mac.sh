#!/bin/sh

VLC_ARCH=""
VLC_BUILD_ID="0109"
if [ `uname -m` != "i386" ]; then
    VLC_ARCH="64";
    VLC_BUILD_ID="0210";
fi
echo "Contribs configured for" `uname -m` "architecture"

VLC_VERSION_DATE="20110719-${VLC_BUILD_ID}"
VLC_VERSION_PREFIX="1.1-branch-intel${VLC_ARCH}-${VLC_VERSION_DATE}"
VLC_FILE="${VLC_VERSION_PREFIX}.zip"
VLC_URL="http://nightlies.videolan.org/build/macosx-intel/${VLC_FILE}"
FREI0R_FILE="frei0r-plugins-1.2.1.tar.gz"
FREI0R_URL="http://www.piksel.no/frei0r/releases/frei0r-plugins-1.2.1.tar.gz"
FREI0R_EFFECTS_FILE="effects${VLC_ARCH}.zip"
FREI0R_EFFECTS_URL="http://dl.rohityadav.in/contribs/effects${VLC_ARCH}.zip"

ROOT_FOLDER=`pwd`

# Get the dependencies in this directory
mkdir -p src-dl/
cd src-dl/

if [ ! -f $VLC_FILE ]; then
    curl -C - -O $VLC_URL ;
else
    echo "VLC OK";
fi

if [ ! -f $FREI0R_FILE ]; then
    curl -C - -O $FREI0R_URL ;
else
    echo "FREI0R OK";
fi

if [ ! -f $FREI0R_EFFECTS_FILE ]; then
    curl -C - -O $FREI0R_EFFECTS_URL ;
else
    echo "FREI0R OK";
fi

cd $ROOT_FOLDER
mkdir temp

# libvlc
unzip src-dl/$VLC_FILE "vlc*/VLC.app/Contents/MacOS/*" -d temp

cd temp/vlc*/VLC.app/Contents/MacOS/
mv -fv include/ $ROOT_FOLDER
mv -fv lib/ $ROOT_FOLDER
mv -fv plugins/ $ROOT_FOLDER

cd $ROOT_FOLDER

# frei0r
tar xvf src-dl/$FREI0R_FILE -C temp --strip-components=2
cp temp/frei0r.h include
unzip src-dl/$FREI0R_EFFECTS_FILE
rm -rf temp
