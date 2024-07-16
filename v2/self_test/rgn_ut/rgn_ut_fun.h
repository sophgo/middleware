#ifndef __RGN_UT_FUN_H__
#define __RGN_UT_FUN_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "cvi_type.h"
#include <cvi_comm_vo.h>
#include <cvi_comm_vb.h>
#include <cvi_comm_vpss.h>
#include <cvi_comm_region.h>

typedef enum _PIC_SIZE_E {
	PIC_CIF,
	PIC_D1_PAL, /* 720 * 576 */
	PIC_D1_NTSC, /* 720 * 480 */
	PIC_720P, /* 1280 * 720  */
	PIC_1600x1200,
	PIC_1080P, /* 1920 * 1080 */
	PIC_1088, /* 1920 * 1088 */
	PIC_1440P, /* 2560 * 1440 */
	PIC_2304x1296,
	PIC_2048x1536,
	PIC_2048x2048,
	PIC_2560x1600,
	PIC_2592x1520,
	PIC_2592x1536,
	PIC_2592x1944,
	PIC_2688x1520,
	PIC_2716x1524,
	PIC_2880x1620,
	PIC_3844x1124,
	PIC_3840x2160,
	PIC_4096x2160,
	PIC_3000x3000,
	PIC_4000x3000,
	PIC_4032x3000,
	PIC_3840x8640,
	PIC_4608x4320,
	PIC_5120x3840,
	PIC_7688x1124,
	PIC_7680x4320,
	PIC_8192x4320,
	PIC_640x480,
	PIC_479P, /* 632 * 479 */
	PIC_400x400,
	PIC_288P, /* 384 * 288 */
	PIC_CUSTOMIZE,
	PIC_BUTT
} PIC_SIZE_E;

typedef enum _VO_MODE_E {
	VO_MODE_1MUX,
	VO_MODE_2MUX,
	VO_MODE_4MUX,
	VO_MODE_8MUX,
	VO_MODE_9MUX,
	VO_MODE_16MUX,
	VO_MODE_25MUX,
	VO_MODE_36MUX,
	VO_MODE_49MUX,
	VO_MODE_64MUX,
	VO_MODE_2X4,
	VO_MODE_BUTT
} VO_MODE_E;

typedef struct _VO_CONFIG_S {
	/* for device */
	VO_DEV VoDev;
	VO_PUB_ATTR_S stVoPubAttr;
	PIC_SIZE_E enPicSize;

	/* for layer */
	PIXEL_FORMAT_E enPixFormat;
	RECT_S stDispRect;
	SIZE_S stImageSize;

	CVI_U32 u32DisBufLen;

	/* for channel */
	VO_MODE_E enVoMode;
} VO_CONFIG_S;

extern RGN_RGBQUARD_S overlay_palette[256];

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

CVI_VOID RGN_COMM_SYS_Exit(void);

CVI_S32 RGN_COMM_SYS_Init(VB_CONFIG_S *pstVbConfig);

CVI_S32 RGN_COMM_VPSS_Init(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
			      VPSS_CHN_ATTR_S *pastVpssChnAttr);

CVI_S32 RGN_COMM_VPSS_Start(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
			      VPSS_CHN_ATTR_S *pastVpssChnAttr);

CVI_S32 RGN_COMM_REGION_Create(CVI_S32 HandleNum, RGN_TYPE_E enType, PIXEL_FORMAT_E pixelFormat);

CVI_S32 RGN_COMM_REGION_AttachToChn(CVI_S32 HandleNum, RGN_TYPE_E enType, MMF_CHN_S *pstChn);

CVI_S32 RGN_COMM_REGION_GetMinHandle(RGN_TYPE_E enType);

CVI_S32 RGN_COMM_REGION_GetUpCanvas(RGN_HANDLE Handle, const char *filename);

CVI_S32 RGN_COMM_VPSS_SendFrame(VPSS_GRP VpssGrp, SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filename);

CVI_BOOL RGN_COMM_FRAME_CompareWithFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);

CVI_S32 RGN_COMM_FRAME_SaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);

CVI_S32 RGN_COMM_REGION_DetachFrmChn(CVI_S32 HandleNum, RGN_TYPE_E enType, MMF_CHN_S *pstChn);

CVI_S32 RGN_COMM_REGION_Destroy(CVI_S32 HandleNum, RGN_TYPE_E enType);

CVI_S32 RGN_COMM_VPSS_Stop(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable);

CVI_S32 RGN_COMM_VO_GetDefConfig(VO_CONFIG_S *pstVoConfig);

CVI_S32 RGN_COMM_VO_StartVO(VO_CONFIG_S *pstVoConfig);

CVI_S32 RGN_COMM_REGION_SetBitMap(RGN_HANDLE Handle, const char *filename,
		PIXEL_FORMAT_E pixelFormat, CVI_BOOL bCompressed);

CVI_S32 RGN_COMM_FRAME_LoadFromFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame,
	SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat);

CVI_S32 RGN_COMM_REGION_MST_LoadBmp(const char *filename, BITMAP_S *pstBitmap, CVI_BOOL bFil,
			CVI_U32 u16FilColor, PIXEL_FORMAT_E enPixelFormat);

CVI_VOID RGN_COMM_VO_Exit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __RGN_UT_FUN_H__ */
