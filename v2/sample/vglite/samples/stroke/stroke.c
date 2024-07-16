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

static vg_lite_buffer_t buffer;
static vg_lite_buffer_t * fb;

vg_lite_path_type_t draw_type[] = {
    VG_LITE_DRAW_FILL_PATH,
    VG_LITE_DRAW_STROKE_PATH,
    VG_LITE_DRAW_FILL_STROKE_PATH
};

vg_lite_cap_style_t cap_style[] = {
    VG_LITE_CAP_BUTT,
    VG_LITE_CAP_ROUND,
    VG_LITE_CAP_SQUARE
};

vg_lite_join_style_t join_style[] = {
    VG_LITE_JOIN_MITER,
    VG_LITE_JOIN_ROUND,
    VG_LITE_JOIN_BEVEL
};

static uint8_t sides_cmd[] = {
    VLC_OP_MOVE,
    VLC_OP_SCWARC,
    VLC_OP_LINE,
    VLC_OP_SCWARC,
    VLC_OP_LINE,
    VLC_OP_SCWARC,
    VLC_OP_LINE,
    VLC_OP_SCWARC,

    VLC_OP_END
};

float sides_data_left[] = {
    50, 0,
    50, 50, 0, 0, 50,
    0, 100,
    50, 50, 0, 50, 150,
    200, 150,
    50, 50, 0, 250, 100,
    250, 50,
    50, 50, 0, 200, 0
};

static vg_lite_path_t path;
void cleanup(void)
{
    if (buffer.handle != NULL) {
        vg_lite_free(&buffer);
    }

    vg_lite_clear_path(&path);

    vg_lite_close();
}

int main(int argc, const char * argv[])
{
    uint32_t feature_check = 0;
    vg_lite_filter_t filter;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_matrix_t matrix;
    uint32_t data_size;
    vg_lite_float_t dash[2] = {32,32};
    int i, j;
    char filename[30];

    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    filter = VG_LITE_FILTER_POINT;

    fb_scale = (float)fb_width / DEFAULT_SIZE;
    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);

    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGB565;
    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    vg_lite_identity(&matrix);
    vg_lite_translate(5,5,&matrix);
    data_size = vg_lite_get_path_length(sides_cmd, sizeof(sides_cmd), VG_LITE_FP32);

    for(i = 0 ; i < sizeof(cap_style)/sizeof(cap_style[0]); i++)
        for(j = 0 ; j < sizeof(join_style)/sizeof(join_style[0]); j++)
        {
            CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));
            sprintf(filename,"stroke%d_%d.png",i,j);
            vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_LOW, data_size, NULL, 0, 0, 0, 0);
            path.path = malloc(data_size);
            CHECK_ERROR(vg_lite_append_path(&path, sides_cmd, sides_data_left, sizeof(sides_cmd)));

            CHECK_ERROR(vg_lite_set_stroke(&path,cap_style[i],join_style[j],4,8,dash,2,8,0));
            CHECK_ERROR(vg_lite_update_stroke(&path));
            CHECK_ERROR(vg_lite_set_path_type(&path,VG_LITE_DRAW_STROKE_PATH));
            CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF0000FF));

            CHECK_ERROR(vg_lite_finish());
            vg_lite_save_png(filename, fb);
            vg_lite_clear_path(&path);
            memset(&path,0,sizeof(vg_lite_path_t));
        }

    for(j = 0 ; j < sizeof(join_style)/sizeof(join_style[0]); j++)
    {
        CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));
        sprintf(filename,"stroke%d.png",j + 3);
        vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0);
        path.path = malloc(data_size);
        CHECK_ERROR(vg_lite_append_path(&path, sides_cmd, sides_data_left, sizeof(sides_cmd)));

        CHECK_ERROR(vg_lite_set_stroke(&path,VG_LITE_CAP_ROUND,join_style[j],4,8,NULL,0,8,0));
        CHECK_ERROR(vg_lite_update_stroke(&path));
        CHECK_ERROR(vg_lite_set_path_type(&path,draw_type[0]));
        CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF0000FF));
        CHECK_ERROR(vg_lite_set_path_type(&path,draw_type[1]));
        CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF00FFFF));

        CHECK_ERROR(vg_lite_finish());
        vg_lite_save_png(filename, fb);
        vg_lite_clear_path(&path);
        memset(&path,0,sizeof(vg_lite_path_t));
    }
    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));
    sprintf(filename, "fill_stroke.png");
    vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0);
    path.path = malloc(data_size);
    CHECK_ERROR(vg_lite_append_path(&path, sides_cmd, sides_data_left, sizeof(sides_cmd)));

    CHECK_ERROR(vg_lite_set_stroke(&path, VG_LITE_CAP_ROUND, join_style[j], 4, 8, NULL, 0, 8, 0));
    CHECK_ERROR(vg_lite_update_stroke(&path));
    CHECK_ERROR(vg_lite_set_path_type(&path, draw_type[2]));
    CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF0000FF));

    CHECK_ERROR(vg_lite_finish());
    vg_lite_save_png(filename, fb);
    vg_lite_clear_path(&path);
    memset(&path, 0, sizeof(vg_lite_path_t));

ErrorHandler:
    cleanup();
    return 0;
}
