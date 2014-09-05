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

# HAL module implemenation stored in
# hw/<OVERLAY_HARDWARE_MODULE_ID>.<ro.product.board>.so
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/hw
LOCAL_SHARED_LIBRARIES := liblog libcutils libvc4 libEGL libGLESv1_CM libvcsm

opengl_cflags := \
		-DANDROID -DEGL_EGLEXT_ANDROID_STRUCT_HEADER \
		-DEGL_SERVER_DISPMANX \
		-DHAVE_VMCS_CONFIG -DOMX_SKIP64BIT -DOpenVG_EXPORTS \
		-DTV_SUPPORTED_MODE_NO_DEPRECATED -DUSE_VCHIQ_ARM \
		-DVCHI_BULK_ALIGN=1 -DVCHI_BULK_GRANULARITY=1 \
		-D_FILE_OFFSET_BITS=64 -D_HAVE_SBRK -D_LARGEFILE64_SOURCE \
		-D_LARGEFILE_SOURCE -D_REENTRANT -D__VIDEOCORE4__

LOCAL_SRC_FILES := 	\
	gralloc.cpp 	\
	framebuffer.cpp \
	mapper.cpp \
	dispmanx.cpp

LOCAL_C_INCLUDES := vendor/broadcom/rpi/opengl/interface/khronos/include/EGL \
					vendor/broadcom/rpi/opengl/host_applications/linux/libs/bcm_host/include \
					vendor/broadcom/rpi/opengl/host_applications/linux/libs/sm \
					vendor/broadcom/rpi/opengl \
					vendor/broadcom/rpi/opengl/interface \
					vendor/broadcom/rpi/opengl/interface/vmcs_host \
					vendor/broadcom/rpi/opengl/interface/vcos/pthreads \
					vendor/broadcom/rpi/opengl/interface/vmcs_host/linux \
	
LOCAL_MODULE := gralloc.bcm2708
LOCAL_CFLAGS:= 	$(opengl_cflags)


include $(BUILD_SHARED_LIBRARY)
