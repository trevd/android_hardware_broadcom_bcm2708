LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include $(ANDROID_BUILD_TOP)/hardware/broadcom/$(TARGET_BOARD_PLATFORM)/cflags.mk

LOCAL_C_INCLUDES += hardware/broadcom/$(TARGET_BOARD_PLATFORM)/include
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)

LOCAL_SRC_FILES := triangle.c utils.c
LOCAL_MODULE := test-rpi-opengl-triangle-android
LOCAL_MODULE_TAGS := tests
LOCAL_SHARED_LIBRARIES := libEGL libGLESv2 libvchost liblog
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += hardware/broadcom/$(TARGET_BOARD_PLATFORM)/include
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)
LOCAL_SRC_FILES := triangle.c utils.c
LOCAL_MODULE := test-rpi-opengl-triangle-egllib
LOCAL_MODULE_TAGS := tests
LOCAL_SHARED_LIBRARIES := libEGL_bcm2708 libvchost liblog
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += hardware/broadcom/$(TARGET_BOARD_PLATFORM)/include
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)
LOCAL_SRC_FILES := triangle.c utils.c
LOCAL_MODULE := test-rpi-opengl-triangle-testlib
LOCAL_MODULE_TAGS := tests
LOCAL_SHARED_LIBRARIES := libGLES_bcm2708_test libvchost liblog


include $(BUILD_EXECUTABLE)
