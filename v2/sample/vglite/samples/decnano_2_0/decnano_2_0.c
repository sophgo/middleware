#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
        printf("[%s: %d] error type is %s\n", __func__, __LINE__,error_type[error]);\
        goto ErrorHandler; \
    }

static vg_lite_buffer_t srcbuff, tmpbuff, dstbuff;
static int buffer_width = 320, buffer_height = 320;

vg_lite_compress_mode_t compress_mode[] = {
    VG_LITE_DEC_NON_SAMPLE,
    VG_LITE_DEC_HSAMPLE,
    VG_LITE_DEC_HV_SAMPLE,
};

vg_lite_buffer_format_t compress_format[] = {
     VG_LITE_BGRA8888,
     VG_LITE_BGRX8888,
     VG_LITE_BGR888
};
/* Formats that supported by compressionand decompression of dec2.0 feature. */

void cleanup(void)
{
    if (srcbuff.handle != NULL)
        vg_lite_free(&srcbuff);

    if (tmpbuff.handle != NULL)
        vg_lite_free(&tmpbuff);

    if (dstbuff.handle != NULL)
        vg_lite_free(&dstbuff);

    vg_lite_close();
}

int main(int argc, const char* argv[])
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_matrix_t matrix;
    //vg_lite_blend_t blend = VG_LITE_BLEND_SRC_OVER;
    char filename_dec[35], filename_png[35];
    int i, j;

    CHECK_ERROR(vg_lite_init(0, 0));

    for (i = 0; i < sizeof(compress_mode) / sizeof(compress_mode[0]); i++) {
        for (j = 0; j < sizeof(compress_format) / sizeof(compress_format[0]); j++) {
            srcbuff.width = buffer_width;
            srcbuff.height = buffer_height;
            srcbuff.format = compress_format[j];
            CHECK_ERROR(vg_lite_allocate(&srcbuff));
            vg_lite_load_png(&srcbuff, "tiger320.png");

            tmpbuff.width = buffer_width;
            tmpbuff.height = buffer_height;
            tmpbuff.compress_mode = compress_mode[i];
            tmpbuff.format = compress_format[j];
            CHECK_ERROR(vg_lite_allocate(&tmpbuff));

            /* When using VG_LITE_DEC_HV_SAMPLE mode, only VG_LITE_TILED mode is supported. */
            tmpbuff.tiled = (tmpbuff.compress_mode == VG_LITE_DEC_HV_SAMPLE) ? VG_LITE_TILED : VG_LITE_LINEAR;

            /* Dec target buffer should be cleared first! */
            CHECK_ERROR(vg_lite_clear(&tmpbuff, NULL, 0xFFFFFFFF));

            CHECK_ERROR(vg_lite_identity(&matrix));
            CHECK_ERROR(vg_lite_blit(&tmpbuff, &srcbuff, &matrix, 0, 0, 0));
            CHECK_ERROR(vg_lite_finish());

            sprintf(filename_dec, "dec_mode_%d_format_%d.dec", i + 1, j + 1);
            if (vg_lite_query_feature(gcFEATURE_BIT_VG_DEC_COMPRESS_2_0)) {
                CHECK_ERROR(vg_lite_save_decnano_2_0_compressd_data(filename_dec, &tmpbuff));
            }
            else {
                CHECK_ERROR(vg_lite_save_decnano_compressd_data(filename_dec, &tmpbuff));
            }

            if (vg_lite_query_feature(gcFEATURE_BIT_VG_IM_DEC_INPUT)) {
                dstbuff.width = buffer_width;
                dstbuff.height = buffer_height;
                dstbuff.format = compress_format[j];
                CHECK_ERROR(vg_lite_allocate(&dstbuff));
                CHECK_ERROR(vg_lite_blit(&dstbuff, &tmpbuff, &matrix, 0, 0, 0));
                CHECK_ERROR(vg_lite_finish());

                sprintf(filename_png, "decompression_mode_%d_format_%d.png", i + 1, j + 1);
                vg_lite_save_png(filename_png, &dstbuff);
                CHECK_ERROR(vg_lite_free(&dstbuff));
            }
            else {
                printf(" dec file input is not support\n");
            }

            CHECK_ERROR(vg_lite_free(&tmpbuff));
            CHECK_ERROR(vg_lite_free(&srcbuff));
        }
    }

ErrorHandler:
    cleanup();
    return 0;
}
