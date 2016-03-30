export CC = ${FORGE_ANDROID_ROOT}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.8/bin/arm-linux-androideabi-gcc -c
export CXX = ${FORGE_ANDROID_ROOT}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.8/bin/arm-linux-androideabi-g++ -c
export AR = ${FORGE_ANDROID_ROOT}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.8/bin/arm-linux-androideabi-ar
export AR_FLAGS = rv
export LD = ${FORGE_ANDROID_ROOT}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.8/bin/arm-linux-androideabi-gcc
export LDOUT = -o 
export RANLIB = ${FORGE_ANDROID_ROOT}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.8/bin/arm-linux-androideabi-gcc-ranlib

export OBJEXT := .o
export LIBEXT := .a
export LIBEXT2 := 

export CC_OUT := -o 
export CC_INC := -I
export CC_DEF := -D
export CC_OPTIMIZE := -O2
export CC_LIB := -l

export CC_SOURCES :=
export CC_CFLAGS := -Wall
export CC_LDFLAGS := 
