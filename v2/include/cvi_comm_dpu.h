#ifndef _CVI_COMM_DPU_H_
#define _CVI_COMM_DPU_H_
#include <linux/cvi_common.h>
#include <linux/cvi_defines.h>
#include <linux/cvi_comm_video.h>
#undef ARRAY_SIZE
#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))


#define uartlog(fmt, args...) \
{ \
    printf(fmt, ##args); \
    fflush(stdout); \
}

typedef CVI_U64 DPU_HANLDE;


typedef enum _DPU_DISP_RANGE_E{
	DPU_DISP_RANGE_DEFAULT = 0x0,
	DPU_DISP_RANGE_16     = 0x1,
	DPU_DISP_RANGE_32     = 0x2,
	DPU_DISP_RANGE_48     = 0x3,
	DPU_DISP_RANGE_64     = 0x4,
	DPU_DISP_RANGE_80     = 0x5,
	DPU_DISP_RANGE_96     = 0x6,
	DPU_DISP_RANGE_112    = 0x7,
	DPU_DISP_RANGE_128    = 0x8,
	DPU_DISP_RANGE_BUTT
}DPU_DISP_RANGE_E;

typedef enum _DPU_MASK_MODE_E{
	DPU_MASK_MODE_DEFAULT = 0x0,
	DPU_MASK_MODE_1x1     = 0x1,
	DPU_MASK_MODE_3x3     = 0x2,
	DPU_MASK_MODE_5x5     = 0x3,
	DPU_MASK_MODE_7x7     = 0x4,
	DPU_MASK_MODE_BUTT
}DPU_MASK_MODE_E;

typedef enum _DPU_DEPTH_UNIT_E{
	DPU_DEPTH_UNIT_DEFAULT = 0x0,
	DPU_DEPTH_UNIT_MM      = 0x1,
	DPU_DEPTH_UNIT_CM      = 0x2,
	DPU_DEPTH_UNIT_DM      = 0x3,
	DPU_DEPTH_UNIT_M       = 0x4,
	DPU_DEPTH_UNIT_BUTT
}DPU_DEPTH_UNIT_E;

typedef enum _DPU_DCC_DIR_E{
	DPU_DCC_DIR_DEFAULT  = 0x0,
	DPU_DCC_DIR_A12      = 0x1,
	DPU_DCC_DIR_A13      = 0x2,
	DPU_DCC_DIR_A14      = 0x3,
	DPU_DCC_DIR_BUTT
}DPU_DCC_DIR_E;

typedef enum _DPU_MODE_E{
	DPU_MODE_DEFAULT = 0x0,               //only sgbm,u8 disp out(no post process),16 align
	DPU_MODE_SGBM_MUX0 = 0x1,			  //only sgbm,u8 disp out(no post process),16 align
	DPU_MODE_SGBM_MUX1 = 0x2,			  //only sgbm,u16 disp out(post process),16 align
	DPU_MODE_SGBM_MUX2 = 0x3,			  //only sgbm,u8 disp out(post process),16 align
	DPU_MODE_SGBM_FGS_ONLINE_MUX0 = 0x4,  //sgbm 2 fgs online, fgs u8 disp out,16 align
	DPU_MODE_SGBM_FGS_ONLINE_MUX1 = 0x5,  //sgbm 2 fgs online, fgs u16 depth out,32 align
	DPU_MODE_SGBM_FGS_ONLINE_MUX2 = 0x6,  //sgbm 2 fgs online, sgbm u16 depth out,32 align
	DPU_MODE_FGS_MUX0 = 0x7,              //only fgs, u8 disp out,16 align
	DPU_MODE_FGS_MUX1 = 0x8,			  //only fgs, u16 depth out,32 align
	DPU_MODE_BUTT
}DPU_MODE_E;


typedef struct _DPU_GRP_ATTR_S {

	SIZE_S stLeftImageSize;
	SIZE_S stRightImageSize;
	DPU_MODE_E enDpuMode;
	DPU_MASK_MODE_E enMaskMode;
	DPU_DISP_RANGE_E enDispRange;
	CVI_U16 u16DispStartPos;
	CVI_U32 u32Rshift1;
	CVI_U32 u32Rshift2;
	CVI_U32 u32CaP1;
	CVI_U32 u32CaP2;
	CVI_U32 u32UniqRatio;
	CVI_U32 u32DispShift;
	CVI_U32 u32CensusShift;
	CVI_U32 u32FxBaseline;
	DPU_DCC_DIR_E enDccDir;
	CVI_U32 u32FgsMaxCount;
	CVI_U32 u32FgsMaxT;
	DPU_DEPTH_UNIT_E enDpuDepthUnit;
	CVI_BOOL bIsBtcostOut;
	CVI_BOOL bNeedSrcFrame;
	FRAME_RATE_CTRL_S stFrameRate;

} DPU_GRP_ATTR_S;

typedef struct _DPU_CHN_ATTR_S {
	SIZE_S stImgSize;
} DPU_CHN_ATTR_S;

#endif