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

#include <gralloc_priv.h>
#include <dispmanx.h>
#include <bcm_host.h>
#include <utils/Log.h>

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

void alloc_dispmanx_window_size(private_handle_t* handle)
{
    DISPMANX_MODEINFO_T info;
    EGL_DISPMANX_WINDOW_T window;
    vc_dispmanx_display_get_info(handle->brcm_handle->dispman_display, &info);
    window.height = info.height;
    window.width = info.width;
    handle->brcm_handle->window = window;
}

void alloc_dispmanx_default_values(private_handle_t* handle)
{
    handle->brcm_handle->gl_format = GRALLOC_MAGICS_HAL_PIXEL_FORMAT_OPAQUE;
    handle->brcm_handle->stride = ALIGN_UP(handle->brcm_handle->window.width, 32);
    handle->brcm_handle->res_type = GRALLOC_PRIV_TYPE_MM_RESOURCE;
    handle->brcm_handle->egl_image = (EGLImageKHR) 0xBADF00D;
}

void open_display(private_handle_t* handle)
{
    handle->brcm_handle->dispman_display = vc_dispmanx_display_open(0);
}

void write_buffer_dispmanx(private_handle_t* handle)
{
    //TODO: get image type from private_handle_t
    VC_IMAGE_TYPE_T type = VC_IMAGE_RGBA565;
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
    bcm_host_init();
    VC_RECT_T src_rect;
    VC_RECT_T dst_rect;

    handle->brcm_handle = (struct gralloc_private_handle_t*) malloc(sizeof(struct gralloc_private_handle_t));

    open_display(handle);
    alloc_dispmanx_window_size(handle);
    alloc_dispmanx_default_values(handle);

    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.width = handle->brcm_handle->window.width;
    dst_rect.height = handle->brcm_handle->window.height;

    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = handle->brcm_handle->window.width << 16;
    src_rect.height = handle->brcm_handle->window.height << 16;

    uint32_t dummy = 0;
    handle->brcm_handle->dispman_resource = vc_dispmanx_resource_create(VC_IMAGE_RGB565, dst_rect.width,
            dst_rect.height, &dummy);

    DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);

    handle->brcm_handle->window.element = vc_dispmanx_element_add(update, handle->brcm_handle->dispman_display,
            -127/*layer*/, &dst_rect, handle->brcm_handle->dispman_resource, &src_rect,
            DISPMANX_PROTECTION_NONE, 0, 0, (DISPMANX_TRANSFORM_T) 0);

    vc_dispmanx_update_submit(update, 0, 0);

    return 0;
}

int dispmanx_lock(private_handle_t* handle, int usage, int l, int t, int w, int h, void** vaddr)
{
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
    // flush the data cache
    write_buffer_dispmanx(handle);

    handle->lock = 0;

    return 0;
}

