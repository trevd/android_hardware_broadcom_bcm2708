ifneq ($(strip $(TARGET_EGL_PREBUILT)),true)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include $(ANDROID_BUILD_TOP)/hardware/broadcom/$(TARGET_BOARD_PLATFORM)/cflags.mk

LOCAL_C_INCLUDES += hardware/broadcom/$(TARGET_BOARD_PLATFORM)/include

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/egl
LOCAL_MODULE := libEGL_bcm2708
	
include $(LOCAL_PATH)/android-sources.mk

		
LOCAL_SHARED_LIBRARIES := libvchiq_arm libvcos libm liblog libvchostif libgralloc.bcm2708 liblog
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

endif
