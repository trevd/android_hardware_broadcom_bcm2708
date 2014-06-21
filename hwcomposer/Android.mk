LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include $(ANDROID_BUILD_TOP)/hardware/broadcom/$(TARGET_BOARD_PLATFORM)/cflags.mk

LOCAL_C_INCLUDES += hardware/broadcom/$(TARGET_BOARD_PLATFORM)/include

LOCAL_MODULE := hwcomposer.bcm2708
LOCAL_SRC_FILES := hwcomposer.cpp
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/hw
LOCAL_SHARED_LIBRARIES := libgralloc.bcm2708 libvchostif libvcos libvchiq_arm liblog libEGL libm libGLESv2 libhardware


LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
