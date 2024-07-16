#include <stdio.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "tiger_paths.h"

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
static int fb_width = 1920, fb_height = 720;
static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t* fb;

void tiger_sample_1(void)
{
    static vg_lite_buffer_t hpm_vg_buf;
    vg_lite_buffer_t* draw_buf = &hpm_vg_buf;
    vg_lite_matrix_t matrix;
    vg_lite_float_t degrees = 1.111;
    int pathstart = 14;
    int pathend = 14;


    float scale = 2;
    memset(draw_buf, 0x00, sizeof(vg_lite_buffer_t));
    draw_buf->width = 1920;
    draw_buf->height = 720;
    draw_buf->format = VG_LITE_BGRA8888;
    vg_lite_error_t error = vg_lite_allocate(draw_buf);
    if (error != VG_LITE_SUCCESS)
        printf("draw_buf allocate failed-%d\n", error);
    
    for (int k = pathstart, testps, testpe; k <= pathend; k++) {
        testps = pathCount / 8 * 3 + k;
        testpe = pathCount / 8 * (3) + k + 1;

        vg_lite_identity(&matrix);
        vg_lite_translate(700, 300, &matrix);
        vg_lite_scale(scale, scale, &matrix);

        vg_lite_error_t err = VG_LITE_SUCCESS;

        for (int i = testps; i < testpe; i++) {
            err = vg_lite_upload_path(&path[i]);
            if (err != VG_LITE_SUCCESS)
                printf("vg_lite_upload_path[%d]: %d\n", i, err);
        }

        uint32_t i = 0;
        while (i < 10000) {
            vg_lite_rotate(degrees, &matrix);
            if (i != 83 && i != 82) {
                i++;
                continue;
            }
            err = vg_lite_clear(draw_buf, NULL, 0x00FF0000);
            if (err != VG_LITE_SUCCESS)
                printf("vg_lite_clear: %d\n", err);

            /* Draw the path using the matrix. */
            for (int i = testps; i < testpe; i++) {
                err = vg_lite_draw(draw_buf, &path[i], VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, color_data[i]);
                if (err != VG_LITE_SUCCESS)
                    printf("vg_lite_draw[%d]: %d\n", i, err);
            }

            err = vg_lite_finish();
            if (err != VG_LITE_SUCCESS)
                printf("vg_lite_finish: %d\n", err);

            uint32_t mem_size = 0;
            err = vg_lite_mem_avail(&mem_size);
            if (err != VG_LITE_SUCCESS)
                printf("vg_lite_mem_avail: %d\n", err);
            printf("m: %u\n", mem_size);

            char filename[30];
            sprintf(filename, "tiger_%d_%d.png", k, i++);
            vg_lite_save_png(filename, draw_buf);
            printf("cnt: %d\n", i);

        }
    }
}

void cleanup(void)
{
    int32_t i;

    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }

    for (i = 0; i < pathCount; i++)
    {
        vg_lite_clear_path(&path[i]);
    }

    vg_lite_close();
}

int main(int argc, const char* argv[])
{
    /* Initialize vglite. */
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));
    tiger_sample_1();

ErrorHandler:
    // Cleanup.
    cleanup();
    return 0;
}
