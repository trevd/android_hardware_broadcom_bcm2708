
#include <gralloc_dispmanx_priv.h>
static int dispmanx_mod_open(const struct hw_module_t *mod,
		const char *name, struct hw_device_t **dev)
{
	struct dispmax_module_t *dmod = (struct drm_module_t *) mod;
	int err;

	if (strcmp(name, GRALLOC_HARDWARE_GPU0) == 0)
		err = drm_mod_open_gpu0(dmod, dev);
	else if (strcmp(name, GRALLOC_HARDWARE_FB0) == 0)
		err = drm_mod_open_fb0(dmod, dev);
	else
		err = -EINVAL;

	return err;
}
