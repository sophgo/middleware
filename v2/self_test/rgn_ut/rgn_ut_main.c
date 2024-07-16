#include "rgn_ut.h"

#define VPSS_FILENAME_IN   "res/rgn/golden.yuv422"
#define VPSS_FILENAME_IN1  "res/rgn/4608_8188_nv21.bin"
#define VO_FILENAME_IN     "res/rgn/vo4.yuv"
#define VPSS_FILENAME_OUT  "output"

#define colorbar_bmp      "res/rgn/colorbar.bmp"
#define dog_bmp           "res/rgn/dog.bmp"
#define tiger_bmp         "res/rgn/tiger.bmp"
#define tiger_8bitmode    "res/rgn/tiger_8bitmode.bmp"

#define test_bmp          tiger_bmp
#define test_hw_bs        "res/rgn/hw_bs_1280x720.bin"
#define test_sw_bs        "res/rgn/sw_bs_1280x720.bin"
#define BYTE_BITS               8
#define NOASCII_CHARACTER_BYTES 2
#define OSD_LIB_FONT_W          24
#define OSD_LIB_FONT_H          24

#define dog_argb8888_bin          "res/rgn/dog_s_72x60_pngto8888.bin"
#define dog_argb4444_bin          "res/rgn/dog_s_80x60_pngto1555.bin"
#define dog_argb1555_bin          "res/rgn/dog_s_80x60_pngto4444.bin"
#define dog_4bitmode_bin          "res/rgn/dog_s_96x60_4bit_pngtoLUT.bin"
#define dog_4bitmode_lut_bin      "res/rgn/dog_s_96x60_4bit_pngLUT.bin"
#define dog_8bitmode_bin          "res/rgn/dog_s_96x60_8bit_pngtoLUT.bin"
#define dog_8bitmode_lut_bin      "res/rgn/dog_s_96x60_8bit_pngLUT.bin"
#define fontmode_bin              "res/rgn/font_54x40_jpgtofont.bin"

#define OverlayMinHandle        0
#define OverlayExMinHandle      20
#define CoverMinHandle          40
#define CoverExMinHandle        60
#define MosaicMinHandle         80
#define OdecHandle              100

#define IsASCII(a)    (((a) >= 0x00 && (a) <= 0x7F) ? 1 : 0)
#define MAX_STR_LEN  (64)

#define NONE	"\033[m"
#define RED	"\033[0;32;31m"
#define GREEN	"\033[0;32;32m"

#define RGN_UT_PRT(fmt, ...) printf("[%s]-%d: "fmt, __func__, __LINE__, ##__VA_ARGS__)

#define RGN_UT_ARGB8888 0
#define RGN_UT_ARGB1555 1
#define RGN_UT_ARGB4444 2

typedef struct _VPSS_TEST_PARAM {
	SIZE_S stSize;
	SIZE_S stSizeOut;
	PIXEL_FORMAT_E pixelFormat;
	PIXEL_FORMAT_E pixelFormatOut;
	VPSS_CROP_INFO_S stCropInfo;
	VPSS_NORMALIZE_S stNormalize;
	ROTATION_E enRotation;
	CVI_FLOAT yRatio;
	VPSS_LDC_ATTR_S stLDCAttr;
	CVI_BOOL bAttachVb;
	CVI_BOOL bManualAspectRatio;
	VPSS_SCALE_COEF_E enCoef;
	CVI_BOOL bChnScaleCoef;
	char fileName[256];
	char fileNameOut[256];
	char fileNameOutRef[256];
} VPSS_TEST_PARAM;

typedef struct _RGN_TEST_PARAM {
	// normal rgn
	CVI_S32 u32HdlNum;
	RGN_TYPE_E enType;

	// compressed rgn
	CVI_U32 u32OdecFileSize;
	SIZE_S stOdecSize;

	// vpss settings
	MMF_CHN_S stChn;
	SIZE_S stInputSize;
	SIZE_S stOutputSize;
	PIXEL_FORMAT_E eInputFmt;
	PIXEL_FORMAT_E eOutputFmt;

	// input/output file settings
	CVI_CHAR *inputFile;
	CVI_CHAR *outputFile;
	CVI_U32 u32RepeatCnt;
} RGN_TEST_PARAM;

typedef enum _RGN_TEST_OP {
	RGN_IOCTL,
	RGN_WITH_VPSS_SEND_FRAME_TEST_ORIG,
	RGN_BIT_MAP_WITH_VPSS_SEND_FRAME_TEST,
	RGN_CANVAS_WITH_VPSS_SEND_FRAME_TEST,
	RGN_8BIT_MODE_CANVAS_WITH_VPSS_SEND_FRAME_TEST,
	RGN_COMPRESSED_SIMPLEOBJS_ARGB8888_TEST = 10, //10
	RGN_COMPRESSED_SIMPLEOBJS_ARGB1555_TEST,
	RGN_COMPRESSED_SIMPLEOBJS_ARGB4444_TEST,
	RGN_COMPRESSED_SIMPLEOBJS_BITMAP_TEST,
	RGN_COMPRESSED_NORMAL_MIXED_TEST,
	RGN_VPSS_COVEREX_TEST = 20,
	RGN_VPSS_MOSAIC_TEST,
	RGN_VO_OSD = 50,
	RGN_VO_COVER_TEST,
	RGN_CREATE_DESTROY = 100,
	RGN_CREATE_ATTACTH_DETACH_DESTROY,
	RGN_ATTR_TEST,
	RGN_DISPLAY_ATTR_TEST,
	// sw verification
	RGN_VPSS_FORMATS_TEST = 150,
	RGN_VPSS_SC_V1_CAPABILITY_TEST,
	RGN_VPSS_SC_V2_CAPABILITY_TEST,
	RGN_VPSS_SCALING_TEST,
	RGN_VPSS_COLORKEY_TEST,
	RGN_VPSS_FONTBOX_TEST,
	RGN_VPSS_CROP_TEST,
	RGN_CHECKSUM_TEST,
	RGN_UT_CFG,
} RGN_TEST_OP;

int rgn_fd = -1;
static CVI_U8 is_enable = 1;
CVI_CHAR *Path_BMP;

static CVI_S32 rgn_GetFontMod(char *Character, CVI_U8 **FontMod, CVI_S32 *FontModLen)
{
	CVI_U32 offset = 0;
	CVI_U32 areacode = 0;
	CVI_U32 bitcode = 0;

	if (IsASCII(Character[0])) {
		areacode = 3;
		bitcode = (CVI_U32)((CVI_U8)Character[0] - 0x20);
	} else {
		areacode = (CVI_U32)((CVI_U8)Character[0] - 0xA0);
		bitcode = (CVI_U32)((CVI_U8)Character[1] - 0xA0);
	}
	offset = (94 * (areacode - 1) + (bitcode - 1)) * (OSD_LIB_FONT_W * OSD_LIB_FONT_H / 8);
	*FontMod = (CVI_U8 *)g_fontLib + offset;
	*FontModLen = OSD_LIB_FONT_W*OSD_LIB_FONT_H / 8;
	return CVI_SUCCESS;
}

static CVI_S32 rgn_GetNonASCNum(char *string, CVI_S32 len)
{
	CVI_S32 i;
	CVI_S32 n = 0;

	for (i = 0; i < len; i++) {
		if (string[i] == '\0')
			break;
		if (!IsASCII(string[i])) {
			i++;
			n++;
		}
	}

	return n;
}

void rgn_GetTimeStr(const struct tm *pstTime, char *pazStr, CVI_S32 s32Len)
{
	time_t nowTime;
	struct tm stTime = {
		0,
	};

	if (!pstTime) {
		time(&nowTime);
		localtime_r(&nowTime, &stTime);
		pstTime = &stTime;
	}

	snprintf(pazStr, s32Len, "%04d-%02d-%02d %02d:%02d:%02d",
		pstTime->tm_year + 1900, pstTime->tm_mon + 1, pstTime->tm_mday,
		pstTime->tm_hour, pstTime->tm_min, pstTime->tm_sec);
}

CVI_S32 rgn_TimeBitmap(char *szStr, BITMAP_S *pstBitmap, CVI_U32 u32Color, CVI_U32 u32BgColor)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32CanvasWidth, u32CanvasHeight;
	SIZE_S stFontSize;
	CVI_S32 s32StrLen = strnlen(szStr, MAX_STR_LEN);
	CVI_S32 NonASCNum = rgn_GetNonASCNum(szStr, s32StrLen);

	u32CanvasWidth = OSD_LIB_FONT_W * (s32StrLen - NonASCNum * (NOASCII_CHARACTER_BYTES - 1));
	u32CanvasHeight = OSD_LIB_FONT_H;
	stFontSize.u32Width = OSD_LIB_FONT_W;
	stFontSize.u32Height = OSD_LIB_FONT_H;

	pstBitmap->u32Width = u32CanvasWidth;
	pstBitmap->u32Height = u32CanvasHeight;
	pstBitmap->pData = malloc(2 * (pstBitmap->u32Width) * (pstBitmap->u32Height));
	if (pstBitmap->pData == NULL)
		RGN_UT_PRT("malloc osd memroy err!\n");

	CVI_U16 *puBmData = (CVI_U16 *)pstBitmap->pData;
	CVI_U32 u32BmRow, u32BmCol;

	for (u32BmRow = 0; u32BmRow < u32CanvasHeight; ++u32BmRow) {
		CVI_S32 NonASCShow = 0;

		for (u32BmCol = 0; u32BmCol < u32CanvasWidth; ++u32BmCol) {
			CVI_S32 s32BmDataIdx = u32BmRow * pstBitmap->u32Width + u32BmCol;
			CVI_S32 s32CharIdx = u32BmCol / stFontSize.u32Width;
			CVI_S32 s32StringIdx = s32CharIdx + NonASCShow * (NOASCII_CHARACTER_BYTES - 1);

			if (NonASCNum > 0 && s32CharIdx > 0) {
				NonASCShow = rgn_GetNonASCNum(szStr, s32StringIdx);
				s32StringIdx = s32CharIdx + NonASCShow * (NOASCII_CHARACTER_BYTES - 1);
			}
			CVI_S32 s32CharCol = (u32BmCol - (stFontSize.u32Width * s32CharIdx)) * OSD_LIB_FONT_W /
							stFontSize.u32Width;
			CVI_S32 s32CharRow = u32BmRow * OSD_LIB_FONT_H / stFontSize.u32Height;
			CVI_S32 s32HexOffset = s32CharRow * OSD_LIB_FONT_W / BYTE_BITS + s32CharCol / BYTE_BITS;
			CVI_S32 s32BitOffset = s32CharCol % BYTE_BITS;
			CVI_U8 *FontMod = NULL;
			CVI_S32 FontModLen = 0;

			if (rgn_GetFontMod(&szStr[s32StringIdx], &FontMod, &FontModLen) == CVI_SUCCESS) {
				if (FontMod != NULL && s32HexOffset < FontModLen) {
					CVI_U8 temp = FontMod[s32HexOffset];

					if ((temp >> ((BYTE_BITS - 1) - s32BitOffset)) & 0x1)
						puBmData[s32BmDataIdx] = (CVI_U16)u32Color;
					else
						puBmData[s32BmDataIdx] = (CVI_U16)u32BgColor;
					continue;
				}
			}
			RGN_UT_PRT("GetFontMod Fail\n");
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

static void dump_mem(VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_U32 size)
{
	CVI_U32 u32DataLen;

	for (int i = 0; i < 3; ++i) {
		u32DataLen = pstVideoFrame->stVFrame.u32Stride[i] * pstVideoFrame->stVFrame.u32Height;
		if (u32DataLen == 0)
			continue;
		u32DataLen = size ? size : u32DataLen;
		if (i > 0 && ((pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
			      (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
			      (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
			u32DataLen >>= 1;

		pstVideoFrame->stVFrame.pu8VirAddr[i]
				= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

		RGN_UT_PRT("plane(%d): paddr(0x%llx) vaddr(0x%llx) stride(%d)\n", i,
			   (unsigned long long)pstVideoFrame->stVFrame.u64PhyAddr[i],
			   (unsigned long long)(intptr_t)pstVideoFrame->stVFrame.pu8VirAddr[i],
			   pstVideoFrame->stVFrame.u32Stride[i]);
		RGN_UT_PRT(" data_len(%d) plane_len(%d)\n",
			   u32DataLen, pstVideoFrame->stVFrame.u32Length[i]);

		for (CVI_U32 j = 0; j < u32DataLen/16; j += 16) {
			CVI_U32 *buf = (CVI_U32 *)(pstVideoFrame->stVFrame.pu8VirAddr[i] + j);
			RGN_UT_PRT("[%d]%04x: %08x %08x %08x %08x\n",
				   i, j, buf[0], buf[1], buf[2], buf[3]);
		}

		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}
}

static CVI_S32 rgn_send_file_test_body(VPSS_TEST_PARAM *pParam)
{
	VB_CONFIG_S        stVbConf;
	CVI_U32            u32BlkSize, u32BlkSizeOut;
	CVI_S32            s32Ret = CVI_SUCCESS;
	CVI_S32            s32ExtRet = CVI_SUCCESS;
	PIXEL_FORMAT_E     pixelFormat = pParam->pixelFormat;
	PIXEL_FORMAT_E     pixelFormatOut = pParam->pixelFormatOut;
	SIZE_S             stSize = pParam->stSize;
	SIZE_S             stSizeOut = pParam->stSizeOut;
	ROTATION_E         enRotation = pParam->enRotation;
	ROTATION_E         enRotationOut;
	VPSS_LDC_ATTR_S    *pstLDCAttr = &pParam->stLDCAttr;
	VPSS_CROP_INFO_S   *pstCropInfo = &pParam->stCropInfo;
	char               *fileName = pParam->fileName;
	char               *fileNameOut = pParam->fileNameOut;
	VPSS_NORMALIZE_S   *pstNormalize = &pParam->stNormalize;
	CVI_FLOAT          yRatio = pParam->yRatio;
	CVI_FLOAT          yRatioOut;
	CVI_BOOL           bAttachVb = pParam->bAttachVb;
	CVI_BOOL           bManualAspectRatio = pParam->bManualAspectRatio;
	ASPECT_RATIO_E     aspectRatioMode = bManualAspectRatio ? ASPECT_RATIO_MANUAL : ASPECT_RATIO_AUTO;
	CVI_BOOL           bInOutSame = CVI_FALSE;
	CVI_BOOL           bChnScaleCoef = pParam->bChnScaleCoef;
	VPSS_SCALE_COEF_E  enCoef = pParam->enCoef;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	RGN_COMM_SYS_Exit();

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 1;

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height, pixelFormat,
					 DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(stSizeOut.u32Width, stSizeOut.u32Height, pixelFormatOut,
					 DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	if (!bAttachVb) {
		u32BlkSize = u32BlkSize > u32BlkSizeOut ? u32BlkSize : u32BlkSizeOut;
		stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
		stVbConf.astCommPool[0].u32BlkCnt	= 5;
		RGN_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSize);
	} else {
		stVbConf.u32MaxPoolCnt                  = 2;
		stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
		stVbConf.astCommPool[0].u32BlkCnt	= 1;	// Only one to send frame
		stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
		stVbConf.astCommPool[1].u32BlkCnt	= 1;
		RGN_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSize);
		RGN_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);
	}

	s32Ret = RGN_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("system init failed with %#x\n", s32Ret);
		return -1;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	VPSS_GRP	   VpssGrp = 0;
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_CHN	   VpssChn = VPSS_CHN0;
	CVI_BOOL	   abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S	    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM] = {0};

	// grp0 for right half
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = pixelFormat;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	astVpssChnAttr[VpssChn].u32Width		    = stSizeOut.u32Width;
	astVpssChnAttr[VpssChn].u32Height		    = stSizeOut.u32Height;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = pixelFormatOut;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = aspectRatioMode;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor   = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32X = 0;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32Y = 0;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Width = stSizeOut.u32Width;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Height = stSizeOut.u32Height;
	astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;

	if (pstNormalize->bEnable) {
		astVpssChnAttr[VpssChn].stNormalize.bEnable = CVI_TRUE;
		astVpssChnAttr[VpssChn].stNormalize.factor[0] = pstNormalize->factor[0];
		astVpssChnAttr[VpssChn].stNormalize.factor[1] = pstNormalize->factor[1];
		astVpssChnAttr[VpssChn].stNormalize.factor[2] = pstNormalize->factor[2];
		astVpssChnAttr[VpssChn].stNormalize.mean[0] = pstNormalize->mean[0];
		astVpssChnAttr[VpssChn].stNormalize.mean[1] = pstNormalize->mean[1];
		astVpssChnAttr[VpssChn].stNormalize.mean[2] = pstNormalize->mean[2];
		astVpssChnAttr[VpssChn].stNormalize.rounding = pstNormalize->rounding;
	}

	/*start vpss*/
	abChnEnable[0] = CVI_TRUE;
	s32Ret = RGN_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		goto EXIT0;
	}

	s32Ret = RGN_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		goto EXIT0;
	}

	if (yRatio != 0.0) {
		s32Ret = CVI_VPSS_SetChnYRatio(VpssGrp, VpssChn, yRatio);
		if (s32Ret != CVI_SUCCESS) {
			RGN_UT_PRT("CVI_VPSS_SetChnYRatio failed. s32Ret: 0x%x !\n", s32Ret);
			goto EXIT1;
		}

		s32Ret = CVI_VPSS_GetChnYRatio(VpssGrp, VpssChn, &yRatioOut);
		if (s32Ret != CVI_SUCCESS) {
			RGN_UT_PRT("CVI_VPSS_GetChnYRatio failed. s32Ret: 0x%x !\n", s32Ret);
			goto EXIT1;
		}
		if (yRatio != yRatioOut) {
			RGN_UT_PRT("CVI_VPSS_GetChnYRatio failed. ratio set=%f, get=%f !\n", yRatio, yRatioOut);
			goto EXIT1;
		}
	}

	if (pixelFormatOut == PIXEL_FORMAT_NV12 || pixelFormatOut == PIXEL_FORMAT_NV21 ||
	    pixelFormatOut == PIXEL_FORMAT_YUV_400) {
		s32Ret = CVI_VPSS_SetChnRotation(VpssGrp, VpssChn, enRotation);
		if (s32Ret != CVI_SUCCESS) {
			RGN_UT_PRT("set vpss chn rotation failed. s32Ret: 0x%x !\n", s32Ret);
			goto EXIT1;
		}

		s32Ret = CVI_VPSS_GetChnRotation(VpssGrp, VpssChn, &enRotationOut);
		if (s32Ret != CVI_SUCCESS) {
			RGN_UT_PRT("CVI_VPSS_GetChnRotation failed. s32Ret: 0x%x !\n", s32Ret);
			goto EXIT1;
		}
		if (enRotation != enRotationOut) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnRotation failed. rot=%d\n",
				enRotationOut);
			goto EXIT1;
		}
	}

	s32Ret = CVI_VPSS_SetChnCrop(0, 0, pstCropInfo);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("set vpss chn crop failed. s32Ret: 0x%x !\n", s32Ret);
		goto EXIT1;
	}

	if (bAttachVb) {
		s32Ret = CVI_VPSS_AttachVbPool(0, 0, 1);
		if (s32Ret != CVI_SUCCESS) {
			RGN_UT_PRT("vpss attach vb pool failed. s32Ret: 0x%x !\n", s32Ret);
			goto EXIT1;
		}
	}

	if (bChnScaleCoef) {
		s32Ret = CVI_VPSS_SetChnScaleCoefLevel(VpssGrp, VpssChn, enCoef);
		if (s32Ret != CVI_SUCCESS) {
			RGN_UT_PRT("vpss set chn scale coef failed. s32Ret: 0x%x !\n", s32Ret);
			goto EXIT1;
		}
	}

	/************************************************
	 * step3:  Init RGN
	 ************************************************/
	CVI_S32 i;
	CVI_S32 MinHandle;
	CVI_S32 HandleNum;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn;
	PIXEL_FORMAT_E enPixelFormat;

	HandleNum = 3;
	enType = OVERLAY_RGN;
	stChn.enModId = CVI_ID_VPSS;
	stChn.s32DevId = 0;
	stChn.s32ChnId = 0;
	Path_BMP = test_bmp;
	enPixelFormat = PIXEL_FORMAT_ARGB_1555;

	s32Ret = RGN_COMM_REGION_Create(HandleNum, enType, enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}
	s32Ret = RGN_COMM_REGION_AttachToChn(HandleNum, enType, &stChn);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_AttachToChn failed!\n");
		goto EXIT2;
	}

	if (enType == OVERLAY_RGN || enType == OVERLAYEX_RGN) {
		MinHandle = RGN_COMM_REGION_GetMinHandle(enType);

		for (i = MinHandle; i < MinHandle + HandleNum; i++) {
			s32Ret = RGN_COMM_REGION_GetUpCanvas(i, Path_BMP);
			if (s32Ret != CVI_SUCCESS) {
				RGN_UT_PRT("RGN_COMM_REGION_GetUpCanvas failed!\n");
				goto EXIT3;
			}
		}
	}

	/************************************************
	 * step4:  VPSS work
	 ************************************************/
	for (int i = 0; i < 1/*10*/; i++) {
		VIDEO_FRAME_INFO_S stVideoFrame;
		s32Ret = RGN_COMM_VPSS_SendFrame(0, &stSize, pixelFormat, fileName);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "[%d] RGN_COMM_VPSS_SendFrame for grp0 chn0. s32Ret: 0x%x !\n",
				i, s32Ret);
			goto EXIT3;
		}

		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, 1000/*-1*/);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "[%d] CVI_VPSS_GetChnFrame for grp0 chn0. s32Ret: 0x%x !\n",
				i, s32Ret);
			goto EXIT3;
		}

		//dump_mem(&stVideoFrame, 32);
		if (enRotation == ROTATION_0 && pixelFormat == pixelFormatOut &&
		    stSize.u32Width == stSizeOut.u32Width && stSize.u32Height == stSizeOut.u32Height)
			bInOutSame = CVI_TRUE;
		if (bInOutSame && pstLDCAttr->bEnable) {
			if (pstLDCAttr->stAttr.bAspect && pstLDCAttr->stAttr.s32XYRatio != 100)
				bInOutSame = CVI_FALSE;

			if (!pstLDCAttr->stAttr.bAspect &&
			    (pstLDCAttr->stAttr.s32XRatio != 100 || pstLDCAttr->stAttr.s32XRatio != 100))
				bInOutSame = CVI_FALSE;

			if (pstLDCAttr->stAttr.s32CenterXOffset || pstLDCAttr->stAttr.s32CenterYOffset ||
			    pstLDCAttr->stAttr.s32DistortionRatio)
				bInOutSame = CVI_FALSE;
		}

		if (bInOutSame && !RGN_COMM_FRAME_CompareWithFile(fileName, &stVideoFrame)) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "[%d] RGN_COMM_FRAME_CompareWithFile for grp0 chn0 fail\n", i);
			s32Ret = CVI_FAILURE;
			dump_mem(&stVideoFrame, 32);
			//goto ERR_VPSS_COMBINE;
			// keep going
		}

		s32Ret = RGN_COMM_FRAME_SaveToFile(fileNameOut, &stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_WARN, "[%d] RGN_COMM_FRAME_SaveToFile. s32Ret: 0x%x !\n",
				i, s32Ret);
			//goto ERR_VPSS_COMBINE;
			// keep going
		}

		s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "[%d] CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n",
				i, s32Ret);
			goto EXIT3;
		}
	}

EXIT3:
	s32ExtRet = RGN_COMM_REGION_DetachFrmChn(HandleNum, enType, &stChn);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_DetachFrmChn failed!\n");
EXIT2:
	s32ExtRet = RGN_COMM_REGION_Destroy(HandleNum, enType);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
EXIT1:
	RGN_COMM_VPSS_Stop(VpssGrp, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();

	return s32Ret;
}

static CVI_S32 rgn_send_file_test(void)
{
	VPSS_TEST_PARAM param;
	CVI_S32 s32Ret;

	memset(&param, 0, sizeof(param));
	param.stSize.u32Width = 1920;
	param.stSize.u32Height = 1080;
	param.stSizeOut.u32Width = 1280;
	param.stSizeOut.u32Height = 720;
	param.pixelFormat = PIXEL_FORMAT_YUV_PLANAR_422;
	param.pixelFormatOut = PIXEL_FORMAT_NV21;
	param.enRotation = ROTATION_0;
	param.yRatio = 0.0f;
	param.stLDCAttr.bEnable = CVI_FALSE;
	param.bManualAspectRatio = CVI_FALSE;
	snprintf(param.fileName, sizeof(param.fileName)-1, "%s", VPSS_FILENAME_IN);
	snprintf(param.fileNameOut, sizeof(param.fileNameOut)-1,
		"1280_720_nv21_out_case%d.yuv", RGN_WITH_VPSS_SEND_FRAME_TEST_ORIG);

	s32Ret = rgn_send_file_test_body(&param);
	return s32Ret;
}

static inline CVI_S32 rgn_vb_init(RGN_TEST_PARAM *param)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	SIZE_S stSize = { .u32Width = param->stInputSize.u32Width,
		.u32Height = param->stInputSize.u32Height};
	SIZE_S stSizeOut = { .u32Width = param->stOutputSize.u32Width,
		.u32Height = param->stOutputSize.u32Height };
	PIXEL_FORMAT_E	pixelFormat = param->eInputFmt;
	PIXEL_FORMAT_E	pixelFormatOut = param->eOutputFmt;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	VB_CONFIG_S	stVbConf;
	CVI_U32		u32BlkSize, u32BlkSizeOut;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt		= 1;

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height, pixelFormat,
					     DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(stSizeOut.u32Width, stSizeOut.u32Height, pixelFormatOut,
						DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	u32BlkSize = u32BlkSize > u32BlkSizeOut ? u32BlkSize : u32BlkSizeOut;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 5;
	RGN_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

	s32Ret = RGN_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "system init failed with %#x\n", s32Ret);
		return -1;
	}
	return s32Ret;
}

static inline CVI_S32 rgn_vpss_init(RGN_TEST_PARAM *param)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	SIZE_S stSize = { .u32Width = param->stInputSize.u32Width,
		.u32Height = param->stInputSize.u32Height};
	SIZE_S stSizeOut = { .u32Width = param->stOutputSize.u32Width,
		.u32Height = param->stOutputSize.u32Height };
	PIXEL_FORMAT_E	pixelFormat = param->eInputFmt;
	PIXEL_FORMAT_E	pixelFormatOut = param->eOutputFmt;

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	VPSS_GRP	VpssGrp = param->stChn.s32DevId;
	VPSS_GRP_ATTR_S	stVpssGrpAttr;
	VPSS_CHN	VpssChn = param->stChn.s32ChnId;
	CVI_BOOL	abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S	astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM] = {0};

	// grp0 for right half
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = pixelFormat;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	astVpssChnAttr[VpssChn].u32Width		    = stSizeOut.u32Width;
	astVpssChnAttr[VpssChn].u32Height		    = stSizeOut.u32Height;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = pixelFormatOut;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[VpssChn] = CVI_TRUE;
	s32Ret = RGN_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = RGN_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

static CVI_S32 rgn_ut_vpss_send_frame(RGN_TEST_PARAM *param)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = param->stChn.s32DevId;
	VPSS_CHN VpssChn = param->stChn.s32ChnId;
	SIZE_S stSize = param->stInputSize;
	PIXEL_FORMAT_E pixelFormat = param->eInputFmt;
	CVI_CHAR *fileName = param->inputFile;
	CVI_CHAR *fileNameOut = param->outputFile;
	CVI_U32 i;

	for (i = 0; i < param->u32RepeatCnt; i++) {
		VIDEO_FRAME_INFO_S stVideoFrame;

		s32Ret = RGN_COMM_VPSS_SendFrame(VpssGrp, &stSize, pixelFormat, fileName);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "[%d] RGN_COMM_VPSS_SendFrame fail. s32Ret: 0x%x !\n",
				i, s32Ret);
			break;
		}

		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, 1000/*-1*/);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "[%d] CVI_VPSS_GetChnFrame. s32Ret: 0x%x !\n",
				i, s32Ret);
			break;
		}

		dump_mem(&stVideoFrame, 32);

		s32Ret = RGN_COMM_FRAME_SaveToFile(fileNameOut, &stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_WARN, "[%d] RGN_COMM_FRAME_SaveToFile. s32Ret: 0x%x !\n",
				i, s32Ret);
			// keep going
		}

		s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "[%d] CVI_VPSS_ReleaseChnFrame. s32Ret: 0x%x !\n",
				i, s32Ret);
			break;
		}
	}
	return s32Ret;
}

#ifndef __CV180X__
static CVI_S32 vo_ut_plat_vo_init(void)
{
	VO_CONFIG_S stVoConfig;
	RECT_S stDefDispRect  = {0, 0, 720, 1280};
	SIZE_S stDefImageSize = {720, 1280};
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_U32 panel_init = false;
	VO_PUB_ATTR_S stVoPubAttr;

	CVI_VO_Get_Panel_Status(1, 0, &panel_init);
	printf("vo_ut_plat_vo_init[%d]\n", panel_init);

	//Hard code
	panel_init = 0;
	if (panel_init) {
		CVI_VO_GetPubAttr(1, &stVoPubAttr);

		printf("Panel w=%d, h=%d.\n",
				stVoPubAttr.stSyncInfo.u16Hact, stVoPubAttr.stSyncInfo.u16Vact);

		stDefDispRect.u32Width = stVoPubAttr.stSyncInfo.u16Hact;
		stDefDispRect.u32Height = stVoPubAttr.stSyncInfo.u16Vact;
		stDefImageSize.u32Width = stVoPubAttr.stSyncInfo.u16Hact;
		stDefImageSize.u32Height = stVoPubAttr.stSyncInfo.u16Vact;
	}

	s32Ret = RGN_COMM_VO_GetDefConfig(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		printf("RGN_COMM_VO_GetDefConfig failed with %#x\n", s32Ret);
		goto error;
	}

	stVoConfig.VoDev	 = 1;
	stVoConfig.stVoPubAttr.enIntfType  = VO_INTF_MIPI;
	stVoConfig.stVoPubAttr.enIntfSync  = VO_OUTPUT_720x1280_60;
	stVoConfig.stDispRect	 = stDefDispRect;
	stVoConfig.stImageSize	 = stDefImageSize;
	stVoConfig.enPixFormat = PIXEL_FORMAT_NV21;
	stVoConfig.enVoMode	 = VO_MODE_1MUX;
	s32Ret = RGN_COMM_VO_StartVO(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		printf("RGN_COMM_VO_StartVO failed with %#x\n", s32Ret);
	}

error:
	return s32Ret;
}
#endif

static CVI_S32 rgn_set_bitmap_with_vpss_sendframe_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	CVI_S32 MinHandle, i;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	param.u32HdlNum = 3;
	param.enType = OVERLAY_RGN;
	Path_BMP = test_bmp;
	PIXEL_FORMAT_E enPixelFormat;

	enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	s32Ret = RGN_COMM_REGION_Create(param.u32HdlNum, param.enType, enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}

	s32Ret = RGN_COMM_REGION_AttachToChn(param.u32HdlNum,
		param.enType, &param.stChn);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_AttachToChn failed!\n");
		goto EXIT2;
	}

	if (param.enType == OVERLAY_RGN || param.enType == OVERLAYEX_RGN) {
		MinHandle = RGN_COMM_REGION_GetMinHandle(param.enType);

		for (i = MinHandle; i < MinHandle + param.u32HdlNum; i++) {
			s32Ret = RGN_COMM_REGION_SetBitMap(i, Path_BMP, enPixelFormat, false);
			if (s32Ret != CVI_SUCCESS) {
				RGN_UT_PRT("RGN_COMM_REGION_SetBitMap failed!\n");
				goto EXIT3;
			}
		}
	}

	char fileName[256];
	char fileNameOut[256];

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut)-1,
		"1280_720_nv21_out_case%d_set_bitmap.yuv", RGN_BIT_MAP_WITH_VPSS_SEND_FRAME_TEST);
	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

EXIT3:
	s32ExtRet = RGN_COMM_REGION_DetachFrmChn(param.u32HdlNum, param.enType, &param.stChn);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_DetachFrmChn failed!\n");
EXIT2:
	s32ExtRet = RGN_COMM_REGION_Destroy(param.u32HdlNum, param.enType);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_update_canvas_with_vpss_sendframe_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	CVI_S32 MinHandle, i;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	param.u32HdlNum = 3;
	param.enType = OVERLAY_RGN;
	Path_BMP = test_bmp;
	PIXEL_FORMAT_E enPixelFormat;

	enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	s32Ret = RGN_COMM_REGION_Create(param.u32HdlNum, param.enType, enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}

	s32Ret = RGN_COMM_REGION_AttachToChn(param.u32HdlNum,
		param.enType, &param.stChn);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_AttachToChn failed!\n");
		goto EXIT2;
	}

	if (param.enType == OVERLAY_RGN || param.enType == OVERLAYEX_RGN) {
		MinHandle = RGN_COMM_REGION_GetMinHandle(param.enType);

		for (i = MinHandle; i < MinHandle + param.u32HdlNum; i++) {
			s32Ret = RGN_COMM_REGION_GetUpCanvas(i, Path_BMP);
			if (s32Ret != CVI_SUCCESS) {
				RGN_UT_PRT("RGN_COMM_REGION_GetUpCanvas failed!\n");
				goto EXIT3;
			}
		}
	}

	char fileName[256];
	char fileNameOut[256];

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut)-1,
		"1280_720_nv21_out_case%d_update_canvas.yuv", RGN_CANVAS_WITH_VPSS_SEND_FRAME_TEST);
	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

EXIT3:
	s32ExtRet = RGN_COMM_REGION_DetachFrmChn(param.u32HdlNum, param.enType, &param.stChn);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_DetachFrmChn failed!\n");
EXIT2:
	s32ExtRet = RGN_COMM_REGION_Destroy(param.u32HdlNum, param.enType);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_8bit_mode_canvas_with_vpss_sendframe_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_PALETTE_S stPalette;
	RGN_TEST_PARAM param;
	CVI_S32 i;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN 8bit mode
	 ************************************************/
	Path_BMP = tiger_8bitmode;
	param.u32HdlNum = 3;
	param.enType = OVERLAY_RGN;
	PIXEL_FORMAT_E enPixelFormat;

	enPixelFormat = PIXEL_FORMAT_8BIT_MODE;
	s32Ret = RGN_COMM_REGION_Create(param.u32HdlNum, param.enType, enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}

	s32Ret = RGN_COMM_REGION_AttachToChn(param.u32HdlNum, param.enType, &param.stChn);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_AttachToChn failed!\n");
		goto EXIT2;
	}

	/* Use indexed palettes format of bmp file in OVERLAY example. */
	for (i = OverlayMinHandle; i < OverlayMinHandle + param.u32HdlNum; i++) {
		s32Ret = RGN_COMM_REGION_SetBitMap(i, Path_BMP, enPixelFormat, false);
		if (s32Ret != CVI_SUCCESS) {
			RGN_UT_PRT("RGN_COMM_REGION_SetBitMap failed!\n");
			goto EXIT3;
		}
	}

	stPalette.pstPaletteTable = overlay_palette;
	stPalette.lut_length = 256;
	stPalette.pixelFormat = RGN_COLOR_FMT_RGB888;
	s32Ret = CVI_RGN_SetChnPalette(OverlayMinHandle, &param.stChn, &stPalette);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_SetChnPalette failed!\n");
		goto EXIT3;
	}

	char fileName[256];
	char fileNameOut[256];

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut)-1,
		"1280_720_nv21_out_case%d_update_8bit_mode_canvas.yuv",
			RGN_8BIT_MODE_CANVAS_WITH_VPSS_SEND_FRAME_TEST);
	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

EXIT3:
	s32ExtRet = RGN_COMM_REGION_DetachFrmChn(param.u32HdlNum, param.enType, &param.stChn);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_DetachFrmChn failed!\n");
EXIT2:
	s32ExtRet = RGN_COMM_REGION_Destroy(param.u32HdlNum, param.enType);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_vpss_coverex_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	RGN_HANDLE Handle;
	RGN_CHN_ATTR_S stRgnChnAttr;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	char fileName[256];
	char fileNameOut[256];

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	param.u32HdlNum = 4;
	param.enType = COVEREX_RGN;

	s32Ret = RGN_COMM_REGION_Create(param.u32HdlNum, param.enType, 0);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}

	s32Ret = RGN_COMM_REGION_AttachToChn(param.u32HdlNum,
		param.enType, &param.stChn);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_AttachToChn failed!\n");
		goto EXIT2;
	}

	Handle = RGN_COMM_REGION_GetMinHandle(param.enType) + 3;

	CVI_RGN_GetDisplayAttr(Handle, &param.stChn, &stRgnChnAttr);
	stRgnChnAttr.unChnAttr.stCoverExChn.u32Color = 0x00ff0000;
	stRgnChnAttr.unChnAttr.stCoverExChn.u32Layer = 0;
	stRgnChnAttr.unChnAttr.stCoverExChn.stRect.s32X = 500;
	stRgnChnAttr.unChnAttr.stCoverExChn.stRect.s32Y = 500;
	CVI_RGN_SetDisplayAttr(Handle, &param.stChn, &stRgnChnAttr);

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut)-1,
		"1280_720_nv21_out_case%d_vpss_coverex.yuv", RGN_VPSS_COVEREX_TEST);
	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

	s32ExtRet = RGN_COMM_REGION_DetachFrmChn(param.u32HdlNum, param.enType, &param.stChn);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_DetachFrmChn failed!\n");
EXIT2:
	s32ExtRet = RGN_COMM_REGION_Destroy(param.u32HdlNum, param.enType);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_vo_cover_test(void)
{
#ifndef __CV180X__
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	RGN_HANDLE Handle;
	RGN_CHN_ATTR_S stRgnChnAttr;
	char fileName[256];
	VIDEO_FRAME_INFO_S stVideoFrame;
	VB_BLK blk;

	/************************************************
	 * Init VB and VO
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VO;
	param.stChn.s32DevId = VO_LAYER_G1;
	param.stChn.s32ChnId = VPSS_CHN0;
	param.stInputSize.u32Width = 1280;
	param.stInputSize.u32Height = 720;
	param.eInputFmt = PIXEL_FORMAT_NV21;
	param.stOutputSize.u32Width = 720;
	param.stOutputSize.u32Height = 1280;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	s32Ret = vo_ut_plat_vo_init();
	if (s32Ret) {
		RGN_UT_PRT("vo_ut_plat_vo_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	param.u32HdlNum = 1;
	param.enType = COVER_RGN;

	s32Ret = RGN_COMM_REGION_Create(param.u32HdlNum, param.enType, 0);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}

	s32Ret = RGN_COMM_REGION_AttachToChn(param.u32HdlNum,
		param.enType, &param.stChn);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_AttachToChn failed!\n");
		goto EXIT2;
	}

	Handle = RGN_COMM_REGION_GetMinHandle(param.enType);

	CVI_RGN_GetDisplayAttr(Handle, &param.stChn, &stRgnChnAttr);
	stRgnChnAttr.unChnAttr.stCoverChn.u32Color = 0x00ff0000;
	stRgnChnAttr.unChnAttr.stCoverChn.u32Layer = 0;
	stRgnChnAttr.unChnAttr.stCoverChn.stRect.s32X = 500;
	stRgnChnAttr.unChnAttr.stCoverChn.stRect.s32Y = 500;
	CVI_RGN_SetDisplayAttr(Handle, &param.stChn, &stRgnChnAttr);

	//send frame
	snprintf(fileName, sizeof(fileName)-1, "%s", VO_FILENAME_IN);
	s32Ret = RGN_COMM_FRAME_LoadFromFile(fileName, &stVideoFrame, &param.stOutputSize,
		param.eOutputFmt);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_FRAME_LoadFromFile failed!\n");
		goto EXIT3;
	}
	s32Ret = CVI_VO_SendFrame(1, 0, &stVideoFrame, 1000);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_VO_SendFrame failed!\n");
		goto EXIT3;
	}

	blk = CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]);
	CVI_VB_ReleaseBlock(blk);
	sleep(1);

EXIT3:
	s32ExtRet = RGN_COMM_REGION_DetachFrmChn(param.u32HdlNum, param.enType, &param.stChn);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_DetachFrmChn failed!\n");
EXIT2:
	s32ExtRet = RGN_COMM_REGION_Destroy(param.u32HdlNum, param.enType);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_Destroy failed!\n");

EXIT1:
	RGN_COMM_VO_Exit();
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
#else
	return CVI_SUCCESS;
#endif
}

static CVI_S32 rgn_vpss_mosaic_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	RGN_HANDLE Handle;
	RGN_CHN_ATTR_S stRgnChnAttr;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	char fileName[256];
	char fileNameOut[256];

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	param.u32HdlNum = 3;
	param.enType = MOSAIC_RGN;

	s32Ret = RGN_COMM_REGION_Create(param.u32HdlNum, param.enType, 0);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}

	s32Ret = RGN_COMM_REGION_AttachToChn(param.u32HdlNum,
		param.enType, &param.stChn);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_AttachToChn failed!\n");
		goto EXIT2;
	}

	Handle = RGN_COMM_REGION_GetMinHandle(param.enType);

	CVI_RGN_GetDisplayAttr(Handle, &param.stChn, &stRgnChnAttr);
	stRgnChnAttr.unChnAttr.stMosaicChn.stRect.s32X = 600;
	stRgnChnAttr.unChnAttr.stMosaicChn.stRect.s32Y = 30;
	stRgnChnAttr.unChnAttr.stMosaicChn.stRect.u32Width = 128;
	stRgnChnAttr.unChnAttr.stMosaicChn.stRect.u32Height = 128;
	CVI_RGN_SetDisplayAttr(Handle, &param.stChn, &stRgnChnAttr);

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut)-1,
		"1280_720_nv21_out_case%d_vpss_mosaic.yuv", RGN_VPSS_MOSAIC_TEST);
	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

	s32ExtRet = RGN_COMM_REGION_DetachFrmChn(param.u32HdlNum, param.enType, &param.stChn);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_DetachFrmChn failed!\n");
EXIT2:
	s32ExtRet = RGN_COMM_REGION_Destroy(param.u32HdlNum, param.enType);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_create_destroy_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_TEST_PARAM param;

	memset(&param, 0, sizeof(param));
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;
	rgn_vb_init(&param);

	/************************************************
	 * step3:  Init RGN
	 ************************************************/
	CVI_S32 HandleNum;
	RGN_TYPE_E enType;

	HandleNum = 3;
	enType = OVERLAY_RGN;
	PIXEL_FORMAT_E enPixelFormat;

	enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	s32Ret = RGN_COMM_REGION_Create(HandleNum, enType, enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}

EXIT1:
	s32ExtRet = RGN_COMM_REGION_Destroy(HandleNum, enType);
	if (s32ExtRet != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
	}

	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_create_attach_detach_destroy_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	CVI_S32 HandleNum;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn;

	HandleNum = 3;
	enType = OVERLAY_RGN;
	stChn.enModId = CVI_ID_VPSS;
	stChn.s32DevId = 0;
	stChn.s32ChnId = 0;
	PIXEL_FORMAT_E enPixelFormat;

	enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	s32Ret = RGN_COMM_REGION_Create(HandleNum, enType, enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}

	s32Ret = RGN_COMM_REGION_AttachToChn(HandleNum, enType, &stChn);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_AttachToChn failed!\n");
		goto EXIT2;
	}

	s32ExtRet = RGN_COMM_REGION_DetachFrmChn(HandleNum, enType, &stChn);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_DetachFrmChn failed!\n");
EXIT2:
	s32ExtRet = RGN_COMM_REGION_Destroy(HandleNum, enType);
	if (s32ExtRet != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
	}
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_attr_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_TEST_PARAM param;

	/************************************************
	 * Init VB
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;
	s32Ret = rgn_vb_init(&param);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("rgn_vb_init failed!\n");
		goto EXIT1;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	CVI_S32 i;
	CVI_S32 MinHandle;
	CVI_S32 HandleNum;
	RGN_TYPE_E enType;
	RGN_ATTR_S stRegion;

	HandleNum = 3;
	enType = OVERLAY_RGN;
	Path_BMP = test_bmp;
	PIXEL_FORMAT_E enPixelFormat;

	enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	s32Ret = RGN_COMM_REGION_Create(HandleNum, enType, enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}

	if (enType == OVERLAY_RGN || enType == OVERLAYEX_RGN) {
		MinHandle = RGN_COMM_REGION_GetMinHandle(enType);

		for (i = MinHandle; i < MinHandle + HandleNum; i++) {
			s32Ret = CVI_RGN_GetAttr(MinHandle, &stRegion);
			if (s32Ret != CVI_SUCCESS) {
				RGN_UT_PRT("CVI_RGN_GetAttr failed!\n");
			}
			RGN_UT_PRT("u32BgColor:	0x%x\n", stRegion.unAttr.stOverlay.u32BgColor);
		}
	}

	//Change BgColor to test set RGN attribute
	stRegion.unAttr.stOverlay.u32BgColor = 0xFF000000;

	s32Ret = CVI_RGN_SetAttr(MinHandle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_SetAttr failed!\n");
	}
	s32Ret = CVI_RGN_GetAttr(MinHandle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_GetAttr failed!\n");
	}
	RGN_UT_PRT("u32BgColor:	0x%x\n", stRegion.unAttr.stOverlay.u32BgColor);

EXIT1:
	s32ExtRet = RGN_COMM_REGION_Destroy(HandleNum, enType);
	if (s32ExtRet != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
	}

	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_display_attr_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	CVI_S32 HandleNum;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn;
	RGN_HANDLE cover_hdl;
	RGN_CHN_ATTR_S stRgnChnAttr;

	// create cover at 1st layer.
	HandleNum = 1;
	enType = COVER_RGN;
	stChn.enModId = CVI_ID_VPSS;
	stChn.s32DevId = 0;
	stChn.s32ChnId = 0;
	PIXEL_FORMAT_E enPixelFormat;

	enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	s32Ret = RGN_COMM_REGION_Create(HandleNum, enType, enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}
	s32Ret = RGN_COMM_REGION_AttachToChn(HandleNum, enType, &stChn);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_AttachToChn failed!\n");
		goto EXIT2;
	}
	cover_hdl = RGN_COMM_REGION_GetMinHandle(enType);

	CVI_RGN_GetDisplayAttr(cover_hdl, &stChn, &stRgnChnAttr);
	RGN_UT_PRT("u32Color:	0x%x\n", stRgnChnAttr.unChnAttr.stCoverChn.u32Color);

	RGN_UT_PRT("Change cover to green!\n");
	stRgnChnAttr.unChnAttr.stCoverChn.u32Color = 0x0000ff00;
	CVI_RGN_SetDisplayAttr(cover_hdl, &stChn, &stRgnChnAttr);

	CVI_RGN_GetDisplayAttr(cover_hdl, &stChn, &stRgnChnAttr);
	RGN_UT_PRT("u32Color:	0x%x\n", stRgnChnAttr.unChnAttr.stCoverChn.u32Color);

	s32ExtRet = RGN_COMM_REGION_DetachFrmChn(HandleNum, enType, &stChn);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_DetachFrmChn failed!\n");
EXIT2:
	s32ExtRet = RGN_COMM_REGION_Destroy(HandleNum, enType);
	if (s32ExtRet != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
	}
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_vo_osd_test(void)
{
#ifndef __CV180X__
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	//VB init for 1920x1080 yuv422 input, 1280x720 nv21 output
	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}
	//Create grp 0 and chn 0
	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}
	//init gVoCtx->rgn_handle in vo_set_chn_attr
	abChnEnable[VPSS_CHN0] = true;
	s32Ret = vo_ut_plat_vo_init();
	if (s32Ret) {
		RGN_UT_PRT("vo_ut_plat_vo_init failed.\n");
		goto EXIT1;
	}

	/************************************************
	 * step3:  Init RGN
	 ************************************************/
	CVI_S32 i;
	CVI_S32 MinHandle;
	CVI_S32 HandleNum;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn;

	HandleNum = 1;
	enType = OVERLAY_RGN;
	stChn.enModId = CVI_ID_VO;
	stChn.s32DevId = VO_LAYER_G1;
	stChn.s32ChnId = 0;
	Path_BMP = test_bmp;
	PIXEL_FORMAT_E enPixelFormat;

	enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	s32Ret = RGN_COMM_REGION_Create(HandleNum, enType, enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT2;
	}
	s32Ret = RGN_COMM_REGION_AttachToChn(HandleNum, enType, &stChn);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_AttachToChn failed!\n");
		goto EXIT3;
	}

	if (enType == OVERLAY_RGN || enType == OVERLAYEX_RGN) {
		MinHandle = RGN_COMM_REGION_GetMinHandle(enType);

		for (i = MinHandle; i < MinHandle + HandleNum; i++) {
			//s32Ret = RGN_COMM_REGION_SetBitMap(i, Path_BMP);
			s32Ret = RGN_COMM_REGION_GetUpCanvas(i, Path_BMP);
			if (s32Ret != CVI_SUCCESS) {
				RGN_UT_PRT("RGN_COMM_REGION_GetUpCanvas failed!\n");
				goto EXIT4;
			}
		}
	}
	sleep(1);

EXIT4:
	s32ExtRet = RGN_COMM_REGION_DetachFrmChn(HandleNum, enType, &stChn);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_DetachFrmChn failed!\n");
EXIT3:
	s32ExtRet = RGN_COMM_REGION_Destroy(HandleNum, enType);
	if (s32ExtRet != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
	}
EXIT2:
	RGN_COMM_VO_Exit();
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
#else
	return CVI_SUCCESS;
#endif
}

static CVI_S32 rgn_compress_test(CVI_U32 u32RgnFormat)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 MinHandle;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn;
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;
	RGN_TEST_PARAM param;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	PIXEL_FORMAT_E aenFormats[3] = {
		PIXEL_FORMAT_ARGB_8888, PIXEL_FORMAT_ARGB_1555, PIXEL_FORMAT_ARGB_4444
	};
	CVI_U32 au32Colors[3][8] = {
		{0xffffffff, 0xff000000, 0xff0000ff, 0xff00ff00,
			0xffff0000, 0xff00ffff, 0xffffff00, 0xffff00ff},
		{0xffff, 0x8000, 0x801f, 0x83e0, 0xfc00, 0x83ff, 0xffe0, 0xfc1f},
		{0xffff, 0xf000, 0xf00f, 0xf0f0, 0xff00, 0xf0ff, 0xfff0, 0xff0f}
	};

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	enType = OVERLAY_RGN;
	stChn.enModId = CVI_ID_VPSS;
	stChn.s32DevId = 0;
	stChn.s32ChnId = 0;

	MinHandle = RGN_COMM_REGION_GetMinHandle(enType);

	stRegion.enType = enType;
	stRegion.unAttr.stOverlay.enPixelFormat = aenFormats[u32RgnFormat];
	stRegion.unAttr.stOverlay.stSize.u32Width = 1280;
	stRegion.unAttr.stOverlay.stSize.u32Height = 720;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00;
	stRegion.unAttr.stOverlay.u32CanvasNum = 2;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_SW;
	stRegion.unAttr.stOverlay.stCompressInfo.u32EstCompressedSize = RGN_CMPR_MIN_SIZE;

	s32Ret = CVI_RGN_Create(MinHandle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT1;
	}

	stChnAttr.bShow = true;
	stChnAttr.enType = enType;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
	s32Ret = CVI_RGN_AttachToChn(MinHandle, &stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT2;
	}

	RGN_CANVAS_CMPR_ATTR_S *pstCanvasCmprAttr;
	RGN_CMPR_OBJ_ATTR_S *pstObjAttr;
	RGN_CANVAS_INFO_S stCanvasInfo;

	s32Ret = CVI_RGN_GetCanvasInfo(MinHandle, &stCanvasInfo);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_GetCanvasInfo failed with %#x!\n", s32Ret);
		goto EXIT3;
	}

	pstCanvasCmprAttr = stCanvasInfo.pstCanvasCmprAttr;
	pstObjAttr = stCanvasInfo.pstObjAttr;

	pstCanvasCmprAttr->u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
	pstCanvasCmprAttr->u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
	pstCanvasCmprAttr->u32BgColor =  stRegion.unAttr.stOverlay.u32BgColor;
	pstCanvasCmprAttr->enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
	pstCanvasCmprAttr->u32BsSize = stRegion.unAttr.stOverlay.stCompressInfo.u32EstCompressedSize;
	pstCanvasCmprAttr->u32ObjNum = 12;

	pstObjAttr[0].stRgnRect.stRect.s32X = 0;
	pstObjAttr[0].stRgnRect.stRect.s32Y = 0;
	pstObjAttr[0].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[0].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[0].stRgnRect.u32Thick = 1;
	pstObjAttr[0].stRgnRect.u32Color = au32Colors[u32RgnFormat][0];
	pstObjAttr[0].stRgnRect.u32IsFill = false;
	pstObjAttr[0].enObjType = RGN_CMPR_RECT;
	pstObjAttr[1].stRgnRect.stRect.s32X = 100;
	pstObjAttr[1].stRgnRect.stRect.s32Y = 100;
	pstObjAttr[1].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[1].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[1].stRgnRect.u32Thick = 2;
	pstObjAttr[1].stRgnRect.u32Color = au32Colors[u32RgnFormat][1];
	pstObjAttr[1].stRgnRect.u32IsFill = false;
	pstObjAttr[1].enObjType = RGN_CMPR_RECT;
	pstObjAttr[2].stRgnRect.stRect.s32X = 200;
	pstObjAttr[2].stRgnRect.stRect.s32Y = 200;
	pstObjAttr[2].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[2].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[2].stRgnRect.u32Thick = 3;
	pstObjAttr[2].stRgnRect.u32Color = au32Colors[u32RgnFormat][2];
	pstObjAttr[2].stRgnRect.u32IsFill = false;
	pstObjAttr[2].enObjType = RGN_CMPR_RECT;
	pstObjAttr[3].stRgnRect.stRect.s32X = 300;
	pstObjAttr[3].stRgnRect.stRect.s32Y = 300;
	pstObjAttr[3].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[3].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[3].stRgnRect.u32Thick = 4;
	pstObjAttr[3].stRgnRect.u32Color = au32Colors[u32RgnFormat][3];
	pstObjAttr[3].stRgnRect.u32IsFill = false;
	pstObjAttr[3].enObjType = RGN_CMPR_RECT;
	pstObjAttr[4].stRgnRect.stRect.s32X = 400;
	pstObjAttr[4].stRgnRect.stRect.s32Y = 300;
	pstObjAttr[4].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[4].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[4].stRgnRect.u32Thick = 5;
	pstObjAttr[4].stRgnRect.u32Color = au32Colors[u32RgnFormat][4];
	pstObjAttr[4].stRgnRect.u32IsFill = false;
	pstObjAttr[4].enObjType = RGN_CMPR_RECT;
	pstObjAttr[5].stRgnRect.stRect.s32X = 500;
	pstObjAttr[5].stRgnRect.stRect.s32Y = 200;
	pstObjAttr[5].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[5].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[5].stRgnRect.u32Thick = 6;
	pstObjAttr[5].stRgnRect.u32Color = au32Colors[u32RgnFormat][5];
	pstObjAttr[5].stRgnRect.u32IsFill = false;
	pstObjAttr[5].enObjType = RGN_CMPR_RECT;
	pstObjAttr[6].stRgnRect.stRect.s32X = 600;
	pstObjAttr[6].stRgnRect.stRect.s32Y = 100;
	pstObjAttr[6].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[6].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[6].stRgnRect.u32Thick = 7;
	pstObjAttr[6].stRgnRect.u32Color = au32Colors[u32RgnFormat][6];
	pstObjAttr[6].stRgnRect.u32IsFill = false;
	pstObjAttr[6].enObjType = RGN_CMPR_RECT;
	pstObjAttr[7].stRgnRect.stRect.s32X = 700;
	pstObjAttr[7].stRgnRect.stRect.s32Y = 000;
	pstObjAttr[7].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[7].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[7].stRgnRect.u32Thick = 8;
	pstObjAttr[7].stRgnRect.u32Color = au32Colors[u32RgnFormat][7];
	pstObjAttr[7].stRgnRect.u32IsFill = false;
	pstObjAttr[7].enObjType = RGN_CMPR_RECT;

	pstObjAttr[8].stLine.stPointStart.s32X = 600;
	pstObjAttr[8].stLine.stPointStart.s32Y = 200;
	pstObjAttr[8].stLine.stPointEnd.s32X = 300;
	pstObjAttr[8].stLine.stPointEnd.s32Y = 400;
	pstObjAttr[8].stLine.u32Thick = 8;
	pstObjAttr[8].stLine.u32Color = au32Colors[u32RgnFormat][7];
	pstObjAttr[8].enObjType = RGN_CMPR_LINE;
	pstObjAttr[9].stLine.stPointStart.s32X = 300;
	pstObjAttr[9].stLine.stPointStart.s32Y = 400;
	pstObjAttr[9].stLine.stPointEnd.s32X = 800;
	pstObjAttr[9].stLine.stPointEnd.s32Y = 700;
	pstObjAttr[9].stLine.u32Thick = 8;
	pstObjAttr[9].stLine.u32Color = au32Colors[u32RgnFormat][7];
	pstObjAttr[9].enObjType = RGN_CMPR_LINE;
	pstObjAttr[10].stLine.stPointStart.s32X = 800;
	pstObjAttr[10].stLine.stPointStart.s32Y = 700;
	pstObjAttr[10].stLine.stPointEnd.s32X = 1100;
	pstObjAttr[10].stLine.stPointEnd.s32Y = 600;
	pstObjAttr[10].stLine.u32Thick = 8;
	pstObjAttr[10].stLine.u32Color = au32Colors[u32RgnFormat][7];
	pstObjAttr[10].enObjType = RGN_CMPR_LINE;
	pstObjAttr[11].stLine.stPointStart.s32X = 1100;
	pstObjAttr[11].stLine.stPointStart.s32Y = 600;
	pstObjAttr[11].stLine.stPointEnd.s32X = 600;
	pstObjAttr[11].stLine.stPointEnd.s32Y = 200;
	pstObjAttr[11].stLine.u32Thick = 8;
	pstObjAttr[11].stLine.u32Color = au32Colors[u32RgnFormat][7];
	pstObjAttr[11].enObjType = RGN_CMPR_LINE;

	s32Ret = CVI_RGN_UpdateCanvas(MinHandle);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_UpdateCanvas failed with %#x!\n", s32Ret);
		goto EXIT3;
	}

	char fileName[256];
	char fileNameOut[256];

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut)-1, "1280_720_nv21_out_case%d_cmpr_argb%s.yuv",
		RGN_COMPRESSED_SIMPLEOBJS_ARGB8888_TEST + u32RgnFormat,
		u32RgnFormat == RGN_UT_ARGB8888 ? "8888" : (u32RgnFormat == RGN_UT_ARGB1555 ? "1555" : "4444"));
	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

EXIT3:
	CVI_RGN_DetachFromChn(MinHandle, &stChn);
EXIT2:
	CVI_RGN_Destroy(MinHandle);
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_compress_simpleobjs_bitmap_test(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 MinHandle;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn;
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;
	RGN_TEST_PARAM param;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	BITMAP_S stBitmap, stBitmapText;
	CVI_U64 u64BitmapPhyAddr, u64BitmapTextPhyAddr;
	CVI_VOID *pBitmapVirAddr, *pBitmapTextVirAddr;
	char szStr[MAX_STR_LEN];

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	enType = OVERLAY_RGN;
	stChn.enModId = CVI_ID_VPSS;
	stChn.s32DevId = 0;
	stChn.s32ChnId = 0;

	MinHandle = RGN_COMM_REGION_GetMinHandle(enType);

	stRegion.enType = OVERLAY_RGN;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	stRegion.unAttr.stOverlay.stSize.u32Width = 1280;
	stRegion.unAttr.stOverlay.stSize.u32Height = 720;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00;
	stRegion.unAttr.stOverlay.u32CanvasNum = 2;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_SW;
	stRegion.unAttr.stOverlay.stCompressInfo.u32EstCompressedSize = RGN_CMPR_MIN_SIZE;

	s32Ret = CVI_RGN_Create(MinHandle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT1;
	}

	stChnAttr.bShow = true;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
	s32Ret = CVI_RGN_AttachToChn(MinHandle, &stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT2;
	}

	RGN_CANVAS_CMPR_ATTR_S * pstCanvasCmprAttr;
	RGN_CMPR_OBJ_ATTR_S *pstObjAttr;
	RGN_CANVAS_INFO_S stCanvasInfo;

	s32Ret = CVI_RGN_GetCanvasInfo(MinHandle, &stCanvasInfo);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_GetCanvasInfo failed with %#x!\n", s32Ret);
		goto EXIT3;
	}

	pstCanvasCmprAttr = stCanvasInfo.pstCanvasCmprAttr;
	pstObjAttr = stCanvasInfo.pstObjAttr;

	pstCanvasCmprAttr->u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
	pstCanvasCmprAttr->u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
	pstCanvasCmprAttr->u32BgColor =  stRegion.unAttr.stOverlay.u32BgColor;
	pstCanvasCmprAttr->enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
	pstCanvasCmprAttr->u32BsSize = stRegion.unAttr.stOverlay.stCompressInfo.u32EstCompressedSize;
	pstCanvasCmprAttr->u32ObjNum = 13;

	CVI_U32 u32Colors[8] = {0xffff, 0x8000, 0x801f, 0x83e0, 0xfc00, 0xffe0, 0xfc1f, 0x83ff};

	for (CVI_S32 i = 0; i < 8; i++) {
		pstObjAttr[i].stRgnRect.stRect.s32X = i * 100;
		pstObjAttr[i].stRgnRect.stRect.s32Y = (i >= 4) ? (7 - i) * 100 : i * 100;
		pstObjAttr[i].stRgnRect.stRect.u32Width = 200;
		pstObjAttr[i].stRgnRect.stRect.u32Height = 200;
		pstObjAttr[i].stRgnRect.u32Thick = i;
		pstObjAttr[i].stRgnRect.u32Color = u32Colors[i];
		pstObjAttr[i].stRgnRect.u32IsFill = false;
		pstObjAttr[i].enObjType = RGN_CMPR_RECT;
	}

	pstObjAttr[8].stLine.stPointStart.s32X = 600;
	pstObjAttr[8].stLine.stPointStart.s32Y = 200;
	pstObjAttr[8].stLine.stPointEnd.s32X = 300;
	pstObjAttr[8].stLine.stPointEnd.s32Y = 400;
	pstObjAttr[8].stLine.u32Thick = 8;
	pstObjAttr[8].stLine.u32Color = 0xe318;
	pstObjAttr[8].enObjType = RGN_CMPR_LINE;
	pstObjAttr[9].stLine.stPointStart.s32X = 300;
	pstObjAttr[9].stLine.stPointStart.s32Y = 400;
	pstObjAttr[9].stLine.stPointEnd.s32X = 800;
	pstObjAttr[9].stLine.stPointEnd.s32Y = 700;
	pstObjAttr[9].stLine.u32Thick = 8;
	pstObjAttr[9].stLine.u32Color = 0xe318;
	pstObjAttr[9].enObjType = RGN_CMPR_LINE;
	pstObjAttr[10].stLine.stPointStart.s32X = 800;
	pstObjAttr[10].stLine.stPointStart.s32Y = 700;
	pstObjAttr[10].stLine.stPointEnd.s32X = 1100;
	pstObjAttr[10].stLine.stPointEnd.s32Y = 600;
	pstObjAttr[10].stLine.u32Thick = 8;
	pstObjAttr[10].stLine.u32Color = 0xe318;
	pstObjAttr[10].enObjType = RGN_CMPR_LINE;
	pstObjAttr[11].stLine.stPointStart.s32X = 1100;
	pstObjAttr[11].stLine.stPointStart.s32Y = 600;
	pstObjAttr[11].stLine.stPointEnd.s32X = 600;
	pstObjAttr[11].stLine.stPointEnd.s32Y = 200;
	pstObjAttr[11].stLine.u32Thick = 8;
	pstObjAttr[11].stLine.u32Color = 0xe318;
	pstObjAttr[11].enObjType = RGN_CMPR_LINE;

	s32Ret = RGN_COMM_REGION_MST_LoadBmp(tiger_bmp, &stBitmap, CVI_FALSE, 0x00,
		pstCanvasCmprAttr->enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_MST_LoadBmp failed with %#x!\n", s32Ret);
		goto EXIT3;
	}
	s32Ret = CVI_SYS_IonAlloc(&u64BitmapPhyAddr, (CVI_VOID **)&pBitmapVirAddr, "rgn_cmpr_bitmap1",
			stBitmap.u32Width * stBitmap.u32Height * 2);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_SYS_IonAlloc failed with %#x!\n", s32Ret);
		goto EXIT4;
	}
	memcpy(pBitmapVirAddr, stBitmap.pData, stBitmap.u32Width * stBitmap.u32Height * 2);
	pstObjAttr[12].stBitmap.stRect.s32X = 20;
	pstObjAttr[12].stBitmap.stRect.s32Y = 100;
	pstObjAttr[12].stBitmap.stRect.u32Width = stBitmap.u32Width;
	pstObjAttr[12].stBitmap.stRect.u32Height = stBitmap.u32Height;
	pstObjAttr[12].stBitmap.u64BitmapPAddr = u64BitmapPhyAddr;
	pstObjAttr[12].enObjType = RGN_CMPR_BIT_MAP;

	rgn_GetTimeStr(NULL, szStr, MAX_STR_LEN);
	s32Ret = rgn_TimeBitmap(szStr, &stBitmapText, 0x9ce7, 0x7fff);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("rgn_TimeBitmap failed with %#x!\n", s32Ret);
		goto EXIT5;
	}
	s32Ret = CVI_SYS_IonAlloc(&u64BitmapTextPhyAddr, (CVI_VOID **)&pBitmapTextVirAddr,
		"rgn_cmpr_bitmap2", stBitmapText.u32Width * stBitmapText.u32Height * 2);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_SYS_IonAlloc failed with %#x!\n", s32Ret);
		goto EXIT6;
	}
	memcpy(pBitmapTextVirAddr, stBitmapText.pData, stBitmapText.u32Width * stBitmapText.u32Height * 2);
	pstObjAttr[13].stBitmap.stRect.s32X = 20;
	pstObjAttr[13].stBitmap.stRect.s32Y = 40;
	pstObjAttr[13].stBitmap.stRect.u32Width = stBitmapText.u32Width;
	pstObjAttr[13].stBitmap.stRect.u32Height = stBitmapText.u32Height;
	pstObjAttr[13].stBitmap.u64BitmapPAddr = u64BitmapTextPhyAddr;
	pstObjAttr[13].enObjType = RGN_CMPR_BIT_MAP;

	s32Ret = CVI_RGN_UpdateCanvas(MinHandle);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_UpdateCanvas failed with %#x!\n", s32Ret);
		goto EXIT7;
	}

	char fileName[256];
	char fileNameOut[256];

	snprintf(fileName, sizeof(fileName) - 1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut) - 1,
		"1280_720_nv21_out_case%d_cmpr_simpleobjs_bitmap.yuv",
		RGN_COMPRESSED_SIMPLEOBJS_BITMAP_TEST);
	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

EXIT7:
	CVI_SYS_IonFree(u64BitmapTextPhyAddr, pBitmapTextVirAddr);
EXIT6:
	free(stBitmapText.pData);
EXIT5:
	CVI_SYS_IonFree(u64BitmapPhyAddr, pBitmapVirAddr);
EXIT4:
	free(stBitmap.pData);
EXIT3:
	CVI_RGN_DetachFromChn(MinHandle, &stChn);
EXIT2:
	CVI_RGN_Destroy(MinHandle);
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_compressed_normal_mixed_test(void)
{
#ifndef __CV180X__
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	CVI_S32 MinHandle, MinHandleOdec;
	CVI_U32 i;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn;
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;
	RGN_TEST_PARAM param;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	CVI_U32 u32Colors[8] = {0xffff, 0x8000, 0x801f, 0x83e0, 0xfc00, 0xffe0, 0xfc1f, 0x83ff};

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN OSD decode
	 ************************************************/
	// compressed rgn settings
	enType = OVERLAY_RGN;
	stChn.enModId = CVI_ID_VPSS;
	stChn.s32DevId = 0;
	stChn.s32ChnId = 0;
	MinHandleOdec = 100;

	stRegion.enType = enType;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	stRegion.unAttr.stOverlay.stSize.u32Width = 1280;
	stRegion.unAttr.stOverlay.stSize.u32Height = 720;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00;
	stRegion.unAttr.stOverlay.u32CanvasNum = 2;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_SW;
	stRegion.unAttr.stOverlay.stCompressInfo.u32EstCompressedSize = RGN_CMPR_MIN_SIZE;

	s32Ret = CVI_RGN_Create(MinHandleOdec, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT1;
	}

	stChnAttr.bShow = true;
	stChnAttr.enType = enType;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
	s32Ret = CVI_RGN_AttachToChn(MinHandleOdec, &stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT2;
	}

	RGN_CANVAS_CMPR_ATTR_S *pstCanvasCmprAttr;
	RGN_CMPR_OBJ_ATTR_S *pstObjAttr;
	RGN_CANVAS_INFO_S stCanvasInfo;

	s32Ret = CVI_RGN_GetCanvasInfo(MinHandleOdec, &stCanvasInfo);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_GetCanvasInfo failed with %#x!\n", s32Ret);
		goto EXIT3;
	}

	pstCanvasCmprAttr = stCanvasInfo.pstCanvasCmprAttr;
	pstObjAttr = stCanvasInfo.pstObjAttr;

	pstCanvasCmprAttr->u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
	pstCanvasCmprAttr->u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
	pstCanvasCmprAttr->u32BgColor =  stRegion.unAttr.stOverlay.u32BgColor;
	pstCanvasCmprAttr->enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
	pstCanvasCmprAttr->u32BsSize = stRegion.unAttr.stOverlay.stCompressInfo.u32EstCompressedSize;
	pstCanvasCmprAttr->u32ObjNum = 8;

	for (i = 0; i < pstCanvasCmprAttr->u32ObjNum; ++i) {
		pstObjAttr[i].stRgnRect.stRect.s32X = i * 50;
		pstObjAttr[i].stRgnRect.stRect.s32Y = i * 40;
		pstObjAttr[i].stRgnRect.stRect.u32Width = 600;
		pstObjAttr[i].stRgnRect.stRect.u32Height = 400;
		pstObjAttr[i].stRgnRect.u32Thick = i;
		pstObjAttr[i].stRgnRect.u32Color = u32Colors[i];
		pstObjAttr[i].stRgnRect.u32IsFill = false;
		pstObjAttr[i].enObjType = RGN_CMPR_RECT;
	}
	s32Ret = CVI_RGN_UpdateCanvas(MinHandleOdec);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_UpdateCanvas failed with %#x!\n", s32Ret);
		goto EXIT3;
	}

	// normal rgn settings
	param.u32HdlNum = 3;
	param.enType = OVERLAY_RGN;
	Path_BMP = test_bmp;
	PIXEL_FORMAT_E OSDpixelFormat;

	OSDpixelFormat = PIXEL_FORMAT_ARGB_1555;
	s32Ret = RGN_COMM_REGION_Create(param.u32HdlNum, param.enType, OSDpixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT3;
	}

	s32Ret = RGN_COMM_REGION_AttachToChn(param.u32HdlNum,
		param.enType, &param.stChn);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("RGN_COMM_REGION_AttachToChn failed!\n");
		goto EXIT4;
	}

	if (param.enType == OVERLAY_RGN || param.enType == OVERLAYEX_RGN) {
		MinHandle = RGN_COMM_REGION_GetMinHandle(param.enType);

		for (i = MinHandle; (CVI_S32)i < MinHandle + param.u32HdlNum; i++) {
			s32Ret = RGN_COMM_REGION_GetUpCanvas(i, Path_BMP);
			if (s32Ret != CVI_SUCCESS) {
				RGN_UT_PRT("RGN_COMM_REGION_GetUpCanvas failed!\n");
				goto EXIT5;
			}
		}
	}

	char fileName[256];
	char fileNameOut[256];

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut)-1,
		"1280_720_nv21_out_case%d_cmpr_normal_mixed.yuv",
		RGN_COMPRESSED_NORMAL_MIXED_TEST);
	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

EXIT5:
	s32ExtRet = RGN_COMM_REGION_DetachFrmChn(param.u32HdlNum, param.enType, &param.stChn);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_DetachFrmChn failed!\n");
EXIT4:
	s32ExtRet = RGN_COMM_REGION_Destroy(param.u32HdlNum, param.enType);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
EXIT3:
	s32ExtRet = CVI_RGN_DetachFromChn(MinHandleOdec, &param.stChn);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("CVI_RGN_DetachFromChn failed!\n");
EXIT2:
	s32ExtRet = CVI_RGN_Destroy(MinHandleOdec);
	if (s32ExtRet != CVI_SUCCESS)
		RGN_UT_PRT("CVI_RGN_Destroy failed!\n");
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
#else
	return CVI_SUCCESS;
#endif

}

static CVI_S32 rgn_vpss_formats_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	CVI_S32 Handle;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;
	BITMAP_S stBitmap;
	FILE *fp = NULL;
	CVI_U32 u32FileSize = 0;

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	Handle = 0;
	stRegion.enType = OVERLAY_RGN;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_8888;
	stRegion.unAttr.stOverlay.stSize.u32Width = 72;
	stRegion.unAttr.stOverlay.stSize.u32Height = 60;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00000000;
	stRegion.unAttr.stOverlay.u32CanvasNum = 1;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
	s32Ret = CVI_RGN_Create(Handle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT1;
	}

	memset(&stChnAttr, 0, sizeof(stChnAttr));
	stChnAttr.bShow = CVI_TRUE;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = CVI_FALSE;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 20;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 20;
	stChnAttr.unChnAttr.stOverlayChn.u32Layer = Handle;
	s32Ret = CVI_RGN_AttachToChn(Handle, &param.stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT2;
	}
	fp = fopen(dog_argb8888_bin, "rb");
	if (fp == NULL) {
		RGN_UT_PRT("fopen failed!\n");
		goto EXIT3;
	}
	fseek(fp, 0L, SEEK_END);
	u32FileSize = ftell(fp);
	rewind(fp);
	stBitmap.pData = malloc(u32FileSize);
	if (stBitmap.pData == NULL) {
		RGN_UT_PRT("malloc size(%d) failed!\n", u32FileSize);
		fclose(fp);
		goto EXIT3;
	}
	fread(stBitmap.pData, u32FileSize, 1, fp);
	fclose(fp);

	stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
	stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
	stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;

	s32Ret = CVI_RGN_SetBitMap(Handle, &stBitmap);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
		free(stBitmap.pData);
		goto EXIT3;
	}
	free(stBitmap.pData);

	Handle = 1;
	stRegion.enType = OVERLAY_RGN;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_4444;
	stRegion.unAttr.stOverlay.stSize.u32Width = 80;
	stRegion.unAttr.stOverlay.stSize.u32Height = 60;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00000000;
	stRegion.unAttr.stOverlay.u32CanvasNum = 1;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
	s32Ret = CVI_RGN_Create(Handle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT3;
	}

	memset(&stChnAttr, 0, sizeof(stChnAttr));
	stChnAttr.bShow = CVI_TRUE;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = CVI_FALSE;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 120;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 20;
	stChnAttr.unChnAttr.stOverlayChn.u32Layer = Handle;
	s32Ret = CVI_RGN_AttachToChn(Handle, &param.stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT4;
	}

	fp = fopen(dog_argb4444_bin, "rb");
	if (fp == NULL) {
		RGN_UT_PRT("fopen failed!\n");
		goto EXIT5;
	}
	fseek(fp, 0L, SEEK_END);
	u32FileSize = ftell(fp);
	rewind(fp);
	stBitmap.pData = malloc(u32FileSize);
	if (stBitmap.pData == NULL) {
		RGN_UT_PRT("malloc size(%d) failed!\n", u32FileSize);
		fclose(fp);
		goto EXIT5;
	}
	fread(stBitmap.pData, u32FileSize, 1, fp);
	fclose(fp);

	stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
	stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
	stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;

	s32Ret = CVI_RGN_SetBitMap(Handle, &stBitmap);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
		free(stBitmap.pData);
		goto EXIT5;
	}
	free(stBitmap.pData);

	Handle = 2;
	stRegion.enType = OVERLAY_RGN;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	stRegion.unAttr.stOverlay.stSize.u32Width = 80;
	stRegion.unAttr.stOverlay.stSize.u32Height = 60;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00000000;
	stRegion.unAttr.stOverlay.u32CanvasNum = 1;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
	s32Ret = CVI_RGN_Create(Handle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT5;
	}

	memset(&stChnAttr, 0, sizeof(stChnAttr));
	stChnAttr.bShow = CVI_TRUE;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = CVI_FALSE;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 220;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 20;
	stChnAttr.unChnAttr.stOverlayChn.u32Layer = Handle;
	s32Ret = CVI_RGN_AttachToChn(Handle, &param.stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT6;
	}

	fp = fopen(dog_argb1555_bin, "rb");
	if (fp == NULL) {
		RGN_UT_PRT("fopen failed!\n");
		goto EXIT7;
	}
	fseek(fp, 0L, SEEK_END);
	u32FileSize = ftell(fp);
	rewind(fp);
	stBitmap.pData = malloc(u32FileSize);
	if (stBitmap.pData == NULL) {
		RGN_UT_PRT("malloc size(%d) failed!\n", u32FileSize);
		fclose(fp);
		goto EXIT7;
	}
	fread(stBitmap.pData, u32FileSize, 1, fp);
	fclose(fp);

	stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
	stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
	stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;

	s32Ret = CVI_RGN_SetBitMap(Handle, &stBitmap);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
		free(stBitmap.pData);
		goto EXIT5;
	}
	free(stBitmap.pData);

	char fileName[256];
	char fileNameOut[256];

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut)-1,
		"1280_720_nv21_out_case%d_formats_test.yuv", RGN_VPSS_FORMATS_TEST);
	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

EXIT7:
	s32Ret = CVI_RGN_DetachFromChn(2, &param.stChn);
	if (s32Ret != CVI_SUCCESS)
		RGN_UT_PRT("CVI_RGN_DetachFromChn Handle(2) failed!\n");
EXIT6:
	s32Ret = CVI_RGN_Destroy(2);
	if (s32Ret != CVI_SUCCESS)
		RGN_UT_PRT("CVI_RGN_Destroy Handle(2) failed!\n");
EXIT5:
	s32Ret = CVI_RGN_DetachFromChn(1, &param.stChn);
	if (s32Ret != CVI_SUCCESS)
		RGN_UT_PRT("CVI_RGN_DetachFromChn Handle(1) failed!\n");
EXIT4:
	s32Ret = CVI_RGN_Destroy(1);
	if (s32Ret != CVI_SUCCESS)
		RGN_UT_PRT("CVI_RGN_Destroy Handle(1) failed!\n");
EXIT3:
	s32Ret = CVI_RGN_DetachFromChn(0, &param.stChn);
	if (s32Ret != CVI_SUCCESS)
		RGN_UT_PRT("CVI_RGN_DetachFromChn Handle(0) failed!\n");
EXIT2:
	s32Ret = CVI_RGN_Destroy(0);
	if (s32Ret != CVI_SUCCESS)
		RGN_UT_PRT("CVI_RGN_Destroy Handle(0) failed!\n");
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_vpss_sc_v1_capability_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 MinHandle;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn;
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;
	RGN_TEST_PARAM param;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	CVI_U32 au32Colors[4] = {0xffffffff, 0xff000000, 0xff0000ff, 0xff00ff00};

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 4608;
	param.stInputSize.u32Height = 8188;
	param.eInputFmt = PIXEL_FORMAT_NV21;
	param.stOutputSize.u32Width = 4608;
	param.stOutputSize.u32Height = 8188;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	enType = OVERLAY_RGN;
	stChn.enModId = CVI_ID_VPSS;
	stChn.s32DevId = 0;
	stChn.s32ChnId = 0;

	MinHandle = RGN_COMM_REGION_GetMinHandle(enType);

	stRegion.enType = enType;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_8888;
	stRegion.unAttr.stOverlay.stSize.u32Width = 4608;
	stRegion.unAttr.stOverlay.stSize.u32Height = 8188;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00;
	stRegion.unAttr.stOverlay.u32CanvasNum = 2;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_SW;
	stRegion.unAttr.stOverlay.stCompressInfo.u32EstCompressedSize = RGN_CMPR_MIN_SIZE * 9;

	s32Ret = CVI_RGN_Create(MinHandle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT1;
	}

	stChnAttr.bShow = true;
	stChnAttr.enType = enType;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
	s32Ret = CVI_RGN_AttachToChn(MinHandle, &stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT2;
	}

	RGN_CANVAS_CMPR_ATTR_S *pstCanvasCmprAttr;
	RGN_CMPR_OBJ_ATTR_S *pstObjAttr;
	RGN_CANVAS_INFO_S stCanvasInfo;

	s32Ret = CVI_RGN_GetCanvasInfo(MinHandle, &stCanvasInfo);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_GetCanvasInfo failed with %#x!\n", s32Ret);
		goto EXIT3;
	}

	pstCanvasCmprAttr = stCanvasInfo.pstCanvasCmprAttr;
	pstObjAttr = stCanvasInfo.pstObjAttr;

	pstCanvasCmprAttr->u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
	pstCanvasCmprAttr->u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
	pstCanvasCmprAttr->u32BgColor =  stRegion.unAttr.stOverlay.u32BgColor;
	pstCanvasCmprAttr->enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
	pstCanvasCmprAttr->u32BsSize = stRegion.unAttr.stOverlay.stCompressInfo.u32EstCompressedSize;
	pstCanvasCmprAttr->u32ObjNum = 4;

	pstObjAttr[0].stRgnRect.stRect.s32X = 0;
	pstObjAttr[0].stRgnRect.stRect.s32Y = 0;
	pstObjAttr[0].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[0].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[0].stRgnRect.u32Thick = 1;
	pstObjAttr[0].stRgnRect.u32Color = au32Colors[0];
	pstObjAttr[0].stRgnRect.u32IsFill = false;
	pstObjAttr[0].enObjType = RGN_CMPR_RECT;
	pstObjAttr[1].stRgnRect.stRect.s32X = 100;
	pstObjAttr[1].stRgnRect.stRect.s32Y = 100;
	pstObjAttr[1].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[1].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[1].stRgnRect.u32Thick = 2;
	pstObjAttr[1].stRgnRect.u32Color = au32Colors[1];
	pstObjAttr[1].stRgnRect.u32IsFill = false;
	pstObjAttr[1].enObjType = RGN_CMPR_RECT;
	pstObjAttr[2].stRgnRect.stRect.s32X = 200;
	pstObjAttr[2].stRgnRect.stRect.s32Y = 200;
	pstObjAttr[2].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[2].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[2].stRgnRect.u32Thick = 3;
	pstObjAttr[2].stRgnRect.u32Color = au32Colors[2];
	pstObjAttr[2].stRgnRect.u32IsFill = false;
	pstObjAttr[2].enObjType = RGN_CMPR_RECT;
	pstObjAttr[3].stRgnRect.stRect.s32X = 300;
	pstObjAttr[3].stRgnRect.stRect.s32Y = 300;
	pstObjAttr[3].stRgnRect.stRect.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width - 4;
	pstObjAttr[3].stRgnRect.stRect.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height - 4;
	pstObjAttr[3].stRgnRect.u32Thick = 4;
	pstObjAttr[3].stRgnRect.u32Color = au32Colors[3];
	pstObjAttr[3].stRgnRect.u32IsFill = false;
	pstObjAttr[3].enObjType = RGN_CMPR_RECT;

	s32Ret = CVI_RGN_UpdateCanvas(MinHandle);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_UpdateCanvas failed with %#x!\n", s32Ret);
		goto EXIT3;
	}

	char fileName[256];
	char fileNameOut[256];

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN1);
	snprintf(fileNameOut, sizeof(fileNameOut)-1,
		"4608_8188_nv21_out_case%d_sc_v1_capability_test.yuv",
		RGN_VPSS_SC_V1_CAPABILITY_TEST);

	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_ut_vpss_send_frame failed.\n");
		goto EXIT3;
	}

EXIT3:
	s32Ret = CVI_RGN_DetachFromChn(MinHandle, &param.stChn);
	if (s32Ret != CVI_SUCCESS)
		RGN_UT_PRT("CVI_RGN_DetachFromChn MinHandle failed!\n");
EXIT2:
	s32Ret = CVI_RGN_Destroy(MinHandle);
	if (s32Ret != CVI_SUCCESS)
		RGN_UT_PRT("CVI_RGN_Destroy MinHandle failed!\n");
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_vpss_sc_v2_capability_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	CVI_S32 Handle;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;
	BITMAP_S stBitmap;
	FILE *fp = NULL;
	CVI_U32 u32FileSize = 0;

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN1; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1920;
	param.stOutputSize.u32Height = 1080;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN1] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	Handle = 0;
	stRegion.enType = OVERLAY_RGN;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_8888;
	stRegion.unAttr.stOverlay.stSize.u32Width = 72;
	stRegion.unAttr.stOverlay.stSize.u32Height = 60;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00000000;
	stRegion.unAttr.stOverlay.u32CanvasNum = 1;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
	s32Ret = CVI_RGN_Create(Handle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT1;
	}
	memset(&stChnAttr, 0, sizeof(stChnAttr));
	stChnAttr.bShow = CVI_TRUE;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = CVI_FALSE;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 1848;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 20;
	stChnAttr.unChnAttr.stOverlayChn.u32Layer = Handle;
	s32Ret = CVI_RGN_AttachToChn(Handle, &param.stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT2;
	}
	fp = fopen(dog_argb8888_bin, "rb");
	if (fp == NULL) {
		RGN_UT_PRT("fopen failed!\n");
		goto EXIT3;
	}
	fseek(fp, 0L, SEEK_END);
	u32FileSize = ftell(fp);
	rewind(fp);
	stBitmap.pData = malloc(u32FileSize);
	if (stBitmap.pData == NULL) {
		RGN_UT_PRT("malloc size(%d) failed!\n", u32FileSize);
		fclose(fp);
		goto EXIT3;
	}
	fread(stBitmap.pData, u32FileSize, 1, fp);
	fclose(fp);
	stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
	stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
	stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;

	s32Ret = CVI_RGN_SetBitMap(Handle, &stBitmap);
	if (s32Ret != CVI_SUCCESS) {
		RGN_UT_PRT("CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
		free(stBitmap.pData);
		goto EXIT3;
	}
	free(stBitmap.pData);

	char fileName[256];
	char fileNameOut[256];

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut)-1,
		"1920_1080_nv21_out_case%d_sc_v2_capability_test.yuv",
		RGN_VPSS_SC_V2_CAPABILITY_TEST);
	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		RGN_UT_PRT("rgn_ut_vpss_send_frame failed.\n");
		goto EXIT3;
	}

EXIT3:
	s32Ret = CVI_RGN_DetachFromChn(Handle, &param.stChn);
	if (s32Ret != CVI_SUCCESS)
		RGN_UT_PRT("CVI_RGN_DetachFromChn Handle(0) failed!\n");
EXIT2:
	s32Ret = CVI_RGN_Destroy(Handle);
	if (s32Ret != CVI_SUCCESS)
		RGN_UT_PRT("CVI_RGN_Destroy Handle(0) failed!\n");
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 _rgn_ut_handle_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	switch (op) {
	case RGN_IOCTL:
		//s32Ret = rgn_ioctl_test(rgn_fd);
		break;

	case RGN_WITH_VPSS_SEND_FRAME_TEST_ORIG:
		s32Ret = rgn_send_file_test();
		break;

	case RGN_BIT_MAP_WITH_VPSS_SEND_FRAME_TEST:
		s32Ret = rgn_set_bitmap_with_vpss_sendframe_test();
		break;

	case RGN_CANVAS_WITH_VPSS_SEND_FRAME_TEST:
		s32Ret = rgn_update_canvas_with_vpss_sendframe_test();
		break;

	case RGN_8BIT_MODE_CANVAS_WITH_VPSS_SEND_FRAME_TEST:
		s32Ret = rgn_8bit_mode_canvas_with_vpss_sendframe_test();
		break;

	case RGN_COMPRESSED_SIMPLEOBJS_ARGB8888_TEST:
		s32Ret = rgn_compress_test(RGN_UT_ARGB8888);
		break;

	case RGN_COMPRESSED_SIMPLEOBJS_ARGB1555_TEST:
		s32Ret = rgn_compress_test(RGN_UT_ARGB1555);
		break;

	case RGN_COMPRESSED_SIMPLEOBJS_ARGB4444_TEST:
		s32Ret = rgn_compress_test(RGN_UT_ARGB4444);
		break;

	case RGN_COMPRESSED_SIMPLEOBJS_BITMAP_TEST:
		s32Ret = rgn_compress_simpleobjs_bitmap_test();
		break;

	case RGN_COMPRESSED_NORMAL_MIXED_TEST:
		s32Ret = rgn_compressed_normal_mixed_test();
		break;

	case RGN_VPSS_COVEREX_TEST:
		s32Ret = rgn_vpss_coverex_test();
		break;

	case RGN_VO_COVER_TEST:
		s32Ret = rgn_vo_cover_test();
		break;

	case RGN_VPSS_MOSAIC_TEST:
		s32Ret = rgn_vpss_mosaic_test();
		break;

	case RGN_VO_OSD:
		s32Ret = rgn_vo_osd_test();
		break;

	case RGN_VPSS_FORMATS_TEST:
		s32Ret = rgn_vpss_formats_test();
		break;

	case RGN_VPSS_SC_V1_CAPABILITY_TEST:
		s32Ret = rgn_vpss_sc_v1_capability_test();
		break;

	case RGN_VPSS_SC_V2_CAPABILITY_TEST:
		s32Ret = rgn_vpss_sc_v2_capability_test();
		break;
#ifdef _SW_VERIFICATION_
	case RGN_VPSS_SCALING_TEST:
		s32Ret = rgn_vpss_scaling_test();
		break;

	case RGN_VPSS_COLORKEY_TEST:
		s32Ret = rgn_vpss_colorkey_test();
		break;

	case RGN_VPSS_FONTBOX_TEST:
		s32Ret = rgn_vpss_fontbox_test();
		break;

	case RGN_VPSS_CROP_TEST:
		s32Ret = rgn_vpss_crop_test();
		break;

	case RGN_CHECKSUM_TEST:
		s32Ret = rgn_checksum_test();
		break;
#endif
	case RGN_CREATE_DESTROY:
		s32Ret = rgn_create_destroy_test();
		break;

	case RGN_CREATE_ATTACTH_DETACH_DESTROY:
		s32Ret = rgn_create_attach_detach_destroy_test();
		break;

	case RGN_ATTR_TEST:
		s32Ret = rgn_attr_test();
		break;

	case RGN_DISPLAY_ATTR_TEST:
		s32Ret = rgn_display_attr_test();
		break;

	case 255:
		is_enable = 0;
		break;

	default:
		break;
	}

	if (s32Ret == CVI_SUCCESS)
		printf(GREEN"\n=== case(%d) pass ===\n"NONE"\n", op);
	else
		printf(RED"\n=== case(%d) fail===\n"NONE"\n", op);

	return s32Ret;
}


int main(int argc, char *argv[])
{
	int op;
	CVI_S32 s32Ret;
	UNUSED(argc);
	UNUSED(argv);

	if (argc >= 2) {
		op = (CVI_S32)atoi(argv[1]);
		s32Ret = _rgn_ut_handle_op(op);
		RGN_UT_PRT("rgn ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	} else {
		do {
			RGN_UT_PRT("========================== RGN testcase ==========================\n");
			//RGN_UT_PRT("0 : ioctl\n");
			RGN_UT_PRT("%03d: send frame test from vpss_ut.c\n", RGN_WITH_VPSS_SEND_FRAME_TEST_ORIG);
			RGN_UT_PRT("%03d: RGN set bit map.\n", RGN_BIT_MAP_WITH_VPSS_SEND_FRAME_TEST);
			RGN_UT_PRT("%03d: RGN update canvas.\n", RGN_CANVAS_WITH_VPSS_SEND_FRAME_TEST);
			RGN_UT_PRT("%03d: RGN update 8bit mode canvas.\n",
				RGN_8BIT_MODE_CANVAS_WITH_VPSS_SEND_FRAME_TEST);

			RGN_UT_PRT("%03d: RGN compress simple objects argb8888 test.\n",
				RGN_COMPRESSED_SIMPLEOBJS_ARGB8888_TEST);
			RGN_UT_PRT("%03d: RGN compress simple objects argb1555 test.\n",
				RGN_COMPRESSED_SIMPLEOBJS_ARGB1555_TEST);
			RGN_UT_PRT("%03d: RGN compress simple objects argb4444 test.\n",
				RGN_COMPRESSED_SIMPLEOBJS_ARGB4444_TEST);
			RGN_UT_PRT("%03d: RGN compress simple objects and bitmap test.\n",
				RGN_COMPRESSED_SIMPLEOBJS_BITMAP_TEST);
			RGN_UT_PRT("%03d: Compressed and normal rgn mixed test.\n",
				RGN_COMPRESSED_NORMAL_MIXED_TEST);

			RGN_UT_PRT("%03d: vpss coverex test.\n", RGN_VPSS_COVEREX_TEST);
			RGN_UT_PRT("%03d: vpss mosaic test.\n", RGN_VPSS_MOSAIC_TEST);

			RGN_UT_PRT("%03d: vo RGN test.\n", RGN_VO_OSD);
			RGN_UT_PRT("%03d: vo cover test.\n", RGN_VO_COVER_TEST);
			RGN_UT_PRT("========================== CVI API testcase ==========================\n");
			RGN_UT_PRT("%03d: create->destroy.\n", RGN_CREATE_DESTROY);
			RGN_UT_PRT("%03d: create->attach->detach->destroy.\n", RGN_CREATE_ATTACTH_DETACH_DESTROY);
			RGN_UT_PRT("%03d: create->get_attr->set_attr->get_attr.\n", RGN_ATTR_TEST);
			RGN_UT_PRT("%03d: create->attach->get_disp_attr->set_disp_attr->get_disp_attr.\n",
				RGN_DISPLAY_ATTR_TEST);
			RGN_UT_PRT("==================== RGN sw verification testcase ====================\n");
			RGN_UT_PRT("%03d: RGN on VPSS formats test.\n", RGN_VPSS_FORMATS_TEST);
			RGN_UT_PRT("%03d: RGN on VPSS sc_v1 capability test.\n", RGN_VPSS_SC_V1_CAPABILITY_TEST);
			RGN_UT_PRT("%03d: RGN on VPSS sc_v2 capability test.\n", RGN_VPSS_SC_V2_CAPABILITY_TEST);
#ifdef _SW_VERIFICATION_
			RGN_UT_PRT("%03d: RGN on VPSS scaling test.\n", RGN_VPSS_SCALING_TEST);
			RGN_UT_PRT("%03d: RGN on VPSS colorkey test.\n", RGN_VPSS_COLORKEY_TEST);
			RGN_UT_PRT("%03d: RGN on VPSS fontbox test.\n", RGN_VPSS_FONTBOX_TEST);
			RGN_UT_PRT("%03d: RGN on VPSS crop test.\n", RGN_VPSS_CROP_TEST);
			RGN_UT_PRT("%03d: RGN on VPSS checksum test.\n", RGN_CHECKSUM_TEST);
#endif
			RGN_UT_PRT("255: exit\n");
			scanf("%d", &op);
			s32Ret = _rgn_ut_handle_op(op);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "op(%d) failed with %#x!\n", op, s32Ret);
				break;
			}
		} while (op != 255);
	}

	CVI_SYS_Exit();
	CVI_VB_Exit();

	return 0;
}

