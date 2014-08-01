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

include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/hw
LOCAL_SHARED_LIBRARIES := liblog libcutils libEGL libvc4

LOCAL_C_INCLUDES := vendor/broadcom/rpi/opengl/interface/khronos/include/EGL \
LOCAL_C_INCLUDES := vendor/broadcom/rpi/opengl/host_applications/linux/libs/bcm_host/include \
					vendor/broadcom/rpi/opengl \
					vendor/broadcom/rpi/opengl/interface \
					vendor/broadcom/rpi/opengl/interface/vcos/pthreads \
					vendor/broadcom/rpi/opengl/interface/vmcs_host/linux \

LOCAL_SRC_FILES := 	\
	gralloc.cpp 	\
	framebuffer.cpp \
	mapper.cpp
	
LOCAL_MODULE := gralloc.bcm2708
LOCAL_CFLAGS:= -DLOG_TAG=\"gralloc\"

include $(BUILD_SHARED_LIBRARY)
