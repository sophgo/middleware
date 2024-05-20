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

#define __func__ __FUNCTION__
#define IS_ERROR(status)         (status > 0)
#define CHECK_ERROR(Function)   \
    error = Function;           \
    if (IS_ERROR(error))        \
    {                           \
        printf("[%s: %d] failed.error type is %s\n", __func__, __LINE__, error_type[error]); \
        rc = CVI_FAILURE;       \
        goto ErrorHandler;      \
    }

#define CVI_MIN(A,B) (A < B ? A : B)
#define CVI_MAX(A,B) (A > B ? A : B)

static vg_lite_buffer_format_t cvi_tde_format_to_vglite_format(cvi_tde_color_format format);
static CVI_S32 cvi_draw_single_line(vg_lite_buffer_t *fb, const cvi_tde_line *line);

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

CVI_S32 cvi_tde_draw_corner_box(CVI_S32 handle,
                                const cvi_tde_surface *dst_surface,
                                const cvi_tde_corner_rect *corner_rect,
                                CVI_U32 num)
{
    UNUSED(handle);
    UNUSED(dst_surface);
    UNUSED(corner_rect);
    UNUSED(num);
    return 0;
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

    err = vg_lite_blit_rect(&dst_buf, &src_buf, &rect, &matrix,VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
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
    const cvi_tde_opt *opt)
{
    UNUSED(handle);
    UNUSED(single_src);
    UNUSED(fill_color);
    UNUSED(opt);
    return 0;
}

CVI_S32 cvi_tde_rotate(CVI_S32 handle, const cvi_tde_single_src *single_src, cvi_tde_rotate_angle rotate)
{
    UNUSED(handle);
    UNUSED(single_src);
    UNUSED(rotate);
    return 0;
}

CVI_S32 cvi_tde_bit_blit(CVI_S32 handle, const cvi_tde_double_src *double_src, const cvi_tde_opt *opt)
{
    UNUSED(handle);
    UNUSED(double_src);
    UNUSED(opt);
    return 0;
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
    case CVI_TDE_COLOR_FORMAT_RGB888:
        return VG_LITE_RGB888;
    case CVI_TDE_COLOR_FORMAT_RGBA8888:
        return VG_LITE_RGBA8888;
    case CVI_TDE_COLOR_FORMAT_BGRA8888:
        return VG_LITE_BGRA8888;
    case CVI_TDE_COLOR_FORMAT_RGB565:
        return VG_LITE_RGB565;
    case CVI_TDE_COLOR_FORMAT_BGR565:
        return VG_LITE_BGR565;
    case CVI_TDE_COLOR_FORMAT_RGBA4444:
        return VG_LITE_RGBA4444;
    case CVI_TDE_COLOR_FORMAT_BGRA4444:
        return VG_LITE_BGRA4444;
    case CVI_TDE_COLOR_FORMAT_A8:
        return VG_LITE_A8;
    default:
        return VG_LITE_ARGB8888;
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
                l_data_load[0] = l_readpixel_data[1];
                l_data_load[1] = l_readpixel_data[2];
                l_data_load[2] = l_readpixel_data[3];
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
