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

#ifndef GRALLOC_DISPMANX_H_
#define GRALLOC_DISPMANX_H_

#include <gralloc_priv.h>
#ifdef __cplusplus
extern "C" {
#endif

EGL_DISPMANX_WINDOW_T* gralloc_private_handle_from_native_window(EGLNativeWindowType window);

gralloc_private_handle_t* gralloc_private_handle_from_client_buffer(EGLClientBuffer buffer);

uint32_t gralloc_private_handle_get_res_type(gralloc_private_handle_t* private_handle);

uint32_t gralloc_private_handle_get_vc_handle(gralloc_private_handle_t *private_handle);

uint32_t gralloc_private_handle_get_egl_image(gralloc_private_handle_t *private_handle);

void gralloc_set_egl_image(gralloc_private_handle_t *private_handle, EGLImageKHR image);

int dispmanx_alloc(private_handle_t* handle);

int dispmanx_lock(private_handle_t* handle, int usage, int l, int t, int w, int h, void** vaddr);

int dispmanx_unlock(private_handle_t* handle);

#ifdef __cplusplus
};
#endif


#endif /* GRALLOC_DISPMANX_H_ */
