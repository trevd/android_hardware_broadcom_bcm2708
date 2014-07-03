ifeq ($(TARGET_BOARD_PLATFORM),bcm2708)

bcm2708_dirs := \
		gralloc \
		hwcomposer \
		libEGL \
		libgralloc \
		libvchiq_arm \
		libvchostif \
		libvcos

ifeq ($(strip $(TARGET_BCM2708_COMPILE_TEST)),true)
bcm2708_dirs += tests
endif


include $(call all-named-subdir-makefiles,$(bcm2708_dirs))

endif
