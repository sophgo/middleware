#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <cvi_type.h>
#include <cvi_comm_video.h>
#include "cvi_debug.h"
#include "cvi_comm_sns.h"
#include "cvi_sns_ctrl.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_isp.h"


#include "ov48b2q_cmos_ex.h"
#include "ov48b2q_cmos_param.h"

#define DIV_0_TO_1(a)   ((0 == (a)) ? 1 : (a))
#define DIV_0_TO_1_FLOAT(a) ((((a) < 1E-10) && ((a) > -1E-10)) ? 1 : (a))
#define OV48B2Q_ID 0x564842

#define OV48B2Q_I2C_ADDR_IS_VALID(addr)      ((addr) == OV48B2Q_I2C_ADDR_1 || (addr) == OV48B2Q_I2C_ADDR_2)
/****************************************************************************
 * global variables                                                            *
 ****************************************************************************/

ISP_SNS_STATE_S *g_pastOv48b2q[VI_MAX_PIPE_NUM] = {CVI_NULL};
SNS_COMBO_DEV_ATTR_S* g_pastOv48b2qComboDevArray[VI_MAX_PIPE_NUM] = {CVI_NULL};

#define OV48B2Q_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastOv48b2q[dev])
#define OV48B2Q_SENSOR_SET_CTX(dev, pstCtx)   (g_pastOv48b2q[dev] = pstCtx)
#define OV48B2Q_SENSOR_RESET_CTX(dev)         (g_pastOv48b2q[dev] = CVI_NULL)
#define OV48B2Q_SENSOR_SET_COMBO(dev, pstCtx)   (g_pastOv48b2qComboDevArray[dev] = pstCtx)
#define OV48B2Q_SENSOR_GET_COMBO(dev, pstCtx)   (pstCtx = g_pastOv48b2qComboDevArray[dev])

#define OV48B2Q_SNNSOR_IS_1L() (ov48b2q_rx_attr.mipi_attr.lane_id[2] == -1 && ov48b2q_rx_attr.mipi_attr.lane_id[4] == -1)
#define OV48B2Q_SNNSOR_IS_2L() (ov48b2q_rx_attr.mipi_attr.lane_id[3] == -1 && ov48b2q_rx_attr.mipi_attr.lane_id[4] == -1)
#define OV48B2Q_SNNSOR_IS_4L() (ov48b2q_rx_attr.mipi_attr.lane_id[3] != -1 && ov48b2q_rx_attr.mipi_attr.lane_id[4] != -1)

ISP_SNS_COMMBUS_U g_aunOv48b2q_BusInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cDev = 0},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cDev = -1}
};

ISP_SNS_COMMADDR_U g_aunOv48b2q_AddrInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cAddr = 0},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cAddr = -1}
};

CVI_U16 g_au16Ov48b2q_GainMode[VI_MAX_PIPE_NUM] = {0};
ISP_SNS_MIRRORFLIP_TYPE_E g_aeOv48b2q_MirrorFip[VI_MAX_PIPE_NUM] = {0};

/****************************************************************************
 * local variables and functions                                                           *
 ****************************************************************************/
static ISP_FSWDR_MODE_E genFSWDRMode[VI_MAX_PIPE_NUM] = {
	[0 ... VI_MAX_PIPE_NUM - 1] = ISP_FSWDR_NORMAL_MODE
};

static CVI_U32 gu32MaxTimeGetCnt[VI_MAX_PIPE_NUM] = {0};
static CVI_U32 g_au32InitExposure[VI_MAX_PIPE_NUM]  = {0};
static CVI_U32 g_au32LinesPer500ms[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16InitWBGain[VI_MAX_PIPE_NUM][3] = {{0} };
static CVI_U16 g_au16SampleRgain[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16SampleBgain[VI_MAX_PIPE_NUM] = {0};
static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg);
/*****Ov48b2q Lines Range*****/
#define OV48B2Q_FULL_LINES_MAX  (0xFFFFF)
#define OV48B2Q_VMAX_12M10_LINEAR_PD	0xe34

/*****Ov48b2q Register Address*****/
#define OV48B2Q_HOLD_ADDR		0x3001

#define OV48B2Q_SHR0_H_ADDR		0x3501 // bit[15:8] exposure time
#define OV48B2Q_SHR0_L_ADDR		0x3502 // bit[7:0]

#define OV48B2Q_SHR1_L_ADDR     0x305C
#define OV48B2Q_SHR1_H_ADDR     0x305E

#define OV48B2Q_AGAIN_COARSE	0x3508 // bit[6:0]
#define OV48B2Q_AGAIN_FINE		0x3509 // bit[7:1]
#define OV48B2Q_DGAIN_COARSE    0x350A // bit[3:0]
#define OV48B2Q_DGAIN_FINE_H	0x350B // bit[9:2]
#define OV48B2Q_DGAIN_FINE_L	0x350C // bit[1:0]

#define OV48B2Q_VMAX_H_ADDR		0x380e //vmax [15:8]
#define OV48B2Q_VMAX_L_ADDR		0x380f //[7:0]

/*****Ov48b2q Size*****/
#define SENSOR_OV48B2Q_8M_WIDTH		3840
#define SENSOR_OV48B2Q_8M_HEIGHT	2160
#define OV48B2Q_RES_IS_8M(w, h)      ((w) == SENSOR_OV48B2Q_8M_WIDTH && (h) == SENSOR_OV48B2Q_8M_HEIGHT)
#define SENSOR_OV48B2Q_12M_WIDTH	4000
#define SENSOR_OV48B2Q_12M_HEIGHT	3000
#define OV48B2Q_RES_IS_12M(w, h)      ((w) == SENSOR_OV48B2Q_12M_WIDTH && (h) == SENSOR_OV48B2Q_12M_HEIGHT)


static CVI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	OV48B2Q_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FlickerFreq = 50 * 256;
	pstAeSnsDft->u32FullLinesMax = OV48B2Q_FULL_LINES_MAX;
	pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * 30);

	pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
	pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
	pstAeSnsDft->stIntTimeAccu.f32Offset = 0;

	pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
	pstAeSnsDft->stAgainAccu.f32Accuracy = 1;

	pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
	pstAeSnsDft->stDgainAccu.f32Accuracy = 1;

	pstAeSnsDft->u32ISPDgainShift = 8;
	pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
	pstAeSnsDft->u32MaxISPDgainTarget = 2 << pstAeSnsDft->u32ISPDgainShift;

	if (g_au32LinesPer500ms[ViPipe] == 0)
		pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * g_astOv48b2q_mode[pstSnsState->u8ImgMode].f32MaxFps / 2;
	else
		pstAeSnsDft->u32LinesPer500ms = g_au32LinesPer500ms[ViPipe];
	pstAeSnsDft->u32SnsStableFrame = 8;

	switch (pstSnsState->enWDRMode) {
	default:
	case WDR_MODE_NONE:   /*linear mode*/
		pstAeSnsDft->f32Fps = g_astOv48b2q_mode[pstSnsState->u8ImgMode].f32MaxFps;
		pstAeSnsDft->f32MinFps = g_astOv48b2q_mode[pstSnsState->u8ImgMode].f32MinFps;
		pstAeSnsDft->au8HistThresh[0] = 0xd;
		pstAeSnsDft->au8HistThresh[1] = 0x28;
		pstAeSnsDft->au8HistThresh[2] = 0x60;
		pstAeSnsDft->au8HistThresh[3] = 0x80;

		pstAeSnsDft->u32MaxAgain = g_astOv48b2q_mode[pstSnsState->u8ImgMode].stAgain[0].u32Max;
		pstAeSnsDft->u32MinAgain = g_astOv48b2q_mode[pstSnsState->u8ImgMode].stAgain[0].u32Min;
		pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
		pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

		pstAeSnsDft->u32MaxDgain = g_astOv48b2q_mode[pstSnsState->u8ImgMode].stDgain[0].u32Max;
		pstAeSnsDft->u32MinDgain = g_astOv48b2q_mode[pstSnsState->u8ImgMode].stDgain[0].u32Min;
		pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
		pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

		pstAeSnsDft->u8AeCompensation = 40;
		pstAeSnsDft->u32InitAESpeed = 64;
		pstAeSnsDft->u32InitAETolerance = 5;
		pstAeSnsDft->u32AEResponseFrame = 4;
		pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
		pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe] ? g_au32InitExposure[ViPipe] : 76151;

		//shutter time [9 to (number of lines perframe -1)]
		pstAeSnsDft->u32MaxIntTime = g_astOv48b2q_mode[pstSnsState->u8ImgMode].stExp[0].u16Max;
		pstAeSnsDft->u32MinIntTime = g_astOv48b2q_mode[pstSnsState->u8ImgMode].stExp[0].u16Min;
		pstAeSnsDft->u32MaxIntTimeTarget = 65535;
		pstAeSnsDft->u32MinIntTimeTarget = 1;
		break;

	case WDR_MODE_2To1_LINE:
		pstAeSnsDft->f32Fps = g_astOv48b2q_mode[pstSnsState->u8ImgMode].f32MaxFps;
		pstAeSnsDft->f32MinFps = g_astOv48b2q_mode[pstSnsState->u8ImgMode].f32MinFps;
		pstAeSnsDft->au8HistThresh[0] = 0xC;
		pstAeSnsDft->au8HistThresh[1] = 0x18;
		pstAeSnsDft->au8HistThresh[2] = 0x60;
		pstAeSnsDft->au8HistThresh[3] = 0x80;

		pstAeSnsDft->u32MaxIntTime = g_astOv48b2q_mode[pstSnsState->u8ImgMode].stExp[1].u16Max;
		pstAeSnsDft->u32MinIntTime = g_astOv48b2q_mode[pstSnsState->u8ImgMode].stExp[0].u16Min;

		pstAeSnsDft->u32MaxIntTimeTarget = 65535;
		pstAeSnsDft->u32MinIntTimeTarget = pstAeSnsDft->u32MinIntTime;

		pstAeSnsDft->u32MaxAgain = g_astOv48b2q_mode[pstSnsState->u8ImgMode].stAgain[0].u32Max;
		pstAeSnsDft->u32MinAgain = g_astOv48b2q_mode[pstSnsState->u8ImgMode].stAgain[0].u32Min;
		pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
		pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

		pstAeSnsDft->u32MaxDgain = g_astOv48b2q_mode[pstSnsState->u8ImgMode].stDgain[0].u32Max;
		pstAeSnsDft->u32MinDgain = g_astOv48b2q_mode[pstSnsState->u8ImgMode].stDgain[0].u32Min;
		pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
		pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;
		pstAeSnsDft->u32MaxISPDgainTarget = 16 << pstAeSnsDft->u32ISPDgainShift;

		pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe] ? g_au32InitExposure[ViPipe] : 52000;
		pstAeSnsDft->u32InitAESpeed = 64;
		pstAeSnsDft->u32InitAETolerance = 5;
		pstAeSnsDft->u32AEResponseFrame = 4;
		if (genFSWDRMode[ViPipe] == ISP_FSWDR_LONG_FRAME_MODE) {
			pstAeSnsDft->u8AeCompensation = 64;
			pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
		} else {
			pstAeSnsDft->u8AeCompensation = 40;
			pstAeSnsDft->enAeExpMode = AE_EXP_LOWLIGHT_PRIOR;
		}
		break;
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_fps_set(VI_PIPE ViPipe, CVI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	CVI_U32 u32VMAX = OV48B2Q_VMAX_12M10_LINEAR_PD;
	CVI_FLOAT f32MaxFps = 0;
	CVI_FLOAT f32MinFps = 0;
	CVI_U32 u32Vts = 0;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	OV48B2Q_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u32Vts = g_astOv48b2q_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	f32MaxFps = g_astOv48b2q_mode[pstSnsState->u8ImgMode].f32MaxFps;
	f32MinFps = g_astOv48b2q_mode[pstSnsState->u8ImgMode].f32MinFps;

	switch (pstSnsState->u8ImgMode) {
	case OV48B2Q_MODE_8M30_WDR:
		if ((f32Fps <= f32MaxFps) && (f32Fps >= f32MinFps)) {
			u32VMAX = u32Vts * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport Fps: %f\n", f32Fps);
			return CVI_FAILURE;
		}
		/* FSC shall be multiple of 8 */
		if (u32VMAX % 4) {
			u32VMAX = u32VMAX - (u32VMAX % 4) + 4;
		}
		u32VMAX = (u32VMAX > OV48B2Q_FULL_LINES_MAX) ? OV48B2Q_FULL_LINES_MAX : u32VMAX;
		break;

	case OV48B2Q_MODE_12M10_PD:
	case OV48B2Q_MODE_12M10:
	case OV48B2Q_MODE_8M30:
		if ((f32Fps <= f32MaxFps) && (f32Fps >= f32MinFps)) {
			u32VMAX = u32Vts * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport Fps: %f\n", f32Fps);
			return CVI_FAILURE;
		}
		u32VMAX = (u32VMAX > OV48B2Q_FULL_LINES_MAX) ? OV48B2Q_FULL_LINES_MAX : u32VMAX;
		break;
	default:
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport sensor mode: %d\n", pstSnsState->u8ImgMode);
		return CVI_FAILURE;
	}

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstSnsRegsInfo->astI2cData[LINEAR_VMAX_L].u32Data = (u32VMAX & 0xFF);
		pstSnsRegsInfo->astI2cData[LINEAR_VMAX_H].u32Data = ((u32VMAX & 0xFF00) >> 8);
	} else {
		if (OV48B2Q_LINEAR_PD_USE_WDR) {
			pstSnsRegsInfo->astI2cData[LINEAR_PD_VMAX_L].u32Data = (u32VMAX & 0xFF);
			pstSnsRegsInfo->astI2cData[LINEAR_PD_VMAX_H].u32Data = ((u32VMAX & 0xFF00) >> 8);
		} else {
			pstSnsRegsInfo->astI2cData[WDR_VMAX_L].u32Data = (u32VMAX & 0xFF);
			pstSnsRegsInfo->astI2cData[WDR_VMAX_H].u32Data = ((u32VMAX & 0xFF00) >> 8);
		}
	}

	if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) {
		if (OV48B2Q_LINEAR_PD_USE_WDR)
			pstSnsState->u32FLStd = u32VMAX;
		else
			pstSnsState->u32FLStd = u32VMAX * 2;
	} else {
		pstSnsState->u32FLStd = u32VMAX;
	}

	pstAeSnsDft->f32Fps = f32Fps;
	pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * f32Fps / 2;
	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 2;
	pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FullLines = pstSnsState->au32FL[0];
	pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * DIV_0_TO_1_FLOAT(f32Fps));

	return CVI_SUCCESS;
}

static CVI_S32 cmos_inttime_update(VI_PIPE ViPipe, CVI_U32 *u32IntTime)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	CVI_U32 u32Value = 0;

	OV48B2Q_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(u32IntTime);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) {

		if (OV48B2Q_LINEAR_PD_USE_WDR) {
			CVI_U32 u32LongIntTime = u32IntTime[0];
			if (u32LongIntTime >= 0x0e34 - 22)
				u32LongIntTime = 0x0e34 - 22;
			else if (u32LongIntTime < 8)
				u32LongIntTime = 8;
			else {
			}
			pstSnsRegsInfo->astI2cData[LINEAR_PD_SHR_L].u32Data = (u32LongIntTime & 0xFF);
			pstSnsRegsInfo->astI2cData[LINEAR_PD_SHR_H].u32Data = ((u32LongIntTime & 0xFF00) >> 8);
		} else {
			CVI_U32 u32ShortIntTime = u32IntTime[0];
			CVI_U32 u32LongIntTime = u32IntTime[1];

			// /* short exposure */
			// pstSnsState->au32WDRIntTime[0] = u32ShortIntTime;
			// /* long exposure */
			// pstSnsState->au32WDRIntTime[1] = u32LongIntTime;
			// /* Return the actual exposure lines*/
			// u32IntTime[0] = pstSnsState->au32WDRIntTime[0];
			// u32IntTime[1] = pstSnsState->au32WDRIntTime[1];

			pstSnsRegsInfo->astI2cData[WDR_SHR0_L].u32Data = (u32LongIntTime & 0xFF);
			pstSnsRegsInfo->astI2cData[WDR_SHR0_H].u32Data = ((u32LongIntTime & 0xFF00) >> 8);
			pstSnsRegsInfo->astI2cData[WDR_SHR1_L].u32Data = (u32ShortIntTime & 0xFF);
			pstSnsRegsInfo->astI2cData[WDR_SHR1_H].u32Data = ((u32ShortIntTime & 0xFF00) >> 8);
		}
	} else {
		u32Value = *u32IntTime = pstSnsState->au32FL[0] - *u32IntTime;

		pstSnsRegsInfo->astI2cData[LINEAR_SHR_L].u32Data = (u32Value & 0xFF);
		pstSnsRegsInfo->astI2cData[LINEAR_SHR_H].u32Data = ((u32Value & 0xFF00) >> 8);
	}

	return CVI_SUCCESS;

}

static CVI_U32 gain_table[] = {
	1024, 1088, 1152, 1216, 1280, 1344, 1408, 1472, 1536, 1600,
	1664, 1728, 1792, 1856, 1920, 1984, 2048, 2176, 2304, 2432,
	2560, 2688, 2816, 2944, 3072, 3200, 3328, 3456, 3584, 3712,
	3840, 3968, 4096, 4352, 4608, 4864, 5120, 5376, 5632, 5888,
	6144, 6400, 6656, 6912, 7168, 7424, 7680, 7936, 8192, 8704,
	9216, 9728, 10240, 10752, 11264, 11776, 12288, 12800, 13312, 13824,
	14336, 14848, 15360, 15872, 0xffffffff,
};

static CVI_U32 gain_list[][5] = {
	/* Again | Dgain*/
	/* 3508 3509 | 350a 350b 350c*/
	{0x01, 0x00, 0x01, 0x00, 0x00}, // 1x
	{0x01, 0x10, 0x01, 0x10, 0x00},
	{0x01, 0x20, 0x01, 0x20, 0x00},
	{0x01, 0x30, 0x01, 0x30, 0x00},
	{0x01, 0x40, 0x01, 0x40, 0x00},
	{0x01, 0x50, 0x01, 0x50, 0x00},
	{0x01, 0x60, 0x01, 0x60, 0x00},
	{0x01, 0x70, 0x01, 0x70, 0x00},
	{0x01, 0x80, 0x01, 0x80, 0x00},
	{0x01, 0x90, 0x01, 0x90, 0x00},
	{0x01, 0xA0, 0x01, 0xA0, 0x00},
	{0x01, 0xB0, 0x01, 0xB0, 0x00},
	{0x01, 0xC0, 0x01, 0xC0, 0x00},
	{0x01, 0xD0, 0x01, 0xD0, 0x00},
	{0x01, 0xE0, 0x01, 0xE0, 0x00},
	{0x01, 0xF0, 0x01, 0xF0, 0x00},
	{0x02, 0x00, 0x02, 0x00, 0x00},
	{0x02, 0x20, 0x02, 0x20, 0x00},
	{0x02, 0x40, 0x02, 0x40, 0x00},
	{0x02, 0x60, 0x02, 0x60, 0x00},
	{0x02, 0x80, 0x02, 0x80, 0x00},
	{0x02, 0xA0, 0x02, 0xA0, 0x00},
	{0x02, 0xC0, 0x02, 0xC0, 0x00},
	{0x02, 0xE0, 0x02, 0xE0, 0x00},
	{0x03, 0x00, 0x03, 0x00, 0x00},
	{0x03, 0x20, 0x03, 0x20, 0x00},
	{0x03, 0x40, 0x03, 0x40, 0x00},
	{0x03, 0x60, 0x03, 0x60, 0x00},
	{0x03, 0x80, 0x03, 0x80, 0x00},
	{0x03, 0xA0, 0x03, 0xA0, 0x00},
	{0x03, 0xC0, 0x03, 0xC0, 0x00},
	{0x03, 0xE0, 0x03, 0xE0, 0x00},
	{0x04, 0x00, 0x04, 0x00, 0x00},
	{0x04, 0x40, 0x04, 0x40, 0x00},
	{0x04, 0x80, 0x04, 0x80, 0x00},
	{0x04, 0xC0, 0x04, 0xC0, 0x00},
	{0x05, 0x00, 0x05, 0x00, 0x00},
	{0x05, 0x40, 0x05, 0x40, 0x00},
	{0x05, 0x80, 0x05, 0x80, 0x00},
	{0x05, 0xC0, 0x05, 0xC0, 0x00},
	{0x06, 0x00, 0x06, 0x00, 0x00},
	{0x06, 0x40, 0x06, 0x40, 0x00},
	{0x06, 0x80, 0x06, 0x80, 0x00},
	{0x06, 0xC0, 0x06, 0xC0, 0x00},
	{0x07, 0x00, 0x07, 0x00, 0x00},
	{0x07, 0x40, 0x07, 0x40, 0x00},
	{0x07, 0x80, 0x07, 0x80, 0x00},
	{0x07, 0xC0, 0x07, 0xC0, 0x00},
	{0x08, 0x00, 0x08, 0x00, 0x00},
	{0x08, 0x80, 0x08, 0x80, 0x00},
	{0x09, 0x00, 0x09, 0x00, 0x00},
	{0x09, 0x80, 0x09, 0x80, 0x00},
	{0x0A, 0x00, 0x0A, 0x00, 0x00},
	{0x0A, 0x80, 0x0A, 0x80, 0x00},
	{0x0B, 0x00, 0x0B, 0x00, 0x00},
	{0x0B, 0x80, 0x0B, 0x80, 0x00},
	{0x0C, 0x00, 0x0C, 0x00, 0x00},
	{0x0C, 0x80, 0x0C, 0x80, 0x00},
	{0x0D, 0x00, 0x0D, 0x00, 0x00},
	{0x0D, 0x80, 0x0D, 0x80, 0x00},
	{0x0E, 0x00, 0x0E, 0x00, 0x00},
	{0x0E, 0x80, 0x0E, 0x80, 0x00},
	{0x0F, 0x00, 0x0F, 0x00, 0x00},
	{0x0F, 0x80, 0x0F, 0x80, 0x00}, // 15.5x
};


static CVI_S32 cmos_again_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32AgainLin, CVI_U32 *pu32AgainDb)
{
	int i, total;

	(void) ViPipe;
	CMOS_CHECK_POINTER(pu32AgainLin);
	CMOS_CHECK_POINTER(pu32AgainDb);
	total = sizeof(gain_table) / sizeof(CVI_U32);

	if (*pu32AgainLin >= gain_table[total - 1]) {
		*pu32AgainLin = *pu32AgainDb = gain_table[total - 1];
		return CVI_SUCCESS;
	}

	for (i = 1; i < total; i++) {
		if (*pu32AgainLin < gain_table[i]) {
			*pu32AgainLin = gain_table[i - 1];
			*pu32AgainDb = i - 1;
			break;
		}
	}
	return CVI_SUCCESS;
}

static CVI_S32 cmos_dgain_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32DgainLin, CVI_U32 *pu32DgainDb)
{
	int i, total;

	(void) ViPipe;
	CMOS_CHECK_POINTER(pu32DgainLin);
	CMOS_CHECK_POINTER(pu32DgainDb);
	total = sizeof(gain_table) / sizeof(CVI_U32);

	if (*pu32DgainLin >= gain_table[total - 1]) {
		*pu32DgainLin = *pu32DgainDb = gain_table[total - 1];
		return CVI_SUCCESS;
	}

	for (i = 1; i < total; i++) {
		if (*pu32DgainLin < gain_table[i]) {
			*pu32DgainLin = gain_table[i - 1];
			*pu32DgainDb = i - 1;
			break;
		}
	}
	return CVI_SUCCESS;
}

static CVI_S32 cmos_gains_update(VI_PIPE ViPipe, CVI_U32 *pu32Again, CVI_U32 *pu32Dgain)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	CVI_U32 u16Mode = g_au16Ov48b2q_GainMode[ViPipe];
	CVI_U32 u32Tmp;
	CVI_U32 u32Again;
	CVI_U32 u32Dgain;

	OV48B2Q_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pu32Again);
	CMOS_CHECK_POINTER(pu32Dgain);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		/* linear mode */
		u32Again = pu32Again[0];
		u32Dgain = pu32Dgain[0];

		u32Tmp = u32Again + u32Dgain;

		pstSnsRegsInfo->astI2cData[LINEAR_GAIN_L].u32Data = (u32Tmp & 0xFF);
		pstSnsRegsInfo->astI2cData[LINEAR_GAIN_H].u32Data = ((u32Tmp & 0xFF00) >> 8);

	} else {
		if (OV48B2Q_LINEAR_PD_USE_WDR) {
			/* linear pd mode */
			u32Again = pu32Again[0];
			u32Dgain = pu32Dgain[0];

			pstSnsRegsInfo->astI2cData[LINEAR_PD_AGAIN_COARSE].u32Data 	= gain_list[u32Again][0];
			pstSnsRegsInfo->astI2cData[LINEAR_PD_AGAIN_FINE].u32Data 	= gain_list[u32Again][1];
			pstSnsRegsInfo->astI2cData[LINEAR_PD_DGAIN_COARSE].u32Data 	= gain_list[u32Dgain][2];
			pstSnsRegsInfo->astI2cData[LINEAR_PD_DGAIN_FINE_H].u32Data 	= gain_list[u32Dgain][3];
			pstSnsRegsInfo->astI2cData[LINEAR_PD_DGAIN_FINE_L].u32Data 	= gain_list[u32Dgain][4];
		} else {
			/* DOL mode */
			if (u16Mode == SNS_GAIN_MODE_WDR_2F) {
				/* don't support gain conversion in this mode. */
				u32Again = pu32Again[1];
				u32Dgain = pu32Dgain[1];

				u32Tmp = u32Again + u32Dgain;
				if (u32Tmp > 0x7FF) {
					u32Tmp = 0x7FF;
				}

				pstSnsRegsInfo->astI2cData[WDR_GAIN_L].u32Data = (u32Tmp & 0xFF);
				pstSnsRegsInfo->astI2cData[WDR_GAIN_H].u32Data = ((u32Tmp & 0xFF00) >> 8);

				u32Again = pu32Again[0];
				u32Dgain = pu32Dgain[0];

				u32Tmp = u32Again + u32Dgain;
				if (u32Tmp > 0x7FF) {
					u32Tmp = 0x7FF;
				}
				pstSnsRegsInfo->astI2cData[WDR_GAIN_SHORT_L].u32Data = (u32Tmp & 0xFF);
				pstSnsRegsInfo->astI2cData[WDR_GAIN_SHORT_L].u32Data = ((u32Tmp & 0xFF00) >> 8);
			} else if (u16Mode == SNS_GAIN_MODE_SHARE) {
				u32Again = pu32Again[0];
				u32Dgain = pu32Dgain[0];

				u32Tmp = u32Again + u32Dgain;
				if (u32Tmp > 0x7FF) {
					u32Tmp = 0x7FF;
				}

				pstSnsRegsInfo->astI2cData[WDR_GAIN_L].u32Data = (u32Tmp & 0xFF);
				pstSnsRegsInfo->astI2cData[WDR_GAIN_H].u32Data = ((u32Tmp & 0xFF00) >> 8);
			}
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_inttime_max(VI_PIPE ViPipe, CVI_U16 u16ManRatioEnable, CVI_U32 *au32Ratio,
		CVI_U32 *au32IntTimeMax, CVI_U32 *au32IntTimeMin, CVI_U32 *pu32LFMaxIntTime)
{
	CVI_U32 u32RatioTmp = 0x40;
	CVI_U32 u32IntTimeMaxTmp = (0x0e34 - 22) / u32RatioTmp;
	CVI_U32 u32ShortTimeMinLimit = 8;

	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(au32Ratio);
	CMOS_CHECK_POINTER(au32IntTimeMax);
	CMOS_CHECK_POINTER(au32IntTimeMin);
	CMOS_CHECK_POINTER(pu32LFMaxIntTime);
	OV48B2Q_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	if (u32IntTimeMaxTmp >= u32ShortTimeMinLimit) {
		if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
			au32IntTimeMax[0] = u32IntTimeMaxTmp;
			au32IntTimeMax[1] = au32IntTimeMax[0] * au32Ratio[0] >> 6;
			au32IntTimeMax[2] = au32IntTimeMax[1] * au32Ratio[1] >> 6;
			au32IntTimeMax[3] = au32IntTimeMax[2] * au32Ratio[2] >> 6;
			au32IntTimeMin[0] = u32ShortTimeMinLimit;
			au32IntTimeMin[1] = au32IntTimeMin[0] * au32Ratio[0] >> 6;
			au32IntTimeMin[2] = au32IntTimeMin[1] * au32Ratio[1] >> 6;
			au32IntTimeMin[3] = au32IntTimeMin[2] * au32Ratio[2] >> 6;
		} else {
		}
	} else {
		if (!OV48B2Q_LINEAR_PD_USE_WDR) {
			if (u16ManRatioEnable) {
				CVI_TRACE_SNS(CVI_DBG_ERR, "Manaul ExpRatio out of range!\n");
				return CVI_FAILURE;
			}
		}
		u32IntTimeMaxTmp = u32ShortTimeMinLimit;

		// if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
		// 	u32RatioTmp = 0xFFF;
		// 	au32IntTimeMax[0] = u32IntTimeMaxTmp;
		// 	au32IntTimeMax[1] = au32IntTimeMax[0] * u32RatioTmp >> 6;
		// } else {
		// }
		au32IntTimeMin[0] = au32IntTimeMax[0];
		au32IntTimeMin[1] = au32IntTimeMax[1];
		au32IntTimeMin[2] = au32IntTimeMax[2];
		au32IntTimeMin[3] = au32IntTimeMax[3];
	}

	return CVI_SUCCESS;
}

/* Only used in LINE_WDR mode */
static CVI_S32 cmos_ae_fswdr_attr_set(VI_PIPE ViPipe, AE_FSWDR_ATTR_S *pstAeFSWDRAttr)
{
	CMOS_CHECK_POINTER(pstAeFSWDRAttr);

	genFSWDRMode[ViPipe] = pstAeFSWDRAttr->enFSWDRMode;
	gu32MaxTimeGetCnt[ViPipe] = 0;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	CMOS_CHECK_POINTER(pstExpFuncs);

	memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

	pstExpFuncs->pfn_cmos_get_ae_default    = cmos_get_ae_default;
	pstExpFuncs->pfn_cmos_fps_set           = cmos_fps_set;
	pstExpFuncs->pfn_cmos_inttime_update    = cmos_inttime_update;
	pstExpFuncs->pfn_cmos_gains_update      = cmos_gains_update;
	pstExpFuncs->pfn_cmos_again_calc_table  = cmos_again_calc_table;
	pstExpFuncs->pfn_cmos_dgain_calc_table  = cmos_dgain_calc_table;
	pstExpFuncs->pfn_cmos_get_inttime_max   = cmos_get_inttime_max;
	pstExpFuncs->pfn_cmos_ae_fswdr_attr_set = cmos_ae_fswdr_attr_set;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_awb_default(VI_PIPE ViPipe, AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
	(void) ViPipe;
	CMOS_CHECK_POINTER(pstAwbSnsDft);
	memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

	pstAwbSnsDft->u16InitGgain = 1024;
	pstAwbSnsDft->u8AWBRunInterval = 1;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	CMOS_CHECK_POINTER(pstExpFuncs);

	memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

	pstExpFuncs->pfn_cmos_get_awb_default = cmos_get_awb_default;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_isp_default(VI_PIPE ViPipe, ISP_CMOS_DEFAULT_S *pstDef)
{
	(void) ViPipe;

	memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));

	memcpy(pstDef->stNoiseCalibration.CalibrationCoef,
		&g_stIspNoiseCalibratio, sizeof(ISP_CMOS_NOISE_CALIBRATION_S));

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_blc_default(VI_PIPE ViPipe, ISP_CMOS_BLACK_LEVEL_S *pstBlc)
{
	(void) ViPipe;

	CMOS_CHECK_POINTER(pstBlc);

	memset(pstBlc, 0, sizeof(ISP_CMOS_BLACK_LEVEL_S));

	memcpy(pstBlc,
		&g_stIspBlcCalibratio, sizeof(ISP_CMOS_BLACK_LEVEL_S));
	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg)
{
	const OV48B2Q_MODE_S *pstMode = CVI_NULL;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	OV48B2Q_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstMode = &g_astOv48b2q_mode[pstSnsState->u8ImgMode];

	if (pstSnsState->enWDRMode != WDR_MODE_NONE) {
		pstIspCfg->frm_num = 2;
		memcpy(&pstIspCfg->img_size[0], &pstMode->astImg[0], sizeof(ISP_WDR_SIZE_S));
		memcpy(&pstIspCfg->img_size[1], &pstMode->astImg[1], sizeof(ISP_WDR_SIZE_S));
	} else {
		pstIspCfg->frm_num = 1;
		memcpy(&pstIspCfg->img_size[0], &pstMode->astImg[0], sizeof(ISP_WDR_SIZE_S));
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_wdr_mode(VI_PIPE ViPipe, CVI_U8 u8Mode)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	OV48B2Q_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
syslog(LOG_INFO, "cmos_set_wdr_mode u8Mode = %d\n", u8Mode);
	pstSnsState->bSyncInit = CVI_FALSE;
	switch (u8Mode) {
	case WDR_MODE_NONE:
		if (pstSnsState->u8ImgMode == OV48B2Q_MODE_8M30_WDR)
			pstSnsState->u8ImgMode = OV48B2Q_MODE_8M30;
		else if (pstSnsState->u8ImgMode == OV48B2Q_MODE_12M10_PD)
			pstSnsState->u8ImgMode = OV48B2Q_MODE_12M10;
		else {
		}
		pstSnsState->enWDRMode = WDR_MODE_NONE;
		pstSnsState->u32FLStd = g_astOv48b2q_mode[pstSnsState->u8ImgMode].u32VtsDef;
		syslog(LOG_INFO, "WDR_MODE_NONE\n");
		break;

	case WDR_MODE_2To1_LINE:
		if (pstSnsState->u8ImgMode == OV48B2Q_MODE_8M30)
			pstSnsState->u8ImgMode = OV48B2Q_MODE_8M30_WDR;
		else if (pstSnsState->u8ImgMode == OV48B2Q_MODE_12M10)
			pstSnsState->u8ImgMode = OV48B2Q_MODE_12M10_PD;
		else {
		}
		pstSnsState->enWDRMode = WDR_MODE_2To1_LINE;
		pstSnsState->u32FLStd = g_astOv48b2q_mode[pstSnsState->u8ImgMode].u32VtsDef * 2;
		break;
	default:
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unknown mode!\n");
		return CVI_FAILURE;
	}

	pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
	pstSnsState->au32FL[1] = pstSnsState->au32FL[0];
	memset(pstSnsState->au32WDRIntTime, 0, sizeof(pstSnsState->au32WDRIntTime));

	return CVI_SUCCESS;
}

static CVI_U32 sensor_cmp_wdr_size(ISP_SNS_ISP_INFO_S *pstWdr1, ISP_SNS_ISP_INFO_S *pstWdr2)
{
	CVI_U32 i;

	if (pstWdr1->frm_num != pstWdr2->frm_num)
		goto _mismatch;
	for (i = 0; i < 2; i++) {
		if (pstWdr1->img_size[i].stSnsSize.u32Width != pstWdr2->img_size[i].stSnsSize.u32Width)
			goto _mismatch;
		if (pstWdr1->img_size[i].stSnsSize.u32Height != pstWdr2->img_size[i].stSnsSize.u32Height)
			goto _mismatch;
		if (pstWdr1->img_size[i].stWndRect.s32X != pstWdr2->img_size[i].stWndRect.s32X)
			goto _mismatch;
		if (pstWdr1->img_size[i].stWndRect.s32Y != pstWdr2->img_size[i].stWndRect.s32Y)
			goto _mismatch;
		if (pstWdr1->img_size[i].stWndRect.u32Width != pstWdr2->img_size[i].stWndRect.u32Width)
			goto _mismatch;
		if (pstWdr1->img_size[i].stWndRect.u32Height != pstWdr2->img_size[i].stWndRect.u32Height)
			goto _mismatch;
	}

	return 0;
_mismatch:
	return 1;
}

static CVI_S32 cmos_get_sns_regs_info(VI_PIPE ViPipe, ISP_SNS_SYNC_INFO_S *pstSnsSyncInfo)
{
	CVI_U32 i;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	ISP_SNS_SYNC_INFO_S *pstCfg0 = CVI_NULL;
	ISP_SNS_SYNC_INFO_S *pstCfg1 = CVI_NULL;
	ISP_I2C_DATA_S *pstI2c_data = CVI_NULL;

	CMOS_CHECK_POINTER(pstSnsSyncInfo);
	OV48B2Q_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstSnsRegsInfo = &pstSnsSyncInfo->snsCfg;
	pstCfg0 = &pstSnsState->astSyncInfo[0];
	pstCfg1 = &pstSnsState->astSyncInfo[1];
	pstI2c_data = pstCfg0->snsCfg.astI2cData;

	if ((pstSnsState->bSyncInit == CVI_FALSE) || (pstSnsRegsInfo->bConfig == CVI_FALSE)) {
		pstCfg0->snsCfg.enSnsType = SNS_I2C_TYPE;
		pstCfg0->snsCfg.unComBus.s8I2cDev = g_aunOv48b2q_BusInfo[ViPipe].s8I2cDev;
		pstCfg0->snsCfg.u8Cfg2ValidDelayMax = 2;
		pstCfg0->snsCfg.use_snsr_sram = CVI_TRUE;
		if (OV48B2Q_LINEAR_PD_USE_WDR) {
			pstCfg0->snsCfg.u32RegNum = (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) ?
						LINEAR_PD_REGS_NUM : LINEAR_REGS_NUM;
		} else {
			pstCfg0->snsCfg.u32RegNum = (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) ?
						WDR_REGS_NUM : LINEAR_REGS_NUM;
		}

		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			pstI2c_data[i].bUpdate = CVI_TRUE;
			pstI2c_data[i].u8DevAddr = g_aunOv48b2q_AddrInfo[ViPipe].s8I2cAddr;
			pstI2c_data[i].u32AddrByteNum = ov48b2q_addr_byte;
			pstI2c_data[i].u32DataByteNum = ov48b2q_data_byte;
		}

		switch (pstSnsState->enWDRMode) {
		case WDR_MODE_2To1_LINE:
			if (OV48B2Q_LINEAR_PD_USE_WDR) {
				pstI2c_data[LINEAR_PD_SHR_L].u32RegAddr = OV48B2Q_SHR0_L_ADDR;
				pstI2c_data[LINEAR_PD_SHR_H].u32RegAddr = OV48B2Q_SHR0_H_ADDR;

				pstI2c_data[LINEAR_PD_AGAIN_COARSE].u32RegAddr 	= OV48B2Q_AGAIN_COARSE;
				pstI2c_data[LINEAR_PD_AGAIN_FINE].u32RegAddr 	= OV48B2Q_AGAIN_FINE;
				pstI2c_data[LINEAR_PD_DGAIN_COARSE].u32RegAddr 	= OV48B2Q_DGAIN_COARSE;
				pstI2c_data[LINEAR_PD_DGAIN_FINE_H].u32RegAddr 	= OV48B2Q_DGAIN_FINE_H;
				pstI2c_data[LINEAR_PD_DGAIN_FINE_L].u32RegAddr 	= OV48B2Q_DGAIN_FINE_L;

				pstI2c_data[LINEAR_PD_VMAX_L].u32RegAddr = OV48B2Q_VMAX_L_ADDR;
				pstI2c_data[LINEAR_PD_VMAX_H].u32RegAddr = OV48B2Q_VMAX_H_ADDR;
			} else {
				pstI2c_data[WDR_SHR0_L].u32RegAddr = OV48B2Q_SHR0_L_ADDR;
				pstI2c_data[WDR_SHR0_H].u32RegAddr = OV48B2Q_SHR0_H_ADDR;
				pstI2c_data[WDR_SHR1_L].u32RegAddr = OV48B2Q_SHR1_L_ADDR;
				pstI2c_data[WDR_SHR1_H].u32RegAddr = OV48B2Q_SHR1_H_ADDR;

				pstI2c_data[WDR_GAIN_L].u32RegAddr = OV48B2Q_AGAIN_COARSE;
				pstI2c_data[WDR_GAIN_L].u8DelayFrmNum = 0;
				pstI2c_data[WDR_GAIN_H].u32RegAddr = OV48B2Q_AGAIN_FINE;
				pstI2c_data[WDR_GAIN_H].u8DelayFrmNum = 0;

				pstI2c_data[WDR_GAIN_SHORT_L].u32RegAddr = OV48B2Q_DGAIN_COARSE;
				pstI2c_data[WDR_GAIN_SHORT_L].u8DelayFrmNum = 0;
				pstI2c_data[WDR_GAIN_SHORT_H].u32RegAddr = OV48B2Q_DGAIN_FINE_H;
				pstI2c_data[WDR_GAIN_SHORT_H].u8DelayFrmNum = 0;

				pstI2c_data[WDR_VMAX_L].u32RegAddr = OV48B2Q_VMAX_L_ADDR;
				pstI2c_data[WDR_VMAX_L].u8DelayFrmNum = 0;
				pstI2c_data[WDR_VMAX_H].u32RegAddr = OV48B2Q_VMAX_H_ADDR;
				pstI2c_data[WDR_VMAX_H].u8DelayFrmNum = 0;
			}

			pstCfg0->ispCfg.u8DelayFrmNum = 0;
			break;
		default:
			pstI2c_data[LINEAR_SHR_L].u32RegAddr = OV48B2Q_SHR0_L_ADDR;
			pstI2c_data[LINEAR_SHR_H].u32RegAddr = OV48B2Q_SHR0_H_ADDR;

			pstI2c_data[LINEAR_GAIN_L].u32RegAddr = OV48B2Q_AGAIN_COARSE;
			pstI2c_data[LINEAR_GAIN_H].u32RegAddr = OV48B2Q_AGAIN_FINE;

			pstI2c_data[LINEAR_VMAX_L].u32RegAddr = OV48B2Q_VMAX_L_ADDR;
			pstI2c_data[LINEAR_VMAX_H].u32RegAddr = OV48B2Q_VMAX_H_ADDR;

			pstCfg0->ispCfg.u8DelayFrmNum = 0;
			break;
		}
		pstSnsState->bSyncInit = CVI_TRUE;
		pstCfg0->snsCfg.need_update = CVI_TRUE;
		/* recalcualte WDR size */
		cmos_get_wdr_size(ViPipe, &pstCfg0->ispCfg);
		pstCfg0->ispCfg.need_update = CVI_TRUE;
	} else {
		pstCfg0->snsCfg.need_update = CVI_FALSE;
		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			if (pstCfg0->snsCfg.astI2cData[i].u32Data == pstCfg1->snsCfg.astI2cData[i].u32Data) {
				pstCfg0->snsCfg.astI2cData[i].bUpdate = CVI_FALSE;
			} else {
				pstCfg0->snsCfg.astI2cData[i].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.need_update = CVI_TRUE;
			}
		}
		/* check update isp crop or not */
		pstCfg0->ispCfg.need_update = (sensor_cmp_wdr_size(&pstCfg0->ispCfg, &pstCfg1->ispCfg) ?
				CVI_TRUE : CVI_FALSE);
	}

	pstSnsRegsInfo->bConfig = CVI_FALSE;
	memcpy(pstSnsSyncInfo, &pstSnsState->astSyncInfo[0], sizeof(ISP_SNS_SYNC_INFO_S));
	memcpy(&pstSnsState->astSyncInfo[1], &pstSnsState->astSyncInfo[0], sizeof(ISP_SNS_SYNC_INFO_S));
	pstSnsState->au32FL[1] = pstSnsState->au32FL[0];

	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_image_mode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
	CVI_U8 u8SensorImageMode = 0;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstSensorImageMode);
	OV48B2Q_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u8SensorImageMode = pstSnsState->u8ImgMode;
	pstSnsState->bSyncInit = CVI_FALSE;

	if (pstSensorImageMode->f32Fps <= 30) {
		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (OV48B2Q_RES_IS_8M(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
				u8SensorImageMode = OV48B2Q_MODE_8M30;
			else if (OV48B2Q_RES_IS_12M(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
				u8SensorImageMode = OV48B2Q_MODE_12M10;
			else {
				CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d WDR_MODE_NONE\n",
				       pstSensorImageMode->u16Width,
				       pstSensorImageMode->u16Height,
				       pstSensorImageMode->f32Fps,
				       pstSnsState->enWDRMode);
				return CVI_FAILURE;
			}
		} else if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
			if (OV48B2Q_RES_IS_8M(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
				u8SensorImageMode = OV48B2Q_MODE_8M30_WDR;
			else if (OV48B2Q_RES_IS_12M(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
				u8SensorImageMode = OV48B2Q_MODE_12M10_PD;
			else {
				CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d WDR_MODE_2To1_LINE\n",
				       pstSensorImageMode->u16Width,
				       pstSensorImageMode->u16Height,
				       pstSensorImageMode->f32Fps,
				       pstSnsState->enWDRMode);
				return CVI_FAILURE;
			}
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d else\n",
			       pstSensorImageMode->u16Width,
			       pstSensorImageMode->u16Height,
			       pstSensorImageMode->f32Fps,
			       pstSnsState->enWDRMode);
			return CVI_FAILURE;
		}
	} else {
	}

	if ((pstSnsState->bInit == CVI_TRUE) && (u8SensorImageMode == pstSnsState->u8ImgMode)) {
		return CVI_FAILURE;
	}

	pstSnsState->u8ImgMode = u8SensorImageMode;

	return CVI_SUCCESS;
}

static CVI_VOID sensor_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	OV48B2Q_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);
	if (pstSnsState->bInit == CVI_TRUE && g_aeOv48b2q_MirrorFip[ViPipe] != eSnsMirrorFlip) {
		ov48b2q_mirror_flip(ViPipe, eSnsMirrorFlip);
		g_aeOv48b2q_MirrorFip[ViPipe] = eSnsMirrorFlip;
	}
}

static CVI_VOID sensor_global_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	OV48B2Q_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsState->bInit = CVI_FALSE;
	pstSnsState->bSyncInit = CVI_FALSE;
	pstSnsState->u8ImgMode = OV48B2Q_MODE_12M10_PD;
	pstSnsState->enWDRMode = WDR_MODE_2To1_LINE;
	pstSnsState->u32FLStd  = g_astOv48b2q_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsState->au32FL[0] = g_astOv48b2q_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsState->au32FL[1] = g_astOv48b2q_mode[pstSnsState->u8ImgMode].u32VtsDef;

	memset(&pstSnsState->astSyncInfo[0], 0, sizeof(ISP_SNS_SYNC_INFO_S));
	memset(&pstSnsState->astSyncInfo[1], 0, sizeof(ISP_SNS_SYNC_INFO_S));
}

static CVI_S32 sensor_rx_attr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *pstRxAttr)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	SNS_COMBO_DEV_ATTR_S *pstRxAttrSrc = CVI_NULL;

	OV48B2Q_SENSOR_GET_CTX(ViPipe, pstSnsState);
	OV48B2Q_SENSOR_GET_COMBO(ViPipe, pstRxAttrSrc);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pstRxAttr);
	CMOS_CHECK_POINTER(pstRxAttrSrc);

	memcpy(pstRxAttr, pstRxAttrSrc, sizeof(*pstRxAttr));

	pstRxAttr->img_size.width = g_astOv48b2q_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Width;
	pstRxAttr->img_size.height = g_astOv48b2q_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Height;

	if (pstSnsState->enWDRMode == WDR_MODE_NONE)
		pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_NONE;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_patch_rx_attr(VI_PIPE ViPipe, RX_INIT_ATTR_S *pstRxInitAttr)
{
	SNS_COMBO_DEV_ATTR_S* pstRxAttr = CVI_NULL;
	int i;

	if (!g_pastOv48b2qComboDevArray[ViPipe]) {
		pstRxAttr = malloc(sizeof(SNS_COMBO_DEV_ATTR_S));
	} else {
		OV48B2Q_SENSOR_GET_COMBO(ViPipe, pstRxAttr);
	}
	memcpy(pstRxAttr, &ov48b2q_rx_attr, sizeof(SNS_COMBO_DEV_ATTR_S));
	OV48B2Q_SENSOR_SET_COMBO(ViPipe, pstRxAttr);

	CMOS_CHECK_POINTER(pstRxInitAttr);

	if (pstRxInitAttr->stMclkAttr.bMclkEn)
		pstRxAttr->mclk.cam = pstRxInitAttr->stMclkAttr.u8Mclk;

	if (pstRxInitAttr->MipiDev >= VI_MAX_DEV_NUM)
		return CVI_SUCCESS;

	pstRxAttr->devno = pstRxInitAttr->MipiDev;
	pstRxAttr->cif_mode = pstRxInitAttr->MipiMode;

	if (pstRxAttr->input_mode == INPUT_MODE_MIPI) {
		struct mipi_dev_attr_s *attr = &pstRxAttr->mipi_attr;

		for (i = 0; i < MIPI_LANE_NUM + 1; i++) {
			attr->lane_id[i] = pstRxInitAttr->as16LaneId[i];
			attr->pn_swap[i] = pstRxInitAttr->as8PNSwap[i];
		}
	} else {
		struct lvds_dev_attr_s *attr = &pstRxAttr->lvds_attr;

		for (i = 0; i < MIPI_LANE_NUM + 1; i++) {
			attr->lane_id[i] = pstRxInitAttr->as16LaneId[i];
			attr->pn_swap[i] = pstRxInitAttr->as8PNSwap[i];
		}
	}
	pstRxAttr = CVI_NULL;
	return CVI_SUCCESS;
}

void ov48b2q_exit(VI_PIPE ViPipe)
{
	if (g_pastOv48b2qComboDevArray[ViPipe]) {
		free(g_pastOv48b2qComboDevArray[ViPipe]);
		g_pastOv48b2qComboDevArray[ViPipe] = CVI_NULL;
	}
	ov48b2q_i2c_exit(ViPipe);
}

static CVI_S32 cmos_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
	CMOS_CHECK_POINTER(pstSensorExpFunc);

	memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

	pstSensorExpFunc->pfn_cmos_sensor_init = ov48b2q_init;
	pstSensorExpFunc->pfn_cmos_sensor_exit = ov48b2q_exit;
	pstSensorExpFunc->pfn_cmos_sensor_global_init = sensor_global_init;
	pstSensorExpFunc->pfn_cmos_set_image_mode = cmos_set_image_mode;
	pstSensorExpFunc->pfn_cmos_set_wdr_mode = cmos_set_wdr_mode;
	pstSensorExpFunc->pfn_cmos_get_isp_default = cmos_get_isp_default;
	pstSensorExpFunc->pfn_cmos_get_isp_black_level = cmos_get_blc_default;
	pstSensorExpFunc->pfn_cmos_get_sns_reg_info = cmos_get_sns_regs_info;
	return CVI_SUCCESS;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
static CVI_VOID sensor_patch_i2c_addr(VI_PIPE ViPipe, CVI_S32 s32I2cAddr)
{
	if (OV48B2Q_I2C_ADDR_IS_VALID(s32I2cAddr))
		g_aunOv48b2q_AddrInfo[ViPipe].s8I2cAddr = s32I2cAddr;
	else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C addr input error ,please check [0x%x]\n", s32I2cAddr);
		g_aunOv48b2q_AddrInfo[ViPipe].s8I2cAddr = OV48B2Q_I2C_ADDR_2;
	}
}

static CVI_S32 ov48b2q_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
	g_aunOv48b2q_BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	OV48B2Q_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

	if (pastSnsStateCtx == CVI_NULL) {
		pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
		if (pastSnsStateCtx == CVI_NULL) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Isp[%d] SnsCtx malloc memory failed!\n", ViPipe);
			return -ENOMEM;
		}
	}

	memset(pastSnsStateCtx, 0, sizeof(ISP_SNS_STATE_S));

	OV48B2Q_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

	return CVI_SUCCESS;
}

static CVI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	OV48B2Q_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
	SENSOR_FREE(pastSnsStateCtx);
	OV48B2Q_SENSOR_RESET_CTX(ViPipe);
}

static CVI_S32 sensor_register_callback(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib, ALG_LIB_S *pstAwbLib)
{
	CVI_S32 s32Ret;
	ISP_SENSOR_REGISTER_S stIspRegister;
	AE_SENSOR_REGISTER_S  stAeRegister;
	AWB_SENSOR_REGISTER_S stAwbRegister;
	ISP_SNS_ATTR_INFO_S   stSnsAttrInfo;

	CMOS_CHECK_POINTER(pstAeLib);
	CMOS_CHECK_POINTER(pstAwbLib);

	s32Ret = sensor_ctx_init(ViPipe);

	if (s32Ret != CVI_SUCCESS)
		return CVI_FAILURE;

	stSnsAttrInfo.eSensorId = OV48B2Q_ID;

	s32Ret  = cmos_init_sensor_exp_function(&stIspRegister.stSnsExp);
	s32Ret |= CVI_ISP_SensorRegCallBack(ViPipe, &stSnsAttrInfo, &stIspRegister);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor register callback function failed!\n");
		return s32Ret;
	}

	s32Ret  = cmos_init_ae_exp_function(&stAeRegister.stAeExp);
	s32Ret |= CVI_AE_SensorRegCallBack(ViPipe, pstAeLib, &stSnsAttrInfo, &stAeRegister);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor register callback function to ae lib failed!\n");
		return s32Ret;
	}

	s32Ret  = cmos_init_awb_exp_function(&stAwbRegister.stAwbExp);
	s32Ret |= CVI_AWB_SensorRegCallBack(ViPipe, pstAwbLib, &stSnsAttrInfo, &stAwbRegister);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor register callback function to awb lib failed!\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 sensor_unregister_callback(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib, ALG_LIB_S *pstAwbLib)
{
	CVI_S32 s32Ret;

	CMOS_CHECK_POINTER(pstAeLib);
	CMOS_CHECK_POINTER(pstAwbLib);

	s32Ret = CVI_ISP_SensorUnRegCallBack(ViPipe, OV48B2Q_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, OV48B2Q_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, OV48B2Q_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to awb lib failed!\n");
		return s32Ret;
	}

	sensor_ctx_exit(ViPipe);

	return CVI_SUCCESS;
}

static CVI_S32 sensor_set_init(VI_PIPE ViPipe, ISP_INIT_ATTR_S *pstInitAttr)
{
	CMOS_CHECK_POINTER(pstInitAttr);

	g_au32InitExposure[ViPipe] = pstInitAttr->u32Exposure;
	g_au32LinesPer500ms[ViPipe] = pstInitAttr->u32LinesPer500ms;
	g_au16InitWBGain[ViPipe][0] = pstInitAttr->u16WBRgain;
	g_au16InitWBGain[ViPipe][1] = pstInitAttr->u16WBGgain;
	g_au16InitWBGain[ViPipe][2] = pstInitAttr->u16WBBgain;
	g_au16SampleRgain[ViPipe] = pstInitAttr->u16SampleRgain;
	g_au16SampleBgain[ViPipe] = pstInitAttr->u16SampleBgain;
	g_au16Ov48b2q_GainMode[ViPipe] = pstInitAttr->enGainMode;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_probe(VI_PIPE ViPipe)
{
	return ov48b2q_probe(ViPipe);
}

ISP_SNS_OBJ_S stSnsOv48b2q_Obj = {
	.pfnRegisterCallback    = sensor_register_callback,
	.pfnUnRegisterCallback  = sensor_unregister_callback,
	.pfnStandby             = ov48b2q_standby,
	.pfnRestart             = ov48b2q_restart,
	.pfnMirrorFlip          = sensor_mirror_flip,
	.pfnWriteReg            = ov48b2q_write_register,
	.pfnReadReg             = ov48b2q_read_register,
	.pfnSetBusInfo          = ov48b2q_set_bus_info,
	.pfnSetInit             = sensor_set_init,
	.pfnPatchRxAttr		= sensor_patch_rx_attr,
	.pfnPatchI2cAddr	= sensor_patch_i2c_addr,
	.pfnGetRxAttr		= sensor_rx_attr,
	.pfnExpSensorCb		= cmos_init_sensor_exp_function,
	.pfnExpAeCb		= cmos_init_ae_exp_function,
	.pfnSnsProbe		= sensor_probe,
};

