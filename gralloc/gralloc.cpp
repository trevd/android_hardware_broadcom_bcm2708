/*
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

#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <cutils/ashmem.h>
#include <cutils/log.h>
#include <cutils/atomic.h>

#include <hardware/hardware.h>
#include <hardware/gralloc.h>

#include "gralloc_priv.h"
#include "gr.h"

/*****************************************************************************/

struct gralloc_context_t {
    alloc_device_t  device;
    /* our private data here */
};

static int gralloc_alloc_buffer(alloc_device_t* dev,
        size_t size, int usage, buffer_handle_t* pHandle);

/*****************************************************************************/

int fb_device_open(const hw_module_t* module, const char* name,
        hw_device_t** device);

static int gralloc_device_open(const hw_module_t* module, const char* name,
        hw_device_t** device);

extern int gralloc_lock(gralloc_module_t const* module,
        buffer_handle_t handle, int usage,
        int l, int t, int w, int h,
        void** vaddr);

extern int gralloc_unlock(gralloc_module_t const* module, 
        buffer_handle_t handle);

extern int gralloc_register_buffer(gralloc_module_t const* module,
        buffer_handle_t handle);

extern int gralloc_unregister_buffer(gralloc_module_t const* module,
        buffer_handle_t handle);

/*****************************************************************************/

static struct hw_module_methods_t gralloc_module_methods = {
        open: gralloc_device_open
};
#define VC_IMAGE_RGBA32 15
static void check_default(private_module_t* m )
{
  
      /*
       * Special identifiers indicating the default windows. Either use the
       * one we've got or create a new one
       * simple hack for VMCSX_VC4_1.0 release to demonstrate concurrent running of apps under linux

       * win ==  0 => full screen window on display 0
       * win == -1 => 1/4 screen top left window on display 0
       * win == -2 => 1/4 screen top right window on display 0
       * win == -3 => 1/4 screen bottom left window on display 0
       * win == -4 => 1/4 screen bottom right window on display 0
       * win == -5 => full screen window on display 2

       * it is expected that Open WFC will provide a proper mechanism in the near future
       */
       bcm_host_init();
	if ( m->window ) {
	    ALOGW("Dispmanx Window Already Initialized : %p",m->window); 
	    return;
	}
	
      
	
	 
	static EGL_DISPMANX_WINDOW_T nativewindow;

	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_UPDATE_HANDLE_T dispman_update;
	DISPMANX_RESOURCE_HANDLE_T dispman_resource;
	VC_RECT_T dst_rect;
	VC_RECT_T src_rect;
   

	uint32_t display_width;
	uint32_t display_height;

	// create an EGL window surface, passing context width/height
	graphics_get_display_size(0 /* LCD */, &display_width, &display_height);

   
	// You can hardcode the resolution here:
	//display_width = 1920;
	//display_height = 1080;

       dst_rect.x = 0;
       dst_rect.y = 0;
       dst_rect.width = display_width;
       dst_rect.height = display_height;
	  
       src_rect.x = 0;
       src_rect.y = 0;
       src_rect.width = display_width << 16;
       src_rect.height = display_height << 16;   

	dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
	dispman_update = vc_dispmanx_update_start( 0 );
         
	
	 uint32_t dummy = 0;
	dispman_resource = vc_dispmanx_resource_create((VC_IMAGE_TYPE_T)VC_IMAGE_RGBA32, dst_rect.width,dst_rect.height, &dummy);
	dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
						0/*layer*/, &dst_rect, dispman_resource,
						&src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, (DISPMANX_TRANSFORM_T)0/*transform*/);
	
	  
	  vc_dispmanx_update_submit_sync( dispman_update );
	  m->gl_format = GRALLOC_MAGICS_HAL_PIXEL_FORMAT_OPAQUE;
	m->stride = ALIGN_UP(display_width, 32);
	m->res_type = GRALLOC_PRIV_TYPE_GL_RESOURCE;
	m->egl_image = (EGLImageKHR) 0xBADF00D;
	m->dispman_display = dispman_display;
	m->dispman_resource = dispman_resource;
	m->window = &nativewindow;
	nativewindow.element = dispman_element;
	nativewindow.width = display_width;
	nativewindow.height = display_height;
	ALOGD("%s m->window=%p",__FUNCTION__,m->window);
	//return &nativewindow;
   
}
struct private_module_t HAL_MODULE_INFO_SYM = {
    base: {
        common: {
            tag: HARDWARE_MODULE_TAG,
            version_major: 1,
            version_minor: 0,
            id: GRALLOC_HARDWARE_MODULE_ID,
            name: "Graphics Memory Allocator Module",
            author: "The Android Open Source Project",
            methods: &gralloc_module_methods
        },
        registerBuffer: gralloc_register_buffer,
        unregisterBuffer: gralloc_unregister_buffer,
        lock: gralloc_lock,
        unlock: gralloc_unlock,
    },
    framebuffer: 0,
    flags: 0,
    numBuffers: 0,
    bufferMask: 0,
    lock: PTHREAD_MUTEX_INITIALIZER,
    currentBuffer: 0,
};

/*****************************************************************************/

static int gralloc_alloc_framebuffer_locked(alloc_device_t* dev,
        size_t size, int usage, buffer_handle_t* pHandle)
{
    private_module_t* m = reinterpret_cast<private_module_t*>(
            dev->common.module);
    //ALOGD("%s:%d", __FUNCTION__,__LINE__);  
    // allocate the framebuffer
    if (m->framebuffer == NULL) {
        // initialize the framebuffer, the framebuffer is mapped once
        // and forever.
        int err = mapFrameBufferLocked(m);
        if (err < 0) {
            return err;
        }
    }
    //ALOGD("%s:%d", __FUNCTION__,__LINE__);  
    const uint32_t bufferMask = m->bufferMask;
    const uint32_t numBuffers = m->numBuffers;
    const size_t bufferSize = m->finfo.line_length * m->info.yres;
    if (numBuffers == 1) {
        // If we have only one buffer, we never use page-flipping. Instead,
        // we return a regular buffer which will be memcpy'ed to the main
        // screen when post is called.
        int newUsage = (usage & ~GRALLOC_USAGE_HW_FB) | GRALLOC_USAGE_HW_2D;
        return gralloc_alloc_buffer(dev, bufferSize, newUsage, pHandle);
    }

    if (bufferMask >= ((1LU<<numBuffers)-1)) {
        // We ran out of buffers.
	ALOGE("%s:%d bufferMask=%d check=%d numBuffers=%d", __FUNCTION__,__LINE__,bufferMask,((1LU<<numBuffers)-1),numBuffers);  
        return 0 ; // -ENOMEM;
    }

    // create a "fake" handles for it
    intptr_t vaddr = intptr_t(m->framebuffer->base);
    private_handle_t* hnd = new private_handle_t(dup(m->framebuffer->fd), size,
            private_handle_t::PRIV_FLAGS_FRAMEBUFFER);

    // find a free slot
    //ALOGD("%s:%d", __FUNCTION__,__LINE__);  
    for (uint32_t i=0 ; i<numBuffers ; i++) {
        if ((bufferMask & (1LU<<i)) == 0) {
            m->bufferMask |= (1LU<<i);
            break;
        }
        vaddr += bufferSize;
    }
    
    hnd->base = vaddr;
    hnd->offset = vaddr - intptr_t(m->framebuffer->base);
    *pHandle = hnd;
    //ALOGD("%s:%d", __FUNCTION__,__LINE__);  
    return 0;
}

static int gralloc_alloc_framebuffer(alloc_device_t* dev,
        size_t size, int usage, buffer_handle_t* pHandle)
{
    private_module_t* m = reinterpret_cast<private_module_t*>(
            dev->common.module);
    pthread_mutex_lock(&m->lock);
    int err = gralloc_alloc_framebuffer_locked(dev, size, usage, pHandle);
    pthread_mutex_unlock(&m->lock);
    return err;
}

static int gralloc_alloc_buffer(alloc_device_t* dev,
        size_t size, int usage, buffer_handle_t* pHandle)
{
    int err = 0;
    int fd = -1;
    ALOGD("%s:%d", __FUNCTION__,__LINE__);

    size = roundUpToPageSize(size);
    
    fd = ashmem_create_region("gralloc-buffer", size);
    if (fd < 0) {
        ALOGE("couldn't create ashmem (%s)", strerror(-errno));
        err = -errno;
    }

    if (err == 0) {
        private_handle_t* hnd = new private_handle_t(fd, size, 0);
        gralloc_module_t* module = reinterpret_cast<gralloc_module_t*>(
                dev->common.module);
        err = mapBuffer(module, hnd);
        if (err == 0) {
            *pHandle = hnd;
        }
    }
    
    ALOGE_IF(err, "gralloc failed err=%s", strerror(-err));
    
    return err;
}

/*****************************************************************************/

static int gralloc_alloc(alloc_device_t* dev,
        int w, int h, int format, int usage,
        buffer_handle_t* pHandle, int* pStride)
{
    ALOGI("%s",__FUNCTION__);
        private_module_t* m = reinterpret_cast<private_module_t*>(dev->common.module);
    check_default(m);
    if (!pHandle || !pStride)
        return -EINVAL;
    
    size_t size, stride;

    int align = 4;
    int bpp = 0;
    int pad = 1;
	unsigned int tempw	 = (w+31)&0xFFFFFFE0;
	unsigned int temph	 = (h+31)&0xFFFFFFE0;
    switch (format) {
        case HAL_PIXEL_FORMAT_RGBA_8888:{
			bpp = 4;
			ALOGI("%s format=HAL_PIXEL_FORMAT_RGBA_8888[%d] bpp=%d",__FUNCTION__,format,bpp);
            break;
		}
        case HAL_PIXEL_FORMAT_RGBX_8888:{
			bpp = 4;
			ALOGI("%s format=HAL_PIXEL_FORMAT_RGBX_8888[%d] bpp=%d",__FUNCTION__,format,bpp);			
            break;
		}
        case HAL_PIXEL_FORMAT_BGRA_8888:{
			bpp = 4;
			ALOGI("%s format=HAL_PIXEL_FORMAT_BGRA_8888[%d] bpp=%d",__FUNCTION__,format,bpp);
            break;
		}
        case HAL_PIXEL_FORMAT_RGB_888:{
			bpp = 3;
			ALOGI("%s format=HAL_PIXEL_FORMAT_RGB_888[%d] bpp=%d",__FUNCTION__,format,bpp);
            break;
		}
        case HAL_PIXEL_FORMAT_RGB_565:{
			tempw	 = (w+63)&0xFFFFFFC0;
			bpp = 2;
			ALOGI("%s format=HAL_PIXEL_FORMAT_RGB_565[%d] bpp=%d",__FUNCTION__,format,bpp);
            break;
		}
        case HAL_PIXEL_FORMAT_RGBA_5551:{
			tempw	 = (w+63)&0xFFFFFFC0;
			bpp = 2;
			ALOGI("%s format=HAL_PIXEL_FORMAT_RGBA_5551[%d] bpp=%d",__FUNCTION__,format,bpp);
            break;
		}
        case HAL_PIXEL_FORMAT_RGBA_4444:{
			tempw	 = (w+63)&0xFFFFFFC0;
			bpp = 2;
			ALOGI("%s format=HAL_PIXEL_FORMAT_RGBA_4444[%d] bpp=%d",__FUNCTION__,format,bpp);
            break;
		}
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:{
			tempw	 = (w+63)&0xFFFFFFC0;
            bpp = 2;
		    pad = 0;
		    ALOGI("%s format=HAL_PIXEL_FORMAT_YCrCb_420_SP[%d] bpp=%d",__FUNCTION__,format,bpp);
            break;
        }
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:{
			tempw	 = (w+63)&0xFFFFFFC0;
            bpp = 2;
		    pad = 0;
		    ALOGI("%s format=HAL_PIXEL_FORMAT_YCbCr_420_SP[%d] bpp=%d",__FUNCTION__,format,bpp);
            break;
        }
        case HAL_PIXEL_FORMAT_YV12:{
			tempw	 = (w+63)&0xFFFFFFC0;
            bpp = 2;
		    pad = 0;
		    ALOGI("%s format=HAL_PIXEL_FORMAT_YV12[%d] bpp=%d",__FUNCTION__,format,bpp);
            break;
        }
        case HAL_PIXEL_FORMAT_YCbCr_420_P:{
			tempw	 = (w+63)&0xFFFFFFC0;
            bpp = 2;
		    pad = 0;
		    ALOGI("%s format=HAL_PIXEL_FORMAT_YCbCr_420_P[%d] bpp=%d",__FUNCTION__,format,bpp);
            break;
        }
		case HAL_PIXEL_FORMAT_YCbCr_422_I:{
			tempw	 = (w+63)&0xFFFFFFC0;
			bpp = 4;
			pad = 0;
			ALOGI("%s format=HAL_PIXEL_FORMAT_YCbCr_422_I[%d] bpp=%d",__FUNCTION__,format,bpp);
            break;
       }
	 default:{
			ALOGI("%s format=Unknown[%d] bpp=%d",__FUNCTION__,format,bpp);
            return -EINVAL;
       } 
           
    }
    size_t bpr = (w*bpp + (align-1)) & ~(align-1);
	if (!(usage & GRALLOC_USAGE_HW_FB)) {
		bpr = (tempw*bpp + (align-1)) & ~(align-1);
	}
    size = bpr * h;
	if (!(usage & GRALLOC_USAGE_HW_FB)) {
		size = bpr*temph;
	}
    stride = bpr / bpp;
	if(pad==0) {
		tempw = w;
		temph =  h;
		stride = w;
	}

    int err =0;
     if (usage & GRALLOC_USAGE_HW_FB) {
	ALOGD("%s:GRALLOC_USAGE_HW_FB",__FUNCTION__);
        err = gralloc_alloc_framebuffer(dev, size, usage, pHandle);
    } else {
	ALOGD("%s:usage=0x%x",__FUNCTION__,usage);
	
        err = gralloc_alloc_buffer(dev, size, usage, pHandle);
    }

    if (err < 0) {
        ALOGE("%s: err = %x",__FUNCTION__,err);
        return err;
    }

    *pStride = stride;
    return 0;
}

static int gralloc_free(alloc_device_t* dev,
        buffer_handle_t handle)
{
    if (private_handle_t::validate(handle) < 0)
        return -EINVAL;

    private_handle_t const* hnd = reinterpret_cast<private_handle_t const*>(handle);
    if (hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER) {
        // free this buffer
        private_module_t* m = reinterpret_cast<private_module_t*>(
                dev->common.module);
        const size_t bufferSize = m->finfo.line_length * m->info.yres;
        int index = (hnd->base - m->framebuffer->base) / bufferSize;
        m->bufferMask &= ~(1<<index); 
    } else { 
        gralloc_module_t* module = reinterpret_cast<gralloc_module_t*>(
                dev->common.module);
        terminateBuffer(module, const_cast<private_handle_t*>(hnd));
    }

    close(hnd->fd);
    delete hnd;
    return 0;
}

/*****************************************************************************/

static int gralloc_close(struct hw_device_t *dev)
{
    ALOGI("%s",__FUNCTION__);
    gralloc_context_t* ctx = reinterpret_cast<gralloc_context_t*>(dev);
    if (ctx) {
        /* TODO: keep a list of all buffer_handle_t created, and free them
         * all here.
         */
        free(ctx);
    }
    return 0;
}

int gralloc_device_open(const hw_module_t* module, const char* name,
        hw_device_t** device)
{
    ALOGI("%s",__FUNCTION__);
    int status = -EINVAL;
    private_module_t* m = (private_module_t*)module;
	check_default(m);
    ALOGD("%s  m=%p m->window=%p",__FUNCTION__, m, m->window);
	
    if (!strcmp(name, GRALLOC_HARDWARE_GPU0)) {
        gralloc_context_t *dev;
        dev = (gralloc_context_t*)malloc(sizeof(*dev));

        /* initialize our state here */
        memset(dev, 0, sizeof(*dev));

        /* initialize the procs */
        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version = 0;
        dev->device.common.module = const_cast<hw_module_t*>(module);
        dev->device.common.close = gralloc_close;

        dev->device.alloc   = gralloc_alloc;
        dev->device.free    = gralloc_free;
	
        *device = &dev->device.common;
	
	
        status = 0;
    } else {
        status = fb_device_open(module, name, device);
    }
    return status;
}

/* Called on first dlopen() */
/*static void __attribute__((constructor)) gralloc_constructor(void)
{
	
	int err;

	ALOGD("%s:%d",__FUNCTION__,__LINE__);
	PVR_ASSERT(!gbGraphicsHALInitialized && psPrivateData->hMutex == 0);

	err = OpenPVRServices(psPrivateData);
	if(err)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: Failed to open services (err=%d)",
								__func__, err));
		return;
	}

	PVRSRVCreateMutex(&psPrivateData->hMutex);

	PVR_DPF((PVR_DBG_MESSAGE, "%s: Graphics HAL loaded (err=%d)",
							  __func__, err));
	gbGraphicsHALInitialized = IMG_TRUE;
}
*/
