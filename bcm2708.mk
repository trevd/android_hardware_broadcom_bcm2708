
# OpenGLES Implementation
PRODUCT_PACKAGES += \
	libvcos \
	libvchiq_arm \
	libvchost \
	libEGL_bcm2708

# hw composer HAL
PRODUCT_PACKAGES += \
	hwcomposer.bcm2708
	
# Gralloc
PRODUCT_PACKAGES += \
	libgralloc.bcm2708 \
	gralloc.bcm2708

TARGET_HWC_VERSION := 1_0

TARGET_LOG_VIDEOCORE := true
TARGET_LOG_EGL := true


TARGET_BCM2708_COMPILE_TEST := true
