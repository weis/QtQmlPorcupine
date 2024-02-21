#!/bin/sh

ARCHS="arm64 x86_64"

# directories
FAT_DIR=`pwd`/"lib/mac/universal"
LIB_OUT=$FAT_DIR/"libpv_porcupine.dylib"
mkdir -p $FAT_DIR

# must be an absolute path
LIB_ARM=`pwd`/"lib/mac/arm64/libpv_porcupine.dylib"
LIB_x86_64=`pwd`/"lib/mac/x86_64/libpv_porcupine.dylib"

echo "building fat binaries..."
set - $ARCHS
lipo $LIB_x86_64 $LIB_ARM -create -output $LIB_OUT
