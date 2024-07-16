/*
 Resolution: 256 x 256
 Format: VG_LITE_RGB565
 Transformation: None
 Alpha Blending: None
 Related APIs: vg_lite_clear
 Description: Clear whole buffer with blue first, then clear a sub-rectangle of the buffer with red.
 */
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
        printf("[%s: %d] failed.error type is %s\n", __func__, __LINE__,error_type[error]);\
        goto ErrorHandler; \
    }

static int   fb_width = 256, fb_height = 256;
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * fb;

void cleanup(void)
{
    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }

    vg_lite_close();
}

int main(int argc, const char * argv[])
{
    printf("clear main  2023-04-01 10:47\n");
    if (argc < 8) {
        printf("clear width height rect_X rect_Y rect_width rect_height format loops [dump png] [sleep(us)]\n");
        printf("\nformat:   0 VG_LITE_RGBA8888;\n");
        printf("            1 VG_LITE_RGB565;\n");
        printf("            2 VG_LITE_ARGB8888;\n");
        printf("            3 VG_LITE_RGB888;\n");
        printf("            4 VG_LITE_ABGR1555;\n");
        printf("sleep(us): for fps  control, defult not set. sleep(7000) core0 is 35fps.\n");
        printf("eg: \n");
        printf("    clear 4096 2160 64 64 2880 1536 0 1 \n");
        return 0;
    }

    fb_width  = atoi(argv[1]);
    fb_height = atoi(argv[2]);
    // printf("fb_width=%d  fb_height=%d\n", fb_width, fb_height);

    int rectX = atoi(argv[3]);
    int rectY = atoi(argv[4]);
    int rectW = atoi(argv[5]);
    int rectH = atoi(argv[6]);
    // printf("rect[%d, %d, %d, %d]. \n", rectX, rectY, rectW, rectH);

    int loops = atoi(argv[8]);

    int dump_png = 0;
    if (argc == 10) {
        dump_png = atoi(argv[9]);
    }
    if (dump_png == 1) {
        // printf("dump png.\n");
    } else {
        // printf("not dump png.\n");
    }

    int fps_us = 0;
    if (argc == 11) {
        fps_us = atoi(argv[10]);
    }

    vg_lite_rectangle_t rect = { rectX, rectY, rectW, rectH };

    // vg_lite_rectangle_t rect = { 64, 64, 64, 64 };
    vg_lite_error_t error = VG_LITE_SUCCESS;
    // Initialize the blitter.
    CHECK_ERROR(vg_lite_init(0, 0));

    fb_scale = (float)fb_width / DEFAULT_SIZE;
    // printf("Framebuffer size: %d x %d\n", fb_width, fb_height);

    // Allocate the off-screen buffer.
    buffer.width  = fb_width;
    buffer.height = fb_height;
    // buffer.format = VG_LITE_RGB565;

    if (atoi(argv[7]) == 0) {
        buffer.format = VG_LITE_RGBA8888;
        // printf("format is VG_LITE_RGBA8888\n");
    }else if (atoi(argv[7]) == 1) {
        buffer.format = VG_LITE_RGB565;
        // printf("format is VG_LITE_RGB565\n");
    }else if (atoi(argv[7]) == 2) {
        buffer.format = VG_LITE_ARGB8888;
        // printf("format is VG_LITE_ARGB8888\n");
    }else if (atoi(argv[7]) == 3) {
        buffer.format = VG_LITE_RGB888;
        // printf("format is VG_LITE_RGB888\n");
    }else if (atoi(argv[7]) == 4) {
        buffer.format = VG_LITE_ABGR1555;
        // printf("format is VG_LITE_ABGR1555\n");
    }




    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    // Clear the buffer with blue.
    // CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));
    // Clear a sub-rectangle of the buffer with red.
    for (int i = 0; i<loops;i++) {
        CHECK_ERROR(vg_lite_clear(fb, &rect, 0xFF0000FF));
        CHECK_ERROR(vg_lite_finish());
        if (fps_us != 0) {
            printf("usleep fps_us:%d\n", fps_us);
            usleep(fps_us);
        }
    }
    if (dump_png == 1) {
        // Save PNG file.
        vg_lite_save_png("clear.png", fb);
    }

ErrorHandler:
    // Cleanup.
    cleanup();
    return 0;
}
