LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include $(ANDROID_BUILD_TOP)/hardware/broadcom/$(TARGET_BOARD_PLATFORM)/cflags.mk

LOCAL_C_INCLUDES += hardware/broadcom/$(TARGET_BOARD_PLATFORM)/include

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/egl
LOCAL_MODULE := libEGL_bcm2708
LOCAL_SRC_FILES := egl/egl_client_config.c \
		egl/egl_client_context.c \
		egl/egl_client.c \
		egl/egl_client_get_proc.c \
		egl/egl_client_surface.c \
		ext/egl_brcm_driver_monitor_client.c \
		ext/egl_brcm_perf_monitor_client.c \
		ext/egl_brcm_global_image_client.c \
		ext/egl_brcm_flush_client.c \
		ext/egl_khr_image_client.c \
		ext/egl_khr_sync_client.c \
		ext/gl_oes_egl_image_client.c \
		ext/egl_khr_lock_surface_client.c \
		ext/ext_gl_debug_marker.c \
		ext/egl_openmaxil_client.c \
		ext/gl_oes_draw_texture_client.c \
		ext/gl_oes_query_matrix_client.c \
		ext/gl_oes_framebuffer_object.c \
		ext/gl_oes_map_buffer.c \
		ext/gl_oes_matrix_palette_client.c \
		vg/vg_int_mat3x3.c \
		vg/vg_client.c \
		glxx/glxx_client.c \
		common/khrn_client.c \
		common/khrn_int_image.c \
		common/khrn_int_util.c \
		common/khrn_options.c \
		common/khrn_client_global_image_map.c \
		common/khrn_client_rpc_linux.c \
		common/khrn_client_platform_linux.c \
		common/khrn_client_pointermap.c \
		common/khrn_client_vector.c \
		common/khrn_int_hash.c \
		common/khrn_client_cache.c \
		common/khrn_int_hash_asm.s \
		wf/wfc_client_stream.c \
		wf/wfc_client.c \
		wf/wfc_client_server_api.c \
		wf/wfc_client_ipc.c
		
LOCAL_SHARED_LIBRARIES := libvchiq_arm libvcos libm liblog libvchostif libgralloc.bcm2708 liblog
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

