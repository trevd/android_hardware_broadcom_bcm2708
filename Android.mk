ifeq ($(TARGET_BOARD_PLATFORM),bcm2708)

bcm2708_dirs := \
		prebuilts \
		libEGL \
		hwcomposer \
		tests/triangle
		

include $(call all-named-subdir-makefiles,$(bcm2708_dirs))

endif
