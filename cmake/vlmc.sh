#!/bin/sh

BIN="bin/vlmc"
if [ -f $BIN ]; then
    exec $BIN "$@"
else
    echo "VLMC not built, read INSTALL first"
fi
