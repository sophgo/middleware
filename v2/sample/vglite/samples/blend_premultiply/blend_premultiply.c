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
printf("[%s: %d] failed.error type is %s\n", __func__, __LINE__, error_type[error]); \
goto ErrorHandler; \
}

static vg_lite_buffer_t src;
static vg_lite_buffer_t dst;

void cleanup(void)
{
    if (src.handle != NULL) {
        // Free the src memory.
        vg_lite_free(&src);
    }

    if (dst.handle != NULL) {
        // Free the dst memory.
        vg_lite_free(&dst);
    }

    vg_lite_close();
}

int main(int argc, const char* argv[])
{
    /* Initialize vg_lite engine. */
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_init(32, 32));

    /* Load the image. */
    if (vg_lite_load_raw(&dst, "landscape.raw") != 0) {
        printf("load raw file error\n");
        cleanup();
        return -1;
    }

    /* Clear src with blue. */
    src.width = dst.width;
    src.height = dst.height;
    src.format = dst.format;
    CHECK_ERROR(vg_lite_allocate(&src));
    CHECK_ERROR(vg_lite_clear(&src, NULL, 0x800000FF));

    /* Blit with VG_LITE_BLEND_PREMULTIPLY_SRC_OVER */
    CHECK_ERROR(vg_lite_blit(&dst, &src, NULL, VG_LITE_BLEND_PREMULTIPLY_SRC_OVER, 0, 0));

    CHECK_ERROR(vg_lite_finish());

    /* Save png file. */ 
    vg_lite_save_png("blend_normal.png", &dst);

ErrorHandler:
    /* Cleanup. */
    cleanup();
    return 0;
}
