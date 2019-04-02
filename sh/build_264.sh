#!/bin/bash

export NDK=/Users/kylodw/android-ndk-r15c
export SYSROOT=$NDK/platforms/android-26/arch-arm/
export TOOLCHAIN=$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64
export CUP=arm
export PREFIX=$(pwd)/android/$CPU
function build_x264
{
./configure \
--prefix=$PREFIX \
--enable-static \
--enable-debug \
--disable-asm \
--enable-pic \
--host=arm-linux \
--cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- \
--sysroot=$SYSROOT
make clean
make
make install
}
build_x264