/*
 * Copyright (C) 2010 The Android Open Source Project
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

#include <hardware/hardware.h>

#include <fcntl.h>
#include <errno.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

#include <hardware/hwcomposer.h>



#include <EGL/egl.h>


#include "gralloc_private.h"
#include <linux/fb.h>

/*****************************************************************************/

struct hwc_context_t {
    hwc_composer_device_1_t device;
    /* our private state goes below here */
    pthread_t               vsync_thread;
    hwc_procs_t *procs;
    const private_module_t  *gralloc_module;
     alloc_device_t          *alloc_device;
    int fb_fd ;
    uint32_t vsync_period;
    uint32_t xres;
    uint32_t yres;
    uint32_t stride;
    float xdpi;
    float ydpi;
    float fps;
    
};

static int hwc_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device);

static struct hw_module_methods_t hwc_module_methods = {
    open: hwc_device_open
};

hwc_module_t HAL_MODULE_INFO_SYM = {
    common: {
        tag: HARDWARE_MODULE_TAG,
        version_major: 1,
        version_minor: 0,
        id: HWC_HARDWARE_MODULE_ID,
        name: "Sample hwcomposer module",
        author: "The Android Open Source Project",
        methods: &hwc_module_methods,
    }
};

/*****************************************************************************/
static int hwc_create_display_attributes(struct hwc_context_t *dev){
    
    ALOGD("%s:%d creating display attributes", __FUNCTION__,__LINE__);
    dev->fb_fd = open("/dev/graphics/fb0", O_RDWR);
    
    int errornum = 0 ; 
    if(dev->fb_fd == -1){
	errornum = errno ;
	ALOGE("%s:%d cannot open framebuffer device", __FUNCTION__,__LINE__);
	goto exit_with_error;
    }
    
    struct fb_var_screeninfo vinfo;
    if (ioctl(dev->fb_fd, FBIOGET_VSCREENINFO, &vinfo) == -1){
	errornum = errno ;
	ALOGE("%s:%d FBIOGET_VSCREENINFO failed for framebuffer fb_fd=%d err=%d %s", __FUNCTION__,__LINE__,dev->fb_fd,errornum,strerror(errornum));
	goto exit_with_error;
    }
    if (int(vinfo.width) <= 0 || int(vinfo.height) <= 0) {
	// the driver doesn't return that information
        // default to 160 dpi
        vinfo.width  = ((vinfo.xres * 25.4f)/160.0f + 0.5f);
        vinfo.height = ((vinfo.yres * 25.4f)/160.0f + 0.5f);
    }
    dev->xdpi = (vinfo.xres * 25.4f) / vinfo.width;
    dev->ydpi = (vinfo.yres * 25.4f) / vinfo.height;
    dev->xres = vinfo.xres;
    dev->yres = vinfo.yres;
    dev->fps  = vinfo.reserved[3] & 0xFF;
    
    struct fb_fix_screeninfo finfo;
    if (ioctl(dev->fb_fd, FBIOGET_FSCREENINFO, &finfo) == -1){
       errornum = errno ;
	ALOGE("%s:%d FBIOGET_FSCREENINFO failed for framebuffer fb_fd=%d err=%d %s", __FUNCTION__,__LINE__,dev->fb_fd,errornum,strerror(errornum));
	goto exit_with_error;
    }
    if (finfo.smem_len <= 0){
	errornum = EINVAL;
	ALOGE("%s:%d Invalid fix_info.smem_len ( %d ) ", __FUNCTION__,__LINE__,finfo.smem_len);
        goto exit_with_error;
    }
    dev->stride = finfo.line_length /(vinfo.xres/8);

exit_with_error:
    if(dev->fb_fd != -1) 
	close(dev->fb_fd);
    
    errno = errornum;
    return errornum;
    
}
static void *hwc_vsync_thread(void *data)
{
	return NULL;
}
static int hwc_create_vsync_thread(struct hwc_context_t *dev)
{
    ALOGD("%s:%d creating vsync thread", __FUNCTION__,__LINE__);
    int return_value = pthread_create(&dev->vsync_thread, NULL, hwc_vsync_thread, dev);
    if (return_value) {
	ALOGE("%s:%d failed to start vsync thread: %s", __FUNCTION__,__LINE__ ,strerror(return_value));
        return_value = -return_value;
    }
    return return_value;
}
static void dump_layer(hwc_layer_1_t const* l) {
    ALOGD("\ttype=%d, flags=%08x, handle=%p, tr=%02x, blend=%04x, {%d,%d,%d,%d}, {%d,%d,%d,%d}",
            l->compositionType, l->flags, l->handle, l->transform, l->blending,
            l->sourceCrop.left,
            l->sourceCrop.top,
            l->sourceCrop.right,
            l->sourceCrop.bottom,
            l->displayFrame.left,
            l->displayFrame.top,
            l->displayFrame.right,
            l->displayFrame.bottom);
}

static int hwc_prepare(hwc_composer_device_1_t *device,size_t numDisplays, hwc_display_contents_1_t** displays) 
{
     ALOGD("%s:%d numDisplays=%d displays=%p", __FUNCTION__,__LINE__,numDisplays,displays);
   
     if (displays && (displays[0]->flags & HWC_GEOMETRY_CHANGED)) {
        for (size_t i=0 ; i<displays[0]->numHwLayers ; i++) {
            //dump_layer(&list->hwLayers[i]);
            displays[0]->hwLayers[i].compositionType = HWC_FRAMEBUFFER;
        }
    }
    return 0;
}

static int hwc_set(hwc_composer_device_1_t *device,size_t numDisplays, hwc_display_contents_1_t** displays)
{
    //for (size_t i=0 ; i<list->numHwLayers ; i++) {
    //    dump_layer(&list->hwLayers[i]);
    //}
     return 0;
    ALOGD("%s:%d numDisplays=%d displays=%p displays[0]->dpy=0x%x displays[0]->sur=%p", __FUNCTION__,__LINE__,numDisplays,displays,displays[0]->dpy,displays[0]->sur);
    EGLBoolean sucess = eglSwapBuffers((EGLDisplay)displays[0]->dpy, (EGLSurface)displays[0]->sur);
    if (!sucess) {
        return HWC_EGL_ERROR;
    }
   
}
static int hwc_event_control(struct hwc_composer_device_1 *device, int dpy,int event, int enabled)
{
    struct hwc_context_t* dev = (struct hwc_context_t*)device;
    ALOGD("%s:%d event=%d enabled=%d", __FUNCTION__,__LINE__,event,enabled);
    int return_value = -EINVAL ;
    switch (event) {
	case HWC_EVENT_VSYNC:
		return_value = 0 ;
	    
    }
    ALOGD("%s:%d return_value=%d", __FUNCTION__,__LINE__,return_value);
    return return_value;
}

// toggle display on or off
static int hwc_blank(struct hwc_composer_device_1* device, int disp, int blank)
{
    // dummy implementation for now
    ALOGD("%s:%d disp=%d blank=%d", __FUNCTION__,__LINE__,disp,blank);
    return 0;
}
static int hwc_query(struct hwc_composer_device_1* device,int param, int* value)
{
    // dummy implementation for now
    ALOGD("%s:%d param=%d value=%p", __FUNCTION__,__LINE__,param,value);
    int return_value = 0 ; 
    switch(param){
	case   HWC_DISPLAY_TYPES_SUPPORTED:
	    value[0] = HWC_DISPLAY_PRIMARY_BIT;
	    break;
	case  HWC_BACKGROUND_LAYER_SUPPORTED:
	    value[0] = 0;
	    break;
	default:
	    return_value = -1; 
	
    }
    return return_value;
}
static void hwc_dump(hwc_composer_device_1* device, char *buff, int buff_len)
{
    // dummy implementation for now
    ALOGD("%s:%d buff=%s buff_len=%d", __FUNCTION__,__LINE__,buff,buff_len);


}
static int hwc_get_display_configs(struct hwc_composer_device_1 *device,int disp, uint32_t *configs, size_t *numConfigs)
{
    // dummy implementation for now
    int return_value = -EINVAL;
    switch(disp) {
	case HWC_DISPLAY_PRIMARY:
            if(*numConfigs > 0) {
                configs[0] = 0;
                *numConfigs = 1;
            }
	    ALOGD("%s:%d HWC_DISPLAY_PRIMARY disp=%d configs=%p numConfigs=%p", __FUNCTION__,__LINE__,disp,configs,numConfigs);
	    return_value = 0 ;
	    break ; 
	default:
	    ALOGE("%s:%d Display Not Supported disp=%d configs=%p numConfigs=%p", __FUNCTION__,__LINE__,disp,configs,numConfigs);
	     *numConfigs = 0;
            break;
    }
    return return_value;
    
}
static int hwc_get_display_attributes(struct hwc_composer_device_1 *device,int disp, uint32_t config, const uint32_t *attributes, int32_t *values)
{
    // dummy implementation for now
    ALOGD("%s:%d disp=%d config=%d attributes=%p values=%p", __FUNCTION__,__LINE__,disp,config,attributes,values);
    struct hwc_context_t* dev = (struct hwc_context_t*)device;
    static const uint32_t DISPLAY_ATTRIBUTES[] = {
        HWC_DISPLAY_VSYNC_PERIOD,
        HWC_DISPLAY_WIDTH,
        HWC_DISPLAY_HEIGHT,
        HWC_DISPLAY_DPI_X,
        HWC_DISPLAY_DPI_Y,
        HWC_DISPLAY_NO_ATTRIBUTE,
    };

    const int NUM_DISPLAY_ATTRIBUTES = (sizeof(DISPLAY_ATTRIBUTES) / sizeof(DISPLAY_ATTRIBUTES)[0]);

    for (size_t i = 0; i < NUM_DISPLAY_ATTRIBUTES - 1; i++) {
        switch (attributes[i]) {
        case HWC_DISPLAY_VSYNC_PERIOD:
            values[i] = dev->vsync_period;
            break;
        case HWC_DISPLAY_WIDTH:
            values[i] = dev->xres;
            ALOGD("%s disp = %d, width = %d",__FUNCTION__, disp,dev->xres);
            break;
        case HWC_DISPLAY_HEIGHT:
            values[i] = dev->yres;
            ALOGD("%s disp = %d, height = %d",__FUNCTION__, disp,dev->yres);
            break;
        case HWC_DISPLAY_DPI_X:
            values[i] = (int32_t) (dev->xdpi*1000.0);
            break;
        case HWC_DISPLAY_DPI_Y:
            values[i] = (int32_t) (dev->ydpi*1000.0);
            break;
        default:
            ALOGE("Unknown display attribute %d", attributes[i]);
            return -EINVAL;
        }
    }
    return 0;

}
static void hwc_register_procs(hwc_composer_device_1_t *dev, hwc_procs_t const* procs)
{
    ALOGD("%s:%d procs=%p", __FUNCTION__,__LINE__,procs);
    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;
    ctx->procs = (hwc_procs_t *) procs;
}

static int hwc_device_close(struct hw_device_t *device)
{
    ALOGD("%s",__FUNCTION__);
    struct hwc_context_t* dev = (struct hwc_context_t*)device;
    pthread_kill(dev->vsync_thread, SIGTERM);
    pthread_join(dev->vsync_thread, NULL);
    if (dev) {
	
        free(dev);
    }
    return 0;
}

/*****************************************************************************/

static int hwc_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
     ALOGD("%s",__FUNCTION__);
    int status = -EINVAL;
    if (!strcmp(name, HWC_HARDWARE_COMPOSER)) {
        struct hwc_context_t *dev;
        dev = (hwc_context_t*)malloc(sizeof(*dev));
	int ret =-1 ;
        /* initialize our state here */
        memset(dev, 0, sizeof(*dev));

        /* initialize the procs */
        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version = HWC_DEVICE_API_VERSION_1_1;
        dev->device.common.module = const_cast<hw_module_t*>(module);
        dev->device.common.close = hwc_device_close;

        dev->device.prepare = hwc_prepare;
        dev->device.set = hwc_set;
	
	
	dev->device.eventControl = hwc_event_control;
	dev->device.blank = hwc_blank;
	dev->device.query = hwc_query;
	dev->device.registerProcs = hwc_register_procs;
	dev->device.dump = hwc_dump;
	dev->device.getDisplayConfigs = hwc_get_display_configs;
	dev->device.getDisplayAttributes = hwc_get_display_attributes;
	
	if (hw_get_module(GRALLOC_HARDWARE_MODULE_ID,
            (const struct hw_module_t **)&dev->gralloc_module)) {
	    ALOGE("failed to get gralloc hw module");
	    ret = -EINVAL;
	    goto err_get_module;
	}

	if (gralloc_open((const hw_module_t *)dev->gralloc_module,
		&dev->alloc_device)) {
	    ALOGE("failed to open gralloc");
	    ret = -EINVAL;
	    goto err_get_module;
	}
		
        *device = &dev->device.common;
	
	// setup a vsync worker thread
	hwc_create_vsync_thread(dev);
	hwc_create_display_attributes(dev);
	
        status = 0;
    }
err_get_module:
    return status;
}

