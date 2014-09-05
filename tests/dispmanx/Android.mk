
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)
LOCAL_SHARED_LIBRARIES := liblog libcutils libvc4

LOCAL_C_INCLUDES := vendor/broadcom/rpi/opengl/interface/khronos/include/EGL \
LOCAL_C_INCLUDES := vendor/broadcom/rpi/opengl/host_applications/linux/libs/bcm_host/include \
					vendor/broadcom/rpi/opengl \
					vendor/broadcom/rpi/opengl/interface \
					vendor/broadcom/rpi/opengl/interface/vcos/pthreads \
					vendor/broadcom/rpi/opengl/interface/vmcs_host/linux \

LOCAL_SRC_FILES := 	\
    dispmanx.c
	
LOCAL_MODULE := test-dispmanx


include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)
LOCAL_SHARED_LIBRARIES := liblog libcutils libvc4

LOCAL_C_INCLUDES := vendor/broadcom/rpi/opengl/interface/khronos/include/EGL \
LOCAL_C_INCLUDES := vendor/broadcom/rpi/opengl/host_applications/linux/libs/bcm_host/include \
					vendor/broadcom/rpi/opengl \
					vendor/broadcom/rpi/opengl/interface \
					vendor/broadcom/rpi/opengl/interface/vcos/pthreads \
					vendor/broadcom/rpi/opengl/interface/vmcs_host/linux \

LOCAL_SRC_FILES := 	\
    dispmanx_resource.c
	
LOCAL_MODULE := test-dispmanx-resource


include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)
LOCAL_SHARED_LIBRARIES := liblog libcutils libvc4

LOCAL_C_INCLUDES := vendor/broadcom/rpi/opengl/interface/khronos/include/EGL \
LOCAL_C_INCLUDES := vendor/broadcom/rpi/opengl/host_applications/linux/libs/bcm_host/include \
					vendor/broadcom/rpi/opengl \
					vendor/broadcom/rpi/opengl/interface \
					vendor/broadcom/rpi/opengl/interface/vcos/pthreads \
					vendor/broadcom/rpi/opengl/interface/vmcs_host/linux \

LOCAL_SRC_FILES := 	\
    vc_memory.c
	
LOCAL_MODULE := test-vc-memory


include $(BUILD_EXECUTABLE)
