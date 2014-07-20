LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../gralloc
LOCAL_SRC_FILES := triangle.c utils.c
LOCAL_MODULE := test-rpi-opengl-triangle-android
LOCAL_MODULE_TAGS := tests
LOCAL_SHARED_LIBRARIES := libEGL libGLESv2 liblog
LOCAL_STATIC_LIBRARIES := libbcm_host_static
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_LDFLAGS += -Wl,-rpath,/system/vendor/lib/egl/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../gralloc
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)
LOCAL_SRC_FILES := triangle.c utils.c
LOCAL_MODULE := test-rpi-opengl-triangle-egllib
LOCAL_STATIC_LIBRARIES := libbcm_host_static
LOCAL_MODULE_TAGS := tests
LOCAL_SHARED_LIBRARIES := libGLES_bcm2708 liblog
include $(BUILD_EXECUTABLE)
