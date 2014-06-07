#
# Copyright (C) 2013 Andreas Harter - RazDroid project
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
#

LOCAL_PATH:= $(call my-dir)

LOCAL_DEBUG_DISPMANX := 1
include $(CLEAR_VARS)

ifeq ($(strip $(LOCAL_DEBUG_DISPMANX)),1)
LOCAL_CFLAGS += -DDEBUG_DISPMANX=1
endif

LOCAL_CFLAGS += $(USERLAND_CFLAGS)
LOCAL_C_INCLUDES += $(USERLAND_INCLUDES) \
                    $(GRALLOC_RPI_INCLUDE)
LOCAL_C_INCLUDES += vendor/rpi/rpi/userland
LOCAL_C_INCLUDES += vendor/rpi/rpi/userland/host_applications/linux/libs/bcm_host/include
LOCAL_C_INCLUDES += vendor/rpi/rpi/userland/interface/vmcs_host
LOCAL_C_INCLUDES += vendor/rpi/rpi/userland/interface/vmcs_host/linux
LOCAL_C_INCLUDES += vendor/rpi/rpi/userland/interface/vcos/pthreads
LOCAL_C_INCLUDES += vendor/rpi/rpi/gralloc
LOCAL_C_INCLUDES += vendor/rpi/rpi/userland/interface/khronos/include
LOCAL_MODULE := libgralloc_dispmanx
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)
LOCAL_SRC_FILES := gralloc_dispmanx.cpp
LOCAL_SHARED_LIBRARIES := libbcm_host libvchostif liblog
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
