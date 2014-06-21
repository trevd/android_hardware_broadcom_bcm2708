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
#define LOG_TAG "khrn_client_platform_linux"
#include <utils/Log.h>
#include <khronos/common/khrn_client_platform.h>
#include <khronos/common/khrn_client.h>
#include <khronos/common/khrn_client_rpc.h>
#include <khronos/common/khrn_int_ids.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <gralloc/bcm_host.h>

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
   ALOGV("%s name[0]=%d name[1]=%d name[2]=%d count=%d",__FUNCTION__,name[0], name[1], name[2],count);
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
	ALOGV("%s",__FUNCTION__);
   if (!process_attached)
      /* TODO: this isn't thread safe */
   {
      ALOGV("Attaching process");
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
  ALOGV("%s",__FUNCTION__);
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

#ifndef KHRN_PLATFORM_VCOS_NO_MALLOC

/**
   Allocate memory

   @param size Size in bytes of memory block to allocate
   @return pointer to memory block
**/
void *khrn_platform_malloc(size_t size, const char * name)
{
	ALOGV("%s",__FUNCTION__);
   return vcos_malloc(size, name);
}

/**
   Free memory

   @param v Pointer to  memory area to free
**/
void khrn_platform_free(void *v)
{
	ALOGV("%s",__FUNCTION__);
   if (v)
   {
      vcos_free(v);
   }
}

#endif


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

#if EGL_BRCM_global_image && EGL_KHR_image

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

#endif

void platform_client_lock(void)
{
	ALOGV("%s",__FUNCTION__);
   platform_mutex_acquire(&client_mutex);
}

void platform_client_release(void)
{
	ALOGV("%s",__FUNCTION__);
   platform_mutex_release(&client_mutex);
}

void platform_init_rpc(struct CLIENT_THREAD_STATE *state)
{
	ALOGV("%s",__FUNCTION__);
   assert(1);
}

void platform_term_rpc(struct CLIENT_THREAD_STATE *state)
{
	ALOGV("%s",__FUNCTION__);
   assert(1);
}

void platform_maybe_free_process(void)
{
	ALOGV("%s",__FUNCTION__);
   assert(1);
}

void platform_destroy_winhandle(void *a, uint32_t b)
{
	ALOGV("%s",__FUNCTION__);
   assert(1);
}

void platform_surface_update(uint32_t handle)
{
	ALOGI("%s",__FUNCTION__);
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
static int xxx_position = 0;
uint32_t khrn_platform_get_window_position(EGLNativeWindowType win)
{
	ALOGI("%s",__FUNCTION__);
   return xxx_position;
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
	ALOGI("%s",__FUNCTION__);
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

static bool have_default_dwin;
static EGL_DISPMANX_WINDOW_T default_dwin;

static EGL_DISPMANX_WINDOW_T *check_default(EGLNativeWindowType win)
{
   	//ALOGI("%s win=%d",__FUNCTION__,win);
   if (win == 0) {
	   //	ALOGI("%s win=%d",__FUNCTION__,win);
      /*
       * Special identifier indicating the default window. Either use the
       * one we've got or create a new one, which would fill a WVGA screen
       * on display 0
       */
	
      if (!have_default_dwin) {
		  
         DISPMANX_DISPLAY_HANDLE_T display = vc_dispmanx_display_open( 0 );
         DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start( 0 );
         VC_DISPMANX_ALPHA_T alpha = {DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS, 255, 0};
         VC_RECT_T dst_rect;
         VC_RECT_T src_rect;
        uint32_t display_width;
		uint32_t display_height;
         graphics_get_display_size(0 /* LCD */, &display_width, &display_height);
  

     
   dst_rect.x = 0;
   dst_rect.y = 0;
   dst_rect.width = display_width;
   dst_rect.height = display_height;
      
   src_rect.x = 0;
   src_rect.y = 0;
   src_rect.width = display_width << 16;
   src_rect.height = display_height << 16;  

         default_dwin.element = vc_dispmanx_element_add ( update, display,
            0/*layer*/, &dst_rect, 0/*src*/,
            &src_rect, DISPMANX_PROTECTION_NONE, &alpha, 0/*clamp*/, 0/*transform*/);

         default_dwin.width= display_width;
         default_dwin.height =display_height;

         vc_dispmanx_update_submit_sync( update );

         have_default_dwin = true;
      }
      return &default_dwin;
   } else
      return (EGL_DISPMANX_WINDOW_T*)win;
}

uint32_t platform_get_handle(EGLDisplay dpy,EGLNativeWindowType win)
{
		//ALOGI("%s",__FUNCTION__);
   EGL_DISPMANX_WINDOW_T *dwin = check_default(win);
   return dwin->element;  /* same handles used on host and videocore sides */
}


void platform_get_dimensions(EGLDisplay dpy, EGLNativeWindowType win,
      uint32_t *width, uint32_t *height, uint32_t *swapchain_count)
{
		
   EGL_DISPMANX_WINDOW_T *dwin = check_default(win);
	//ALOGI("%s  dwin->width=%d  dwin->height=%d",__FUNCTION__, dwin->width, dwin->height);
   *width =  dwin->width;
   *height = dwin->height;
}


uint32_t platform_get_color_format ( uint32_t format ) { return format; }
void platform_dequeue(EGLDisplay dpy, EGLNativeWindowType window) {}
