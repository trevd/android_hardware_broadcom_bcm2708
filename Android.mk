ifeq ($(TARGET_BOARD_PLATFORM),bcm2708)

bcm2708_dirs := \
		gralloc \
		libEGL

include $(call all-named-subdir-makefiles,$(bcm2708_dirs))

endif
