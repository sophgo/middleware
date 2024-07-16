/*
 Resolution: 320 x 480
 Format: VG_LITE_RGB565
 Transformation: Scale/Translate
 Alpha Blending: None
 Related APIs: vg_lite_clear/vg_lite_translate/vg_lite_scale/vg_lite_draw_pattern/vg_lite_blit
 Description: Load outside landscape image, then blit and draw it to blue dest buffer with stencil image mode.
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

static vg_lite_buffer_image_mode_t  image_mode[2] = {  
    VG_LITE_NORMAL_IMAGE_MODE,
    VG_LITE_STENCIL_MODE,
};

static char path_data[] = {
    2, -5, -10, 
    4, 5, -10, 
    4, 10, -5, 
    4, 0, 0,
    4, 10, 5,
    4, 5, 10,
    4, -5, 10,
    4, -10, 5,
    4, -10, -5,
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
    1,
    0,
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
    vg_lite_matrix_t matrix, matPath;
    uint32_t feature_check = 0;
    int i = 0;
    char fname[64];
    /* Initialize vg_lite engine. */
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    feature_check = vg_lite_query_feature(gcFEATURE_BIT_VG_STENCIL);
    if (!feature_check) {
        printf("stencil imaage mode is not supported.\n");
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
    buffer.format = VG_LITE_RGBA8888;
    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    for(i = 0; i < sizeof(image_mode)/sizeof(image_mode[0]); i++) {
        image.image_mode = image_mode[i];
        /* Clear the buffer with blue. */
        CHECK_ERROR(vg_lite_clear(fb, NULL, 0x3366ffff));
        vg_lite_identity(&matrix);

        /* Blit the image using the matrix. */
        CHECK_ERROR(vg_lite_blit(fb, &image, &matrix, VG_LITE_BLEND_NONE, 0x22666600, filter));
        CHECK_ERROR(vg_lite_finish());
        sprintf(fname, "blit_stencil_%d.png", i);
        vg_lite_save_png(fname, fb);

        CHECK_ERROR(vg_lite_clear(fb, NULL, 0x3366ffff));
        /* Build a 33 degree rotation matrix from the center of the buffer. */
        vg_lite_identity(&matrix);
        vg_lite_translate(fb_width / 2.0f, fb_height / 4.0f, &matrix);
        vg_lite_rotate(33.0f, &matrix);
        vg_lite_scale(0.4f, 0.4f, &matrix);
        vg_lite_translate(fb_width / -2.0f, fb_height / -4.0f, &matrix);
        vg_lite_identity(&matPath);
        vg_lite_translate(fb_width / 2.0f, fb_height / 4.0f, &matPath);
        vg_lite_scale(10, 10, &matPath);

        /* Fill the path using an image. */
        CHECK_ERROR(vg_lite_draw_pattern(fb, &path, VG_LITE_FILL_EVEN_ODD, &matPath, &image, &matrix, VG_LITE_BLEND_NONE, VG_LITE_PATTERN_COLOR, 0x22666600, 0xff000000, filter));
        CHECK_ERROR(vg_lite_finish());

        /* Save PNG file. */
        sprintf(fname, "draw_pattern_stencil_%d.png", i);
        vg_lite_save_png(fname, fb);
    }

ErrorHandler:
    /* Cleanup. */
    cleanup();
    return 0;
}
