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

#ifndef GRALLOC_PRIV_H_
#define GRALLOC_PRIV_H_

#include <stdint.h>
#include <limits.h>
#include <sys/cdefs.h>
#include <hardware/gralloc.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#include <cutils/native_handle.h>

#include <linux/fb.h>


#include <cutils/log.h>

/*****************************************************************************/
enum {
    /* gralloc usage bits indicating the type
     * of allocation that should be used */

    /* SYSTEM heap comes from kernel vmalloc,
     * can never be uncached, is not secured*/
    GRALLOC_USAGE_PRIVATE_SYSTEM_HEAP     =       GRALLOC_USAGE_PRIVATE_0,
    /* SF heap is used for application buffers, is not secured */
    GRALLOC_USAGE_PRIVATE_UI_CONTIG_HEAP  =       GRALLOC_USAGE_PRIVATE_1,
    /* IOMMU heap comes from manually allocated pages,
     * can be cached/uncached, is not secured */
    GRALLOC_USAGE_PRIVATE_IOMMU_HEAP      =       GRALLOC_USAGE_PRIVATE_2,
    /* MM heap is a carveout heap for video, can be secured*/
    GRALLOC_USAGE_PRIVATE_MM_HEAP         =       GRALLOC_USAGE_PRIVATE_3,
    /* CAMERA heap is a carveout heap for camera, is not secured*/
    GRALLOC_USAGE_PRIVATE_CAMERA_HEAP     =       0x01000000,

    /* Set this for allocating uncached memory (using O_DSYNC)
     * cannot be used with noncontiguous heaps */
    GRALLOC_USAGE_PRIVATE_UNCACHED        =       0x02000000,

    /* Buffer content should be displayed on an external display only */
    GRALLOC_USAGE_PRIVATE_EXTERNAL_ONLY   =       0x08000000,

    /* Only this buffer content should be displayed on external, even if
     * other EXTERNAL_ONLY buffers are available. Used during suspend.
     */
    GRALLOC_USAGE_PRIVATE_EXTERNAL_BLOCK  =       0x00100000,

    /* Close Caption displayed on an external display only */
    GRALLOC_USAGE_PRIVATE_EXTERNAL_CC     =       0x00200000,

    /* Use this flag to request content protected buffers. Please note
     * that this flag is different from the GRALLOC_USAGE_PROTECTED flag
     * which can be used for buffers that are not secured for DRM
     * but still need to be protected from screen captures
     */
    GRALLOC_USAGE_PRIVATE_CP_BUFFER       =       0x00400000,
};

enum {
    /* Gralloc perform enums
    */
    GRALLOC_MODULE_PERFORM_CREATE_HANDLE_FROM_BUFFER = 1,
    GRALLOC_MODULE_PERFORM_GET_STRIDE,
    GRALLOC_MODULE_PERFORM_GET_CUSTOM_STRIDE_FROM_HANDLE,
};

#define GRALLOC_HEAP_MASK   (GRALLOC_USAGE_PRIVATE_UI_CONTIG_HEAP |\
                             GRALLOC_USAGE_PRIVATE_SYSTEM_HEAP    |\
                             GRALLOC_USAGE_PRIVATE_IOMMU_HEAP     |\
                             GRALLOC_USAGE_PRIVATE_MM_HEAP        |\
                             GRALLOC_USAGE_PRIVATE_CAMERA_HEAP)
                             
#define NUM_BUFFERS 2
struct private_module_t;
struct private_handle_t;

struct private_module_t {
    gralloc_module_t base;

    struct private_handle_t* framebuffer;
    uint32_t fbFormat;
    uint32_t flags;
    uint32_t numBuffers;
    uint32_t bufferMask;
    pthread_mutex_t lock;
    buffer_handle_t currentBuffer;


    struct fb_var_screeninfo info;
    struct fb_fix_screeninfo finfo;
    float xdpi;
    float ydpi;
    float fps;
    uint32_t swapInterval;
    uint32_t currentOffset;
};

/*****************************************************************************/

#ifdef __cplusplus
struct private_handle_t : public native_handle {
#else
struct private_handle_t {
    struct native_handle nativeHandle;
#endif

    enum {
        PRIV_FLAGS_FRAMEBUFFER        = 0x00000001,
            PRIV_FLAGS_USES_PMEM          = 0x00000002,
            PRIV_FLAGS_USES_PMEM_ADSP     = 0x00000004,
            PRIV_FLAGS_USES_ION           = 0x00000008,
            PRIV_FLAGS_USES_ASHMEM        = 0x00000010,
            PRIV_FLAGS_NEEDS_FLUSH        = 0x00000020,
            PRIV_FLAGS_DO_NOT_FLUSH       = 0x00000040,
            PRIV_FLAGS_SW_LOCK            = 0x00000080,
            PRIV_FLAGS_NONCONTIGUOUS_MEM  = 0x00000100,
            // Set by HWC when storing the handle
            PRIV_FLAGS_HWC_LOCK           = 0x00000200,
            PRIV_FLAGS_SECURE_BUFFER      = 0x00000400,
            // For explicit synchronization
            PRIV_FLAGS_UNSYNCHRONIZED     = 0x00000800,
            // Not mapped in userspace
            PRIV_FLAGS_NOT_MAPPED         = 0x00001000,
            // Display on external only
            PRIV_FLAGS_EXTERNAL_ONLY      = 0x00002000,
            // Display only this buffer on external
            PRIV_FLAGS_EXTERNAL_BLOCK     = 0x00004000,
            // Display this buffer on external as close caption
            PRIV_FLAGS_EXTERNAL_CC        = 0x00008000,
            PRIV_FLAGS_VIDEO_ENCODER      = 0x00010000,
            PRIV_FLAGS_CAMERA_WRITE       = 0x00020000,
            PRIV_FLAGS_CAMERA_READ        = 0x00040000,
            PRIV_FLAGS_HW_COMPOSER        = 0x00080000,
            PRIV_FLAGS_HW_TEXTURE         = 0x00100000,
            PRIV_FLAGS_ITU_R_601          = 0x00200000,
            PRIV_FLAGS_ITU_R_601_FR       = 0x00400000,
            PRIV_FLAGS_ITU_R_709          = 0x00800000,
    };

    enum {
        READ_LOCK = 0x00000001,
        WRITE_LOCK = 0x00000002
    };

    // file-descriptors
    int     fd;
    // ints
    int     magic;
    int     flags;
    int     lock;
    int     size;
    int     offset;

    // FIXME: the attributes below should be out-of-line
    int     base;
     // The gpu address mapped into the mmu.
        int     gpuaddr;
        int     format;
        int     width;
        int     height;
    int     pid;

    struct gralloc_private_handle_t* brcm_handle;

#ifdef __cplusplus
    static const int sNumInts = 6;
    static const int sNumFds = 1;
    static const int sMagic = 0x3141592;

    private_handle_t(int fd, int size, int flags) :
        fd(fd), magic(sMagic), flags(flags), lock(0), size(size), offset(0),
        base(0), pid(getpid()), brcm_handle(0)
    {
        version = sizeof(native_handle);
        numInts = sNumInts;
        numFds = sNumFds;
    }
    ~private_handle_t() {
        magic = 0;
    }

    static int validate(const native_handle* h) {
        const private_handle_t* hnd = (const private_handle_t*)h;
        if (!h || h->version != sizeof(native_handle) ||
                h->numInts != sNumInts || h->numFds != sNumFds ||
                hnd->magic != sMagic)
        {
            ALOGE("invalid gralloc handle (at %p)", h);
            return -EINVAL;
        }
        return 0;
    }
#endif
};

#endif /* GRALLOC_PRIV_H_ */
