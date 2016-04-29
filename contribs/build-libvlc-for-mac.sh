#!/bin/sh
set -e

info()
{
    local green="\033[1;32m"
    local normal="\033[0m"
    echo "[${green}build${normal}] $1"
}

OSX_VERSION="10.11"
ARCH="x86_64"
MINIMAL_OSX_VERSION="10.7"
SDKROOT=`xcode-select -print-path`/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$OSX_VERSION.sdk
UNSTABLE=no

usage()
{
cat << EOF
usage: $0 [-v] [-d]

OPTIONS
   -v            Be more verbose
   -u            Use unstable libvlc
   -k <sdk>      Use the specified sdk (default: $SDKROOT for $ARCH)
   -a <arch>     Use the specified arch (default: $ARCH)
EOF
}

spushd()
{
     pushd "$1" 2>&1> /dev/null
}

spopd()
{
     popd 2>&1> /dev/null
}

info()
{
     local green="\033[1;32m"
     local normal="\033[0m"
     echo "[${green}info${normal}] $1"
}

while getopts "hvua:k:" OPTION
do
     case $OPTION in
         h)
             usage
             exit 1
             ;;
         v)
             VERBOSE=yes
             ;;
         u)
             UNSTABLE=yes
             ;;
         a)
             ARCH=$OPTARG
             ;;
         k)
             SDKROOT=$OPTARG
             ;;
         ?)
             usage
             exit 1
             ;;
     esac
done
shift $(($OPTIND - 1))

out="/dev/null"
if [ "$VERBOSE" = "yes" ]; then
   out="/dev/stdout"
fi

if [ "x$1" != "x" ]; then
    usage
    exit 1
fi

export OSX_VERSION
export SDKROOT

# Get root dir
spushd .
vlmc_root_dir=`pwd`
spopd

info $vlmc_root_dir

info "Preparing build dirs"

spushd contribs

if ! [ -e vlc ]; then
if [ "$UNSTABLE" = "yes" ]; then
git clone git://git.videolan.org/vlc.git vlc
else
git clone git://git.videolan.org/vlc/vlc-2.2.git vlc
fi
fi

spopd #contribs

#
# Build time
#

export PATH="${vlmc_root_dir}/contribs/vlc/extras/tools/build/bin:${vlmc_root_dir}/contribs/contrib/${ARCH}-apple-darwin11/bin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/X11/bin"

info "Building tools"
spushd contribs/vlc/extras/tools
if ! [ -e build ]; then
./bootstrap && make
fi
spopd

info "Fetching contrib"

spushd contribs/vlc/contrib

if ! [ -e ${ARCH}-vlmc ]; then
mkdir ${ARCH}-vlmc
fi
cd ${ARCH}-vlmc
../bootstrap --build=${ARCH}-apple-darwin11
make prebuilt
spopd

export CC="xcrun clang"
export CXX="xcrun clang++"
export OBJC="xcrun clang"
PREFIX="${vlmc_root_dir}/contribs/vlc/${ARCH}-install"

info "Configuring VLC"

if ! [ -e ${PREFIX} ]; then
    mkdir ${PREFIX}
fi

spushd contribs/vlc
if ! [ -e configure ]; then
    ./bootstrap > ${out}
fi
if ! [ -e ${ARCH}-build ]; then
    mkdir ${ARCH}-build
fi

CONFIG_OPTIONS=""
if [ "$ARCH" = "i686" ]; then
    CONFIG_OPTIONS="--disable-vda"
    export LDFLAGS="-Wl,-read_only_relocs,suppress"
fi

cd ${ARCH}-build
../configure \
        --build=${ARCH}-apple-darwin11 \
        --prefix=${PREFIX} \
        --with-macosx-version-min=${MINIMAL_OSX_VERSION} \
        --with-macosx-sdk=$SDKROOT \
        --disable-lua --disable-httpd --disable-vlm \
        --disable-vcd --disable-screen \
        --disable-debug \
        --disable-macosx \
        --disable-notify \
        --disable-projectm \
        --disable-growl \
        --disable-faad \
        --disable-bluray \
        --enable-flac \
        --enable-theora \
        --enable-shout \
        --disable-ncurses \
        --disable-twolame \
        --disable-realrtsp \
        --enable-libass \
        --disable-macosx-avfoundation \
        --disable-macosx-dialog-provider \
        --disable-macosx-eyetv \
        --disable-macosx-qtkit \
        --disable-macosx-quartztext \
        --disable-macosx-vlc-app \
        --disable-skins2 \
        --disable-xcb \
        --disable-caca \
        --disable-sdl \
        --disable-samplerate \
        --disable-upnp \
        --disable-goom \
        --disable-nls \
        --disable-sdl \
        --disable-sdl-image \
        ${CONFIG_OPTIONS} \
         > ${out}

info "Compiling VLC"

CORE_COUNT=`sysctl -n machdep.cpu.core_count`
let MAKE_JOBS=$CORE_COUNT+1

if [ "$VERBOSE" = "yes" ]; then
    make V=1 -j$MAKE_JOBS > ${out}
else
    make -j$MAKE_JOBS > ${out}
fi

info "Installing VLC"
make install > ${out}
cd ..

info "Removing unneeded modules"
blacklist="
stats
access_bd
shm
oldrc
real
hotkeys
gestures
sap
dynamicoverlay
rss
ball
magnify
audiobargraph_
clone
mosaic
osdmenu
puzzle
mediadirs
ripple
motion
sharpen
grain
posterize
mirror
wall
scene
blendbench
psychedelic
alphamask
netsync
audioscrobbler
motiondetect
motionblur
export
smf
podcast
bluescreen
erase
remoteosd
magnify
gradient
logger
visual
fb
aout_file
dummy
invert
sepia
wave
hqdn3d
headphone_channel_mixer
gaussianblur
gradfun
extract
colorthres
antiflicker
anaglyph
remap
"

for i in ${blacklist}
do
    find ${PREFIX}/lib/vlc/plugins -name *$i* -type f -exec rm '{}' \;
done

spopd

info "Build completed"
