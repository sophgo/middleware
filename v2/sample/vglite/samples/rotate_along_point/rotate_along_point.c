/*
 Resolution: 500 x 500
 Format: VG_LITE_RGB565
 Alpha Blending: None
 Related APIs: vg_lite_clear/vg_lite_draw
 Description: Added vg_lite_rotate_2 to test rotate aroud a given center.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include <math.h>

#define DEFAULT_SIZE   256.0f;
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
static int   fb_width = 500, fb_height = 500;
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;
static vg_lite_buffer_t* fb;

static uint8_t sides_cmd[] = {
    VLC_OP_MOVE,
    VLC_OP_LINE,
    VLC_OP_LINE,
    VLC_OP_LINE,
    VLC_OP_END
};

float sides_data_left[] = {
    100, 100,
    300, 100,
    300, 200,
    100, 200
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

static void multiply(vg_lite_matrix_t* matrix, vg_lite_matrix_t* mult)
{
    vg_lite_matrix_t temp;
    int row, column;

    /* Process all rows. */
    for (row = 0; row < 3; row++) {
        /* Process all columns. */
        for (column = 0; column < 3; column++) {
            /* Compute matrix entry. */
            temp.m[row][column] = (matrix->m[row][0] * mult->m[0][column])
                + (matrix->m[row][1] * mult->m[1][column])
                + (matrix->m[row][2] * mult->m[2][column]);
        }
    }

    /* Copy temporary matrix into result. */
    memcpy(matrix, &temp, sizeof(temp));
}

static vg_lite_error_t vg_lite_rotate_2(vg_lite_float_t degrees, vg_lite_matrix_t* matrix, vg_lite_float_t tx, vg_lite_float_t ty)
{
#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_rotate %f %p\n", degrees, matrix);
#endif

    /* Convert degrees into radians. */
    vg_lite_float_t angle = (degrees / 180.0f) * 3.141592654f;

    /* Compuet cosine and sine values. */
    vg_lite_float_t cos_angle = cosf(angle);
    vg_lite_float_t sin_angle = sinf(angle);

    /* Set rotation matrix. */
    vg_lite_matrix_t r = { { {cos_angle, -sin_angle, 0.0f},
        {sin_angle, cos_angle, 0.0f},
        {0.0f, 0.0f, 1.0f}
    } };

    vg_lite_matrix_t ts1 = { { {1.0f, 0.0f, tx},
        {0.0f, 1.0f, ty},
        {0.0f, 0.0f, 1.0f}
    } };

    vg_lite_matrix_t ts2 = { { {1.0f, 0.0f, -tx},
        {0.0f, 1.0f, -ty},
        {0.0f, 0.0f, 1.0f}
    } };
    /* Multiply with current matrix. */
    multiply(matrix, &r);

    //Matrix left multiplication
    vg_lite_matrix_t temp;
    int row, column;
    for (row = 0; row < 3; row++) {
        for (column = 0; column < 3; column++) {
            temp.m[row][column] = (ts1.m[row][0] * matrix->m[0][column])
                + (ts1.m[row][1] * matrix->m[1][column])
                + (ts1.m[row][2] * matrix->m[2][column]);
        }
    }
    memcpy(matrix, &temp, sizeof(temp));

    multiply(matrix, &ts2);

    return VG_LITE_SUCCESS;
}

int main(int argc, const char* argv[])
{
    //uint32_t feature_check = 0;
    //vg_lite_filter_t filter;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_matrix_t matrix;
    uint32_t data_size;
    /* Initialize the draw. */
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    //filter = VG_LITE_FILTER_POINT;

    fb_scale = (float)fb_width / DEFAULT_SIZE;
    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);

    /* Allocate the off-screen buffer. */
    buffer.width = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGB565;
    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));

    vg_lite_identity(&matrix);
    vg_lite_rotate_2(90, &matrix, 200, 150);

    data_size = vg_lite_get_path_length(sides_cmd, sizeof(sides_cmd), VG_LITE_FP32);
    vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0);
    CHECK_ERROR(vg_lite_append_path(&path, sides_cmd, sides_data_left, sizeof(sides_cmd)));

    CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF0000FF));

    CHECK_ERROR(vg_lite_finish());
    vg_lite_save_png("rotate_along_point.png", fb);

ErrorHandler:
    cleanup();
    return 0;
}
