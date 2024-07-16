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

vg_lite_buffer_format_t format[] = 
{
    VG_LITE_ABGR8565_PLANAR,
    VG_LITE_BGRA5658_PLANAR,
    VG_LITE_ARGB8565_PLANAR,
    VG_LITE_RGBA5658_PLANAR
};
void cleanup(void)
{
    if (buffer.handle != NULL) {
        vg_lite_free(&buffer);
    }

    vg_lite_close();
}

int main(int argc, const char * argv[])
{
    vg_lite_rectangle_t rect = { 64, 64, 64, 64 };
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int i;
    char filename[20];
    CHECK_ERROR(vg_lite_init(0, 0));
    if(!vg_lite_query_feature(gcFEATURE_BIT_VG_24BIT_PLANAR)) {
        printf("24bit planar is not supported.\n");
        return VG_LITE_NOT_SUPPORT;
    }
    fb_scale = (float)fb_width / DEFAULT_SIZE;
    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);
    
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_ABGR8565_PLANAR;
    
    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;
    
    for(i = 0; i < sizeof(format) / sizeof(format[0]); i++)
    {
        buffer.format = format[i];
        sprintf(filename,"24bit_planar%d.png",i);
        CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));
        CHECK_ERROR(vg_lite_clear(fb, &rect, 0xFF0000FF));
        CHECK_ERROR(vg_lite_finish());
        vg_lite_save_png(filename, fb);
    }

ErrorHandler:
    cleanup();
    return 0;
}
