LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES += hardware/broadcom/$(TARGET_BOARD_PLATFORM)/include
LOCAL_CFLAGS:= -DUSE_VCHIQ_ARM -DVCHI_BULK_ALIGN=1 -DVCHI_BULK_GRANULARITY=1
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)
LOCAL_MODULE := libvchiq_arm
LOCAL_SRC_FILES := \
	vchiq_lib.c \
	vchiq_util.c \
	vcfiled_check.c
	
LOCAL_SHARED_LIBRARIES := libvcos
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

#add_executable(vchiq_test
#              vchiq_test.c)

#target_link_libraries(vchiq_test
#                     vchiq_arm
#                      vcos)

#install(TARGETS vchiq_test RUNTIME DESTINATION bin)
