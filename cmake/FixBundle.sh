#!/bin/bash

if [ $# -lt 2 ] ; then
    echo "Usage: $0 <vlmc_binary_path> <libvlc_dir>"
    exit 1
fi

VLMC_BIN_DIR=$1
VLMC_BIN="$VLMC_BIN_DIR/vlmc"
LIBVLC_DIR=$2

install_name_tool -change @loader_path/lib/libvlc.5.dylib @executable_path/lib/libvlc.5.dylib $VLMC_BIN
install_name_tool -change @loader_path/lib/libvlccore.7.dylib @executable_path/lib/libvlccore.7.dylib $VLMC_BIN

ln -Fs $LIBVLC_DIR/../lib $VLMC_BIN_DIR
ln -Fs $LIBVLC_DIR/../plugins $VLMC_BIN_DIR
