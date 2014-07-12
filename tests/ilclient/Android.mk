LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include $(ANDROID_BUILD_TOP)/hardware/broadcom/$(TARGET_BOARD_PLATFORM)/cflags.mk

LOCAL_C_INCLUDES += hardware/broadcom/$(TARGET_BOARD_PLATFORM)/include

LOCAL_SRC_FILES := ilclient.c ilcore.c vcilcs_common.c vcilcs_in.c vcilcs_out.c vcilcs.c
LOCAL_MODULE := libilclient
LOCAL_MODULE_TAGS := tests

include $(BUILD_STATIC_LIBRARY)
