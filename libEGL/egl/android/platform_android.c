#define LOG_TAG "platform_android"
#define LOG_NDEBUG 0
#include <utils/Log.h>
#include <hardware/hardware.h>
#include <hardware/gralloc.h>
#include <gralloc/gralloc_dispmanx.h>

int android_open_device(void)
{
	const hw_module_t *mod;
	int fd = -1, err;
	ALOGV("%s",__FUNCTION__);
	err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &mod);
	if (!err) {
	  const gralloc_module_t *gr = (gralloc_module_t *) mod;

	  err = -EINVAL;
	  if (gr->perform)
		 err = gr->perform(gr, GRALLOC_MODULE_PERFORM_CREATE_DISPMANX_WINDOW_HANDLE);
	}
	if (err) {
	  ALOGW("%s fail to create dispmanx handle",__FUNCTION__);

	}

	return err;
}
EGL_DISPMANX_WINDOW_T* android_get_dispmanx(void)
{
	const hw_module_t *mod;
	EGL_DISPMANX_WINDOW_T* win = -1; int err;
	ALOGV("%s",__FUNCTION__);
	
	err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &mod);
	if (!err) {
	  const gralloc_module_t *gr = (gralloc_module_t *) mod;

	  err = -EINVAL;
	  if (gr->perform)
		 err = gr->perform(gr, GRALLOC_MODULE_PERFORM_GET_DISPMANX_WINDOW_HANDLE); //
		 //,&win);
	}
	ALOGV("%s 0x%x",__FUNCTION__,err);
	//if (err) {
	  //ALOGW("%s fail to get dispmanx handle",__FUNCTION__);
	//  win = -1;
	//}

	return (EGL_DISPMANX_WINDOW_T*)err ;
}
uint32_t android_get_dispmanx_element(void)
{
	const hw_module_t *mod;
	uint32_t win = -1; uint32_t err;
	ALOGV("%s",__FUNCTION__);
	
	err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &mod);
	if (!err) {
	  const gralloc_module_t *gr = (gralloc_module_t *) mod;

	  err = -EINVAL;
	  if (gr->perform)
		 err = gr->perform(gr, GRALLOC_MODULE_PERFORM_GET_DISPMANX_ELEMENT); //
		 //,&win);
	}
	ALOGV("%s 0x%x",__FUNCTION__,err);
	//if (err) {
	  //ALOGW("%s fail to get dispmanx handle",__FUNCTION__);
	//  win = -1;
	//}

	return (uint32_t)err ;
}
