
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DEFAULT_SIZE   480.0f;
#define DEFAULT_WIDTH 480
#define DEFAULT_HEIGHT 640

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
static int   fb_width = DEFAULT_WIDTH, fb_height = DEFAULT_HEIGHT;

static vg_lite_buffer_t buffer;
static vg_lite_buffer_t* fb;

vg_lite_gradient_spreadmode_t spreadmode[] = {
    VG_LITE_GRADIENT_SPREAD_FILL,
    VG_LITE_GRADIENT_SPREAD_PAD,
    VG_LITE_GRADIENT_SPREAD_REPEAT,
    VG_LITE_GRADIENT_SPREAD_REFLECT,
};
/*
*-----*
/       \
/         \
*           *
|          /
|         X
|          \
*           *
\         /
\       /
*-----*
*/
static short path_data[] = {
    2, 30, 50,
    4, 450, 50,
    4, 450, 350,
    4, 30, 350,
    0,
};

static vg_lite_path_t path = {
    { 0,   0,
      400, 400 },
    VG_LITE_HIGH,
    VG_LITE_S16,
    {0},
    sizeof(path_data),
    path_data,
    1
};

void cleanup(void)
{
    if (buffer.handle != NULL) {
        vg_lite_free(&buffer);
    }

    vg_lite_close();
}

int main(int argc, const char* argv[])
{
    //vg_lite_filter_t filter;
    vg_lite_ext_linear_gradient_t grad;

    vg_lite_color_ramp_t vg_color_ramp[] =
    {
        {
            0.1f,
            0.9f, 0.2f, 0.0f, 1.0f
        },
        {
            0.45f,
            0.0f, 8.0f, 0.3f, 0.8f
        },
        {
            0.75f,
            0.2f, 0.1f, 0.8f, 0.9f
        },
        {
            0.9f,
            0.8f, 0.3f, 0.1f, 1.0f
        }
    };
    vg_lite_linear_gradient_parameter_t linear_gradient = { 25.0f ,50.0f ,470.0f ,50.0f };
    vg_lite_matrix_t* mat_grad;
    vg_lite_matrix_t mat_path;
    char filename[20];

    /* Initialize vg_lite engine. */
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    //filter = VG_LITE_FILTER_POINT;
    if (!vg_lite_query_feature(gcFEATURE_BIT_VG_LINEAR_GRADIENT_EXT)) {
        printf("linearGradient is not supported.\n");
        return VG_LITE_NOT_SUPPORT;
    }
    /* Allocate the off-screen buffer. */
    buffer.width = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_BGRX8888;

    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    printf("Render size: %d x %d\n", fb_width, fb_height);

    sprintf(filename, "linearGradient.png");
    memset(&grad, 0, sizeof(grad));
    vg_lite_set_linear_grad(&grad, 4, vg_color_ramp, linear_gradient, spreadmode[0], 1);
    mat_grad = vg_lite_get_linear_grad_matrix(&grad);
    vg_lite_identity(mat_grad);
    vg_lite_update_linear_grad(&grad);

    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xffffffff));
    vg_lite_identity(&mat_path);

    CHECK_ERROR(vg_lite_draw_linear_grad(fb, &path, VG_LITE_FILL_EVEN_ODD, &mat_path, &grad, 0, VG_LITE_BLEND_NONE, VG_LITE_FILTER_LINEAR));
    CHECK_ERROR(vg_lite_finish());
    vg_lite_clear_linear_grad(&grad);

    printf("Rendering  done\n");
    vg_lite_save_png(filename, fb);

ErrorHandler:

    cleanup();
    return 0;
}