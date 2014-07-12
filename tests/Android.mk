dirs := \
	libkhrn \
	dispmanx \
	triangle \
	teapot \
	ilclient 

include $(call all-named-subdir-makefiles, $(dirs))
