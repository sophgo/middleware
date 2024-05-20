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

/**
 * @brief Open tde device.
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_open(CVI_VOID);

/**
 * @brief Close tde device.
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_VOID cvi_tde_close(CVI_VOID);


/**
 * @brief Begin a TDE job.
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_begin_job(CVI_VOID);

/**
 * @brief End a TDE job.
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_end_job(CVI_S32 handle, CVI_BOOL is_sync, CVI_BOOL is_block, CVI_U32 time_out);

/**
 * @brief Cancel a TDE job.
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_cancel_job(CVI_S32 handle);

/**
 * @brief wait for a TDE job to complete
 *
 * @param[IN] handle, a task handle of TDE
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_wait_the_task_done(CVI_S32 handle);

/**
 * @brief wait for all the TDE jobs to complete
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_wait_all_task_done(CVI_VOID);

/**
 * @brief perfoam a quick fill with fill data
 *
 * @param[IN] handle, a task handle of TDE
 * @param[IN/OUT] none_src, pointer to an none source
 * @param[IN] fill_data, a fill color data
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_quick_fill(CVI_S32 handle, const cvi_tde_none_src *none_src, CVI_U32 fill_data);

/**
 * @brief Draw corner box(es).
 *
 * @param[IN] handle, a task handle of TDE
 * @param[IN/OUT] dst_surface, pointer to destination surface
 * @param[IN] corner_rect, pointer to an array of corner rectangle structure
 * @param[IN] num, the number of corner rectangles
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_draw_corner_box(CVI_S32 handle, const cvi_tde_surface *dst_surface, const cvi_tde_corner_rect *corner_rect,
    CVI_U32 num);

/**
 * @brief Draw line(s).
 *
 * @param[IN] handle, a task handle of TDE
 * @param[IN/OUT] dst_surface, pointer to destination surface
 * @param[IN] line, pointer to an array of line structures
 * @param[IN] num, the number of lines
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_draw_line(CVI_S32 handle, const cvi_tde_surface *dst_surface, const cvi_tde_line *line, CVI_U32 num);

/**
 * @brief Quick copy a rectangular region from a source surface to a destination surface
 *
 * @param[IN] handle, a task handle of TDE
 * @param[IN/OUT] single_src, pointer to an single source
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_quick_copy(CVI_S32 handle, const cvi_tde_single_src *single_src);

/**
 * @brief Quick resize a rectangular region from a source surface to a destination surface
 *
 * @param[IN] handle, a task handle of TDE
 * @param[IN/OUT] single_src, pointer to an single source
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_quick_resize(CVI_S32 handle, const cvi_tde_single_src *single_src);

/**
 * @brief perfoam a solid draw in destination surface with a fiil color
 *
 * @param[IN] handle, a task handle of TDE
 * @param[IN/OUT] single_src, pointer to an single source
 * @param[IN] fill_color, fill color
 * @param[IN] opt, pointer to operation structure
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_solid_draw(CVI_S32 handle, const cvi_tde_single_src *single_src, const cvi_tde_fill_color *fill_color,
    const cvi_tde_opt *opt);

/**
 * @brief rotate a rectangular region from a source surface to a destination surface
 *
 * @param[IN] handle, a task handle of TDE
 * @param[IN/OUT] single_src, pointer to an single source
 * @param[IN] rotate,
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_rotate(CVI_S32 handle, const cvi_tde_single_src *single_src, cvi_tde_rotate_angle rotate);

/**
 * @brief perfoam a bit blit operation with foreground and background surfaces
 *
 * @param[IN] handle, a task handle of TDE
 * @param[IN/OUT] double_src, pointer to an double source
 * @param[IN] opt, pointer to operation structure
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_bit_blit(CVI_S32 handle, const cvi_tde_double_src *double_src, const cvi_tde_opt *opt);

/**
 * @brief perfoam a pattern fill with foreground and background surfaces
 *
 * @param[IN] handle, a task handle of TDE
 * @param[IN/OUT] double_src, pointer to an double source
 * @param[IN] fill_opt, pointer to fill operation structure
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_pattern_fill(CVI_S32 handle, const cvi_tde_double_src *double_src,
    const cvi_tde_pattern_fill_opt *fill_opt);

/**
 * @brief set alpha threshold value
 *
 * @param[IN] threshold_value, threshold value
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_mb_blit(CVI_S32 handle, const cvi_tde_mb_src *mb_src, const cvi_tde_mb_opt *opt);

/**
 * @brief set alpha threshold value
 *
 * @param[IN] threshold_value, threshold value
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_set_alpha_threshold_value(CVI_U8 threshold_value);

/**
 * @brief get alpha threshold value
 *
 * @param[OUT] threshold_value, pointer to threshold value
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_get_alpha_threshold_value(CVI_U8 *threshold_value);

/**
 * @brief set alpha threshold state
 *
 * @param[IN] threshold_en, threshold enable
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_set_alpha_threshold_state(CVI_BOOL threshold_en);

/**
 * @brief get alpha threshold value
 *
 * @param[OUT] threshold_en, pointer to threshold enable
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_tde_get_alpha_threshold_state(CVI_BOOL *threshold_en);

/**
 * @brief save image buffer as a bmp format file
 *
 * @param[IN] image_name, pointer to image file name
 * @param[IN] p, pointer to image buffer
 * @param[IN] width, image width
 * @param[IN] height, image height
 * @param[IN] stride, image stride
 * @param[IN] cvi_tde_format, image color format
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_save_bmp(const CVI_CHAR *image_name, CVI_U8 *p, CVI_U32 width, CVI_U32 height,
				CVI_U32 stride, cvi_tde_color_format cvi_tde_format);

/**
 * @brief save image buffer as a raw format file
 *
 * @param[IN] image_name, pointer to image file name
 * @param[IN] p, pointer to image buffer
 * @param[IN] width, image width
 * @param[IN] height, image height
 * @param[IN] stride, image stride
 * @param[IN] cvi_tde_format, image color format
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 cvi_save_raw(const CVI_CHAR *name, CVI_U8* buffer, CVI_U32 width, CVI_U32 height,
				CVI_U32 stride, cvi_tde_color_format cvi_tde_format);

#ifdef __cplusplus
}
#endif

#endif /* __TDE_API__ */
