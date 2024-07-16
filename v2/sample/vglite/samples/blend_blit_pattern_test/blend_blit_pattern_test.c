#include <stdio.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "hmi_res_sprite.h"

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

 static vg_lite_buffer_t buffer1;     //offscreen framebuffer object for rendering.
 static vg_lite_buffer_t buffer2;     //offscreen framebuffer object for rendering.

 vg_lite_path_t hmi_img_main_rpm_circle_path;

struct hmi_img {
    vg_lite_buffer_format_t vg_format;
    vg_lite_buffer_image_mode_t vg_image_mode;
    vg_lite_buffer_t vg_buf;
    const void* img;
    uint16_t width;
    uint16_t height;

};

int hmi_img_init(struct hmi_img* img)
{
    vg_lite_buffer_t* vg_buf = &img->vg_buf;
    memset(vg_buf, 0x00, sizeof(vg_lite_buffer_t));
    vg_buf->width = img->width;
    vg_buf->height = img->height;
    vg_buf->format = img->vg_format;
    vg_buf->image_mode = img->vg_image_mode;
    //vg_buf->transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    vg_lite_error_t error = vg_lite_allocate(vg_buf);
    if (error != VG_LITE_SUCCESS) {
        printf("alloc_image_buf failed-%d\n", error);
        return -1;
    }

    vg_lite_clear(vg_buf, NULL, 0x000000);
    vg_lite_finish();
    /*uint8_t* dst = (uint8_t*)vg_buf->address;*/
    uint8_t* dst = (uint8_t*)vg_buf->memory;
    const uint8_t* src = img->img;
    uint16_t src_stride = img->vg_format == VG_LITE_BGR565 ? img->width * 2 : img->width * 4;
    for (int i = 0; i < img->height; i++)
        memcpy(dst + i * vg_buf->stride, src + i * src_stride, src_stride);

    return 0;
}

struct hmi_img hmi_img_main_rpm_circle = {
    .vg_format = VG_LITE_BGRA8888,
    .vg_image_mode = VG_LITE_NORMAL_IMAGE_MODE,
    .img = hmi_img_main_rpm_circle_noneARGB8888_alpha,
    .width = 296,
    .height = 256,
};

void main_rpm_circle_path_init(vg_lite_path_t* path)
{
    uint8_t cmd[] = {
        VLC_OP_MOVE,
        VLC_OP_LINE,
        VLC_OP_LINE,
        VLC_OP_LINE,
        VLC_OP_LINE,
        VLC_OP_END
    };

    int path_w = 296;
    int path_h = 256;
    int16_t path_data[] = {
        0, 0,
        path_w, 0,
        path_w, path_h,
        0, path_h,
        0, 0
    };
    vg_lite_init_path(path, VG_LITE_S16, VG_LITE_LOW, vg_lite_get_path_length(cmd, sizeof(cmd), VG_LITE_S16), NULL, 0, 0, 0, 0);
    vg_lite_path_append(path, cmd, path_data, sizeof(cmd));
}

void double_buffer_display(void)
{
    vg_lite_matrix_t matrix;
    vg_lite_matrix_t pattern_matrix;
    vg_lite_buffer_t* draw_buf;
    vg_lite_error_t error;
    hmi_img_init(&hmi_img_main_rpm_circle);
    main_rpm_circle_path_init(&hmi_img_main_rpm_circle_path);

    draw_buf = &buffer1;
    draw_buf->width = 800;
    draw_buf->height = 480;
    draw_buf->format = VG_LITE_BGRA8888;
    error = vg_lite_allocate(draw_buf);
    if (error != VG_LITE_SUCCESS)
        printf("buffer allocate failed-%d\n", error);
    draw_buf = &buffer2;
    draw_buf->width = 800;
    draw_buf->height = 480;
    draw_buf->format = VG_LITE_BGRA8888;
    error = vg_lite_allocate(draw_buf);
    if (error != VG_LITE_SUCCESS)
        printf("buffer allocate failed-%d\n", error);
    uint16_t translate_x = 254;
    uint16_t translate_y = 100;
    /*
        * draw main rpm circle
        */
    draw_buf = &buffer1;
    vg_lite_clear(draw_buf, NULL, 0xFF000000);
    vg_lite_identity(&matrix);
    vg_lite_translate(translate_x, translate_y, &matrix);
    vg_lite_blit(draw_buf, &hmi_img_main_rpm_circle.vg_buf,
        &matrix,
        OPENVG_BLEND_SRC_OVER,
        0x0,
        VG_LITE_FILTER_BI_LINEAR);
    /*
     * draw main rpm circle
     */
    draw_buf = &buffer2;
    vg_lite_clear(draw_buf, NULL, 0xFF000000);
    vg_lite_identity(&matrix);
    vg_lite_translate(translate_x, translate_y, &matrix);
    vg_lite_identity(&pattern_matrix);
    vg_lite_translate(translate_x, translate_y, &pattern_matrix);
    vg_lite_draw_pattern(draw_buf,
        &hmi_img_main_rpm_circle_path,
        VG_LITE_FILL_NON_ZERO,
        &matrix,
        &hmi_img_main_rpm_circle.vg_buf,
        &pattern_matrix,
        OPENVG_BLEND_SRC_OVER,
        VG_LITE_PATTERN_COLOR,
        0x0,
        0x0,
        VG_LITE_FILTER_LINEAR);

    vg_lite_finish();
    // Save PNG file.
    vg_lite_save_png("blend_bd_blit.png", &buffer1);
    vg_lite_save_png("blend_bd_draw_pattern.png", &buffer2);
}

static void gpu_vg_lite_startup(void)
{
    vg_lite_error_t error = vg_lite_init(1024, 1024);
    if (error != VG_LITE_SUCCESS) {
        printf("Err(%d): vg_lite_init\n", error);
    }

    char name[64] = "gpu";
    uint32_t chip_id;
    uint32_t chip_rev;
    uint32_t chip_cid;

    vg_lite_get_register(0x30, &chip_cid);
    vg_lite_get_product_info(name, &chip_id, &chip_rev);
    printf("gpu: name: %s, id: 0x%08X, rev: 0x%08X, cid: 0x%08X\n", name, chip_id, chip_rev, chip_cid);
}

void cleanup(void)
{
    int32_t i;

    if (buffer1.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer1);
    }
    if (buffer2.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer2);
    }
    vg_lite_clear_path(&hmi_img_main_rpm_circle_path);

    vg_lite_close();
}

int main(int argc, const char* argv[])
{
    int i;
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;

    /* Initialize vglite. */
    gpu_vg_lite_startup();
    double_buffer_display();

ErrorHandler:
    // Cleanup.
    cleanup();
    return 0;
}
