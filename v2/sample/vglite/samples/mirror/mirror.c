/*
 Resolution: 320 x 480
 Format: VG_LITE_RGB565
 Transformation: Scale/Translate
 Alpha Blending: None
 Related APIs: vg_lite_clear/vg_lite_translate/vg_lite_scale/vg_lite_set_mirror/vg_lite_blit
 Description: Load outside landscape image, then blit it to blue dest buffer with vg_lite_set_mirror/scale/translate.
 Image filter type is selected by hardware feature gcFEATURE_BIT_VG_IM_FILTER(ON: VG_LITE_FILTER_BI_LINEAR, OFF: VG_LITE_FILTER_POINT).
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DEFAULT_SIZE   320.0f;
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
static int   fb_width = 320, fb_height = 480;
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;     /*offscreen framebuffer object for rendering. */
static vg_lite_buffer_t * fb;

static vg_lite_buffer_t image;

static char path_data[] = {
    2, 0, 0,
    4, 5, 10,
    4, 10, 0,
    0,
};

static vg_lite_path_t path = {
    {-10, -10,
    10, 10},
    VG_LITE_HIGH,
    VG_LITE_S8,
    {0},
    sizeof(path_data),
    path_data,
    1
};

void cleanup(void)
{
    if (buffer.handle != NULL) {
        /* Free the buffer memory. */
        vg_lite_free(&buffer);
    }

    if (image.handle != NULL) {
        /* Free the image memory. */
        vg_lite_free(&image);
    }
    vg_lite_clear_path(&path);
    vg_lite_close();
}

int main(int argc, const char * argv[])
{
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    uint32_t feature_check = 1;
    /* Initialize vg_lite engine. */
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));
    feature_check = vg_lite_query_feature(gcFEATURE_BIT_VG_MIRROR);
    if (!feature_check) {
        printf("mirror is not supported.\n");
        cleanup();
        return -1;
    }
    /* Set image filter type according to hardware feature. */
    filter = VG_LITE_FILTER_POINT;   

    /* Load the image. */
    if (vg_lite_load_raw(&image, "landscape.raw") != 0) {
        printf("load raw file error\n");
        cleanup();
        return -1;
    }

    fb_scale = (float)fb_width / DEFAULT_SIZE;
    
    /* Allocate the off-screen buffer. */
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGB565;
    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;
    CHECK_ERROR(vg_lite_set_mirror(VG_LITE_ORIENTATION_BOTTOM_TOP));
    /* Clear the buffer with blue. */
    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));

    vg_lite_identity(&matrix);
    /* Blit the image using the matrix. */
    CHECK_ERROR(vg_lite_blit(fb, &image, &matrix, VG_LITE_BLEND_NONE, 0, filter));

    vg_lite_scale(10, 10, &matrix);
    CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF6600FF));

    CHECK_ERROR(vg_lite_finish());
    CHECK_ERROR(vg_lite_set_mirror(VG_LITE_ORIENTATION_TOP_BOTTOM));
    /* Save PNG file. */
    vg_lite_save_png("mirror.png", fb);

ErrorHandler:
    /* Cleanup. */
    cleanup();
    return 0;
}
