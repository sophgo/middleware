#include <stdio.h>
#include "cvi_sys.h"
#include "cvi_tde.h"
#include "sample_tde.h"
#include "sample_tde_util.h"



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
static CVI_S32 sample_draw_line(cvi_tde_surface *dst_surface,
                        const cvi_tde_line *line,
                        CVI_U32 num);
int main(void)
{
    CVI_S32 s32Ret = 0;
    const CVI_U32 surf_width = 256;
    const CVI_U32 surf_height = 256;
	const CVI_U32 surf_fmt = CVI_TDE_COLOR_FORMAT_ARGB8888;
	CVI_U32 pixel_bytes = 0;
    cvi_tde_rect src_rect;
    cvi_tde_rect dst_rect;
    CVI_U32 fill_data = 0x00FFFF; 	// yellow
    cvi_tde_surface src_surface = {0};
    cvi_tde_surface dst_surface = {0};
    CVI_U8 *back_ground_vir = NULL;
    CVI_U8 num = 0;
    cvi_tde_line lines[] = {
        {0, 64, 256, 64, 3, 0xffff00},
        {0, 128, 256, 128, 3, 0xffff00},
        {0, 192, 256, 192, 3, 0xff00},
        {64, 0, 64, 256, 3, 0xffff00},
        {128, 0, 128, 256, 3, 0xffff00},
        {192, 0, 192, 256, 3, 0xff00},
        {128, 0, 256, 128, 3, 0xff},
        {0, 128, 128, 256, 3, 0xff},
        {128, 0, 0, 128, 3, 0xff},
        {256, 128, 128, 256, 3, 0xff},
        {0, 0, 256, 256, 3, 0xff},
        {256, 0, 0, 256, 3, 0xff},
        {128, 0, 256, 192, 3, 0xff00},
        {128, 0, 256, 256, 3, 0xff00},
        {128, 0, 192, 128, 3, 0xff00},
        {128, 0, 192, 192, 3, 0xff00},
        {128, 0, 192,256, 3, 0xff00},
        {128, 0, 0, 192, 3, 0xff00},
        {128, 0, 0, 256, 3, 0xff00},
        {128, 0, 64, 128, 3, 0xff00},
        {128, 0, 64, 192, 3, 0xff00},
        {128, 0, 64, 256, 3, 0xff00},
        {0, 0, 256, 64, 3, 0xff},
        {0, 0, 256, 128, 3, 0xff},
        {256, 0, 0, 64, 3, 0xff},
        {256, 0, 0, 128, 3, 0xff},
        {192, 0, 256, 64, 3, 0xff00},
        {192, 0, 256, 128, 3, 0xff00},
        {64, 0, 0, 64, 3, 0xff00},
        {64, 0, 0, 128, 3, 0xff00}
    };

	s32Ret = sample_tde_init();
	if (s32Ret != CVI_SUCCESS) {
		return CVI_FAILURE;
	}

	pixel_bytes = sample_tde_get_pixel_bytes_by_fmt(surf_fmt);

    if (CVI_SYS_IonAlloc(&src_surface.phys_addr, (CVI_VOID**)&back_ground_vir,
		"sample_tde", surf_width * surf_height * pixel_bytes *2) != CVI_SUCCESS) {
		sample_tde_exit();
		return CVI_FAILURE;
    }

    if (back_ground_vir == NULL || src_surface.phys_addr == 0) {
        s32Ret = CVI_FAILURE;
		goto exit;
    }

	sample_tde_create_surface(&src_surface, surf_fmt, surf_width, surf_height, surf_width * pixel_bytes);
	sample_tde_create_surface(&dst_surface, surf_fmt, surf_width, surf_height, surf_width * pixel_bytes);
    dst_surface.phys_addr = src_surface.phys_addr + src_surface.stride * src_surface.height;

    src_rect.pos_x = 0;
    src_rect.pos_y = 0;
    src_rect.width = src_surface.width;
    src_rect.height = src_surface.height;

    // fill with yellow color
	fill_data = 0x00FFFF; 	// yellow
    if (sample_fill_surface(&src_surface, &src_rect, fill_data) == CVI_SUCCESS) {
        cvi_save_bmp("sample_tde_back_ground.bmp", back_ground_vir, src_surface.width, src_surface.height,
            src_surface.stride, src_surface.color_format);
    } else {
        s32Ret = CVI_FAILURE;
		goto exit;
    }

    // fill with blue color
	fill_data = 0xFF0000; 	// blue
    if (sample_fill_surface(&dst_surface, &src_rect, fill_data) == CVI_SUCCESS) {
        cvi_save_bmp("sample_tde_dst_surface.bmp",
			back_ground_vir + src_surface.stride * src_surface.height, src_surface.width,
            src_surface.height, src_surface.stride, src_surface.color_format);
	} else {
        s32Ret = CVI_FAILURE;
		goto exit;
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
        cvi_save_bmp("sample_tde_quick_copy.bmp",
        back_ground_vir + src_surface.stride * src_surface.height, src_surface.width,
            src_surface.height, src_surface.stride, src_surface.color_format);
	} else {
        s32Ret = CVI_FAILURE;
		goto exit;
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
        cvi_save_bmp("sample_tde_quick_resize.bmp",
        back_ground_vir + src_surface.stride * src_surface.height, src_surface.width,
            src_surface.height, src_surface.stride, src_surface.color_format);
	} else {
        s32Ret = CVI_FAILURE;
		goto exit;
	}

    num = sizeof(lines) / sizeof(lines[0]);
    if (sample_draw_line(&dst_surface, lines, num) == CVI_SUCCESS) {
        printf("draw line success!\n");
        cvi_save_bmp("sample_draw_line.bmp",
        back_ground_vir + src_surface.stride * src_surface.height, src_surface.width,
            src_surface.height, src_surface.stride, src_surface.color_format);
	} else {
        s32Ret = CVI_FAILURE;
		goto exit;
	}

    s32Ret = sample_tde_rotate();
	if (s32Ret) {
		goto exit;
	}

    s32Ret = sample_tde_draw_corner_box();
	if (s32Ret) {
		goto exit;
	}

    s32Ret = sample_tde_solid_draw();
	if (s32Ret) {
		goto exit;
	}

    s32Ret = sample_tde_bit_blit();
	if (s32Ret) {
		goto exit;
	}

exit:

	CVI_SYS_IonFree(src_surface.phys_addr, back_ground_vir);

	sample_tde_exit();

	if (s32Ret == CVI_SUCCESS)
		printf("SAMPLE_TDE exit success!\n");
	else
		printf("SAMPLE_TDE exit abnormally!\n");

	return s32Ret;
}

CVI_S32 sample_tde_init(CVI_VOID)
{
	CVI_S32  s32Ret;

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		return CVI_FAILURE;
	}

	s32Ret = cvi_tde_open();
	if (s32Ret != CVI_SUCCESS) {
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 sample_tde_exit(CVI_VOID)
{
	cvi_tde_close();

	CVI_SYS_Exit();

	return CVI_SUCCESS;
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

CVI_S32 sample_quick_resize(cvi_tde_surface *src_surface,
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

CVI_S32 sample_draw_line(cvi_tde_surface *dst_surface,
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



