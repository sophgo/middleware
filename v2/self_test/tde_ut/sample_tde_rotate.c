#include <stdio.h>
#include "cvi_sys.h"
#include "cvi_tde.h"
#include "tde_ut.h"
#include "sample_tde_util.h"

CVI_S32 sample_tde_do_rotate(cvi_tde_surface *src_surface,
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

	/* 2. do some operations on surfaces */
	single_src.src_surface = src_surface;
	single_src.dst_surface = dst_surface;
	single_src.src_rect = src_rect;
	single_src.dst_rect = dst_rect;
	ret = cvi_tde_rotate(handle, &single_src, CVI_TDE_ROTATE_CLOCKWISE_270);
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

CVI_S32 sample_tde_rotate(CVI_VOID)
{
	cvi_tde_rect src_rect;
	cvi_tde_rect dst_rect;
	CVI_U32 fill_data = 0x0000FF00;   // green
	CVI_U32 surf_fmt = CVI_TDE_COLOR_FORMAT_ARGB8888;
	cvi_tde_surface src_surface = {0};
	cvi_tde_surface dst_surface = {0};
	CVI_U8 *back_ground_vir = NULL;
	CVI_U32 pixel_bytes = 0;
	CVI_U32 surf_width = 256;
	CVI_U32 surf_height = 256;

	pixel_bytes = sample_tde_get_pixel_bytes_by_fmt(surf_fmt);

	if (CVI_SYS_IonAlloc(&src_surface.phys_addr, (CVI_VOID**)&back_ground_vir, "sample_tde_rotate", surf_width * surf_height * pixel_bytes * 2) != CVI_SUCCESS) {
		return CVI_FAILURE;
	}

	if (back_ground_vir == NULL || src_surface.phys_addr == 0) {
		return CVI_FAILURE;
	}

	sample_tde_create_surface(&src_surface, surf_fmt, surf_width, surf_height, surf_width * pixel_bytes);
	sample_tde_create_surface(&dst_surface, surf_fmt, surf_width, surf_height, surf_width * pixel_bytes);
    dst_surface.phys_addr = src_surface.phys_addr + src_surface.stride * src_surface.height;

	src_rect.pos_x = 0;
	src_rect.pos_y = 0;
	src_rect.width = src_surface.width;
	src_rect.height = src_surface.height;
	fill_data = 0x0000FF00;   // green
	sample_tde_do_fill_surface(&src_surface, &src_rect, fill_data);

	src_rect.pos_x = 0;
	src_rect.pos_y = 0;
	src_rect.width = dst_surface.width;
	src_rect.height = dst_surface.height;
	fill_data = 0x000000FF;   // red
	sample_tde_do_fill_surface(&dst_surface, &src_rect, fill_data);

	src_rect.pos_x = 0;
	src_rect.pos_y = 0;
	src_rect.width = 50;
	src_rect.height = 100;

	dst_rect.pos_x = 50;
	dst_rect.pos_y = 50;
	dst_rect.width = 100;
	dst_rect.height = 50;
	if (sample_tde_do_rotate(&src_surface, &dst_surface, &src_rect, &dst_rect) == CVI_SUCCESS) {
		printf("rotate success!\n");
		cvi_save_bmp("sample_tde_rotate.bmp", back_ground_vir + src_surface.stride * src_surface.height, dst_surface.width,
			dst_surface.height, dst_surface.stride, dst_surface.color_format);
	}

	CVI_SYS_IonFree(src_surface.phys_addr, back_ground_vir);

	return CVI_SUCCESS;
}