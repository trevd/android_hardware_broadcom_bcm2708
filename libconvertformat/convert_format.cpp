/*
 * Copyright (C) 2013 Andreas Harter - RazDroid project
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

#include "convert_format.h"

#include "interface/khronos/common/khrn_int_image.h"
#include <ui/PixelFormat.h>

int32_t android_to_videocore(int32_t android_pixelformat)
{
    KHRN_IMAGE_FORMAT_T format;
    switch (android_pixelformat) {
        case android::PIXEL_FORMAT_RGBA_8888:
        case android::PIXEL_FORMAT_BGRA_8888:
            format = ABGR_8888;
            break;
        case android::PIXEL_FORMAT_RGBX_8888:
            format = XBGR_8888;
            break;
        case android::PIXEL_FORMAT_RGB_888:
            format = RGB_888;
            break;
        case android::PIXEL_FORMAT_RGBA_5551:
            format = RGB_565_TF;
            break;
        case android::PIXEL_FORMAT_RGBA_4444:
            format = RGBA_4444;
            break;
        case android::PIXEL_FORMAT_RGB_565:
            format = RGB_565;
            break;
        default:
            return android_pixelformat;
    }
    return format;
}

//ANRDOID
//    PIXEL_FORMAT_RGBA_8888   = HAL_PIXEL_FORMAT_RGBA_8888,  // 4x8-bit RGBA
//    PIXEL_FORMAT_RGBX_8888   = HAL_PIXEL_FORMAT_RGBX_8888,  // 4x8-bit RGB0
//    PIXEL_FORMAT_RGB_888     = HAL_PIXEL_FORMAT_RGB_888,    // 3x8-bit RGB
//    PIXEL_FORMAT_RGB_565     = HAL_PIXEL_FORMAT_RGB_565,    // 16-bit RGB
//    PIXEL_FORMAT_BGRA_8888   = HAL_PIXEL_FORMAT_BGRA_8888,  // 4x8-bit BGRA
//    PIXEL_FORMAT_RGBA_5551   = HAL_PIXEL_FORMAT_RGBA_5551,  // 16-bit ARGB
//    PIXEL_FORMAT_RGBA_4444   = HAL_PIXEL_FORMAT_RGBA_4444,  // 16-bit ARGB
//    PIXEL_FORMAT_A_8         = GGL_PIXEL_FORMAT_A_8,        // 8-bit A
//    PIXEL_FORMAT_L_8         = GGL_PIXEL_FORMAT_L_8,        // 8-bit L (R=G=B=L)
//    PIXEL_FORMAT_LA_88       = GGL_PIXEL_FORMAT_LA_88,      // 16-bit LA
//    PIXEL_FORMAT_RGB_332     = GGL_PIXEL_FORMAT_RGB_332,    // 8-bit RGB

//enum GGLPixelFormat {
//    // these constants need to match those
//    // in graphics/PixelFormat.java, ui/PixelFormat.h, BlitHardware.h
//    GGL_PIXEL_FORMAT_UNKNOWN    =   0,
//    GGL_PIXEL_FORMAT_NONE       =   0,
//
//    GGL_PIXEL_FORMAT_RGBA_8888   =   1,  // 4x8-bit ARGB
//    GGL_PIXEL_FORMAT_RGBX_8888   =   2,  // 3x8-bit RGB stored in 32-bit chunks
//    GGL_PIXEL_FORMAT_RGB_888     =   3,  // 3x8-bit RGB
//    GGL_PIXEL_FORMAT_RGB_565     =   4,  // 16-bit RGB
//    GGL_PIXEL_FORMAT_BGRA_8888   =   5,  // 4x8-bit BGRA
//    GGL_PIXEL_FORMAT_RGBA_5551   =   6,  // 16-bit RGBA
//    GGL_PIXEL_FORMAT_RGBA_4444   =   7,  // 16-bit RGBA
//
//    GGL_PIXEL_FORMAT_A_8         =   8,  // 8-bit A
//    GGL_PIXEL_FORMAT_L_8         =   9,  // 8-bit L (R=G=B = L)
//    GGL_PIXEL_FORMAT_LA_88       = 0xA,  // 16-bit LA
//    GGL_PIXEL_FORMAT_RGB_332     = 0xB,  // 8-bit RGB (non paletted)
//
//};

//VIDEOCORE
int32_t videocore_to_android(int32_t videocore_format)
{
    int32_t format;
    switch (videocore_format) {
        case ABGR_8888:
            format = android::PIXEL_FORMAT_RGBA_8888;
            break;
        case XBGR_8888:
            format = android::PIXEL_FORMAT_RGBX_8888;
            break;
        case RGB_888:
            format = android::PIXEL_FORMAT_RGB_888;
            break;
        case RGB_565_TF:
            format = android::PIXEL_FORMAT_RGB_565;
            break;
        case RGBA_4444:
            format = android::PIXEL_FORMAT_RGBA_4444;
            break;
        case RGB_565:
            format = android::PIXEL_FORMAT_RGB_565;
            break;
        default:
            format = videocore_format;
            break;
    }
    return format;
}
