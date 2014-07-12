/*
Copyright (c) 2012, Broadcom Europe Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#define VCOS_LOG_CATEGORY (&khrn_client_log)
#define LOG_TAG "khrn_client_platform_android_vchiq"
#include <utils/Log.h>
#include <khronos/common/khrn_client_platform.h>
#include <khronos/common/khrn_client.h>
#include <khronos/common/khrn_client_rpc.h>
#include <khronos/common/khrn_int_ids.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <vmcs_host/vc_host.h>

#include <system/window.h>
extern VCOS_LOG_CAT_T khrn_client_log;

extern void vc_vchi_khronos_init();

static void send_bound_pixmaps(void);

//see helpers\scalerlib\scalerlib_misc.c
//int32_t scalerlib_convert_vcimage_to_display_element()
//dark blue, 1<<3 in 888
#define CHROMA_KEY_565 0x0001
//



VCOS_STATUS_T khronos_platform_semaphore_create(PLATFORM_SEMAPHORE_T *sem, int name[3], int count)
{
   //ALOGD("%s name[0]=%d name[1]=%d name[2]=%d count=%d",__FUNCTION__,name[0], name[1], name[2],count);
   char buf[64];
   vcos_snprintf(buf,sizeof(buf),"KhanSemaphore%08x%08x%08x", name[0], name[1], name[2]);
   return vcos_named_semaphore_create(sem, buf, count);
}

uint64_t khronos_platform_get_process_id()
{
   uint64_t result =vcos_process_id_current();
   //ALOGI("%s %lld",__FUNCTION__,result);
   
   return result;
}

static bool process_attached = false;

void *platform_tls_get(PLATFORM_TLS_T tls)
{
   void *ret;
	//ALOGD("%s",__FUNCTION__);
   if (!process_attached)
      /* TODO: this isn't thread safe */
   {
      ALOGD("Attaching process");
      client_process_attach();
      process_attached = true;
      tls = client_tls;

      vc_vchi_khronos_init();
   }

   ret = vcos_tls_get(tls);
   if (!ret)
   {
     /* The problem here is that on VCFW, the first notification we get that a thread
       * exists at all is when it calls an arbitrary EGL function. We need to detect this
       * case and initiliase the per-thread state.
       *
       * On Windows this gets done in DllMain.
       */
      client_thread_attach();
      vcos_thread_at_exit(client_thread_detach, NULL);
      ret = vcos_tls_get(tls);
   }
   return ret;
}

void *platform_tls_get_check(PLATFORM_TLS_T tls)
{
  //ALOGD("%s",__FUNCTION__);
   return platform_tls_get(tls);
}

/* ----------------------------------------------------------------------
 * workaround for broken platforms which don't detect threads exiting
 * -------------------------------------------------------------------- */
void platform_hint_thread_finished()
{
   /*
      todo: should we do this:

      vcos_thread_deregister_at_exit(client_thread_detach);
      client_thread_detach();

      here?
   */
}

/**
   Allocate memory

   @param size Size in bytes of memory block to allocate
   @return pointer to memory block
**/
void *khrn_platform_malloc(size_t size, const char * name)
{
   return vcos_malloc(size, name);
}
/**
   Free memory
   @param v Pointer to  memory area to free
**/
void khrn_platform_free(void *v)
{
   if (v){
      vcos_free(v);
   }
}

static KHRN_IMAGE_FORMAT_T convert_format(uint32_t format)
{	
	ALOGI("%s",__FUNCTION__);
   switch (format & ~EGL_PIXEL_FORMAT_USAGE_MASK_BRCM) {
      case EGL_PIXEL_FORMAT_ARGB_8888_PRE_BRCM: return (KHRN_IMAGE_FORMAT_T)(ABGR_8888 | IMAGE_FORMAT_PRE);
      case EGL_PIXEL_FORMAT_ARGB_8888_BRCM:     return ABGR_8888;
      case EGL_PIXEL_FORMAT_XRGB_8888_BRCM:     return XBGR_8888;
      case EGL_PIXEL_FORMAT_RGB_565_BRCM:       return RGB_565;
      case EGL_PIXEL_FORMAT_A_8_BRCM:           return A_8;
      default:                                  vcos_verify(0); return (KHRN_IMAGE_FORMAT_T)0;
   }
}

bool platform_get_pixmap_info(EGLNativePixmapType pixmap, KHRN_IMAGE_WRAP_T *image)
{
 ALOGI("%s",__FUNCTION__);
  image->format = convert_format(((uint32_t *)pixmap)[4]);
   image->width = ((uint32_t *)pixmap)[2];
   image->height = ((uint32_t *)pixmap)[3];

   /* can't actually access data */
   image->stride = 0;
   image->aux = 0;
   image->storage = 0;

   return image->format != 0;
}
void khrn_platform_release_pixmap_info(EGLNativePixmapType pixmap, KHRN_IMAGE_WRAP_T *image)
{
   /* Nothing to do */
}


void platform_get_pixmap_server_handle(EGLNativePixmapType pixmap, uint32_t *handle)
{
  ALOGI("%s",__FUNCTION__);
   handle[0] = ((uint32_t *)pixmap)[0];
   handle[1] = ((uint32_t *)pixmap)[1];
}

bool platform_match_pixmap_api_support(EGLNativePixmapType pixmap, uint32_t api_support)
{
  ALOGI("%s",__FUNCTION__);
   return
      (!(api_support & EGL_OPENGL_BIT) || (((uint32_t *)pixmap)[4] & EGL_PIXEL_FORMAT_RENDER_GL_BRCM)) &&
      (!(api_support & EGL_OPENGL_ES_BIT) || (((uint32_t *)pixmap)[4] & EGL_PIXEL_FORMAT_RENDER_GLES_BRCM)) &&
      (!(api_support & EGL_OPENGL_ES2_BIT) || (((uint32_t *)pixmap)[4] & EGL_PIXEL_FORMAT_RENDER_GLES2_BRCM)) &&
      (!(api_support & EGL_OPENVG_BIT) || (((uint32_t *)pixmap)[4] & EGL_PIXEL_FORMAT_RENDER_VG_BRCM));
}



bool platform_use_global_image_as_egl_image(uint32_t id_0, uint32_t id_1, EGLNativePixmapType pixmap, EGLint *error)
{
   ALOGI("%s",__FUNCTION__);
   return true;
}

void platform_acquire_global_image(uint32_t id_0, uint32_t id_1)
{
}

void platform_release_global_image(uint32_t id_0, uint32_t id_1)
{
}

void platform_get_global_image_info(uint32_t id_0, uint32_t id_1,
   uint32_t *pixel_format, uint32_t *width, uint32_t *height)
{
	ALOGI("%s",__FUNCTION__);
   EGLint id[2] = {id_0, id_1};
   EGLint width_height_pixel_format[3];
   verify(eglQueryGlobalImageBRCM(id, width_height_pixel_format));
   width_height_pixel_format[2] |=
      /* this isn't right (the flags should be those passed in to
       * eglCreateGlobalImageBRCM), but this stuff is just for basic testing, so
       * it doesn't really matter */
      EGL_PIXEL_FORMAT_RENDER_GLES_BRCM | EGL_PIXEL_FORMAT_RENDER_GLES2_BRCM |
      EGL_PIXEL_FORMAT_RENDER_VG_BRCM | EGL_PIXEL_FORMAT_VG_IMAGE_BRCM |
      EGL_PIXEL_FORMAT_GLES_TEXTURE_BRCM | EGL_PIXEL_FORMAT_GLES2_TEXTURE_BRCM;
   if (pixel_format) { *pixel_format = width_height_pixel_format[2]; }
   if (width) { *width = width_height_pixel_format[0]; }
   if (height) { *height = width_height_pixel_format[1]; }
}



void platform_client_lock(void)
{
	//ALOGD("%s",__FUNCTION__);
   platform_mutex_acquire(&client_mutex);
}

void platform_client_release(void)
{
	//ALOGD("%s",__FUNCTION__);
   platform_mutex_release(&client_mutex);
}

void platform_init_rpc(struct CLIENT_THREAD_STATE *state)
{
	ALOGD("%s",__FUNCTION__);
   assert(1);
}

void platform_term_rpc(struct CLIENT_THREAD_STATE *state)
{
	ALOGD("%s",__FUNCTION__);
   assert(1);
}

void platform_maybe_free_process(void)
{
	ALOGD("%s",__FUNCTION__);
   assert(1);
}

void platform_destroy_winhandle(void *a, uint32_t b)
{
	ALOGD("%s",__FUNCTION__);
   assert(1);
}

void platform_surface_update(uint32_t handle)
{
		ALOGI("%s handle=%p",__FUNCTION__,handle);
   /*
   XXX This seems as good a place as any to do the client side pixmap hack.
   (called from eglSwapBuffers)
   */
   send_bound_pixmaps();
}

void egl_gce_win_change_image(void)
{
	ALOGI("%s",__FUNCTION__);
   assert(0);
}

void platform_retrieve_pixmap_completed(EGLNativePixmapType pixmap)
{
	ALOGI("%s",__FUNCTION__);
   assert(0);
}

void platform_send_pixmap_completed(EGLNativePixmapType pixmap)
{
	ALOGI("%s",__FUNCTION__);
   assert(0);
}

uint32_t platform_memcmp(const void * aLeft, const void * aRight, size_t aLen)
{
   return memcmp(aLeft, aRight, aLen);
}

void platform_memcpy(void * aTrg, const void * aSrc, size_t aLength)
{
	ALOGI("%s",__FUNCTION__);
   memcpy(aTrg, aSrc, aLength);
}



EGLDisplay khrn_platform_set_display_id(EGLNativeDisplayType display_id)
{
   if (display_id == EGL_DEFAULT_DISPLAY)
      return (EGLDisplay)1;
   else
      return EGL_NO_DISPLAY;
}

uint32_t khrn_platform_get_window_position(EGLNativeWindowType win)
{
	
   return 0;
}

#define NUM_PIXMAP_BINDINGS 16
static struct
{
   bool used;
   bool send;
   EGLNativePixmapType pixmap;
   EGLImageKHR egl_image;
} pixmap_binding[NUM_PIXMAP_BINDINGS];

static void set_egl_image_color_data(EGLImageKHR egl_image, KHRN_IMAGE_WRAP_T *image)
{
	ALOGI("%s",__FUNCTION__);
   int line_size = (image->stride < 0) ? -image->stride : image->stride;
   int lines = KHDISPATCH_WORKSPACE_SIZE / line_size;
   int offset = 0;
   int height = image->height;

   if (khrn_image_is_brcm1(image->format))
      lines &= ~63;

   assert(lines > 0);

   while (height > 0) {
      int batch = _min(lines, height);
      uint32_t len = batch * line_size;

      CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
      int adjusted_offset = (image->stride < 0) ? (offset + (batch - 1)) : offset;

      RPC_CALL8_IN_BULK(eglIntImageSetColorData_impl,
         thread,
         EGLINTIMAGESETCOLORDATA_ID,
         RPC_EGLID(egl_image),
         RPC_UINT(image->format),
         RPC_UINT(0),
         RPC_INT(offset),
         RPC_UINT(image->width),
         RPC_INT(batch),
         RPC_UINT(image->stride),
         (const char *)image->storage + adjusted_offset * image->stride,
         len);

      offset += batch;
      height -= batch;
   }
}

static void send_bound_pixmap(int i)
{
	ALOGI("%s",__FUNCTION__);
   KHRN_IMAGE_WRAP_T image;

   vcos_log_trace("send_bound_pixmap %d %d", i, (int)pixmap_binding[i].egl_image);

   vcos_assert(i >= 0 && i < NUM_PIXMAP_BINDINGS);
   vcos_assert(pixmap_binding[i].used);

   platform_get_pixmap_info(pixmap_binding[i].pixmap, &image);
   set_egl_image_color_data(pixmap_binding[i].egl_image, &image);
   khrn_platform_release_pixmap_info(pixmap_binding[i].pixmap, &image);
}

static void send_bound_pixmaps(void)
{
	//ALOGI("%s",__FUNCTION__);
   int i;
   for (i = 0; i < NUM_PIXMAP_BINDINGS; i++)
   {
      if (pixmap_binding[i].used && pixmap_binding[i].send)
      {
         send_bound_pixmap(i);
      }
   }
}

void khrn_platform_bind_pixmap_to_egl_image(EGLNativePixmapType pixmap, EGLImageKHR egl_image, bool send)
{
	ALOGI("%s",__FUNCTION__);
   int i;
   for (i = 0; i < NUM_PIXMAP_BINDINGS; i++)
   {
      if (!pixmap_binding[i].used)
      {

         vcos_log_trace("khrn_platform_bind_pixmap_to_egl_image %d", i);

         pixmap_binding[i].used = true;
         pixmap_binding[i].pixmap = pixmap;
         pixmap_binding[i].egl_image = egl_image;
         pixmap_binding[i].send = send;
         if(send)
            send_bound_pixmap(i);
         return;
      }
   }
   vcos_assert(0);  /* Not enough NUM_PIXMAP_BINDINGS? */
}

void khrn_platform_unbind_pixmap_from_egl_image(EGLImageKHR egl_image)
{
	 ALOGI("%s",__FUNCTION__);
   int i;
   for (i = 0; i < NUM_PIXMAP_BINDINGS; i++)
   {
      if (pixmap_binding[i].used && pixmap_binding[i].egl_image == egl_image)
      {
         pixmap_binding[i].used = false;
      }
   }
}





uint32_t platform_get_handle(EGLDisplay dpy,EGLNativeWindowType window)
{
 
		errno = 0 ;
		// Do we have a window?
		if(window == NULL){
			errno = EINVAL;
			ALOGE("%s Error Window %p %d : %s ",__FUNCTION__,window,errno,strerror(errno));
			return NULL;	
		}	
		void *window_ptr = window;
		uint32_t* window_magic_ptr = window_ptr;
		uint32_t window_magic = (*window_magic_ptr);
		uint32_t return_value = 0;
		if(window_magic != ANDROID_NATIVE_WINDOW_MAGIC){
			ALOGI("%s win=%p is not a ANativeWindow 0x%x\n",__FUNCTION__,window , window_magic);
			ALOGI("%s Trying EGL_DISPMANX_WINDOW_T \n",__FUNCTION__);
			EGL_DISPMANX_WINDOW_T* native_window = window_ptr;
			return_value = (uint32_t) android_get_dispmanx_element();
		}else if(window_magic == ANDROID_NATIVE_WINDOW_MAGIC){
			ALOGI("%s ANativeWindow Found win=%p 0x%x\n",__FUNCTION__,window , window_magic);
			return_value = (uint32_t) android_get_dispmanx_element();
			//return_value= (uint32_t)window;
		}
		return return_value;
}

static void android_native_window_get_dimensions(ANativeWindow *native_window,int32_t *width, int32_t *height){
		  

	
	// Query ALL THE THINGS!
	native_window->query(native_window, NATIVE_WINDOW_WIDTH, width);
	ALOGD("%s NATIVE_WINDOW_WIDTH = %d ",__FUNCTION__,(*width));
	
	native_window->query(native_window, NATIVE_WINDOW_HEIGHT, height);
	ALOGD("%s NATIVE_WINDOW_HEIGHT = %d ",__FUNCTION__,(*height));

	int minbuffers = 0 ;
	native_window->query(native_window, NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &minbuffers);
	ALOGD("%s  NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS = %d ",__FUNCTION__,minbuffers);
	
	int composer_queues = 0 ;
	native_window->query(native_window,  NATIVE_WINDOW_QUEUES_TO_WINDOW_COMPOSER, &composer_queues);
	ALOGD("%s   NATIVE_WINDOW_QUEUES_TO_WINDOW_COMPOSER = %d ",__FUNCTION__,composer_queues);
	
	
	ALOGD("%s native_window->common=%p\n",__FUNCTION__,native_window->common);
	if(native_window->common.magic == NULL){
		ALOGE("%s native_window->common.magic not set\n",__FUNCTION__);
	}else{
		ALOGD("%s native_window->common.magic=%p\n",__FUNCTION__,native_window->common.magic);
	}
	
	 
}

void platform_get_dimensions(EGLDisplay display, EGLNativeWindowType window,
      int32_t *width, int32_t *height, uint32_t *swapchain_count)
{
	errno = 0;
	
	// Do some sanity checks	
	
	// Make sure we have a valid pointer to store
	// the width in
	if(width == NULL){
		errno = EINVAL;
		ALOGE("%s Error Width %d : %s ",__FUNCTION__,errno,strerror(errno));
		return ;
	}
	// set width to zero now we know the
	// pointer is valid
	(*width) = 0;
	
	// Make sure we have a valid pointer to store the height in
	if(height == NULL){
		errno = EINVAL;
		ALOGE("%s Error Height %d : %s ",__FUNCTION__,errno,strerror(errno));
		return ;
	}
	
	// set height to zero now we know the
	// pointer is valid
	(*height) = 0;

	// Make sure we have a valid pointer to store the height in
	if(swapchain_count == NULL){
		errno = EINVAL;
		ALOGE("%s Error swapchain_count %d : %s ",__FUNCTION__,errno,strerror(errno));
		return ;
	}
	
	// set swapchain_count to zero now we know the
	// pointer is valid
	(*swapchain_count) = 0;

	
	// Do we have a window?
	if(window == NULL){
		errno = EINVAL;
		ALOGE("%s Error Window %p %d : %s ",__FUNCTION__,window,errno,strerror(errno));
		return ;	
	}	
	
	// We need to figure out what the concrete type backing
	// the EGLNativeWindow is. ANDROID_NATIVE_WINDOW_MAGIC is
	// "_wnd" [ 0x5f776e64 ] which fits nicely into a 4 byte integer
	// Extra void* variable to avoid unneccessary casting drama!
	// it compiler will abstract it away so there's no extra overhead.
	void *window_ptr = window;
	uint32_t* window_magic_ptr = window_ptr;
	uint32_t window_magic = (*window_magic_ptr);
	if(window_magic != ANDROID_NATIVE_WINDOW_MAGIC){
		ALOGI("%s win=%p is not a ANativeWindow 0x%x\n",__FUNCTION__,window , window_magic);
		ALOGI("%s Trying EGL_DISPMANX_WINDOW_T \n",__FUNCTION__);
		
		EGL_DISPMANX_WINDOW_T* native_window = android_get_dispmanx();
		(*height) = native_window->height;
		(*width) = native_window->width;
		
		
	}else if(window_magic == ANDROID_NATIVE_WINDOW_MAGIC){
				  	
		ALOGD("%s ANDROID_NATIVE_WINDOW_MAGIC FOUND window=%p 0x%x\n",__FUNCTION__,window,window_magic);
		ANativeWindow *native_window = window;
		android_native_window_get_dimensions(native_window,height,width);
	}
	ALOGD("%s returning height=%d width=%d",__FUNCTION__,(*height),(*width));
	return ;
	
	
}


uint32_t platform_get_color_format ( uint32_t format ) { return format; }
void platform_dequeue(EGLDisplay dpy, EGLNativeWindowType window){
}
	//ALOGD("%s dpy=%p win=%p ",__FUN}
