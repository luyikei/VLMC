#! /bin/sh

QT4_FILE="qt4-4.7.1-win32-bin.tar.bz2"
QT4_URL="http://download.videolan.org/pub/videolan/contrib/qt4-4.7.1-win32-bin.tar.bz2"
VLC_VERSION_DATE="20101031-0202"
VLC_VERSION_PREFIX="vlc-1.1.5-git-${VLC_VERSION_DATE}"
VLC_FILE="${VLC_VERSION_PREFIX}-win32.7z"
VLC_URL="http://nightlies.videolan.org/build/win32/branch-${VLC_VERSION_DATE}/${VLC_FILE}"
FREI0R_FILE="frei0r-latest.tar.gz"
FREI0R_URL="http://www.piksel.no/frei0r/snapshot/frei0r-latest.tar.gz"

ROOT_FOLDER=`pwd`

# Get the dependencies, aka VLC+Qt
mkdir -p src-dl/
cd src-dl/
if [ ! -f $QT4_FILE ]; then
    wget $QT4_URL ;
else
    echo "Qt4 OK";
fi
if [ ! -f $VLC_FILE ]; then
    wget $VLC_URL ;
else
    echo "VLC OK";
fi
if [ ! -f $FREI0R_FILE ]; then
    wget $FREI0R_URL ;
else
    echo "FREI0R OK";
fi
cd $ROOT_FOLDER


# bin and dlls
mkdir bin && mkdir include && mkdir temp

7z e src-dl/$VLC_FILE "$VLC_VERSION_PREFIX/libvlc.dll" -otemp
7z e src-dl/$VLC_FILE "$VLC_VERSION_PREFIX/libvlccore.dll" -otemp
7z e src-dl/$VLC_FILE "$VLC_VERSION_PREFIX/plugins/*" -otemp/plugins
cd temp
  for i in libvlc.dll libvlccore.dll; do
    cp -v $i $ROOT_FOLDER/bin/
  done
  cd plugins
    for i in libqt4_plugin.dll libskins2_plugin.dll libstream_out_raop_plugin.dll libvout_sdl_plugin.dll libaout_sdl_plugin.dll; do
        rm $i
    done
  cd ..
cd ..
cp -r temp/plugins/ $ROOT_FOLDER/bin/

rm -rf temp
cd $ROOT_FOLDER

#VLC sdk
7z x src-dl/$VLC_FILE "$VLC_VERSION_PREFIX/sdk"
mv -fuv $VLC_VERSION_PREFIX/sdk/include/vlc $ROOT_FOLDER/include/vlc
mv -fuv $VLC_VERSION_PREFIX/sdk/lib/ $ROOT_FOLDER/

# Qt
tar xvf src-dl/$QT4_FILE -C . --strip-components=1
cd include && ln -sf qt4/src && cd ..

#frei0r
tar xvf src-dl/$FREI0R_FILE --wildcards --no-anchored 'frei0r.h' --strip-components=1
