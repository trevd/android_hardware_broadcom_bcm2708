
#ifndef DISPMANX_PRIV_H_
#define DISPMANX_PRIV_H_

#include <interface/khronos/include/EGL/eglplatform.h>
#include <interface/khronos/include/EGL/egl.h>
#include <interface/khronos/include/EGL/eglext.h>
#include <interface/vctypes/vc_image_types.h>
#include <vc_dispmanx.h>
#include <bcm_host.h>
#include "gralloc_priv.h"

typedef struct dispmanx_private_handle dispmanx_private_handle;
struct dispmanx_private_handle {
    int mailbox_fd ;
    int inited ; 
    EGLImageKHR egl_image;
    DISPMANX_DISPLAY_HANDLE_T display;
    DISPMANX_RESOURCE_HANDLE_T resource;
    EGL_DISPMANX_WINDOW_T* window;
    int res_type;
    int gl_format;
};
unsigned int mailbox_memory_alloc(unsigned int size);
unsigned int mailbox_memory_lock(unsigned int handle);
unsigned int mailbox_memory_unlock(unsigned int handle);
unsigned int mailbox_memory_free(unsigned int handle);
EGL_DISPMANX_WINDOW_T* get_dispmanx_window_element();
EGLImageKHR* get_egl_image(framebuffer_device_t *dev);
#endif
