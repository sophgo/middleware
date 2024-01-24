/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_tde.h
 * Description:
 *   tde interfaces.
 */

#ifndef CVI_COMMON_TDE_H
#define CVI_COMMON_TDE_H

#include "cvi_type.h"
#include "cvi_tde_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* tde start err no. */
#define CVI_ERR_TDE_BASE ((CVI_S32)(((0x80UL + 0x20UL) << 24) | (100 << 16) | (4 << 13) | 1))

typedef enum {
    CVI_ERR_TDE_DEV_NOT_OPEN = CVI_ERR_TDE_BASE, /* tde device not open yet */
    CVI_ERR_TDE_DEV_OPEN_FAILED,                /* open tde device failed */
    CVI_ERR_TDE_NULL_PTR,                       /* input parameters contain null ptr */
    CVI_ERR_TDE_NO_MEM,                         /* malloc failed  */
    CVI_ERR_TDE_INVALID_HANDLE,                 /* invalid job handle */
    CVI_ERR_TDE_INVALID_PARAM,                  /* invalid parameter */
    CVI_ERR_TDE_NOT_ALIGNED,                    /* aligned error for position, stride, width */
    CVI_ERR_TDE_MINIFICATION,                   /* invalid minification */
    CVI_ERR_TDE_CLIP_AREA,                      /* clip area and operation area have no intersection */
    CVI_ERR_TDE_JOB_TIMEOUT,                    /* blocked job wait timeout */
    CVI_ERR_TDE_UNSUPPORTED_OPERATION,          /* unsupported operation */
    CVI_ERR_TDE_QUERY_TIMEOUT,                  /* query time out */
    CVI_ERR_TDE_INTERRUPT,                      /* blocked job was interrupted */
    CVI_ERR_TDE_BUTT,
} cvi_tde_err_code;

/* RGB and packet YUV formats and semi-planar YUV format */
typedef enum {
    CVI_TDE_COLOR_FORMAT_RGB444 = 0,       /* RGB444 format */
    CVI_TDE_COLOR_FORMAT_BGR444,           /* BGR444 format */
    CVI_TDE_COLOR_FORMAT_RGB555,           /* RGB555 format */
    CVI_TDE_COLOR_FORMAT_BGR555,           /* BGR555 format */
    CVI_TDE_COLOR_FORMAT_RGB565,           /* RGB565 format */
    CVI_TDE_COLOR_FORMAT_BGR565,           /* BGR565 format */
    CVI_TDE_COLOR_FORMAT_RGB888,           /* RGB888 format */
    CVI_TDE_COLOR_FORMAT_BGR888,           /* BGR888 format */
    CVI_TDE_COLOR_FORMAT_ARGB4444,         /* ARGB4444 format */
    CVI_TDE_COLOR_FORMAT_ABGR4444,         /* ABGR4444 format */
    CVI_TDE_COLOR_FORMAT_RGBA4444,         /* RGBA4444 format */
    CVI_TDE_COLOR_FORMAT_BGRA4444,         /* BGRA4444 format */
    CVI_TDE_COLOR_FORMAT_ARGB1555,         /* ARGB1555 format */
    CVI_TDE_COLOR_FORMAT_ABGR1555,         /* ABGR1555 format */
    CVI_TDE_COLOR_FORMAT_RGBA1555,         /* RGBA1555 format */
    CVI_TDE_COLOR_FORMAT_BGRA1555,         /* BGRA1555 format */
    CVI_TDE_COLOR_FORMAT_ARGB8565,         /* ARGB8565 format */
    CVI_TDE_COLOR_FORMAT_ABGR8565,         /* ABGR8565 format */
    CVI_TDE_COLOR_FORMAT_RGBA8565,         /* RGBA8565 format */
    CVI_TDE_COLOR_FORMAT_BGRA8565,         /* BGRA8565 format */
    CVI_TDE_COLOR_FORMAT_ARGB8888,         /* ARGB8888 format */
    CVI_TDE_COLOR_FORMAT_ABGR8888,         /* ABGR8888 format */
    CVI_TDE_COLOR_FORMAT_RGBA8888,         /* RGBA8888 format */
    CVI_TDE_COLOR_FORMAT_BGRA8888,         /* BGRA8888 format */
    CVI_TDE_COLOR_FORMAT_RABG8888,         /* RABG8888 format */
    /* 1-bit palette format without alpha component. Each pixel occupies one bit. */
    CVI_TDE_COLOR_FORMAT_CLUT1,
    /* 2-bit palette format without alpha component. Each pixel occupies two bits. */
    CVI_TDE_COLOR_FORMAT_CLUT2,
    /* 4-bit palette format without alpha component. Each pixel occupies four bits. */
    CVI_TDE_COLOR_FORMAT_CLUT4,
    /* 8-bit palette format without alpha component. Each pixel occupies eight bits. */
    CVI_TDE_COLOR_FORMAT_CLUT8,
    /* 4-bit palette format with alpha component. Each pixel occupies 8 bit. */
    CVI_TDE_COLOR_FORMAT_ACLUT44,
    /* 8-bit palette format with alpha component. Each pixel occupies 16 bit. */
    CVI_TDE_COLOR_FORMAT_ACLUT88,
    CVI_TDE_COLOR_FORMAT_A1,                 /* Alpha format. Each pixel occupies one bit. */
    CVI_TDE_COLOR_FORMAT_A8,                 /* Alpha format. Each pixel occupies eight bits. */
    CVI_TDE_COLOR_FORMAT_YCbCr888,           /* 33：YCbCr888 */
    CVI_TDE_COLOR_FORMAT_AYCbCr8888,         /* 34：AYCbCr8888 */
    CVI_TDE_COLOR_FORMAT_YCbCr422,           /* 35：PKYUYV */
    CVI_TDE_COLOR_FORMAT_PKGYVYU,            /* 36：PKYVYU */
    CVI_TDE_COLOR_FORMAT_PKGUYVY,            /* 37：PKUYVY */
    CVI_TDE_COLOR_FORMAT_PKGVYUY,            /* 38：PKVYUY */
    CVI_TDE_COLOR_FORMAT_PKGVUYY,            /* 39：PKVUYY */
    CVI_TDE_COLOR_FORMAT_PKGYYUV,            /* 40：PKYYUV */
    CVI_TDE_COLOR_FORMAT_PKGUVYY,            /* 41：PKUVYY */
    CVI_TDE_COLOR_FORMAT_PKGYYVU,            /* 42：PKYYVU */
    CVI_TDE_COLOR_FORMAT_JPG_YCbCr400MBP,    /* 43：SP400 */
    CVI_TDE_COLOR_FORMAT_JPG_YCbCr422MBHP,   /* 44：SP422h(1*2)(像素宽度为偶数) */
    CVI_TDE_COLOR_FORMAT_JPG_YCbCr422MBVP,   /* 45：SP422V(2*1)(像素高度为偶数) */
    CVI_TDE_COLOR_FORMAT_MP1_YCbCr420MBP,    /* 46：SP420(cd hi像素宽高为偶数) */
    CVI_TDE_COLOR_FORMAT_MP2_YCbCr420MBP,    /* 47：SP420(cd hi像素宽高为偶数) */
    CVI_TDE_COLOR_FORMAT_MP2_YCbCr420MBI,    /* 48：SP420(cd hi像素宽高为偶数) */
    CVI_TDE_COLOR_FORMAT_JPG_YCbCr420MBP,    /* 49：SP420(cd hi像素宽高为偶数) */
    CVI_TDE_COLOR_FORMAT_JPG_YCbCr444MBP,    /* 50：SP444(cd hi像素宽高为偶数) */
    CVI_TDE_COLOR_FORMAT_MAX                 /* 51：End of enumeration */
} cvi_tde_color_format;

/* Definition of the semi-planar YUV format */
typedef enum {
    CVI_TDE_MB_COLOR_FORMAT_JPG_YCbCr400MBP = 0, /* Semi-planar YUV400 format, for JPG decoding */
    /* Semi-planar YUV422 format, horizontal sampling, for JPG decoding */
    CVI_TDE_MB_COLOR_FORMAT_JPG_YCbCr422MBHP,
    CVI_TDE_MB_COLOR_FORMAT_JPG_YCbCr422MBVP, /* Semi-planar YUV422 format, vertical sampling, for JPG decoding */
    CVI_TDE_MB_COLOR_FORMAT_MP1_YCbCr420MBP,  /* Semi-planar YUV420 format */
    CVI_TDE_MB_COLOR_FORMAT_MP2_YCbCr420MBP,  /* Semi-planar YUV420 format */
    CVI_TDE_MB_COLOR_FORMAT_MP2_YCbCr420MBI,  /* Semi-planar YUV420 format */
    CVI_TDE_MB_COLOR_FORMAT_JPG_YCbCr420MBP,  /* Semi-planar YUV420 format, for JPG pictures */
    CVI_TDE_MB_COLOR_FORMAT_JPG_YCbCr444MBP,  /* Semi-planar YUV444 format, for JPG pictures */
    CVI_TDE_MB_COLOR_FORMAT_MAX
} cvi_tde_mb_color_format;

/* Structure of the bitmap information set by customers */
typedef struct {
    td_phys_addr_t phys_addr; /* Header address of a bitmap or the Y component */
    CVI_U32 phys_len;
    cvi_tde_color_format color_format; /* Color format */
    CVI_U32 height; /* Bitmap height */
    CVI_U32 width; /* Bitmap width */
    CVI_U32 stride; /* Stride of a bitmap or the Y component */
    CVI_BOOL is_ycbcr_clut; /* Whether the CLUT is in the YCbCr space. */
    CVI_BOOL alpha_max_is_255; /* The maximum alpha value of a bitmap is 255 or 128. */
    CVI_BOOL support_alpha_ex_1555; /* Whether to enable the alpha extension of an ARGB1555 bitmap. */
    CVI_U8 alpha0;        /* Values of alpha0 and alpha1, used as the ARGB1555 format */
    CVI_U8 alpha1;        /* Values of alpha0 and alpha1, used as the ARGB1555 format */
    td_phys_addr_t cbcr_phys_addr;    /* Address of the CbCr component, pilot */
    CVI_U32 cbcr_phys_len;
    CVI_U32 cbcr_stride;  /* Stride of the CbCr component, pilot */
    /* <Address of the color look-up table (CLUT), for color extension or color correction */
    td_phys_addr_t clut_phys_addr;
    CVI_U32 clut_phys_len;
} cvi_tde_surface;

/* Definition of the semi-planar YUV data */
typedef struct {
    cvi_tde_mb_color_format mb_color_format; /* YUV format */
    td_phys_addr_t y_addr;             /* Physical address of the Y component */
    CVI_U32 y_len;
    CVI_U32 y_width;            /* Width of the Y component */
    CVI_U32 y_height;           /* Height of the Y component */
    CVI_U32 y_stride;           /* Stride of the Y component, indicating bytes in each row */
    td_phys_addr_t cbcr_phys_addr;      /* Width of the UV component */
    CVI_U32 cbcr_phys_len;
    CVI_U32 cbcr_stride;        /* Stride of the UV component, indicating the bytes in each row */
} cvi_tde_mb_surface;

/* Definition of the TDE rectangle */
typedef struct {
    CVI_S32 pos_x;   /* Horizontal coordinate */
    CVI_S32 pos_y;   /* Vertical coordinate */
    CVI_U32 width;  /* Width */
    CVI_U32 height; /* Height */
} cvi_tde_rect;

/* dma module */
typedef struct {
    cvi_tde_surface *dst_surface;
    cvi_tde_rect *dst_rect;
} cvi_tde_none_src;

/* single source */
typedef struct {
    cvi_tde_surface *src_surface;
    cvi_tde_surface *dst_surface;
    cvi_tde_rect *src_rect;
    cvi_tde_rect *dst_rect;
} cvi_tde_single_src;

/* mb source */
typedef struct {
    cvi_tde_mb_surface *mb_surface;
    cvi_tde_surface *dst_surface;
    cvi_tde_rect *src_rect;
    cvi_tde_rect *dst_rect;
} cvi_tde_mb_src;

/* double source */
typedef struct {
    cvi_tde_surface *bg_surface;
    cvi_tde_surface *fg_surface;
    cvi_tde_surface *dst_surface;
    cvi_tde_rect *bg_rect;
    cvi_tde_rect *fg_rect;
    cvi_tde_rect *dst_rect;
} cvi_tde_double_src;

/* Logical operation type */
typedef enum {
    CVI_TDE_ALPHA_BLENDING_NONE = 0x0,     /* No alpha and raster of operation (ROP) blending */
    CVI_TDE_ALPHA_BLENDING_BLEND = 0x1,    /* Alpha blending */
    CVI_TDE_ALPHA_BLENDING_ROP = 0x2,      /* ROP blending */
    CVI_TDE_ALPHA_BLENDING_COLORIZE = 0x4, /* Colorize operation */
    CVI_TDE_ALPHA_BLENDING_MAX = 0x8       /* End of enumeration */
} cvi_tde_alpha_blending;

/* Definition of ROP codes */
typedef enum {
    CVI_TDE_ROP_BLACK = 0,   /* Blackness */
    CVI_TDE_ROP_NOTMERGEPEN, /* ~(S2 | S1) */
    CVI_TDE_ROP_MASKNOTPEN,  /* ~S2&S1 */
    CVI_TDE_ROP_NOTCOPYPEN,  /* ~S2 */
    CVI_TDE_ROP_MASKPENNOT,  /* S2&~S1 */
    CVI_TDE_ROP_NOT,         /* ~S1 */
    CVI_TDE_ROP_XORPEN,      /* S2^S1 */
    CVI_TDE_ROP_NOTMASKPEN,  /* ~(S2 & S1) */
    CVI_TDE_ROP_MASKPEN,     /* S2&S1 */
    CVI_TDE_ROP_NOTXORPEN,   /* ~(S2^S1) */
    CVI_TDE_ROP_NOP,         /* S1 */
    CVI_TDE_ROP_MERGENOTPEN, /* ~S2|S1 */
    CVI_TDE_ROP_COPYPEN,     /* S2 */
    CVI_TDE_ROP_MERGEPENNOT, /* S2|~S1 */
    CVI_TDE_ROP_MERGEPEN,    /* S2|S1 */
    CVI_TDE_ROP_WHITE,       /* Whiteness */
    CVI_TDE_ROP_MAX
} cvi_tde_rop_mode;

/* Definition of the blit mirror */
typedef enum {
    CVI_TDE_MIRROR_NONE = 0,   /* No mirror */
    CVI_TDE_MIRROR_HOR, /* Horizontal mirror */
    CVI_TDE_MIRROR_VER,   /* Vertical mirror */
    CVI_TDE_MIRROR_BOTH,       /* Horizontal and vertical mirror */
    CVI_TDE_MIRROR_MAX
} cvi_tde_mirror_mode;

/* Clip operation type */
typedef enum {
    CVI_TDE_CLIP_MODE_NONE = 0, /* No clip */
    CVI_TDE_CLIP_MODE_INSIDE,   /* Clip the data within the rectangle to output and discard others */
    CVI_TDE_CLIP_MODE_OUTSIDE,  /* Clip the data outside the rectangle to output and discard others */
    CVI_TDE_CLIP_MODE_MAX
} cvi_tde_clip_mode;

/* Scaling mode for the macroblock */
typedef enum {
    CVI_TDE_MB_RESIZE_NONE = 0,       /* No scaling */
    CVI_TDE_MB_RESIZE_QUALITY_LOW,    /* Low-quality scaling */
    CVI_TDE_MB_RESIZE_QUALITY_MIDDLE, /* Medium-quality scaling */
    CVI_TDE_MB_RESIZE_QUALITY_HIGH,   /* High-quality scaling */
    CVI_TDE_MB_RESIZE_MAX
} cvi_tde_mb_resize;

/* Definition of fill colors */
typedef struct {
    cvi_tde_color_format color_format; /* TDE pixel format */
    CVI_U32 color_value;         /* Fill colors that vary according to pixel formats */
} cvi_tde_fill_color;

/* Definition of colorkey modes */
typedef enum {
    CVI_TDE_COLORKEY_MODE_NONE = 0,   /* No colorkey */
    /* When performing the colorkey operation on the foreground bitmap,
    you need to perform this operation before the CLUT for color extension
    and perform this operation after the CLUT for color correction. */
    CVI_TDE_COLORKEY_MODE_FG,
    CVI_TDE_COLORKEY_MODE_BG, /* Perform the colorkey operation on the background bitmap */
    CVI_TDE_COLORKEY_MODE_MAX
} cvi_tde_colorkey_mode;

/* Definition of colorkey range */
typedef struct {
    CVI_U8 min_component;   /* Minimum value of a component */
    CVI_U8 max_component;   /* Maximum value of a component */
    CVI_U8 is_component_out;    /* The colorkey of a component is within or beyond the range. */
    CVI_U8 is_component_ignore; /* Whether to ignore a component. */
    CVI_U8 component_mask;  /* Component mask */
} cvi_tde_colorkey_component;

/* Definition of colorkey values */
typedef union {
    struct {
        cvi_tde_colorkey_component alpha; /* Alpha component */
        cvi_tde_colorkey_component red;   /* Red component */
        cvi_tde_colorkey_component green; /* Green component */
        cvi_tde_colorkey_component blue;  /* Blue component */
    } argb_colorkey;                     /* AUTO:cvi_tde_colorkey_mode:CVI_TDE_COLORKEY_MODE_NONE; */
    struct {
        cvi_tde_colorkey_component alpha; /* Alpha component */
        cvi_tde_colorkey_component y;     /* Y component */
        cvi_tde_colorkey_component cb;    /* Cb component */
        cvi_tde_colorkey_component cr;    /* Cr component */
    } ycbcr_colorkey;                    /* AUTO:cvi_tde_colorkey_mode:CVI_TDE_COLORKEY_MODE_FG; */
    struct {
        cvi_tde_colorkey_component alpha; /* Alpha component */
        cvi_tde_colorkey_component clut;  /* Palette component */
    } clut_colorkey;                     /* AUTO:cvi_tde_colorkey_mode:CVI_TDE_COLORKEY_MODE_BG; */
} cvi_tde_colorkey;

/* Definition of alpha output sources */
typedef enum {
    CVI_TDE_OUT_ALPHA_FROM_NORM = 0,    /* Output from the result of alpha blending or anti-flicker */
    CVI_TDE_OUT_ALPHA_FROM_BG,  /* Output from the background bitmap */
    CVI_TDE_OUT_ALPHA_FROM_FG,  /* Output from the foreground bitmap */
    CVI_TDE_OUT_ALPHA_FROM_GLOBALALPHA, /* Output from the global alpha */
    CVI_TDE_OUT_ALPHA_FROM_MAX
} cvi_tde_out_alpha_from;

/* Definition of filtering */
typedef enum {
    CVI_TDE_FILTER_MODE_COLOR = 0, /* Filter the color */
    CVI_TDE_FILTER_MODE_ALPHA,     /* Filter the alpha channel */
    CVI_TDE_FILTER_MODE_BOTH,      /* Filter the color and alpha channel */
    CVI_TDE_FILTER_MODE_NONE,      /* No filter */
    CVI_TDE_FILTER_MODE_MAX
} cvi_tde_filter_mode;

/* blend mode */
typedef enum {
    CVI_TDE_BLEND_ZERO = 0x0,
    CVI_TDE_BLEND_ONE,
    CVI_TDE_BLEND_SRC2COLOR,
    CVI_TDE_BLEND_INVSRC2COLOR,
    CVI_TDE_BLEND_SRC2ALPHA,
    CVI_TDE_BLEND_INVSRC2ALPHA,
    CVI_TDE_BLEND_SRC1COLOR,
    CVI_TDE_BLEND_INVSRC1COLOR,
    CVI_TDE_BLEND_SRC1ALPHA,
    CVI_TDE_BLEND_INVSRC1ALPHA,
    CVI_TDE_BLEND_SRC2ALPHASAT,
    CVI_TDE_BLEND_MAX
} cvi_tde_blend_mode;

/* Alpha blending command. You can set parameters or select Porter or Duff. */
/* pixel = (source * fs + destination * fd),
   sa = source alpha,
   da = destination alpha */
typedef enum {
    CVI_TDE_BLEND_CMD_NONE = 0x0, /* fs: sa      fd: 1.0-sa */
    CVI_TDE_BLEND_CMD_CLEAR,      /* fs: 0.0     fd: 0.0 */
    CVI_TDE_BLEND_CMD_SRC,        /* fs: 1.0     fd: 0.0 */
    CVI_TDE_BLEND_CMD_SRCOVER,    /* fs: 1.0     fd: 1.0-sa */
    CVI_TDE_BLEND_CMD_DSTOVER,    /* fs: 1.0-da  fd: 1.0 */
    CVI_TDE_BLEND_CMD_SRCIN,      /* fs: da      fd: 0.0 */
    CVI_TDE_BLEND_CMD_DSTIN,      /* fs: 0.0     fd: sa */
    CVI_TDE_BLEND_CMD_SRCOUT,     /* fs: 1.0-da  fd: 0.0 */
    CVI_TDE_BLEND_CMD_DSTOUT,     /* fs: 0.0     fd: 1.0-sa */
    CVI_TDE_BLEND_CMD_SRCATOP,    /* fs: da      fd: 1.0-sa */
    CVI_TDE_BLEND_CMD_DSTATOP,    /* fs: 1.0-da  fd: sa */
    CVI_TDE_BLEND_CMD_ADD,        /* fs: 1.0     fd: 1.0 */
    CVI_TDE_BLEND_CMD_XOR,        /* fs: 1.0-da  fd: 1.0-sa */
    CVI_TDE_BLEND_CMD_DST,        /* fs: 0.0     fd: 1.0 */
    CVI_TDE_BLEND_CMD_CONFIG,     /* You can set the parameteres. */
    CVI_TDE_BLEND_CMD_MAX
} cvi_tde_blend_cmd;

/* Options for the alpha blending operation */
typedef struct {
    CVI_BOOL global_alpha_en;       /* Global alpha enable */
    CVI_BOOL pixel_alpha_en;        /* Pixel alpha enable */
    CVI_BOOL src1_alpha_premulti;       /* Src1 alpha premultiply enable */
    CVI_BOOL src2_alpha_premulti;       /* Src2 alpha premultiply enable */
    cvi_tde_blend_cmd blend_cmd;        /* Alpha blending command */
    /* Src1 blending mode select. It is valid when blend_cmd is set to CVI_TDE_BLEND_CMD_CONFIG. */
    cvi_tde_blend_mode src1_blend_mode;
    /* Src2 blending mode select. It is valid when blend_cmd is set to CVI_TDE_BLEND_CMD_CONFIG. */
    cvi_tde_blend_mode src2_blend_mode;
} cvi_tde_blend_opt;

/* CSC parameter option */
typedef struct {
    CVI_BOOL src_csc_user_en;  /* User-defined ICSC parameter enable */
    CVI_BOOL src_csc_param_reload_en; /* User-defined ICSC parameter reload enable */
    CVI_BOOL dst_csc_user_en;  /* User-defined OCSC parameter enable */
    CVI_BOOL dst_csc_param_reload_en; /* User-defined OCSC parameter reload enable */
    td_phys_addr_t src_csc_param_addr; /* ICSC parameter address. The address must be 128-bit aligned. */
    CVI_S32 src_csc_param_len;
    td_phys_addr_t dst_csc_param_addr; /* OCSC parameter address. The address must be 128-bit aligned. */
    CVI_S32 dst_csc_param_len;
} cvi_tde_csc_opt;

/* Definition of blit operation options */
typedef struct {
    cvi_tde_alpha_blending alpha_blending_cmd; /* Logical operation type */

    cvi_tde_rop_mode rop_color; /* ROP type of the color space */

    cvi_tde_rop_mode rop_alpha; /* ROP type of the alpha component */

    cvi_tde_colorkey_mode colorkey_mode; /* Colorkey mode */

    cvi_tde_colorkey colorkey_value; /* Colorkey value */

    cvi_tde_clip_mode clip_mode; /* Perform the clip operation within or beyond the area */

    cvi_tde_rect clip_rect; /* Definition of the clipping area */

    CVI_BOOL resize; /* Whether to scale */

    cvi_tde_filter_mode filter_mode; /* Filtering mode during scaling */

    cvi_tde_mirror_mode mirror; /* Mirror type */

    CVI_BOOL clut_reload; /* Whether to reload the CLUT */

    CVI_U8 global_alpha; /* Global alpha value */

    cvi_tde_out_alpha_from out_alpha_from; /* Source of the output alpha */

    CVI_U32 color_resize; /* Colorize value */

    cvi_tde_blend_opt blend_opt;

    cvi_tde_csc_opt csc_opt;
    CVI_BOOL is_compress;
    CVI_BOOL is_decompress;
} cvi_tde_opt;

/* Definition of macroblock operation options */
typedef struct {
    cvi_tde_clip_mode clip_mode; /* Clip mode */

    cvi_tde_rect clip_rect; /* Definition of the clipping area */

    cvi_tde_mb_resize resize_en; /* Scaling information */

    /* If the alpha value is not set, the maximum alpha value is output by default. */
    CVI_BOOL is_set_out_alpha;

    CVI_U8 out_alpha; /* Global alpha for operation */
} cvi_tde_mb_opt;

/* Definition of the pattern filling operation */
typedef struct {
    cvi_tde_alpha_blending alpha_blending_cmd; /* Logical operation type */

    cvi_tde_rop_mode rop_color; /* ROP type of the color space */

    cvi_tde_rop_mode rop_alpha; /* ROP type of the alpha component */

    cvi_tde_colorkey_mode colorkey_mode; /* Colorkey mode */

    cvi_tde_colorkey colorkey_value; /* Colorkey value */

    cvi_tde_clip_mode clip_mode; /* Clip mode */

    cvi_tde_rect clip_rect; /* Clipping area */

    CVI_BOOL clut_reload; /* Whether to reload the CLUT */

    CVI_U8 global_alpha; /* Global alpha */

    cvi_tde_out_alpha_from out_alpha_from; /* Source of the output alpha */

    CVI_U32 color_resize; /* Colorize value */

    cvi_tde_blend_opt blend_opt; /* Options of the blending operation */

    cvi_tde_csc_opt csc_opt; /* CSC parameter option */
} cvi_tde_pattern_fill_opt;

/* Definition of rotation directions */
typedef enum {
    CVI_TDE_ROTATE_CLOCKWISE_90 = 0, /* Rotate 90 degree clockwise */
    CVI_TDE_ROTATE_CLOCKWISE_180,    /* Rotate 180 degree clockwise */
    CVI_TDE_ROTATE_CLOCKWISE_270,    /* Rotate 270 degree clockwise */
    CVI_TDE_ROTATE_MAX
} cvi_tde_rotate_angle;

/* Definition of corner_rect */
typedef struct {
    CVI_U32 width;
    CVI_U32 height;
    CVI_U32 inner_color;
    CVI_U32 outer_color;
} cvi_tde_corner_rect_info;

typedef struct {
    cvi_tde_rect *corner_rect_region;
    cvi_tde_corner_rect_info *corner_rect_info;
} cvi_tde_corner_rect;

typedef struct {
    CVI_S32 start_x;
    CVI_S32 start_y;
    CVI_S32 end_x;
    CVI_S32 end_y;
    CVI_U32 thick;
    CVI_U32 color;
} cvi_tde_line;

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* CVI_COMMON_TDE_H */
