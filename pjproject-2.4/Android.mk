LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

FORGE_ANDROID_ROOT=$(shell pwd)
FORGE_PJDIR=$(shell pwd)/$(LOCAL_PATH)

$(shell echo "#!/bin/bash" > $(LOCAL_PATH)/clean.sh)
$(shell echo  >> $(LOCAL_PATH)/clean.sh)
$(shell echo "#Auto generate for clean,should not be commit to git!" >> $(LOCAL_PATH)/clean.sh)
$(shell echo  >> $(LOCAL_PATH)/clean.sh)
$(shell echo "export PJDIR=$(FORGE_PJDIR)" >> $(LOCAL_PATH)/clean.sh)
$(shell echo "export FORGE_ANDROID_ROOT=$(FORGE_ANDROID_ROOT)" >> $(LOCAL_PATH)/clean.sh)
$(shell echo "make distclean" >> $(LOCAL_PATH)/clean.sh)
$(shell chmod 777 $(LOCAL_PATH)/clean.sh)

$(shell export PJDIR=$(FORGE_PJDIR); \
		export FORGE_ANDROID_ROOT=$(FORGE_ANDROID_ROOT); \
		make dep -C $(LOCAL_PATH); \
		make clean -C $(LOCAL_PATH); \
		make -C $(LOCAL_PATH))

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/pjsip/include \
					$(LOCAL_PATH)/pjlib-util/include \
					$(LOCAL_PATH)/pjlib/include \
					$(LOCAL_PATH)/third_party/resample/include \
					$(LOCAL_PATH)/third_party/speex/include \
					$(LOCAL_PATH)/third_party/bdsound/include \
					$(LOCAL_PATH)/third_party/srtp/include \
					$(LOCAL_PATH)/third_party/portaudio/include \
					$(LOCAL_PATH)/pjnath/include \
					$(LOCAL_PATH)/pjmedia/include \
					$(LOCAL_PATH)/third_party/srtp/crypto/include \
					$(LOCAL_PATH)/third_party/speex/libspeex \
					$(LOCAL_PATH)/openssl-include \
					$(LOCAL_PATH)/third_party/gsm/inc \
					$(LOCAL_PATH)/pjsipd \
					$(LOCAL_PATH)/pjsipd/util \
					$(TOP)/prebuilts/ndk/8/sources/cxx-stl/stlport/stlport \
					$(TOP)/prebuilts/ndk/9/platforms/android-18/arch-arm/usr/include/SLES
          
LOCAL_SRC_FILES := pjsipd/main.cpp \
				   pjsipd/SipService.cc \
				   pjsipd/pjsua_app.cpp \
				   pjsipd/pjsua_app_common.cpp \
				   pjsipd/pjsua_app_config.cpp \
				   pjsipd/pjsua_app_legacy.cpp \
				   pjsipd/SipCaller.cpp \

LOCAL_SHARED_LIBRARIES := libui \
						  libcutils \
						  libutils \
						  libbinder \
						  libsonivox \
						  libicuuc \
						  libexpat \
						  libOpenSLES \
						  libdl 

#LOCAL_STATIC_LIBRARIES : = libstlport_static
         
LOCAL_LDFLAGS += \
				 $(LOCAL_PATH)/pjsip/lib/libpjsua-arm-unknown-linux-androideabi.a \
				 $(LOCAL_PATH)/pjmedia/lib/libpjmedia-audiodev-arm-unknown-linux-androideabi.a \
				 $(LOCAL_PATH)/pjmedia/lib/libpjmedia-codec-arm-unknown-linux-androideabi.a \
				 $(LOCAL_PATH)/pjmedia/lib/libpjmedia-arm-unknown-linux-androideabi.a \
				 $(LOCAL_PATH)/third_party/lib/libsrtp-arm-unknown-linux-androideabi.a \
				 $(LOCAL_PATH)/third_party/lib/libspeex-arm-unknown-linux-androideabi.a \
				 $(LOCAL_PATH)/pjmedia/lib/libpjmedia-videodev-arm-unknown-linux-androideabi.a \
				 $(LOCAL_PATH)/pjnath/lib/libpjnath-arm-unknown-linux-androideabi.a \
				 $(LOCAL_PATH)/pjmedia/lib/libpjsdp-arm-unknown-linux-androideabi.a \
				 $(LOCAL_PATH)/pjsip/lib/libpjsip-simple-arm-unknown-linux-androideabi.a \
				 $(LOCAL_PATH)/pjsip/lib/libpjsip-ua-arm-unknown-linux-androideabi.a \
				 $(LOCAL_PATH)/pjsip/lib/libpjsip-arm-unknown-linux-androideabi.a \
				 $(LOCAL_PATH)/pjsip/lib/libpjsua2-arm-unknown-linux-androideabi.a \
				 $(LOCAL_PATH)/pjlib/lib/libpj-arm-unknown-linux-androideabi.a \
				 $(LOCAL_PATH)/pjlib-util/lib/libpjlib-util-arm-unknown-linux-androideabi.a \
				 $(LOCAL_PATH)/third_party/lib/libresample-arm-unknown-linux-androideabi.a \
				 $(LOCAL_PATH)/openssl-lib/libssl.a \
				 $(LOCAL_PATH)/openssl-lib/libcrypto.a \
				 $(TOP)/prebuilts/ndk/8/sources/cxx-stl/stlport/libs/armeabi/libstlport_static.a
				 
LOCAL_LDLIBS := \
				-ldl \
				-lz \
				-lm 

EXTRA_LDLIBS := -lrt -lpthread

LOCAL_CPPFLAGS := -DPJSIP_HAS_TLS_TRANSPORT=1 -fpermissive -fno-short-enums

LOCAL_MODULE := sipd
include $(BUILD_EXECUTABLE)
