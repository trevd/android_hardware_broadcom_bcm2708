
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/hw
LOCAL_SHARED_LIBRARIES := liblog libEGL libhardware
LOCAL_SRC_FILES := hwc.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../gralloc
LOCAL_MODULE := hwcomposer.bcm2708
LOCAL_CFLAGS:= -DLOG_TAG=\"hwcomposer.bcm2708\"
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)
