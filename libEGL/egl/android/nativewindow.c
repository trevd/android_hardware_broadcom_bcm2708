/* ============================================================================
Copyright (c) 2008-2014, Broadcom Corporation
All rights reserved.
Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
============================================================================ */
#include <system/window.h>
#include <errno.h>
extern ANativeWindow* nativeServerWindow;
enum {
    //
    // these constants need to match those
    // in graphics/PixelFormat.java & pixelflinger/format.h
    //
    PIXEL_FORMAT_UNKNOWN    =   0,
    PIXEL_FORMAT_NONE       =   0,

    // logical pixel formats used by the SurfaceFlinger -----------------------
    PIXEL_FORMAT_CUSTOM         = -4,
        // Custom pixel-format described by a PixelFormatInfo structure

    PIXEL_FORMAT_TRANSLUCENT    = -3,
        // System chooses a format that supports translucency (many alpha bits)

    PIXEL_FORMAT_TRANSPARENT    = -2,
        // System chooses a format that supports transparency
        // (at least 1 alpha bit)

    PIXEL_FORMAT_OPAQUE         = -1,
        // System chooses an opaque format (no alpha bits required)

    // real pixel formats supported for rendering -----------------------------

    PIXEL_FORMAT_RGBA_8888   = HAL_PIXEL_FORMAT_RGBA_8888,  // 4x8-bit RGBA
    PIXEL_FORMAT_RGBX_8888   = HAL_PIXEL_FORMAT_RGBX_8888,  // 4x8-bit RGB0
    PIXEL_FORMAT_RGB_888     = HAL_PIXEL_FORMAT_RGB_888,    // 3x8-bit RGB
    PIXEL_FORMAT_RGB_565     = HAL_PIXEL_FORMAT_RGB_565,    // 16-bit RGB
    PIXEL_FORMAT_BGRA_8888   = HAL_PIXEL_FORMAT_BGRA_8888,  // 4x8-bit BGRA
    PIXEL_FORMAT_RGBA_5551   = 6,                           // 16-bit ARGB
    PIXEL_FORMAT_RGBA_4444   = 7,                           // 16-bit ARGB
};

typedef int32_t PixelFormat;

ssize_t bytesPerPixel(PixelFormat format);
ssize_t bitsPerPixel(PixelFormat format);

ANativeWindow* get_android_native_window(void)
{
	return NULL;
}
ssize_t bytesPerPixel(PixelFormat format) {
    switch (format) {
        case PIXEL_FORMAT_RGBA_8888:
        case PIXEL_FORMAT_RGBX_8888:
        case PIXEL_FORMAT_BGRA_8888:
            return 4;
        case PIXEL_FORMAT_RGB_888:
            return 3;
        case PIXEL_FORMAT_RGB_565:
        case PIXEL_FORMAT_RGBA_5551:
        case PIXEL_FORMAT_RGBA_4444:
            return 2;
    }
    return  -EINVAL;
}
int getBytesPerPixel(int format)
{
	return bytesPerPixel((PixelFormat)format);
}
