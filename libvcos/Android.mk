LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES += hardware/broadcom/$(TARGET_BOARD_PLATFORM)/include

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)

LOCAL_MODULE := libvcos
LOCAL_SRC_FILES := \
	pthreads/vcos_pthreads.c \
	pthreads/vcos_dlfcn.c \
	generic/vcos_generic_event_flags.c \
	generic/vcos_mem_from_malloc.c \
	generic/vcos_generic_named_sem.c \
	generic/vcos_generic_safe_string.c \
	generic/vcos_generic_reentrant_mtx.c \
	generic/vcos_abort.c \
	generic/vcos_cmd.c \
	generic/vcos_init.c \
	generic/vcos_msgqueue.c \
	generic/vcos_logcat.c \
	generic/vcos_generic_blockpool.c
LOCAL_SHARED_LIBRARIES := libdl liblog
LOCAL_MODULE_TAGS := optional


include $(BUILD_SHARED_LIBRARY)
