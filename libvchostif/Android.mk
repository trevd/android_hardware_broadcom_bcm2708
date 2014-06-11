LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES += hardware/broadcom/$(TARGET_BOARD_PLATFORM)/include

LOCAL_CFLAGS:= 	-DUSE_VCHIQ_ARM -DVCHI_BULK_ALIGN=1 -DVCHI_BULK_GRANULARITY=1 \
				-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64

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
	vc_service_common.c

LOCAL_SHARED_LIBRARIES := libvchiq_arm libvcos
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

