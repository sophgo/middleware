/*
 Resolution: 256 x 256
 Format: VG_LITE_RGB565
 Transformation: None
 Alpha Blending: None
 Related APIs: vg_lite_clear
 Description: Clear whole buffer with blue first, then clear a sub-rectangle of the buffer with red.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DEFAULT_SIZE   256.0f;
#define __func__ __FUNCTION__
char *error_type[] = 
{
    "VG_LITE_SUCCESS",
    "VG_LITE_INVALID_ARGUMENT",
    "VG_LITE_OUT_OF_MEMORY",
    "VG_LITE_NO_CONTEXT",      
    "VG_LITE_TIMEOUT",
    "VG_LITE_OUT_OF_RESOURCES",
    "VG_LITE_GENERIC_IO",
    "VG_LITE_NOT_SUPPORT",
};
#define IS_ERROR(status)         (status > 0)
#define CHECK_ERROR(Function) \
    error = Function; \
    if (IS_ERROR(error)) \
    { \
        printf("[%s: %d] error type is %s\n", __func__, __LINE__,error_type[error]);\
        goto ErrorHandler; \
    }

static int   fb_width = 256, fb_height = 256;
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * fb;

void cleanup(void)
{
    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }

    vg_lite_close();
}

int main(int argc, const char * argv[])
{
    vg_lite_rectangle_t rect = { 64, 64, 64, 64 };
    vg_lite_error_t error = VG_LITE_SUCCESS;
    // Initialize the blitter.
    CHECK_ERROR(vg_lite_init(0, 0));

    fb_scale = (float)fb_width / DEFAULT_SIZE;
    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);

    // Allocate the off-screen buffer.
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGB565;

    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    // Clear the buffer with blue.
    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));
    // Clear a sub-rectangle of the buffer with red.
    CHECK_ERROR(vg_lite_clear(fb, &rect, 0xFF0000FF));
    CHECK_ERROR(vg_lite_finish());
    // Save PNG file.
    vg_lite_save_png("clear.png", fb);

ErrorHandler:
    // Cleanup.
    cleanup();
    return 0;
}
