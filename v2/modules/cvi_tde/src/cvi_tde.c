#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "cvi_tde.h"
#include "vglite/vg_lite.h"
#include "vglite/vg_lite_util.h"

#define UNUSED(x) (void)(x)
#define CVI_TDE_ERR(fmt, ...)   printf("[ERR]" fmt, ##__VA_ARGS__)
#define CVI_TDE_WRN(fmt, ...)   printf("[WRN]" fmt, ##__VA_ARGS__)
#define CVI_TDE_INFO(fmt, ...)   printf("[INFO]" fmt, ##__VA_ARGS__)
#define CVI_TDE_DBG(fmt, ...)
/*Required alignment size for buffers*/
#define CVI_TED_ATTRIBUTE_MEM_ALIGN_SIZE 1
/* Stride required by VG-Lite HW. Don't change this. */
#define CVI_TDE_STRIDE_ALIGN 16U

#define VG_LITE_RETURN_INV(fmt, ...)            \
    do {                                        \
        CVI_TDE_ERR(fmt, ##__VA_ARGS__);             \
        return -1;                              \
    } while(0)


char* error_type[] =
{
    "VG_LITE_SUCCESS",
    "VG_LITE_INVALID_ARGUMENT",
    "VG_LITE_OUT_OF_MEMORY",
    "VG_LITE_NO_CONTEXT",
    "VG_LITE_TIMEOUT",
    "VG_LITE_OUT_OF_RESOURCES",
    "VG_LITE_GENERIC_IO",
    "VG_LITE_NOT_SUPPORT",
};

char* cvi_error_type[] = {
    "CVI_ERR_TDE_DEV_NOT_OPEN",
    "CVI_ERR_TDE_DEV_OPEN_FAILED",  
    "CVI_ERR_TDE_NULL_PTR",   
    "CVI_ERR_TDE_NO_MEM",
    "CVI_ERR_TDE_INVALID_HANDLE",
    "CVI_ERR_TDE_INVALID_PARAM",
    "CVI_ERR_TDE_NOT_ALIGNED",
    "CVI_ERR_TDE_MINIFICATION",
    "CVI_ERR_TDE_CLIP_AREA",
    "CVI_ERR_TDE_JOB_TIMEOUT",
    "CVI_ERR_TDE_UNSUPPORTED_OPERATION",
    "CVI_ERR_TDE_QUERY_TIMEOUT",
    "CVI_ERR_TDE_INTERRUPT",
    "CVI_ERR_TDE_BUTT",
    "CVI_ERR_TDE_FAILURE",
};

#define cvi_errcode_index(X) X - CVI_ERR_TDE_BASE
#define __func__ __FUNCTION__
#define IS_ERROR(status)         (status > 0)
#define CHECK_ERROR(Function)   \
    error = Function;           \
    if (IS_ERROR(error))        \
    {                           \
        CVI_TDE_ERR("[%s: %d] failed.error type is %s\n", __func__, __LINE__, error_type[error]); \
        rc = CVI_FAILURE;       \
        goto ErrorHandler;      \
    }

#define CVI_MIN(A,B) (A < B ? A : B)
#define CVI_MAX(A,B) (A > B ? A : B)

#define print_cvi_error_message(vg_err, cvi_error_message)\
        if(vg_err != VG_LITE_SUCCESS){\
            vg_lite_error_to_cvi_tde_error(vg_err, &cvi_error_message);\
            CVI_TDE_ERR("[%s:%d] throw the error,error type is %s\n",__func__, __LINE__, cvi_error_type[cvi_error_message.message_index]);\
            return cvi_error_message.cvi_error_code;\
        }
#define colorkey_alpha_set_zeros(colorkey) \
        colorkey[0].alpha = 0x00;\
        colorkey[1].alpha = 0x00;\
        colorkey[2].alpha = 0x00;\
        colorkey[3].alpha = 0x00

#define enable_colorkey(colorkey) \
        colorkey[0].enable = 1;\
        colorkey[1].enable = 1;\
        colorkey[2].enable = 1;\
        colorkey[3].enable = 1

#define unenable_colorkey(colorkey) \
        colorkey[0].enable = 0;\
        colorkey[1].enable = 0;\
        colorkey[2].enable = 0;\
        colorkey[3].enable = 0

#define clear_two_bufs(buf1, buf2)\
        vg_lite_free(&buf1); \
        vg_lite_free(&buf2)

#define program_test 1
#define colorkey_bug_resolve 1

typedef enum color_format_case
{
    RGB = 0x00,
    ARGB,
} color_format_case_t;

typedef struct error_message{
    int message_index;
    CVI_U32 cvi_error_code;
}error_message_t;

static vg_lite_buffer_format_t cvi_tde_format_to_vglite_format(cvi_tde_color_format format);
static vg_lite_blend_t cvi_tde_blend_to_vglite_blend(cvi_tde_blend_cmd blend);
static CVI_S32 cvi_draw_single_line(vg_lite_buffer_t* fb, const cvi_tde_line* line);
static color_format_case_t get_color_format_case(vg_lite_buffer_format_t format);
static void insert_sort(CVI_S32 arr[], int len);
static void vg_lite_error_to_cvi_tde_error(vg_lite_error_t vg_lite_error, error_message_t* cvi_error_message);
static CVI_U32 get_alpha_mode_val(vg_lite_buffer_format_t format,
                                  const cvi_tde_opt* opt,
                                  vg_lite_global_alpha_t* mode,
                                  unsigned char* global_alpha);

static vg_lite_rectangle_t *rectangle_intersection(const cvi_tde_rect* dst_rect,
                                                   const cvi_tde_rect* clip_rect,
                                                   vg_lite_rectangle_t* rect0,
                                                   bool* intersect);
static CVI_U32 ver_mirror(vg_lite_buffer_t* source_image, vg_lite_rectangle_t* src_rect);
static CVI_U32 hor_mirror(vg_lite_buffer_t* source_image, vg_lite_rectangle_t* src_rect);
static CVI_U32 clip_outside(vg_lite_buffer_t* dst_buf,
                            const cvi_tde_rect* dst_rect,
                            vg_lite_buffer_t* src_buf,
                            const cvi_tde_rect* src_rect,
                            const cvi_tde_rect* clip_rect,
                            vg_lite_matrix_t* matrix);
static CVI_U32 clip_inside(vg_lite_buffer_t* dst_buf,
                           const cvi_tde_rect* dst_rect,
                           vg_lite_buffer_t* src_buf,
                            const cvi_tde_rect* src_rect,
                            const cvi_tde_rect* clip_rect,
                            vg_lite_matrix_t* matrix);
static CVI_U32 init_vg_middle_buf(vg_lite_buffer_t* mid_buf,
                                  vg_lite_buffer_t* src_buf,
                                  vg_lite_rectangle_t* src_rect);
static CVI_U32 alpha_blending(vg_lite_buffer_t* bg_buf,
                              vg_lite_buffer_t* fg_buf,
                              const cvi_tde_opt* opt);
static void compute_resize_matrix(vg_lite_matrix_t* matrix,
                                  vg_lite_rectangle_t* src_rect,
                                  vg_lite_rectangle_t* dst_rect);
#if (program_test == 1)
static void get_colorkey_value_to_vg_lite(vg_lite_color_key4_t colorkey, const cvi_tde_opt* opt);
#endif
CVI_S32 cvi_tde_open(CVI_VOID)
{
    return vg_lite_init(1024, 1024);
}

CVI_VOID cvi_tde_close(CVI_VOID)
{
    vg_lite_close();
}

CVI_S32 cvi_tde_begin_job(CVI_VOID)
{
    return CVI_SUCCESS;
}

CVI_S32 cvi_tde_end_job(CVI_S32 handle, CVI_BOOL is_sync, CVI_BOOL is_block, CVI_U32 time_out)
{
	UNUSED(handle);
	UNUSED(is_sync);
	UNUSED(time_out);

	if (is_block) {
		vg_lite_finish();
	} else {
		vg_lite_flush();
	}

	return CVI_SUCCESS;
}

CVI_S32 cvi_tde_cancel_job(CVI_S32 handle)
{
	UNUSED(handle);
	return CVI_SUCCESS;
}

CVI_S32 cvi_tde_wait_the_task_done(CVI_S32 handle)
{
	UNUSED(handle);
	return CVI_SUCCESS;
}

CVI_S32 cvi_tde_wait_all_task_done(CVI_VOID)
{
	vg_lite_finish();

	return CVI_SUCCESS;
}



CVI_S32 cvi_vglite_init_buf(vg_lite_buffer_t * vgbuf, uint32_t width, uint32_t height, uint32_t stride,
                           vg_lite_buffer_format_t fmt , const uint8_t * ptr, bool source)
{
    /*Test for memory alignment*/
    if((((uintptr_t)ptr) % (uintptr_t)CVI_TED_ATTRIBUTE_MEM_ALIGN_SIZE) != (uintptr_t)0x0U)
        VG_LITE_RETURN_INV("%s buffer (0x%lx) not aligned to %d.", source ? "Src" : "Dest",
                           (size_t) ptr, CVI_TED_ATTRIBUTE_MEM_ALIGN_SIZE);

    /*Test for stride alignment*/
    if(source && (stride % (CVI_TDE_STRIDE_ALIGN * sizeof(uint8_t))) != 0x0U)
        VG_LITE_RETURN_INV("Src buffer stride (%u bytes) not aligned to %lu bytes.", stride,
                           CVI_TDE_STRIDE_ALIGN * sizeof(uint8_t));

    vgbuf->format = fmt;
    vgbuf->tiled = VG_LITE_LINEAR;
    vgbuf->image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    vgbuf->transparency_mode = VG_LITE_IMAGE_OPAQUE;

    vgbuf->width = (int32_t)width;
    vgbuf->height = (int32_t)height;
    vgbuf->stride = (int32_t)stride;

    memset(&vgbuf->yuv, 0, sizeof(vgbuf->yuv));

    vgbuf->memory = (vg_lite_pointer)ptr;
    vgbuf->address = (vg_lite_uint32_t)(vg_lite_uint64_t)vgbuf->memory;
    vgbuf->handle = NULL;

    return VG_LITE_SUCCESS;
}

CVI_S32 cvi_tde_quick_fill(CVI_S32 handle, const cvi_tde_none_src *none_src, CVI_U32 fill_data)
{
    CVI_U32 dest_width = 0;
    CVI_U32 dest_height = 0;
    CVI_U32 dest_stride = 0;
    CVI_U8 *dest_buf = NULL;
    vg_lite_buffer_t vgbuf = { 0 };
    vg_lite_rectangle_t rect;
    vg_lite_error_t err = VG_LITE_SUCCESS;
    vg_lite_buffer_format_t format = 0;
    cvi_tde_rect *dst_rect = NULL;
    cvi_tde_surface *dst_surface = NULL;

    if (handle < 0 || none_src == NULL) {
        return CVI_FAILURE;
    }

    dst_rect = none_src->dst_rect;
    dst_surface = none_src->dst_surface;
    if (dst_rect == NULL || dst_surface == NULL) {
        return CVI_FAILURE;
    }

    dest_width = dst_surface->width;
    dest_height = dst_surface->height;
    dest_stride = dst_surface->stride;
    dest_buf = (CVI_U8 *)(CVI_U64)dst_surface->phys_addr;
    format = cvi_tde_format_to_vglite_format(dst_surface->color_format);

    if(cvi_vglite_init_buf(&vgbuf, dest_width, dest_height, dest_stride, format, dest_buf, CVI_FALSE) != CVI_SUCCESS) {
        VG_LITE_RETURN_INV("Init buffer failed.");
    }

    rect.x = dst_rect->pos_x;
    rect.y = dst_rect->pos_y;
    rect.width = dst_rect->width;
    rect.height = dst_rect->height;
    err = vg_lite_clear(&vgbuf, &rect, fill_data);
    if (err != VG_LITE_SUCCESS) {
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

static CVI_S32 cvi_tde_draw_single_corner_box(CVI_S32 handle,
								const cvi_tde_surface *dst_surface,
								const cvi_tde_corner_rect *corner_rect)
{
	CVI_U32 dest_width = 0;
	CVI_U32 dest_height = 0;
	CVI_U32 dest_stride = 0;
	CVI_U8 *dest_buf = NULL;
	vg_lite_buffer_t vgbuf = { 0 };
	vg_lite_buffer_format_t format = 0;

	CVI_S32 rect_pos_x = 0;
	CVI_S32 rect_pos_y = 0;
	CVI_S32 rect_width = 0;
	CVI_S32 rect_height = 0;
	CVI_U32 in_color = 0;
	CVI_U32 out_color = 0;
	CVI_S32 ref_w = 0;
	CVI_U32 thick_h = 0;
	cvi_tde_line box_lines[9] = {{0,0,0,0,0,0}};

	if (handle < 0
		|| dst_surface == NULL
		|| corner_rect == NULL) {
		return CVI_FAILURE;
	}

	dest_width = dst_surface->width;
	dest_height = dst_surface->height;
	dest_stride = dst_surface->stride;
	dest_buf = (CVI_U8 *)(CVI_U64)dst_surface->phys_addr;
	format = cvi_tde_format_to_vglite_format(dst_surface->color_format);

	rect_pos_x = corner_rect->corner_rect_region->pos_x;
	rect_pos_y = corner_rect->corner_rect_region->pos_y;
	rect_width = corner_rect->corner_rect_region->width;
	rect_height = corner_rect->corner_rect_region->height;
	in_color = corner_rect->corner_rect_info->inner_color;
	out_color = corner_rect->corner_rect_info->outer_color;
	ref_w = corner_rect->corner_rect_info->width;
	thick_h = corner_rect->corner_rect_info->height;

	if (rect_pos_x <= 0 && rect_pos_y <= 0
		&& (rect_pos_x + rect_width) <= (CVI_S32)dest_width && (rect_pos_y + rect_height) <= (CVI_S32)dest_height) {
		rect_width = rect_pos_x + rect_width;
		rect_height = rect_pos_y + rect_height;
		rect_pos_x = 0;
		rect_pos_y = 0;
		box_lines[0].start_x = rect_pos_x + rect_width / 2.0;
		box_lines[0].start_y = rect_pos_y;
		box_lines[0].end_x = rect_pos_x + rect_width / 2.0;
		box_lines[0].end_y = rect_pos_y + rect_height;
		box_lines[0].thick = rect_width;
		box_lines[0].color = out_color;
	} else if (rect_pos_x <= 0 && rect_pos_y <= 0
		&& (rect_pos_x + rect_width) >= (CVI_S32)dest_width && (rect_pos_y + rect_height) >= (CVI_S32)dest_height) {
		rect_width = dest_width;
		rect_height = dest_height;
		rect_pos_x = 0;
		rect_pos_y = 0;
		box_lines[0].start_x = rect_pos_x + rect_width / 2.0;
		box_lines[0].start_y = rect_pos_y;
		box_lines[0].end_x = rect_pos_x + rect_width / 2.0;
		box_lines[0].end_y = rect_pos_y + rect_height;
		box_lines[0].thick = rect_width;
		box_lines[0].color = out_color;
	} else if (rect_pos_x <= 0 && rect_pos_y <= 0
		&& (rect_pos_x + rect_width) >= (CVI_S32)dest_width && (rect_pos_y + rect_height) <= (CVI_S32)dest_height) {
		rect_width = dest_width;
		rect_height = rect_pos_y + rect_height;
		rect_pos_x = 0;
		rect_pos_y = 0;
		box_lines[0].start_x = rect_pos_x + rect_width / 2.0;
		box_lines[0].start_y = rect_pos_y;
		box_lines[0].end_x = rect_pos_x + rect_width / 2.0;
		box_lines[0].end_y = rect_pos_y + rect_height;
		box_lines[0].thick = rect_width;
		box_lines[0].color = out_color;
	} else if (rect_pos_x <= 0 && rect_pos_y <= 0
		&& (rect_pos_x + rect_width) <= (CVI_S32)dest_width && (rect_pos_y + rect_height) >= (CVI_S32)dest_height) {
		rect_width = rect_pos_x + rect_width;
		rect_height = dest_height;
		rect_pos_x = 0;
		rect_pos_y = 0;
		box_lines[0].start_x = rect_pos_x + rect_width / 2.0;
		box_lines[0].start_y = rect_pos_y;
		box_lines[0].end_x = rect_pos_x + rect_width / 2.0;
		box_lines[0].end_y = rect_pos_y + rect_height;
		box_lines[0].thick = rect_width;
		box_lines[0].color = out_color;
	} else if (rect_pos_x > 0 && rect_pos_y < 0
		&& (rect_pos_x + rect_width) <= (CVI_S32)dest_width && (rect_pos_y + rect_height) <= (CVI_S32)dest_height) {
		rect_height = rect_pos_y + rect_height;
		rect_pos_y = 0;
		box_lines[0].start_x = rect_pos_x;
		box_lines[0].start_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].end_x = rect_pos_x + rect_width;
		box_lines[0].end_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].thick = rect_height;
		box_lines[0].color = out_color;
	} else if (rect_pos_x >0 && rect_pos_y < 0
		&& (rect_pos_x + rect_width) > (CVI_S32)dest_width && (rect_pos_y + rect_height) > (CVI_S32)dest_height) {
		rect_width = dest_width - rect_pos_x;
		rect_height = dest_height;
		rect_pos_y = 0;
		box_lines[0].start_x = rect_pos_x;
		box_lines[0].start_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].end_x = rect_pos_x + rect_width;
		box_lines[0].end_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].thick = rect_height;
		box_lines[0].color = out_color;
	} else if (rect_pos_x >0 && rect_pos_y < 0
		&& (rect_pos_x + rect_width) >= (CVI_S32)dest_width && (rect_pos_y + rect_height) <= (CVI_S32)dest_height) {
		rect_width = dest_width - rect_pos_x;
		rect_height = rect_pos_y + rect_height;
		rect_pos_y = 0;
		box_lines[0].start_x = rect_pos_x;
		box_lines[0].start_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].end_x = rect_pos_x + rect_width;
		box_lines[0].end_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].thick = rect_height;
		box_lines[0].color = out_color;
	} else if (rect_pos_x >0 && rect_pos_y < 0
		&& (rect_pos_x + rect_width) <= (CVI_S32)dest_width && (rect_pos_y + rect_height) >= (CVI_S32)dest_height) {
		rect_height = dest_height;
		rect_pos_y = 0;
		box_lines[0].start_x = rect_pos_x;
		box_lines[0].start_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].end_x = rect_pos_x + rect_width;
		box_lines[0].end_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].thick = rect_height;
		box_lines[0].color = out_color;
	} else if (rect_pos_x >= 0 && rect_pos_x <= (CVI_S32)dest_width && rect_pos_y >= 0
		&& rect_pos_y <= (CVI_S32)dest_height
		&& (rect_pos_x + rect_width) <= (CVI_S32)dest_width && (rect_pos_y + rect_height) <= (CVI_S32)dest_height) {
		box_lines[0].start_x = rect_pos_x;
		box_lines[0].start_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].end_x = rect_pos_x + rect_width;
		box_lines[0].end_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].thick = rect_height;
		box_lines[0].color = out_color;
	} else if (rect_pos_x >= 0 && rect_pos_x <= (CVI_S32)dest_width && rect_pos_y >= 0
		&& rect_pos_y <= (CVI_S32)dest_height && (rect_pos_x + rect_width) > (CVI_S32)dest_width
		&& (rect_pos_y + rect_height) > (CVI_S32)dest_height) {
		rect_width = dest_width - rect_pos_x;
		rect_height = dest_height - rect_pos_y;
		box_lines[0].start_x = rect_pos_x;
		box_lines[0].start_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].end_x = rect_pos_x + rect_width;
		box_lines[0].end_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].thick = rect_height;
		box_lines[0].color = out_color;
	} else if (rect_pos_x >= 0 && rect_pos_x <= (CVI_S32)dest_width && rect_pos_y >= 0
		&& rect_pos_y <= (CVI_S32)dest_height
		&& (rect_pos_x + rect_width) > (CVI_S32)dest_width && (rect_pos_y + rect_height) < (CVI_S32)dest_height) {
		rect_width = dest_width - rect_pos_x;
		box_lines[0].start_x = rect_pos_x;
		box_lines[0].start_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].end_x = rect_pos_x + rect_width;
		box_lines[0].end_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].thick = rect_height;
		box_lines[0].color = out_color;
	} else if (rect_pos_x >= 0 && rect_pos_x <= (CVI_S32)dest_width && rect_pos_y >= 0
		&& rect_pos_y <= (CVI_S32)dest_height
		&& (rect_pos_x + rect_width) < (CVI_S32)dest_width && (rect_pos_y + rect_height) > (CVI_S32)dest_height) {
		rect_height = dest_height - rect_pos_y;
		box_lines[0].start_x = rect_pos_x;
		box_lines[0].start_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].end_x = rect_pos_x + rect_width;
		box_lines[0].end_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].thick = rect_height;
		box_lines[0].color = out_color;
	} else if (rect_pos_x < 0 && rect_pos_y > 0
		&& (rect_pos_x + rect_width) <= (CVI_S32)dest_width && (rect_pos_y + rect_height) <= (CVI_S32)dest_height) {
		rect_width = rect_pos_x + rect_width;
		rect_pos_x = 0;
		box_lines[0].start_x = rect_pos_x;
		box_lines[0].start_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].end_x = rect_pos_x + rect_width;
		box_lines[0].end_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].thick = rect_height;
		box_lines[0].color = out_color;
	} else if (rect_pos_x < 0 && rect_pos_y > 0
		&& (rect_pos_x + rect_width) > (CVI_S32)dest_width && (rect_pos_y + rect_height) > (CVI_S32)dest_height) {
		rect_width = dest_width;
		rect_height = dest_height - rect_pos_y;
		rect_pos_x = 0;
		box_lines[0].start_x = rect_pos_x;
		box_lines[0].start_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].end_x = rect_pos_x + rect_width;
		box_lines[0].end_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].thick = rect_height;
		box_lines[0].color = out_color;
	} else if (rect_pos_x < 0 && rect_pos_y > 0
		&& (rect_pos_x + rect_width) > (CVI_S32)dest_width && (rect_pos_y + rect_height) < (CVI_S32)dest_height) {
		rect_width = dest_width;
		rect_pos_x = 0;
		box_lines[0].start_x = rect_pos_x;
		box_lines[0].start_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].end_x = rect_pos_x + rect_width;
		box_lines[0].end_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].thick = rect_height;
		box_lines[0].color = out_color;
	} else if (rect_pos_x < 0 && rect_pos_y > 0
		&& (rect_pos_x + rect_width) < (CVI_S32)dest_width && (rect_pos_y + rect_height) > (CVI_S32)dest_height) {
		rect_width = rect_pos_x + rect_width;
		rect_height = dest_height - rect_pos_y;
		rect_pos_x = 0;
		box_lines[0].start_x = rect_pos_x;
		box_lines[0].start_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].end_x = rect_pos_x + rect_width;
		box_lines[0].end_y = rect_pos_y + rect_height / 2.0;
		box_lines[0].thick = rect_height;
		box_lines[0].color = out_color;
	} else {
		// do nothing
	}
	box_lines[1].start_x = rect_pos_x + (thick_h / 2.0);
	box_lines[1].start_y = rect_pos_y;
	box_lines[1].end_x = rect_pos_x + (thick_h / 2.0);
	box_lines[1].end_y = rect_pos_y + ref_w;
	box_lines[1].thick = thick_h;
	box_lines[1].color = in_color;

	box_lines[2].start_x = rect_pos_x;
	box_lines[2].start_y = rect_pos_y + (thick_h / 2.0);
	box_lines[2].end_x = rect_pos_x + ref_w;
	box_lines[2].end_y = rect_pos_y + (thick_h / 2.0);
	box_lines[2].thick = thick_h;
	box_lines[2].color = in_color;

	box_lines[3].start_x = rect_pos_x + rect_width;
	box_lines[3].start_y = rect_pos_y + (thick_h / 2.0);
	box_lines[3].end_x = rect_pos_x + rect_width - ref_w;
	box_lines[3].end_y = rect_pos_y + (thick_h / 2.0);
	box_lines[3].thick = thick_h;
	box_lines[3].color = in_color;

	box_lines[4].start_x = rect_pos_x + rect_width - (thick_h / 2.0);
	box_lines[4].start_y = rect_pos_y;
	box_lines[4].end_x = rect_pos_x + rect_width - (thick_h / 2.0);
	box_lines[4].end_y = rect_pos_y + ref_w;
	box_lines[4].thick = thick_h;
	box_lines[4].color = in_color;

	box_lines[5].start_x = rect_pos_x + rect_width - (thick_h / 2.0);
	box_lines[5].start_y = rect_pos_y + rect_height;
	box_lines[5].end_x = rect_pos_x + rect_width - (thick_h / 2.0);
	box_lines[5].end_y = rect_pos_y + rect_height - ref_w;
	box_lines[5].thick = thick_h;
	box_lines[5].color = in_color;

	box_lines[6].start_x = rect_pos_x + rect_width;
	box_lines[6].start_y = rect_pos_y + rect_height - (thick_h / 2.0);
	box_lines[6].end_x = rect_pos_x + rect_width - ref_w;
	box_lines[6].end_y = rect_pos_y + rect_height - (thick_h / 2.0);
	box_lines[6].thick = thick_h;
	box_lines[6].color = in_color;

	box_lines[7].start_x = rect_pos_x;
	box_lines[7].start_y = rect_pos_y + rect_height - (thick_h / 2.0);
	box_lines[7].end_x = rect_pos_x + ref_w;
	box_lines[7].end_y = rect_pos_y + rect_height - (thick_h / 2.0);
	box_lines[7].thick = thick_h;
	box_lines[7].color = in_color;

	box_lines[8].start_x = rect_pos_x + (thick_h / 2.0);
	box_lines[8].start_y = rect_pos_y + rect_height;
	box_lines[8].end_x = rect_pos_x + (thick_h / 2.0);
	box_lines[8].end_y = rect_pos_y + rect_height - ref_w;
	box_lines[8].thick = thick_h;
	box_lines[8].color = in_color;

	if(cvi_vglite_init_buf(&vgbuf, dest_width, dest_height, dest_stride, format, dest_buf, CVI_FALSE) != CVI_SUCCESS) {
	    VG_LITE_RETURN_INV("Init buffer failed.");
	}

	// FIXME: hard-coded number
	for (CVI_U32 i = 0; i < (2 * 4 + 1); i++) {
	    cvi_draw_single_line(&vgbuf, &box_lines[i]);
	}

	return CVI_SUCCESS;
}

CVI_S32 cvi_tde_draw_corner_box(CVI_S32 handle,
                                const cvi_tde_surface *dst_surface,
                                const cvi_tde_corner_rect *corner_rect,
                                CVI_U32 num)
{

	if (num <= 0 || num >= 256) {
		return CVI_ERR_TDE_INVALID_PARAM;
	}

	for (CVI_U32 i = 0; i < num; i++) {
		cvi_tde_draw_single_corner_box(handle, dst_surface, &corner_rect[i]);
	}

	return CVI_SUCCESS;
}

CVI_S32 cvi_tde_draw_line(CVI_S32 handle, const cvi_tde_surface *dst_surface, const cvi_tde_line *line, CVI_U32 num)
{
    UNUSED(handle);
    UNUSED(dst_surface);
    UNUSED(line);
    UNUSED(num);

    CVI_U32 dest_width = 0;
    CVI_U32 dest_height = 0;
    CVI_U32 dest_stride = 0;
    CVI_U8 *dest_buf = NULL;
    vg_lite_buffer_t vgbuf = { 0 };
    vg_lite_buffer_format_t format = 0;
    // vg_lite_error_t err = VG_LITE_SUCCESS;

    if (handle < 0
        || dst_surface == NULL
        || line == NULL
        || num == 0) {
        return CVI_FAILURE;
    }

    dest_width = dst_surface->width;
    dest_height = dst_surface->height;
    dest_stride = dst_surface->stride;
    dest_buf = (CVI_U8 *)(CVI_U64)dst_surface->phys_addr;
    format = cvi_tde_format_to_vglite_format(dst_surface->color_format);

    if(cvi_vglite_init_buf(&vgbuf, dest_width, dest_height, dest_stride, format, dest_buf, CVI_FALSE) != CVI_SUCCESS) {
        VG_LITE_RETURN_INV("Init buffer failed.");
    }

    for (CVI_U32 i = 0; i < num; i++) {
        cvi_draw_single_line(&vgbuf, &line[i]);
    }

    return 0;
}

CVI_S32 cvi_tde_quick_copy(CVI_S32 handle, const cvi_tde_single_src *single_src)
{

    cvi_tde_rect *src_rect = NULL;
    cvi_tde_rect *dst_rect = NULL;
    cvi_tde_surface *src_surface = NULL;
    cvi_tde_surface *dst_surface = NULL;

    CVI_U32 vgbuf_width = 0;
    CVI_U32 vgbuf_height = 0;
    CVI_U32 vgbuf_stride = 0;
    CVI_U8 *vgbuf_address = NULL;

    vg_lite_matrix_t matrix;
    vg_lite_float_t scale_x, scale_y;
    vg_lite_float_t delta_x, delta_y;
    vg_lite_error_t err = VG_LITE_SUCCESS;
    vg_lite_buffer_format_t format = 0;
    vg_lite_rectangle_t rect = { 0 };
    vg_lite_buffer_t src_buf = { 0 };
    vg_lite_buffer_t dst_buf = { 0 };

    if (handle < 0
        || single_src == NULL) {
        return CVI_FAILURE;
    }

    src_rect = single_src->src_rect;
    dst_rect = single_src->dst_rect;
    src_surface = single_src->src_surface;
    dst_surface = single_src->dst_surface;

    if (src_rect == NULL
        || dst_rect == NULL
        || src_surface == NULL
        || dst_surface == NULL) {
        return CVI_FAILURE;
    }

    vgbuf_width = src_surface->width;
    vgbuf_height = src_surface->height;
    vgbuf_stride = src_surface->stride;
    vgbuf_address = (CVI_U8 *)(CVI_U64)src_surface->phys_addr;
    format = cvi_tde_format_to_vglite_format(src_surface->color_format);

    if(cvi_vglite_init_buf(&src_buf, vgbuf_width, vgbuf_height, vgbuf_stride, format, vgbuf_address, CVI_FALSE) != CVI_SUCCESS) {
        VG_LITE_RETURN_INV("Init buffer failed.");
    }

    vgbuf_width = dst_surface->width;
    vgbuf_height = dst_surface->height;
    vgbuf_stride = dst_surface->stride;
    vgbuf_address = (CVI_U8 *)(CVI_U64)dst_surface->phys_addr;
    format = cvi_tde_format_to_vglite_format(dst_surface->color_format);

    if(cvi_vglite_init_buf(&dst_buf, vgbuf_width, vgbuf_height, vgbuf_stride, format, vgbuf_address, CVI_FALSE) != CVI_SUCCESS) {
        VG_LITE_RETURN_INV("Init buffer failed.");
    }

    rect.x = src_rect->pos_x;
    rect.y = src_rect->pos_y;
    rect.width = src_rect->width;
    rect.height = src_rect->height;
    CVI_TDE_DBG("rect :%d, %d, %d, %d\n", rect.x, rect.y, rect.width, rect.height);

    vg_lite_identity(&matrix);

    scale_x = dst_rect->width * 1.0f / src_rect->width;
    scale_y = dst_rect->height * 1.0f / src_rect->height;
    CVI_TDE_DBG("scale_x:%f, scale_y:%f\n", scale_x, scale_y);
    vg_lite_scale(scale_x, scale_y, &matrix);

    delta_x = dst_rect->pos_x - src_rect->pos_x;
    delta_y = dst_rect->pos_y - src_rect->pos_y;
    CVI_TDE_DBG("delta_x:%f, delta_y:%f\n", delta_x, delta_y);
    vg_lite_translate(delta_x, delta_y, &matrix);

    err = vg_lite_blit_rect(&dst_buf, &src_buf, &rect, &matrix,VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
    if (err != VG_LITE_SUCCESS) {
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

CVI_S32 cvi_tde_quick_resize(CVI_S32 handle, const cvi_tde_single_src *single_src)
{
    cvi_tde_rect *src_rect = NULL;
    cvi_tde_rect *dst_rect = NULL;
    cvi_tde_surface *src_surface = NULL;
    cvi_tde_surface *dst_surface = NULL;

    CVI_U32 vgbuf_width = 0;
    CVI_U32 vgbuf_height = 0;
    CVI_U32 vgbuf_stride = 0;
    CVI_U8 *vgbuf_address = NULL;

    vg_lite_matrix_t matrix;
    vg_lite_float_t scale_x, scale_y;
    vg_lite_float_t delta_x, delta_y;
    vg_lite_error_t err = VG_LITE_SUCCESS;
    vg_lite_buffer_format_t format = 0;
    vg_lite_rectangle_t rect = { 0 };
    vg_lite_buffer_t src_buf = { 0 };
    vg_lite_buffer_t dst_buf = { 0 };

    if (handle < 0
        || single_src == NULL) {
        return CVI_FAILURE;
    }

    src_rect = single_src->src_rect;
    dst_rect = single_src->dst_rect;
    src_surface = single_src->src_surface;
    dst_surface = single_src->dst_surface;

    if (src_rect == NULL
        || dst_rect == NULL
        || src_surface == NULL
        || dst_surface == NULL) {
        return CVI_FAILURE;
    }

    vgbuf_width = src_surface->width;
    vgbuf_height = src_surface->height;
    vgbuf_stride = src_surface->stride;
    vgbuf_address = (CVI_U8 *)(CVI_U64)src_surface->phys_addr;
    format = cvi_tde_format_to_vglite_format(src_surface->color_format);

    if(cvi_vglite_init_buf(&src_buf, vgbuf_width, vgbuf_height, vgbuf_stride, format, vgbuf_address, CVI_FALSE) != CVI_SUCCESS) {
        VG_LITE_RETURN_INV("Init buffer failed.");
    }

    vgbuf_width = dst_surface->width;
    vgbuf_height = dst_surface->height;
    vgbuf_stride = dst_surface->stride;
    vgbuf_address = (CVI_U8 *)(CVI_U64)dst_surface->phys_addr;
    format = cvi_tde_format_to_vglite_format(dst_surface->color_format);

    if(cvi_vglite_init_buf(&dst_buf, vgbuf_width, vgbuf_height, vgbuf_stride, format, vgbuf_address, CVI_FALSE) != CVI_SUCCESS) {
        VG_LITE_RETURN_INV("Init buffer failed.");
    }

    rect.x = src_rect->pos_x;
    rect.y = src_rect->pos_y;
    rect.width = src_rect->width;
    rect.height = src_rect->height;
    CVI_TDE_DBG("rect :%d, %d, %d, %d\n", rect.x, rect.y, rect.width, rect.height);

    vg_lite_identity(&matrix);

    scale_x = dst_rect->width * 1.0f / src_rect->width;
    scale_y = dst_rect->height * 1.0f / src_rect->height;
    CVI_TDE_DBG("scale_x:%f, scale_y:%f\n", scale_x, scale_y);
    vg_lite_scale(scale_x, scale_y, &matrix);

    delta_x = dst_rect->pos_x - src_rect->pos_x;
    delta_y = dst_rect->pos_y - src_rect->pos_y;
    CVI_TDE_DBG("delta_x:%f, delta_y:%f\n", delta_x, delta_y);
    vg_lite_translate(delta_x, delta_y, &matrix);

    err = vg_lite_blit_rect(&dst_buf, &src_buf, &rect, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
    if (err != VG_LITE_SUCCESS) {
        CVI_TDE_ERR("[%s:%d]\n", __FUNCTION__, __LINE__);
        return CVI_FAILURE;
    }
    CVI_TDE_DBG("[%s:%d]\n", __FUNCTION__, __LINE__);
    return 0;
}



CVI_S32 cvi_tde_solid_draw(
	CVI_S32 handle,
	const cvi_tde_single_src *single_src,
	const cvi_tde_fill_color *fill_color,
	const cvi_tde_opt* opt)
{
	cvi_tde_rect *src_rect = NULL;
	cvi_tde_rect *dst_rect = NULL;
	cvi_tde_surface *src_surface = NULL;
	cvi_tde_surface *dst_surface = NULL;
	cvi_tde_none_src none_src;

	CVI_U32 vgbuf_width = 0;
	CVI_U32 vgbuf_height = 0;
	CVI_U32 vgbuf_stride = 0;
	CVI_U8 *vgbuf_address = NULL;

	vg_lite_buffer_t src_buf = { 0 };
	vg_lite_buffer_t dst_buf = { 0 };
	vg_lite_buffer_t source_image = { 0 };
	vg_lite_buffer_t fill_color_buf = { 0 };
	vg_lite_rectangle_t src_image_rect = { 0 };
	vg_lite_rectangle_t dst_image_rect = { 0 };
	vg_lite_rectangle_t *src_image_rect_ptr = NULL;

	CVI_U32 fill_color_val = fill_color->color_value;
	vg_lite_matrix_t matrix;
	vg_lite_buffer_format_t format = 0;

	vg_lite_error_t vg_err = VG_LITE_SUCCESS;
	error_message_t cvi_error_message;

	src_rect = single_src->src_rect;
	dst_rect = single_src->dst_rect;
	src_surface = single_src->src_surface;
	dst_surface = single_src->dst_surface;
	none_src.dst_surface = single_src->dst_surface;
	none_src.dst_rect = single_src->src_rect;

	if (handle < 0) {
		return CVI_ERR_TDE_INVALID_HANDLE;
	}

	if (single_src == NULL || fill_color == NULL){
		return CVI_ERR_TDE_NULL_PTR;
	}

	if (src_surface != NULL && opt == NULL){
		return CVI_ERR_TDE_NULL_PTR;
	}

	if (src_surface == NULL && opt == NULL) {
		if(cvi_tde_quick_fill(handle, &none_src, fill_color_val) != CVI_SUCCESS){
			CVI_TDE_DBG("[%s: %d]: none source operate failed\n", __func__, __LINE__);
		}
	}

	vgbuf_width = src_surface->width;
	vgbuf_height = src_surface->height;
	vgbuf_stride = src_surface->stride;
	vgbuf_address = (CVI_U8 *)(CVI_U64)src_surface->phys_addr;
	format = cvi_tde_format_to_vglite_format(src_surface->color_format);
	if(cvi_vglite_init_buf(&src_buf, vgbuf_width, vgbuf_height, vgbuf_stride, format, vgbuf_address, CVI_FALSE) != CVI_SUCCESS) {
		VG_LITE_RETURN_INV("Init buffer failed.");
	}

	vgbuf_width = dst_surface->width;
	vgbuf_height = dst_surface->height;
	vgbuf_stride = dst_surface->stride;
	vgbuf_address = (CVI_U8 *)(CVI_U64)dst_surface->phys_addr;
	format = cvi_tde_format_to_vglite_format(dst_surface->color_format);
	if(cvi_vglite_init_buf(&dst_buf, vgbuf_width, vgbuf_height, vgbuf_stride, format, vgbuf_address, CVI_FALSE) != CVI_SUCCESS) {
		VG_LITE_RETURN_INV("Init buffer failed.");
	}

	/*get source image*/
	if(src_rect != NULL){
		src_image_rect.x = src_rect->pos_x;
		src_image_rect.y = src_rect->pos_y;
		src_image_rect.width = src_rect->width;
		src_image_rect.height = src_rect->height;
		src_image_rect_ptr = &src_image_rect;
	}else src_image_rect_ptr = NULL;
	if(init_vg_middle_buf(&source_image, &src_buf, src_image_rect_ptr) != CVI_SUCCESS){
		CVI_TDE_ERR("[%s: %d]: init source_image buffer failed\n", __func__, __LINE__);
		return CVI_FAILURE;
	}
	/*end*/

	/*Init fill color*/
	format = cvi_tde_format_to_vglite_format(fill_color->color_format);
	fill_color_buf.width = source_image.width;
	fill_color_buf.height = source_image.height;
	fill_color_buf.stride = source_image.stride;
	fill_color_buf.format = format;
	vg_err = vg_lite_allocate(&fill_color_buf);
	print_cvi_error_message(vg_err, cvi_error_message);
	vg_lite_clear(&fill_color_buf, NULL, fill_color_val);
	vg_lite_finish();
	/*end*/
	/*function*/
	/*alpha blending*/
	if(opt->alpha_blending_cmd == CVI_TDE_ALPHA_BLENDING_BLEND){
		if(alpha_blending(&fill_color_buf, &source_image,opt) != CVI_SUCCESS){
			CVI_TDE_ERR("[%s: %d]: alpha blending failed\n", __func__, __LINE__);
			clear_two_bufs(fill_color_buf, source_image);
			return CVI_FAILURE;
		}
		#ifdef CVI_TDE_DEBUG_ENABLE
		cvi_save_bmp("solid_draw_alpha_blend.bmp", (uint8_t *)fill_color_buf.memory, fill_color_buf.width, fill_color_buf.height, fill_color_buf.stride, CVI_TDE_COLOR_FORMAT_RGB565);
		#endif
	}
	/*mirror*/
	if(opt->mirror == CVI_TDE_MIRROR_VER){
		if(ver_mirror(&fill_color_buf, NULL) != CVI_SUCCESS){
			CVI_TDE_ERR("[%s: %d]: vertical mirror failed\n", __func__, __LINE__);
			clear_two_bufs(fill_color_buf, source_image);
			return CVI_FAILURE;
		}
	} else if(opt->mirror == CVI_TDE_MIRROR_HOR){
		if(hor_mirror(&fill_color_buf, NULL) != CVI_SUCCESS){
			CVI_TDE_ERR("[%s: %d]: horizontal mirror failed\n", __func__, __LINE__);
			clear_two_bufs(fill_color_buf, source_image);
			return CVI_FAILURE;
		}
	} else if(opt->mirror == CVI_TDE_MIRROR_BOTH){
		if(ver_mirror(&fill_color_buf, NULL) != CVI_SUCCESS ||
			hor_mirror(&fill_color_buf, NULL) != CVI_SUCCESS){
			CVI_TDE_ERR("[%s: %d]: vertical&&horizontal mirror failed\n", __func__, __LINE__);
			clear_two_bufs(fill_color_buf, source_image);
			return CVI_FAILURE;
		}
	}
	#ifdef CVI_TDE_DEBUG_ENABLE
	cvi_save_bmp("solid_draw_output_mirror_image_buf.bmp", (uint8_t *)fill_color_buf.memory, fill_color_buf.width, fill_color_buf.height, fill_color_buf.stride, CVI_TDE_COLOR_FORMAT_RGB565);
	#endif

	/*resize*/
	if(opt->resize == true) {
		src_image_rect.x = 0;
		src_image_rect.y = 0;
		if(dst_rect != NULL){
			dst_image_rect.x = dst_rect->pos_x;
			dst_image_rect.y = dst_rect->pos_y;
			dst_image_rect.width = dst_rect->width;
			dst_image_rect.height = dst_rect->height;
		}else{
			dst_image_rect.x = 0;
			dst_image_rect.y = 0;
			dst_image_rect.width = dst_buf.width;
			dst_image_rect.height = dst_buf.height;
		}
		compute_resize_matrix(&matrix, &src_image_rect, &dst_image_rect);
	} else {
		vg_lite_identity(&matrix);
	}
	/* clip */
	if(opt->clip_mode == CVI_TDE_CLIP_MODE_INSIDE){
		if(clip_inside(&dst_buf, dst_rect, &fill_color_buf, NULL, &(opt->clip_rect), &matrix) != CVI_SUCCESS){
			CVI_TDE_ERR("[%s: %d]: clip inside failed\n", __func__, __LINE__);
			clear_two_bufs(fill_color_buf, source_image);
			return CVI_FAILURE;
		}
	} else if(opt->clip_mode == CVI_TDE_CLIP_MODE_OUTSIDE){
		if(clip_outside(&dst_buf, dst_rect, &fill_color_buf, NULL, &(opt->clip_rect), &matrix) != CVI_SUCCESS){
			CVI_TDE_ERR("[%s: %d]: clip outside failed\n", __func__, __LINE__);
			clear_two_bufs(fill_color_buf, source_image);
			return CVI_FAILURE;
		}
	} else {
		vg_err = vg_lite_blit(&dst_buf, &fill_color_buf, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
		if(vg_err != VG_LITE_SUCCESS){
			clear_two_bufs(fill_color_buf, source_image);
		}
		print_cvi_error_message(vg_err, cvi_error_message);
	}
	clear_two_bufs(fill_color_buf, source_image);
	return CVI_SUCCESS;
}

CVI_S32 cvi_tde_rotate(CVI_S32 handle, const cvi_tde_single_src *single_src, cvi_tde_rotate_angle rotate)
{
	cvi_tde_rect *src_rect = NULL;
	cvi_tde_rect *dst_rect = NULL;
	cvi_tde_surface *src_surface = NULL;
	cvi_tde_surface *dst_surface = NULL;

	CVI_U32 vgbuf_width = 0;
	CVI_U32 vgbuf_height = 0;
	CVI_U32 vgbuf_stride = 0;
	CVI_U8 *vgbuf_address = NULL;

	vg_lite_matrix_t matrix;
	vg_lite_float_t delta_x, delta_y;
	vg_lite_float_t center_x, center_y;
	vg_lite_float_t start_x, start_y;
	vg_lite_float_t target_x, target_y;
	vg_lite_error_t err = VG_LITE_SUCCESS;
	vg_lite_buffer_format_t format = 0;
	vg_lite_rectangle_t rect = { 0 };
	vg_lite_buffer_t src_buf = { 0 };
	vg_lite_buffer_t vg_src_surf = { 0 };
	vg_lite_buffer_t dst_buf = { 0 };
	vg_lite_float_t deg = 0.0f;

	if (handle < 0) {
		return CVI_ERR_TDE_INVALID_HANDLE;
	}

	if (single_src == NULL) {
		return CVI_ERR_TDE_NULL_PTR;
	}

	src_rect = single_src->src_rect;
	dst_rect = single_src->dst_rect;
	src_surface = single_src->src_surface;
	dst_surface = single_src->dst_surface;

	if (src_rect == NULL
		|| dst_rect == NULL
		|| src_surface == NULL
		|| dst_surface == NULL) {
		return CVI_ERR_TDE_NULL_PTR;
	}

	vgbuf_width = src_surface->width;
	vgbuf_height = src_surface->height;
	vgbuf_stride = src_surface->stride;
	vgbuf_address = (CVI_U8 *)(CVI_U64)src_surface->phys_addr;
	format = cvi_tde_format_to_vglite_format(src_surface->color_format);

	if(cvi_vglite_init_buf(&vg_src_surf, vgbuf_width, vgbuf_height, vgbuf_stride, format, vgbuf_address, CVI_FALSE) != CVI_SUCCESS) {
		VG_LITE_RETURN_INV("Init buffer failed.");
	}

	vg_lite_identity(&matrix);
	if (format == VG_LITE_ARGB8888) {
		// FIXME: this is workaround
		src_buf.format = VG_LITE_RGB888;
		src_buf.width = vgbuf_width;
		src_buf.height = vgbuf_height;

		err = vg_lite_allocate(&src_buf);
		if (err != VG_LITE_SUCCESS) {
			return CVI_FAILURE;
		}

		err = vg_lite_blit(&src_buf, &vg_src_surf, NULL, 0, 0, VG_LITE_FILTER_POINT);
		if (err != VG_LITE_SUCCESS) {
			return CVI_FAILURE;
		}

		err = vg_lite_finish();
		if (err != VG_LITE_SUCCESS) {
			return CVI_FAILURE;
		}
	} else {
		src_buf = vg_src_surf;
	}

	vgbuf_width = dst_surface->width;
	vgbuf_height = dst_surface->height;
	vgbuf_stride = dst_surface->stride;
	vgbuf_address = (CVI_U8 *)(CVI_U64)dst_surface->phys_addr;
	format = cvi_tde_format_to_vglite_format(dst_surface->color_format);

	if(cvi_vglite_init_buf(&dst_buf, vgbuf_width, vgbuf_height, vgbuf_stride, format, vgbuf_address, CVI_FALSE) != CVI_SUCCESS) {
		VG_LITE_RETURN_INV("Init buffer failed.");
	}

	rect.x = src_rect->pos_x;
	rect.y = src_rect->pos_y;
	rect.width = src_rect->width;
	rect.height = src_rect->height;
	CVI_TDE_DBG("rect :%d, %d, %d, %d\n", rect.x, rect.y, rect.width, rect.height);

	// move to center of the surface
	center_x = vgbuf_width / 2.0f;
	center_y = vgbuf_height / 2.0f;
	vg_lite_translate(center_x, center_y, &matrix);

	switch(rotate) {
		case CVI_TDE_ROTATE_CLOCKWISE_90:
			deg = 90.0f;
			start_x = src_rect->pos_x;
			start_y = src_rect->pos_y + src_rect->height;
			target_x = dst_rect->pos_y;
			target_y = vgbuf_height - dst_rect->pos_x;
			break;
		case CVI_TDE_ROTATE_CLOCKWISE_180:
			deg = 180.0f;
			start_x = src_rect->pos_x + src_rect->width;
			start_y = src_rect->pos_y + src_rect->height;
			target_x = vgbuf_width - dst_rect->pos_x;
			target_y = vgbuf_height - dst_rect->pos_y;
			break;
		case CVI_TDE_ROTATE_CLOCKWISE_270:
			deg = 270.0f;
			start_x = src_rect->pos_x + src_rect->width;
			start_y = src_rect->pos_y;
			target_x = vgbuf_width - dst_rect->pos_y ;
			target_y = dst_rect->pos_x;
			break;
		case CVI_TDE_ROTATE_MAX:
		default:
			return CVI_ERR_TDE_INVALID_PARAM;
	}

	vg_lite_rotate(deg, &matrix);
	vg_lite_translate(-center_x, -center_y, &matrix);

	delta_x = target_x - start_x;
	delta_y = target_y - start_y;
	CVI_TDE_DBG("delta_x:%f, delta_y:%f\n", delta_x, delta_y);
	CVI_TDE_DBG("start:%.2f %.2f\n", start_x, start_y);
	CVI_TDE_DBG("target:%.2f %.2f\n", target_x, target_y);
	vg_lite_translate(delta_x, delta_y, &matrix);

	err = vg_lite_blit_rect(&dst_buf, &src_buf, &rect, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
	if (err != VG_LITE_SUCCESS) {
		return CVI_FAILURE;
	}

	if (format == VG_LITE_ARGB8888) {
		vg_lite_free(&src_buf);
	}

	return 0;
}

CVI_S32 cvi_tde_bit_blit(CVI_S32 handle,
                         const cvi_tde_double_src *double_src,
                         const cvi_tde_opt *opt)
{
	cvi_tde_rect *bg_rect = NULL;
	cvi_tde_rect *fg_rect = NULL;
	cvi_tde_rect *dst_rect = NULL;
	cvi_tde_surface *bg_surface = NULL;
	cvi_tde_surface *fg_surface = NULL;
	cvi_tde_surface *dst_surface = NULL;

	CVI_U32 vgbuf_width = 0;
	CVI_U32 vgbuf_height = 0;
	CVI_U32 vgbuf_stride = 0;
	CVI_U8 *vgbuf_address = NULL;
	vg_lite_buffer_format_t format = 0;


	vg_lite_buffer_t bg_buf = { 0 };
	vg_lite_buffer_t fg_buf = { 0 };
	vg_lite_buffer_t dst_buf = { 0 };

	vg_lite_rectangle_t bg_buf_rect = { 0 };
	vg_lite_rectangle_t fg_buf_rect = { 0 };
	vg_lite_rectangle_t dst_buf_rect = { 0 };
	vg_lite_rectangle_t* bg_buf_rect_ptr = NULL;
	vg_lite_rectangle_t* fg_buf_rect_ptr = NULL;

	vg_lite_buffer_t bg_image = { 0 };
	vg_lite_buffer_t fg_image = { 0 };

	vg_lite_matrix_t matrix;

	if (handle < 0) {
		return CVI_ERR_TDE_INVALID_HANDLE;
	}

	if (double_src == NULL || opt == NULL) {
		return CVI_ERR_TDE_NULL_PTR;
	}

	if (double_src->bg_surface == NULL || double_src->dst_surface ==NULL){
		return CVI_ERR_TDE_NULL_PTR;
	}

	//initialization parameter
	bg_rect = double_src->bg_rect;
	fg_rect = double_src->fg_rect;
	dst_rect = double_src->dst_rect;
	bg_surface = double_src->bg_surface;
	fg_surface = double_src->fg_surface;
	dst_surface = double_src->dst_surface;

	/*background image init*/
	vgbuf_width = bg_surface->width;
	vgbuf_height = bg_surface->height;
	vgbuf_stride = bg_surface->stride;
	vgbuf_address = (CVI_U8 *)(CVI_U64)bg_surface->phys_addr;
	format = cvi_tde_format_to_vglite_format(bg_surface->color_format);
	if (cvi_vglite_init_buf(&bg_buf, vgbuf_width, vgbuf_height, vgbuf_stride, format, vgbuf_address, CVI_FALSE) != CVI_SUCCESS) {
		VG_LITE_RETURN_INV("Init buffer failed.");
	}
	if (bg_rect != NULL) {
		bg_buf_rect.x = bg_rect->pos_x;
		bg_buf_rect.y = bg_rect->pos_y;
		bg_buf_rect.width = bg_rect->width;
		bg_buf_rect.height = bg_rect->height;
		bg_buf_rect_ptr = &bg_buf_rect;
	} else {
		bg_buf_rect_ptr = NULL;
	}
	if(init_vg_middle_buf(&bg_image, &bg_buf, bg_buf_rect_ptr) != CVI_SUCCESS){
		CVI_TDE_ERR("[%s: %d]: init bg_image buffer failed\n", __func__, __LINE__);
		return CVI_FAILURE;
	}

	#ifdef CVI_TDE_DEBUG_ENABLE
    cvi_save_bmp("bg_image_buf.bmp", (uint8_t *)bg_image.memory, bg_image.width, bg_image.height, bg_image.stride, CVI_TDE_COLOR_FORMAT_RGBA8888);
    #endif

    /*end*/
    /*dstination image init*/
    vgbuf_width = dst_surface->width;
    vgbuf_height = dst_surface->height;
    vgbuf_stride = dst_surface->stride;
    vgbuf_address = (CVI_U8 *)(CVI_U64)dst_surface->phys_addr;
    format = cvi_tde_format_to_vglite_format(dst_surface->color_format);
    if(cvi_vglite_init_buf(&dst_buf, vgbuf_width, vgbuf_height, vgbuf_stride, format, vgbuf_address, CVI_FALSE) != CVI_SUCCESS) {
        VG_LITE_RETURN_INV("Init buffer failed.");
    }
    /*end*/
    /*foreground image init*/
    if(fg_surface != NULL) {
        vgbuf_width = fg_surface->width;
        vgbuf_height = fg_surface->height;
        vgbuf_stride = fg_surface->stride;
        vgbuf_address = (CVI_U8 *)(CVI_U64)fg_surface->phys_addr;
        format = cvi_tde_format_to_vglite_format(fg_surface->color_format);
        if(cvi_vglite_init_buf(&fg_buf, vgbuf_width, vgbuf_height, vgbuf_stride, format, vgbuf_address, CVI_FALSE) != CVI_SUCCESS) {
            VG_LITE_RETURN_INV("Init buffer failed.");
        }

        if(fg_rect != NULL){
            fg_buf_rect.x = fg_rect->pos_x;
            fg_buf_rect.y = fg_rect->pos_y;
            fg_buf_rect.width = fg_rect->width;
            fg_buf_rect.height = fg_rect->height;
            fg_buf_rect_ptr = &fg_buf_rect;
        }else fg_buf_rect_ptr = NULL;

        if(init_vg_middle_buf(&fg_image, &fg_buf, fg_buf_rect_ptr) != CVI_SUCCESS){
            CVI_TDE_ERR("[%s: %d]: init bg_image buffer failed\n", __func__, __LINE__);
            return CVI_FAILURE;
        }

		#ifdef CVI_TDE_DEBUG_ENABLE
		cvi_save_bmp("fg_image_buf.bmp", (uint8_t *)fg_image.memory, fg_image.width, fg_image.height, fg_image.stride, CVI_TDE_COLOR_FORMAT_RGBA8888);
		#endif

        /* bitmap operation*/
        if(opt->alpha_blending_cmd == CVI_TDE_ALPHA_BLENDING_BLEND){
            if(alpha_blending(&bg_image, &fg_image,opt) != CVI_SUCCESS){
                CVI_TDE_ERR("[%s: %d]: alpha blending failed\n", __func__, __LINE__);
                clear_two_bufs(bg_image, fg_image);
                return CVI_FAILURE;
            }
        } else {
            CVI_TDE_DBG("[%s: %d]: no bitmap operation\n", __func__, __LINE__);
        }
	} else {
		CVI_TDE_DBG("Single source operation\n");
	}
	/*end*/
	#ifdef CVI_TDE_DEBUG_ENABLE
	cvi_save_bmp("bit_blit_output_image_buf.bmp", (uint8_t *)bg_image.memory, bg_image.width, bg_image.height, bg_image.stride, CVI_TDE_COLOR_FORMAT_RGBA8888);
	#endif
	/* function */
	/* mirror */
	if(opt->mirror == CVI_TDE_MIRROR_VER){
		if(ver_mirror(&bg_image, NULL) != CVI_SUCCESS){
			CVI_TDE_ERR("[%s: %d]: vertical mirror failed\n", __func__, __LINE__);
			if(fg_surface != NULL){
				clear_two_bufs(bg_image, fg_image);
			} else {
				vg_lite_free(&bg_image);
			}
			return CVI_FAILURE;
		}
    } else if(opt->mirror == CVI_TDE_MIRROR_HOR){
		if(hor_mirror(&bg_image, NULL) != CVI_SUCCESS){
			CVI_TDE_ERR("[%s: %d]: horizontal mirror failed\n", __func__, __LINE__);
			if(fg_surface != NULL){
				clear_two_bufs(bg_image, fg_image);
			} else {
				vg_lite_free(&bg_image);
			}
			return CVI_FAILURE;
		}
	} else if(opt->mirror == CVI_TDE_MIRROR_BOTH){
		if(ver_mirror(&bg_image, NULL) != CVI_SUCCESS ||
			hor_mirror(&bg_image, NULL) != CVI_SUCCESS) {
			CVI_TDE_ERR("[%s: %d]: vertical&&horizontal mirror failed\n", __func__, __LINE__);
			if(fg_surface != NULL){
				clear_two_bufs(bg_image, fg_image);
			} else {
				vg_lite_free(&bg_image);
			}
			return CVI_FAILURE;
		}
	}

	#ifdef CVI_TDE_DEBUG_ENABLE
	cvi_save_bmp("output_mirror_image_buf.bmp", (uint8_t *)bg_image.memory, bg_image.width, bg_image.height, bg_image.stride, CVI_TDE_COLOR_FORMAT_RGB565);
	#endif
	/*resize*/
	if(opt->resize == true){
		bg_buf_rect.x = 0;
		bg_buf_rect.y = 0;
		bg_buf_rect.width = bg_image.width;
		bg_buf_rect.height = bg_image.height;

		if(dst_rect != NULL){
			dst_buf_rect.x = dst_rect->pos_x;
			dst_buf_rect.y = dst_rect->pos_y;
			dst_buf_rect.width = dst_rect->width;
			dst_buf_rect.height = dst_rect->height;
		}else{
			dst_buf_rect.x = 0;
			dst_buf_rect.y = 0;
			dst_buf_rect.width = dst_buf.width;
			dst_buf_rect.height = dst_buf.height;
		}
		compute_resize_matrix(&matrix, &bg_buf_rect, &dst_buf_rect);
	} else {
		vg_lite_identity(&matrix);
	}
	/* clip */
	if (opt->clip_mode == CVI_TDE_CLIP_MODE_INSIDE) {
		if(clip_inside(&dst_buf, dst_rect, &bg_image, NULL, &(opt->clip_rect), &matrix) != CVI_SUCCESS){
			CVI_TDE_ERR("[%s: %d]: clip inside failed\n", __func__, __LINE__);
			if(fg_surface != NULL){
				clear_two_bufs(bg_image, fg_image);
			}else vg_lite_free(&bg_image);
			return CVI_FAILURE;
		}
	} else if (opt->clip_mode == CVI_TDE_CLIP_MODE_OUTSIDE) {
		if(clip_outside(&dst_buf, dst_rect, &bg_image, NULL, &(opt->clip_rect), &matrix) != CVI_SUCCESS){
			CVI_TDE_ERR("[%s: %d]: clip outside failed\n", __func__, __LINE__);
			if(fg_surface != NULL){
				clear_two_bufs(bg_image, fg_image);
			} else {
				vg_lite_free(&bg_image);
			}
			return CVI_FAILURE;
		}
	} else {
		vg_lite_blit(&dst_buf, &bg_image, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
	}
	if(fg_surface != NULL){
		clear_two_bufs(bg_image, fg_image);
	} else {
		vg_lite_free(&bg_image);
	}
	return CVI_SUCCESS;
}

CVI_S32 cvi_tde_pattern_fill(
    CVI_S32 handle,
    const cvi_tde_double_src *double_src,
    const cvi_tde_pattern_fill_opt *fill_opt)
{
    UNUSED(handle);
    UNUSED(double_src);
    UNUSED(fill_opt);
    return 0;
}

CVI_S32 cvi_tde_mb_blit(CVI_S32 handle, const cvi_tde_mb_src *mb_src, const cvi_tde_mb_opt *opt)
{
    UNUSED(handle);
    UNUSED(mb_src);
    UNUSED(opt);
    return 0;
}

CVI_S32 cvi_tde_set_alpha_threshold_value(CVI_U8 threshold_value)
{
    UNUSED(threshold_value);
    return 0;
}

CVI_S32 cvi_tde_get_alpha_threshold_value(CVI_U8 *threshold_value)
{
    UNUSED(threshold_value);
    return 0;
}

CVI_S32 cvi_tde_set_alpha_threshold_state(CVI_BOOL threshold_en)
{
    UNUSED(threshold_en);
    return 0;
}

CVI_S32 cvi_tde_get_alpha_threshold_state(CVI_BOOL *threshold_en)
{
    UNUSED(threshold_en);
    return 0;
}


#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L

typedef struct tagBITMAPINFOHEADER{
    unsigned int      biSize;
    unsigned int       biWidth;
    unsigned int       biHeight;
    unsigned short       biPlanes;
    unsigned short       biBitCount;
    unsigned int      biCompression;
    unsigned int      biSizeImage;
    unsigned int       biXPelsPerMeter;
    unsigned int       biYPelsPerMeter;
    unsigned int      biClrUsed;
    unsigned int      biClrImportant;
} __attribute((packed)) BITMAPINFOHEADER;

typedef struct tagBITMAPFILEHEADER {
    unsigned short    bfType;
    unsigned int   bfSize;
    unsigned short    bfReserved1;
    unsigned short    bfReserved2;
    unsigned int   bfOffBits;
} __attribute((packed)) BITMAPFILEHEADER;

void WriteBMPFile (const char * file_name, unsigned char * pbuf, unsigned int size)
{
    FILE* fd;
    fd = fopen(file_name, "wb");

    if(fd != NULL)
    {
        fwrite(pbuf, 1, size, fd);

        fclose(fd);
    }
    else
    {
        CVI_TDE_ERR("errno:%d %s(%d)open file: %s failed!\n",
            ferror(fd), __FUNCTION__, __LINE__, file_name);
    }
}
vg_lite_buffer_format_t cvi_tde_format_to_vglite_format(cvi_tde_color_format format)
{
    switch (format) {
    case CVI_TDE_COLOR_FORMAT_ARGB1555:
        return VG_LITE_ARGB1555;
    case CVI_TDE_COLOR_FORMAT_ABGR1555:
        return VG_LITE_ABGR1555;
    case CVI_TDE_COLOR_FORMAT_RGBA5551:
        return VG_LITE_RGBA5551;
    case CVI_TDE_COLOR_FORMAT_BGRA5551:
        return VG_LITE_BGRA5551;
    case CVI_TDE_COLOR_FORMAT_RGB888:
        return VG_LITE_RGB888;
    case CVI_TDE_COLOR_FORMAT_BGR888:
        return VG_LITE_BGR888;
    case CVI_TDE_COLOR_FORMAT_ARGB8888:
        return VG_LITE_ARGB8888;
    case CVI_TDE_COLOR_FORMAT_ABGR8888:
        return VG_LITE_ABGR8888;
    case CVI_TDE_COLOR_FORMAT_RGBA8888:
        return VG_LITE_RGBA8888;
    case CVI_TDE_COLOR_FORMAT_BGRA8888:
        return VG_LITE_BGRA8888;
    case CVI_TDE_COLOR_FORMAT_RGB565:
        return VG_LITE_RGB565;
    case CVI_TDE_COLOR_FORMAT_BGR565:
        return VG_LITE_BGR565;
    case CVI_TDE_COLOR_FORMAT_ARGB4444:
        return VG_LITE_ARGB4444;
    case CVI_TDE_COLOR_FORMAT_ABGR4444:
        return VG_LITE_ABGR4444;
    case CVI_TDE_COLOR_FORMAT_RGBA4444:
        return VG_LITE_RGBA4444;
    case CVI_TDE_COLOR_FORMAT_BGRA4444:
        return VG_LITE_BGRA4444;
    case CVI_TDE_COLOR_FORMAT_ARGB8565:
        return VG_LITE_ARGB8565;
    case CVI_TDE_COLOR_FORMAT_ABGR8565:
        return VG_LITE_ABGR8565;
    case CVI_TDE_COLOR_FORMAT_A8:
        return VG_LITE_A8;
	default:
		return VG_LITE_ARGB8888;
	}
}


vg_lite_blend_t cvi_tde_blend_to_vglite_blend(cvi_tde_blend_cmd blend){
	switch (blend)
	{
	case CVI_TDE_BLEND_CMD_NONE:
		return VG_LITE_BLEND_NORMAL_LVGL;
	case CVI_TDE_BLEND_CMD_SRC:
		return VG_LITE_BLEND_NONE;
	case CVI_TDE_BLEND_CMD_SRCOVER:
		return VG_LITE_BLEND_SRC_OVER;
	case CVI_TDE_BLEND_CMD_DSTOVER:
		return VG_LITE_BLEND_DST_OVER;
	case CVI_TDE_BLEND_CMD_SRCIN:
		return VG_LITE_BLEND_SRC_IN;
	case CVI_TDE_BLEND_CMD_DSTIN:
		return VG_LITE_BLEND_DST_IN;
	case CVI_TDE_BLEND_CMD_ADD:
		return VG_LITE_BLEND_ADDITIVE;
	default:
		return VG_LITE_BLEND_ADDITIVE;
	}
}

CVI_S32 cvi_save_bmp(const CVI_CHAR *image_name, CVI_U8 *p, CVI_U32 width, CVI_U32 height,
            CVI_U32 stride, cvi_tde_color_format cvi_tde_format)
{
    BITMAPINFOHEADER *infoHeader;
    BITMAPFILEHEADER *fileHeader;
    uint8_t *l_image_data = NULL;
    unsigned char *l_readpixel_data = NULL;
    unsigned char *l_data_load = NULL;
    int data_size;
    int index, index2;
    int file_len;
    uint16_t color;
    vg_lite_buffer_format_t format;

    format = cvi_tde_format_to_vglite_format(cvi_tde_format);
    CVI_TDE_DBG("save bmp args: %s %d %d %d, fmt:%d\n", image_name, width, height, stride, format);

    //data
    data_size = width * height * 3;
    file_len = data_size + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    l_image_data = (uint8_t *)malloc(file_len);
    if (l_image_data == NULL) {
        CVI_TDE_ERR("cvi tde out of memory!\n");
        return -1;
    }

    fileHeader =(BITMAPFILEHEADER *) l_image_data;
    infoHeader =(BITMAPINFOHEADER *)(l_image_data + sizeof(BITMAPFILEHEADER));

    // only clear headers
    memset(l_image_data, 0, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));

    //file header.
    fileHeader->bfType       = 0x4D42;
    fileHeader->bfSize       = sizeof(BITMAPFILEHEADER);
    fileHeader->bfReserved1  = 0;
    fileHeader->bfReserved2  = 0;
    fileHeader->bfOffBits    = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    //infoHeader.
    *(char *)&infoHeader->biSize = sizeof(BITMAPINFOHEADER);
    infoHeader->biWidth = width;
    infoHeader->biHeight = height;
    infoHeader->biPlanes = 1;
    infoHeader->biBitCount = 24;
    infoHeader->biCompression = BI_RGB;

    for(index = 0; index < (int)height; index++){
        l_readpixel_data = (uint8_t *)p + index*stride;
        l_data_load = l_image_data + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (height-1-index) * width * 3;
        for (index2 = 0; index2 < (int)width; index2++, l_data_load+=3)
        {
            // TO-BE-DONE
            switch (format)
            {
            case VG_LITE_ARGB8888:
                l_data_load[0] = l_readpixel_data[3];
                l_data_load[1] = l_readpixel_data[2];
                l_data_load[2] = l_readpixel_data[1];
                l_readpixel_data +=4;
                break;
            case VG_LITE_RGBA8888:
            case VG_LITE_RGBX8888:
                l_data_load[0] = l_readpixel_data[2];
                l_data_load[1] = l_readpixel_data[1];
                l_data_load[2] = l_readpixel_data[0];
                l_readpixel_data +=4;
                break;
            case VG_LITE_BGRA8888:
            case VG_LITE_BGRX8888:
                l_data_load[0] = l_readpixel_data[0];
                l_data_load[1] = l_readpixel_data[1];
                l_data_load[2] = l_readpixel_data[2];
                l_readpixel_data +=4;
                break;
            case VG_LITE_RGB565:
                color = *(uint16_t *)l_readpixel_data;
                l_readpixel_data += 2;
                l_data_load[2] = ((color & 0x001F) << 3) | ((color & 0x001C) >> 2);
                l_data_load[1] = ((color & 0x07E0) >> 3) | ((color & 0x0600) >> 9);
                l_data_load[0] = ((color & 0xF800) >> 8) | ((color & 0xE000) >> 13);
                break;
            case VG_LITE_BGR565:
                color = *(uint16_t *)l_readpixel_data;
                l_readpixel_data += 2;
                l_data_load[2] = ((color & 0xF800) >> 8) | ((color & 0xE000) >> 13);
                l_data_load[1] = ((color & 0x07E0) >> 3) | ((color & 0x0600) >> 9);
                l_data_load[0] = ((color & 0x001F) << 3) | ((color & 0x001C) >> 2);
                break;
            case VG_LITE_RGB888:
                l_data_load[0] = l_readpixel_data[0];
                l_data_load[1] = l_readpixel_data[1];
                l_data_load[2] = l_readpixel_data[2];
                l_readpixel_data += 3;
                break;

            case VG_LITE_ARGB1555:
                color = *(uint16_t*)l_readpixel_data;
                // color = ((color & 0xff00) >> 8) | (( color & 0x00ff) << 8);
                // color = (color >> 1);
                l_readpixel_data += 2;
                l_data_load[0] = ((color & 0x001F) << 3) | ((color & 0x001C) >> 2) ;
                l_data_load[1] = ((color & 0x03E0) >> 2) | ((color & 0x0380) >> 7);
                l_data_load[2] = ((color & 0x7C00) >> 7) | ((color & 0x7000) >> 12);

                break;

            case VG_LITE_ABGR1555:
                color = *(uint16_t*)l_readpixel_data;
                // color = ((color & 0xff00) >> 8) | (( color & 0x00ff) << 8);
                // color = (color >> 1);
                l_readpixel_data += 2;
                l_data_load[0] = ((color & 0x001F) << 3) | ((color & 0x001C) >> 2);
                l_data_load[1] = ((color & 0x03E0) >> 2) | ((color & 0x0380) >> 7);
                l_data_load[2] = ((color & 0x7C00) >> 7) | ((color & 0x7000) >> 12);
                break;

            case  VG_LITE_RGBA4444:
                color = *(uint16_t*)l_readpixel_data;
                l_readpixel_data += 2;
                l_data_load[2] = (color & 0x000F) << 4;
                l_data_load[1] = (color & 0x00F0);
                l_data_load[0] = (color & 0x0F00) >> 4;
                break;
            case VG_LITE_BGRA4444:
                color = *(uint16_t*)l_readpixel_data;
                l_readpixel_data += 2;
                l_data_load[0] = (color & 0x000F) << 4;
                l_data_load[1] = (color & 0x00F0);
                l_data_load[2] = (color & 0x0F00) >> 4;
                break;

            case VG_LITE_A8:
            case VG_LITE_L8:
                l_data_load[0] = l_data_load[1] = l_data_load[2] = l_readpixel_data[0];
                l_readpixel_data++;
                break;
            case VG_LITE_YUYV:
                CVI_TDE_WRN("not support format!\n");
                break;
            default:
                CVI_TDE_ERR("unkonwn format!\n");
                break;
            }
        }
    }

    WriteBMPFile(image_name, l_image_data, file_len);

    if (l_image_data) {
        free(l_image_data);
        l_image_data = NULL;
    }

    return 0;
}

// Write a 32-bit signed integer.
static int write_long(FILE *fp,int  l)
{
    putc(l, fp);
    putc(l >> 8, fp);
    putc(l >> 16, fp);
    return (putc(l >> 24, fp));
}

CVI_S32 cvi_save_raw(const CVI_CHAR *name, CVI_U8* buffer, CVI_U32 width, CVI_U32 height,
            CVI_U32 stride, cvi_tde_color_format cvi_tde_format)
{
    FILE * fp;
    int status = 1;

    fp = fopen(name, "wb");

    if (fp != NULL) {
        // Save width, height, stride and format info.
        write_long(fp, cvi_tde_format);
        write_long(fp, width);
        write_long(fp, height);
        write_long(fp, stride);

        // Save buffer info.
        fwrite(buffer, 1, stride * height, fp);

        fclose(fp);
        fp = NULL;

        status = 0;
    }

    // Return the status.
    return status;
}

CVI_S32 cvi_draw_single_line(vg_lite_buffer_t *fb, const cvi_tde_line *line)
{
    vg_lite_matrix_t matrix;
    uint32_t data_size;
    vg_lite_path_t path = { 0 };
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_blend_t blend = VG_LITE_BLEND_NONE;
    vg_lite_color_t color = 0;
    CVI_U32 thick = 0;
    CVI_S32 rc = CVI_SUCCESS;

    static uint8_t sides_cmd[] = {
        VLC_OP_MOVE,
        VLC_OP_LINE,
        VLC_OP_LINE,
        VLC_OP_LINE,

        VLC_OP_END
    };

    static float sides_data_left[] = {
        0, 0,
        50, 0,
        50, 50,
        0, 50,
    };

    if (fb == NULL
        || line == NULL) {
        return CVI_FAILURE;
    }

    color = line->color;
    thick = line->thick;
    if (thick == 0) {
        return CVI_FAILURE;
    }

    vg_lite_identity(&matrix);

    if (line->start_y == line->end_y) {

        sides_data_left[0] = CVI_MIN(line->start_x, line->end_x);
        sides_data_left[1] = line->start_y - thick / 2.0;
        sides_data_left[2] = CVI_MAX(line->end_x, line->start_x);
        sides_data_left[3] = line->end_y - thick / 2.0;
        sides_data_left[4] = CVI_MAX(line->end_x, line->start_x);
        sides_data_left[5] = line->end_y + thick / 2.0;
        sides_data_left[6] = CVI_MIN(line->start_x, line->end_x);
        sides_data_left[7] = line->start_y + thick / 2.0;

    } else if(line->start_x == line->end_x){

        sides_data_left[0] = line->start_x + thick / 2.0;
        sides_data_left[1] = CVI_MIN(line->start_y, line->end_y);
        sides_data_left[2] = line->end_x + thick / 2.0;
        sides_data_left[3] = CVI_MAX(line->end_y, line->start_y);
        sides_data_left[4] = line->end_x - thick / 2.0;
        sides_data_left[5] = CVI_MAX(line->end_y, line->start_y);
        sides_data_left[6] = line->start_x - thick / 2.0;
        sides_data_left[7] = CVI_MIN(line->start_y, line->end_y);

    } else if(((line->start_x < line->end_x) && (line->start_y < line->end_y)) || ((line->start_x > line->end_x) && (line->start_y > line->end_y))) {

        int w = thick / 2;
        sides_data_left[0] = CVI_MIN(line->start_x, line->end_x) + w;
        sides_data_left[1] = CVI_MIN(line->start_y, line->end_y) - w;
        sides_data_left[2] = CVI_MAX(line->end_x, line->start_x) + w;
        sides_data_left[3] = CVI_MAX(line->end_y, line->start_y) - w;
        sides_data_left[4] = CVI_MAX(line->end_x, line->start_x) - w;
        sides_data_left[5] = CVI_MAX(line->end_y, line->start_y) + w;
        sides_data_left[6] = CVI_MIN(line->start_x, line->end_x) - w;
        sides_data_left[7] = CVI_MIN(line->start_y, line->end_y) + w;

    } else if(((line->start_x < line->end_x) && (line->start_y > line->end_y)) || ((line->start_x > line->end_x) && (line->start_y < line->end_y))) {

        int w = thick / 2;
        sides_data_left[0] = CVI_MAX(line->end_x, line->start_x) + w;
        sides_data_left[1] = CVI_MIN(line->end_y, line->start_y) + w;
        sides_data_left[2] = CVI_MIN(line->start_x, line->end_x) + w;
        sides_data_left[3] = CVI_MAX(line->start_y, line->end_y) + w;
        sides_data_left[4] = CVI_MIN(line->start_x, line->end_x) - w;
        sides_data_left[5] = CVI_MAX(line->start_y, line->end_y) - w;
        sides_data_left[6] = CVI_MAX(line->end_x, line->start_x) - w;
        sides_data_left[7] = CVI_MIN(line->end_y, line->start_y) - w;

    }


    data_size = vg_lite_get_path_length(sides_cmd, sizeof(sides_cmd), VG_LITE_FP32);

    CHECK_ERROR(vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0));
    path.path = malloc(data_size);
    CHECK_ERROR(vg_lite_append_path(&path, sides_cmd, sides_data_left, sizeof(sides_cmd)));

    CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_NON_ZERO, &matrix, blend, color));

    CHECK_ERROR(vg_lite_clear_path(&path));


ErrorHandler:
    return rc;
}

/* ascending sort*/
static void insert_sort(CVI_S32 arr[], int len){
    int i, j;
    CVI_S32 key;
    for(i = 1; i < len; i++){
        key = arr[i];
        j = i - 1;
        while( j >= 0 && arr[j] > key){
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

static vg_lite_rectangle_t *rectangle_intersection(const cvi_tde_rect *dst_rect,
												const cvi_tde_rect *clip_rect,
												vg_lite_rectangle_t *rect0,
												bool *intersect)
{
	int pos[4];
	*(intersect) = false;
	vg_lite_rectangle_t rect1 = {0};

	if(clip_rect == NULL) return NULL;

	if(dst_rect == NULL) {
		rect0->x = clip_rect->pos_x;
		rect0->y = clip_rect->pos_y;
		rect0->width = clip_rect->width;
		rect0->height = clip_rect->height;
		*(intersect) = true;
	} else {
		rect0->x = dst_rect->pos_x;
		rect0->y = dst_rect->pos_y;
		rect0->width = dst_rect->width;
		rect0->height = dst_rect->height;

		rect1.x = clip_rect->pos_x;
		rect1.y = clip_rect->pos_y;
		rect1.width = clip_rect->width;
		rect1.height = clip_rect->height;

		if(!( (rect0->x + rect0->width) < rect1.x
			|| (rect1.x + rect1.width) < rect0->x
			|| (rect0->y + rect0->height) < rect1.y
			|| (rect1.y + rect1.height) < rect0->y)){
			pos[0] = rect0->x;
			pos[1] = rect0->x + rect0->width;
			pos[2] = rect1.x;
			pos[3] = rect1.x + rect1.width;

			insert_sort(pos, 4);
			rect0->x = pos[1];
			rect0->width = pos[2] - pos[1];

			pos[0] = rect0->y;
			pos[1] = rect0->y + rect0->height;
			pos[2] = rect1.y;
			pos[3] = rect1.y + rect1.height;
			insert_sort(pos, 4);
			rect0->y = pos[1];
			rect0->height= pos[2] - pos[1];
			*(intersect) = true;
		}
		else{
			CVI_TDE_DBG("targrt area doesn't overlap with cliping area\n");
			*(intersect) = false;
		}
	}
	return rect0;
}

static CVI_U32 ver_mirror(vg_lite_buffer_t *source_image, vg_lite_rectangle_t *src_rect){
	vg_lite_buffer_t mid = { 0 };
	vg_lite_matrix_t matrix;

	vg_lite_error_t vg_err;
	error_message_t cvi_error_message;

	if(src_rect != NULL){
		mid.width = src_rect->width;
		mid.height = src_rect->height;
	}else{
		mid.width = source_image->width;
		mid.height = source_image->height;
	}
	mid.format = source_image->format;

	vg_err = vg_lite_allocate(&mid);
	print_cvi_error_message(vg_err, cvi_error_message);

	vg_err = vg_lite_set_mirror(VG_LITE_ORIENTATION_BOTTOM_TOP);
	if(vg_err != VG_LITE_SUCCESS) vg_lite_free(&mid);
	print_cvi_error_message(vg_err, cvi_error_message);

	vg_lite_identity(&matrix);
	vg_lite_scale(1.0f, 1.0f, &matrix);
	vg_lite_translate(0, 0, &matrix);

	vg_err = vg_lite_blit_rect(&mid, source_image, src_rect, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
	vg_lite_finish();
	if(vg_err != VG_LITE_SUCCESS) vg_lite_free(&mid);
	print_cvi_error_message(vg_err, cvi_error_message);

	#ifdef CVI_TDE_DEBUG_ENABLE
	cvi_save_bmp("mid_image.bmp", (uint8_t *)mid.memory, mid.width, mid.height,mid.stride, CVI_TDE_COLOR_FORMAT_RGB565);
	#endif
	vg_err = vg_lite_set_mirror(VG_LITE_ORIENTATION_TOP_BOTTOM);
	if(vg_err != VG_LITE_SUCCESS) vg_lite_free(&mid);
	print_cvi_error_message(vg_err, cvi_error_message);

	vg_lite_identity(&matrix);
	vg_lite_scale(1.0f, 1.0f, &matrix);
	if(src_rect != NULL){
		vg_lite_translate(src_rect->x / 1.0f,src_rect->y / 1.0f, &matrix);
	}else{
		vg_lite_translate(0 ,0 , &matrix);
	}

	vg_err = vg_lite_blit(source_image, &mid, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
	vg_lite_finish();
	if(vg_err != VG_LITE_SUCCESS) vg_lite_free(&mid);
	print_cvi_error_message(vg_err, cvi_error_message);

	vg_lite_free(&mid);
	return CVI_SUCCESS;
}

static CVI_U32 hor_mirror(vg_lite_buffer_t *source_image, vg_lite_rectangle_t *src_rect){
	vg_lite_buffer_t mid = { 0 };
	vg_lite_buffer_t mid1 = { 0 };
	vg_lite_matrix_t matrix;

	vg_lite_error_t vg_err;
	error_message_t cvi_error_message;

	if(src_rect != NULL){
		mid.width = src_rect->width;
		mid.height = src_rect->height;
		mid1.width = src_rect->width;
		mid1.height = src_rect->height;
	}else{
		mid.width = source_image->width;
		mid.height = source_image->height;
		mid1.width = source_image->width;
		mid1.height = source_image->height;
	}
	mid.format = source_image->format;
	mid1.format = source_image->format;

	vg_err = vg_lite_allocate(&mid);
	print_cvi_error_message(vg_err, cvi_error_message);

	vg_err = vg_lite_allocate(&mid1);
	print_cvi_error_message(vg_err, cvi_error_message);

	vg_err = vg_lite_set_mirror(VG_LITE_ORIENTATION_BOTTOM_TOP);
	if(vg_err != VG_LITE_SUCCESS){
		vg_lite_free(&mid);
		vg_lite_free(&mid1);
	}
	print_cvi_error_message(vg_err, cvi_error_message);

	vg_lite_identity(&matrix);
	vg_lite_scale(1.0f, 1.0f, &matrix);
	vg_lite_translate(0, 0, &matrix);

	vg_err = vg_lite_blit_rect(&mid, source_image, src_rect, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
	vg_lite_finish();
	if(vg_err != VG_LITE_SUCCESS){
		vg_lite_free(&mid);
		vg_lite_free(&mid1);
	}
	print_cvi_error_message(vg_err, cvi_error_message);
	vg_err = vg_lite_set_mirror(VG_LITE_ORIENTATION_TOP_BOTTOM);
	if(vg_err != VG_LITE_SUCCESS){
		vg_lite_free(&mid);
		vg_lite_free(&mid1);
	}
	print_cvi_error_message(vg_err, cvi_error_message);
	vg_lite_identity(&matrix);
	vg_lite_scale(1.0f, 1.0f, &matrix);
	vg_lite_translate(mid1.width / 1.0f, mid1.height / 1.0f, &matrix);
	vg_lite_rotate(180.0f, &matrix);

	vg_err = vg_lite_blit(&mid1, &mid, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
	vg_lite_finish();
	if(vg_err != VG_LITE_SUCCESS){
		vg_lite_free(&mid);
		vg_lite_free(&mid1);
	}
	print_cvi_error_message(vg_err, cvi_error_message);
	vg_err = vg_lite_set_mirror(VG_LITE_ORIENTATION_TOP_BOTTOM);
	if(vg_err != VG_LITE_SUCCESS){
		vg_lite_free(&mid);
		vg_lite_free(&mid1);
	}
	print_cvi_error_message(vg_err, cvi_error_message);

	vg_lite_identity(&matrix);
	vg_lite_scale(1.0f, 1.0f, &matrix);
	if(src_rect != NULL){
		vg_lite_translate(src_rect->x / 1.0f,src_rect->y / 1.0f, &matrix);
	}else{
		vg_lite_translate(0 ,0 , &matrix);
	}
	vg_err = vg_lite_blit(source_image, &mid1, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
	vg_lite_finish();
	if(vg_err != VG_LITE_SUCCESS){
		vg_lite_free(&mid);
		vg_lite_free(&mid1);
	}
	print_cvi_error_message(vg_err, cvi_error_message);

	vg_lite_free(&mid);
	vg_lite_free(&mid1);
	return CVI_SUCCESS;
}

static color_format_case_t get_color_format_case(vg_lite_buffer_format_t format){
	color_format_case_t format_case;
	switch (format)	{
	case VG_LITE_RGB565:
	case VG_LITE_BGR565:
	case VG_LITE_RGB888:
	case VG_LITE_BGR888:
		format_case = RGB;
		break;
	case VG_LITE_ARGB4444:
	case VG_LITE_ABGR4444:
	case VG_LITE_RGBA4444:
	case VG_LITE_BGRA4444:
	case VG_LITE_ARGB1555:
	case VG_LITE_ABGR1555:
	case VG_LITE_RGBA5551:
	case VG_LITE_BGRA5551:
	case VG_LITE_ARGB8565:
	case VG_LITE_ABGR8565:
	case VG_LITE_ARGB8888:
	case VG_LITE_ABGR8888:
	case VG_LITE_RGBA8888:
	case VG_LITE_BGRA8888:
		format_case = ARGB;
		break;
	default:
		format_case = ARGB;
		break;
	}
	return format_case;
}

static CVI_U32 get_alpha_mode_val(vg_lite_buffer_format_t format,
								const cvi_tde_opt* opt,
								vg_lite_global_alpha_t* mode,
								unsigned char* global_alpha)
{
	color_format_case_t format_case = ARGB;
	CVI_U32 flag = CVI_SUCCESS;
	format_case = get_color_format_case(format);
	if(format_case == RGB){
		if(opt->blend_opt.global_alpha_en){
			*(mode) = VG_LITE_GLOBAL;
			*(global_alpha) = opt->global_alpha;
			flag = CVI_SUCCESS;
			CVI_TDE_DBG("enable global alpha.\n");
		} else {
			flag = CVI_FAILURE;
			CVI_TDE_DBG("please enable global alpha\n");
		}
	}else if(format_case == ARGB){
		if(opt->blend_opt.pixel_alpha_en){
			*(mode) = VG_LITE_NORMAL;
			*(global_alpha) = opt->global_alpha;
			flag = CVI_SUCCESS;

			CVI_TDE_DBG("enable pixel alpha\n");

		}else if(opt->blend_opt.pixel_alpha_en == false &&
					opt->blend_opt.global_alpha_en == true){
			*(mode) = VG_LITE_GLOBAL;
			*(global_alpha) = opt->global_alpha;
			flag = CVI_SUCCESS;
			CVI_TDE_DBG("enable global alpha\n");
		}else{
			flag = CVI_FAILURE;
			CVI_TDE_DBG("please enable global alpha\n");
		}
	}else{
		flag = CVI_FAILURE;
		CVI_TDE_ERR("color format don't support\n");
	}

	CVI_TDE_DBG("alpha mode: %#x, global alpha value: %#x\n", *(mode), *(global_alpha));

	return flag;
}

static CVI_U32 clip_outside(vg_lite_buffer_t* dst_buf,
                            const cvi_tde_rect* dst_rect,
                            vg_lite_buffer_t* src_buf,
                            const cvi_tde_rect* src_rect,
                            const cvi_tde_rect* clip_rect,
                            vg_lite_matrix_t* matrix)
{
	vg_lite_buffer_t destination_image = { 0 };
	vg_lite_rectangle_t rect0 = { 0 };
	vg_lite_rectangle_t rect1 = { 0 };
	vg_lite_matrix_t matrix1 = { 0 };
	vg_lite_rectangle_t *rect1_ptr = NULL;
	vg_lite_float_t scale_x, scale_y;
	vg_lite_float_t delta_x, delta_y;
	bool intersect = false;

	vg_lite_error_t vg_err = VG_LITE_SUCCESS;
	error_message_t cvi_error_message;

	if(init_vg_middle_buf(&destination_image, dst_buf, NULL) != CVI_SUCCESS){
		CVI_TDE_ERR("[%s: %d]: init destination_image buffer failed\n", __func__, __LINE__);
		return CVI_FAILURE;
	}
	rectangle_intersection(dst_rect, clip_rect, &rect0, &intersect);

	CVI_TDE_DBG("clip subarea data: \n");
	CVI_TDE_DBG("subarea_x_start:%d ", rect0.x);
	CVI_TDE_DBG("subarea_y_start:%d ", rect0.y);
	CVI_TDE_DBG("subarea_width:%d ", rect0.width);
	CVI_TDE_DBG("subarea_height:%d \n", rect0.height);
	#ifdef CVI_TDE_DEBUG_ENABLE
	cvi_save_bmp("clip_outside_destination_image.bmp", (uint8_t *)destination_image.memory, destination_image.width, destination_image.height,destination_image.stride, CVI_TDE_COLOR_FORMAT_RGB565);
	#endif

	if(src_rect != NULL){
		rect1.x = src_rect->pos_x;
		rect1.y = src_rect->pos_y;
		rect1.width = src_rect->width;
		rect1.height = src_rect->height;
		rect1_ptr = &rect1;
	}else{
		rect1_ptr = NULL;
	}

	if(intersect == false) {
		vg_err = vg_lite_blit_rect(dst_buf, src_buf, rect1_ptr, matrix,VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
		vg_lite_finish();
		if(vg_err != VG_LITE_SUCCESS) vg_lite_free(&destination_image);
		print_cvi_error_message(vg_err, cvi_error_message);
	} else {
		vg_err = vg_lite_blit_rect(dst_buf, src_buf, rect1_ptr, matrix,VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
		vg_lite_finish();
		if(vg_err != VG_LITE_SUCCESS) vg_lite_free(&destination_image);
		print_cvi_error_message(vg_err, cvi_error_message);

		vg_lite_identity(&matrix1);
		scale_x = rect0.width * 1.0f / rect0.width;
		scale_y = rect0.height * 1.0f / rect0.height;
		vg_lite_scale(scale_x, scale_y, &matrix1);
		delta_x = rect0.x / scale_x;
		delta_y = rect0.y / scale_y;
		vg_lite_translate(delta_x, delta_y, &matrix1);

		vg_err = vg_lite_blit_rect(dst_buf, &destination_image, &rect0, &matrix1, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
		vg_lite_finish();
		if(vg_err != VG_LITE_SUCCESS) vg_lite_free(&destination_image);
		print_cvi_error_message(vg_err, cvi_error_message);
	}
	vg_lite_free(&destination_image);
	return CVI_SUCCESS;
}

static CVI_U32 clip_inside(vg_lite_buffer_t* dst_buf,
							const cvi_tde_rect* dst_rect,
							vg_lite_buffer_t* src_buf,
							const cvi_tde_rect* src_rect,
							const cvi_tde_rect* clip_rect,
							vg_lite_matrix_t* matrix) {
	vg_lite_buffer_t destination_image = { 0 };
	vg_lite_rectangle_t rect0 = { 0 };
	vg_lite_rectangle_t rect1 = { 0 };
	vg_lite_matrix_t matrix1 = { 0 };
	vg_lite_rectangle_t *rect1_ptr = NULL;
	vg_lite_float_t scale_x, scale_y;
	vg_lite_float_t delta_x, delta_y;
	bool intersect = false;

	vg_lite_error_t vg_err = VG_LITE_SUCCESS;
	error_message_t cvi_error_message;

	if(init_vg_middle_buf(&destination_image, dst_buf, NULL) != CVI_SUCCESS){
		return CVI_FAILURE;
	}

	rectangle_intersection(dst_rect, clip_rect, &rect0, &intersect);

	CVI_TDE_DBG("clip subarea data: \n");
	CVI_TDE_DBG("subarea_x_start:%d ", rect0.x);
	CVI_TDE_DBG("subarea_y_start:%d ", rect0.y);
	CVI_TDE_DBG("subarea_width:%d ", rect0.width);
	CVI_TDE_DBG("subarea_height:%d \n", rect0.height);
	#ifdef CVI_TDE_DEBUG_ENABLE
	cvi_save_bmp("clip_inside_destination_image.bmp", (uint8_t *)destination_image.memory, destination_image.width, destination_image.height,destination_image.stride, CVI_TDE_COLOR_FORMAT_RGB565);
	#endif

	if(src_rect != NULL){
		rect1.x = src_rect->pos_x;
		rect1.y = src_rect->pos_y;
		rect1.width = src_rect->width;
		rect1.height = src_rect->height;
		rect1_ptr = &rect1;
	}else{
		rect1_ptr = NULL;
	}

	if(intersect == false) {
		return CVI_SUCCESS;
	} else {
		vg_err = vg_lite_blit_rect(&destination_image, src_buf, rect1_ptr, matrix,VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
		vg_lite_finish();
		if(vg_err != VG_LITE_SUCCESS) vg_lite_free(&destination_image);
		print_cvi_error_message(vg_err, cvi_error_message);
		vg_lite_identity(&matrix1);
		scale_x = rect0.width * 1.0f / rect0.width;
		scale_y = rect0.height * 1.0f / rect0.height;
		vg_lite_scale(scale_x, scale_y, &matrix1);
		delta_x = rect0.x / scale_x;
		delta_y = rect0.y / scale_y;
		vg_lite_translate(delta_x, delta_y, &matrix1);

		vg_err = vg_lite_blit_rect(dst_buf, &destination_image, &rect0, &matrix1, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
		vg_lite_finish();
		if(vg_err != VG_LITE_SUCCESS) vg_lite_free(&destination_image);
		print_cvi_error_message(vg_err, cvi_error_message);
	}
	vg_lite_free(&destination_image);

    return CVI_SUCCESS;
}

static CVI_U32 init_vg_middle_buf(vg_lite_buffer_t* mid_buf,
								vg_lite_buffer_t* src_buf,
								vg_lite_rectangle_t* src_rect) {

	vg_lite_matrix_t matrix;
	vg_lite_float_t scale_x, scale_y;
	vg_lite_float_t delta_x, delta_y;

	error_message_t cvi_error_message;
	vg_lite_error_t vg_err = VG_LITE_SUCCESS;

	if(src_rect == NULL){
		mid_buf->width = src_buf->width;
		mid_buf->height = src_buf->height;
	}else{
		mid_buf->width = src_rect->width;
		mid_buf->height = src_rect->height;
	}
	mid_buf->stride = src_buf->stride;
	mid_buf->format = src_buf->format;
	vg_err = vg_lite_allocate(mid_buf);
	print_cvi_error_message(vg_err, cvi_error_message);
	vg_lite_identity(&matrix);
	scale_x = 1.0f;
	scale_y = 1.0f;
	vg_lite_scale(scale_x, scale_y, &matrix);
	delta_x = 0 / scale_x;
	delta_y = 0 / scale_y;
	vg_lite_translate(delta_x, delta_y, &matrix);
	vg_err = vg_lite_blit_rect(mid_buf, src_buf, src_rect, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
	vg_lite_finish();
	print_cvi_error_message(vg_err, cvi_error_message);
	return CVI_SUCCESS;
}

static CVI_U32 alpha_blending(vg_lite_buffer_t* bg_buf, vg_lite_buffer_t* fg_buf, const cvi_tde_opt* opt){
	vg_lite_error_t vg_err = VG_LITE_SUCCESS;
	error_message_t cvi_error_message;

	unsigned char bg_global_alpha = 0;
	vg_lite_global_alpha_t bg_alpha_mode = 0;
	unsigned char fg_global_alpha = 0;
	vg_lite_global_alpha_t fg_alpha_mode = 0;

	vg_lite_matrix_t matrix = { 0 };
	vg_lite_float_t scale_x, scale_y;
	vg_lite_float_t delta_x, delta_y;

	vg_lite_blend_t blend = 0;
	#if (program_test == 1)
		vg_lite_color_key4_t colorkey = {
			{0, 0, 0, 0, 0, 0 ,0, 0},
			{0, 0, 0, 0, 0, 0 ,0, 0},
			{0, 0, 0, 0, 0, 0 ,0, 0},
			{0, 0, 0, 0, 0, 0 ,0, 0},
		};
		#if (colorkey_bug_resolve == 1)
			vg_lite_image_mode_t fg_image_mode;
			vg_lite_image_mode_t bg_image_mode;
			vg_lite_transparency_t fg_image_transparency;
			vg_lite_transparency_t bg_image_transparency;
		#endif
	#endif
	// get background image alpha mode && alpha value
	if(get_alpha_mode_val(bg_buf->format, opt,
							&bg_alpha_mode, &bg_global_alpha) != CVI_SUCCESS){
		CVI_TDE_ERR("background image alpha mode set failed\n");
		return CVI_FAILURE;
	}
	vg_lite_dest_global_alpha(bg_alpha_mode, bg_global_alpha);
	//get foreground image alpha mode && alpha value
	if(get_alpha_mode_val(fg_buf->format, opt,
							&fg_alpha_mode, &fg_global_alpha) != CVI_SUCCESS){
		CVI_TDE_ERR("foreground image alpha mode set failed\n");
		return CVI_FAILURE;
	}
	vg_lite_source_global_alpha(fg_alpha_mode, fg_global_alpha);
	/* set color key*/
	if(opt->colorkey_mode == CVI_TDE_COLORKEY_MODE_FG) {
		#if (program_test == 1)
			CVI_TDE_DBG("This is the colorkey testing function\n");
			#if(colorkey_bug_resolve == 1)
				get_colorkey_value_to_vg_lite(colorkey, opt);
				enable_colorkey(colorkey);
				colorkey_alpha_set_zeros(colorkey);
				vg_lite_set_color_key(colorkey);
				vg_lite_identity(&matrix);
				vg_err = vg_lite_blit(fg_buf, fg_buf, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
				vg_lite_finish();
				print_cvi_error_message(vg_err, cvi_error_message);
				fg_image_mode = fg_buf->image_mode;
				bg_image_mode = fg_buf->transparency_mode;
				fg_image_transparency = bg_buf->image_mode;
				bg_image_transparency = bg_buf->transparency_mode;

				fg_buf->image_mode = VG_LITE_NORMAL_IMAGE_MODE;
				fg_buf->transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
				bg_buf->image_mode = VG_LITE_NORMAL_IMAGE_MODE;
				bg_buf->transparency_mode = VG_LITE_IMAGE_TRANSPARENT;

				unenable_colorkey(colorkey);
				vg_lite_set_color_key(colorkey);
				vg_lite_identity(&matrix);
				scale_x = bg_buf->width * 1.0f / fg_buf->width;
				scale_y = bg_buf->height * 1.0f / fg_buf->height;
				vg_lite_scale(scale_x, scale_y, &matrix);
				delta_x = 0 / scale_x;
				delta_y = 0 / scale_y;
				vg_lite_translate(delta_x, delta_y, &matrix);
				blend = cvi_tde_blend_to_vglite_blend(opt->blend_opt.blend_cmd);
				vg_err = vg_lite_blit(bg_buf, fg_buf, &matrix, blend, 0, VG_LITE_FILTER_POINT);
				print_cvi_error_message(vg_err, cvi_error_message);
				vg_lite_finish();
				fg_buf->image_mode = fg_image_mode;
				fg_buf->transparency_mode = bg_image_mode;
				bg_buf->image_mode = fg_image_transparency;
				bg_buf->transparency_mode = bg_image_transparency;
			#endif
			#if (colorkey_bug_resolve == 2)
				get_colorkey_value_to_vg_lite(colorkey, opt);
				enable_colorkey(colorkey);
				colorkey_alpha_set_zeros(colorkey);
				vg_lite_set_color_key(colorkey);
				vg_lite_identity(&matrix);
				scale_x = bg_buf->width * 1.0f / fg_buf->width;
				scale_y = bg_buf->height * 1.0f / fg_buf->height;
				vg_lite_scale(scale_x, scale_y, &matrix);
				delta_x = 0 / scale_x;
				delta_y = 0 / scale_y;
				vg_lite_translate(delta_x, delta_y, &matrix);
				blend = cvi_tde_blend_to_vglite_blend(opt->blend_opt.blend_cmd);
				vg_err = vg_lite_blit(bg_buf, fg_buf, &matrix, blend, 0, VG_LITE_FILTER_POINT);
				print_cvi_error_message(vg_err, cvi_error_message);
				vg_lite_finish();
			#endif
		#endif
	} else {
		vg_lite_identity(&matrix);
		scale_x = bg_buf->width * 1.0f / fg_buf->width;
		scale_y = bg_buf->height * 1.0f / fg_buf->height;
		vg_lite_scale(scale_x, scale_y, &matrix);
		delta_x = 0 / scale_x;
		delta_y = 0 / scale_y;
		vg_lite_translate(delta_x, delta_y, &matrix);
		blend = cvi_tde_blend_to_vglite_blend(opt->blend_opt.blend_cmd);
		vg_err = vg_lite_blit(bg_buf, fg_buf, &matrix, blend, 0, VG_LITE_FILTER_POINT);
		print_cvi_error_message(vg_err, cvi_error_message);
		vg_lite_finish();
	}

	return CVI_SUCCESS;
}

static void compute_resize_matrix(vg_lite_matrix_t* matrix,
								vg_lite_rectangle_t* src_rect,
								vg_lite_rectangle_t* dst_rect)
{
	vg_lite_float_t scale_x, scale_y;
	vg_lite_float_t delta_x, delta_y;
	vg_lite_identity(matrix);
	scale_x = dst_rect->width * 1.0f / src_rect->width;
	scale_y = dst_rect->height * 1.0f / src_rect->height;
	vg_lite_scale(scale_x, scale_y, matrix);
	delta_x = dst_rect->x / scale_x;
	delta_y = dst_rect->y / scale_y;
	vg_lite_translate(delta_x, delta_y, matrix);
}

static void vg_lite_error_to_cvi_tde_error(vg_lite_error_t vg_lite_error, error_message_t* cvi_error_message)
{
	cvi_error_message->message_index = -1;
	switch (vg_lite_error)
	{
	case VG_LITE_INVALID_ARGUMENT:
		cvi_error_message->cvi_error_code = CVI_ERR_TDE_INVALID_PARAM;
		cvi_error_message->message_index = cvi_errcode_index(CVI_ERR_TDE_INVALID_PARAM);
		break;
	case VG_LITE_NOT_SUPPORT:
		cvi_error_message->cvi_error_code = CVI_ERR_TDE_UNSUPPORTED_OPERATION;
		cvi_error_message->message_index = cvi_errcode_index(CVI_ERR_TDE_UNSUPPORTED_OPERATION);
		break;
	case VG_LITE_OUT_OF_MEMORY:
		cvi_error_message->cvi_error_code = CVI_ERR_TDE_NO_MEM;
		cvi_error_message->message_index = cvi_errcode_index(CVI_ERR_TDE_NO_MEM);
		break;
	case VG_LITE_TIMEOUT:
		cvi_error_message->cvi_error_code = CVI_ERR_TDE_JOB_TIMEOUT;
		cvi_error_message->message_index = cvi_errcode_index(CVI_ERR_TDE_JOB_TIMEOUT);
		break;
	case VG_LITE_NOT_ALIGNED:
		cvi_error_message->cvi_error_code = CVI_ERR_TDE_NOT_ALIGNED;
		cvi_error_message->message_index = cvi_errcode_index(CVI_ERR_TDE_NOT_ALIGNED);
		break;
	default:
		cvi_error_message->cvi_error_code = CVI_FAILURE;
		cvi_error_message->message_index = 14;
		break;
	}
}

#if (program_test == 1)
static void get_colorkey_value_to_vg_lite(vg_lite_color_key4_t colorkey, const cvi_tde_opt* opt){
    const cvi_tde_colorkey_component* cvi_colorkey_val = NULL;
    cvi_colorkey_val = &(opt->colorkey_value.argb_colorkey.red);
    if(cvi_colorkey_val->is_component_ignore == false){
        if(cvi_colorkey_val->is_component_out == false){
            colorkey[0].low_r = cvi_colorkey_val->min_component;
            colorkey[0].hign_r = cvi_colorkey_val->max_component;
        }else{
            colorkey[0].low_r = 0;
            colorkey[0].hign_r = cvi_colorkey_val->min_component;
            colorkey[1].low_r = cvi_colorkey_val->max_component;
            colorkey[1].hign_r = 0xFF;
        }
    }else{
        colorkey[0].low_r = 0X00;
        colorkey[0].hign_r = 0xFF;
    }

    cvi_colorkey_val = &(opt->colorkey_value.argb_colorkey.green);
    if(cvi_colorkey_val->is_component_ignore == false){
        if(cvi_colorkey_val->is_component_out == false){
            colorkey[0].low_g = cvi_colorkey_val->min_component;
            colorkey[0].hign_g = cvi_colorkey_val->max_component;
        }else{
            colorkey[0].low_g = 0;
            colorkey[0].hign_g = cvi_colorkey_val->min_component;
            colorkey[1].low_g = cvi_colorkey_val->max_component;
            colorkey[1].hign_g = 0xFF;
        }
    }else{
        colorkey[0].low_g = 0X00;
        colorkey[0].hign_g = 0xFF;
    }

    cvi_colorkey_val = &(opt->colorkey_value.argb_colorkey.blue);
    if(cvi_colorkey_val->is_component_ignore == false){
        if(cvi_colorkey_val->is_component_out == false){
            colorkey[0].low_b = cvi_colorkey_val->min_component;
            colorkey[0].hign_b = cvi_colorkey_val->max_component;
        }else{
            colorkey[0].low_b = 0;
            colorkey[0].hign_b = cvi_colorkey_val->min_component;
            colorkey[1].low_b = cvi_colorkey_val->max_component;
            colorkey[1].hign_b = 0xFF;
        }
    }else{
        colorkey[0].low_b = 0X00;
        colorkey[0].hign_b = 0xFF;
    }
}
#endif