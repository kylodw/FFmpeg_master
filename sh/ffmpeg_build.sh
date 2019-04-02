make clean
export TMPDIR=/Users/kylodw/ffmpeg-3.4.5/android/temp
export NDK=/Users/kylodw/android-ndk-r15c
SYSROOT=$NDK/platforms/android-26/arch-arm/
TOOLCHAIN=$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64
export CPU=arm
export PREFIX=/Users/kylodw/ffmpeg-3.4.5/android/result
export ADDI_CFLAGS="-marm"

./configure \
--target-os=linux \
--prefix=$PREFIX \
--arch=arm \
--disable-doc \
--enable-shared \
--disable-static \
--disable-yasm \
--disable-symver \
--enable-gpl \
--disable-ffmpeg \
--disable-ffplay \
--disable-ffprobe \
--disable-doc \
--disable-ffserver \
--disable-linux-perf \
--cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- \
--enable-cross-compile \
--sysroot=$SYSROOT \
--extra-cflags="-Os -fpic $ADDI_CFLAGS" \
--extra-ldflags="$ADDI_CFLAGS" \
$ADDITIONAL_CONFIGURE_FLAG
