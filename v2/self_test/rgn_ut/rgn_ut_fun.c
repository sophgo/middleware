#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "cvi_type.h"
#include "cvi_sys.h"
#include "cvi_debug.h"
#include "cvi_vb.h"
#include "cvi_vo.h"
#include "cvi_vpss.h"
#include "cvi_region.h"
#include "cvi_buffer.h"
#include "cvi_math.h"

#include "rgn_ut_fun.h"
#include "loadbmp.h"

#define OverlayMinHandle 0
#define OverlayExMinHandle 20
#define CoverMinHandle 40
#define CoverExMinHandle 60
#define MosaicMinHandle 80
#define OdecHandle 100

#define VO_DEV_DHD0 0 /* VO's device HD0 */
#define VO_DEV_DHD1 1 /* VO's device HD1 */
#define VO_DEV_UHD VO_DEV_DHD0 /* VO's ultra HD device:HD0 */
#define VO_DEV_HD VO_DEV_DHD1 /* VO's HD device:HD1 */
#define COLOR_10_RGB_BLUE RGB(0, 0, 0x3FF)

RGN_RGBQUARD_S overlay_palette[256];
CVI_S32 RGN_COMM_REGION_MST_LoadBmp(const char *filename, BITMAP_S *pstBitmap, CVI_BOOL bFil,
			CVI_U32 u16FilColor, PIXEL_FORMAT_E enPixelFormat)
{
	OSD_SURFACE_S Surface;
	OSD_BITMAPFILEHEADER bmpFileHeader;
	OSD_BITMAPINFO bmpInfo;
	CVI_S32 Bpp;
	CVI_U32 nColors;
	CVI_U32 i, u32PdataSize;

	if (GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0) {
		printf("GetBmpInfo err!\n");
		return CVI_FAILURE;
	}
	Bpp = bmpInfo.bmiHeader.biBitCount/8;
	nColors = 0;
	if (Bpp == 1) {
		if (bmpInfo.bmiHeader.biClrUsed == 0)
			nColors = 1 << bmpInfo.bmiHeader.biBitCount;
		else
			nColors = bmpInfo.bmiHeader.biClrUsed;

		if (nColors > 256) {
			printf("Number of indexed palette is over 256.");
			return CVI_FAILURE;
		}

		/* Create the palette */
		for (i = 0; i < nColors; i++) {
			overlay_palette[i].argbAlpha = bmpInfo.bmiColors[i].rgbReserved;
			overlay_palette[i].argbRed = bmpInfo.bmiColors[i].rgbRed;
			overlay_palette[i].argbGreen = bmpInfo.bmiColors[i].rgbGreen;
			overlay_palette[i].argbBlue = bmpInfo.bmiColors[i].rgbBlue;
#ifdef _RGN_COMMON_REGION_DEBUG_
			CVI_U32 u32Pixel =
				((overlay_palette[i].argbBlue | overlay_palette[i].argbGreen << 8) |
				(overlay_palette[i].argbRed << 16 | overlay_palette[i].argbAlpha << 24));
			printf("overlay_palette index(%d) (0x%x).\n", i, u32Pixel);
#endif
		}
	}

	if (enPixelFormat == PIXEL_FORMAT_ARGB_4444) {
		Surface.enColorFmt = OSD_COLOR_FMT_RGB4444;
	} else if (enPixelFormat == PIXEL_FORMAT_ARGB_1555) {
		Surface.enColorFmt = OSD_COLOR_FMT_RGB1555;
	} else if (enPixelFormat == PIXEL_FORMAT_ARGB_8888) {
		Surface.enColorFmt = OSD_COLOR_FMT_RGB8888;
	} else if (enPixelFormat == PIXEL_FORMAT_8BIT_MODE) {
		Surface.enColorFmt = OSD_COLOR_FMT_8BIT_MODE;
	} else {
		printf("enPixelFormat err %d\n", enPixelFormat);
		return CVI_FAILURE;
	}

	u32PdataSize = Bpp * (bmpInfo.bmiHeader.biWidth) * (bmpInfo.bmiHeader.biHeight);
	pstBitmap->pData = malloc(u32PdataSize);
	if (pstBitmap->pData == NULL) {
		printf("malloc osd memory err!\n");
		return CVI_FAILURE;
	}

	CreateSurfaceByBitMap(filename, &Surface, (CVI_U8 *)(pstBitmap->pData));

	pstBitmap->u32Width = Surface.u16Width;
	pstBitmap->u32Height = Surface.u16Height;
	pstBitmap->enPixelFormat = enPixelFormat;

	if (bFil) {
		CVI_U32 i, j;
		CVI_U16 *pu16Temp;

		pu16Temp = (CVI_U16 *)pstBitmap->pData;
		for (i = 0; i < pstBitmap->u32Height; i++) {
			for (j = 0; j < pstBitmap->u32Width; j++) {
				if (u16FilColor == *pu16Temp) {
					*pu16Temp &= 0x7FFF;
				}

				pu16Temp++;
			}
		}
	}

	return CVI_SUCCESS;
}

VO_LVDS_ATTR_S lvds_lcm185x56_cfg = {
	.lvds_vesa_mode = VO_LVDS_MODE_VESA,
	.out_bits = VO_LVDS_OUT_8BIT,
	.chn_num = 1,
	.data_big_endian = 0,
	.lane_id = {VO_LVDS_LANE_0, VO_LVDS_LANE_1, VO_LVDS_LANE_CLK, VO_LVDS_LANE_2, VO_LVDS_LANE_3},
	.lane_pn_swap = {false, false, false, false, false},
};

CVI_VOID RGN_COMM_SYS_Exit(void)
{
	CVI_SYS_Exit();
	CVI_VB_Exit();
}

CVI_S32 RGN_COMM_SYS_Init(VB_CONFIG_S *pstVbConfig)
{
	CVI_S32 s32Ret = CVI_FAILURE;

	CVI_SYS_Exit();
	CVI_VB_Exit();

	if (pstVbConfig == NULL) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "input parameter is null, it is invaild!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_VB_SetConfig(pstVbConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_Init failed!\n");
		CVI_VB_Exit();
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 RGN_COMM_VPSS_Init(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
			      VPSS_CHN_ATTR_S *pastVpssChnAttr)
{
	VPSS_CHN VpssChn;
	CVI_S32 s32Ret;
	CVI_S32 j;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, pstVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		return CVI_FAILURE;
	}

	for (j = 0; j < VPSS_MAX_PHY_CHN_NUM; j++) {
		if (pabChnEnable[j]) {
			VpssChn = j;
			s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &pastVpssChnAttr[VpssChn]);

			if (s32Ret != CVI_SUCCESS) {
				printf("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
				return CVI_FAILURE;
			}

			s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);

			if (s32Ret != CVI_SUCCESS) {
				printf("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
				return CVI_FAILURE;
			}
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 RGN_COMM_VPSS_Start(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
			      VPSS_CHN_ATTR_S *pastVpssChnAttr)
{
	CVI_S32 s32Ret;
	UNUSED(pabChnEnable);
	UNUSED(pstVpssGrpAttr);
	UNUSED(pastVpssChnAttr);

	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 REGION_CreateOverLay(CVI_S32 HandleNum, PIXEL_FORMAT_E pixelFormat)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i;
	RGN_ATTR_S stRegion;

	stRegion.enType = OVERLAY_RGN;
	if (pixelFormat ==  PIXEL_FORMAT_8BIT_MODE)
		stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_8BIT_MODE;
	else
		stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;

	stRegion.unAttr.stOverlay.stSize.u32Height = 200;
	stRegion.unAttr.stOverlay.stSize.u32Width = 300;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00000000; // ARGB1555 transparent
	stRegion.unAttr.stOverlay.u32CanvasNum = 2;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
	for (i = OverlayMinHandle; i < OverlayMinHandle + HandleNum; i++) {
		s32Ret = CVI_RGN_Create(i, &stRegion);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_RGN_Create failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

CVI_S32 REGION_CreateOverLayEx(CVI_S32 HandleNum)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i;
	RGN_ATTR_S stRegion;

	stRegion.enType = OVERLAYEX_RGN;
	stRegion.unAttr.stOverlayEx.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	stRegion.unAttr.stOverlayEx.stSize.u32Height = 200;
	stRegion.unAttr.stOverlayEx.stSize.u32Width = 300;
	stRegion.unAttr.stOverlayEx.u32BgColor = 0x00000000; // ARGB1555 transparent
	stRegion.unAttr.stOverlayEx.u32CanvasNum = 2;
	stRegion.unAttr.stOverlayEx.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
	for (i = OverlayExMinHandle; i < OverlayExMinHandle + HandleNum; i++) {
		s32Ret = CVI_RGN_Create(i, &stRegion);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_RGN_Create failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

CVI_S32 REGION_CreateCover(CVI_S32 HandleNum)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i;
	RGN_ATTR_S stRegion;

	stRegion.enType = COVER_RGN;

	for (i = CoverMinHandle; i < CoverMinHandle + HandleNum; i++) {
		s32Ret = CVI_RGN_Create(i, &stRegion);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_RGN_Create failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

CVI_S32 REGION_CreateCoverEx(CVI_S32 HandleNum)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i;
	RGN_ATTR_S stRegion;

	stRegion.enType = COVEREX_RGN;

	for (i = CoverExMinHandle; i < CoverExMinHandle + HandleNum; i++) {
		s32Ret = CVI_RGN_Create(i, &stRegion);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_RGN_Create failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

CVI_S32 REGION_CreateMosaic(CVI_S32 HandleNum)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i;
	RGN_ATTR_S stRegion;

	stRegion.enType = MOSAIC_RGN;

	for (i = MosaicMinHandle; i < MosaicMinHandle + HandleNum; i++) {
		s32Ret = CVI_RGN_Create(i, &stRegion);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_RGN_Create failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

CVI_S32 RGN_COMM_REGION_Create(CVI_S32 HandleNum, RGN_TYPE_E enType, PIXEL_FORMAT_E pixelFormat)
{
	CVI_S32 s32Ret;

	if (HandleNum <= 0 || HandleNum > 16) {
		printf("HandleNum is illegal %d!\n", HandleNum);
		return CVI_FAILURE;
	}
	if (enType < OVERLAY_RGN || enType >= RGN_BUTT) {
		printf("enType is illegal %d!\n", enType);
		return CVI_FAILURE;
	}
	switch (enType) {
	case OVERLAY_RGN:
		s32Ret = REGION_CreateOverLay(HandleNum, pixelFormat);
		break;
	case OVERLAYEX_RGN:
		s32Ret = REGION_CreateOverLayEx(HandleNum);
		break;
	case COVER_RGN:
		s32Ret = REGION_CreateCover(HandleNum);
		break;
	case COVEREX_RGN:
		s32Ret = REGION_CreateCoverEx(HandleNum);
		break;
	case MOSAIC_RGN:
		s32Ret = REGION_CreateMosaic(HandleNum);
		break;
	default:
		s32Ret = CVI_FAILURE;
		break;
	}
	if (s32Ret != CVI_SUCCESS) {
		printf("RGN_COMM_REGION_Create failed! HandleNum%d,entype:%d!\n", HandleNum, enType);
		return CVI_FAILURE;
	}
	return s32Ret;
}

CVI_S32 REGION_AttachToChn(RGN_HANDLE Handle, MMF_CHN_S *pstChn, RGN_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret;

	s32Ret = CVI_RGN_AttachToChn(Handle, pstChn, pstChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}
	return s32Ret;
}

CVI_S32 REGION_DetachFromChn(RGN_HANDLE Handle, MMF_CHN_S *pstChn)
{
	CVI_S32 s32Ret;

	s32Ret = CVI_RGN_DetachFromChn(Handle, pstChn);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_DetachFromChn failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}
	return s32Ret;
}

CVI_S32 RGN_COMM_REGION_AttachToChn(CVI_S32 HandleNum, RGN_TYPE_E enType, MMF_CHN_S *pstChn)
{
	CVI_S32 i;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 MinHadle;
	RGN_CHN_ATTR_S stChnAttr;

	if (HandleNum <= 0 || HandleNum > 16) {
		printf("HandleNum is illegal %d!\n", HandleNum);
		return CVI_FAILURE;
	}
	if (enType < OVERLAY_RGN || enType >= RGN_BUTT) {
		printf("enType is illegal %d!\n", enType);
		return CVI_FAILURE;
	}
	if (pstChn == CVI_NULL) {
		printf("pstChn is NULL !\n");
		return CVI_FAILURE;
	}
	memset(&stChnAttr, 0, sizeof(stChnAttr));

	/*set the chn config*/
	stChnAttr.bShow = CVI_TRUE;
	switch (enType) {
	case OVERLAY_RGN:
		MinHadle = OverlayMinHandle;
		stChnAttr.bShow = CVI_TRUE;
		stChnAttr.enType = OVERLAY_RGN;
		stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = CVI_FALSE;
		break;
	case OVERLAYEX_RGN:
		MinHadle = OverlayExMinHandle;
		stChnAttr.bShow = CVI_TRUE;
		stChnAttr.enType = OVERLAYEX_RGN;
		stChnAttr.unChnAttr.stOverlayExChn.stInvertColor.bInvColEn = CVI_FALSE;
		break;
	case COVER_RGN:
		MinHadle = CoverMinHandle;

		stChnAttr.bShow = CVI_TRUE;
		stChnAttr.enType = COVER_RGN;
		stChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;

		stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 100;
		stChnAttr.unChnAttr.stCoverChn.stRect.u32Width = 100;

		stChnAttr.unChnAttr.stCoverChn.u32Color = 0x0000ffff;

		stChnAttr.unChnAttr.stCoverChn.enCoordinate = RGN_ABS_COOR;
		break;
	case COVEREX_RGN:
		MinHadle = CoverExMinHandle;

		stChnAttr.bShow = CVI_TRUE;
		stChnAttr.enType = COVEREX_RGN;
		stChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_RECT;

		stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 100;
		stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width = 100;

		stChnAttr.unChnAttr.stCoverExChn.u32Color = 0x0000ffff;
		break;
	case MOSAIC_RGN:
		MinHadle = MosaicMinHandle;
		stChnAttr.enType = MOSAIC_RGN;
		stChnAttr.unChnAttr.stMosaicChn.enBlkSize = MOSAIC_BLK_SIZE_8;
		stChnAttr.unChnAttr.stMosaicChn.stRect.u32Height = 96; // 8 pixel align
		stChnAttr.unChnAttr.stMosaicChn.stRect.u32Width = 96;
		break;
	default:
		return CVI_FAILURE;
	}
	/*attach to Chn*/
	for (i = MinHadle; i < MinHadle + HandleNum; i++) {
		if (enType == OVERLAY_RGN) {
			stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 20 + 200 * (i - OverlayMinHandle);
			stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 20 + 200 * (i - OverlayMinHandle);
			stChnAttr.unChnAttr.stOverlayChn.u32Layer = (i - OverlayMinHandle);
		}
		if (enType == OVERLAYEX_RGN) {
			stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 20 + 200 * (i - OverlayExMinHandle);
			stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 20 + 200 * (i - OverlayExMinHandle);
			stChnAttr.unChnAttr.stOverlayExChn.u32Layer = i - OverlayExMinHandle;
		}
		if (enType == COVER_RGN) {
			stChnAttr.unChnAttr.stCoverChn.stRect.s32X = 20 + 200 * (i - CoverMinHandle);
			stChnAttr.unChnAttr.stCoverChn.stRect.s32Y = 20 + 200 * (i - CoverMinHandle);
			stChnAttr.unChnAttr.stCoverChn.u32Layer = (i - CoverMinHandle);
		}
		if (enType == COVEREX_RGN) {
			stChnAttr.unChnAttr.stCoverExChn.stRect.s32X = 20 + 200 * (i - CoverExMinHandle);
			stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y = 20 + 200 * (i - CoverExMinHandle);
			stChnAttr.unChnAttr.stCoverExChn.u32Layer = i - CoverExMinHandle;
		}
		if (enType == MOSAIC_RGN) {
			stChnAttr.unChnAttr.stMosaicChn.stRect.s32X = 20 + 200 * (i - MosaicMinHandle);
			stChnAttr.unChnAttr.stMosaicChn.stRect.s32Y = 20 + 200 * (i - MosaicMinHandle);
			stChnAttr.unChnAttr.stMosaicChn.u32Layer = i - MosaicMinHandle;
		}
		s32Ret = REGION_AttachToChn(i, pstChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			printf("REGION_AttachToChn failed!\n");
			break;
		}
	}
	/*detach region from chn */
	if (s32Ret != CVI_SUCCESS && i > 0) {
		i--;
		for (; i >= MinHadle; i--)
			s32Ret = REGION_DetachFromChn(i, pstChn);
	}
	return s32Ret;
}

CVI_S32 RGN_COMM_REGION_GetMinHandle(RGN_TYPE_E enType)
{
	CVI_S32 MinHandle;

	switch (enType) {
	case OVERLAY_RGN:
		MinHandle = OverlayMinHandle;
		break;
	case OVERLAYEX_RGN:
		MinHandle = OverlayExMinHandle;
		break;
	case COVER_RGN:
		MinHandle = CoverMinHandle;
		break;
	case COVEREX_RGN:
		MinHandle = CoverExMinHandle;
		break;
	case MOSAIC_RGN:
		MinHandle = MosaicMinHandle;
		break;
	default:
		MinHandle = -1;
		break;
	}
	return MinHandle;
}

CVI_S32 RGN_COMM_REGION_MST_UpdateCanvas(const char *filename, BITMAP_S *pstBitmap, CVI_BOOL bFil,
				CVI_U32 u16FilColor, SIZE_S *pstSize, CVI_U32 u32Stride, PIXEL_FORMAT_E enPixelFormat)
{
	OSD_SURFACE_S Surface;
	OSD_BITMAPFILEHEADER bmpFileHeader;
	OSD_BITMAPINFO bmpInfo;

	if (GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0) {
		printf("GetBmpInfo err!\n");
		return CVI_FAILURE;
	}

	if (enPixelFormat == PIXEL_FORMAT_ARGB_1555) {
		Surface.enColorFmt = OSD_COLOR_FMT_RGB1555;
	} else if (enPixelFormat == PIXEL_FORMAT_ARGB_4444) {
		Surface.enColorFmt = OSD_COLOR_FMT_RGB4444;
	} else if (enPixelFormat == PIXEL_FORMAT_ARGB_8888) {
		Surface.enColorFmt = OSD_COLOR_FMT_RGB8888;
	} else if (enPixelFormat == PIXEL_FORMAT_8BIT_MODE) {
		Surface.enColorFmt = OSD_COLOR_FMT_RGB8888;
	} else {
		printf("Pixel format is not support!\n");
		return CVI_FAILURE;
	}

	if (pstBitmap->pData == NULL) {
		printf("malloc osd memory err!\n");
		return CVI_FAILURE;
	}

	CreateSurfaceByCanvas(filename, &Surface, (CVI_U8 *)(pstBitmap->pData)
			     , pstSize->u32Width, pstSize->u32Height, u32Stride);

	pstBitmap->u32Width = Surface.u16Width;
	pstBitmap->u32Height = Surface.u16Height;
	pstBitmap->enPixelFormat = enPixelFormat;

	// if pixel value match color, make it transparent.
	// Only works for ARGB1555
	if (bFil) {
		CVI_U32 i, j;
		CVI_U16 *pu16Temp;

		pu16Temp = (CVI_U16 *)pstBitmap->pData;
		for (i = 0; i < pstBitmap->u32Height; i++) {
			for (j = 0; j < pstBitmap->u32Width; j++) {
				if (u16FilColor == *pu16Temp)
					*pu16Temp &= 0x7FFF;

				pu16Temp++;
			}
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 RGN_COMM_REGION_GetUpCanvas(RGN_HANDLE Handle, const char *filename)
{
	CVI_S32 s32Ret;
	SIZE_S stSize;
	BITMAP_S stBitmap;
	RGN_CANVAS_INFO_S stCanvasInfo;
	FILE *pFile;

	s32Ret = CVI_RGN_GetCanvasInfo(Handle, &stCanvasInfo);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_GetCanvasInfo failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	stBitmap.pData = stCanvasInfo.pu8VirtAddr;
	stSize.u32Width = stCanvasInfo.stSize.u32Width;
	stSize.u32Height = stCanvasInfo.stSize.u32Height;

	if (stCanvasInfo.enOSDCompressMode == OSD_COMPRESS_MODE_SW) {
		pFile = fopen((char *)filename, "rb");

		if (pFile == NULL) {
			printf("Open file failed:%s!\n", filename);
			return -1;
		}

		fread(stBitmap.pData, 1, stCanvasInfo.u32CompressedSize, pFile);
		fclose(pFile);
	} else {
		if (stCanvasInfo.enPixelFormat == PIXEL_FORMAT_8BIT_MODE)
			RGN_COMM_REGION_MST_UpdateCanvas(filename, &stBitmap, CVI_FALSE, 0, &stSize,
						stCanvasInfo.u32Stride, PIXEL_FORMAT_8BIT_MODE);
		else
			RGN_COMM_REGION_MST_UpdateCanvas(filename, &stBitmap, CVI_FALSE, 0, &stSize,
						stCanvasInfo.u32Stride, PIXEL_FORMAT_ARGB_1555);
	}

	s32Ret = CVI_RGN_UpdateCanvas(Handle);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_UpdateCanvas failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 RGN_COMM_VPSS_SendFrame(VPSS_GRP VpssGrp, SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filename)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	VB_BLK blk;
	FILE *fp;
	CVI_U32 u32len;
	VB_CAL_CONFIG_S stVbCalConfig;

	COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	memset(&stVideoFrame, 0, sizeof(stVideoFrame));
	stVideoFrame.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	stVideoFrame.stVFrame.enPixelFormat = enPixelFormat;
	stVideoFrame.stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	stVideoFrame.stVFrame.enColorGamut = COLOR_GAMUT_BT709;
	stVideoFrame.stVFrame.u32Width = stSize->u32Width;
	stVideoFrame.stVFrame.u32Height = stSize->u32Height;
	stVideoFrame.stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
	stVideoFrame.stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	stVideoFrame.stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
	stVideoFrame.stVFrame.u32TimeRef = 0;
	stVideoFrame.stVFrame.u64PTS = 0;
	stVideoFrame.stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		printf("RGN_COMM_VPSS_SendFrame: Can't acquire vb block\n");
		return CVI_FAILURE;
	}

	//open data file & fread into the mmap address
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		printf("open data file error\n");
		CVI_VB_ReleaseBlock(blk);
		return CVI_FAILURE;
	}

	stVideoFrame.u32PoolId = CVI_VB_Handle2PoolId(blk);
	stVideoFrame.stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	stVideoFrame.stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
	stVideoFrame.stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	stVideoFrame.stVFrame.u64PhyAddr[1] = stVideoFrame.stVFrame.u64PhyAddr[0]
		+ ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	if (stVbCalConfig.plane_num == 3) {
		stVideoFrame.stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
		stVideoFrame.stVFrame.u64PhyAddr[2] = stVideoFrame.stVFrame.u64PhyAddr[1]
			+ ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
	}

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			continue;
		stVideoFrame.stVFrame.pu8VirAddr[i]
			= CVI_SYS_MmapCache(stVideoFrame.stVFrame.u64PhyAddr[i], stVideoFrame.stVFrame.u32Length[i]);

		u32len = fread(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i], 1, fp);
		if (u32len <= 0) {
			printf("vpss send frame: fread plane%d error\n", i);
			fclose(fp);
			CVI_VB_ReleaseBlock(blk);
			return CVI_FAILURE;
		}
		CVI_SYS_IonInvalidateCache(stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Length[i]);
	}

	printf("length of buffer(%d, %d, %d)\n", stVideoFrame.stVFrame.u32Length[0]
		, stVideoFrame.stVFrame.u32Length[1], stVideoFrame.stVFrame.u32Length[2]);
	printf("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", stVideoFrame.stVFrame.u64PhyAddr[0]
		, stVideoFrame.stVFrame.u64PhyAddr[1], stVideoFrame.stVFrame.u64PhyAddr[2]);
	printf("vir addr(%p, %p, %p)\n", stVideoFrame.stVFrame.pu8VirAddr[0]
		, stVideoFrame.stVFrame.pu8VirAddr[1], stVideoFrame.stVFrame.pu8VirAddr[2]);

	fclose(fp);

	printf("read file done and send out frame.\n");
	CVI_VPSS_SendFrame(VpssGrp, &stVideoFrame, -1);
	CVI_VB_ReleaseBlock(blk);

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			continue;
		CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i]);
	}
	return CVI_SUCCESS;
}

CVI_BOOL RGN_COMM_FRAME_CompareWithFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	FILE *fp;
	CVI_U32 u32len, plane_len, data_len;
	CVI_U32 u32LumaData, u32ChromaData = 0, data_height;
	bool result = true;
	VB_CAL_CONFIG_S stVbCalConfig;

	u32LumaData = pstVideoFrame->stVFrame.u32Width;
	data_height = pstVideoFrame->stVFrame.u32Height;

	COMMON_GetPicBufferConfig(pstVideoFrame->stVFrame.u32Width, pstVideoFrame->stVFrame.u32Height,
		pstVideoFrame->stVFrame.enPixelFormat, DATA_BITWIDTH_8,
		COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_RGB_888_PLANAR ||
	    pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_BGR_888_PLANAR ||
	    pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_444) {
		u32ChromaData = u32LumaData;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_422) {
		u32ChromaData =  (pstVideoFrame->stVFrame.u32Width / 2);
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) {
		u32ChromaData =  (pstVideoFrame->stVFrame.u32Width / 2);
		data_height = pstVideoFrame->stVFrame.u32Height / 2;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12 ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21) {
		u32ChromaData = u32LumaData;
		data_height = pstVideoFrame->stVFrame.u32Height / 2;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV16 ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV61) {
		u32ChromaData = u32LumaData;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUYV ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_UYVY ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YVYU ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_VYUY) {
		u32LumaData *= 2;
		u32ChromaData = 0;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_400) {
		u32ChromaData = 0;
	}

	CVI_TRACE_LOG(CVI_DBG_INFO, "u32LumaSize(%d): u32ChromaSize(%d)\n",
		stVbCalConfig.u32MainYSize, stVbCalConfig.u32MainCSize);
	CVI_TRACE_LOG(CVI_DBG_INFO, "u32LumaData(%d): u32ChromaData(%d)\n", u32LumaData, u32ChromaData);
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		printf("open data file, %s, error\n", filename);
		return false;
	}

	CVI_U8 buffer[stVbCalConfig.u32MainYSize];
	CVI_U32 offset = 0;

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		plane_len = (i == 0) ? stVbCalConfig.u32MainYSize : stVbCalConfig.u32MainCSize;
		if (plane_len == 0)
			continue;
		data_len = (i == 0) ? u32LumaData : u32ChromaData;
		offset = 0;

		pstVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

		u32len = fread(buffer, plane_len, 1, fp);
		if (u32len <= 0) {
			printf("fread data(%d) error\n", i);
			result = false;
			break;
		}
		// line by line check to avoid padding data mismatch problem.
		for (CVI_U32 line = 0; line < data_height; ++line) {
			if (memcmp(buffer + offset, pstVideoFrame->stVFrame.pu8VirAddr[i] + offset, data_len) != 0) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "plane(%d) line(%d) offset(%d) data mismatch:\n",
					      i, line, offset);
				CVI_TRACE_LOG(CVI_DBG_ERR, " paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
					      pstVideoFrame->stVFrame.u64PhyAddr[i],
					      pstVideoFrame->stVFrame.pu8VirAddr[i],
					      pstVideoFrame->stVFrame.u32Stride[i]);

				result = false;
				break;
			}
			offset += pstVideoFrame->stVFrame.u32Stride[i];
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	fclose(fp);
	return result;
}

CVI_S32 RGN_COMM_FRAME_SaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	FILE *fp;
	CVI_U32 u32len, u32DataLen;

	fp = fopen(filename, "w");
	if (fp == CVI_NULL) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "open data file error\n");
		return CVI_FAILURE;
	}
	for (int i = 0; i < 3; ++i) {
		u32DataLen = pstVideoFrame->stVFrame.u32Stride[i] * pstVideoFrame->stVFrame.u32Height;
		if (u32DataLen == 0)
			continue;
		if (i > 0 && ((pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
			(pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
			(pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
			u32DataLen >>= 1;

		pstVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

		CVI_TRACE_LOG(CVI_DBG_INFO, "plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
			   i, pstVideoFrame->stVFrame.u64PhyAddr[i],
			   pstVideoFrame->stVFrame.pu8VirAddr[i],
			   pstVideoFrame->stVFrame.u32Stride[i]);
		CVI_TRACE_LOG(CVI_DBG_INFO, " data_len(%d) plane_len(%d)\n",
			      u32DataLen, pstVideoFrame->stVFrame.u32Length[i]);
		u32len = fwrite(pstVideoFrame->stVFrame.pu8VirAddr[i], u32DataLen, 1, fp);
		if (u32len <= 0) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "fwrite data(%d) error\n", i);
			break;
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	fclose(fp);
	return CVI_SUCCESS;
}

CVI_S32 RGN_COMM_REGION_DetachFrmChn(CVI_S32 HandleNum, RGN_TYPE_E enType, MMF_CHN_S *pstChn)
{
	CVI_S32 i;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 MinHadle;

	if (HandleNum <= 0 || HandleNum > 16) {
		printf("HandleNum is illegal %d!\n", HandleNum);
		return CVI_FAILURE;
	}
	if (enType < OVERLAY_RGN || enType >= RGN_BUTT) {
		printf("enType is illegal %d!\n", enType);
		return CVI_FAILURE;
	}
	if (pstChn == CVI_NULL) {
		printf("pstChn is NULL !\n");
		return CVI_FAILURE;
	}
	switch (enType) {
	case OVERLAY_RGN:
		MinHadle = OverlayMinHandle;
		break;
	case OVERLAYEX_RGN:
		MinHadle = OverlayExMinHandle;
		break;
	case COVER_RGN:
		MinHadle = CoverMinHandle;
		break;
	case COVEREX_RGN:
		MinHadle = CoverExMinHandle;
		break;
	case MOSAIC_RGN:
		MinHadle = MosaicMinHandle;
		break;
	default:
		return CVI_FAILURE;
	}
	for (i = MinHadle; i < MinHadle + HandleNum; i++) {
		s32Ret = REGION_DetachFromChn(i, pstChn);
		if (s32Ret != CVI_SUCCESS)
			printf("REGION_DetachFromChn failed! Handle:%d\n", i);
	}
	return s32Ret;
}

CVI_S32 REGION_Destroy(RGN_HANDLE Handle)
{
	CVI_S32 s32Ret;

	s32Ret = CVI_RGN_Destroy(Handle);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_Destroy failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}
	return s32Ret;
}

CVI_S32 RGN_COMM_REGION_Destroy(CVI_S32 HandleNum, RGN_TYPE_E enType)
{
	CVI_S32 i;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 MinHandle;

	if (HandleNum <= 0 || HandleNum > 16) {
		printf("HandleNum is illegal %d!\n", HandleNum);
		return CVI_FAILURE;
	}
	if (enType < OVERLAY_RGN || enType >= RGN_BUTT) {
		printf("enType is illegal %d!\n", enType);
		return CVI_FAILURE;
	}
	switch (enType) {
	case OVERLAY_RGN:
		MinHandle = OverlayMinHandle;
		break;
	case OVERLAYEX_RGN:
		MinHandle = OverlayExMinHandle;
		break;
	case COVER_RGN:
		MinHandle = CoverMinHandle;
		break;
	case COVEREX_RGN:
		MinHandle = CoverExMinHandle;
		break;
	case MOSAIC_RGN:
		MinHandle = MosaicMinHandle;
		break;
	default:
		return CVI_FAILURE;
	}
	for (i = MinHandle; i < MinHandle + HandleNum; i++) {
		s32Ret = REGION_Destroy(i);
		if (s32Ret != CVI_SUCCESS)
			printf("RGN_COMM_REGION_Destroy failed!\n");
	}
	return s32Ret;
}

CVI_S32 RGN_COMM_VPSS_Stop(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable)
{
	CVI_S32 j;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_CHN VpssChn;

	for (j = 0; j < VPSS_MAX_PHY_CHN_NUM; j++) {
		if (pabChnEnable[j]) {
			VpssChn = j;
			s32Ret = CVI_VPSS_DisableChn(VpssGrp, VpssChn);
			if (s32Ret != CVI_SUCCESS) {
				printf("Vpss stop Grp %d channel %d failed! Please check param\n",
				VpssGrp, VpssChn);
				return CVI_FAILURE;
			}
		}
	}

	s32Ret = CVI_VPSS_StopGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		printf("Vpss Stop Grp %d failed! Please check param\n", VpssGrp);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VPSS_DestroyGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		printf("Vpss Destroy Grp %d failed! Please check\n", VpssGrp);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

/*
 * Name : RGN_COMM_VO_GetDefConfig
 * Desc : An instance of VO_CONFIG_S, which allows you to use vo immediately.
 */
CVI_S32 RGN_COMM_VO_GetDefConfig(VO_CONFIG_S *pstVoConfig)
{
	if (pstVoConfig == NULL) {
		printf("Error:argument can not be NULL\n");
		return CVI_FAILURE;
	}

	pstVoConfig->VoDev             = VO_DEV_UHD;

	pstVoConfig->stVoPubAttr.enIntfType = VO_INTF_MIPI;

	RECT_S stDefDispRect  = {0, 0, 1920, 1080};
	SIZE_S stDefImageSize = {1920, 1080};

	pstVoConfig->stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
	pstVoConfig->stDispRect    = stDefDispRect;
	pstVoConfig->stImageSize   = stDefImageSize;
	pstVoConfig->enPixFormat   = PIXEL_FORMAT_RGB_888_PLANAR;
	pstVoConfig->stVoPubAttr.u32BgColor = COLOR_10_RGB_BLUE;
	pstVoConfig->u32DisBufLen  = 3;
	pstVoConfig->enVoMode      = VO_MODE_1MUX;

	return CVI_SUCCESS;
}

CVI_S32 RGN_COMM_VO_StartDev(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_VO_SetPubAttr(VoDev, pstPubAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	if (pstPubAttr->enIntfType == VO_INTF_LVDS) {
		VO_LVDS_ATTR_S stLvdsAttr = lvds_lcm185x56_cfg;
		s32Ret = CVI_VO_SetLVDSParam(VoDev, &stLvdsAttr);
		if (s32Ret != CVI_SUCCESS) {
			printf("failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	s32Ret = CVI_VO_Enable(VoDev);
	if (s32Ret != CVI_SUCCESS) {
		printf("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 RGN_COMM_VO_GetWH(VO_INTF_SYNC_E enIntfSync, CVI_U32 *pu32W, CVI_U32 *pu32H, CVI_U32 *pu32Frm)
{
	switch (enIntfSync) {
	case VO_OUTPUT_PAL:
		*pu32W = 720;
		*pu32H = 576;
		*pu32Frm = 25;
		break;
	case VO_OUTPUT_NTSC:
		*pu32W = 720;
		*pu32H = 480;
		*pu32Frm = 30;
		break;
	case VO_OUTPUT_1080P24:
		*pu32W = 1920;
		*pu32H = 1080;
		*pu32Frm = 24;
		break;
	case VO_OUTPUT_1080P25:
		*pu32W = 1920;
		*pu32H = 1080;
		*pu32Frm = 25;
		break;
	case VO_OUTPUT_1080P30:
		*pu32W = 1920;
		*pu32H = 1080;
		*pu32Frm = 30;
		break;
	case VO_OUTPUT_720P50:
		*pu32W = 1280;
		*pu32H = 720;
		*pu32Frm = 50;
		break;
	case VO_OUTPUT_720P60:
		*pu32W = 1280;
		*pu32H = 720;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1080P50:
		*pu32W = 1920;
		*pu32H = 1080;
		*pu32Frm = 50;
		break;
	case VO_OUTPUT_1080P60:
		*pu32W = 1920;
		*pu32H = 1080;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_576P50:
		*pu32W = 720;
		*pu32H = 576;
		*pu32Frm = 50;
		break;
	case VO_OUTPUT_480P60:
		*pu32W = 720;
		*pu32H = 480;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_800x600_60:
		*pu32W = 800;
		*pu32H = 600;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1024x768_60:
		*pu32W = 1024;
		*pu32H = 768;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1280x1024_60:
		*pu32W = 1280;
		*pu32H = 1024;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1366x768_60:
		*pu32W = 1366;
		*pu32H = 768;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1440x900_60:
		*pu32W = 1440;
		*pu32H = 900;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1280x800_60:
		*pu32W = 1280;
		*pu32H = 800;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1600x1200_60:
		*pu32W = 1600;
		*pu32H = 1200;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1680x1050_60:
		*pu32W = 1680;
		*pu32H = 1050;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1920x1200_60:
		*pu32W = 1920;
		*pu32H = 1200;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_640x480_60:
		*pu32W = 640;
		*pu32H = 480;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_720x1280_60:
		*pu32W = 720;
		*pu32H = 1280;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1080x1920_60:
		*pu32W = 1080;
		*pu32H = 1920;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_480x800_60:
		*pu32W = 480;
		*pu32H = 800;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_USER:
		*pu32W = 720;
		*pu32H = 576;
		*pu32Frm = 25;
		break;
	default:
		printf("vo enIntfSync %d not support!\n", enIntfSync);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 RGN_COMM_VO_StopDev(VO_DEV VoDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_VO_Disable(VoDev);
	if (s32Ret != CVI_SUCCESS) {
		printf("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 RGN_COMM_VO_StartLayer(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_VO_SetVideoLayerAttr(VoLayer, pstLayerAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VO_EnableVideoLayer(VoLayer);
	if (s32Ret != CVI_SUCCESS) {
		printf("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 RGN_COMM_VO_StartChn(VO_LAYER VoLayer, VO_MODE_E enMode)
{
	CVI_U32 i;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32WndNum = 0;
	CVI_U32 u32Square = 0;
	CVI_U32 u32Row = 0;
	CVI_U32 u32Col = 0;
	CVI_U32 u32Width = 0;
	CVI_U32 u32Height = 0;
	VO_CHN_ATTR_S stChnAttr = { 0 };
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;

	switch (enMode) {
	case VO_MODE_1MUX:
		u32WndNum = 1;
		u32Square = 1;
		break;
	case VO_MODE_2MUX:
		u32WndNum = 2;
		u32Square = 2;
		break;
	case VO_MODE_4MUX:
		u32WndNum = 4;
		u32Square = 2;
		break;
	case VO_MODE_8MUX:
		u32WndNum = 8;
		u32Square = 3;
		break;
	case VO_MODE_9MUX:
		u32WndNum = 9;
		u32Square = 3;
		break;
	case VO_MODE_16MUX:
		u32WndNum = 16;
		u32Square = 4;
		break;
	case VO_MODE_25MUX:
		u32WndNum = 25;
		u32Square = 5;
		break;
	case VO_MODE_36MUX:
		u32WndNum = 36;
		u32Square = 6;
		break;
	case VO_MODE_49MUX:
		u32WndNum = 49;
		u32Square = 7;
		break;
	case VO_MODE_64MUX:
		u32WndNum = 64;
		u32Square = 8;
		break;
	case VO_MODE_2X4:
		u32WndNum = 8;
		u32Square = 3;
		u32Row = 4;
		u32Col = 2;
		break;
	default:
		printf("Undefined VO_MODE(%d)!\n", enMode);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}
	u32Width = stLayerAttr.stImageSize.u32Width;
	u32Height = stLayerAttr.stImageSize.u32Height;
	printf("u32Width:%d, u32Height:%d, u32Square:%d\n", u32Width, u32Height, u32Square);
	for (i = 0; i < u32WndNum; i++) {
		if (enMode == VO_MODE_1MUX || enMode == VO_MODE_2MUX || enMode == VO_MODE_4MUX ||
		    enMode == VO_MODE_8MUX || enMode == VO_MODE_9MUX || enMode == VO_MODE_16MUX ||
		    enMode == VO_MODE_25MUX || enMode == VO_MODE_36MUX || enMode == VO_MODE_49MUX ||
		    enMode == VO_MODE_64MUX) {
			stChnAttr.stRect.s32X = ALIGN_DOWN((u32Width / u32Square) * (i % u32Square), 2);
			stChnAttr.stRect.s32Y = ALIGN_DOWN((u32Height / u32Square) * (i / u32Square), 2);
			stChnAttr.stRect.u32Width = ALIGN_DOWN(u32Width / u32Square, 2);
			stChnAttr.stRect.u32Height = ALIGN_DOWN(u32Height / u32Square, 2);
			stChnAttr.u32Priority = 0;
			stChnAttr.u32Depth = 0;
		} else if (enMode == VO_MODE_2X4) {
			stChnAttr.stRect.s32X = ALIGN_DOWN((u32Width / u32Col) * (i % u32Col), 2);
			stChnAttr.stRect.s32Y = ALIGN_DOWN((u32Height / u32Row) * (i / u32Col), 2);
			stChnAttr.stRect.u32Width = ALIGN_DOWN(u32Width / u32Col, 2);
			stChnAttr.stRect.u32Height = ALIGN_DOWN(u32Height / u32Row, 2);
			stChnAttr.u32Priority = 0;
			stChnAttr.u32Depth = 0;
		}

		s32Ret = CVI_VO_SetChnAttr(VoLayer, i, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			printf("failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}

		s32Ret = CVI_VO_EnableChn(VoLayer, i);
		if (s32Ret != CVI_SUCCESS) {
			printf("failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 RGN_COMM_VO_StopLayer(VO_LAYER VoLayer)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_VO_DisableVideoLayer(VoLayer);
	if (s32Ret != CVI_SUCCESS) {
		printf("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 RGN_COMM_VO_StartVO(VO_CONFIG_S *pstVoConfig)
{
	/*******************************************
	 * VO device VoDev# information declaration.
	 *******************************************/
	VO_DEV VoDev = 0;
	VO_LAYER VoLayer = 0;
	VO_MODE_E enVoMode = 0;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr = { 0 };
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (pstVoConfig == NULL) {
		printf("Error:argument can not be NULL\n");
		return CVI_FAILURE;
	}
	VoDev = pstVoConfig->VoDev;
	VoLayer = pstVoConfig->VoDev;
	enVoMode = pstVoConfig->enVoMode;

	/********************************
	 * Set and start VO device VoDev#.
	 ********************************/
	s32Ret = RGN_COMM_VO_StartDev(VoDev, &pstVoConfig->stVoPubAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("RGN_COMM_VO_StartDev failed!\n");
		return s32Ret;
	}

	/******************************
	 * Set and start layer VoDev#.
	 ********************************/

	s32Ret = RGN_COMM_VO_GetWH(pstVoConfig->stVoPubAttr.enIntfSync, &stLayerAttr.stDispRect.u32Width,
				      &stLayerAttr.stDispRect.u32Height, &stLayerAttr.u32DispFrmRt);
	if (s32Ret != CVI_SUCCESS) {
		printf("RGN_COMM_VO_GetWH failed!\n");
		RGN_COMM_VO_StopDev(VoDev);
		return s32Ret;
	}
	stLayerAttr.enPixFormat = pstVoConfig->enPixFormat;

	stLayerAttr.stDispRect.s32X = 0;
	stLayerAttr.stDispRect.s32Y = 0;

	/******************************
	 * Set display rectangle if Layer.
	 ********************************/
	if (memcmp(&pstVoConfig->stDispRect, &stLayerAttr.stDispRect, sizeof(RECT_S)) != 0)
		stLayerAttr.stDispRect = pstVoConfig->stDispRect;

	/******************************
	 * Set image size if Layer.
	 ********************************/
	stLayerAttr.stImageSize.u32Width = stLayerAttr.stDispRect.u32Width;
	stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;
	if (memcmp(&pstVoConfig->stImageSize, &stLayerAttr.stImageSize, sizeof(SIZE_S)) != 0)
		stLayerAttr.stImageSize = pstVoConfig->stImageSize;

	if (pstVoConfig->u32DisBufLen) {
		s32Ret = CVI_VO_SetDisplayBufLen(VoLayer, pstVoConfig->u32DisBufLen);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_VO_SetDisplayBufLen failed with %#x!\n", s32Ret);
			RGN_COMM_VO_StopDev(VoDev);
			return s32Ret;
		}
		//for get screen frame
		stLayerAttr.u32Depth = 0;
	}

	s32Ret = RGN_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("RGN_COMM_VO_Start video layer failed!\n");
		RGN_COMM_VO_StopDev(VoDev);
		return s32Ret;
	}

	/******************************
	 * start vo channels.
	 ********************************/
	s32Ret = RGN_COMM_VO_StartChn(VoLayer, enVoMode);
	if (s32Ret != CVI_SUCCESS) {
		printf("RGN_COMM_VO_StartChn failed!\n");
		RGN_COMM_VO_StopLayer(VoLayer);
		RGN_COMM_VO_StopDev(VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 REGION_SetBitMap(RGN_HANDLE Handle, BITMAP_S *pstBitmap)
{
	CVI_S32 s32Ret;

	s32Ret = CVI_RGN_SetBitMap(Handle, pstBitmap);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}
	return s32Ret;
}

CVI_S32 RGN_COMM_REGION_SetBitMap(RGN_HANDLE Handle, const char *filename,
		PIXEL_FORMAT_E pixelFormat, CVI_BOOL bCompressed)
{
	CVI_S32 s32Ret, u32FileSize;
	BITMAP_S stBitmap;
	RGN_CANVAS_INFO_S stCanvasInfo;
	FILE *pFile;

	s32Ret = CVI_RGN_GetCanvasInfo(Handle, &stCanvasInfo);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_GetCanvasInfo failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}
	s32Ret = CVI_RGN_UpdateCanvas(Handle);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_UpdateCanvas failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	if (bCompressed) {
		if (filename == NULL) {
			printf("OSD_LoadBMP: filename=NULL\n");
			return -1;
		}

		pFile = fopen((char *)filename, "rb");
		fseek(pFile, 0, SEEK_END); // seek to end of file
		u32FileSize = ftell(pFile); // get current file pointer
		fseek(pFile, 0, SEEK_SET); // seek back to beginning of file

		stBitmap.pData = malloc(u32FileSize);
		if (stBitmap.pData == NULL) {
			printf("malloc osd memory err!\n");
			return CVI_FAILURE;
		}

		stBitmap.enPixelFormat = pixelFormat;
		stBitmap.u32Width = stCanvasInfo.stSize.u32Width;
		stBitmap.u32Height = stCanvasInfo.stSize.u32Height;

		fread(stBitmap.pData, 1, u32FileSize, pFile);
		fclose(pFile);
	} else
		RGN_COMM_REGION_MST_LoadBmp(filename, &stBitmap, CVI_FALSE, 0, pixelFormat);

	s32Ret = REGION_SetBitMap(Handle, &stBitmap);
	if (s32Ret != CVI_SUCCESS)
		printf("REGION_SetBitMap failed!Handle:%d\n", Handle);
	free(stBitmap.pData);
	return s32Ret;
}

/* RGN_COMM_FRAME_LoadFromFile:
 *   Load data to frame, whose data loaded from given filename.
 *
 * [in]filename: file to read.
 * [in]pstVideoFrame: the video-frame to store data from file.
 * [in]stSize: size of image.
 * [in]enPixelFormat: format of image
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 RGN_COMM_FRAME_LoadFromFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame,
	SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat)
{
	VB_BLK blk;
	FILE *fp;
	CVI_U32 u32len;
	VB_CAL_CONFIG_S stVbCalConfig;

	COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	memset(pstVideoFrame, 0, sizeof(*pstVideoFrame));
	pstVideoFrame->stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	pstVideoFrame->stVFrame.enPixelFormat = enPixelFormat;
	pstVideoFrame->stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	pstVideoFrame->stVFrame.enColorGamut = COLOR_GAMUT_BT601;
	pstVideoFrame->stVFrame.u32Width = stSize->u32Width;
	pstVideoFrame->stVFrame.u32Height = stSize->u32Height;
	pstVideoFrame->stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
	pstVideoFrame->stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	pstVideoFrame->stVFrame.u32TimeRef = 0;
	pstVideoFrame->stVFrame.u64PTS = 0;
	pstVideoFrame->stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		printf("Can't acquire vb block\n");
		return CVI_FAILURE;
	}

	//open data file & fread into the mmap address
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		printf("open data file error\n");
		return CVI_FAILURE;
	}

	pstVideoFrame->u32PoolId = CVI_VB_Handle2PoolId(blk);
	pstVideoFrame->stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	pstVideoFrame->stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
	pstVideoFrame->stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	pstVideoFrame->stVFrame.u64PhyAddr[1] = pstVideoFrame->stVFrame.u64PhyAddr[0]
		+ ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	if (stVbCalConfig.plane_num == 3) {
		pstVideoFrame->stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
		pstVideoFrame->stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
		pstVideoFrame->stVFrame.u64PhyAddr[2] = pstVideoFrame->stVFrame.u64PhyAddr[1]
			+ ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
	}

	printf("length of buffer(%d, %d, %d)\n", pstVideoFrame->stVFrame.u32Length[0]
		, pstVideoFrame->stVFrame.u32Length[1], pstVideoFrame->stVFrame.u32Length[2]);
	printf("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", pstVideoFrame->stVFrame.u64PhyAddr[0]
		, pstVideoFrame->stVFrame.u64PhyAddr[1], pstVideoFrame->stVFrame.u64PhyAddr[2]);

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (pstVideoFrame->stVFrame.u32Length[i] == 0)
			continue;
		pstVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_MmapCache(pstVideoFrame->stVFrame.u64PhyAddr[i],
					    pstVideoFrame->stVFrame.u32Length[i]);
		if (pstVideoFrame->stVFrame.pu8VirAddr[i] == CVI_NULL) {
			printf("mmap plane%d error\n", i);
			return CVI_FAILURE;
		}

		u32len = fread(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i], 1, fp);
		if (u32len <= 0) {
			printf("fread plane%d error\n", i);
			return CVI_FAILURE;
		}
		CVI_SYS_IonInvalidateCache(pstVideoFrame->stVFrame.u64PhyAddr[i],
					   pstVideoFrame->stVFrame.pu8VirAddr[i],
					   pstVideoFrame->stVFrame.u32Length[i]);
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	fclose(fp);

	printf("read file done and send out frame.\n");

	return CVI_SUCCESS;
}

CVI_VOID RGN_COMM_VO_Exit(void)
{
	CVI_S32 i = 0, j = 0;

	for (i = 0; i < VO_MAX_VIDEO_LAYER_NUM; i++)
		for (j = 0; j < VO_MAX_CHN_NUM; j++)
			CVI_VO_DisableChn(i, j);

	for (i = 0; i < VO_MAX_VIDEO_LAYER_NUM; i++)
		CVI_VO_DisableVideoLayer(i);

	for (i = 0; i < VO_MAX_DEV_NUM; i++)
		CVI_VO_Disable(i);
}