/*
 Resolution: 2 x 2
 Format: VG_LITE_RGBA8888
 Transformation: None
 Alpha Blending: None
 Related APIs: vg_lite_blit
 Description: Read 4 pixels data into srcbuf, then blit to dstbuf to get a 2x2 picture. 
 It shows that dstbuf's width does not need to be aligned to 16 pixels.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DEFAULT_SIZE   320.0f;
#define __func__ __FUNCTION__
char* error_type[] =
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

static int   fb_width = 2, fb_height = 2;
static vg_lite_buffer_t srcbuf;
static vg_lite_buffer_t dstbuf;

int main(int argc, const char* argv[])
{
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    uint32_t feature_check = 0;

    /* Initialize vg_lite engine. */
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    const unsigned int color_data[4] = { 0xff000050, 0xff005000, 0xff500000, 0xff505050 };
    vg_lite_identity(&matrix);
    filter = VG_LITE_FILTER_POINT;

    srcbuf.width = fb_width;
    srcbuf.height = fb_height;
    srcbuf.format = VG_LITE_RGBA8888;
    if (vg_lite_query_feature(gcFEATURE_BIT_VG_16PIXELS_ALIGN))
        srcbuf.width = ((srcbuf.width + 15) & ~0xf);
    CHECK_ERROR(vg_lite_allocate(&srcbuf));
    memset(srcbuf.memory, 0xffffffff, srcbuf.stride * srcbuf.height);
    memcpy(srcbuf.memory, &color_data[0], 8);
    memcpy(((char*)srcbuf.memory + srcbuf.stride), &color_data[2], 8);

    dstbuf.width = fb_width;
    dstbuf.height = fb_height;
    dstbuf.format = VG_LITE_RGBA8888;
    if (vg_lite_query_feature(gcFEATURE_BIT_VG_16PIXELS_ALIGN))
        dstbuf.width = ((dstbuf.width + 15) & ~0xf);
    CHECK_ERROR(vg_lite_allocate(&dstbuf));

    CHECK_ERROR(vg_lite_blit(&dstbuf, &srcbuf, &matrix, VG_LITE_BLEND_NONE, 0, filter));
    CHECK_ERROR(vg_lite_finish());
    vg_lite_save_png("16pixels_align.png", &dstbuf);

ErrorHandler:
    vg_lite_free(&srcbuf);
    vg_lite_free(&dstbuf);
    return 0;
}
