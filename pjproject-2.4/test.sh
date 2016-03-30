#!/bin/bash
FORGE_ANDROID_ROOT=$1
export ANDROID_SYSROOT=${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/platforms/android-18/arch-arm
export CC=${FORGE_ANDROID_ROOT}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.7/bin/arm-linux-androideabi-gcc
export CXX=${FORGE_ANDROID_ROOT}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.7/bin/arm-linux-androideabi-g++
export AR=${FORGE_ANDROID_ROOT}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.7/bin/arm-linux-androideabi-ar
export RANLIB=${FORGE_ANDROID_ROOT}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.7/bin/arm-linux-androideabi-ranlib
export LDFLAGS="-nostdlib -L${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/platforms/android-18/arch-arm/usr/lib -L${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/sources/cxx-stl/gnu-libstdc++/4.7/libs/armeabi"
export LIBS="${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/platforms/android-18/arch-arm/usr/lib/crtbegin_so.o -lgnustl_static -lc -lgcc"
export CFLAGS="-I${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/platforms/android-18/arch-arm/usr/include -I${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/sources/cxx-stl/gnu-libstdc++/4.7/include -I${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/sources/cxx-stl/gnu-libstdc++/4.7/libs/armeabi/include -fexceptions -frtti"
export CPPFLAGS="-I${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/platforms/android-18/arch-arm/usr/include -fexceptions -frtti"
export CXXFLAGS="-shared --sysroot=${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/platforms/android-18/arch-arm -fexceptions -frtti"
export TARGET_HOST=arm-linux-androideabi
./configure --host=${TARGET_HOST}
