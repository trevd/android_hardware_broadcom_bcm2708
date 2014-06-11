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
#include <hwc.h>

#include <EGL/egl.h>

#include <gralloc/bcm_host.h>

#define HWC_DBG 1

#ifndef ALIGN_UP
#define ALIGN_UP(x,y)  ((x + (y)-1) & ~((y)-1))
#endif
/*****************************************************************************/

struct hwc_context_t {
    hwc_composer_device_t device;
    DISPMANX_DISPLAY_HANDLE_T disp;
    DISPMANX_MODEINFO_T info;
    DISPMANX_RESOURCE_HANDLE_T resources[2];
    bool selectResource;
};

struct hwc_layer_rd {
	hwc_layer_t *layer;
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
static void hwc_get_rd_layer(hwc_layer_t *src, struct hwc_layer_rd *dst){
	dst->layer = src;
	dst->format = HAL_PIXEL_FORMAT_RGB_565; //XXX: FIX THIS HACK
}

static void dump_layer(hwc_layer_t const* l) {
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

static bool hwc_can_render_layer(struct hwc_layer_rd *layer)
{
    bool ret = false;
	switch(layer->format){
	case HAL_PIXEL_FORMAT_RGB_565:
    case HAL_PIXEL_FORMAT_RGBX_8888:
    case HAL_PIXEL_FORMAT_RGBA_8888:
		ret=true;
		break;
	default:
		ret=false;
		break;
	}
	return ret;
}

static VC_IMAGE_TYPE_T hwc_format_to_vc_format(struct hwc_layer_rd *layer){
    VC_IMAGE_TYPE_T ret = VC_IMAGE_RGBA32;
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

static void* hwc_get_frame_data(const int32_t pitch, const int32_t height) {
    void* dst = malloc(pitch * height);
    memset(dst, 0, sizeof(dst));
    return dst;
}

static void hwc_set_frame_data(void *frame, hwc_layer_t *layer) {
    int dstpitch = ALIGN_UP(layer->displayFrame.right - layer->displayFrame.left*2, 32);
    int srcpitch = ALIGN_UP(layer->sourceCrop.right - layer->sourceCrop.left*2, 32);
    int y;
    int h = layer->displayFrame.bottom - layer->displayFrame.top;
	for (y = 0; y < h; y++) { 
		uint8_t *src = (uint8_t *)layer->handle + srcpitch * y;
		uint8_t *dst = (uint8_t *)frame + dstpitch * y;
		memcpy(dst, src, dstpitch); 
	}
	
}

static void hwc_actually_do_stuff_with_layer(hwc_composer_device_t *dev, hwc_layer_t *layer){
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

    struct hwc_context_t* device_context = (struct hwc_context_t*)dev;
    int dfpitch = ALIGN_UP(dfwidth*2, 32);
	
	vc_dispmanx_rect_set( &dst_rect, layer->displayFrame.left, layer->displayFrame.top, dfwidth, dfheight );
	vc_dispmanx_rect_set( &src_rect, 0, 0, 1920 << 16, 1080 << 16);
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

static void hwc_do_stuff_with_layer(hwc_composer_device_t *dev, hwc_layer_t *layer){
	
	if(layer->compositionType == HWC_OVERLAY){
		hwc_actually_do_stuff_with_layer(dev, layer);
	}

}

static int hwc_prepare(hwc_composer_device_t *dev, hwc_layer_list_t* list) {
    if (list && (list->flags & HWC_GEOMETRY_CHANGED)) {
        for (size_t i=0 ; i<list->numHwLayers ; i++) {
            //dump_layer(&list->hwLayers[i]);
			hwc_get_rd_layer(&list->hwLayers[i], lr);
			if(hwc_can_render_layer(lr)){
				if(HWC_DBG)	ALOGD("Layer %d = OVERLAY!", i);
				list->hwLayers[i].compositionType = HWC_OVERLAY;
			}else{
				if(HWC_DBG)	ALOGD("Layer %d = NOT OVERLAY!", i);
				list->hwLayers[i].compositionType = HWC_FRAMEBUFFER;
			}
        }
    }
    return 0;
}

static int hwc_set(hwc_composer_device_t *dev,
        hwc_display_t dpy,
        hwc_surface_t sur,
        hwc_layer_list_t* list)
{
    //for (size_t i=0 ; i<list->numHwLayers ; i++) {
    //    dump_layer(&list->hwLayers[i]);
    //}
	
	if(list == NULL){	//NULL list means hwc won't run or we're powering down screen
		if(dpy && sur){	//if we have dpy and sur, hwcomposer has been disabled. swap buffers and leave.
			if(HWC_DBG)	ALOGD("list == NULL");
			return eglSwapBuffers((EGLDisplay)dpy, (EGLSurface)sur) ? 0 : HWC_EGL_ERROR;
		}
	}
	

    EGLBoolean success = eglSwapBuffers((EGLDisplay)dpy, (EGLSurface)sur);
    if (!success) {
		if(HWC_DBG)	ALOGD("eglSwapBuffers errored.");
        return HWC_EGL_ERROR;
    }
	
	
	
	for (size_t i=0 ; i<list->numHwLayers ; i++) {
        hwc_do_stuff_with_layer(dev, &list->hwLayers[i]);
    }
	
		
    return 0;
}

static int hwc_device_close(struct hw_device_t *dev)
{
    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;
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
static int hwc_query(struct hwc_composer_device* dev,
                     int param, int* value)
{
    hwc_context_t* ctx = (hwc_context_t*)(dev);
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
    if (!strcmp(name, HWC_HARDWARE_COMPOSER)) {
        struct hwc_context_t *dev;
        dev = (hwc_context_t*)malloc(sizeof(*dev));

        /* initialize our state here */
        memset(dev, 0, sizeof(*dev));

        /* initialize the procs */
        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version =  HWC_DEVICE_API_VERSION_1_0;
        dev->device.common.module = const_cast<hw_module_t*>(module);
        dev->device.common.close = hwc_device_close;
		dev->device.query          = hwc_query;
        dev->device.prepare = hwc_prepare;
        dev->device.set = hwc_set;

        *device = &dev->device.common;
        status = 0;
		
		bcm_host_init();
		
	    dev->disp = vc_dispmanx_display_open( 0 );
	lr = (struct hwc_layer_rd *)malloc(sizeof(struct hwc_layer_rd));

        RECT_VARS_T *vars;
        vars = &gRectVars;
        dev->resources[0] = vc_dispmanx_resource_create(convertDisplayFormatToImageType(dev->info.input_format),
                dev->info.width, dev->info.height, &vars->vc_image_ptr);
        dev->resources[1] = vc_dispmanx_resource_create(convertDisplayFormatToImageType(dev->info.input_format),
                dev->info.width, dev->info.height, &vars->vc_image_ptr);

        dev->selectResource = false;

    }
    return status;
}
