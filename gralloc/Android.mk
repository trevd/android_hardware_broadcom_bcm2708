# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


LOCAL_PATH := $(call my-dir)


ifeq ($(strip $(TARGET_DEBUG_GRALLOC)),true)
LOCAL_CFLAGS += -DDEBUG_DISPMANX=1
endif

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += hardware/broadcom/$(TARGET_BOARD_PLATFORM)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../userland/host_applications/linux/libs/bcm_host/include
LOCAL_C_INCLUDES += vendor/rpi/rpi/userland

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_SHARED_LIBRARIES := liblog libcutils libbcm_host libvchostif libgralloc_brcm libgralloc_dispmanx

LOCAL_SRC_FILES := 	\
	gralloc.cpp 	\
	framebuffer.cpp \
	mapper.cpp
	
LOCAL_MODULE := gralloc.bcm2708
LOCAL_CFLAGS += -DLOG_TAG=\"gralloc.bcm2708\"
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)
