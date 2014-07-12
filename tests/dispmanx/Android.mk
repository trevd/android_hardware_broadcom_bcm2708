LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include $(ANDROID_BUILD_TOP)/hardware/broadcom/$(TARGET_BOARD_PLATFORM)/cflags.mk

LOCAL_C_INCLUDES += hardware/broadcom/$(TARGET_BOARD_PLATFORM)/include
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)

LOCAL_SRC_FILES := dispmanx.c
LOCAL_MODULE := test-rpi-dispmanx
LOCAL_MODULE_TAGS := tests
LOCAL_SHARED_LIBRARIES := libEGL libGLESv2 libvchost liblog

include $(BUILD_EXECUTABLE)
