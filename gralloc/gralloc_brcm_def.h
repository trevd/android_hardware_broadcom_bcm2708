/*
 * Copyright (C) 2013 Andreas Harter
 *
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GRALLOC_BRCM_DEF_H_
#define GRALLOC_BRCM_DEF_H_
#include <sys/types.h>
#include <system/window.h>
struct ANativeWindowBuffer;
/* Opaque handles */
typedef uint32_t DISPMANX_DISPLAY_HANDLE_T;
typedef uint32_t DISPMANX_UPDATE_HANDLE_T;
typedef uint32_t DISPMANX_ELEMENT_HANDLE_T;
typedef uint32_t DISPMANX_RESOURCE_HANDLE_T;

typedef uint32_t DISPMANX_PROTECTION_T;
/* We have so many rectangle types; let's try to introduce a common one. */
typedef struct tag_VC_RECT_T {
   int32_t x;
   int32_t y;
   int32_t width;
   int32_t height;
} VC_RECT_T;
struct VC_IMAGE_T;
typedef struct VC_IMAGE_T VC_IMAGE_T;

/* Types of image supported. */
/* Please add any new types to the *end* of this list.  Also update
 * case_VC_IMAGE_ANY_xxx macros (below), and the vc_image_type_info table in
 * vc_image/vc_image_helper.c.
 */
typedef enum
{
   VC_IMAGE_MIN = 0, //bounds for error checking

   VC_IMAGE_RGB565 = 1,
   VC_IMAGE_1BPP,
   VC_IMAGE_YUV420,
   VC_IMAGE_48BPP,
   VC_IMAGE_RGB888,
   VC_IMAGE_8BPP,
   VC_IMAGE_4BPP,    // 4bpp palettised image
   VC_IMAGE_3D32,    /* A separated format of 16 colour/light shorts followed by 16 z values */
   VC_IMAGE_3D32B,   /* 16 colours followed by 16 z values */
   VC_IMAGE_3D32MAT, /* A separated format of 16 material/colour/light shorts followed by 16 z values */
   VC_IMAGE_RGB2X9,   /* 32 bit format containing 18 bits of 6.6.6 RGB, 9 bits per short */
   VC_IMAGE_RGB666,   /* 32-bit format holding 18 bits of 6.6.6 RGB */
   VC_IMAGE_PAL4_OBSOLETE,     // 4bpp palettised image with embedded palette
   VC_IMAGE_PAL8_OBSOLETE,     // 8bpp palettised image with embedded palette
   VC_IMAGE_RGBA32,   /* RGB888 with an alpha byte after each pixel */ /* xxx: isn't it BEFORE each pixel? */
   VC_IMAGE_YUV422,   /* a line of Y (32-byte padded), a line of U (16-byte padded), and a line of V (16-byte padded) */
   VC_IMAGE_RGBA565,  /* RGB565 with a transparent patch */
   VC_IMAGE_RGBA16,   /* Compressed (4444) version of RGBA32 */
   VC_IMAGE_YUV_UV,   /* VCIII codec format */
   VC_IMAGE_TF_RGBA32, /* VCIII T-format RGBA8888 */
   VC_IMAGE_TF_RGBX32,  /* VCIII T-format RGBx8888 */
   VC_IMAGE_TF_FLOAT, /* VCIII T-format float */
   VC_IMAGE_TF_RGBA16, /* VCIII T-format RGBA4444 */
   VC_IMAGE_TF_RGBA5551, /* VCIII T-format RGB5551 */
   VC_IMAGE_TF_RGB565, /* VCIII T-format RGB565 */
   VC_IMAGE_TF_YA88, /* VCIII T-format 8-bit luma and 8-bit alpha */
   VC_IMAGE_TF_BYTE, /* VCIII T-format 8 bit generic sample */
   VC_IMAGE_TF_PAL8, /* VCIII T-format 8-bit palette */
   VC_IMAGE_TF_PAL4, /* VCIII T-format 4-bit palette */
   VC_IMAGE_TF_ETC1, /* VCIII T-format Ericsson Texture Compressed */
   VC_IMAGE_BGR888,  /* RGB888 with R & B swapped */
   VC_IMAGE_BGR888_NP,  /* RGB888 with R & B swapped, but with no pitch, i.e. no padding after each row of pixels */
   VC_IMAGE_BAYER,  /* Bayer image, extra defines which variant is being used */
   VC_IMAGE_CODEC,  /* General wrapper for codec images e.g. JPEG from camera */
   VC_IMAGE_YUV_UV32,   /* VCIII codec format */
   VC_IMAGE_TF_Y8,   /* VCIII T-format 8-bit luma */
   VC_IMAGE_TF_A8,   /* VCIII T-format 8-bit alpha */
   VC_IMAGE_TF_SHORT,/* VCIII T-format 16-bit generic sample */
   VC_IMAGE_TF_1BPP, /* VCIII T-format 1bpp black/white */
   VC_IMAGE_OPENGL,
   VC_IMAGE_YUV444I, /* VCIII-B0 HVS YUV 4:4:4 interleaved samples */
   VC_IMAGE_YUV422PLANAR,  /* Y, U, & V planes separately (VC_IMAGE_YUV422 has them interleaved on a per line basis) */
   VC_IMAGE_ARGB8888,   /* 32bpp with 8bit alpha at MS byte, with R, G, B (LS byte) */
   VC_IMAGE_XRGB8888,   /* 32bpp with 8bit unused at MS byte, with R, G, B (LS byte) */

   VC_IMAGE_YUV422YUYV,  /* interleaved 8 bit samples of Y, U, Y, V */
   VC_IMAGE_YUV422YVYU,  /* interleaved 8 bit samples of Y, V, Y, U */
   VC_IMAGE_YUV422UYVY,  /* interleaved 8 bit samples of U, Y, V, Y */
   VC_IMAGE_YUV422VYUY,  /* interleaved 8 bit samples of V, Y, U, Y */

   VC_IMAGE_RGBX32,      /* 32bpp like RGBA32 but with unused alpha */
   VC_IMAGE_RGBX8888,    /* 32bpp, corresponding to RGBA with unused alpha */
   VC_IMAGE_BGRX8888,    /* 32bpp, corresponding to BGRA with unused alpha */

   VC_IMAGE_YUV420SP,    /* Y as a plane, then UV byte interleaved in plane with with same pitch, half height */
   
   VC_IMAGE_YUV444PLANAR,  /* Y, U, & V planes separately 4:4:4 */

   VC_IMAGE_TF_U8,   /* T-format 8-bit U - same as TF_Y8 buf from U plane */
   VC_IMAGE_TF_V8,   /* T-format 8-bit U - same as TF_Y8 buf from V plane */
   
   VC_IMAGE_MAX,     //bounds for error checking
   VC_IMAGE_FORCE_ENUM_16BIT = 0xffff,
} VC_IMAGE_TYPE_T;

#define DISPMANX_PROTECTION_NONE  0
//enums of display input format
typedef enum
{
   VCOS_DISPLAY_INPUT_FORMAT_INVALID = 0,
   VCOS_DISPLAY_INPUT_FORMAT_RGB888,
   VCOS_DISPLAY_INPUT_FORMAT_RGB565
}
VCOS_DISPLAY_INPUT_FORMAT_T;
typedef enum {
  /* Bottom 2 bits sets the alpha mode */
  DISPMANX_FLAGS_ALPHA_FROM_SOURCE = 0,
  DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS = 1,
  DISPMANX_FLAGS_ALPHA_FIXED_NON_ZERO = 2,
  DISPMANX_FLAGS_ALPHA_FIXED_EXCEED_0X07 = 3,

  DISPMANX_FLAGS_ALPHA_PREMULT = 1 << 16,
  DISPMANX_FLAGS_ALPHA_MIX = 1 << 17
} DISPMANX_FLAGS_ALPHA_T;

typedef struct {
  DISPMANX_FLAGS_ALPHA_T flags;
  uint32_t opacity;
  VC_IMAGE_T *mask;
} DISPMANX_ALPHA_T;

typedef struct {
  DISPMANX_FLAGS_ALPHA_T flags;
  uint32_t opacity;
  DISPMANX_RESOURCE_HANDLE_T mask;
} VC_DISPMANX_ALPHA_T;  /* for use with vmcs_host */

typedef enum {
  DISPMANX_FLAGS_CLAMP_NONE = 0,
  DISPMANX_FLAGS_CLAMP_LUMA_TRANSPARENT = 1,
#if __VCCOREVER__ >= 0x04000000
  DISPMANX_FLAGS_CLAMP_TRANSPARENT = 2,
  DISPMANX_FLAGS_CLAMP_REPLACE = 3
#else
  DISPMANX_FLAGS_CLAMP_CHROMA_TRANSPARENT = 2,
  DISPMANX_FLAGS_CLAMP_TRANSPARENT = 3
#endif
} DISPMANX_FLAGS_CLAMP_T;

typedef enum {
  DISPMANX_FLAGS_KEYMASK_OVERRIDE = 1,
  DISPMANX_FLAGS_KEYMASK_SMOOTH = 1 << 1,
  DISPMANX_FLAGS_KEYMASK_CR_INV = 1 << 2,
  DISPMANX_FLAGS_KEYMASK_CB_INV = 1 << 3,
  DISPMANX_FLAGS_KEYMASK_YY_INV = 1 << 4
} DISPMANX_FLAGS_KEYMASK_T;

typedef union {
  struct {
    uint8_t yy_upper;
    uint8_t yy_lower;
    uint8_t cr_upper;
    uint8_t cr_lower;
    uint8_t cb_upper;
    uint8_t cb_lower;
  } yuv;
  struct {
    uint8_t red_upper;
    uint8_t red_lower;
    uint8_t blue_upper;
    uint8_t blue_lower;
    uint8_t green_upper;
    uint8_t green_lower;
  } rgb;
} DISPMANX_CLAMP_KEYS_T;

typedef struct {
  DISPMANX_FLAGS_CLAMP_T mode;
  DISPMANX_FLAGS_KEYMASK_T key_mask;
  DISPMANX_CLAMP_KEYS_T key_value;
  uint32_t replace_value;
} DISPMANX_CLAMP_T;
/** For backward compatibility */
#define DISPLAY_INPUT_FORMAT_INVALID VCOS_DISPLAY_INPUT_FORMAT_INVALID
#define DISPLAY_INPUT_FORMAT_RGB888  VCOS_DISPLAY_INPUT_FORMAT_RGB888
#define DISPLAY_INPUT_FORMAT_RGB565  VCOS_DISPLAY_INPUT_FORMAT_RGB565
typedef VCOS_DISPLAY_INPUT_FORMAT_T DISPLAY_INPUT_FORMAT_T;
typedef enum {
  /* Bottom 2 bits sets the orientation */
  DISPMANX_NO_ROTATE = 0,
  DISPMANX_ROTATE_90 = 1,
  DISPMANX_ROTATE_180 = 2,
  DISPMANX_ROTATE_270 = 3,

  DISPMANX_FLIP_HRIZ = 1 << 16,
  DISPMANX_FLIP_VERT = 1 << 17,

  /* extra flags for controlling snapshot behaviour */
  DISPMANX_SNAPSHOT_NO_YUV = 1 << 24,
  DISPMANX_SNAPSHOT_NO_RGB = 1 << 25,
  DISPMANX_SNAPSHOT_FILL = 1 << 26,
  DISPMANX_SNAPSHOT_SWAP_RED_BLUE = 1 << 27,
  DISPMANX_SNAPSHOT_PACK = 1 << 28
} DISPMANX_TRANSFORM_T;
typedef struct {
  int32_t width;
  int32_t height;
  DISPMANX_TRANSFORM_T transform;
  DISPLAY_INPUT_FORMAT_T input_format;
} DISPMANX_MODEINFO_T;

typedef void *EGLImageKHR;
typedef struct {
   DISPMANX_ELEMENT_HANDLE_T element;
   int width;   /* This is necessary because dispmanx elements are not queriable. */
   int height;
} EGL_DISPMANX_WINDOW_T;


static const int GRALLOC_PRIV_TYPE_MM_RESOURCE = 0;
static const int GRALLOC_PRIV_TYPE_GL_RESOURCE = 1;

static const int GRALLOC_MAGICS_HAL_PIXEL_FORMAT_OPAQUE = 0;

typedef struct gralloc_private_handle_t {

    EGLImageKHR egl_image;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_RESOURCE_HANDLE_T dispman_resource;
    EGL_DISPMANX_WINDOW_T window;
    //GRALLOC_PRIV_TYPE_GL_RESOURCE or GRALLOC_PRIV_TYPE_MM_RESOURCE
    int res_type;
    int gl_format;
    int stride;
    int pixelformat;

} gralloc_private_handle_t;

#endif /* GRALLOC_BRCM_DEF_H_ */
