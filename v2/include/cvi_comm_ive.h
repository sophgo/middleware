#ifndef _CVI_COMM_IVE_H_
#define _CVI_COMM_IVE_H_

typedef void *IVE_HANDLE;

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#include "cvi_type.h"
#include "cvi_errno.h"

typedef unsigned char CVI_U0Q8;
typedef unsigned char CVI_U1Q7;
typedef unsigned char CVI_U5Q3;
typedef unsigned short CVI_U0Q16;
typedef unsigned short CVI_U4Q12;
typedef unsigned short CVI_U6Q10;
typedef unsigned short CVI_U8Q8;
typedef unsigned short CVI_U9Q7;
typedef unsigned short CVI_U12Q4;
typedef unsigned short CVI_U14Q2;
typedef unsigned short CVI_U5Q11;
typedef short CVI_S9Q7;
typedef short CVI_S14Q2;
typedef short CVI_S1Q15;
typedef unsigned int CVI_U22Q10;
typedef unsigned int CVI_U25Q7;
typedef unsigned int CVI_U21Q11;
typedef int CVI_S25Q7;
typedef int CVI_S16Q16;
typedef unsigned short CVI_U8Q4F4;
typedef float CVI_FLOAT;
typedef double CVI_DOUBLE;

/* IVE Image Type*/
typedef enum _IVE_IMAGE_TYPE_E {
	IVE_IMAGE_TYPE_U8C1 = 0x0,
	IVE_IMAGE_TYPE_S8C1 = 0x1,

	IVE_IMAGE_TYPE_YUV420SP = 0x2, /*YUV420 SemiPlanar*/
	IVE_IMAGE_TYPE_YUV422SP = 0x3, /*YUV422 SemiPlanar*/
	IVE_IMAGE_TYPE_YUV420P = 0x4, /*YUV420 Planar */
	IVE_IMAGE_TYPE_YUV422P = 0x5, /*YUV422 planar */

	IVE_IMAGE_TYPE_S8C2_PACKAGE = 0x6,
	IVE_IMAGE_TYPE_S8C2_PLANAR = 0x7,

	IVE_IMAGE_TYPE_S16C1 = 0x8,
	IVE_IMAGE_TYPE_U16C1 = 0x9,

	IVE_IMAGE_TYPE_U8C3_PACKAGE = 0xa,
	IVE_IMAGE_TYPE_U8C3_PLANAR = 0xb,

	IVE_IMAGE_TYPE_S32C1 = 0xc,
	IVE_IMAGE_TYPE_U32C1 = 0xd,

	IVE_IMAGE_TYPE_S64C1 = 0xe,
	IVE_IMAGE_TYPE_U64C1 = 0xf,

	IVE_IMAGE_TYPE_BF16C1 = 0x10,
	IVE_IMAGE_TYPE_FP32C1 = 0x11,
	IVE_IMAGE_TYPE_BUTT

} IVE_IMAGE_TYPE_E;

typedef struct CVI_IMG CVI_IMG_S;
/*
 * u64PhyAddr[3]: Image Phyaddr array
 * u64VirAddr[3]: Image Viraddr array
 * u32Width:  Image Width
 * u32Height: Image Height
 * u32Reserved: Reserved para
 */
typedef struct _IVE_IMAGE_S {
	IVE_IMAGE_TYPE_E enType;

	CVI_U64 u64PhyAddr[3];
	CVI_U64 u64VirAddr[3];
	CVI_U32 u32Stride[3];
	CVI_U32 u32Width;
	CVI_U32 u32Height;
	CVI_U32 u32Reserved;
} IVE_IMAGE_S;
typedef IVE_IMAGE_S IVE_SRC_IMAGE_S;
typedef IVE_IMAGE_S IVE_DST_IMAGE_S;

/*
 * u64PhyAddr: Memory Phyaddr
 * u64VirAddr: Memory Viraddr
 * u32Size:  Memory Size
 */
typedef struct _IVE_MEM_INFO_S {
	CVI_U64 u64PhyAddr;
	CVI_U64 u64VirAddr;
	CVI_U32 u32Size;
} IVE_MEM_INFO_S;
typedef IVE_MEM_INFO_S IVE_SRC_MEM_INFO_S;
typedef IVE_MEM_INFO_S IVE_DST_MEM_INFO_S;

/*
 * u64PhyAddr: Data Phyaddr
 * u64VirAddr: Data Viraddr
 * u32Width:  Data Width
 * u32Height: Data Height
 * u32Reserved: Reserved para
 */
typedef struct _IVE_DATA_S {
	CVI_U64 u64PhyAddr;
	CVI_U64 u64VirAddr;
	CVI_U32 u32Stride;
	CVI_U32 u32Width;
	CVI_U32 u32Height;
	CVI_U32 u32Reserved;
} IVE_DATA_S;
typedef IVE_DATA_S IVE_SRC_DATA_S;
typedef IVE_DATA_S IVE_DST_DATA_S;

/*
 * s8Val: signed 8bit num
 * u8Val: unsigned 8bit num
 */
typedef union _IVE_8BIT_U {
	CVI_S8 s8Val;
	CVI_U8 u8Val;
} IVE_8BIT_U;

/*
 * u16X: unsigned 16bit X coordinate
 * u16Y: unsigned 16bit Y coordinate
 */
typedef struct _IVE_POINT_U16_S {
	CVI_U16 u16X;
	CVI_U16 u16Y;
} IVE_POINT_U16_S;

/*
 * u16X: signed 16bit X coordinate
 * u16Y: signed 16bit Y coordinate
 */
typedef struct _IVE_POINT_S16_S {
	CVI_U16 s16X;
	CVI_U16 s16Y;
} IVE_POINT_S16_S;

/*
 * s25q7X:  X coordinate
 * s25q7X:  Y coordinate
 */
typedef struct _IVE_POINT_S25Q7_S {
	CVI_S25Q7 s25q7X; /*X coordinate*/
	CVI_S25Q7 s25q7Y; /*Y coordinate*/
} IVE_POINT_S25Q7_S;

/*
 * u16X: Rectangle X coordinate
 * u16Y: Rectangle Y coordinate
 * u32Width:  Rectangle Width
 * u32Height: Rectangle Height
 */
typedef struct _IVE_RECT_U16_S {
	CVI_U16 u16X;
	CVI_U16 u16Y;
	CVI_U16 u16Width;
	CVI_U16 u16Height;
} IVE_RECT_U16_S;

/*
 * stTable: LUT Table Memory
 * u16ElemNum: LUT's Elements Number
 * s32TabInLower: LUT's Original Input Lower Limit
 * s32TabInUpper: LUT's Original Input Upper Limit
 */
typedef struct _IVE_LOOK_UP_TABLE_S {
	IVE_MEM_INFO_S stTable;
	CVI_U16 u16ElemNum; /*LUT's elements number*/

	CVI_U8 u8TabInPreci;
	CVI_U8 u8TabOutNorm;

	CVI_S32 s32TabInLower; /*LUT's original input lower limit*/
	CVI_S32 s32TabInUpper; /*LUT's original input upper limit*/
} IVE_LOOK_UP_TABLE_S;

/* IVE Error Code Type */
typedef enum _EN_IVE_ERR_CODE_E {
	ERR_IVE_SYS_TIMEOUT = 0x40, /* IVE process timeout */
	ERR_IVE_QUERY_TIMEOUT = 0x41, /* IVE query timeout */
	ERR_IVE_OPEN_FILE = 0x42, /* IVE open file error */
	ERR_IVE_READ_FILE = 0x43, /* IVE read file error */
	ERR_IVE_WRITE_FILE = 0x44, /* IVE write file error */
	ERR_IVE_BUTT
} EN_IVE_ERR_CODE_E;

/* IVE Error Code Type Invalid Devid */
#define CVI_ERR_IVE_INVALID_DEVID                                              \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_DEVID)
/* IVE Error Code Type Invalid Chnid */
#define CVI_ERR_IVE_INVALID_CHNID                                              \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_CHNID)
/* IVE Error Code Type Illegal Parameter */
#define CVI_ERR_IVE_ILLEGAL_PARAM                                              \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
/* IVE Error Code Type File Exist */
#define CVI_ERR_IVE_EXIST                                                      \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, EN_ERR_EXIST)
/* IVE Error Code Type File Unexist */
#define CVI_ERR_IVE_UNEXIST                                                    \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, EN_ERR_UNEXIST)
/* IVE Error Code Type NULL Pointer */
#define CVI_ERR_IVE_NULL_PTR                                                   \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
/* IVE Error Code Type Pra Not Config */
#define CVI_ERR_IVE_NOT_CONFIG                                                 \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_CONFIG)
/* IVE Error Code Type Fucition Not Support */
#define CVI_ERR_IVE_NOT_SURPPORT                                               \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SUPPORT)
/* IVE Error Code Type Insufficient Permission */
#define CVI_ERR_IVE_NOT_PERM                                                   \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)
/* IVE Error Code Type Out of Memory*/
#define CVI_ERR_IVE_NOMEM                                                      \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)
/* IVE Error Code Type Has Not Enough Buffer*/
#define CVI_ERR_IVE_NOBUF                                                      \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, EN_ERR_NOBUF)
/* IVE Error Code Type Buffer Is Empty*/
#define CVI_ERR_IVE_BUF_EMPTY                                                  \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_EMPTY)
/* IVE Error Code Type Buffer Is Full*/
#define CVI_ERR_IVE_BUF_FULL                                                   \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_FULL)
/* IVE Error Code Type IVE Not Ready*/
#define CVI_ERR_IVE_NOTREADY                                                   \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
/* IVE Error Code Type Address Is Bad*/
#define CVI_ERR_IVE_BADADDR                                                    \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, EN_ERR_BADADDR)
/* IVE Error Code Type IVE Core Is Busy */
#define CVI_ERR_IVE_BUSY                                                       \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)
/* IVE Error Code Type System Processing Task Timeout */
#define CVI_ERR_IVE_SYS_TIMEOUT                                                \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, ERR_IVE_SYS_TIMEOUT)
/* IVE Error Code Type Query Task Timeout */
#define CVI_ERR_IVE_QUERY_TIMEOUT                                              \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, ERR_IVE_QUERY_TIMEOUT)
/* IVE Error Code Type Open File Fail */
#define CVI_ERR_IVE_OPEN_FILE                                                  \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, ERR_IVE_OPEN_FILE)
/* IVE Error Code Type Read File Fail */
#define CVI_ERR_IVE_READ_FILE                                                  \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, ERR_IVE_READ_FILE)
/* IVE Error Code Type Write File Fail */
#define CVI_ERR_IVE_WRITE_FILE                                                 \
	CVI_DEF_ERR(CVI_ID_IVE, EN_ERR_LEVEL_ERROR, ERR_IVE_WRITE_FILE)

//==============================================================================

#define IVE_HIST_NUM 256                 /* Histogram bin count for 8-bit grayscale images (0-255) */
#define IVE_MAP_NUM 256                  /* Number of mapping values, typically for color or intensity mapping */
#define IVE_MAX_REGION_NUM 254           /* Maximum number of regions that can be processed (1-254, reserved 0 and 255) */
#define IVE_ST_MAX_CORNER_NUM 500        /* Maximum number of corners for feature detection or tracking */


/* Enumeration of DMA modes for image processing */
typedef enum _IVE_DMA_MODE_E {
    IVE_DMA_MODE_DIRECT_COPY = 0x0,    /* Direct Copy Mode: Data is copied directly from the source to the destination without any processing */
    IVE_DMA_MODE_INTERVAL_COPY = 0x1,   /* Interval Copy Mode: Data may be selectively skipped during copying, suitable for specific application scenarios */
    IVE_DMA_MODE_SET_3BYTE = 0x2,       /* Set 3-Byte Mode: Transfers 3 bytes of data at a time, suitable for specific formats or protocols */
    IVE_DMA_MODE_SET_8BYTE = 0x3,       /* Set 8-Byte Mode: Transfers 8 bytes of data at a time, suitable for efficient data transfer */
    IVE_DMA_MODE_BUTT                    /* End Marker: Used as a boundary for the enumeration, typically not used as a valid mode */
} IVE_DMA_MODE_E;


typedef struct _IVE_DMA_CTRL_S {
	IVE_DMA_MODE_E enMode;
	CVI_U64 u64Val; /*Used in memset mode*/
	/*
	 * Used in interval-copy mode, every row was segmented by
	 * u8HorSegSize bytes, restricted in values of 2,3,4,8,16
	 */
	CVI_U8 u8HorSegSize;
	/*
	 * Used in interval-copy mode, the valid bytes copied
	 * in front of every segment in a valid row, w_ch 0<u8ElemSize<u8HorSegSize
	 */
	CVI_U8 u8ElemSize;
	CVI_U8 u8VerSegRows; /*Used in interval-copy mode, copy one row in every u8VerSegRows*/
} IVE_DMA_CTRL_S;

/* Structure to control filter parameters for image processing */
typedef struct _IVE_FILTER_CTRL_S {
    CVI_S8 as8Mask[25]; /* Template parameter filter coefficient */
    CVI_U8 u8Norm;      /* Normalization parameter, by right s_ft */
} IVE_FILTER_CTRL_S;



typedef enum _IVE_CSC_MODE_E {
	IVE_CSC_MODE_VIDEO_BT601_YUV2RGB =
		0x0, /*CSC: YUV2RGB, video transfer mode, RGB value range [16, 235]*/
	IVE_CSC_MODE_VIDEO_BT709_YUV2RGB =
		0x1, /*CSC: YUV2RGB, video transfer mode, RGB value range [16, 235]*/
	IVE_CSC_MODE_PIC_BT601_YUV2RGB =
		0x2, /*CSC: YUV2RGB, picture transfer mode, RGB value range [0, 255]*/
	IVE_CSC_MODE_PIC_BT709_YUV2RGB =
		0x3, /*CSC: YUV2RGB, picture transfer mode, RGB value range [0, 255]*/

	IVE_CSC_MODE_PIC_BT601_YUV2HSV =
		0x4, /*CSC: YUV2HSV, picture transfer mode, HSV value range [0, 255]*/
	IVE_CSC_MODE_PIC_BT709_YUV2HSV =
		0x5, /*CSC: YUV2HSV, picture transfer mode, HSV value range [0, 255]*/

	IVE_CSC_MODE_PIC_BT601_YUV2LAB =
		0x6, /*CSC: YUV2LAB, picture transfer mode, Lab value range [0, 255]*/
	IVE_CSC_MODE_PIC_BT709_YUV2LAB =
		0x7, /*CSC: YUV2LAB, picture transfer mode, Lab value range [0, 255]*/

	IVE_CSC_MODE_VIDEO_BT601_RGB2YUV =
		0x8, /*CSC: RGB2YUV, video transfer mode, YUV value range [0, 255]*/
	IVE_CSC_MODE_VIDEO_BT709_RGB2YUV =
		0x9, /*CSC: RGB2YUV, video transfer mode, YUV value range [0, 255]*/
	IVE_CSC_MODE_PIC_BT601_RGB2YUV =
		0xa, /*CSC: RGB2YUV, picture transfer mode, Y:[16, 235],U\V:[16, 240]*/
	IVE_CSC_MODE_PIC_BT709_RGB2YUV =
		0xb, /*CSC: RGB2YUV, picture transfer mode, Y:[16, 235],U\V:[16, 240]*/

	IVE_CSC_MODE_PIC_RGB2HSV = 0xb,
	IVE_CSC_MODE_PIC_RGB2GRAY = 0xc,
	IVE_CSC_MODE_BUTT
} IVE_CSC_MODE_E;

/*
 * enMode: Working mode
 */
typedef struct _IVE_CSC_CTRL_S {
    IVE_CSC_MODE_E enMode; /* Working mode */
} IVE_CSC_CTRL_S;


/*
 * enMode: CSC working mode
 * as8Mask: Template parameter filter coefficient
 * u8Norm: Normalization parameter, by right s_ft
 */
typedef struct _IVE_FILTER_AND_CSC_CTRL_S {
    IVE_CSC_MODE_E enMode;
    CVI_S8 as8Mask[25];
    CVI_U8 u8Norm;
} IVE_FILTER_AND_CSC_CTRL_S;


/* Sobel Output Control Type */
typedef enum _IVE_SOBEL_OUT_CTRL_E {
    IVE_SOBEL_OUT_CTRL_BOTH = 0x0, /* Output horizontal and vertical */
    IVE_SOBEL_OUT_CTRL_HOR = 0x1,  /* Output horizontal */
    IVE_SOBEL_OUT_CTRL_VER = 0x2,  /* Output vertical */
    IVE_SOBEL_OUT_CTRL_BUTT
} IVE_SOBEL_OUT_CTRL_E;


/*
 * enOutCtrl: Output format
 * as8Mask: Template parameter
 */
typedef struct _IVE_SOBEL_CTRL_S {
    IVE_SOBEL_OUT_CTRL_E enOutCtrl;
    CVI_S8 as8Mask[25];
} IVE_SOBEL_CTRL_S;


/* Magnitude and Angle Output Control Type */
typedef enum _IVE_MAG_AND_ANG_OUT_CTRL_E {
    IVE_MAG_AND_ANG_OUT_CTRL_MAG = 0x0,          /* Only the magnitude is output. */
    IVE_MAG_AND_ANG_OUT_CTRL_MAG_AND_ANG = 0x1,  /* The magnitude and angle are output. */
    IVE_MAG_AND_ANG_OUT_CTRL_BUTT
} IVE_MAG_AND_ANG_OUT_CTRL_E;


/*
 * enOutCtrl: Output control for magnitude and angle
 * u16Thr: Threshold value
 * as8Mask: Template parameter
 */
typedef struct _IVE_MAG_AND_ANG_CTRL_S {
    IVE_MAG_AND_ANG_OUT_CTRL_E enOutCtrl;
    CVI_U16 u16Thr;
    CVI_S8 as8Mask[25];
} IVE_MAG_AND_ANG_CTRL_S;



/*
 * au8Mask: The template parameter value must be 0 or 255.
 */
typedef struct _IVE_DILATE_CTRL_S {
    CVI_U8 au8Mask[25];
} IVE_DILATE_CTRL_S;

typedef IVE_DILATE_CTRL_S IVE_ERODE_CTRL_S;

typedef enum _IVE_THRESH_MODE_E {
	/*srcVal <= lowThr, dstVal = minVal; srcVal > lowThr, dstVal = maxVal.*/
	IVE_THRESH_MODE_BINARY = 0x0,
	/*srcVal <= lowThr, dstVal = srcVal; srcVal > lowThr, dstVal = maxVal.*/
	IVE_THRESH_MODE_TRUNC = 0x1,
	/*srcVal <= lowThr, dstVal = minVal; srcVal > lowThr, dstVal = srcVal.*/
	IVE_THRESH_MODE_TO_MINVAL = 0x2,
	/*
	 * srcVal <= lowThr, dstVal = minVal;  lowThr < srcVal <= _ghThr,
	 * dstVal = midVal; srcVal > _ghThr, dstVal = maxVal.
	 */
	IVE_THRESH_MODE_MIN_MID_MAX = 0x3,
	/*
	 * srcVal <= lowThr, dstVal = srcVal;  lowThr < srcVal <= _ghThr,
	 * dstVal = midVal; srcVal > _ghThr, dstVal = maxVal.
	 */
	IVE_THRESH_MODE_ORI_MID_MAX = 0x4,
	/*
	 * srcVal <= lowThr, dstVal = minVal;  lowThr < srcVal <= _ghThr,
	 * dstVal = midVal; srcVal > _ghThr, dstVal = srcVal.
	 */
	IVE_THRESH_MODE_MIN_MID_ORI = 0x5,
	/*
	 * srcVal <= lowThr, dstVal = minVal;  lowThr < srcVal <= _ghThr,
	 * dstVal = srcVal; srcVal > _ghThr, dstVal = maxVal.
	 */
	IVE_THRESH_MODE_MIN_ORI_MAX = 0x6,
	/*
	 * srcVal <= lowThr, dstVal = srcVal;  lowThr < srcVal <= _ghThr,
	 * dstVal = midVal; srcVal > _ghThr, dstVal = srcVal.
	 */
	IVE_THRESH_MODE_ORI_MID_ORI = 0x7,

	IVE_THRESH_MODE_BUTT
} IVE_THRESH_MODE_E;

typedef struct _IVE_THRESH_CTRL_S {
	IVE_THRESH_MODE_E enMode;
	CVI_U8 u8LowThr; /*user-defined threshold,  0<=u8LowThr<=255 */
	/*
	 * user-defined threshold, if enMode<IVE_THRESH_MODE_MIN_MID_MAX,
	 * u8HighThr is not used, else 0<=u8LowThr<=u8HighThr<=255;
	 */
	CVI_U8 u8HighThr;
	CVI_U8 u8MinVal; /*Minimum value when tri-level thresholding*/
	CVI_U8 u8MidVal; /*Middle value when tri-level thresholding, if enMode<2, u32MidVal is not used; */
	CVI_U8 u8MaxVal; /*Maxmum value when tri-level thresholding*/
} IVE_THRESH_CTRL_S;

/* Subtraction Mode Type */
typedef enum _IVE_SUB_MODE_E {
    IVE_SUB_MODE_ABS = 0x0,            /* Absolute value of the difference */
    IVE_SUB_MODE_SHIFT = 0x1,          /* The output result is obtained by shifting the result one digit right to reserve the signed bit */
    IVE_SUB_MODE_BUTT
} IVE_SUB_MODE_E;


/*
 * enMode: Subtraction mode
 */
typedef struct _IVE_SUB_CTRL_S {
    IVE_SUB_MODE_E enMode;
} IVE_SUB_CTRL_S;


/* Integration Output Control Type */
typedef enum _IVE_INTEG_OUT_CTRL_E {
    IVE_INTEG_OUT_CTRL_COMBINE = 0x0,  /* Combine the results */
    IVE_INTEG_OUT_CTRL_SUM = 0x1,      /* Sum of the results */
    IVE_INTEG_OUT_CTRL_SQSUM = 0x2,    /* Sum of squares of the results */
    IVE_INTEG_OUT_CTRL_BUTT
} IVE_INTEG_OUT_CTRL_E;


/*
 * enOutCtrl: Output control mode
 */
typedef struct _IVE_INTEG_CTRL_S {
    IVE_INTEG_OUT_CTRL_E enOutCtrl;
} IVE_INTEG_CTRL_S;


/* Threshold Mode for S16 to S8/U8 Conversion */
typedef enum _IVE_THRESH_S16_MODE_E {
    IVE_THRESH_S16_MODE_S16_TO_S8_MIN_MID_MAX = 0x0,  /* Convert S16 to S8 using minimum, midpoint, and maximum values */
    IVE_THRESH_S16_MODE_S16_TO_S8_MIN_ORI_MAX = 0x1,  /* Convert S16 to S8 using minimum, original, and maximum values */
    IVE_THRESH_S16_MODE_S16_TO_U8_MIN_MID_MAX = 0x2,  /* Convert S16 to U8 using minimum, midpoint, and maximum values */
    IVE_THRESH_S16_MODE_S16_TO_U8_MIN_ORI_MAX = 0x3,  /* Convert S16 to U8 using minimum, original, and maximum values */

    IVE_THRESH_S16_MODE_BUTT
} IVE_THRESH_S16_MODE_E;


typedef struct _IVE_THRESH_S16_CTRL_S {
	IVE_THRESH_S16_MODE_E enMode;
	CVI_S16 s16LowThr; /*User-defined threshold*/
	CVI_S16 s16HighThr; /*User-defined threshold*/
	IVE_8BIT_U un8MinVal; /*Minimum value when tri-level thresholding*/
	IVE_8BIT_U un8MidVal; /*Middle value when tri-level thresholding*/
	IVE_8BIT_U un8MaxVal; /*Maxmum value when tri-level thresholding*/
} IVE_THRESH_S16_CTRL_S;

/* Threshold Mode for U16 to U8 Conversion */
typedef enum _IVE_THRESH_U16_MODE_E {
    IVE_THRESH_U16_MODE_U16_TO_U8_MIN_MID_MAX = 0x0,  /* Convert U16 to U8 using minimum, midpoint, and maximum values */
    IVE_THRESH_U16_MODE_U16_TO_U8_MIN_ORI_MAX = 0x1,  /* Convert U16 to U8 using minimum, original, and maximum values */

    IVE_THRESH_U16_MODE_BUTT
} IVE_THRESH_U16_MODE_E;


/*
 * enMode: Threshold mode
 * u16LowThr: Low threshold value
 * u16HighThr: High threshold value
 * u8MinVal: Minimum value
 * u8MidVal: Middle value
 * u8MaxVal: Maximum value
 */
typedef struct _IVE_THRESH_U16_CTRL_S {
    IVE_THRESH_U16_MODE_E enMode;
    CVI_U16 u16LowThr;
    CVI_U16 u16HighThr;
    CVI_U8 u8MinVal;
    CVI_U8 u8MidVal;
    CVI_U8 u8MaxVal;
} IVE_THRESH_U16_CTRL_S;


/* 16-bit to 8-bit Conversion Mode */
typedef enum _IVE_16BIT_TO_8BIT_MODE_E {
    IVE_16BIT_TO_8BIT_MODE_S16_TO_S8 = 0x0,              /* Convert signed 16-bit (S16) to signed 8-bit (S8) */
    IVE_16BIT_TO_8BIT_MODE_S16_TO_U8_ABS = 0x1,         /* Convert signed 16-bit (S16) to unsigned 8-bit (U8) using absolute values */
    IVE_16BIT_TO_8BIT_MODE_S16_TO_U8_BIAS = 0x2,        /* Convert signed 16-bit (S16) to unsigned 8-bit (U8) with bias adjustment */
    IVE_16BIT_TO_8BIT_MODE_U16_TO_U8 = 0x3,              /* Convert unsigned 16-bit (U16) to unsigned 8-bit (U8) */

    IVE_16BIT_TO_8BIT_MODE_BUTT
} IVE_16BIT_TO_8BIT_MODE_E;


/*
 * enMode: Conversion mode from 16-bit to 8-bit
 * u16Denominator: Denominator for the conversion
 * u8Numerator: Numerator for the conversion
 * s8Bias: Bias value to adjust the conversion
 */
typedef struct _IVE_16BIT_TO_8BIT_CTRL_S {
    IVE_16BIT_TO_8BIT_MODE_E enMode;
    CVI_U16 u16Denominator;
    CVI_U8 u8Numerator;
    CVI_S8 s8Bias;
} IVE_16BIT_TO_8BIT_CTRL_S;


/* Order Statistic Filter Mode */
typedef enum _IVE_ORD_STAT_FILTER_MODE_E {
    IVE_ORD_STAT_FILTER_MODE_MEDIAN = 0x0,  /* Median filter mode */
    IVE_ORD_STAT_FILTER_MODE_MAX = 0x1,     /* Maximum filter mode */
    IVE_ORD_STAT_FILTER_MODE_MIN = 0x2,     /* Minimum filter mode */

    IVE_ORD_STAT_FILTER_MODE_BUTT
} IVE_ORD_STAT_FILTER_MODE_E;


/*
 * enMode: Mode of the order statistic filter
 */
typedef struct _IVE_ORD_STAT_FILTER_CTRL_S {
    IVE_ORD_STAT_FILTER_MODE_E enMode;
} IVE_ORD_STAT_FILTER_CTRL_S;


/* Mapping Mode */
typedef enum _IVE_MAP_MODE_E {
    IVE_MAP_MODE_U8 = 0x0,   /* 8-bit unsigned mapping mode */
    IVE_MAP_MODE_S16 = 0x1,  /* 16-bit signed mapping mode */
    IVE_MAP_MODE_U16 = 0x2,  /* 16-bit unsigned mapping mode */

    IVE_MAP_MODE_BUTT
} IVE_MAP_MODE_E;


/*
 * enMode: Mapping mode
 */
typedef struct _IVE_MAP_CTRL_S {
    IVE_MAP_MODE_E enMode;
} IVE_MAP_CTRL_S;


/*
 * au8Map: Lookup table for 8-bit mapping, size defined by IVE_MAP_NUM
 */
typedef struct _IVE_MAP_U8BIT_LUT_MEM_S {
    CVI_U8 au8Map[IVE_MAP_NUM];
} IVE_MAP_U8BIT_LUT_MEM_S;


/*
 * au16Map: Lookup table for 16-bit mapping, size defined by IVE_MAP_NUM
 */
typedef struct _IVE_MAP_U16BIT_LUT_MEM_S {
    CVI_U16 au16Map[IVE_MAP_NUM];
} IVE_MAP_U16BIT_LUT_MEM_S;
;

/*
 * as16Map: Lookup table for signed 16-bit mapping, size defined by IVE_MAP_NUM
 */
typedef struct _IVE_MAP_S16BIT_LUT_MEM_S {
    CVI_S16 as16Map[IVE_MAP_NUM];
} IVE_MAP_S16BIT_LUT_MEM_S;


/*
 * au32Hist: Histogram data array for equalization, size defined by IVE_HIST_NUM
 * au8Map: Mapping array for equalization, size defined by IVE_MAP_NUM
 */
typedef struct _IVE_EQUALIZE_HIST_CTRL_MEM_S {
    CVI_U32 au32Hist[IVE_HIST_NUM];
    CVI_U8 au8Map[IVE_MAP_NUM];
} IVE_EQUALIZE_HIST_CTRL_MEM_S;


/*
 * stMem: Memory information structure for histogram equalization control
 */
typedef struct _IVE_EQUALIZE_HIST_CTRL_S {
    IVE_MEM_INFO_S stMem;
} IVE_EQUALIZE_HIST_CTRL_S;

/*
 * u0q16X: X component of the equation "xA + yB"
 * u0q16Y: Y component of the equation "xA + yB"
 */
typedef struct _IVE_ADD_CTRL_S {
	CVI_U0Q16 u0q16X;
	CVI_U0Q16 u0q16Y;
} IVE_ADD_CTRL_S;

/*
 * u64Numerator: Numerator value for NCC (Normalized Cross-Correlation)
 * u64QuadSum1: Quadratic sum for the first input
 * u64QuadSum2: Quadratic sum for the second input
 * u8Reserved: Reserved space for future use (8 bytes)
 */
typedef struct _IVE_NCC_DST_MEM_S {
    CVI_U64 u64Numerator;
    CVI_U64 u64QuadSum1;
    CVI_U64 u64QuadSum2;
    CVI_U8 u8Reserved[8];
} IVE_NCC_DST_MEM_S;

/*
 * u32Area: Represented by the pixel number
 * u16Left: Circumscribed rectangle left border
 * u16Right: Circumscribed rectangle right border
 * u16Top: Circumscribed rectangle top border
 * u16Bottom: Circumscribed rectangle bottom border
 */
typedef struct _IVE_REGION_S {
	CVI_U32 u32Area;
	CVI_U16 u16Left;
	CVI_U16 u16Right;
	CVI_U16 u16Top;
	CVI_U16 u16Bottom;
} IVE_REGION_S;

/*
 * u16CurAreaThr: Threshold of the result regions' area
 * s8LabelStatus: -1: Labeled failed; 0: Labeled successfully
 * u8RegionNum: Number of valid regions, non-continuous stored
 *               Valid regions with 'u32Area > 0' and 'label = ArrayIndex + 1'
 * astRegion: Array of valid regions (up to IVE_MAX_REGION_NUM)
 */
typedef struct _IVE_CCBLOB_S {
	CVI_U16 u16CurAreaThr;
	CVI_S8 s8LabelStatus;
	CVI_U8 u8RegionNum;
	IVE_REGION_S astRegion[IVE_MAX_REGION_NUM];
} IVE_CCBLOB_S;

/*
 * IVE_CCL_MODE_4C: 4-connected
 * IVE_CCL_MODE_8C: 8-connected
 * IVE_CCL_MODE_BUTT: End of enumeration (not a valid mode)
 */
typedef enum _IVE_CCL_MODE_E {
	IVE_CCL_MODE_4C = 0x0,
	IVE_CCL_MODE_8C = 0x1,
	IVE_CCL_MODE_BUTT
} IVE_CCL_MODE_E;

/*
 * enMode: Mode of connected component labeling
 * u16InitAreaThr: Initial threshold of region area
 * u16Step: Increase area step for once
 */
typedef struct _IVE_CCL_CTRL_S {
	IVE_CCL_MODE_E enMode;
	CVI_U16 u16InitAreaThr;
	CVI_U16 u16Step;
} IVE_CCL_CTRL_S;

/*
 * u22q10NoiseVar: Initial noise variance
 * u22q10MaxVar: Maximum variance
 * u22q10MinVar: Minimum variance
 * u0q16LearnRate: Learning rate for the model
 * u0q16BgRatio: Background ratio
 * u8q8VarThr: Variance threshold
 * u0q16InitWeight: Initial weight for the model
 * u8ModelNum: Model number, can be 3 or 5
 */
typedef struct _IVE_GMM_CTRL_S {
    CVI_U22Q10 u22q10NoiseVar;
    CVI_U22Q10 u22q10MaxVar;
    CVI_U22Q10 u22q10MinVar;
    CVI_U0Q16 u0q16LearnRate;
    CVI_U0Q16 u0q16BgRatio;
    CVI_U8Q8 u8q8VarThr;
    CVI_U0Q16 u0q16InitWeight;
    CVI_U8 u8ModelNum;
} IVE_GMM_CTRL_S;


/* Gaussian Mixture Model 2 Sensitivity Factor Mode */
typedef enum _IVE_GMM2_SNS_FACTOR_MODE_E {
    IVE_GMM2_SNS_FACTOR_MODE_GLB = 0x0,  /* Global sensitivity factor mode */
    IVE_GMM2_SNS_FACTOR_MODE_PIX = 0x1,  /* Pixel sensitivity factor mode */

    IVE_GMM2_SNS_FACTOR_MODE_BUTT
} IVE_GMM2_SNS_FACTOR_MODE_E;

/* Gaussian Mixture Model 2 Life Update Factor Mode */
typedef enum _IVE_GMM2_LIFE_UPDATE_FACTOR_MODE_E {
    IVE_GMM2_LIFE_UPDATE_FACTOR_MODE_GLB = 0x0,  /* Global life update factor mode */
    IVE_GMM2_LIFE_UPDATE_FACTOR_MODE_PIX = 0x1,  /* Pixel life update factor mode */

    IVE_GMM2_LIFE_UPDATE_FACTOR_MODE_BUTT
} IVE_GMM2_LIFE_UPDATE_FACTOR_MODE_E;


/*
 * enSnsFactorMode: Sensitivity factor mode
 * enLifeUpdateFactorMode: Life update factor mode
 * u16GlbLifeUpdateFactor: Global life update factor (default: 4)
 * u16LifeThr: Life threshold (default: 5000)
 * u16FreqInitVal: Initial frequency (default: 20000)
 * u16FreqReduFactor: Frequency reduction factor (default: 0xFF00)
 * u16FreqAddFactor: Frequency adding factor (default: 0xEF)
 * u16FreqThr: Frequency threshold (default: 12000)
 * u16VarRate: Variation update rate (default: 1)
 * u9q7MaxVar: Max variation (default: (16 * 16)<<7)
 * u9q7MinVar: Min variation (default: ( 8 *  8)<<7)
 * u8GlbSnsFactor: Global sensitivity factor (default: 8)
 * u8ModelNum: Model number (range: 1~5, default: 3)
 */
typedef struct _IVE_GMM2_CTRL_S {
    IVE_GMM2_SNS_FACTOR_MODE_E enSnsFactorMode;
    IVE_GMM2_LIFE_UPDATE_FACTOR_MODE_E enLifeUpdateFactorMode;
    CVI_U16 u16GlbLifeUpdateFactor;
    CVI_U16 u16LifeThr;
    CVI_U16 u16FreqInitVal;
    CVI_U16 u16FreqReduFactor;
    CVI_U16 u16FreqAddFactor;
    CVI_U16 u16FreqThr;
    CVI_U16 u16VarRate;
    CVI_U9Q7 u9q7MaxVar;
    CVI_U9Q7 u9q7MinVar;
    CVI_U8 u8GlbSnsFactor;
    CVI_U8 u8ModelNum;
} IVE_GMM2_CTRL_S;


/*
 * stMem: Memory information structure
 * u16LowThr: Low threshold for edge detection
 * u16HighThr: High threshold for edge detection
 * as8Mask: Mask for Canny edge detection
 */
typedef struct _IVE_CANNY_HYS_EDGE_CTRL_S {
    IVE_MEM_INFO_S stMem;
    CVI_U16 u16LowThr;
    CVI_U16 u16HighThr;
    CVI_S8 as8Mask[25];
} IVE_CANNY_HYS_EDGE_CTRL_S;


/*
 * u32StackSize: Stack size for output
 * u8Reserved: Reserved space for 16 byte alignment
 */
typedef struct _IVE_CANNY_STACK_SIZE_S {
    CVI_U32 u32StackSize;
    CVI_U8 u8Reserved[12];
} IVE_CANNY_STACK_SIZE_S;


/* Local Binary Pattern Comparison Mode */
typedef enum _IVE_LBP_CMP_MODE_E {
    IVE_LBP_CMP_MODE_NORMAL = 0x0,  /* P(x) - P(center) >= un8BitThr.s8Val, s(x) = 1; else s(x) = 0; */
    IVE_LBP_CMP_MODE_ABS = 0x1,     /* Abs(P(x) - P(center)) >= un8BitThr.u8Val, s(x) = 1; else s(x) = 0; */

    IVE_LBP_CMP_MODE_BUTT
} IVE_LBP_CMP_MODE_E;


/*
 * enMode: LBP comparison mode
 * un8BitThr: 8-bit threshold for LBP
 */
typedef struct _IVE_LBP_CTRL_S {
    IVE_LBP_CMP_MODE_E enMode;
    IVE_8BIT_U un8BitThr;
} IVE_LBP_CTRL_S;


/* Normalized Gradient Output Control */
typedef enum _IVE_NORM_GRAD_OUT_CTRL_E {
    IVE_NORM_GRAD_OUT_CTRL_HOR_AND_VER = 0x0,  /* Horizontal and Vertical gradient output */
    IVE_NORM_GRAD_OUT_CTRL_HOR = 0x1,          /* Horizontal gradient output */
    IVE_NORM_GRAD_OUT_CTRL_VER = 0x2,          /* Vertical gradient output */
    IVE_NORM_GRAD_OUT_CTRL_COMBINE = 0x3,     /* Combined gradient output */

    IVE_NORM_GRAD_OUT_CTRL_BUTT
} IVE_NORM_GRAD_OUT_CTRL_E;

/*
 * enOutCtrl: Normalized gradient output control
 * as8Mask: Mask for gradient calculation
 * u8Norm: Normalization factor
 */
typedef struct _IVE_NORM_GRAD_CTRL_S {
    IVE_NORM_GRAD_OUT_CTRL_E enOutCtrl;
    CVI_S8 as8Mask[25];
    CVI_U8 u8Norm;
} IVE_NORM_GRAD_CTRL_S;


/*
 * enSubMode: Subtraction mode for frame difference
 * enThrMode: Thresholding mode
 * u8ThrLow: User-defined low threshold
 * u8ThrHigh: User-defined high threshold
 * u8ThrMinVal: Minimum value for tri-level thresholding
 * u8ThrMidVal: Middle value for tri-level thresholding
 * u8ThrMaxVal: Maximum value for tri-level thresholding
 * au8ErodeMask: Erosion mask
 * au8DilateMask: Dilation mask
 */
typedef struct _IVE_FRAME_DIFF_MOTION_CTRL_S {
    IVE_SUB_MODE_E enSubMode;
    IVE_THRESH_MODE_E enThrMode;
    CVI_U8 u8ThrLow;
    CVI_U8 u8ThrHigh;
    CVI_U8 u8ThrMinVal;
    CVI_U8 u8ThrMidVal;
    CVI_U8 u8ThrMaxVal;
    CVI_U8 au8ErodeMask[25];
    CVI_U8 au8DilateMask[25];
} IVE_FRAME_DIFF_MOTION_CTRL_S;



/*
 * u16MaxEig: Output of the second step in S_-Tomasi method
 * u8Reserved: Reserved for alignment
 */
typedef struct _IVE_ST_MAX_EIG_S {
    CVI_U16 u16MaxEig;
    CVI_U8 u8Reserved[14];
} IVE_ST_MAX_EIG_S;


/*
 * stMem: Memory information for corner detection
 * u0q8QualityLevel: Quality level for corner detection
 */
typedef struct _IVE_ST_CANDI_CORNER_CTRL_S {
    IVE_MEM_INFO_S stMem;
    CVI_U0Q8 u0q8QualityLevel;
} IVE_ST_CANDI_CORNER_CTRL_S;


/*
 * u16CornerNum: Number of detected corners
 * astCorner: Array of detected corner points
 */
typedef struct _IVE_ST_CORNER_INFO_S {
    CVI_U16 u16CornerNum;
    IVE_POINT_U16_S astCorner[IVE_ST_MAX_CORNER_NUM];
} IVE_ST_CORNER_INFO_S;


/*
 * u16MaxCornerNum: Maximum number of corners to detect
 * u16MinDist: Minimum distance between detected corners
 */
typedef struct _IVE_ST_CORNER_CTRL_S {
    CVI_U16 u16MaxCornerNum;
    CVI_U16 u16MinDist;
} IVE_ST_CORNER_CTRL_S;


typedef enum _IVE_GRAD_FG_MODE_E {
	IVE_GRAD_FG_MODE_USE_CUR_GRAD = 0x0,
	IVE_GRAD_FG_MODE_FIND_MIN_GRAD = 0x1,

	IVE_GRAD_FG_MODE_BUTT
} IVE_GRAD_FG_MODE_E;

/*
 * enMode: Calculation mode
 * u16EdwFactor: Edge width adjustment factor (range: 500 to 2000; default: 1000)
 * u8CrlCoefThr: Gradient vector correlation coefficient threshold (ranges: 50 to 100; default: 80)
 * u8MagCrlThr: Gradient amplitude threshold (range: 0 to 20; default: 4)
 * u8MinMagDiff: Gradient magnitude difference threshold (range: 2 to 8; default: 2)
 * u8NoiseVal: Gradient amplitude noise threshold (range: 1 to 8; default: 1)
 * u8EdwDark: Black pixels enable flag (range: 0 (no), 1 (yes); default: 1)
 */
typedef struct _IVE_GRAD_FG_CTRL_S {
    IVE_GRAD_FG_MODE_E enMode;
    CVI_U16 u16EdwFactor;
    CVI_U8 u8CrlCoefThr;
    CVI_U8 u8MagCrlThr;
    CVI_U8 u8MinMagDiff;
    CVI_U8 u8NoiseVal;
    CVI_U8 u8EdwDark;
} IVE_GRAD_FG_CTRL_S;


/*
 * u8q4f4Mean: Candidate background gray value
 * u16StartTime: Candidate Background start time
 * u16SumAccessTime: Candidate Background cumulative access time
 * u16ShortKeepTime: Candidate background short hold time
 * u8ChgCond: Time condition for candidate background into the changing state
 * u8PotenBgLife: Potential background cumulative access time
 */
typedef struct _IVE_CANDI_BG_PIX_S {
    CVI_U8Q4F4 u8q4f4Mean;
    CVI_U16 u16StartTime;
    CVI_U16 u16SumAccessTime;
    CVI_U16 u16ShortKeepTime;
    CVI_U8 u8ChgCond;
    CVI_U8 u8PotenBgLife;
} IVE_CANDI_BG_PIX_S;


/*
 * u8q4f4Mean: 0# background gray value
 * u16AccTime: Background cumulative access time
 * u8PreGray: Gray value of last pixel
 * u5q3DiffThr: Differential threshold
 * u8AccFlag: Background access flag
 * u8BgGray: 1# ~ 3# background gray values
 */
typedef struct _IVE_WORK_BG_PIX_S {
    CVI_U8Q4F4 u8q4f4Mean;
    CVI_U16 u16AccTime;
    CVI_U8 u8PreGray;
    CVI_U5Q3 u5q3DiffThr;
    CVI_U8 u8AccFlag;
    CVI_U8 u8BgGray[3];
} IVE_WORK_BG_PIX_S;


/*
 * u8WorkBgLife: 1# ~ 3# background vitality
 * u8CandiBgLife: Candidate background vitality
 */
typedef struct _IVE_BG_LIFE_S {
    CVI_U8 u8WorkBgLife[3];
    CVI_U8 u8CandiBgLife;
} IVE_BG_LIFE_S;


/*
 * stWorkBgPixel: Working background
 * stCandiPixel: Candidate background
 * stBgLife: Background vitality
 */
typedef struct _IVE_BG_MODEL_PIX_S {
    IVE_WORK_BG_PIX_S stWorkBgPixel;
    IVE_CANDI_BG_PIX_S stCandiPixel;
    IVE_BG_LIFE_S stBgLife;
} IVE_BG_MODEL_PIX_S;


/*
 * u32PixNum: Number of pixels
 * u32SumLum: Sum of luminance
 * u8Reserved: Reserved for future use
 */
typedef struct _IVE_FG_STAT_DATA_S {
    CVI_U32 u32PixNum;
    CVI_U32 u32SumLum;
    CVI_U8 u8Reserved[8];
} IVE_FG_STAT_DATA_S;


/*
 * u32PixNum: Number of pixels
 * u32SumLum: Sum of luminance
 * u8Reserved: Reserved for future use
 */
typedef struct _IVE_BG_STAT_DATA_S {
    CVI_U32 u32PixNum;
    CVI_U32 u32SumLum;
    CVI_U8 u8Reserved[8];
} IVE_BG_STAT_DATA_S;


/*
 * u32CurFrmNum: Current frame timestamp, in frame units
 * u32PreFrmNum: Previous frame timestamp, in frame units
 * u16TimeThr: Potential background replacement time threshold (range: 2 to 100 frames; default: 20)
 * u8DiffThrCrlCoef: Correlation coefficients between differential threshold and gray value (range: 0 to 5; default: 0)
 * u8DiffMaxThr: Maximum of background differential threshold (range: 3 to 15; default: 6)
 * u8DiffMinThr: Minimum of background differential threshold (range: 3 to 15; default: 4)
 * u8DiffThrInc: Dynamic Background differential threshold increment (range: 0 to 6; default: 0)
 * u8FastLearnRate: Quick background learning rate (range: 0 to 4; default: 2)
 * u8DetChgRegion: Whether to detect change region (range: 0 (no), 1 (yes); default: 0)
 */
typedef struct _IVE_MATCH_BG_MODEL_CTRL_S {
    CVI_U32 u32CurFrmNum;
    CVI_U32 u32PreFrmNum;
    CVI_U16 u16TimeThr;
    CVI_U8 u8DiffThrCrlCoef;
    CVI_U8 u8DiffMaxThr;
    CVI_U8 u8DiffMinThr;
    CVI_U8 u8DiffThrInc;
    CVI_U8 u8FastLearnRate;
    CVI_U8 u8DetChgRegion;
} IVE_MATCH_BG_MODEL_CTRL_S;


/*
 * u32CurFrmNum: Current frame timestamp, in frame units
 * u32PreChkTime: The last time when background status is checked
 * u32FrmChkPeriod: Background status checking period (range: 0 to 2000 frames; default: 50)
 * u32InitMinTime: Background initialization shortest time (range: 20 to 6000 frames; default: 100)
 * u32StyBgMinBlendTime: Steady background integration shortest time (range: 20 to 6000 frames; default: 200)
 * u32StyBgMaxBlendTime: Steady background integration longest time (range: 20 to 40000 frames; default: 1500)
 * u32DynBgMinBlendTime: Dynamic background integration shortest time (range: 0 to 6000 frames; default: 0)
 * u32StaticDetMinTime: Still detection shortest time (range: 20 to 6000 frames; default: 80)
 * u16FgMaxFadeTime: Foreground disappearing longest time (range: 1 to 255 seconds; default: 15)
 * u16BgMaxFadeTime: Background disappearing longest time (range: 1 to 255 seconds; default: 60)
 * u8StyBgAccTimeRateThr: Steady background access time ratio threshold (range: 10 to 100; default: 80)
 * u8ChgBgAccTimeRateThr: Change background access time ratio threshold (range: 10 to 100; default: 60)
 * u8DynBgAccTimeThr: Dynamic background access time ratio threshold (range: 0 to 50; default: 0)
 * u8DynBgDepth: Dynamic background depth (range: 0 to 3; default: 3)
 * u8BgEffStaRateThr: Background state time ratio threshold when initializing (range: 90 to 100; default: 90)
 * u8AcceBgLearn: Whether to accelerate background learning (range: 0 (no), 1 (yes); default: 0)
 * u8DetChgRegion: Whether to detect change region (range: 0 (no), 1 (yes); default: 0)
 */
typedef struct _IVE_UPDATE_BG_MODEL_CTRL_S {
    CVI_U32 u32CurFrmNum;
    CVI_U32 u32PreChkTime;
    CVI_U32 u32FrmChkPeriod;
    CVI_U32 u32InitMinTime;
    CVI_U32 u32StyBgMinBlendTime;
    CVI_U32 u32StyBgMaxBlendTime;
    CVI_U32 u32DynBgMinBlendTime;
    CVI_U32 u32StaticDetMinTime;
    CVI_U16 u16FgMaxFadeTime;
    CVI_U16 u16BgMaxFadeTime;
    CVI_U8 u8StyBgAccTimeRateThr;
    CVI_U8 u8ChgBgAccTimeRateThr;
    CVI_U8 u8DynBgAccTimeThr;
    CVI_U8 u8DynBgDepth;
    CVI_U8 u8BgEffStaRateThr;
    CVI_U8 u8AcceBgLearn;
    CVI_U8 u8DetChgRegion;
} IVE_UPDATE_BG_MODEL_CTRL_S;


/* Sum of Absolute Differences Mode */
typedef enum _IVE_SAD_MODE_E {
    IVE_SAD_MODE_MB_4X4 = 0x0,   /* 4x4 block mode */
    IVE_SAD_MODE_MB_8X8 = 0x1,   /* 8x8 block mode */
    IVE_SAD_MODE_MB_16X16 = 0x2,  /* 16x16 block mode */

    IVE_SAD_MODE_BUTT
} IVE_SAD_MODE_E;


/* SAD Output Control */
typedef enum _IVE_SAD_OUT_CTRL_E {
    IVE_SAD_OUT_CTRL_16BIT_BOTH = 0x0,  /* Output 16 bit SAD and threshold */
    IVE_SAD_OUT_CTRL_8BIT_BOTH = 0x1,   /* Output 8 bit SAD and threshold */
    IVE_SAD_OUT_CTRL_16BIT_SAD = 0x2,   /* Output 16 bit SAD */
    IVE_SAD_OUT_CTRL_8BIT_SAD = 0x3,    /* Output 8 bit SAD */
    IVE_SAD_OUT_CTRL_THRESH = 0x4,      /* Output threshold, 16 bits SAD */

    IVE_SAD_OUT_CTRL_BUTT
} IVE_SAD_OUT_CTRL_E;


/*
 * enMode: SAD mode selection
 * enOutCtrl: Output control settings
 * u16Thr: Threshold value; if srcVal <= u16Thr, dstVal = minVal; if srcVal > u16Thr, dstVal = maxVal.
 * u8MinVal: Minimum value
 * u8MaxVal: Maximum value
 */
typedef struct _IVE_SAD_CTRL_S {
    IVE_SAD_MODE_E enMode;
    IVE_SAD_OUT_CTRL_E enOutCtrl;
    CVI_U16 u16Thr;
    CVI_U8 u8MinVal;
    CVI_U8 u8MaxVal;
} IVE_SAD_CTRL_S;


/* Resize Mode */
typedef enum _IVE_RESIZE_MODE_E {
    IVE_RESIZE_MODE_LINEAR = 0x0,  /* Bilinear interpolation */
    IVE_RESIZE_MODE_AREA = 0x1,    /* Area-based (or super) interpolation */

    IVE_RESIZE_MODE_BUTT
} IVE_RESIZE_MODE_E;


/*
 * enMode: Resize mode selection
 * stMem: Memory information structure
 * u16Num: Number of resize operations
 */
typedef struct _IVE_RESIZE_CTRL_S {
    IVE_RESIZE_MODE_E enMode;
    IVE_MEM_INFO_S stMem;
    CVI_U16 u16Num;
} IVE_RESIZE_CTRL_S;


/* Bernsen Mode */
typedef enum _IVE_BERNSEN_MODE_E {
    IVE_BERNSEN_MODE_NORMAL = 0x0,  /* Simple Bernsen thresh */
    IVE_BERNSEN_MODE_THRESH = 0x1,   /* Thresh based on the global threshold and local Bernsen threshold */
    IVE_BERNSEN_MODE_PAPER = 0x2,    /* This method is same with original paper */

    IVE_BERNSEN_MODE_BUTT
} IVE_BERNSEN_MODE_E;


/*
 * enMode: Bernsen mode selection
 * u8WinSize: Window size, can be 3x3 or 5x5
 * u8Thr: Threshold value
 * u8ContrastThreshold: Contrast threshold for comparison with mid-gray
 */
typedef struct _IVE_BERNSEN_CTRL_S {
    IVE_BERNSEN_MODE_E enMode;
    CVI_U8 u8WinSize;
    CVI_U8 u8Thr;
    CVI_U8 u8ContrastThreshold;
} IVE_BERNSEN_CTRL_S;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /*_CVI_COMM_IVE_H*/
