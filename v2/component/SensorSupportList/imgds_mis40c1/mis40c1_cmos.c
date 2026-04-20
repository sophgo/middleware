#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
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

#include "mis40c1_cmos_ex.h"
#include "mis40c1_cmos_param.h"

#define DIV_0_TO_1(a)   ((0 == (a)) ? 1 : (a))
#define DIV_0_TO_1_FLOAT(a) ((((a) < 1E-10) && ((a) > -1E-10)) ? 1 : (a))
#define MIS40C1_ID 2008
#define SENSOR_MIS40C1_WIDTH 2560
#define SENSOR_MIS40C1_HEIGHT 1440
#define MIS40C1_I2C_ADDR_1 0x30
#define MIS40C1_I2C_ADDR_2 0x31
#define MIS40C1_I2C_ADDR_IS_VALID(addr)	((addr) == MIS40C1_I2C_ADDR_1 || (addr) == MIS40C1_I2C_ADDR_2)

#define MIS40C1_EXPACCURACY                    (1)

/****************************************************************************
 * global variables                                                            *
 ****************************************************************************/

ISP_SNS_STATE_S *g_pastMIS40C1[VI_MAX_PIPE_NUM] = {CVI_NULL};
SNS_COMBO_DEV_ATTR_S *g_pastMIS40C1ComboDevArray[VI_MAX_PIPE_NUM] = {CVI_NULL};

#define MIS40C1_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastMIS40C1[dev])
#define MIS40C1_SENSOR_SET_CTX(dev, pstCtx)   (g_pastMIS40C1[dev] = pstCtx)
#define MIS40C1_SENSOR_RESET_CTX(dev)         (g_pastMIS40C1[dev] = CVI_NULL)
#define MIS40C1_SENSOR_GET_COMBO(dev, pstCtx)   (pstCtx = g_pastMIS40C1ComboDevArray[dev])
#define MIS40C1_SENSOR_SET_COMBO(dev, pstCtx)   (g_pastMIS40C1ComboDevArray[dev] = pstCtx)

ISP_SNS_COMMBUS_U g_aunMIS40C1_BusInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cDev = 0},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cDev = -1}
};

ISP_SNS_COMMADDR_U g_aunMIS40C1_AddrInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cAddr = 0},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cAddr = -1}
};

CVI_U16 g_au16MIS40C1_GainMode[VI_MAX_PIPE_NUM] = {0};
CVI_U16 g_au16MIS40C1_L2SMode[VI_MAX_PIPE_NUM] = {0};

ISP_SNS_MIRRORFLIP_TYPE_E g_aeMis40c1_MirrorFip[VI_MAX_PIPE_NUM] = {0};

/****************************************************************************
 * local variables and functions                                             *
 ****************************************************************************/
static CVI_U32 g_au32InitExposure[VI_MAX_PIPE_NUM]  = {0};
static CVI_U32 g_au32LinesPer500ms[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16InitWBGain[VI_MAX_PIPE_NUM][3] = {{0} };
static CVI_U16 g_au16SampleRgain[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16SampleBgain[VI_MAX_PIPE_NUM] = {0};
static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg);
/*****MIS40C1 Lines Range*****/
#define MIS40C1_FULL_LINES_MAX  (0xFFFF)

/*****MIS40C1 Register Address*****/
#define MIS40C1_EXP_ADDR	0x3100
#define MIS40C1_AGAIN_ADDR	0x3103
#define MIS40C1_DGAIN_ADDR	0x3709
#define LINEAR_HOLD_ADDR		0x300C
#define MIS40C1_VMAX_ADDR	0x3105
#define MIS40C1_FLIP_MIRROR_ADDR 0x3007
#define MIS40C1_UNUSED_ADDR 0x301c

#define MIS40C1_RES_IS_1440P(w, h)      ((w) == 2560 && (h) == 1440)

static CVI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	MIS40C1_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FlickerFreq = 50 * 256;
	pstAeSnsDft->u32FullLinesMax = MIS40C1_FULL_LINES_MAX;
	pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * 30);

	pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
	pstAeSnsDft->stIntTimeAccu.f32Accuracy = MIS40C1_EXPACCURACY;
	pstAeSnsDft->stIntTimeAccu.f32Offset = 0;

	pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
	pstAeSnsDft->stAgainAccu.f32Accuracy = 1;

	pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
	pstAeSnsDft->stDgainAccu.f32Accuracy = 1;

	pstAeSnsDft->u32ISPDgainShift = 8;
	pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
	pstAeSnsDft->u32MaxISPDgainTarget = 2 << pstAeSnsDft->u32ISPDgainShift;

	if (g_au32LinesPer500ms[ViPipe] == 0)
		pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * 30 / 2;
	else
		pstAeSnsDft->u32LinesPer500ms = g_au32LinesPer500ms[ViPipe];
	pstAeSnsDft->u32SnsStableFrame = 0;

	switch (pstSnsState->enWDRMode) {
	case WDR_MODE_NONE:   /*linear mode*/
		pstAeSnsDft->f32Fps = g_astMIS40C1_mode[pstSnsState->u8ImgMode].f32MaxFps;
		pstAeSnsDft->f32MinFps = g_astMIS40C1_mode[pstSnsState->u8ImgMode].f32MinFps;
		pstAeSnsDft->au8HistThresh[0] = 0xd;
		pstAeSnsDft->au8HistThresh[1] = 0x28;
		pstAeSnsDft->au8HistThresh[2] = 0x60;
		pstAeSnsDft->au8HistThresh[3] = 0x80;

		pstAeSnsDft->u32MaxAgain = g_astMIS40C1_mode[pstSnsState->u8ImgMode].stAgain[0].u32Max;
		pstAeSnsDft->u32MinAgain = g_astMIS40C1_mode[pstSnsState->u8ImgMode].stAgain[0].u32Min;
		pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
		pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

		pstAeSnsDft->u32MaxDgain = g_astMIS40C1_mode[pstSnsState->u8ImgMode].stDgain[0].u32Max;
		pstAeSnsDft->u32MinDgain = g_astMIS40C1_mode[pstSnsState->u8ImgMode].stDgain[0].u32Min;
		pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
		pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

		pstAeSnsDft->u8AeCompensation = 40;
		pstAeSnsDft->u32InitAESpeed = 64;
		pstAeSnsDft->u32InitAETolerance = 5;
		pstAeSnsDft->u32AEResponseFrame = 4;
		pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
		pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe] ? g_au32InitExposure[ViPipe] : 76151;

		pstAeSnsDft->u32MaxIntTime = g_astMIS40C1_mode[pstSnsState->u8ImgMode].stExp[0].u32Max;
		pstAeSnsDft->u32MinIntTime = g_astMIS40C1_mode[pstSnsState->u8ImgMode].stExp[0].u32Min;
		pstAeSnsDft->u32MaxIntTimeTarget = 65535;
		pstAeSnsDft->u32MinIntTimeTarget = 1;
		break;
	default:
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support sensor mode: %d\n", pstSnsState->u8ImgMode);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

/* the function of sensor set fps */
static CVI_S32 cmos_fps_set(VI_PIPE ViPipe, CVI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	CVI_U32 u32VMAX;
	CVI_FLOAT f32MaxFps = 0;
	CVI_FLOAT f32MinFps = 0;
	CVI_U32 u32Vts = 0;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	MIS40C1_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u32Vts = g_astMIS40C1_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	f32MaxFps = g_astMIS40C1_mode[pstSnsState->u8ImgMode].f32MaxFps;
	f32MinFps = g_astMIS40C1_mode[pstSnsState->u8ImgMode].f32MinFps;

	switch (pstSnsState->u8ImgMode) {
	case MIS40C1_MODE_1440P20_1L:
	case MIS40C1_MODE_1440P20_2L:
		if ((f32Fps <= f32MaxFps) && (f32Fps >= f32MinFps)) {
			u32VMAX = u32Vts * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
			return CVI_FAILURE;
		}
		u32VMAX = (u32VMAX > MIS40C1_FULL_LINES_MAX) ? MIS40C1_FULL_LINES_MAX : u32VMAX;
		break;
	default:
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support sensor mode: %d\n", pstSnsState->u8ImgMode);
		return CVI_FAILURE;
	}

	pstSnsState->u32FLStd = u32VMAX;
    //frame high addr
	pstSnsRegsInfo->astI2cData[LINEAR_VMAX_0_ADDR].u32Data = ((u32VMAX & 0xFF00)>>8);//data[15:8]  => [7:0] //h
	pstSnsRegsInfo->astI2cData[LINEAR_VMAX_1_ADDR].u32Data = (u32VMAX & 0xFF);//data[7:0]  [7:0] //l

	pstAeSnsDft->f32Fps = f32Fps;
	pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * f32Fps / 2;
	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32MaxIntTime = (pstSnsState->u32FLStd << 1) - 8;
	pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FullLines = pstSnsState->au32FL[0];
	pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * DIV_0_TO_1_FLOAT(f32Fps));

	return CVI_SUCCESS;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static CVI_S32 cmos_inttime_update(VI_PIPE ViPipe, CVI_U32 *u32IntTime)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	CVI_U32 u32TmpIntTime, u32MinTime, u32MaxTime;

	MIS40C1_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(u32IntTime);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	/* linear exposure reg range:
	 * min : 1
	 * max : vts - 1
	 * step : 1
	 */
	u32MinTime = 1;
	u32MaxTime = pstSnsState->au32FL[0] - 1;
	u32TmpIntTime = (u32IntTime[0] > u32MaxTime) ? u32MaxTime : u32IntTime[0];
	u32TmpIntTime = (u32TmpIntTime < u32MinTime) ? u32MinTime : u32TmpIntTime;

	pstSnsRegsInfo->astI2cData[LINEAR_SHS1_0_ADDR].u32Data = ((u32TmpIntTime & 0xFF00) >> 8); //data[15:8]  => [7:0] //h
	pstSnsRegsInfo->astI2cData[LINEAR_SHS1_1_ADDR].u32Data = ((u32TmpIntTime & 0xFF));  //data[7:0]  [7:0] //l

	return CVI_SUCCESS;
}

struct gain_tbl_info_s {
	CVI_U16	gainMax;
	CVI_U16	idxBase;
	CVI_U8	regGain;
	CVI_U8	regGainFineBase;
	CVI_U8	regGainFineStep;
};

static struct gain_tbl_info_s DgainInfo[] = {
	{
		.gainMax = 2032,
		.idxBase = 0,
		.regGain = 0x01,
		.regGainFineBase = 0x80,
		.regGainFineStep = 1,
	},

};



static CVI_U32 Dgain_table[] = {
	1024, 1032, 1040, 1048, 1056, 1064, 1072, 1080, 1088, 1096, 1104, 1112, 1120, 1128, 1136, 1144,
	1152, 1160, 1168, 1176, 1184, 1192, 1200, 1208, 1216, 1224, 1232, 1240, 1248, 1256, 1264, 1272,
	1280, 1288, 1296, 1304, 1312, 1320, 1328, 1336, 1344, 1352, 1360, 1368, 1376, 1384, 1392, 1400,
	1408, 1416, 1424, 1432, 1440, 1448, 1456, 1464, 1472, 1480, 1488, 1496, 1504, 1512, 1520, 1528,
	1536, 1544, 1552, 1560, 1568, 1576, 1584, 1592, 1600, 1608, 1616, 1624, 1632, 1640, 1648, 1656,
	1664, 1672, 1680, 1688, 1696, 1704, 1712, 1720, 1728, 1736, 1744, 1752, 1760, 1768, 1776, 1784,
	1792, 1800, 1808, 1816, 1824, 1832, 1840, 1848, 1856, 1864, 1872, 1880, 1888, 1896, 1904, 1912,
	1920, 1928, 1936, 1944, 1952, 1960, 1968, 1976, 1984, 1992, 2000, 2008, 2016, 2024, 2032
};

static CVI_U32 Again_table[] = {
    1024, 1088, 1152, 1216, 1280, 1344, 1408, 1472, 1536, 1600, 1664, 1728, 1792, 1856, 1920, 1984,
	2048, 2112, 2176, 2240, 2304, 2368, 2432, 2496, 2560, 2624, 2688, 2752, 2816, 2880, 2944, 3008,
	3072, 3136, 3200, 3264, 3328, 3392, 3456, 3520, 3584, 3648, 3712, 3776, 3840, 3904, 3968, 4032,
	4096, 4160, 4224, 4288, 4352, 4416, 4480, 4544, 4608, 4672, 4736, 4800, 4864, 4928, 4992, 5056,
	5120, 5184, 5248, 5312, 5376, 5440, 5504, 5568, 5632, 5696, 5760, 5824, 5888, 5952, 6016, 6080,
	6144, 6208, 6272, 6336, 6400, 6464, 6528, 6592, 6656, 6720, 6784, 6848, 6912, 6976, 7040, 7104,
	7168, 7232, 7296, 7360, 7424, 7488, 7552, 7616, 7680, 7744, 7808, 7872, 7936, 8000, 8064, 8128,
	8192, 8256, 8320, 8384, 8448, 8512, 8576, 8640, 8704, 8768, 8832, 8896, 8960, 9024, 9088, 9152,
	9216, 9280, 9344, 9408, 9472, 9536, 9600, 9664, 9728, 9792, 9856, 9920, 9984, 10048, 10112, 10176,
	10240, 10304, 10368, 10432, 10496, 10560, 10624, 10688, 10752, 10816, 10880, 10944, 11008, 11072, 11136, 11200,
	11264, 11328, 11392, 11456, 11520, 11584, 11648, 11712, 11776, 11840, 11904, 11968, 12032, 12096, 12160, 12224,
	12288, 12352, 12416, 12480, 12544, 12608, 12672, 12736, 12800, 12864, 12928, 12992, 13056, 13120, 13184, 13248,
	13312, 13376, 13440, 13504, 13568, 13632, 13696, 13760, 13824, 13888, 13952, 14016, 14080, 14144, 14208, 14272,
	14336, 14400, 14464, 14528, 14592, 14656, 14720, 14784, 14848, 14912, 14976, 15040, 15104, 15168, 15232, 15296,
	15360, 15424, 15488, 15552, 15616, 15680, 15744, 15808, 15872, 15936, 16000, 16064, 16128, 16192, 16256, 16320,
	16384, 16512, 16640, 16768, 16896, 17024, 17152, 17280, 17408, 17536, 17664, 17792, 17920, 18048, 18176, 18304,
	18432, 18560, 18688, 18816, 18944, 19072, 19200, 19328, 19456, 19584, 19712, 19840, 19968, 20096, 20224, 20352,
	20480, 20608, 20736, 20864, 20992, 21120, 21248, 21376, 21504, 21632, 21760, 21888, 22016, 22144, 22272, 22400,
	22528, 22656, 22784, 22912, 23040, 23168, 23296, 23424, 23552, 23680, 23808, 23936, 24064, 24192, 24320, 24448,
	24576, 24704, 24832, 24960, 25088, 25216, 25344, 25472, 25600, 25728, 25856, 25984, 26112, 26240, 26368, 26496,
	26624, 26752, 26880, 27008, 27136, 27264, 27392, 27520, 27648, 27776, 27904, 28032, 28160, 28288, 28416, 28544,
	28672, 28800, 28928, 29056, 29184, 29312, 29440, 29568, 29696, 29824, 29952, 30080, 30208, 30336, 30464, 30592,
	30720, 30848, 30976, 31104, 31232, 31360, 31488, 31616, 31744, 31872, 32000, 32128, 32256, 32384, 32512, 32640,
	32768, 33024, 33280, 33536, 33792, 34048, 34304, 34560, 34816, 35072, 35328, 35584, 35840, 36096, 36352, 36608,
	36864, 37120, 37376, 37632, 37888, 38144, 38400, 38656, 38912, 39168, 39424, 39680, 39936, 40192, 40448, 40704,
	40960, 41216, 41472, 41728, 41984, 42240, 42496, 42752, 43008, 43264, 43520, 43776, 44032, 44288, 44544, 44800,
	45056, 45312, 45568, 45824, 46080, 46336, 46592, 46848, 47104, 47360, 47616, 47872, 48128, 48384, 48640, 48896,
	49152, 49664, 50176, 50688, 51200, 51712, 52224, 52736, 53248, 53760, 54272, 54784, 55296, 55808, 56320, 56832,
	57344, 57856, 58368, 58880, 59392, 59904, 60416, 60928, 61440, 61952, 62464, 62976, 63488, 64000, 64512, 65024,
	65536
};

static const CVI_U32 again_table_size = ARRAY_SIZE(Again_table);
static const CVI_U32 dgain_table_size = ARRAY_SIZE(Dgain_table);

static CVI_S32 cmos_again_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32AgainLin, CVI_U32 *pu32AgainDb)
{
	CVI_U32 i;

	(void) ViPipe;

	CMOS_CHECK_POINTER(pu32AgainLin);
	CMOS_CHECK_POINTER(pu32AgainDb);

	if (*pu32AgainLin >= Again_table[again_table_size - 1]) {
		*pu32AgainLin = Again_table[again_table_size - 1];
		*pu32AgainDb = again_table_size - 1;
		return CVI_SUCCESS;
	}

	for (i = 1; i < again_table_size; i++) {
		if (*pu32AgainLin < Again_table[i]) {
			*pu32AgainLin = Again_table[i - 1];
			*pu32AgainDb = i - 1;
			break;
		}
	}
	return CVI_SUCCESS;
}

static CVI_S32 cmos_dgain_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32DgainLin, CVI_U32 *pu32DgainDb)
{
	CVI_U32 i;

	(void)ViPipe;

	CMOS_CHECK_POINTER(pu32DgainLin);
	CMOS_CHECK_POINTER(pu32DgainDb);

	if (*pu32DgainLin >= Dgain_table[dgain_table_size - 1]) {
		*pu32DgainLin = Dgain_table[dgain_table_size - 1];
		*pu32DgainDb = dgain_table_size - 1;
		return CVI_SUCCESS;
	}

	for (i = 1; i < dgain_table_size; i++) {
		if (*pu32DgainLin < Dgain_table[i]) {
			*pu32DgainLin = Dgain_table[i - 1];
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
	CVI_U32 u32AgainIdx;
	CVI_U32 u32DgainIdx;
	CVI_U32 u32Again;
	CVI_U32 u32Dgain;
	struct gain_tbl_info_s *info;
	int i, tbl_num;


	MIS40C1_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pu32Again);
	CMOS_CHECK_POINTER(pu32Dgain);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	u32AgainIdx = pu32Again[0];
	u32DgainIdx = pu32Dgain[0];
	u32Again = (CVI_U32)(1024.0 * (1.0 - 1024.0/(double)Again_table[u32AgainIdx]));
	u32Dgain = u32DgainIdx;

	pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_0_ADDR].u32Data = ((u32Again >> 8) & 0x03);
	pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_1_ADDR].u32Data = (u32Again & 0xFF);

	/* find Dgain register setting. */
	tbl_num = sizeof(DgainInfo)/sizeof(struct gain_tbl_info_s);
	for (i = tbl_num - 1; i >= 0; i--) {
		info = &DgainInfo[i];
		if (u32Dgain >= info->idxBase)
			break;
	}

	u32Dgain = info->regGainFineBase + (u32Dgain - info->idxBase) * info->regGainFineStep;
	pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_ADDR_R].u32Data = u32Dgain;
	pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_ADDR_GR].u32Data = u32Dgain;
	pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_ADDR_GB].u32Data = u32Dgain;
	pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_ADDR_B].u32Data = u32Dgain;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	CMOS_CHECK_POINTER(pstExpFuncs);

	memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

	pstExpFuncs->pfn_cmos_get_ae_default    = cmos_get_ae_default;
	pstExpFuncs->pfn_cmos_fps_set           = cmos_fps_set;
	//pstExpFuncs->pfn_cmos_slow_framerate_set = cmos_slow_framerate_set;
	pstExpFuncs->pfn_cmos_inttime_update    = cmos_inttime_update;
	pstExpFuncs->pfn_cmos_gains_update      = cmos_gains_update;
	pstExpFuncs->pfn_cmos_again_calc_table  = cmos_again_calc_table;
	pstExpFuncs->pfn_cmos_dgain_calc_table  = cmos_dgain_calc_table;
	//pstExpFuncs->pfn_cmos_get_inttime_max   = cmos_get_inttime_max;
	//pstExpFuncs->pfn_cmos_ae_fswdr_attr_set = cmos_ae_fswdr_attr_set;

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
	const MIS40C1_MODE_S *pstMode = CVI_NULL;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	MIS40C1_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstMode = &g_astMIS40C1_mode[pstSnsState->u8ImgMode];

	if (pstSnsState->enWDRMode != WDR_MODE_NONE) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport WDRMode: %d\n", pstSnsState->enWDRMode);
	} else {
		pstIspCfg->frm_num = 1;
		memcpy(&pstIspCfg->img_size[0], &pstMode->astImg[0], sizeof(ISP_WDR_SIZE_S));
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_wdr_mode(VI_PIPE ViPipe, CVI_U8 u8Mode)
{
	UNUSED(ViPipe);
	UNUSED(u8Mode);
	CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport sensor mode!\n");
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
	MIS40C1_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstSnsRegsInfo = &pstSnsSyncInfo->snsCfg;
	pstCfg0 = &pstSnsState->astSyncInfo[0];
	pstCfg1 = &pstSnsState->astSyncInfo[1];
	pstI2c_data = pstCfg0->snsCfg.astI2cData;

	if ((pstSnsState->bSyncInit == CVI_FALSE) || (pstSnsRegsInfo->bConfig == CVI_FALSE)) {
		pstCfg0->snsCfg.enSnsType = SNS_I2C_TYPE;
		pstCfg0->snsCfg.unComBus.s8I2cDev = g_aunMIS40C1_BusInfo[ViPipe].s8I2cDev;
		pstCfg0->snsCfg.u8Cfg2ValidDelayMax = 0;
		pstCfg0->snsCfg.use_snsr_sram = CVI_TRUE;
		pstCfg0->snsCfg.u32RegNum = LINEAR_REGS_NUM;

		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			pstI2c_data[i].bUpdate = CVI_TRUE;
			pstI2c_data[i].u8DevAddr =  g_aunMIS40C1_AddrInfo[ViPipe].s8I2cAddr;
			pstI2c_data[i].u32AddrByteNum = mis40c1_addr_byte;
			pstI2c_data[i].u32DataByteNum = mis40c1_data_byte;
		}
		switch (pstSnsState->enWDRMode) {
		case WDR_MODE_NONE:
			//Linear Mode Regs
			pstI2c_data[LINEAR_SHS1_0_ADDR].u32RegAddr     = MIS40C1_EXP_ADDR;
			pstI2c_data[LINEAR_SHS1_1_ADDR].u32RegAddr     = MIS40C1_EXP_ADDR + 1;

			pstI2c_data[LINEAR_AGAIN_0_ADDR].u32RegAddr      = MIS40C1_AGAIN_ADDR;
			pstI2c_data[LINEAR_AGAIN_1_ADDR].u32RegAddr      = MIS40C1_AGAIN_ADDR + 1;
			pstI2c_data[LINEAR_DGAIN_ADDR_R].u32RegAddr      = MIS40C1_DGAIN_ADDR;
			pstI2c_data[LINEAR_DGAIN_ADDR_GR].u32RegAddr = MIS40C1_DGAIN_ADDR + 1;
			pstI2c_data[LINEAR_DGAIN_ADDR_GB].u32RegAddr = MIS40C1_DGAIN_ADDR + 2;
			pstI2c_data[LINEAR_DGAIN_ADDR_B].u32RegAddr   = MIS40C1_DGAIN_ADDR + 3;

			pstI2c_data[LINEAR_VMAX_0_ADDR].u32RegAddr     = MIS40C1_VMAX_ADDR;
			pstI2c_data[LINEAR_VMAX_1_ADDR].u32RegAddr     = MIS40C1_VMAX_ADDR + 1;
			pstI2c_data[LINEAR_LAUNCH].u32RegAddr     = LINEAR_HOLD_ADDR;
			pstI2c_data[LINEAR_LAUNCH].u32Data = 0x01;
			pstI2c_data[LINEAR_UNUSDE_ADDR].u32RegAddr     = MIS40C1_UNUSED_ADDR;
			pstI2c_data[LINEAR_FLIP_MIRROR_ADDR].u32RegAddr     = MIS40C1_FLIP_MIRROR_ADDR;
			break;
		default:
			CVI_TRACE_SNS(CVI_DBG_ERR, "NOT support this mode!\n");
			return CVI_FAILURE;
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

		if (pstCfg0->snsCfg.need_update == CVI_TRUE) {
			pstI2c_data[LINEAR_LAUNCH].u32Data     = 0x01;
			pstI2c_data[LINEAR_LAUNCH].bUpdate = CVI_TRUE;
		}
		/* check update isp crop or not */
		pstCfg0->ispCfg.need_update = (sensor_cmp_wdr_size(&pstCfg0->ispCfg, &pstCfg1->ispCfg) ?
				CVI_TRUE : CVI_FALSE);
		pstCfg0->ispCfg.u8DelayFrmNum = 1;

	}

	pstSnsRegsInfo->bConfig = CVI_FALSE;
	memcpy(pstSnsSyncInfo, &pstSnsState->astSyncInfo[0], sizeof(ISP_SNS_SYNC_INFO_S));
	memcpy(&pstSnsState->astSyncInfo[1], &pstSnsState->astSyncInfo[0], sizeof(ISP_SNS_SYNC_INFO_S));
	pstSnsState->au32FL[1] = pstSnsState->au32FL[0];
	pstCfg0->snsCfg.astI2cData[LINEAR_FLIP_MIRROR_ADDR].bDropFrm = CVI_FALSE;
	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_image_mode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
	CVI_U8 u8SensorImageMode = 0;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstSensorImageMode);
	MIS40C1_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u8SensorImageMode = pstSnsState->u8ImgMode;
	pstSnsState->bSyncInit = CVI_FALSE;

	if (pstSensorImageMode->f32Fps <= 30) {
		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (MIS40C1_RES_IS_1440P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height)) {
				if (pstSensorImageMode->u8LaneNum == 2) {
					u8SensorImageMode = MIS40C1_MODE_1440P20_2L;
				} else {
					u8SensorImageMode = MIS40C1_MODE_1440P20_1L;
				}
			} else {
				CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
				       pstSensorImageMode->u16Width,
				       pstSensorImageMode->u16Height,
				       pstSensorImageMode->f32Fps,
				       pstSnsState->enWDRMode);
				return CVI_FAILURE;
			}
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
			       pstSensorImageMode->u16Width,
			       pstSensorImageMode->u16Height,
			       pstSensorImageMode->f32Fps,
			       pstSnsState->enWDRMode);
			return CVI_FAILURE;
		}
	} else {
	}

	if ((pstSnsState->bInit == CVI_TRUE) && (u8SensorImageMode == pstSnsState->u8ImgMode)) {
		/* Don't need to switch SensorImageMode */
		return CVI_FAILURE;
	}

	pstSnsState->u8ImgMode = u8SensorImageMode;

	return CVI_SUCCESS;
}

static CVI_VOID sensor_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	ISP_SNS_ISP_INFO_S *pstIspCfg0 = CVI_NULL;
	CVI_U8 value = 0;
	CVI_U8 state = 0;
	CVI_U8 start_x, start_y;

	MIS40C1_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	pstIspCfg0 = &pstSnsState->astSyncInfo[0].ispCfg;

	if (pstSnsState->bInit == CVI_TRUE && g_aeMis40c1_MirrorFip[ViPipe] != eSnsMirrorFlip) {
		switch (eSnsMirrorFlip) {
		case ISP_SNS_NORMAL:
			value = 0x0;
			start_x = 0;
			start_y = 0;
			state  = 0x2;
			break;
		case ISP_SNS_MIRROR:
			value = 0x1;
			start_x = 1;
			start_y = 0;
			state  = 0x4;
			break;
		case ISP_SNS_FLIP:
			value = 0x2;
			start_x = 0;
			start_y = 1;
			state  = 0x6;
			break;
		case ISP_SNS_MIRROR_FLIP:
			value = 0x3;
			start_x = 1;
			start_y = 1;
			state  = 0x8;
			break;
		default:
			return;
		}

		pstSnsRegsInfo->astI2cData[LINEAR_UNUSDE_ADDR].u32Data = state;
		pstSnsRegsInfo->astI2cData[LINEAR_UNUSDE_ADDR].bDropFrm = 1;
		pstSnsRegsInfo->astI2cData[LINEAR_UNUSDE_ADDR].u8DropFrmNum = 2;

		pstSnsRegsInfo->astI2cData[LINEAR_FLIP_MIRROR_ADDR].u32Data = value;
		pstSnsRegsInfo->astI2cData[LINEAR_FLIP_MIRROR_ADDR].u8DelayFrmNum = 1;
		pstIspCfg0->img_size[0].stWndRect.s32X = start_x;
		pstIspCfg0->img_size[0].stWndRect.s32Y = start_y;
		g_aeMis40c1_MirrorFip[ViPipe] = eSnsMirrorFlip;
	}

}

static CVI_VOID sensor_global_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	const MIS40C1_MODE_S *pstMode = CVI_NULL;

	MIS40C1_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsState->bInit = CVI_FALSE;
	pstSnsState->bSyncInit = CVI_FALSE;
	pstSnsState->u8ImgMode = MIS40C1_MODE_1440P20_2L;
	pstSnsState->enWDRMode = WDR_MODE_NONE;
	pstMode = &g_astMIS40C1_mode[pstSnsState->u8ImgMode];
	pstSnsState->u32FLStd  = pstMode->u32VtsDef;
	pstSnsState->au32FL[0] = pstMode->u32VtsDef;
	pstSnsState->au32FL[1] = pstMode->u32VtsDef;

	memset(&pstSnsState->astSyncInfo[0], 0, sizeof(ISP_SNS_SYNC_INFO_S));
	memset(&pstSnsState->astSyncInfo[1], 0, sizeof(ISP_SNS_SYNC_INFO_S));
}

static CVI_S32 sensor_rx_attr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *pstRxAttr)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	SNS_COMBO_DEV_ATTR_S *pstRxAttrSrc = CVI_NULL;

	MIS40C1_SENSOR_GET_CTX(ViPipe, pstSnsState);
	MIS40C1_SENSOR_GET_COMBO(ViPipe, pstRxAttrSrc);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pstRxAttr);
	CMOS_CHECK_POINTER(pstRxAttrSrc);

	memcpy(pstRxAttr, pstRxAttrSrc, sizeof(*pstRxAttr));

	pstRxAttr->img_size.start_x = g_astMIS40C1_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.s32X;
	pstRxAttr->img_size.start_y = g_astMIS40C1_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.s32Y;
	pstRxAttr->img_size.active_w = g_astMIS40C1_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.u32Width;
	pstRxAttr->img_size.active_h = g_astMIS40C1_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.u32Height;
	pstRxAttr->img_size.width = g_astMIS40C1_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Width;
	pstRxAttr->img_size.height = g_astMIS40C1_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Height;
	pstRxAttr->img_size.max_width = g_astMIS40C1_mode[pstSnsState->u8ImgMode].astImg[0].stMaxSize.u32Width;
	pstRxAttr->img_size.max_height = g_astMIS40C1_mode[pstSnsState->u8ImgMode].astImg[0].stMaxSize.u32Height;


	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_NONE;
	}

	pstRxAttrSrc = CVI_NULL;

	return CVI_SUCCESS;

}

static CVI_S32 sensor_patch_rx_attr(VI_PIPE ViPipe, RX_INIT_ATTR_S *pstRxInitAttr)
{
	int i;
	SNS_COMBO_DEV_ATTR_S *pstRxAttr = malloc(sizeof(SNS_COMBO_DEV_ATTR_S));

	if (!g_pastMIS40C1ComboDevArray[ViPipe]) {
		pstRxAttr = malloc(sizeof(SNS_COMBO_DEV_ATTR_S));
	} else {
		MIS40C1_SENSOR_GET_COMBO(ViPipe, pstRxAttr);
	}
	memcpy(pstRxAttr, &mis40c1_rx_attr, sizeof(SNS_COMBO_DEV_ATTR_S));
	MIS40C1_SENSOR_SET_COMBO(ViPipe, pstRxAttr);

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

static CVI_S32 cmos_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
	CMOS_CHECK_POINTER(pstSensorExpFunc);

	memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

	pstSensorExpFunc->pfn_cmos_sensor_init = mis40c1_init;
	pstSensorExpFunc->pfn_cmos_sensor_exit = mis40c1_exit;
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
	if (MIS40C1_I2C_ADDR_IS_VALID(s32I2cAddr))
	    g_aunMIS40C1_AddrInfo[ViPipe].s8I2cAddr = s32I2cAddr;
	else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C addr input error ,please check [0x%x]\n", s32I2cAddr);
		g_aunMIS40C1_AddrInfo[ViPipe].s8I2cAddr = MIS40C1_I2C_ADDR_1;
	}
}

static CVI_S32 mis40c1_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
	g_aunMIS40C1_BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	MIS40C1_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

	if (pastSnsStateCtx == CVI_NULL) {
		pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
		if (pastSnsStateCtx == CVI_NULL) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Isp[%d] SnsCtx malloc memory failed!\n", ViPipe);
			return -ENOMEM;
		}
	}

	memset(pastSnsStateCtx, 0, sizeof(ISP_SNS_STATE_S));

	MIS40C1_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

	return CVI_SUCCESS;
}

static CVI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	MIS40C1_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
	SENSOR_FREE(pastSnsStateCtx);
	MIS40C1_SENSOR_RESET_CTX(ViPipe);
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

	stSnsAttrInfo.eSensorId = MIS40C1_ID;

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

	s32Ret = CVI_ISP_SensorUnRegCallBack(ViPipe, MIS40C1_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, MIS40C1_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, MIS40C1_ID);
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
	g_au16MIS40C1_GainMode[ViPipe] = pstInitAttr->enGainMode;
	g_au16MIS40C1_L2SMode[ViPipe] = pstInitAttr->enL2SMode;

	return CVI_SUCCESS;
}

ISP_SNS_OBJ_S stSnsMIS40C1_Obj = {
	.pfnRegisterCallback	= sensor_register_callback,
	.pfnUnRegisterCallback	= sensor_unregister_callback,
	.pfnStandby		= mis40c1_standby,
	.pfnRestart		= mis40c1_restart,
	.pfnMirrorFlip		= sensor_mirror_flip,
	.pfnWriteReg		= mis40c1_write_register,
	.pfnReadReg		= mis40c1_read_register,
	.pfnSetBusInfo		= mis40c1_set_bus_info,
	.pfnSetInit		= sensor_set_init,
	.pfnPatchRxAttr		= sensor_patch_rx_attr,
	.pfnPatchI2cAddr	= sensor_patch_i2c_addr,
	.pfnGetRxAttr		= sensor_rx_attr,
	.pfnExpSensorCb		= cmos_init_sensor_exp_function,
	.pfnExpAeCb		= cmos_init_ae_exp_function,
	.pfnSnsProbe		= mis40c1_probe,
};
