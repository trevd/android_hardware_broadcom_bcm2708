LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PREBUILT_LIBS := libbcm_host_static.a
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)


include $(CLEAR_VARS)

LOCAL_PREBUILT_LIBS := libkhrn_static.a
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
