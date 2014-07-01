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
#define LOG_TAG "HWC-BCM2708"
#include <hardware/hardware.h>

#include <fcntl.h>
#include <errno.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

#include <hardware/hwcomposer.h>


#include <EGL/egl.h>

#include <gralloc/bcm_host.h>
#include <gralloc/gralloc_priv.h>
#include <gralloc/gralloc_brcm.h>
#include <gralloc/dispmanx.h>

#define HWC_DBG 1
#define VSYNC_DEBUG 1
#define BLANK_DEBUG 1
#ifndef ALIGN_UP
#define ALIGN_UP(x,y)  ((x + (y)-1) & ~((y)-1))

#endif
#define ESUCCESS 0
enum HWCCompositionType {
    HWC_USE_GPU = HWC_FRAMEBUFFER, // This layer is to be handled by
                                   // Surfaceflinger
    HWC_USE_OVERLAY = HWC_OVERLAY, // This layer is to be handled by the overlay
    HWC_USE_BACKGROUND
                = HWC_BACKGROUND,  // This layer is to be handled by TBD
    HWC_USE_COPYBIT = 4            // This layer is to be handled by copybit
};
/*****************************************************************************/
struct DisplayAttributes {
    uint32_t vsync_period; //nanos
    uint32_t xres;
    uint32_t yres;
    uint32_t stride;
    float xdpi;
    float ydpi;
    int fd;
    bool connected; //Applies only to pluggable disp.
    //Connected does not mean it ready to use.
    //It should be active also. (UNBLANKED)
    bool isActive;
    // In pause state, composition is bypassed
    // used for WFD displays only
    bool isPause;
};
struct LayerProp {
    uint32_t mFlags; //qcom specific layer flags
    LayerProp():mFlags(0) {};
};

struct VsyncState {
    bool enable;
    bool fakevsync;
};
struct bcm2708_hwc_composer_device_1_t {
    hwc_composer_device_1_t device;
    const private_module_t  *gralloc_module;
    alloc_device_t          *alloc_device;
    const gralloc_private_handle_t* gralloc_handle ; 
     const hwc_procs_t* proc;
     DisplayAttributes dpyAttr[HWC_NUM_DISPLAY_TYPES];
     VsyncState vstate;
    DISPMANX_DISPLAY_HANDLE_T disp;
    DISPMANX_MODEINFO_T info;
    DISPMANX_RESOURCE_HANDLE_T resources[2];
    bool selectResource;
     hwc_display_t dpys[HWC_NUM_DISPLAY_TYPES];
};

struct hwc_layer_rd {
	hwc_layer_1_t *layer;
	uint32_t format;
};

typedef struct
{
    DISPMANX_DISPLAY_HANDLE_T   display;
    DISPMANX_MODEINFO_T         info;
    void                       *image;
    DISPMANX_UPDATE_HANDLE_T    update;
    DISPMANX_RESOURCE_HANDLE_T  resource;
    DISPMANX_ELEMENT_HANDLE_T   element;
    uint32_t                    vc_image_ptr;

} RECT_VARS_T;
static DISPMANX_RESOURCE_HANDLE_T  layerResource;
static RECT_VARS_T  gRectVars;
static hwc_layer_rd * lr;


static int hwc_device_open(const struct hw_module_t* module, const char* name,
                           struct hw_device_t** device);
                           
static struct hw_module_methods_t hwc_module_methods = {
    open: hwc_device_open
};

hwc_module_t HAL_MODULE_INFO_SYM = {
    common: {
        tag: HARDWARE_MODULE_TAG,
        version_major: 2,
        version_minor: 0,
        id: HWC_HARDWARE_MODULE_ID,
        name: "RaspberryPI Hardware Composer",
        author: "Trevor Drake",
        methods: &hwc_module_methods,
        dso: 0,
        reserved: {0},
    }
};
/*****************************************************************************/
static void hwc_get_rd_layer(hwc_layer_1_t *src, struct hwc_layer_rd *dst){
	//ALOGD("%s src=%p src->format=%d",__FUNCTION__,src,src->format);
	dst->layer = src;
	dst->format = HAL_PIXEL_FORMAT_RGBA_8888 ;
	//src->format; 
	// HAL_PIXEL_FORMAT_RGBA_8888; 
	//XXX: FIX THIS HACK
}
static void dump_layer(hwc_layer_1_t const* l)
{
    ALOGD("\ttype=%d, flags=%08x, handle=%p, tr=%02x, blend=%04x, {%d,%d,%d,%d}"
          ", {%d,%d,%d,%d}",
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


static int bcm2708_hwc_device_open(bcm2708_hwc_composer_device_1_t *dev)
{
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo info;
	int return_value = ESUCCESS ;
	if (hw_get_module(GRALLOC_HARDWARE_MODULE_ID, (const struct hw_module_t **)&dev->gralloc_module)) {
        ALOGE("failed to get gralloc hw module");
        return_value = -EINVAL;
        
    }

    if (gralloc_open((const hw_module_t *)dev->gralloc_module, &dev->alloc_device)) {
        ALOGE("failed to open gralloc");
        return_value = -EINVAL;
    }

    int fb_fd = open("/dev/graphics/fb0", O_RDWR);
     if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &info) == -1)
        return -errno;

    if (int(info.width) <= 0 || int(info.height) <= 0) {
        // the driver doesn't return that information
        // default to 160 dpi
        info.width  = ((info.xres * 25.4f)/160.0f + 0.5f);
        info.height = ((info.yres * 25.4f)/160.0f + 0.5f);
    }

    float xdpi = (info.xres * 25.4f) / info.width;
    float ydpi = (info.yres * 25.4f) / info.height;
    float fps  = info.reserved[3] & 0xFF;
    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) == -1)
        return -errno;

    if (finfo.smem_len <= 0)
        return -errno;

    dev->dpyAttr[HWC_DISPLAY_PRIMARY].fd = fb_fd;
    //xres, yres may not be 32 aligned
    dev->dpyAttr[HWC_DISPLAY_PRIMARY].stride = finfo.line_length /(info.xres/8);
    dev->dpyAttr[HWC_DISPLAY_PRIMARY].xres = info.xres;
    dev->dpyAttr[HWC_DISPLAY_PRIMARY].yres = info.yres;
    dev->dpyAttr[HWC_DISPLAY_PRIMARY].xdpi = xdpi;
    dev->dpyAttr[HWC_DISPLAY_PRIMARY].ydpi = ydpi;
    dev->dpyAttr[HWC_DISPLAY_PRIMARY].vsync_period = 1000000000l / fps;
    
   // size_t bpr = ( info.width*4 + (4-1)) & ~(4-1);
    //int size = bpr * info.height;
    //private_handle_t* hnd = new private_handle_t(fb_fd, size,  private_handle_t::PRIV_FLAGS_HW_COMPOSER);
		//dispmanx_alloc(hnd); 
	//	dev->gralloc_handle = hnd->brcm_handle;
    //Unblank primary on first boot
    //if(ioctl(fb_fd, FBIOBLANK,FB_BLANK_UNBLANK) < 0) {
    //    ALOGE("%s: Failed to unblank display", __FUNCTION__);
    //    return -errno;
    //}
    dev->dpyAttr[HWC_DISPLAY_PRIMARY].isActive = true;
	dev->vstate.enable = true;
    dev->vstate.fakevsync = true;
    return 0;

    
}
// hwc_can_render_layer
// Returns 0 on success and -1 on Error
static int32_t  hwc_can_render_layer(uint32_t format)
{
    int32_t ret = 0;
	switch(format){
	case HAL_PIXEL_FORMAT_RGB_565:
		ALOGD("%s HAL_PIXEL_FORMAT_RGB_565",__FUNCTION__);
		break;
    case HAL_PIXEL_FORMAT_RGBX_8888:
    	ALOGD("%s HAL_PIXEL_FORMAT_RGBX_8888",__FUNCTION__);
		break;
    case HAL_PIXEL_FORMAT_RGBA_8888:
    	ALOGD("%s HAL_PIXEL_FORMAT_RGBA_8888",__FUNCTION__);
		break;
	default:
		ret=-1;
		ALOGD("%s ret=%d format=%d",__FUNCTION__);
		break;
	}
	
	return ret;
}

static VC_IMAGE_TYPE_T hwc_format_to_vc_format(struct hwc_layer_rd *layer){
    VC_IMAGE_TYPE_T ret = VC_IMAGE_RGB565;
	switch(layer->format){
	case HAL_PIXEL_FORMAT_RGB_565:
		ret=VC_IMAGE_RGB565;
		break;
	case HAL_PIXEL_FORMAT_RGBX_8888:
		ret=VC_IMAGE_RGBX8888;
		break;
	case HAL_PIXEL_FORMAT_RGBA_8888:
		ret=VC_IMAGE_RGBA32;
		break;
	default:
		break;
	}
	return ret;
}

void hwc_dump(struct hwc_composer_device_1* dev, char *buff, int buff_len)
{
	ALOGI("%s",__FUNCTION__);
}
int hwc_getDisplayConfigs(struct hwc_composer_device_1* dev, int disp,
        uint32_t* configs, size_t* numConfigs) {
   ALOGI("%s",__FUNCTION__);
    int ret = 0;
    bcm2708_hwc_composer_device_1_t* ctx = (bcm2708_hwc_composer_device_1_t*)(dev);
    //in 1.1 there is no way to choose a config, report as config id # 0
    //This config is passed to getDisplayAttributes. Ignore for now.
    switch(disp) {
        case HWC_DISPLAY_PRIMARY:
            if(*numConfigs > 0) {
                configs[0] = 0;
                *numConfigs = 1;
            }
            ALOGI("%s HWC_DISPLAY_PRIMARY numConfigs=%d",__FUNCTION__,*numConfigs);
            ret = 0; //NO_ERROR
            break;
        case HWC_DISPLAY_EXTERNAL:
			ALOGI("%s HWC_DISPLAY_EXTERNAL",__FUNCTION__);
			ret = -1; //Not connected
            break;
        default:
            ret = -1; //Not connected
            break;
        
    }
    return ret;
}
static int hwc_eventControl(struct hwc_composer_device_1* dev, int dpy,
                             int event, int enable)
{
	ALOGI("%s event=%d",__FUNCTION__,event);
	    int ret = 0;
    bcm2708_hwc_composer_device_1_t* ctx = (bcm2708_hwc_composer_device_1_t*)(dev);
    //if(!ctx->dpyAttr[dpy].isActive) {
    //    ALOGE("Display is blanked - Cannot %s vsync",
    //          enable ? "enable" : "disable");
    //    return 0;
    //}

    switch(event) {
        case HWC_EVENT_VSYNC:
            if (ctx->vstate.enable == enable)
                break;
            ret = 0 ; //hwc_vsync_control(ctx, dpy, enable);
            if(ret == 0)
                ctx->vstate.enable = !!enable;
            ALOGD_IF (VSYNC_DEBUG, "VSYNC state changed to %s",
                      (enable)?"ENABLED":"DISABLED");
            break;
        default:
            ret = -EINVAL;
    }
    return ret;
	
}
static void hwc_registerProcs(struct hwc_composer_device_1* dev,
                              hwc_procs_t const* procs)
{
	ALOGI("%s",__FUNCTION__);
    bcm2708_hwc_composer_device_1_t* ctx = (bcm2708_hwc_composer_device_1_t*)(dev);
    if(!ctx) {
        ALOGE("%s: Invalid context", __FUNCTION__);
        return;
    }
    ctx->proc = procs;
}
static int hwc_blank(struct hwc_composer_device_1* dev, int dpy, int blank)
{


    bcm2708_hwc_composer_device_1_t* ctx = (bcm2708_hwc_composer_device_1_t*)(dev);

   
    int ret = 0;
    ALOGD_IF(BLANK_DEBUG, "%s: %s display: %d", __FUNCTION__,
          blank==1 ? "Blanking":"Unblanking", dpy);
   
    switch(dpy) {
        case HWC_DISPLAY_PRIMARY:
        if(blank) {
                ret = ioctl(ctx->dpyAttr[dpy].fd, FBIOBLANK,
                            FB_BLANK_POWERDOWN);
            } else {
                ret = ioctl(ctx->dpyAttr[dpy].fd, FBIOBLANK,FB_BLANK_UNBLANK);
               
            }
             ALOGD_IF(BLANK_DEBUG, "%s: %s display: %d ctx->dpyAttr[dpy].fd=%d", __FUNCTION__,
          blank==1 ? "Blanking":"Unblanking", dpy,ctx->dpyAttr[dpy].fd);
            break;
        case HWC_DISPLAY_EXTERNAL:
        case HWC_DISPLAY_VIRTUAL:
            break;
        default:
            return -EINVAL;
    }
    

    ALOGD_IF(BLANK_DEBUG, "%s: Done %s display: %d", __FUNCTION__,
          blank==1 ? "blanking":"unblanking", dpy);
    return 0;
}

static void* hwc_get_frame_data(const int32_t pitch, const int32_t height) {
	ALOGI("%s pitch=%d height=%d",__FUNCTION__,pitch , height);
    void* dst = malloc(pitch * height);
    memset(dst, 0, sizeof(dst));
    return dst;
}

static void hwc_set_frame_data(void *frame, hwc_layer_1_t *layer) {
	ALOGD("%s frame=%p layer=%p",__FUNCTION__,frame,layer);
    int dstpitch = ALIGN_UP(layer->displayFrame.right - layer->displayFrame.left*2, 32);
    int srcpitch = ALIGN_UP(layer->sourceCrop.right - layer->sourceCrop.left*2, 32);
    int y;
    int h = layer->displayFrame.bottom - layer->displayFrame.top;
    ALOGD("%s layer->displayFrame.bottom=%d layer->displayFrame.top=%d",__FUNCTION__,layer->displayFrame.bottom ,layer->displayFrame.top);
	/*for (y = 0; y < h; y++) { 
		uint8_t *src = (uint8_t *)layer->handle + srcpitch * y;
		uint8_t *dst = (uint8_t *)frame + dstpitch * y;
		memcpy(dst, src, dstpitch); 
	}*/
	
}

static void hwc_actually_do_stuff_with_layer(hwc_composer_device_1_t *dev, hwc_layer_1_t *layer){
	ALOGI("%s",__FUNCTION__);
    RECT_VARS_T    *vars;
	vars = &gRectVars;
	VC_RECT_T       dst_rect, src_rect;
	hwc_get_rd_layer(layer, lr);
	VC_IMAGE_TYPE_T type = hwc_format_to_vc_format(lr);

	VC_DISPMANX_ALPHA_T alpha = { DISPMANX_FLAGS_ALPHA_FROM_SOURCE, 
                             120, /*alpha 0->255*/
                             0 };

	
	//layer->handle is the buffer
	//layer->sourceCrop.left/right/top/bottom is which part of the buffer to display
	//layer->displayFrame.left/right/top/bottom is WHERE it should be displayed
	//to solve this, we're going to need to make the buffer into an image, crop it, then apply to the displayFrame.
	
	int dfwidth = layer->displayFrame.right - layer->displayFrame.left;
	int dfheight = layer->displayFrame.bottom - layer->displayFrame.top;
	int srcwidth = layer->sourceCrop.right - layer->sourceCrop.left;
	int srcheight = layer->sourceCrop.bottom - layer->sourceCrop.top;

    struct bcm2708_hwc_composer_device_1_t* device_context = (struct bcm2708_hwc_composer_device_1_t*)dev;
    int dfpitch = ALIGN_UP(dfwidth*2, 32);
	
	vc_dispmanx_rect_set( &dst_rect, layer->displayFrame.left, layer->displayFrame.top, dfwidth, dfheight );
	vc_dispmanx_rect_set( &src_rect, 0, 0, dfwidth << 16, dfheight << 16);
	void* frame = hwc_get_frame_data(dfpitch, dfheight);
	hwc_set_frame_data(frame, layer);
    int ret = vc_dispmanx_resource_write_data(  layerResource,
												type,
												dfpitch,
												frame,
												&dst_rect );

	if(ret != 0){
		if(HWC_DBG)	ALOGD("vc_dispmanx_resource_write_data failed.");
		return;
	}
	vc_dispmanx_rect_set( &dst_rect, layer->displayFrame.left, layer->displayFrame.top, dfwidth, dfheight );

	DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
	
	vc_dispmanx_element_add(
						update,
						device_context->disp,
						0/*layer*/,
						&dst_rect,
						layerResource,
						&src_rect,
						DISPMANX_PROTECTION_NONE,
						0 /*alpha*/,
						0/*clamp*/,
						(DISPMANX_TRANSFORM_T) 0/*transform*/
						); 

	
	
	
	
	
	vc_dispmanx_update_submit_sync(update);
	if (ret != 0) {
	    if (HWC_DBG) ALOGD("vc_dispmanx_update_submit_sync failed. %d", ret);
	    return;
	}

	device_context->selectResource = !device_context->selectResource;
	free(frame);
}

static void hwc_do_stuff_with_layer(hwc_composer_device_1_t *dev, hwc_layer_1_t *layer){
	ALOGI("%s",__FUNCTION__);
	if(layer->compositionType == HWC_OVERLAY){
	//	hwc_actually_do_stuff_with_layer(dev, layer);
	}

}

static int hwc_prepare(hwc_composer_device_1 *dev, size_t numDisplays,
                       hwc_display_contents_1_t** displays)
{
	ALOGI("%s numDisplays=%d",__FUNCTION__,numDisplays);
    for (int32_t i = numDisplays - 1; i >= 0; i--) {
        hwc_display_contents_1_t *list = displays[i];
        if (list && (list->flags & HWC_GEOMETRY_CHANGED)) {
			ALOGD("%s EGLDisplay dpy=%p sur=%p",__FUNCTION__,list->dpy,list->sur);
			for (size_t i=0 ; i<list->numHwLayers ; i++) {
				dump_layer(&list->hwLayers[i]);
				hwc_get_rd_layer(&list->hwLayers[i], lr);
				if(!hwc_can_render_layer(lr->format)){
					if(HWC_DBG)	ALOGD("Layer %d = OVERLAY!", i);
					list->hwLayers[i].compositionType = HWC_FRAMEBUFFER;
				}else{
					if(HWC_DBG)	ALOGD("Layer %d = NOT OVERLAY!", i);
					list->hwLayers[i].compositionType = HWC_FRAMEBUFFER;
				}
			}
		}
    }
    return 0;
}
int hwc_getDisplayAttributes(struct hwc_composer_device_1* dev, int disp,
        uint32_t config, const uint32_t* attributes, int32_t* values) {
	ALOGI("%s",__FUNCTION__);
   
    bcm2708_hwc_composer_device_1_t* ctx = (bcm2708_hwc_composer_device_1_t*)(dev);
    //If hotpluggable displays are inactive return error
    //if(disp == HWC_DISPLAY_EXTERNAL && !ctx->dpyAttr[disp].connected) {
    //    return -1;
    //}

    //From HWComposer
    static const uint32_t DISPLAY_ATTRIBUTES[] = {
        HWC_DISPLAY_VSYNC_PERIOD,
        HWC_DISPLAY_WIDTH,
        HWC_DISPLAY_HEIGHT,
        HWC_DISPLAY_DPI_X,
        HWC_DISPLAY_DPI_Y,
        HWC_DISPLAY_NO_ATTRIBUTE,
    };

    const int NUM_DISPLAY_ATTRIBUTES = (sizeof(DISPLAY_ATTRIBUTES) /
            sizeof(DISPLAY_ATTRIBUTES)[0]);

    for (size_t i = 0; i < NUM_DISPLAY_ATTRIBUTES - 1; i++) {
        switch (attributes[i]) {
        case HWC_DISPLAY_VSYNC_PERIOD:
            values[i] = ctx->dpyAttr[disp].vsync_period;
            break;
        case HWC_DISPLAY_WIDTH:
            values[i] = ctx->dpyAttr[disp].xres;
            ALOGD("%s disp = %d, width = %d",__FUNCTION__, disp,
                    ctx->dpyAttr[disp].xres);
            break;
        case HWC_DISPLAY_HEIGHT:
            values[i] = ctx->dpyAttr[disp].yres;
            ALOGD("%s disp = %d, height = %d",__FUNCTION__, disp,
                    ctx->dpyAttr[disp].yres);
            break;
        case HWC_DISPLAY_DPI_X:
            values[i] = (int32_t) (ctx->dpyAttr[disp].xdpi*1000.0);
            break;
        case HWC_DISPLAY_DPI_Y:
            values[i] = (int32_t) (ctx->dpyAttr[disp].ydpi*1000.0);
            break;
        default:
            ALOGE("Unknown display attribute %d",
                    attributes[i]);
            return -EINVAL;
        }
    }
    return 0;
}

static int hwc_set(hwc_composer_device_1 *dev,
                   size_t numDisplays,
                   hwc_display_contents_1_t** displays)
{
    ALOGI("%s",__FUNCTION__);

	  int ret = 0;
	const int dpy = HWC_DISPLAY_PRIMARY;
    bcm2708_hwc_composer_device_1_t* ctx = (bcm2708_hwc_composer_device_1_t*)(dev);
    for (uint32_t i = 0; i < numDisplays; i++) {
        hwc_display_contents_1_t* list = displays[i];
            for (size_t i=0 ; i<list->numHwLayers ; i++) {
        dump_layer(&list->hwLayers[i]);
    }
		if(list == NULL){	//NULL list means hwc won't run or we're powering down screen
			if(dpy && list->sur){	//if we have dpy and sur, hwcomposer has been disabled. swap buffers and leave.
				if(HWC_DBG)	ALOGD("list == NULL");
				return eglSwapBuffers((EGLDisplay)list->dpy, (EGLSurface)list->sur) ? 0 : HWC_EGL_ERROR;
			}
		}
		
	ALOGI("%s EGLDisplay dpy=%p sur=%p cdpy=%p csur=%p",__FUNCTION__,(EGLDisplay)list->dpy,(EGLSurface)list->sur,eglGetCurrentDisplay(),eglGetCurrentSurface(EGL_DRAW));
		EGLBoolean success = eglSwapBuffers(eglGetCurrentDisplay(),eglGetCurrentSurface(EGL_DRAW));
		if (!success) {
			if(HWC_DBG)	ALOGD("eglSwapBuffers errored.");
			return HWC_EGL_ERROR;
		}
		
		
		
	/*	for (size_t i=0 ; i<list->numHwLayers ; i++) {
			hwc_do_stuff_with_layer(dev, &list->hwLayers[i]);
		}*/
	}
	
		
    return 0;
}

static int hwc_device_close(struct hw_device_t *dev)
{
    ALOGI("%s",__FUNCTION__);
    struct bcm2708_hwc_composer_device_1_t* ctx = (struct bcm2708_hwc_composer_device_1_t*)dev;
    if (ctx) {
        vc_dispmanx_resource_delete(ctx->resources[ctx->selectResource]);
        ctx->selectResource = !ctx->selectResource;
        vc_dispmanx_resource_delete(ctx->resources[ctx->selectResource]);
        vc_dispmanx_display_close(ctx->disp);
        free(ctx);
    }
    free(lr);
    return 0;
}
static int hwc_query(struct hwc_composer_device_1* dev,
                     int param, int* value)
{
    ALOGI("%s",__FUNCTION__);
    bcm2708_hwc_composer_device_1_t* ctx = (bcm2708_hwc_composer_device_1_t*)(dev);
    //private_module_t* m = reinterpret_cast<private_module_t*>(/
     //   ctx->mFbDev->common.module);

    switch (param) {
    case HWC_BACKGROUND_LAYER_SUPPORTED:
        // Not supported for now
        value[0] = 0;
        break;
    case HWC_VSYNC_PERIOD:
        value[0] = 1000000000.0 / 60 ; //m->fps;
        ALOGI("fps: %d", value[0]);
        break;
    default:
        return -EINVAL;
    }
    return 0;
}
/*****************************************************************************/

static VC_IMAGE_TYPE_T convertDisplayFormatToImageType(const DISPLAY_INPUT_FORMAT_T &format) {
    VC_IMAGE_TYPE_T output_format = VC_IMAGE_RGB565;
    if (format == VCOS_DISPLAY_INPUT_FORMAT_RGB888) {
        output_format = VC_IMAGE_RGB888;
    }
    return output_format;
}

static int hwc_device_open(const struct hw_module_t* module, const char* name,
                           struct hw_device_t** device)
{
    int status = -EINVAL;
ALOGI("%s",__FUNCTION__);
    if (!strcmp(name, HWC_HARDWARE_COMPOSER)) {
        struct bcm2708_hwc_composer_device_1_t *dev;
        dev = (bcm2708_hwc_composer_device_1_t*)malloc(sizeof(*dev));
        memset(dev, 0, sizeof(*dev));
		bcm2708_hwc_device_open(dev) ;
        //Initialize hwc context
        //initContext(dev);

        /* initialize the procs */
         dev->device.common.tag          = HARDWARE_DEVICE_TAG;
        dev->device.common.version      = HWC_DEVICE_API_VERSION_1_3;
        dev->device.common.module       = const_cast<hw_module_t*>(module);
        dev->device.common.close        = hwc_device_close;
        dev->device.prepare             = hwc_prepare;
        dev->device.set                 = hwc_set;
        dev->device.eventControl        = hwc_eventControl;
        dev->device.blank               = hwc_blank;
        dev->device.query               = hwc_query;
        dev->device.registerProcs       = hwc_registerProcs;
        dev->device.dump                = hwc_dump;
        dev->device.getDisplayConfigs   = hwc_getDisplayConfigs;
        dev->device.getDisplayAttributes = hwc_getDisplayAttributes;
        *device = &dev->device.common;
        
     //dev->disp = vc_dispmanx_display_open( 0 );
     ALOGI("%s dev->disp =%p",__FUNCTION__,dev->disp );
        status = 0;
		lr = (struct hwc_layer_rd *)malloc(sizeof(struct hwc_layer_rd));
		//bcm_host_init();
		
	    
	
/*ALOGI("%s dev->info.input_format=%d",__FUNCTION__,dev->info.input_format);
        RECT_VARS_T *vars;
        vars = &gRectVars;
        dev->resources[0] = vc_dispmanx_resource_create(convertDisplayFormatToImageType(dev->info.input_format),
                dev->info.width, dev->info.height, &vars->vc_image_ptr);
        dev->resources[1] = vc_dispmanx_resource_create(convertDisplayFormatToImageType(dev->info.input_format),
                dev->info.width, dev->info.height, &vars->vc_image_ptr);

        dev->selectResource = false;
*/
    }
    return status;
}

