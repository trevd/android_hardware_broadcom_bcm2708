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
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "gralloc-mapper" 

#ifdef LOG_NDEBUG
#undef LOG_NDEBUG
#endif
#define LOG_NDEBUG 0
#include <limits.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

#include <hardware/hardware.h>
#include <hardware/gralloc.h>

#include "gralloc_priv.h"
#include "dispmanx.h"
#include <user-vcsm.h>


/* desktop Linux needs a little help with gettid() */
#if defined(ARCH_X86) && !defined(HAVE_ANDROID_OS)
#define __KERNEL__
# include <linux/unistd.h>
pid_t gettid() { return syscall(__NR_gettid);}
#undef __KERNEL__
#endif

/*****************************************************************************/

/*****************************************************************************/

static pthread_mutex_t sMapLock = PTHREAD_MUTEX_INITIALIZER;

/*****************************************************************************/

int gralloc_register_buffer(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    ALOGV("\nint %s:%d ", __FUNCTION__,__LINE__);
    ALOGV("\t gralloc_module_t const* module=%p",module);
    ALOGV("\t buffer_handle_t handle=%p",handle);
    if (private_handle_t::validate(handle) < 0)
        return -EINVAL;

    // *** WARNING WARNING WARNING ***
    //
    // If a buffer handle is passed from the process that allocated it to a
    // different process, and then back to the allocator process, we will
    // create a second mapping of the buffer. If the process reads and writes
    // through both mappings, normal memory ordering guarantees may be
    // violated, depending on the processor cache implementation*.
    //
    // If you are deriving a new gralloc implementation from this code, don't
    // do this. A "real" gralloc should provide a single reference-counted
    // mapping for each buffer in a process.
    //
    // In the current system, there is one case that needs a buffer to be
    // registered in the same process that allocated it. The SurfaceFlinger
    // process acts as the IGraphicBufferAlloc Binder provider, so all gralloc
    // allocations happen in its process. After returning the buffer handle to
    // the IGraphicBufferAlloc client, SurfaceFlinger free's its handle to the
    // buffer (unmapping it from the SurfaceFlinger process). If
    // SurfaceFlinger later acts as the producer end of the buffer queue the
    // buffer belongs to, it will get a new handle to the buffer in response
    // to IGraphicBufferProducer::requestBuffer(). Like any buffer handle
    // received through Binder, the SurfaceFlinger process will register it.
    // Since it already freed its original handle, it will only end up with
    // one mapping to the buffer and there will be no problem.
    //
    // Currently SurfaceFlinger only acts as a buffer producer for a remote
    // consumer when taking screenshots and when using virtual displays.
    //
    // Eventually, each application should be allowed to make its own gralloc
    // allocations, solving the problem. Also, this ashmem-based gralloc
    // should go away, replaced with a real ion-based gralloc.
    //
    // * Specifically, associative virtually-indexed caches are likely to have
    //   problems. Most modern L1 caches fit that description.

    private_handle_t* hnd = (private_handle_t*)handle;
    ALOGV("Registering a buffer in the process that created it. "
            "This may cause memory ordering problems. pid=%d",hnd->pid);

    void *vaddr;
    return 0; //gralloc_map(module, handle, &vaddr);
}

int gralloc_unregister_buffer(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    ALOGV("\nint %s:%d ", __FUNCTION__,__LINE__);
    ALOGV("\t gralloc_module_t const* module=%p",module);
    ALOGV("\t buffer_handle_t handle=%p",handle);
    if (private_handle_t::validate(handle) < 0)
        return -EINVAL;

    private_handle_t* hnd = (private_handle_t*)handle;
  //  if (hnd->base)
    //    gralloc_unmap(module, handle);

    return 0;
}



int terminateBuffer(gralloc_module_t const* module,
        private_handle_t* hnd)
{
    ALOGV("\nint %s:%d ", __FUNCTION__,__LINE__);
    ALOGV("\t gralloc_module_t const* module=%p",module);
    ALOGV("\t private_handle_t* handle=%p",hnd);
    ALOGV("%s:%d hnd->base=0x%x", __FUNCTION__,__LINE__,hnd->base);
    //if (hnd->base) {
     //   // this buffer was mapped, unmap it now
     //   gralloc_unmap(module, hnd);
   // }

    return 0;
}

int gralloc_lock(gralloc_module_t const* module,
        buffer_handle_t handle, int usage,
        int l, int t, int w, int h,
        void** vaddr)
{

    ALOGV("\nint %s:%d ", __FUNCTION__,__LINE__);
    ALOGV("\t gralloc_module_t const* module=%p",module);
    ALOGV("\t buffer_handle_t handle=%p",handle);
    ALOGV("\t int usage=%d",usage);
    ALOGV("\t int l=%d",l);
    ALOGV("\t int t=%d",t);
    ALOGV("\t int w=%d",w);
    ALOGV("\t int h=%d",h);
    if (private_handle_t::validate(handle) < 0)
        return -EINVAL;

    private_handle_t* hnd = (private_handle_t*)handle;
    *vaddr = (void*) vcsm_lock(hnd->fd);
    return 0 ;
  
}


int gralloc_lock_ycbcr(gralloc_module_t const* module,
                 buffer_handle_t handle, int usage,
                 int l, int t, int w, int h,
                 struct android_ycbcr *ycbcr)
{
   
    
    ALOGV("\nint %s:%d ", __FUNCTION__,__LINE__);
    ALOGV("\t gralloc_module_t const* module=%p",module);
    ALOGV("\t buffer_handle_t handle=%p",handle);
    ALOGV("\t int usage=%d",usage);
    ALOGV("\t int l=%d",l);
    ALOGV("\t int t=%d",t);
    ALOGV("\t int w=%d",w);
    ALOGV("\t int h=%d",h);
    ALOGV("\t struct android_ycbcr *ycbcr=%p",ycbcr);
    
    private_handle_t* hnd = (private_handle_t*)handle;
    ALOGV_IF(hnd->pid == getpid(),
            "Registering a buffer in the process that created it. "
            "This may cause memory ordering problems.");

    void *vaddr;

    int err = 0 ; //gralloc_map(module, handle, &vaddr);
    int ystride;
    if(!err) {
        //hnd->format holds our implementation defined format
        //HAL_PIXEL_FORMAT_YCrCb_420_SP is the only one set right now.
        switch (hnd->format) {
            case HAL_PIXEL_FORMAT_YCrCb_420_SP:
                ystride = ALIGN_UP(hnd->width, 16);
                ycbcr->y  = (void*)hnd->base;
                ycbcr->cr = (void*)(hnd->base + ystride * hnd->height);
                ycbcr->cb = (void*)(hnd->base + ystride * hnd->height + 1);
                ycbcr->ystride = ystride;
                ycbcr->cstride = ystride;
                ycbcr->chroma_step = 2;
                memset(ycbcr->reserved, 0, sizeof(ycbcr->reserved));
                break;
            default:
                ALOGV("%s: Invalid format passed: 0x%x", __FUNCTION__,
                      hnd->format);
                err = -EINVAL;
        }
    }
    return err;
}
int gralloc_unlock(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    ALOGV("\nint %s:%d ", __FUNCTION__,__LINE__);
    ALOGV("\t gralloc_module_t const* module=%p",module);
    ALOGV("\t buffer_handle_t handle=%p",handle);
    // we're done with a software buffer. nothing to do in this
    // implementation. typically this is used to flush the data cache.
	ALOGV("%s:%d ", __FUNCTION__,__LINE__);
    if (private_handle_t::validate(handle) < 0)
        return -EINVAL;
     private_handle_t* hnd = (private_handle_t*)handle;
   vcsm_unlock_hdl( hnd->fd );
    return 0;
}
int gralloc_perform(struct gralloc_module_t const* module,
                    int operation, ... )
{
    //ALOGV("int %s:%d ", __FUNCTION__,__LINE__);
    //ALOGV("\t gralloc_module_t const* module=%p",module);
    //ALOGV("\t int operation=0x%x",operation);
    int res = -EINVAL;
    va_list args;
    va_start(args, operation);
    switch (operation) {
        case GRALLOC_MODULE_PERFORM_GET_DISPMANX_HANDLE:
            {
		EGL_DISPMANX_WINDOW_T** win = va_arg(args, EGL_DISPMANX_WINDOW_T **);
		
		*win = get_dispmanx_window_element();
		return 0;
		
		break;
	    }

	}
    return 0;
}
