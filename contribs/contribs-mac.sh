#!/bin/sh

set -x
echo "Contribs configured for intel-universal builds"

LIBVLC_VERSION="2.0.4"
LIBVLC_VERSION_PREFIX="libvlc-${LIBVLC_VERSION}-macosx"
LIBVLC_FILE="${LIBVLC_VERSION_PREFIX}.zip"
LIBVLC_URL="http://bhaisaab.org/contribs/${LIBVLC_FILE}"

FREI0R_FILE="frei0r-plugins-1.2.1.tar.gz"
FREI0R_URL="http://www.piksel.no/frei0r/releases/frei0r-plugins-1.2.1.tar.gz"
FREI0R_EFFECTS_FILE="frei0r-effects-macosx.zip"
FREI0R_EFFECTS_URL="http://bhaisaab.org/contribs/frei0r-effects-macosx.zip"

QT4_FILE="qt4-4.8-win32-bin.tar.bz2"
QT4_URL="http://bhaisaab.org/files/contribs/${QT4_FILE}"

ROOT_FOLDER=`pwd`

if [ -z `which lrelease` ]; then
    echo "The process require lrelease built in Qt !!!";
#    exit 1;
fi

# Get the dependencies in this directory
mkdir -p src-dl/
cd src-dl/

if [ ! -f $LIBVLC_FILE ]; then
    curl -C - -O $LIBVLC_URL ;
else
    echo "LIBVLC OK";
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

if [ ! -f $QT4_FILE ]; then
    curl -C - -O $QT4_URL ;
else
    echo "Qt4 OK";
fi

cd $ROOT_FOLDER
mkdir temp

# libvlc
unzip src-dl/$LIBVLC_FILE "libvlc*" -d temp

cd temp/libvlc-mac/
mv -fv include/ $ROOT_FOLDER
mv -fv lib/ $ROOT_FOLDER
mv -fv plugins/ $ROOT_FOLDER

cd $ROOT_FOLDER

# frei0r
tar xvf src-dl/$FREI0R_FILE -C temp --strip-components=2
cp temp/frei0r.h include/
unzip src-dl/$FREI0R_EFFECTS_FILE

# qt translations
tar xvf src-dl/$QT4_FILE -C temp --strip-components=1
mkdir -p ts
lrelease -compress -silent -nounfinished temp/ts/*.ts
mv temp/ts/*.qm ts/
rm -rf temp
