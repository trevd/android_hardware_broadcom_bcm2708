LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include $(ANDROID_BUILD_TOP)/hardware/broadcom/$(TARGET_BOARD_PLATFORM)/cflags.mk

LOCAL_C_INCLUDES += hardware/broadcom/$(TARGET_BOARD_PLATFORM)/include

LOCAL_MODULE := hwcomposer.bcm2708

ifeq ($(strip $(TARGET_HWC_VERSION)),1_3)
$(info Using HWC Version $(TARGET_HWC_VERSION))
	LOCAL_SRC_FILES := hwc_1_3.cpp
endif
ifeq ($(strip $(TARGET_HWC_VERSION)),1_1)
$(info Using HWC Version $(TARGET_HWC_VERSION))
	LOCAL_SRC_FILES := hwc_1_1.cpp
endif
ifeq ($(strip $(TARGET_HWC_VERSION)),1_0)
$(info Using HWC Version $(TARGET_HWC_VERSION))
	LOCAL_SRC_FILES := hwc_1_0.cpp
endif

ifeq ($(strip $(TARGET_HWC_VERSION)),)
$(warning Unknown HWC Version $(TARGET_HWC_VERSION). Using v1.3)
LOCAL_SRC_FILES := hwc_1_3.cpp
endif

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/hw
LOCAL_SHARED_LIBRARIES := libvchost libvcos libvchiq_arm liblog libEGL libm libGLESv2 libhardware


LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
