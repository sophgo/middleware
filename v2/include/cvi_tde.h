/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2023. All rights reserved.
 *
 * File Name: include/cvi_tde.h
 * Description:
 *   tde interfaces.
 */

#ifndef CVI_TDE_H
#define CVI_TDE_H

#include "cvi_comm_tde.h"


#ifdef __cplusplus
extern "C" {
#endif

/* API Declaration */
CVI_S32 cvi_tde_open(CVI_VOID);
CVI_VOID cvi_tde_close(CVI_VOID);
CVI_S32 cvi_tde_begin_job(CVI_VOID);
CVI_S32 cvi_tde_end_job(CVI_S32 handle, CVI_BOOL is_sync, CVI_BOOL is_block, CVI_U32 time_out);
CVI_S32 cvi_tde_cancel_job(CVI_S32 handle);
CVI_S32 cvi_tde_wait_the_task_done(CVI_S32 handle);
CVI_S32 cvi_tde_wait_all_task_done(CVI_VOID);
CVI_S32 cvi_tde_quick_fill(CVI_S32 handle, const cvi_tde_none_src *none_src, CVI_U32 fill_data);
CVI_S32 cvi_tde_draw_corner_box(CVI_S32 handle, const cvi_tde_surface *dst_surface, const cvi_tde_corner_rect *corner_rect,
    CVI_U32 num);
CVI_S32 cvi_tde_draw_line(CVI_S32 handle, const cvi_tde_surface *dst_surface, const cvi_tde_line *line, CVI_U32 num);
CVI_S32 cvi_tde_quick_copy(CVI_S32 handle, const cvi_tde_single_src *single_src);
CVI_S32 cvi_tde_quick_resize(CVI_S32 handle, const cvi_tde_single_src *single_src);
CVI_S32 cvi_tde_solid_draw(CVI_S32 handle, const cvi_tde_single_src *single_src, const cvi_tde_fill_color *fill_color,
    const cvi_tde_opt *opt);
CVI_S32 cvi_tde_rotate(CVI_S32 handle, const cvi_tde_single_src *single_src, cvi_tde_rotate_angle rotate);
CVI_S32 cvi_tde_bit_blit(CVI_S32 handle, const cvi_tde_double_src *double_src, const cvi_tde_opt *opt);
CVI_S32 cvi_tde_pattern_fill(CVI_S32 handle, const cvi_tde_double_src *double_src,
    const cvi_tde_pattern_fill_opt *fill_opt);
CVI_S32 cvi_tde_mb_blit(CVI_S32 handle, const cvi_tde_mb_src *mb_src, const cvi_tde_mb_opt *opt);
CVI_S32 cvi_tde_set_alpha_threshold_value(CVI_U8 threshold_value);
CVI_S32 cvi_tde_get_alpha_threshold_value(CVI_U8 *threshold_value);
CVI_S32 cvi_tde_set_alpha_threshold_state(CVI_BOOL threshold_en);
CVI_S32 cvi_tde_get_alpha_threshold_state(CVI_BOOL *threshold_en);
CVI_S32 cvi_save_bmp(const CVI_CHAR *image_name, CVI_U8 *p, CVI_U32 width, CVI_U32 height,
            CVI_U32 stride, cvi_tde_color_format cvi_tde_format);
CVI_S32 cvi_save_raw(const CVI_CHAR *name, CVI_U8* buffer, CVI_U32 width, CVI_U32 height,
            CVI_U32 stride, cvi_tde_color_format cvi_tde_format);

#ifdef __cplusplus
}
#endif

#endif /* __TDE_API__ */
