#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DEFAULT_SIZE   320.0f;
#define TEST_ALIGMENT  16
#define __func__ __FUNCTION__

static int   fb_width = 320, fb_height = 480;
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * sys_fb;   //system framebuffer object to show the rendering result.
static vg_lite_buffer_t * fb;
static int has_fb = 0;

static vg_lite_buffer_t image;
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

void cleanup(void)
{
    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }

    if (image.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&image);
    }
    vg_lite_close();
}

void create_imgA4(vg_lite_buffer_t *buffer)
{
    uint32_t i;
    uint32_t block = 16;
    uint8_t *p = (uint8_t*)buffer->memory;
    uint8_t values[] = {
    0x00, 0x11, 0x22, 0x33,
    0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb,
    0xcc, 0xdd, 0xee, 0xff
    };

    for (i = 0; i < buffer->height; i++)
    {
        memset(p, values[(i / block) % TEST_ALIGMENT], buffer->stride);
        p += buffer->stride;
    }
}

int main(int argc, const char * argv[])
{
    vg_lite_filter_t filter;
    uint32_t colors[256] = {0};
    vg_lite_matrix_t matrix;

    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_init(32, 32));

    filter = VG_LITE_FILTER_BI_LINEAR;

    image.format = VG_LITE_A4;
    image.width = 256;
    image.height = 256;
    image.image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
    image.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    CHECK_ERROR(vg_lite_allocate(&image));
    memset(image.memory, 0xc0, image.width * image.height/2);
    create_imgA4(&image);

    fb_scale = (float)fb_width / DEFAULT_SIZE;
    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);

    // Allocate the off-screen buffer.
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGBA8888;
    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    // Build a 33 degree matrix from the center of the buffer.
    vg_lite_identity(&matrix);
    vg_lite_translate(fb_width / 2.0f, fb_height / 2.0f, &matrix);
    vg_lite_rotate(33.0f, &matrix);
    vg_lite_translate(fb_width / -2.0f, fb_height / -2.0f, &matrix);
    vg_lite_scale((vg_lite_float_t) fb_width / (vg_lite_float_t) image.width,
                  (vg_lite_float_t) fb_height / (vg_lite_float_t) image.height, &matrix);

    // Clear the buffer with red.
    CHECK_ERROR(vg_lite_clear( &buffer, NULL, 0xFF0000FF));
    // Blit the image using the matrix.
    CHECK_ERROR(vg_lite_blit(fb, &image, &matrix, VG_LITE_BLEND_SRC_OVER, 0xFF00FF00, filter));
    CHECK_ERROR(vg_lite_finish());
    // Save PNG file.
    vg_lite_save_png("imgA4.png", &buffer);

ErrorHandler:
    // Cleanup.
    cleanup();
    return 0;
}