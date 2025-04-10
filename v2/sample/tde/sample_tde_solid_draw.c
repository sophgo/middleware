#include <stdio.h>
#include "cvi_sys.h"
#include "cvi_tde.h"
#include "sample_tde.h"
#include "sample_tde_util.h"

static CVI_S32 sample_tde_do_solid_draw(cvi_tde_surface *src_surface,
                                cvi_tde_surface *dst_surface,
                                cvi_tde_rect *src_rect,
                                cvi_tde_rect *dst_rect,
                                cvi_tde_fill_color *fill_color,
                                cvi_tde_opt *opt){
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
    ret = cvi_tde_solid_draw(handle, &single_src, fill_color, opt);
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

CVI_S32 sample_tde_solid_draw(CVI_VOID)
{
    cvi_tde_rect src_rect;
	cvi_tde_rect dst_rect;

    CVI_U32 surf_fmt = CVI_TDE_COLOR_FORMAT_RGB565;
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

    src_rect.pos_x = 32;
    src_rect.pos_y = 64;
    src_rect.width = 128;
    src_rect.height = 96;

    dst_rect.pos_x = 100;
    dst_rect.pos_y = 50;
    dst_rect.width = src_surface.width / 4;
    dst_rect.height = src_surface.height / 4;

    cvi_tde_rect argc_rect = {0};
    argc_rect.pos_x = 0;
    argc_rect.pos_y = 0;
    argc_rect.width = src_surface.width;
    argc_rect.height = src_surface.height;
    sample_tde_do_fill_surface(&src_surface, &argc_rect, 0xFF000000);
    sample_tde_do_fill_surface(&dst_surface, &argc_rect, 0xFF005500);
    argc_rect.pos_x = 32;
    argc_rect.pos_y = 64;
    argc_rect.width = 32;
    argc_rect.height = 96;
    sample_tde_do_fill_surface(&src_surface, &argc_rect, 0xFF550000);
    argc_rect.pos_x = 64;
    argc_rect.pos_y = 64;
    argc_rect.width = 96;
    argc_rect.height = 32;
    sample_tde_do_fill_surface(&src_surface, &argc_rect, 0xFF555500);
    cvi_save_bmp("solid_draw_src.bmp", back_ground_vir, src_surface.width, src_surface.height,
        src_surface.stride, src_surface.color_format);
    cvi_save_bmp("solid_draw_dst.bmp", back_ground_vir + src_surface.stride * src_surface.height, src_surface.width, src_surface.height,
        src_surface.stride, src_surface.color_format);

    cvi_tde_fill_color fill_color = {0};
    fill_color.color_format = CVI_TDE_COLOR_FORMAT_RGB565;
    fill_color.color_value = 0xFF555555;
    cvi_tde_opt opt = {0};
    opt.alpha_blending_cmd = CVI_TDE_ALPHA_BLENDING_BLEND;
    opt.blend_opt.blend_cmd = CVI_TDE_BLEND_CMD_ADD;
    opt.blend_opt.global_alpha_en = true;
    opt.blend_opt.pixel_alpha_en = true;
    opt.global_alpha = 0xFF;

    opt.mirror = CVI_TDE_MIRROR_BOTH;
    opt.resize = true;

    opt.clip_mode = CVI_TDE_CLIP_MODE_OUTSIDE;
    opt.clip_rect.pos_x = 100;
    opt.clip_rect.pos_y = 50;
    opt.clip_rect.width = 32;
    opt.clip_rect.height = 32;

    if(sample_tde_do_solid_draw(&src_surface, &dst_surface, &src_rect, &dst_rect, &fill_color, &opt) == CVI_SUCCESS){
        printf("sample_solid_draw exec success !\n");
        cvi_save_bmp("solid_draw_finish.bmp", back_ground_vir + src_surface.stride * src_surface.height, dst_surface.width,
            dst_surface.height, dst_surface.stride, dst_surface.color_format);
    }

    CVI_SYS_IonFree(src_surface.phys_addr, back_ground_vir);

    return CVI_SUCCESS;

}