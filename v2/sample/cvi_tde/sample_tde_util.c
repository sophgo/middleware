#include "cvi_tde.h"
#include "sample_tde_util.h"

CVI_VOID sample_tde_create_surface(cvi_tde_surface *surface, CVI_U32 colorfmt, CVI_U32 w, CVI_U32 h, CVI_U32 stride)
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

CVI_U32 sample_tde_get_pixel_bytes_by_fmt(cvi_tde_color_format format)
{
	CVI_U32 pixel_bytes = 0;

	switch (format) {
	case CVI_TDE_COLOR_FORMAT_A8:
		pixel_bytes = 1;
		break;
	case CVI_TDE_COLOR_FORMAT_ARGB1555:
	case CVI_TDE_COLOR_FORMAT_ABGR1555:
	case CVI_TDE_COLOR_FORMAT_RGB565:
	case CVI_TDE_COLOR_FORMAT_BGR565:
	case CVI_TDE_COLOR_FORMAT_RGBA4444:
	case CVI_TDE_COLOR_FORMAT_BGRA4444:
		pixel_bytes = 2;
		break;
	case CVI_TDE_COLOR_FORMAT_RGB888:
		pixel_bytes = 3;
		break;
	case CVI_TDE_COLOR_FORMAT_RGBA8888:
	case CVI_TDE_COLOR_FORMAT_BGRA8888:
	case CVI_TDE_COLOR_FORMAT_ARGB8888:
	default:
		pixel_bytes = 4;
		break;
	}

	return pixel_bytes;
}


CVI_S32 sample_tde_do_fill_surface(cvi_tde_surface *surface, cvi_tde_rect *rect, CVI_U32 fill_data)
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
