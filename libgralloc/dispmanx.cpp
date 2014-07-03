/*
 * Copyright (C) 2013 Andreas Harter - RazDroid project
 *
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_TAG "GRALLOC-DISPMANX"
#include <gralloc/gralloc_brcm.h>
#include <gralloc/dispmanx.h>
#include <vmcs_host/vc_host.h>
#include <cutils/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/mman.h>

#ifndef ALIGN_UP
#define ALIGN_UP(x,y)  ((x + (y)-1) & ~((y)-1))
#endif
static VC_IMAGE_TYPE_T format_to_vc_format(int format){
ALOGD("%s",__FUNCTION__);
    VC_IMAGE_TYPE_T ret = VC_IMAGE_RGBA32;
	switch(format){
	case HAL_PIXEL_FORMAT_RGB_565:
		ALOGD("%s HAL_PIXEL_FORMAT_RGB_565",__FUNCTION__);
		ret=VC_IMAGE_RGB565;
		break;
	case HAL_PIXEL_FORMAT_RGBX_8888:
		ALOGD("%s HAL_PIXEL_FORMAT_RGBX_8888",__FUNCTION__);
		ret=VC_IMAGE_RGBX8888;
		break;
	case HAL_PIXEL_FORMAT_RGBA_8888:
		ALOGD("%s HAL_PIXEL_FORMAT_RGBA_8888",__FUNCTION__);
		ret=VC_IMAGE_RGBA32;
		break;
	default:
	ALOGD("%s unknown format=%d",__FUNCTION__,format);
		break;
	}
	return ret;
}


void open_display(private_handle_t* handle)
{
    ALOGD("%s",__FUNCTION__);
    handle->brcm_handle->dispman_display = vc_dispmanx_display_open(0);
}

void write_buffer_dispmanx(private_handle_t* handle)
{
    //TODO: get image type from private_handle_t
      ALOGI("%s format=%d",__FUNCTION__,handle->format);
    VC_IMAGE_TYPE_T type = format_to_vc_format( handle->format);
  
    VC_RECT_T dst_rect;

    void* image = (void*) handle->base;
    int width = handle->brcm_handle->window.width;
    int height = handle->brcm_handle->window.height;

    vc_dispmanx_rect_set(&dst_rect, 0, 0, width, height);

    vc_dispmanx_resource_write_data(handle->brcm_handle->dispman_resource, type, handle->brcm_handle->stride, image,
            &dst_rect);
}

int dispmanx_alloc(private_handle_t* handle)
{

  	
  	
	ALOGD("%s handle->brcm_handle=%p",__FUNCTION__,handle->brcm_handle);
	if(handle->brcm_handle){
		ALOGD("%s Already Allocated!",__FUNCTION__);
		return 0; 
	}
	
    VC_RECT_T src_rect;
    VC_RECT_T dst_rect;
	uint32_t display_width;
	uint32_t display_height;
    handle->brcm_handle = (struct gralloc_private_handle_t*) malloc(sizeof(struct gralloc_private_handle_t));
 
    handle->brcm_handle->gl_format = GRALLOC_MAGICS_HAL_PIXEL_FORMAT_OPAQUE;
    handle->brcm_handle->stride = ALIGN_UP(display_width, 32);
    handle->brcm_handle->res_type = GRALLOC_PRIV_TYPE_GL_RESOURCE;
    handle->brcm_handle->egl_image = (EGLImageKHR) 0xBADF00D;
   

	vc_dispmanx_display_get_size(0 /* LCD */, &display_width, &display_height);
	handle->brcm_handle->window.width = display_width;
	handle->brcm_handle->window.height = display_height;
    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.width = handle->brcm_handle->window.width;
    dst_rect.height = handle->brcm_handle->window.height;

    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = handle->brcm_handle->window.width << 16;
    src_rect.height = handle->brcm_handle->window.height << 16;
	VC_IMAGE_TYPE_T type = VC_IMAGE_RGB565 ; // format_to_vc_format( handle->format);
    uint32_t dummy = 0;
    
     handle->brcm_handle->dispman_display = vc_dispmanx_display_open(0);
      handle->brcm_handle->dispman_update = vc_dispmanx_update_start(0);
      handle->brcm_handle->window.element = vc_dispmanx_element_add(handle->brcm_handle->dispman_update, handle->brcm_handle->dispman_display,
              0/*layer*/, &dst_rect, 0/*src*/,
      &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/,  (DISPMANX_TRANSFORM_T)0/*transform*/);

    
    
    //handle->brcm_handle->dispman_resource = vc_dispmanx_resource_create(type, dst_rect.width, dst_rect.height, &dummy);
    
    vc_dispmanx_update_submit_sync(handle->brcm_handle->dispman_update);
	ALOGD("%s handle->brcm_handle=%p",__FUNCTION__,handle->brcm_handle);
	ALOGD("%s handle->brcm_handle->egl_image=%p",__FUNCTION__,handle->brcm_handle->egl_image);
	ALOGD("%s handle->brcm_handle->dispman_display=%p",__FUNCTION__,handle->brcm_handle->dispman_display);
	ALOGD("%s handle->brcm_handle->dispman_resource=%p",__FUNCTION__,handle->brcm_handle->dispman_resource);
	ALOGD("%s handle->brcm_handle->dispman_element=%p",__FUNCTION__,handle->brcm_handle->dispman_element);
	ALOGD("%s handle->brcm_handle->dispman_update=%p",__FUNCTION__,handle->brcm_handle->dispman_update);
	ALOGD("%s handle->brcm_handle->window=%p",__FUNCTION__,handle->brcm_handle->window);
	ALOGD("%s handle->brcm_handle->window.element=%p",__FUNCTION__,handle->brcm_handle->window.element);
	
	
    return 0;
}
/*****/

int dispmanx_lock(private_handle_t* handle, int usage, int l, int t, int w, int h, void** vaddr)
{
    ALOGD("%s",__FUNCTION__);
    // this is called when a buffer is being locked for software
    // access. in thin implementation we have nothing to do since
    // not synchronization with the h/w is needed.
    // typically this is used to wait for the h/w to finish with
    // this buffer if relevant. the data cache may need to be
    // flushed or invalidated depending on the usage bits and the
    // hardware.
    if (handle->lock & private_handle_t::WRITE_LOCK) {
        return -EBUSY;
    } else if (handle->flags & GRALLOC_USAGE_SW_WRITE_MASK) {
        handle->lock |= private_handle_t::WRITE_LOCK;
    }

    return 0;
}

int dispmanx_unlock(private_handle_t* handle)
{
    ALOGD("%s",__FUNCTION__);
    // flush the data cache
    write_buffer_dispmanx(handle);

    handle->lock = 0;

    return 0;
}

