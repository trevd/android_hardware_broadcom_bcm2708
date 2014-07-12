LOCAL_PATH:= $(call my-dir)

# libkhrn_test.a - We use BUILD_MULTI_PREBUILT here to create 
# obj/STATIC_LIBRARIES/libkhrn_static_test so we can 
# reference the library in other projects
# This is an the library from the RaspberryPI userland github
# repository. khrn_client_platfrom_openwfc.c.o has been removed
# as it's symbol clash with khrn_client_platfrom_linux.c.o
include $(CLEAR_VARS)

LOCAL_PREBUILT_LIBS := libkhrn_static_test.a
LOCAL_MODULE_TAGS := tests
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libkhrn_test
LOCAL_SRC_FILES:= __errno_location.c stderr.c
LOCAL_WHOLE_STATIC_LIBRARIES := libkhrn_static_test
LOCAL_MODULE_TAGS:= tests
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_LDFLAGS := -Wl,--no-warn-shared-textrel
LOCAL_MODULE := libGLES_bcm2708_test
LOCAL_SRC_FILES:= __errno_location.c stderr.c
LOCAL_WHOLE_STATIC_LIBRARIES := libkhrn_static_test
LOCAL_SHARED_LIBRARIES := libvchost liblog libvcos libvchiq_arm
LOCAL_MODULE_TAGS:= tests
LOCAL_MODULE_PATH:= $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)
include $(BUILD_SHARED_LIBRARY)

