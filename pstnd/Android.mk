LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := \
					$(LOCAL_PATH)/interface \
					$(LOCAL_PATH)/util
LOCAL_C_INCLUDES += bionic
LOCAL_C_INCLUDES += external/stlport/stlport
LOCAL_C_INCLUDES += external/webrtc/src \
					external/webrtc/src/modules/interface \
					external/webrtc/src/modules/audio_processing/interface
#LOCAL_C_INCLUDES += $(TOP)/prebuilts/ndk/8/sources/cxx-stl/stlport/stlport
LOCAL_SRC_FILES := main.cpp
				   

LOCAL_SHARED_LIBRARIES := libui \
						  libcutils \
						  libutils \
						  libbinder \
						  libsonivox \
						  libicuuc \
						  libexpat \
						  libdl \
						  libwebrtc_audio_preprocessing

				 
LOCAL_LDLIBS := \
				-llog \
				-ldl \
				-lz \
				-lm

EXTRA_LDLIBS := \
				-lpthread \
				-lrt

LOCAL_LDFLAGS += $(TOP)/prebuilts/ndk/8/sources/cxx-stl/stlport/libs/armeabi/libstlport_static.a

LOCAL_MODULE := pstnd
include $(BUILD_EXECUTABLE)

