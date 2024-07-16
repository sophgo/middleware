#include <stdio.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "tiger_paths.h"

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
static int fb_width = 800, fb_height = 1280;
static vg_lite_buffer_t buffer;
static vg_lite_buffer_t* draw_buf;

void cleanup(void)
{
    int32_t i;
	
    if (buffer.handle != NULL) {
        vg_lite_free(&buffer);
    }

    for (i = 0; i < pathCount; i++)
    {
        vg_lite_clear_path(&path[i]);
    }
    
    vg_lite_close();
}

int main(int argc, const char * argv[])
{
    vg_lite_matrix_t matrix;
    vg_lite_float_t degrees = 1.5, scale = 5.5;
    int i, j;
    char result_name[20];

    vg_lite_init(fb_width, fb_height);
    buffer.width = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGBA8888;
    vg_lite_allocate(&buffer);
    draw_buf = &buffer;

    vg_lite_identity(&matrix);
    vg_lite_translate((float)fb_width/3, (float)(fb_height/3), &matrix);
    vg_lite_scale(scale, scale, &matrix);

    for (j = 0; j < 1000; j++) {
        vg_lite_rotate(degrees, &matrix);
        vg_lite_clear(draw_buf, NULL, 0x00FF0000);

        /* Draw the path using the matrix. */
        for (i = 0; i < pathCount; i++) {
            vg_lite_draw(draw_buf, &path[i], VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, color_data[i]);
        }
        vg_lite_finish();
        sprintf(result_name, "tiger_%d.png", j);
        vg_lite_save_png(result_name, draw_buf);
    }

    vg_lite_free(draw_buf);

    cleanup();
    return 0;
}
