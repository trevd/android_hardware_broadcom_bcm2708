#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "Dispmanx_Gralloc"

#ifdef LOG_NDEBUG
#undef LOG_NDEBUG
#endif
#define LOG_NDEBUG 0
#include <sys/ioctl.h>
#include <utils/Log.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dispmanx.h>
#include <interface/khronos/include/GLES2/gl2.h>
#include <interface/khronos/common/khrn_int_image.h>


#define DEVICE_FILE_NAME "/dev/vcio"
#define VCMSG_MEMORY_ALLOC 0x3000c
#define VCMSG_MEMORY_LOCK 0x3000d
#define VCMSG_MEMORY_UNLOCK 0x3000e
#define VCMSG_MEMORY_FREE 0x3000f
#define VCMSG_GET_DISPMANX_HANDLE 0x30014
#define MAJOR_NUM 100
#define IOCTL_MBOX_PROPERTY _IOWR(MAJOR_NUM, 0, char *)

// Flags for allocate memory.
enum {
MEM_FLAG_DISCARDABLE = 1 << 0, /* can be resized to 0 at any time. Use for cached data */
MEM_FLAG_NORMAL = 0 << 2, /* normal allocating alias. Don't use from ARM */
MEM_FLAG_DIRECT = 1 << 2, /* 0xC alias uncached */
MEM_FLAG_COHERENT = 2 << 2, /* 0x8 alias. Non-allocating in L2 but coherent */
MEM_FLAG_L1_NONALLOCATING = (MEM_FLAG_DIRECT | MEM_FLAG_COHERENT), /* Allocating in L2 */
MEM_FLAG_ZERO = 1 << 4,  /* initialise buffer to all zeros */
MEM_FLAG_NO_INIT = 1 << 5, /* don't initialise (default is initialise to all ones */
MEM_FLAG_HINT_PERMALOCK = 1 << 6, /* Likely to be locked for long periods of time. */
};
static int mailbox_perform(void *buf)
{
    ALOGV("\nint %s:%d ", __FUNCTION__,__LINE__);
    ALOGV("\t void *buf=%p ",buf);
    int fd =  open(DEVICE_FILE_NAME, 0);
    if (fd < 0) {
	ALOGE("Can't open device file: %s [ %d %s ]\n", DEVICE_FILE_NAME,errno,strerror(errno));
	return -1 ; 
    }
    int ioctl_result = ioctl(fd, IOCTL_MBOX_PROPERTY, buf);
    if(ioctl_result < 0){
	ALOGE("IOCTL_MBOX_PROPERTY failed: [ %d %s ]\n", errno,strerror(errno));
    }
    // we should have a valid open fd here but it's always worth a double check
    // it's pretty vital that we successfully close the device file otherwise
    // other applications will be locked out
    int close_result = fd ? close(fd) : -2;
    switch(close_result){
	case -1:
	    ALOGE("Unable to close device file: %s : [ %d %s ]\n", DEVICE_FILE_NAME,errno,strerror(errno));
	    break;
	case -2:
	    ALOGE("If you're seeing this something is very wrong!! [ %d %s ]\n", errno,strerror(errno));
	    break;
	default:
	    ALOGI("device file: %s closed successfully",DEVICE_FILE_NAME);
    }
    return ioctl_result;

}

unsigned int mailbox_get_dispmanx_handle(DISPMANX_RESOURCE_HANDLE_T resource) {
        ALOGV("\nint %s:%d ", __FUNCTION__,__LINE__);
	ALOGV("\t DISPMANX_RESOURCE_HANDLE_T resource=%p ",resource);
	int i=0;
	unsigned p[32];
	p[i++] = 0; // size
	p[i++] = 0x00000000; // process request
	
	p[i++] = VCMSG_GET_DISPMANX_HANDLE; // (the tag id)
	p[i++] = 8; // (size of the buffer)
	p[i++] = 4; // (size of the data)
	p[i++] = resource;
	p[i++] = 0; // filler
	
	p[i++] = 0x00000000; // end tag
	p[0] = i*sizeof *p; // actual size
	
	mailbox_perform( p);
	ALOGD("success %d, handle %x\n",p[5],p[6]);
	return p[6];
}

unsigned int mailbox_memory_lock(unsigned int memhandle)
{
           ALOGV("\nint %s:%d ", __FUNCTION__,__LINE__);
	ALOGV("\t unsigned int memhandle=0x%x ",memhandle);
   int i=0;
   unsigned int p[32];
   p[i++] = 0; // size
   p[i++] = 0x00000000; // process request

   p[i++] = VCMSG_MEMORY_LOCK; // (the tag id)
   p[i++] = 4; // (size of the buffer)
   p[i++] = 4; // (size of the data)
   p[i++] = memhandle;

   p[i++] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size

   mailbox_perform( p);
   return p[5];
}

unsigned int mailbox_memory_unlock(unsigned int memhandle)
{
   ALOGV("\nint %s:%d ", __FUNCTION__,__LINE__);
	ALOGV("\t unsigned int memhandle=0x%x ",memhandle);
   int i=0;
   unsigned int p[32];
   p[0] = 0; // size
   p[1] = 0x00000000; // process request

   p[2] = VCMSG_MEMORY_UNLOCK; // (the tag id)
   p[3] = 4; // (size of the buffer)
   p[4] = 4; // (size of the data)
   p[5] = memhandle;

   p[6] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size

   mailbox_perform( p);
   return p[5];
}
unsigned int mailbox_memory_free(unsigned int handle)
{
   int i=0;
   unsigned p[32];
   p[i++] = 0; // size
   p[i++] = 0x00000000; // process request

   p[i++] = VCMSG_MEMORY_FREE; // (the tag id)
   p[i++] = 4; // (size of the buffer)
   p[i++] = 4; // (size of the data)
   p[i++] = handle;

   p[i++] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size

   mailbox_perform(p);
   return p[5];
}


unsigned int mailbox_memory_alloc(unsigned int size)
{
    ALOGV("\nint %s:%d ", __FUNCTION__,__LINE__);
	ALOGV("\t unsigned int size=0x%x ",size);
   int i=0;
   int flags = MEM_FLAG_COHERENT | MEM_FLAG_ZERO | MEM_FLAG_NORMAL ;
   int align = 0x1000;
   unsigned int p[32];
   p[i++] = 0; // size
   p[i++] = 0x00000000; // process request

   p[i++] = VCMSG_MEMORY_ALLOC; // (the tag id)
   p[i++] = 12; // (size of the buffer)
   p[i++] = 12; // (size of the data)
   p[i++] = size; // (num bytes? or pages?)
   p[i++] = align; // (alignment)
   p[i++] = flags; // (MEM_FLAG_L1_NONALLOCATING)

   p[i++] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size

   mailbox_perform(p);
   return p[5];
}
EGL_DISPMANX_WINDOW_T* get_dispmanx_window_element(){
    	
    //ALOGV("\nint %s:%d ", __FUNCTION__,__LINE__);
    
    static EGL_DISPMANX_WINDOW_T nativewindow ;
    static int count = 0 ; 
    if(nativewindow.element != NULL ) {
	if(count==0)
	    ALOGD("%s nativewindow.element=%p",__FUNCTION__,nativewindow.element);
	
	count++;
	return &nativewindow ; 
    }
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
       
    bcm_host_init();
    uint32_t display_width;
    uint32_t display_height;
    graphics_get_display_size(0 /* LCD */, &display_width, &display_height);
    nativewindow.width = display_width;
    nativewindow.height = display_height;
    dispman_display = vc_dispmanx_display_open( 0 /* LCD */);

    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;
    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.width = nativewindow.width;
    dst_rect.height = nativewindow.height;
      
    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = nativewindow.width << 16;
    src_rect.height = nativewindow.height << 16;   
    VC_IMAGE_TYPE_T type = VC_IMAGE_RGBA32;
    uint32_t dummy;
   
    //ALOGD("%s handle->resource=%p",__FUNCTION__,dispman_resource);
    dispman_update = vc_dispmanx_update_start( 0 );  
    nativewindow.element = vc_dispmanx_element_add ( dispman_update, dispman_display,
						0/*layer*/, &dst_rect,0,
						&src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, (DISPMANX_TRANSFORM_T)0/*transform*/);
    vc_dispmanx_update_submit_sync( dispman_update );
   // int fd = ashmem_create_region("dispmanx-window", sizeof(EGL_DISPMANX_WINDOW_T));
   // ashmem_pin_region(fd, 0, 0);

    return &nativewindow;
}

EGLImageKHR* get_egl_image(framebuffer_device_t *dev){
    
    EGLDisplay display;
    static EGLImageKHR  egl_image;
  static int count = 0 ; 
  ALOGV("%s:%d ", __FUNCTION__,__LINE__);
    if(egl_image != NULL ) {
	if(count==0)
	    ALOGV("%s egl_image=%p",__FUNCTION__,&egl_image);
	
	count++;
	return &egl_image ; 
    }
  
   
   display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if ( display == EGL_NO_DISPLAY )
   {
	  ALOGE("%s eglGetDisplay==EGL_NO_DISPLAY",__FUNCTION__);
      return EGL_FALSE;
   }
      EGLint majorVersion;
   EGLint minorVersion;
 // Initialize EGL
   if ( !eglInitialize(display, &majorVersion, &minorVersion) )
   {
    ALOGE("%s eglInitialize Failed",__FUNCTION__);
      return EGL_FALSE;
   }
    KHRN_IMAGE_WRAP_T* wrap = (KHRN_IMAGE_WRAP_T*)calloc(1, sizeof(KHRN_IMAGE_WRAP_T));
    
    wrap->storage = NULL;
    wrap->format = ABGR_8888;
    wrap->width  = dev->width;
    wrap->height = dev->height;
    wrap->stride  = dev->stride;
    egl_image = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_IMAGE_WRAP_BRCM, (EGLClientBuffer) wrap, NULL);
     ALOGV("%s egl_image=%p",__FUNCTION__,&egl_image);
     return &egl_image;
   
    
}
    

