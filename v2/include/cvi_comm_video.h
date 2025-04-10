/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_comm_video.h
 * Description:
 *   Common video definitions.
 */

#ifndef __CVI_COMM_VIDEO_H__
#define __CVI_COMM_VIDEO_H__

#include "linux/types.h"
#include <cvi_common.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*check if pixel format is rgb type*/
#define IS_FMT_RGB(fmt) \
	((fmt == PIXEL_FORMAT_RGB_888) || (fmt == PIXEL_FORMAT_BGR_888) || \
	 (fmt == PIXEL_FORMAT_RGB_888_PLANAR) || (fmt == PIXEL_FORMAT_BGR_888_PLANAR))

/*check if pixel format is yuv type*/
#define IS_FMT_YUV(fmt) \
	((fmt == PIXEL_FORMAT_YUV_PLANAR_420) || (fmt == PIXEL_FORMAT_YUV_PLANAR_422) || \
	 (fmt == PIXEL_FORMAT_YUV_PLANAR_444) || (fmt == PIXEL_FORMAT_YUV_400) || \
	 (fmt == PIXEL_FORMAT_NV12) || (fmt == PIXEL_FORMAT_NV21) || \
	 (fmt == PIXEL_FORMAT_NV16) || (fmt == PIXEL_FORMAT_NV61) || \
	 (fmt == PIXEL_FORMAT_YUYV) || (fmt == PIXEL_FORMAT_UYVY) || \
	 (fmt == PIXEL_FORMAT_YVYU) || (fmt == PIXEL_FORMAT_VYUY) || \
	 (fmt == PIXEL_FORMAT_YUV_444))

/*check if pixel format is yuv420 type*/
#define IS_FMT_YUV420(fmt) \
	((fmt == PIXEL_FORMAT_YUV_PLANAR_420) || \
	 (fmt == PIXEL_FORMAT_NV12) || (fmt == PIXEL_FORMAT_NV21))

/*check if pixel format is yuv422 type*/
#define IS_FMT_YUV422(fmt) \
	((fmt == PIXEL_FORMAT_YUV_PLANAR_422) || \
	 (fmt == PIXEL_FORMAT_NV16) || (fmt == PIXEL_FORMAT_NV61) || \
	 (fmt == PIXEL_FORMAT_YUYV) || (fmt == PIXEL_FORMAT_UYVY) || \
	 (fmt == PIXEL_FORMAT_YVYU) || (fmt == PIXEL_FORMAT_VYUY))

/*check if frame offset valid*/
#define IS_FRAME_OFFSET_INVALID(f) \
	((f).s16OffsetLeft < 0 || (f).s16OffsetRight < 0 || \
	 (f).s16OffsetTop < 0 || (f).s16OffsetBottom < 0 || \
	 ((u32)((f).s16OffsetLeft + (f).s16OffsetRight) > (f).u32Width) || \
	 ((u32)((f).s16OffsetTop + (f).s16OffsetBottom) > (f).u32Height))

/*operation mode*/
typedef enum _OPERATION_MODE_E {
	OPERATION_MODE_AUTO = 0,	/*Automatic operation mode*/
	OPERATION_MODE_MANUAL = 1,	/*Manual operation mode*/
	OPERATION_MODE_BUTT
} OPERATION_MODE_E;

/*Angle of rotation*/
typedef enum _ROTATION_E {
	ROTATION_0 = 0,
	ROTATION_90,
	ROTATION_180,
	ROTATION_270,
	ROTATION_XY_FLIP,
	ROTATION_MAX
} ROTATION_E;

/* Mirror direction*/
typedef enum _MIRROR_TYPE_E {
    MIRDIR_TYPE_NONE,   /**< No mirroring */
    MIRDIR_TYPE_VER,    /**< Vertical mirroring */
    MIRDIR_TYPE_HOR,    /**< Horizontal mirroring */
    MIRDIR_TYPE_HOR_VER, /**< Horizontal and vertical mirroring */
	MIRDIR_TYPE_BUTT
} MIRROR_TYPE_E;

/*vb source*/
typedef enum _VB_SOURCE_E {
	VB_SOURCE_COMMON = 0,	/*vb from common pool*/
	VB_SOURCE_MODULE = 1,	/*vb from multi media module*/
	VB_SOURCE_PRIVATE = 2,	/*vb from private pool*/
	VB_SOURCE_USER = 3,	/*vb from user*/
	VB_SOURCE_BUTT
} VB_SOURCE_E;

/*border definition*/
typedef struct _BORDER_S {
	CVI_U32 u32TopWidth;	/*border width from top*/
	CVI_U32 u32BottomWidth;	/*border width from bottom*/
	CVI_U32 u32LeftWidth;	/*border width from left*/
	CVI_U32 u32RightWidth;	/*border width from right*/
	CVI_U32 u32Color;	/*border background color*/
} BORDER_S;

/*coordinate*/
typedef struct _POINT_S {
	CVI_S32 s32X;	/*abscissa*/
	CVI_S32 s32Y;	/*ordinate*/
} POINT_S;

/*size*/
typedef struct _SIZE_S {
	CVI_U32 u32Width;	/*width*/
	CVI_U32 u32Height;	/*height*/
} SIZE_S;

/*width, height, and position of the rectangle*/
typedef struct _RECT_S {
	CVI_S32 s32X;		/*X coordinate*/
	CVI_S32 s32Y;		/*Y coordinate*/
	CVI_U32 u32Width;	/*Width*/
	CVI_U32 u32Height;	/*Height*/
} RECT_S;

typedef struct _VIDEO_REGION_INFO_S {
	CVI_U32 u32RegionNum; /* W; count of the region */
	RECT_S *pstRegion; /* W; region attribute */
} VIDEO_REGION_INFO_S;

/*crop information*/
typedef struct _CROP_INFO_S {
	CVI_BOOL bEnable;	/*crop enable selection*/
	RECT_S stRect;		/*crop rectangular area*/
} CROP_INFO_S;

// ++++++++ If you want to change these interfaces, please contact the isp team. ++++++++
typedef struct _FRAME_RATE_CTRL_S {
	CVI_S32 s32SrcFrameRate; /* RW; source frame rate */
	CVI_S32 s32DstFrameRate; /* RW; dest frame rate */
} FRAME_RATE_CTRL_S;
// -------- If you want to change these interfaces, please contact the isp team. --------

/*
 * ASPECT_RATIO_NONE: full screen
 * ASPECT_RATIO_AUTO: Keep ratio, automatically get the region of video.
 * ASPECT_RATIO_MANUAL: Manully set the region of video.
 */
typedef enum _ASPECT_RATIO_E {
	ASPECT_RATIO_NONE = 0,
	ASPECT_RATIO_AUTO,
	ASPECT_RATIO_MANUAL,
	ASPECT_RATIO_MAX
} ASPECT_RATIO_E;

/*
 * enMode: aspect ratio mode: none/auto/manual
 * bEnableBgColor: fill bgcolor
 * u32BgColor: background color, RGB 888
 * stVideoRect: valid in ASPECT_RATIO_MANUAL mode
 */
typedef struct _ASPECT_RATIO_S {
	ASPECT_RATIO_E enMode;
	CVI_BOOL bEnableBgColor;
	CVI_U32 u32BgColor;
	RECT_S stVideoRect;
} ASPECT_RATIO_S;

// ++++++++ If you want to change these interfaces, please contact the isp team. ++++++++
typedef enum _PIXEL_FORMAT_E {
	PIXEL_FORMAT_RGB_888 = 0,
	PIXEL_FORMAT_BGR_888,
	PIXEL_FORMAT_RGB_888_PLANAR,
	PIXEL_FORMAT_BGR_888_PLANAR,

	PIXEL_FORMAT_ARGB_1555, // 4,
	PIXEL_FORMAT_ARGB_4444,
	PIXEL_FORMAT_ARGB_8888,

	PIXEL_FORMAT_RGB_BAYER_8BPP, // 7,
	PIXEL_FORMAT_RGB_BAYER_10BPP,
	PIXEL_FORMAT_RGB_BAYER_12BPP,
	PIXEL_FORMAT_RGB_BAYER_14BPP,
	PIXEL_FORMAT_RGB_BAYER_16BPP,

	PIXEL_FORMAT_YUV_PLANAR_422, // 12,
	PIXEL_FORMAT_YUV_PLANAR_420,
	PIXEL_FORMAT_YUV_PLANAR_444,
	PIXEL_FORMAT_YUV_400,

	PIXEL_FORMAT_HSV_888, // 16,
	PIXEL_FORMAT_HSV_888_PLANAR,

	PIXEL_FORMAT_NV12, // 18,
	PIXEL_FORMAT_NV21,
	PIXEL_FORMAT_NV16,
	PIXEL_FORMAT_NV61,
	PIXEL_FORMAT_YUYV, //22
	PIXEL_FORMAT_UYVY,
	PIXEL_FORMAT_YVYU,
	PIXEL_FORMAT_VYUY,
	PIXEL_FORMAT_YUV_444,

	PIXEL_FORMAT_FP32_C1 = 32, // 32
	PIXEL_FORMAT_FP32_C3_PLANAR,
	PIXEL_FORMAT_INT32_C1,
	PIXEL_FORMAT_INT32_C3_PLANAR,
	PIXEL_FORMAT_UINT32_C1,
	PIXEL_FORMAT_UINT32_C3_PLANAR,
	PIXEL_FORMAT_FP16_C1,
	PIXEL_FORMAT_FP16_C3_PLANAR,
	PIXEL_FORMAT_BF16_C1, // 40
	PIXEL_FORMAT_BF16_C3_PLANAR,
	PIXEL_FORMAT_INT16_C1,
	PIXEL_FORMAT_INT16_C3_PLANAR,
	PIXEL_FORMAT_UINT16_C1,
	PIXEL_FORMAT_UINT16_C3_PLANAR,
	PIXEL_FORMAT_INT8_C1,
	PIXEL_FORMAT_INT8_C3_PLANAR,
	PIXEL_FORMAT_UINT8_C1,
	PIXEL_FORMAT_UINT8_C3_PLANAR,

	PIXEL_FORMAT_8BIT_MODE = 50, //50

	PIXEL_FORMAT_MAX
} PIXEL_FORMAT_E;
// -------- If you want to change these interfaces, please contact the isp team. --------

/*
 * VIDEO_FORMAT_LINEAR: nature video line.
 */
// ++++++++ If you want to change these interfaces, please contact the isp team. ++++++++
typedef enum _VIDEO_FORMAT_E {
	VIDEO_FORMAT_LINEAR = 0,
	VIDEO_FORMAT_MAX
} VIDEO_FORMAT_E;
// -------- If you want to change these interfaces, please contact the isp team. --------

/*
 * COMPRESS_MODE_NONE: no compress.
 * COMPRESS_MODE_TILE: compress unit is a tile.
 * COMPRESS_MODE_LINE: compress unit is the whole line.
 * COMPRESS_MODE_FRAME: compress unit is the whole frame.
 */
// ++++++++ If you want to change these interfaces, please contact the isp team. ++++++++
typedef enum _COMPRESS_MODE_E {
	COMPRESS_MODE_NONE = 0,
	COMPRESS_MODE_TILE,
	COMPRESS_MODE_LINE,
	COMPRESS_MODE_FRAME,
	COMPRESS_MODE_BUTT
} COMPRESS_MODE_E;
// -------- If you want to change these interfaces, please contact the isp team. --------

// ++++++++ If you want to change these interfaces, please contact the isp team. ++++++++
typedef enum _BAYER_FORMAT_E {
	BAYER_FORMAT_BG = 0,
	BAYER_FORMAT_GB,
	BAYER_FORMAT_GR,
	BAYER_FORMAT_RG,
	/*for rgb-ir sensor*/
	BAYER_FORMAT_GRGBI = 8,
	BAYER_FORMAT_RGBGI,
	BAYER_FORMAT_GBGRI,
	BAYER_FORMAT_BGRGI,
	BAYER_FORMAT_IGRGB,
	BAYER_FORMAT_IRGBG,
	BAYER_FORMAT_IBGRG,
	BAYER_FORMAT_IGBGR,
	BAYER_FORMAT_MAX
} BAYER_FORMAT_E;
// -------- If you want to change these interfaces, please contact the isp team. --------

/*display mode*/
typedef enum _VIDEO_DISPLAY_MODE_E {
	VIDEO_DISPLAY_MODE_PREVIEW = 0x0,	/*preview mode*/
	VIDEO_DISPLAY_MODE_PLAYBACK = 0x1,	/*playback mode*/

	VIDEO_DISPLAY_MODE_MAX
} VIDEO_DISPLAY_MODE_E;

/*
 * u32ISO:  ISP internal ISO : Again*Dgain*ISPgain
 * u32ExposureTime:  Exposure time (reciprocal of shutter speed),unit is us
 * u32FNumber: The actual F-number (F-stop) of lens when the image was taken
 * u32SensorID: which sensor is used
 * u32HmaxTimes: Sensor HmaxTimes,unit is ns
 * u32VcNum: when dump wdr frame, which is long or short  exposure frame.
 */
// ++++++++ If you want to change these interfaces, please contact the isp team. ++++++++
typedef struct _ISP_FRAME_INFO_S {
	CVI_U32 u32ISO;
	CVI_U32 u32ExposureTime;
	CVI_U32 u32IspDgain;
	CVI_U32 u32Again;
	CVI_U32 u32Dgain;
	CVI_U32 au32Ratio[3];
	CVI_U32 u32IspNrStrength;
	CVI_U32 u32FNumber;
	CVI_U32 u32SensorID;
	CVI_U32 u32SensorMode;
	CVI_U32 u32HmaxTimes;
	CVI_U32 u32VcNum;
} ISP_FRAME_INFO_S;
// -------- If you want to change these interfaces, please contact the isp team. --------

/* RGGB=4 */
#define ISP_WB_GAIN_NUM 4
/* 3*3=9 matrix */
#define ISP_CAP_CCM_NUM 9
/*ISP configuration information*/
typedef struct _ISP_CONFIG_INFO_S {
	CVI_U32 u32ISO;									/*ISP internal ISO*/
	CVI_U32 u32IspDgain;							/*ISP digital gain value*/
	CVI_U32 u32ExposureTime;						/*Exposure time*/
	CVI_U32 au32WhiteBalanceGain[ISP_WB_GAIN_NUM];	/*White balance gain value array*/
	CVI_U32 u32ColorTemperature;					/*Color temperature*/
	CVI_U16 au16CapCCM[ISP_CAP_CCM_NUM];			/*Color Correction Matrix*/
} ISP_CONFIG_INFO_S;

// ++++++++ If you want to change these interfaces, please contact the isp team. ++++++++
typedef enum _COLOR_GAMUT_E {
	COLOR_GAMUT_BT601 = 0,
	COLOR_GAMUT_BT709,
	COLOR_GAMUT_BT2020,
	COLOR_GAMUT_USER,
	COLOR_GAMUT_MAX
} COLOR_GAMUT_E;
// -------- If you want to change these interfaces, please contact the isp team. --------

typedef struct _ISP_COLORGAMMUT_INFO_S {
	COLOR_GAMUT_E enColorGamut;
} ISP_COLORGAMMUT_INFO_S;

// ++++++++ If you want to change these interfaces, please contact the isp team. ++++++++
typedef enum _DYNAMIC_RANGE_E {
	DYNAMIC_RANGE_SDR8 = 0,
	DYNAMIC_RANGE_SDR10,
	DYNAMIC_RANGE_HDR10,
	DYNAMIC_RANGE_HLG,
	DYNAMIC_RANGE_SLF,
	DYNAMIC_RANGE_XDR,
	DYNAMIC_RANGE_MAX
} DYNAMIC_RANGE_E;
// -------- If you want to change these interfaces, please contact the isp team. --------

// ++++++++ If you want to change these interfaces, please contact the isp team. ++++++++
typedef enum _DATA_BITWIDTH_E {
	DATA_BITWIDTH_8 = 0,
	DATA_BITWIDTH_10,
	DATA_BITWIDTH_12,
	DATA_BITWIDTH_14,
	DATA_BITWIDTH_16,
	DATA_BITWIDTH_MAX
} DATA_BITWIDTH_E;
// -------- If you want to change these interfaces, please contact the isp team. --------

/**
 * @brief Define video frame
 *
 * s16OffsetTop: top offset of show area
 * s16OffsetBottom: bottom offset of show area
 * s16OffsetLeft: left offset of show area
 * s16OffsetRight: right offset of show area
 * u32FrameFlag: FRAME_FLAG_E, can be OR operation.
 */
// ++++++++ If you want to change these interfaces, please contact the isp team. ++++++++
typedef struct _VIDEO_FRAME_S {
	CVI_U32 u32Width;
	CVI_U32 u32Height;
	PIXEL_FORMAT_E enPixelFormat;
	BAYER_FORMAT_E enBayerFormat;
	VIDEO_FORMAT_E enVideoFormat;
	COMPRESS_MODE_E enCompressMode;
	DYNAMIC_RANGE_E enDynamicRange;
	COLOR_GAMUT_E enColorGamut;
	CVI_U32 u32Stride[3];

	CVI_U64 u64PhyAddr[3];
	CVI_U8 *pu8VirAddr[3];
#ifdef __arm__
	__u32 u32VirAddrPadding[3];
#endif
	CVI_U32 u32Length[3];

	CVI_U64 u64ExtPhyAddr;
	CVI_U8 *pu8ExtVirtAddr;
#ifdef __arm__
	__u32 u32ExtVirtAddrPadding;
#endif
	CVI_U32 u32ExtLength;

	CVI_S16 s16OffsetTop;
	CVI_S16 s16OffsetBottom;
	CVI_S16 s16OffsetLeft;
	CVI_S16 s16OffsetRight;

	CVI_U32 u32TimeRef;
	CVI_U64 u64PTS;
	CVI_U64 u64DTS;

	void *pPrivateData;
#ifdef __arm__
	__u32 u32PrivateDataPadding;
#endif
	CVI_U32 u32FrameFlag;
	CVI_S32 s32FrameIdx;
	CVI_BOOL bSrcEnd;
	CVI_U32 u32Align;
	CVI_U8 u8InterlacedFrame;
	CVI_U8 u8PicType;
	CVI_U8 u8Endian;
	CVI_U32 u32SeqenceNo;
} VIDEO_FRAME_S;
// -------- If you want to change these interfaces, please contact the isp team. --------

/**
 * @brief Define the information of video frame.
 *
 * stVFrame: Video frame info.
 * u32PoolId: VB pool ID.
 */
// ++++++++ If you want to change these interfaces, please contact the isp team. ++++++++
typedef struct _VIDEO_FRAME_INFO_S {
	VIDEO_FRAME_S stVFrame;
	CVI_U32 u32PoolId;
} VIDEO_FRAME_INFO_S;
// -------- If you want to change these interfaces, please contact the isp team. --------

/*
 * u32VBSize: size of VB needed.
 * u32MainStride: stride of planar0.
 * u32CStride: stride of planar1/2 if there is.
 * u32MainSize: size of all planars.
 * u32MainYSize: size of planar0.
 * u32MainCSize: size of planar1/2 if there is.
 * u16AddrAlign: address alignment needed between planar.
 */
typedef struct _VB_CAL_CONFIG_S {
	CVI_U32 u32VBSize;

	CVI_U32 u32MainStride;
	CVI_U32 u32CStride;
	CVI_U32 u32MainSize;
	CVI_U32 u32MainYSize;
	CVI_U32 u32MainCSize;
	CVI_U16 u16AddrAlign;
	CVI_U8  plane_num;
} VB_CAL_CONFIG_S;

/*
 * enPixelFormat: Bitmap's pixel format
 * u32Width: Bitmap's width
 * u32Height: Bitmap's height
 * pData: Address of Bitmap's data
 */
typedef struct _BITMAP_S {
	PIXEL_FORMAT_E enPixelFormat;
	CVI_U32 u32Width;
	CVI_U32 u32Height;

	CVI_VOID * ATTRIBUTE pData;
} BITMAP_S;

/*
 *
 * s32CenterXOffset: RW; Range: [-511, 511], horizontal offset of the image distortion center relative to image center
 * s32CenterYOffset: RW; Range: [-511, 511], vertical offset of the image distortion center relative to image center
 * s32DistortionRatio: RW; Range: [-300, 500], LDC Distortion ratio.
 *		    When spread on,s32DistortionRatio range should be [0, 500]
 */
// ++++++++ If you want to change these interfaces, please contact the isp team. ++++++++

typedef struct _HOMOGRAPH_REGION_S {
	char regionName[128];
	float matrix[9];
	uint8_t meshgrid_x;
	uint8_t meshgrid_y;
	uint8_t meshgrid_w;
	uint8_t meshgrid_h;
} HOMOGRAPH_REGION_S;

/*Properties of GridInfo*/
typedef struct _GRID_INFO_ATTR_S {
	CVI_BOOL Enable;		/*enable gridinfo or not*/
	char gridFileName[128];	/*Gridinfo file name*/
	char gridBindName[128];	/*Gridinfo bind name*/
	SIZE_S grid_in;	/*grid_in size*/
	SIZE_S grid_out; /*grid_out size*/
	CVI_BOOL isBlending; /*is blending*/
	CVI_BOOL bEISEnable; /*is enable EIS */
	uint8_t homoRgnNum; /*homo region num */
	CVI_VOID *pBuf; /*gridinfo buffer*/
	CVI_U32 Len; /*gridinfo buffer length*/
} GRID_INFO_ATTR_S;

typedef struct _LDC_ATTR_S {
	CVI_BOOL bAspect; /* RW;Whether aspect ration  is keep */
	CVI_S32 s32XRatio; /* RW; Range: [0, 0x64], field angle ration of  horizontal,valid when bAspect=0.*/
	CVI_S32 s32YRatio; /* RW; Range: [0, 0x64], field angle ration of  vertical,valid when bAspect=0.*/
	CVI_S32 s32XYRatio; /* RW; Range: [0, 0x64], field angle ration of  all,valid when bAspect=1.*/
	CVI_S32 s32CenterXOffset; /* RW; Range: [-0x1ff, 0x1ff] */
	CVI_S32 s32CenterYOffset; /* RW; Range: [-0x1ff, 0x1ff] */
	CVI_S32 s32DistortionRatio; /* RW; Range: [-0x12c, 0x1f4] */
	GRID_INFO_ATTR_S stGridInfoAttr; /*gridinfo attribute*/
	ROTATION_E enRotation; /*rotation*/
} LDC_ATTR_S;
// -------- If you want to change these interfaces, please contact the isp team. --------

// ++++++++ If you want to change these interfaces, please contact the isp team. ++++++++
/*Wide dynamic range mode*/
typedef enum _WDR_MODE_E {
	WDR_MODE_NONE = 0,				/*Do not use wide dynamic range mode*/
	WDR_MODE_BUILT_IN,				/*Built in wide dynamic range mode*/
	WDR_MODE_QUDRA,

	WDR_MODE_2To1_LINE,
	WDR_MODE_2To1_FRAME,
	WDR_MODE_2To1_FRAME_FULL_RATE,

	WDR_MODE_3To1_LINE,
	WDR_MODE_3To1_FRAME,
	WDR_MODE_3To1_FRAME_FULL_RATE,

	WDR_MODE_4To1_LINE,
	WDR_MODE_4To1_FRAME,
	WDR_MODE_4To1_FRAME_FULL_RATE,

	WDR_MODE_MAX,
} WDR_MODE_E;
// -------- If you want to change these interfaces, please contact the isp team. --------

// ++++++++ If you want to change these interfaces, please contact the isp team. ++++++++
/*Processing Amplifier attribute*/
typedef enum _PROC_AMP_E {
	PROC_AMP_BRIGHTNESS = 0,	/*Brightness adjustment*/
	PROC_AMP_CONTRAST,			/*Contrast adjustment*/
	PROC_AMP_SATURATION,		/*Saturation adjustment*/
	PROC_AMP_HUE,				/*Hue adjustment*/
	PROC_AMP_MAX,
} PROC_AMP_E;
// -------- If you want to change these interfaces, please contact the isp team. --------

/*Processing Amplifier Control Parameters*/
typedef struct _PROC_AMP_CTRL_S {
	CVI_S32 minimum;		/*The minimum adjustable value of this attribute*/
	CVI_S32 maximum;		/*The maximum adjustable value of this attribute*/
	CVI_S32 step;			/*Step size when adjusting this attribute*/
	CVI_S32 default_value;	/*The default value of this attribute*/
} PROC_AMP_CTRL_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* _CVI_COMM_VIDEO_H_ */
