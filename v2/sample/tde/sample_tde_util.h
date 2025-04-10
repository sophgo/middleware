#ifndef __SAMPLE_TDE_UTIL_H__
#define __SAMPLE_TDE_UTIL_H__

CVI_VOID sample_tde_create_surface(cvi_tde_surface *surface, CVI_U32 colorfmt, CVI_U32 w, CVI_U32 h, CVI_U32 stride);

CVI_U32 sample_tde_get_pixel_bytes_by_fmt(cvi_tde_color_format format);

CVI_S32 sample_tde_do_fill_surface(cvi_tde_surface *surface, cvi_tde_rect *rect, CVI_U32 fill_data);

CVI_S32 sample_tde_do_draw_corner_box(cvi_tde_surface *dst_surface, cvi_tde_corner_rect *corner_rect, CVI_U32 num);

#endif