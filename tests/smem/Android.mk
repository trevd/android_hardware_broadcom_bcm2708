LOCAL_PATH:= $(call my-dir)

opengl_cflags := \
		-DANDROID -DEGL_EGLEXT_ANDROID_STRUCT_HEADER \
		-DEGL_SERVER_DISPMANX \
		-DHAVE_VMCS_CONFIG -DOMX_SKIP64BIT -DOpenVG_EXPORTS \
		-DTV_SUPPORTED_MODE_NO_DEPRECATED -DUSE_VCHIQ_ARM \
		-DVCHI_BULK_ALIGN=1 -DVCHI_BULK_GRANULARITY=1 \
		-D_FILE_OFFSET_BITS=64 -D_HAVE_SBRK -D_LARGEFILE64_SOURCE \
		-D_LARGEFILE_SOURCE -D_REENTRANT -D__VIDEOCORE4__

include $(CLEAR_VARS)
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)
LOCAL_C_INCLUDES := vendor/broadcom/rpi/opengl/interface/khronos/include/EGL \
					vendor/broadcom/rpi/opengl/host_applications/linux/libs/bcm_host/include \
					vendor/broadcom/rpi/opengl \
					vendor/broadcom/rpi/opengl/interface \
					vendor/broadcom/rpi/opengl/interface/vmcs_host \
					vendor/broadcom/rpi/opengl/interface/vcos/pthreads \
					vendor/broadcom/rpi/opengl/interface/vmcs_host/linux \
LOCAL_CFLAGS += $(opengl_cflags)
LOCAL_SRC_FILES := smem.c vcsm.c
LOCAL_MODULE := test-rpi-vcsm
LOCAL_MODULE_TAGS := tests
LOCAL_SHARED_LIBRARIES := liblog libcutils libvc4

include $(BUILD_EXECUTABLE)

