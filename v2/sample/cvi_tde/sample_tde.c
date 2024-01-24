#include <stdio.h>
#include "cvi_sys.h"
#include "cvi_tde.h"


static CVI_VOID tde_create_surface(cvi_tde_surface *surface, CVI_U32 colorfmt, CVI_U32 w, CVI_U32 h, CVI_U32 stride);

// test cases
static CVI_S32 sample_fill_surface(cvi_tde_surface *surface,
                        cvi_tde_rect *rect, CVI_U32 fill_data);
static CVI_S32 sample_quick_copy(cvi_tde_surface *src_surface,
                        cvi_tde_surface *dst_surface,
                        cvi_tde_rect *src_rect,
                        cvi_tde_rect *dst_rect);
static CVI_S32 sample_quick_resize(cvi_tde_surface *src_surface,
                        cvi_tde_surface *dst_surface,
                        cvi_tde_rect *src_rect,
                        cvi_tde_rect *dst_rect);
static CVI_S32 sampel_draw_line(cvi_tde_surface *dst_surface,
                        const cvi_tde_line *line,
                        CVI_U32 num);

int main(void)
{
    CVI_S32 rc = 0;
    const CVI_U32 surf_width = 256;
    const CVI_U32 surf_height = 256;
    cvi_tde_rect src_rect;
    cvi_tde_rect dst_rect;
    CVI_U32 fill_data = 0xFFFF00;   // yellow
    cvi_tde_surface src_surface = {0};
    cvi_tde_surface dst_surface = {0};
    CVI_U8 *back_ground_vir = NULL;
    CVI_U8 num = 0;
    cvi_tde_line lines[] = {
        {0, 0, 256, 128, 3, 0xffff},
        {0, 128, 256, 256, 3, 0xffff},
        {256, 0, 0, 128, 3, 0xff0000},
        {256, 128, 0, 256, 3, 0xff0000},
    };

    rc = CVI_SYS_Init();
    if (rc != CVI_SUCCESS) {
        return CVI_FAILURE;
    }


    rc = cvi_tde_open();
    if (rc != CVI_SUCCESS) {
        return CVI_FAILURE;
    }

    if (CVI_SYS_IonAlloc(&src_surface.phys_addr, (CVI_VOID**)&back_ground_vir, "sample_tde", surf_width * surf_height * 4 *2) != CVI_SUCCESS) {
        return CVI_FAILURE;
    }

    if (back_ground_vir == NULL || src_surface.phys_addr == 0) {
        return CVI_FAILURE;
    }


    tde_create_surface(&src_surface, CVI_TDE_COLOR_FORMAT_ARGB8888, surf_width, surf_height, surf_width * 4);
    tde_create_surface(&dst_surface, CVI_TDE_COLOR_FORMAT_ARGB8888, surf_width, surf_height, surf_width * 4);
    dst_surface.phys_addr = src_surface.phys_addr + src_surface.stride * src_surface.height;

    src_rect.pos_x = 0;
    src_rect.pos_y = 0;
    src_rect.width = src_surface.width;
    src_rect.height = src_surface.height;

    // fill with yellow color
    if (sample_fill_surface(&src_surface, &src_rect, fill_data) == CVI_SUCCESS) {
        cvi_save_bmp("sample_tde_back_ground.bmp", back_ground_vir, src_surface.width, src_surface.height,
            src_surface.stride, src_surface.color_format);
        cvi_save_raw("sample_tde_back_ground.raw", back_ground_vir, src_surface.width, src_surface.height,
            src_surface.stride, src_surface.color_format);
    }

    // fill with blue color
    if (sample_fill_surface(&dst_surface, &src_rect, 0xFF) == CVI_SUCCESS) {
        cvi_save_bmp("sample_tde_dst_surface.bmp", back_ground_vir + src_surface.stride * src_surface.height, src_surface.width,
            src_surface.height, src_surface.stride, src_surface.color_format);
        cvi_save_raw("sample_tde_dst_surface.raw", back_ground_vir + src_surface.stride * src_surface.height, src_surface.width,
            src_surface.height, src_surface.stride, src_surface.color_format);
    }


    src_rect.pos_x = 0;
    src_rect.pos_y = 0;
    src_rect.width = src_surface.width / 2;
    src_rect.height = src_surface.height / 2;

    dst_rect.pos_x = 0;
    dst_rect.pos_y = dst_surface.height / 2;
    dst_rect.width = dst_surface.width / 2;
    dst_rect.height = dst_surface.height / 2;

    if (sample_quick_copy(&src_surface, &dst_surface, &src_rect, &dst_rect) == CVI_SUCCESS) {
        printf("quick copy success!\n");
        cvi_save_bmp("sample_tde_quick_copy.bmp", back_ground_vir + src_surface.stride * src_surface.height, src_surface.width,
            src_surface.height, src_surface.stride, src_surface.color_format);
        cvi_save_raw("sample_tde_quick_copy.raw", back_ground_vir + src_surface.stride * src_surface.height, src_surface.width,
            src_surface.height, src_surface.stride, src_surface.color_format);
    }

    src_rect.pos_x = 0;
    src_rect.pos_y = 0;
    src_rect.width = 50;
    src_rect.height = 50;

    dst_rect.pos_x = 0;
    dst_rect.pos_y = 0;
    dst_rect.width = dst_surface.width - 100;
    dst_rect.height = dst_surface.height / 2;

    if (sample_quick_resize(&src_surface, &dst_surface, &src_rect, &dst_rect) == CVI_SUCCESS) {
        printf("quick resize success!\n");
        cvi_save_bmp("sample_tde_quick_resize.bmp", back_ground_vir + src_surface.stride * src_surface.height, src_surface.width,
            src_surface.height, src_surface.stride, src_surface.color_format);
        cvi_save_raw("sample_tde_quick_resize.raw", back_ground_vir + src_surface.stride * src_surface.height, src_surface.width,
            src_surface.height, src_surface.stride, src_surface.color_format);
    }

    num = sizeof(lines) / sizeof(lines[0]);
    if (sampel_draw_line(&dst_surface, lines, num) == CVI_SUCCESS) {
        printf("draw line success!\n");
        cvi_save_bmp("sample_draw_line.bmp", back_ground_vir + src_surface.stride * src_surface.height, src_surface.width,
            src_surface.height, src_surface.stride, src_surface.color_format);
        cvi_save_raw("sample_draw_line.raw", back_ground_vir + src_surface.stride * src_surface.height, src_surface.width,
            src_surface.height, src_surface.stride, src_surface.color_format);
    }

    CVI_SYS_IonFree(src_surface.phys_addr, back_ground_vir);

    cvi_tde_close();

    CVI_SYS_Exit();


    return CVI_SUCCESS;
}

CVI_VOID tde_create_surface(cvi_tde_surface *surface, CVI_U32 colorfmt, CVI_U32 w, CVI_U32 h, CVI_U32 stride)
{
    surface->color_format = colorfmt;
    surface->width = w;
    surface->height = h;
    surface->stride = stride;
    surface->alpha0 = 0xff;
    surface->alpha1 = 0xff;
    surface->alpha_max_is_255 = CVI_TRUE;
    surface->support_alpha_ex_1555 = CVI_TRUE;
}

CVI_S32 sample_fill_surface(cvi_tde_surface *surface, cvi_tde_rect *rect, CVI_U32 fill_data)
{
    CVI_S32 ret;
    CVI_S32 handle = 0;
    cvi_tde_none_src none_src = {0};

    /* 1. start job */
    handle = cvi_tde_begin_job();
    if (handle == CVI_ERR_TDE_INVALID_HANDLE) {
        return CVI_FAILURE;
    }

    /* 2. do some operations to surface  */
    none_src.dst_surface = surface;
    none_src.dst_rect = rect;
    ret = cvi_tde_quick_fill(handle, &none_src, fill_data);
    if (ret < 0) {
        cvi_tde_cancel_job(handle);
        return CVI_FAILURE;
    }

    /* 3. submit job */
    ret = cvi_tde_end_job(handle, CVI_FALSE, CVI_TRUE, 1000); /* 1000 time out */
    if (ret < 0) {
        cvi_tde_cancel_job(handle);
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}


CVI_S32 sample_quick_copy(cvi_tde_surface *src_surface,
                        cvi_tde_surface *dst_surface,
                        cvi_tde_rect *src_rect,
                        cvi_tde_rect *dst_rect)
{
    CVI_S32 ret;
    CVI_S32 handle = 0;
    cvi_tde_single_src single_src = {0};

    /* 1. start job */
    handle = cvi_tde_begin_job();
    if (handle == CVI_ERR_TDE_INVALID_HANDLE) {
        return CVI_FAILURE;
    }

    /* 2. do some operations to surface */
    single_src.src_surface = src_surface;
    single_src.dst_surface = dst_surface;
    single_src.src_rect = src_rect;
    single_src.dst_rect = dst_rect;
    ret = cvi_tde_quick_copy(handle, &single_src);
    if (ret < 0) {
        cvi_tde_cancel_job(handle);
        return CVI_FAILURE;
    }

    /* 5. submit job */
    ret = cvi_tde_end_job(handle, CVI_FALSE, CVI_TRUE, 1000); /* 1000 time out */
    if (ret < 0) {
        cvi_tde_cancel_job(handle);
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

static CVI_S32 sample_quick_resize(cvi_tde_surface *src_surface,
                        cvi_tde_surface *dst_surface,
                        cvi_tde_rect *src_rect,
                        cvi_tde_rect *dst_rect)
{
    CVI_S32 ret;
    CVI_S32 handle = 0;
    cvi_tde_single_src single_src = {0};

    /* 1. start job */
    handle = cvi_tde_begin_job();
    if (handle == CVI_ERR_TDE_INVALID_HANDLE) {
        return CVI_FAILURE;
    }

    /* 2. do some operations to surface */
    single_src.src_surface = src_surface;
    single_src.dst_surface = dst_surface;
    single_src.src_rect = src_rect;
    single_src.dst_rect = dst_rect;
    ret = cvi_tde_quick_resize(handle, &single_src);
    if (ret < 0) {
        cvi_tde_cancel_job(handle);
        return CVI_FAILURE;
    }

    /* 5. submit job */
    ret = cvi_tde_end_job(handle, CVI_FALSE, CVI_TRUE, 1000); /* 1000 time out */
    if (ret < 0) {
        cvi_tde_cancel_job(handle);
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

static CVI_S32 sampel_draw_line(cvi_tde_surface *dst_surface,
                        const cvi_tde_line *line,
                        CVI_U32 num)
{
    CVI_S32 ret;
    CVI_S32 handle = 0;

    /* 1. start job */
    handle = cvi_tde_begin_job();
    if (handle == CVI_ERR_TDE_INVALID_HANDLE) {
        return CVI_FAILURE;
    }

    /* 2. do some operations to surface */
    ret = cvi_tde_draw_line(handle, dst_surface, line, num);
    if (ret < 0) {
        cvi_tde_cancel_job(handle);
        return CVI_FAILURE;
    }

    /* 5. submit job */
    ret = cvi_tde_end_job(handle, CVI_FALSE, CVI_TRUE, 1000); /* 1000 time out */
    if (ret < 0) {
        cvi_tde_cancel_job(handle);
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;}