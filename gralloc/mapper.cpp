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
#define LOG_TAG "GRALLOC-MAPPER"
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

#include <gralloc/gralloc_priv.h>
#include <gralloc/dispmanx.h>


/* desktop Linux needs a little help with gettid() */
#if defined(ARCH_X86) && !defined(HAVE_ANDROID_OS)
#define __KERNEL__
# include <linux/unistd.h>
pid_t gettid() { return syscall(__NR_gettid);}
#undef __KERNEL__
#endif

inline size_t ALIGN(size_t x, size_t align) {
    return (x + align-1) & ~(align-1);
}
/*****************************************************************************/

static int gralloc_map(gralloc_module_t const* /*module*/,
        buffer_handle_t handle,
        void** vaddr)
{
	ALOGI("%s",__FUNCTION__);
    private_handle_t* hnd = (private_handle_t*)handle;
    if (!(hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER)) {
        size_t size = hnd->size;
        void* mappedAddress = mmap(0, size,
                PROT_READ|PROT_WRITE, MAP_SHARED, hnd->fd, 0);
        if (mappedAddress == MAP_FAILED) {
            ALOGE("Could not mmap %s", strerror(errno));
            return -errno;
        }
        hnd->base = uintptr_t(mappedAddress) + hnd->offset;
        //ALOGD("gralloc_map() succeeded fd=%d, off=%d, size=%d, vaddr=%p",
        //        hnd->fd, hnd->offset, hnd->size, mappedAddress);
    }
    *vaddr = (void*)hnd->base;
     //dispmanx_alloc(hnd);
    return 0;
}

static int gralloc_unmap(gralloc_module_t const* /*module*/,
        buffer_handle_t handle)
{
    ALOGI("%s",__FUNCTION__);
    private_handle_t* hnd = (private_handle_t*)handle;
    if (!(hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER)) {
        void* base = (void*)hnd->base;
        size_t size = hnd->size;
        //ALOGD("unmapping from %p, size=%d", base, size);
        if (munmap(base, size) < 0) {
            ALOGE("Could not unmap %s", strerror(errno));
        }
    }
    hnd->base = 0;
    return 0;
}

/*****************************************************************************/

int gralloc_register_buffer(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    ALOGI("%s",__FUNCTION__);
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
    ALOGD_IF(hnd->pid == getpid(),
            "Registering a buffer in the process that created it. "
            "This may cause memory ordering problems.");

    void *vaddr;
    return gralloc_map(module, handle, &vaddr);
}

int gralloc_unregister_buffer(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    ALOGI("%s",__FUNCTION__);
    if (private_handle_t::validate(handle) < 0)
        return -EINVAL;

    private_handle_t* hnd = (private_handle_t*)handle;
    if (hnd->base)
        gralloc_unmap(module, handle);

    return 0;
}

int mapBuffer(gralloc_module_t const* module,
        private_handle_t* hnd)
{
    ALOGI("%s",__FUNCTION__);
    void* vaddr;
    return gralloc_map(module, hnd, &vaddr);
}

int terminateBuffer(gralloc_module_t const* module,
        private_handle_t* hnd)
{
    ALOGI("%s",__FUNCTION__);
    if (hnd->base) {
        // this buffer was mapped, unmap it now
        gralloc_unmap(module, hnd);
    }

    return 0;
}

int gralloc_lock(gralloc_module_t const* /*module*/,
        buffer_handle_t handle, int usage,
        int l, int t, int w, int h,
        void** vaddr)
{
    ALOGI("%s",__FUNCTION__);
    // this is called when a buffer is being locked for software
    // access. in thin implementation we have nothing to do since
    // not synchronization with the h/w is needed.
    // typically this is used to wait for the h/w to finish with
    // this buffer if relevant. the data cache may need to be
    // flushed or invalidated depending on the usage bits and the
    // hardware.

    if (private_handle_t::validate(handle) < 0)
        return -EINVAL;

    private_handle_t* hnd = (private_handle_t*)handle;
    *vaddr = (void*)hnd->base;
    return dispmanx_lock(hnd, usage, l, t, w, h, vaddr);
}

int gralloc_unlock(gralloc_module_t const* /*module*/,
        buffer_handle_t handle)
{
    ALOGI("%s",__FUNCTION__);
    // we're done with a software buffer. nothing to do in this
    // implementation. typically this is used to flush the data cache.

    if (private_handle_t::validate(handle) < 0)
        return -EINVAL;
        
          private_handle_t* hnd = (private_handle_t*)handle;
    dispmanx_unlock(hnd);
      dispmanx_unlock(hnd);
    return 0;
}
int gralloc_perform(struct gralloc_module_t const* module,
                    int operation, ... )
{
	 ALOGI("%s",__FUNCTION__);
	    int res = -EINVAL;
    va_list args;
    va_start(args, operation);
    switch (operation) {
        case GRALLOC_MODULE_PERFORM_CREATE_HANDLE_FROM_BUFFER:
            {
                int fd = va_arg(args, int);
                size_t size = va_arg(args, size_t);
                size_t offset = va_arg(args, size_t);
                void* base = va_arg(args, void*);
                int width = va_arg(args, int);
                int height = va_arg(args, int);
                int format = va_arg(args, int);

                native_handle_t** handle = va_arg(args, native_handle_t**);
                int memoryFlags = va_arg(args, int);
                private_handle_t* hnd = (private_handle_t*)native_handle_create(
                    private_handle_t::sNumFds, private_handle_t::sNumInts);
                hnd->magic = private_handle_t::sMagic;
                hnd->fd = fd;
                hnd->flags = private_handle_t::PRIV_FLAGS_USES_ASHMEM;
                hnd->size = size;
                hnd->offset = offset;
                hnd->base = intptr_t(base) + offset;
                hnd->gpuaddr = 0;
                hnd->width = width;
                hnd->height = height;
                hnd->format = format;
                *handle = (native_handle_t *)hnd;
                res = 0;
                break;

            }
		default:
			break;
    }

    va_end(args);
    return res;
}
int gralloc_lock_ycbcr(gralloc_module_t const* module,
                 buffer_handle_t handle, int usage,
                 int l, int t, int w, int h,
                 struct android_ycbcr *ycbcr)
{
     
   ALOGI("%s",__FUNCTION__);
    if (private_handle_t::validate(handle) < 0)
        return -EINVAL;

    private_handle_t* hnd = (private_handle_t*)handle;
    void *vaddr;
	int err = gralloc_map(module, handle, &vaddr);
 
   
    int ystride;
    if(!err) {
        //hnd->format holds our implementation defined format
        //HAL_PIXEL_FORMAT_YCrCb_420_SP is the only one set right now.
        switch (hnd->format) {
            case HAL_PIXEL_FORMAT_YCrCb_420_SP:
                ystride = ALIGN(hnd->width, 16);
                ycbcr->y  = (void*)hnd->base;
                ycbcr->cr = (void*)(hnd->base + ystride * hnd->height);
                ycbcr->cb = (void*)(hnd->base + ystride * hnd->height + 1);
                ycbcr->ystride = ystride;
                ycbcr->cstride = ystride;
                ycbcr->chroma_step = 2;
                memset(ycbcr->reserved, 0, sizeof(ycbcr->reserved));
                break;
            default:
                ALOGD("%s: Invalid format passed: 0x%x", __FUNCTION__,
                      hnd->format);
                err = -EINVAL;
        }
    }
    return err;
}
