#include <stdio.h>
#include "cvi_sys.h"
#include "cvi_tde.h"
#include "sample_tde.h"
#include "sample_tde_util.h"

static CVI_S32 sample_tde_do_bit_blit(cvi_tde_surface *bg_surface,
                                cvi_tde_surface *fg_surface,
                                cvi_tde_surface *dst_surface,
                                cvi_tde_rect *bg_rect,
                                cvi_tde_rect *fg_rect,
                                cvi_tde_rect *dst_rect,
                                cvi_tde_opt *opt){
    CVI_S32 ret;
    CVI_S32 handle = 0;
    cvi_tde_double_src double_src = {0};

    /* 1. start job */
    handle = cvi_tde_begin_job();
    if (handle == CVI_ERR_TDE_INVALID_HANDLE) {
        return CVI_FAILURE;
    }

    /* 2. do some operations to surface */
    double_src.bg_surface = bg_surface;
    double_src.fg_surface = fg_surface;
    double_src.dst_surface = dst_surface;
    double_src.bg_rect = bg_rect;
    double_src.fg_rect = fg_rect;
    double_src.dst_rect = dst_rect;
    ret = cvi_tde_bit_blit(handle, &double_src, opt);
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

CVI_S32 sample_tde_bit_blit(CVI_VOID){

    cvi_tde_rect bg_rect;
	cvi_tde_rect fg_rect;
    cvi_tde_rect dst_rect;

    CVI_U32 surf_fmt = CVI_TDE_COLOR_FORMAT_RGBA8888;
    cvi_tde_surface bg_surface = {0};
	cvi_tde_surface fg_surface = {0};
    cvi_tde_surface dst_surface = {0};
    CVI_U8 *back_ground_vir = NULL;
	CVI_U32 pixel_bytes = 0;
	CVI_U32 surf_width = 256;
	CVI_U32 surf_height = 256;

    pixel_bytes = sample_tde_get_pixel_bytes_by_fmt(surf_fmt);

    if (CVI_SYS_IonAlloc(&bg_surface.phys_addr, (CVI_VOID**)&back_ground_vir, "sample_tde_bit_blit", surf_width * surf_height * pixel_bytes * 3) != CVI_SUCCESS) {
		return CVI_FAILURE;
	}

    if (back_ground_vir == NULL || bg_surface.phys_addr == 0) {
		return CVI_FAILURE;
	}

    sample_tde_create_surface(&bg_surface, surf_fmt, surf_width, surf_height, surf_width * pixel_bytes);
	sample_tde_create_surface(&fg_surface, surf_fmt, surf_width, surf_height, surf_width * pixel_bytes);
    sample_tde_create_surface(&dst_surface, surf_fmt, surf_width, surf_height, surf_width * pixel_bytes);
    fg_surface.phys_addr = bg_surface.phys_addr + bg_surface.stride * bg_surface.height;
    dst_surface.phys_addr = fg_surface.phys_addr + fg_surface.stride * fg_surface.height;

    bg_rect.pos_x = 32;
    bg_rect.pos_y = 64;
    bg_rect.width = 128;
    bg_rect.height = 96;

    fg_rect.pos_x = 32;
    fg_rect.pos_y = 64;
    fg_rect.width = 128;
    fg_rect.height = 96;

    dst_rect.pos_x = 100;
    dst_rect.pos_y = 50;
    dst_rect.width = 64;
    dst_rect.height = 48;

    cvi_tde_rect argc_rect = {0};
    argc_rect.pos_x = 0;
    argc_rect.pos_y = 0;
    argc_rect.width = bg_surface.width;
    argc_rect.height = bg_surface.height;
    sample_tde_do_fill_surface(&bg_surface, &argc_rect, 0xFFFF0000);
    sample_tde_do_fill_surface(&fg_surface, &argc_rect, 0xFF00FF00);
    sample_tde_do_fill_surface(&dst_surface, &argc_rect, 0xFF0000FF);

    argc_rect.pos_x = 32;
    argc_rect.pos_y = 64;
    argc_rect.width = 32;
    argc_rect.height = 96;
    sample_tde_do_fill_surface(&fg_surface, &argc_rect, 0xFF000000);

    argc_rect.pos_x = 64;
    argc_rect.pos_y = 64;
    argc_rect.width = 96;
    argc_rect.height = 32;
    sample_tde_do_fill_surface(&fg_surface, &argc_rect, 0xFF332233);

    argc_rect.pos_x = 64;
    argc_rect.pos_y = 96;
    argc_rect.width = 96;
    argc_rect.height = 64;
    sample_tde_do_fill_surface(&fg_surface, &argc_rect, 0xFF03AA00);

    argc_rect.pos_x = 64;
    argc_rect.pos_y = 96;
    argc_rect.width = 96;
    argc_rect.height = 64;
    sample_tde_do_fill_surface(&bg_surface, &argc_rect, 0xFF555555);

    cvi_save_bmp("bit_blit_bg.bmp", back_ground_vir, bg_surface.width, bg_surface.height, bg_surface.stride, bg_surface.color_format);

    cvi_save_bmp("bit_blit_fg.bmp", back_ground_vir + bg_surface.stride * bg_surface.height, fg_surface.width, fg_surface.height, fg_surface.stride, fg_surface.color_format);

    unsigned long int offest = bg_surface.stride * bg_surface.height + fg_surface.stride * fg_surface.height;
    cvi_save_bmp("bit_blit_dst.bmp", back_ground_vir + offest, dst_surface.width, dst_surface.height, dst_surface.stride, dst_surface.color_format);

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
    opt.clip_rect.width = 10;
    opt.clip_rect.height = 10;

    opt.colorkey_mode = CVI_TDE_COLORKEY_MODE_FG;
    opt.colorkey_value.argb_colorkey.alpha.is_component_ignore = true;
    opt.colorkey_value.argb_colorkey.red.is_component_ignore = false;
    opt.colorkey_value.argb_colorkey.red.is_component_out = false;
    opt.colorkey_value.argb_colorkey.red.min_component = 0x00;
    opt.colorkey_value.argb_colorkey.red.max_component = 0x00;
    opt.colorkey_value.argb_colorkey.green.is_component_ignore = false;
    opt.colorkey_value.argb_colorkey.green.is_component_out = false;
    opt.colorkey_value.argb_colorkey.green.min_component = 0xAA;
    opt.colorkey_value.argb_colorkey.green.max_component = 0xAA;
    opt.colorkey_value.argb_colorkey.blue.is_component_ignore = false;
    opt.colorkey_value.argb_colorkey.blue.is_component_out = false;
    opt.colorkey_value.argb_colorkey.blue.min_component = 0x03;
    opt.colorkey_value.argb_colorkey.blue.max_component = 0x03;
    if(sample_tde_do_bit_blit(&bg_surface,
                                &fg_surface,
                                &dst_surface,
                                &bg_rect,
                                &fg_rect,
                                &dst_rect,
                                &opt) == CVI_SUCCESS){
        printf("sample_bit_blit exec success !\n");
        cvi_save_bmp("bit_blit_finish.bmp", back_ground_vir + offest, dst_surface.width, dst_surface.height, dst_surface.stride, dst_surface.color_format);
    }

    CVI_SYS_IonFree(bg_surface.phys_addr, back_ground_vir);

    return CVI_SUCCESS;
}