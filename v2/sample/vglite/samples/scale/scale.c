#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "vg_lite.h"
#include "vg_lite_util.h"

static char* error_type[] =
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
    do { \
        error = Function; \
        if (IS_ERROR(error)) \
        { \
            printf("return error\n"); \
        } \
    } while (0)

static int fb_width = 500;
static int fb_height = 500;
static vg_lite_buffer_t buffer;
static vg_lite_buffer_t source;

static void cleanup(void)
{
    if (buffer.handle != NULL) {
        /* Free the buffer memory. */
        vg_lite_free(&buffer);
    }

    if (source.handle != NULL) {
        /* Free the image memory. */
        vg_lite_free(&source);
    }
    vg_lite_close();
}

static int blitOperation(vg_lite_buffer_t* src, vg_lite_buffer_t* dst, float scale)
{

    vg_lite_filter_t filter = VG_LITE_FILTER_BI_LINEAR;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_matrix_t matrix;
    vg_lite_buffer_t* src_buf = src;
    vg_lite_buffer_t* dst_buf = dst;
    int s = scale * 100;
    float sw, sh;
    float tx, ty;
    int rotate = 0 % 360;
    char filename[30];
    sprintf(filename, "result_s%d.png", s);

    CHECK_ERROR(vg_lite_clear(dst, NULL, 0xff000000));

    sw = (float)src->width * scale;
    sh = (float)src->height * scale;
    tx = ((float)dst->width - sw) / 2.0f;
    ty = ((float)dst->height - sh) / 2.0f;

    vg_lite_identity(&matrix);
    vg_lite_translate(tx, ty, &matrix);
    vg_lite_scale(scale, scale, &matrix);

    CHECK_ERROR(vg_lite_blit(dst_buf, src_buf, &matrix, VG_LITE_BLEND_SRC_OVER, 0, filter));
    CHECK_ERROR(vg_lite_finish());

    // Save picture file.
    vg_lite_save_png(filename, dst_buf);
    return error;
}


void main()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;

    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    // Allocate the off-screen buffer.
    buffer.width = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGBA8888;
    CHECK_ERROR(vg_lite_allocate(&buffer));

    if (vg_lite_load_raw(&source, "circle.raw") != 0) {
        printf("load raw file error\n");
        cleanup();
        return;
    }

    blitOperation(&source, &buffer, 1.2);
    blitOperation(&source, &buffer, 1.0);
    blitOperation(&source, &buffer, 0.25);
    blitOperation(&source, &buffer, 0.4);
    blitOperation(&source, &buffer, 0.55);
    blitOperation(&source, &buffer, 0.7);
    blitOperation(&source, &buffer, 0.85);
    blitOperation(&source, &buffer, 0.95);

    cleanup();
}