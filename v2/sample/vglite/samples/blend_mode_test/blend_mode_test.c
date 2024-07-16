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
static vg_lite_blend_t blend_mode[26] = {
        VG_LITE_BLEND_NONE,                 /*! S, i.e. no blending. */
        VG_LITE_BLEND_SRC_OVER,             /*! S + (1 - Sa) * D */
        VG_LITE_BLEND_DST_OVER,             /*! (1 - Da) * S + D */
        VG_LITE_BLEND_SRC_IN,               /*! Da * S */
        VG_LITE_BLEND_DST_IN,               /*! Sa * D */
        VG_LITE_BLEND_MULTIPLY,             /*! S * (1 - Da) + D * (1 - Sa) + S * D */
        VG_LITE_BLEND_SCREEN,               /*! S + D - S * D */
        VG_LITE_BLEND_DARKEN,               /*! min(SrcOver, DstOver) */
        VG_LITE_BLEND_LIGHTEN,              /*! max(SrcOver, DstOver) */
        VG_LITE_BLEND_ADDITIVE,             /*! S + D */
        VG_LITE_BLEND_SUBTRACT,             /*! D * (1 - Sa) */
        VG_LITE_BLEND_SUBTRACT_LVGL,        /*! D - S */
        VG_LITE_BLEND_NORMAL_LVGL,          /*! S * Sa + (1 - Sa) * D  */
        VG_LITE_BLEND_ADDITIVE_LVGL,        /*! (S + D) * Sa + D * (1 - Sa) */
        VG_LITE_BLEND_MULTIPLY_LVGL,        /*! (S * D) * Sa + D * (1 - Sa) */
        VG_LITE_BLEND_PREMULTIPLY_SRC_OVER,  /*! S * Sa + (1 - Sa) * D */

        OPENVG_BLEND_SRC,   /*! Copy SRC, no blend, Premultiplied */
        OPENVG_BLEND_SRC_OVER,   /*! Porter-Duff SRC_OVER blend, Premultiplied */
        OPENVG_BLEND_DST_OVER,   /*! Porter-Duff DST_OVER blend, Premultiplied */
        OPENVG_BLEND_SRC_IN,   /*! Porter-Duff SRC_IN blend, Premultiplied */
        OPENVG_BLEND_DST_IN,   /*! Porter-Duff DST_IN blend, Premultiplied */
        OPENVG_BLEND_MULTIPLY,   /*! Porter-Duff MULTIPLY blend, Premultiplied */
        OPENVG_BLEND_SCREEN,   /*! Porter-Duff SCREEN blend, Premultiplied */
        OPENVG_BLEND_DARKEN,   /*! Porter-Duff DARKEN blend, Premultiplied */
        OPENVG_BLEND_LIGHTEN,   /*! Porter-Duff LIGHTEN blend, Premultiplied */
        OPENVG_BLEND_ADDITIVE,   /*! Porter-Duff ADDITIVE blend, Premultiplied */
};

static vg_lite_string blend_name[26] = {
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
        "PREMULTIPLY_SRC_OVER",

        "OPENVG_NONE",
        "OPENVG_SRC_OVER",
        "OPENVG_DST_OVER",
        "OPENVG_SRC_IN",
        "OPENVG_DST_IN",
        "OPENVG_MULTIPLY",
        "OPENVG_SCREEN",
        "OPENVG_DARKEN",
        "OPENVG_LIGHTEN",
        "OPENVG_ADDITIVE",
};

static vg_lite_uint8_t blend_result[26][3] = {
/*
* S: src_buffer/image  0x80808080 RGBA(128, 128, 128, 128)
* D: fb1 framebuffer   0xA0BBBBBB RGBA(187, 187, 187, 160)
*/
        {128, 128, 128},  /* NONE                   ! S                                         */
        {221, 221, 221},  /* SRC_OVER               ! S + (1 - Sa) * D                          */
        {234, 234, 234},  /* DST_OVER               ! (1 - Da) * S + D                          */
        {80, 80, 80},     /* SRC_IN                 ! Da * S                                    */
        {94, 94, 94},     /* DST_IN                 ! Sa * D                                    */
        {221, 221, 221},  /* MULTIPLY               ! S * (1 - Da) + D * (1 - Sa) + S * D       */
        {221, 221, 221},  /* SCREEN                 ! S + D - S * D                             */
        {221, 221, 221},  /* DARKEN                 ! min(SrcOver, DstOver)                     */
        {235, 235, 235},  /* LIGHTEN                ! max(SrcOver, DstOver)                     */
        {255, 255, 255},  /* ADDITIVE               ! S + D                                     */
        {93, 93, 93},     /* SUBTRACT               ! D * (1 - Sa)                              */
        {59, 59, 59},     /* SUBTRACT_LVGL          ! D - S                                     */
        {157, 157, 157},  /* NORMAL_LVGL            ! S * Sa + (1 - Sa) * D                     */
        {255, 255, 255},  /* ADDITIVE_LVGL          ! (S + D) * Sa + D * (1 - Sa)               */
        {221, 221, 221},  /* MULTIPLY_LVGL          ! (S * D) * Sa + D * (1 - Sa)               */
        {157, 157, 157},   /* PREMULTIPLY_SRC_OVER  ! S * Sa + (1 - Sa) * D                     */

        {128, 128, 128},  /* OPENVG_NONE            ! S                                                            */
        {151, 151, 151},  /* OPENVG_SRC_OVER        ! (S * Sa + (1 - Sa) * D * Da) / (Sa + (1 - Sa) * Da)          */
        {174, 174, 174},  /* OPENVG_DST_OVER        ! ((1 - Da) * S * Sa + D * Da) / (Da + (1 - Da) * Sa)          */
        {128, 128, 128},  /* OPENVG_SRC_IN          ! (Da * S * Sa) / (Sa * Da)                                    */
        {187, 187, 187},  /* OPENVG_DST_IN          ! (Sa * D * Da) / (Sa * Da)                                    */
        {138, 138, 138},  /* OPENVG_MULTIPLY        ! (S * Sa * (1 - Da) + D * Da * (1 - Sa) + S * Sa * D * Da) / (Sa * (1 - Da) + Da * (1 - Sa) + Sa * Da)       */
        {187, 187, 187},  /* OPENVG_SCREEN          ! (S * Sa + D * Da - S * Sa * D * Da) / (Sa + Da - Sa * Da)    */
        {151, 151, 151},  /* OPENVG_DARKEN          ! min(SrcOver, DstOver)                                        */
        {174, 174, 174},  /* OPENVG_LIGHTEN         ! max(SrcOver, DstOver)                                        */
        /* alpha value overflow */
        {182, 182, 182},  /* OPENVG_ADDITIVE        ! (S * Sa + D * Da) / (Sa + Da)                                */
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

void checkresultpixel(vg_lite_buffer_t *fb, uint32_t i)
{
    uint32_t x = 10;
    uint32_t y = 10;
    uint32_t addr = y * fb->stride + x * 4;
    vg_lite_uint8_t *fbmem = (vg_lite_uint8_t *)fb->memory;
    vg_lite_uint8_t  r, g, b, a;

    r = fbmem[addr];
    g = fbmem[addr+1];
    b = fbmem[addr+2];
    a = fbmem[addr+3];

    if (abs(blend_result[i][0]-r) > 1 || abs(blend_result[i][1]-g) > 1 || abs(blend_result[i][2]-b) > 1) {
        printf("###### Blend mode %s pixel result is NOT correct! ######\n", blend_name[i]);
    }
    else {
        printf("Blend mode %s pixel result is correct! \n", blend_name[i]);
    }
}

void cleanup(void)
{
    if (src_buffer.handle != NULL) {
        vg_lite_free(&src_buffer);
    }
    if (image.handle != NULL) {
        vg_lite_free(&image);
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

    CHECK_ERROR(vg_lite_clear(&src_buffer, NULL, 0X80808080));
    CHECK_ERROR(vg_lite_clear(&image, NULL, 0X80808080));

    for (i = 0; i < sizeof(blend_mode) / sizeof(blend_mode[0]); i++)
    {
        if ((blend_mode[i] >= VG_LITE_BLEND_MULTIPLY && blend_mode[i] <= VG_LITE_BLEND_LIGHTEN) || (blend_mode[i] >= OPENVG_BLEND_MULTIPLY && blend_mode[i] <= OPENVG_BLEND_LIGHTEN)) {
            if (!vg_lite_query_feature(gcFEATURE_BIT_VG_NEW_BLEND_MODE)) {
                continue;
            }
        }
        if (blend_mode[i] >= VG_LITE_BLEND_SUBTRACT_LVGL && blend_mode[i] <= VG_LITE_BLEND_MULTIPLY_LVGL) {
            if (!vg_lite_query_feature(gcFEATURE_BIT_VG_LVGL_SUPPORT)) {
                continue;
            }
        }
        if (blend_mode[i] >= VG_LITE_BLEND_PREMULTIPLY_SRC_OVER) {
            if (!vg_lite_query_feature(gcFEATURE_BIT_VG_SRC_PREMULTIPLIED) && !vg_lite_query_feature(gcFEATURE_BIT_VG_HW_PREMULTIPLY)) {
                continue;
            }
        }

        CHECK_ERROR(vg_lite_clear(fb1, NULL, 0XA0BBBBBB));
        vg_lite_identity(&matrix);
        CHECK_ERROR(vg_lite_finish());
        vg_lite_save_png("fb1_image.png", fb1);
        vg_lite_save_png("src_buffer.png", &src_buffer);

        CHECK_ERROR(vg_lite_blit(fb1, &src_buffer, &matrix, blend_mode[i], 0, filter));
        CHECK_ERROR(vg_lite_finish());
        sprintf(fname, "blend_mode_%s_1.png", blend_name[i]);
        vg_lite_save_png(fname, fb1);
        checkresultpixel(fb1, i);


        CHECK_ERROR(vg_lite_clear(fb1, NULL, 0XA0BBBBBB));
        CHECK_ERROR(vg_lite_blit_rect(fb1, &src_buffer, &rect, &matrix, blend_mode[i], 0, filter));
        CHECK_ERROR(vg_lite_finish());
        sprintf(fname, "blend_mode_%s_2.png", blend_name[i]);
        vg_lite_save_png(fname, fb1);
        checkresultpixel(fb1, i);

        CHECK_ERROR(vg_lite_clear(fb1, NULL, 0XA0BBBBBB));
        CHECK_ERROR(vg_lite_draw(fb1, &path, VG_LITE_FILL_EVEN_ODD, &matrix, blend_mode[i], 0X80808080));
        CHECK_ERROR(vg_lite_finish());
        sprintf(fname, "blend_mode_%s_3.png", blend_name[i]);
        vg_lite_save_png(fname, fb1);
        checkresultpixel(fb1, i);

        vg_lite_identity(&matPath);
        vg_lite_translate(5, 5, &matrix);

        CHECK_ERROR(vg_lite_clear(fb1, NULL, 0XA0BBBBBB));
        CHECK_ERROR(vg_lite_draw_pattern(fb1, &path, VG_LITE_FILL_EVEN_ODD, &matPath, &image, &matrix, blend_mode[i], VG_LITE_PATTERN_COLOR, 0XFF800000, 0, filter));
        CHECK_ERROR(vg_lite_finish());
        sprintf(fname, "blend_mode_%s_4.png", blend_name[i]);
        vg_lite_save_png(fname, fb1);
        checkresultpixel(fb1, i);
    }

ErrorHandler:
    cleanup();
    return 0;
}