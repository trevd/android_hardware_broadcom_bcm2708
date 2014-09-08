#ifndef GRALLOC_BRCM_DEF_H_
#define GRALLOC_BRCM_DEF_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#include <utils/Log.h>
static const int GRALLOC_PRIV_TYPE_MM_RESOURCE = 0;
static const int GRALLOC_PRIV_TYPE_GL_RESOURCE = 1;

static const int GRALLOC_MAGICS_HAL_PIXEL_FORMAT_OPAQUE = 0;

typedef struct gralloc_private_handle_t {

    EGLImageKHR egl_image;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_RESOURCE_HANDLE_T dispman_resource;
    EGL_DISPMANX_WINDOW_T window;
    //GRALLOC_PRIV_TYPE_GL_RESOURCE or GRALLOC_PRIV_TYPE_MM_RESOURCE
    int res_type;
    int gl_format;
    int stride;
    int pixelformat;

} gralloc_private_handle_t;

#endif /* GRALLOC_BRCM_DEF_H_ */
