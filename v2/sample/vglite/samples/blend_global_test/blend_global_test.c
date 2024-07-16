/*
 Resolution: 128 x 128
 Format: VG_LITE_RGBA8888
 Alpha Blending: None
 Related APIs: vg_lite_clear/vg_lite_draw/vg_lite_blit/vg_lite_blit_rect
 Description: Test the blend feature of gc265, mainly including api such as vg_lite_blit, vg_lite_blit_rect, vg_lite_draw.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

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
        printf("[%s: %d] failed.error type is %s\n", __func__, __LINE__,error_type[error]);\
        goto ErrorHandler; \
    }

static int   fb_width = 128, fb_height = 128;
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer1, src_buffer, image;
static vg_lite_buffer_t* fb1;
static vg_lite_path_t path;
static vg_lite_blend_t blend_mode[16] = {   /* DD is D * Da (global_dest_alpha) */
        VG_LITE_BLEND_NONE,                 /*! S, i.e. no blending. */
        VG_LITE_BLEND_SRC_OVER,             /*! S + (1 - Sa) * DD */
        VG_LITE_BLEND_DST_OVER,             /*! (1 - Da) * S + DD */
        VG_LITE_BLEND_SRC_IN,               /*! Da * S */
        VG_LITE_BLEND_DST_IN,               /*! Sa * DD */
        VG_LITE_BLEND_MULTIPLY,             /*! S * (1 - Da) + DD * (1 - Sa) + S * DD * Da */
        VG_LITE_BLEND_SCREEN,               /*! S + D - S * DD */
        VG_LITE_BLEND_DARKEN,               /*! min(SrcOver, DstOver) */
        VG_LITE_BLEND_LIGHTEN,              /*! max(SrcOver, DstOver) */
        VG_LITE_BLEND_ADDITIVE,             /*! S + DD */
        VG_LITE_BLEND_SUBTRACT,             /*! DD * (1 - Sa) */
        VG_LITE_BLEND_SUBTRACT_LVGL,        /*! DD - S */
        VG_LITE_BLEND_NORMAL_LVGL,          /*! S * Sa + (1 - Sa) * DD  */
        VG_LITE_BLEND_ADDITIVE_LVGL,        /*! (S + DD) * Sa + DD * (1 - Sa) */
        VG_LITE_BLEND_MULTIPLY_LVGL,        /*! (S * DD) * Sa + DD * (1 - Sa) */
        VG_LITE_BLEND_PREMULTIPLY_SRC_OVER  /*! S * Sa + (1 - Sa) * DD */
};

static vg_lite_string blend_name[16] = {
        "NONE",
        "SRC_OVER",
        "DST_OVER",
        "SRC_IN",
        "DST_IN",
        "MULTIPLY",
        "SCREEN",
        "DARKEN",
        "LIGHTEN",
        "ADDITIVE",
        "SUBTRACT",
        "SUBTRACT_LVGL",
        "NORMAL_LVGL",
        "ADDITIVE_LVGL",
        "MULTIPLY_LVGL",
        "PREMULTIPLY_SRC_OVER"
};

static uint8_t sides_cmd[] = {
    VLC_OP_MOVE,
    VLC_OP_LINE,
    VLC_OP_LINE,
    VLC_OP_LINE,
    VLC_OP_END
};

float sides_data_left[] = {
    0, 0,
    110, 0,
    110, 110,
    0, 110,
};

void cleanup(void)
{
    if (src_buffer.handle != NULL) {
        vg_lite_free(&src_buffer);
    }
    if (fb1->handle != NULL) {
        /* Free the raw memory. */
        vg_lite_free(fb1);
    }
    vg_lite_close();
}

int main(int argc, const char* argv[])
{
    uint32_t feature_check = 0;
    vg_lite_matrix_t matrix, matPath;
    vg_lite_filter_t filter;
    char fname[64];
    int i;
    vg_lite_rectangle_t rect = { 0, 0, 100, 100 };
    uint32_t data_size;

    vg_lite_error_t error = vg_lite_init(128, 128);

    if (error) {
        printf("vg_lite engine init failed: vg_lite_init() returned error %d\n", error);
        cleanup();
        return -1;
    }
    filter = VG_LITE_FILTER_POINT;

    buffer1.width = fb_width;
    buffer1.height = fb_height;
    buffer1.format = VG_LITE_RGBA8888;
    error = vg_lite_allocate(&buffer1);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }
    fb1 = &buffer1;

    src_buffer.width = fb_width;
    src_buffer.height = fb_height;
    src_buffer.format = VG_LITE_RGBA8888;
    error = vg_lite_allocate(&src_buffer);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }

    image.width = 100;
    image.height = 100;
    image.format = VG_LITE_RGBA8888;
    error = vg_lite_allocate(&image);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }

    data_size = vg_lite_get_path_length(sides_cmd, sizeof(sides_cmd), VG_LITE_FP32);
    vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0);
    CHECK_ERROR(vg_lite_append_path(&path, sides_cmd, sides_data_left, sizeof(sides_cmd)));

    CHECK_ERROR(vg_lite_clear(&src_buffer, NULL, 0XFF808080));
    CHECK_ERROR(vg_lite_clear(&image, NULL, 0XFF808080));

    CHECK_ERROR(vg_lite_source_global_alpha(VG_LITE_GLOBAL, 0x80));
    CHECK_ERROR(vg_lite_dest_global_alpha(VG_LITE_GLOBAL, 0xA8));

    for (i = 0; i < sizeof(blend_mode) / sizeof(blend_mode[0]); i++)
    {
        CHECK_ERROR(vg_lite_clear(fb1, NULL, 0XFFBBBBBB));
        vg_lite_identity(&matrix);

        CHECK_ERROR(vg_lite_finish());
        vg_lite_save_png("fb1_image.png", fb1);
        vg_lite_save_png("src_buffer.png", &src_buffer);

        CHECK_ERROR(vg_lite_blit(fb1, &src_buffer, &matrix, blend_mode[i], 0, filter));
        CHECK_ERROR(vg_lite_finish());
        sprintf(fname, "blend_global_%s_1.png", blend_name[i]);
        vg_lite_save_png(fname, fb1);

        CHECK_ERROR(vg_lite_clear(fb1, NULL, 0XFFBBBBBB));
        CHECK_ERROR(vg_lite_blit_rect(fb1, &src_buffer, &rect, &matrix, blend_mode[i], 0, filter));
        CHECK_ERROR(vg_lite_finish());
        sprintf(fname, "blend_global_%s_2.png", blend_name[i]);
        vg_lite_save_png(fname, fb1);

        CHECK_ERROR(vg_lite_clear(fb1, NULL, 0XFFBBBBBB));
        CHECK_ERROR(vg_lite_draw(fb1, &path, VG_LITE_FILL_EVEN_ODD, &matrix, blend_mode[i], 0X80808080));
        CHECK_ERROR(vg_lite_finish());
        sprintf(fname, "blend_global_%s_3.png", blend_name[i]);
        vg_lite_save_png(fname, fb1);

        vg_lite_identity(&matPath);
        vg_lite_translate(5, 5, &matrix);
        CHECK_ERROR(vg_lite_clear(fb1, NULL, 0XFFBBBBBB));
        CHECK_ERROR(vg_lite_draw_pattern(fb1, &path, VG_LITE_FILL_EVEN_ODD, &matPath, &image, &matrix, blend_mode[i], VG_LITE_PATTERN_COLOR, 0XFF800000, 0, filter));
        CHECK_ERROR(vg_lite_finish());
        sprintf(fname, "blend_global_%s_4.png", blend_name[i]);
        vg_lite_save_png(fname, fb1);
    }

    CHECK_ERROR(vg_lite_clear(&src_buffer, NULL, 0XFF808080));
    CHECK_ERROR(vg_lite_clear(&image, NULL, 0XFF808080));

    CHECK_ERROR(vg_lite_source_global_alpha(VG_LITE_SCALED, 0x80));
    CHECK_ERROR(vg_lite_dest_global_alpha(VG_LITE_SCALED, 0xA8));

    for (i = 0; i < sizeof(blend_mode) / sizeof(blend_mode[0]); i++)
    {
        CHECK_ERROR(vg_lite_clear(fb1, NULL, 0XFFBBBBBB));
        vg_lite_identity(&matrix);

        CHECK_ERROR(vg_lite_finish());
        vg_lite_save_png("fb1_image.png", fb1);
        vg_lite_save_png("src_buffer.png", &src_buffer);

        CHECK_ERROR(vg_lite_blit(fb1, &src_buffer, &matrix, blend_mode[i], 0, filter));
        CHECK_ERROR(vg_lite_finish());
        sprintf(fname, "blend_scale_%s_1.png", blend_name[i]);
        vg_lite_save_png(fname, fb1);

        CHECK_ERROR(vg_lite_clear(fb1, NULL, 0XFFBBBBBB));
        CHECK_ERROR(vg_lite_blit_rect(fb1, &src_buffer, &rect, &matrix, blend_mode[i], 0, filter));
        CHECK_ERROR(vg_lite_finish());
        sprintf(fname, "blend_scale_%s_2.png", blend_name[i]);
        vg_lite_save_png(fname, fb1);

        CHECK_ERROR(vg_lite_clear(fb1, NULL, 0XFFBBBBBB));
        CHECK_ERROR(vg_lite_draw(fb1, &path, VG_LITE_FILL_EVEN_ODD, &matrix, blend_mode[i], 0X80808080));
        CHECK_ERROR(vg_lite_finish());
        sprintf(fname, "blend_scale_%s_3.png", blend_name[i]);
        vg_lite_save_png(fname, fb1);

        vg_lite_identity(&matPath);
        vg_lite_translate(5, 5, &matrix);
        CHECK_ERROR(vg_lite_clear(fb1, NULL, 0XFFBBBBBB));
        CHECK_ERROR(vg_lite_draw_pattern(fb1, &path, VG_LITE_FILL_EVEN_ODD, &matPath, &image, &matrix, blend_mode[i], VG_LITE_PATTERN_COLOR, 0XFF800000, 0, filter));
        CHECK_ERROR(vg_lite_finish());
        sprintf(fname, "blend_scale_%s_4.png", blend_name[i]);
        vg_lite_save_png(fname, fb1);
    }

ErrorHandler:
    cleanup();
    return 0;
}