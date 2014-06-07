
#PRODUCT_VENDOR_KERNEL_HEADERS := hardware/broadcom/kernel-headers
TARGET_DEBUG_GRALLOC := true

# hw composer HAL
PRODUCT_PACKAGES += \
	hwcomposer.bcm2835
	
# libconvertformat
PRODUCT_PACKAGES += \
	gralloc.bcm2835

