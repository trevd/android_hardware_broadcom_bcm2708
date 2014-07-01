
# OpenGLES Implementation
PRODUCT_PACKAGES += \
	libvcos \
	libvchiq_arm \
	libvchostif \
	libEGL_bcm2708

# hw composer HAL
PRODUCT_PACKAGES += \
	hwcomposer.bcm2708
	
# Gralloc
PRODUCT_PACKAGES += \
	libgralloc.bcm2708 \
	gralloc.bcm2708

TARGET_HWC_VERSION := 3

TARGET_EGL_PREBUILT := false

TARGET_LOG_VIDEOCORE := true
