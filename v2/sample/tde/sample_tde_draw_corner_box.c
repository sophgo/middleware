#include <stdio.h>
#include "cvi_sys.h"
#include "cvi_tde.h"
#include "sample_tde.h"
#include "sample_tde_util.h"

CVI_S32 sample_tde_do_draw_corner_box(cvi_tde_surface *dst_surface,
					cvi_tde_corner_rect *corner_rect,
					CVI_U32 num)
{
	CVI_S32 rc;
	CVI_S32 handle = 0;

	/* 1. start job */
	handle = cvi_tde_begin_job();
	if (handle == CVI_ERR_TDE_INVALID_HANDLE) {
		return CVI_FAILURE;
	}

	/* 2. do some operations on surfaces */
	rc = cvi_tde_draw_corner_box(handle, dst_surface, corner_rect, num);
	if (rc < 0) {
		cvi_tde_cancel_job(handle);
		return CVI_FAILURE;
	}

	/* 3. submit job */
	rc = cvi_tde_end_job(handle, CVI_FALSE, CVI_TRUE, 1000);
	if (rc < 0) {
		cvi_tde_cancel_job(handle);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 sample_tde_draw_corner_box(CVI_VOID)
{
	cvi_tde_rect src_rect;
	CVI_U32 fill_data = 0x0000FF00;   // green
	CVI_U32 surf_fmt = CVI_TDE_COLOR_FORMAT_ARGB8888;
	cvi_tde_surface src_surface = {0};
	cvi_tde_surface dst_surface = {0};
	CVI_U8 *back_ground_vir = NULL;
	CVI_U32 pixel_bytes = 0;
	CVI_U32 surf_width = 256;
	CVI_U32 surf_height = 256;
	cvi_tde_rect rect1 = {0, 100, 300, 56};
	cvi_tde_rect rect2 = {100, 0, 56, 300};
	cvi_tde_rect rect3 = {100, 100, 56, 56};
	cvi_tde_corner_rect_info info1 = {16, 4, 0xffff, 0xff0000};
	cvi_tde_corner_rect_info info2 = {16, 4, 0xff0000, 0xffff};
	cvi_tde_corner_rect_info info3 = {8, 4, 0xffff00, 0xff};
	cvi_tde_corner_rect box_rects[] = {
							{&rect1, &info1},
							{&rect2, &info2},
							{&rect3, &info3},
						};
	CVI_U32 num = 0;

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

	num = sizeof(box_rects) / sizeof(box_rects[0]);
	if (sample_tde_do_draw_corner_box(&dst_surface, box_rects, num) == CVI_SUCCESS) {
		printf("draw box success!\n");
		cvi_save_bmp("sample_tde_draw_boxes.bmp", back_ground_vir + src_surface.stride * src_surface.height, dst_surface.width,
			dst_surface.height, dst_surface.stride, dst_surface.color_format);
	}

	CVI_SYS_IonFree(src_surface.phys_addr, back_ground_vir);

	return CVI_SUCCESS;
}