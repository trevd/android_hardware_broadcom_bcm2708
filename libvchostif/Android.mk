LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES += hardware/broadcom/$(TARGET_BOARD_PLATFORM)/include

include $(ANDROID_BUILD_TOP)/hardware/broadcom/$(TARGET_BOARD_PLATFORM)/cflags.mk

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)

LOCAL_MODULE := libvchostif
LOCAL_SRC_FILES := \
	linux/vcfilesys.c \
	linux/vcmisc.c \
	vc_vchi_gencmd.c \
	vc_vchi_filesys.c \
	vc_vchi_tvservice.c \
	vc_vchi_cecservice.c \
	vc_vchi_dispmanx.c \
	vc_service_common.c \
	vc_host.c

ifeq ($(strip $(TARGET_LOG_VIDEOCORE)),true)
$(info VIDEOCORE LOGGING ENABLED)
LOCAL_CFLAGS += -LOG_NDEBUG=1
endif

LOCAL_SHARED_LIBRARIES := libvchiq_arm libvcos liblog
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

