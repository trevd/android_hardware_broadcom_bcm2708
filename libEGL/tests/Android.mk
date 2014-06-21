LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include $(ANDROID_BUILD_TOP)/hardware/broadcom/$(TARGET_BOARD_PLATFORM)/cflags.mk

LOCAL_C_INCLUDES += hardware/broadcom/$(TARGET_BOARD_PLATFORM)/include
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)/egl

LOCAL_SRC_FILES := hello_triangle.c utils.c
LOCAL_MODULE := hello_triangle
LOCAL_MODULE_TAGS := tests
LOCAL_SHARED_LIBRARIES := libEGL_bcm2708 libgralloc.bcm2708 libvchostif liblog

include $(BUILD_EXECUTABLE)
