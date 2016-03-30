# build/os-auto.mak.  Generated from os-auto.mak.in by configure.

export OS_CFLAGS   := $(CC_DEF)PJ_AUTOCONF=1 -I${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/platforms/android-18/arch-arm/usr/include -I${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/sources/cxx-stl/gnu-libstdc++/4.7/include -I${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/sources/cxx-stl/gnu-libstdc++/4.7/libs/armeabi/include -fexceptions -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1 -I${FORGE_ANDROID_ROOT}/vendor/forge/cubic/pjproject-2.4/openssl-include -fno-short-enums

export OS_CXXFLAGS := $(CC_DEF)PJ_AUTOCONF=1 -I${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/platforms/android-18/arch-arm/usr/include -I${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/sources/cxx-stl/gnu-libstdc++/4.7/include -I${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/sources/cxx-stl/gnu-libstdc++/4.7/libs/armeabi/include -fexceptions -shared --sysroot=${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/platforms/android-18/arch-arm -fexceptions -fno-short-enums

export OS_LDFLAGS  := -nostdlib -L${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/platforms/android-18/arch-arm/usr/lib -L${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/sources/cxx-stl/gnu-libstdc++/4.7/libs/armeabi -lm -L${FORGE_ANDROID_ROOT}/vendor/forge/cubic/pjproject-2.4/openssl-lib  -lssl -lcrypto -ldl -lz -lm ${FORGE_ANDROID_ROOT}/prebuilts/ndk/9/platforms/android-18/arch-arm/usr/lib/crtbegin_so.o -lgnustl_static -lc -lgcc -lOpenSLES -llog -lGLESv2 -lEGL -landroid

export OS_SOURCES  := 


