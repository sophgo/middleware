
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/param.h>
#include <inttypes.h>
#include <signal.h>

#include "cvi_buffer.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_comm_isp.h"
#include "cvi_comm_sns.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_isp.h"
#include "cvi_sns_ctrl.h"
#include "sample_comm.h"

#define DELAY_500MS() (usleep(500 * 1000))

typedef struct STITCH_CALI_INFO_T {
	CVI_U32 u32ExpTime;
	CVI_U32 u32AGain;
	CVI_U32 u32DGain;
	CVI_U32 u32ISPDGain;
	CVI_U32 u32RGain;
	CVI_U32 u32BGain;
} STITCH_CALI_INFO;

static STITCH_CALI_INFO g_caliInfo;

static CVI_BOOL g_bEnableRun;

static SAMPLE_VI_CONFIG_S g_stViConfig;
static SAMPLE_INI_CFG_S g_stIniCfg;

static int sys_vi_init(void)
{
	MMF_VERSION_S stVersion;
	SAMPLE_INI_CFG_S stIniCfg;
	SAMPLE_VI_CONFIG_S stViConfig;

	PIC_SIZE_E enPicSize;
	SIZE_S stSize;
	CVI_S32 s32Ret = CVI_SUCCESS;
	LOG_LEVEL_CONF_S log_conf;

	CVI_SYS_GetVersion(&stVersion);
	SAMPLE_PRT("MMF Version:%s\n", stVersion.version);

	log_conf.enModId = CVI_ID_LOG;
	log_conf.s32Level = CVI_DBG_INFO;
	CVI_LOG_SetLevelConf(&log_conf);

	// Get config from ini if found.
	s32Ret = SAMPLE_COMM_VI_ParseIni(&stIniCfg);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Parse fail\n");
	} else {
		SAMPLE_PRT("Parse complete\n");
	}

	//Set sensor number
	CVI_VI_SetDevNum(stIniCfg.devNum);

	/************************************************
	 * step1:  Config VI
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_IniToViCfg(&stIniCfg, &stViConfig);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memcpy(&g_stViConfig, &stViConfig, sizeof(SAMPLE_VI_CONFIG_S));
	memcpy(&g_stIniCfg, &stIniCfg, sizeof(SAMPLE_INI_CFG_S));

	/************************************************
	 * step2:  Get input size
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stIniCfg.enSnsType[0], &enPicSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VI_GetSizeBySensor failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * step3:  Init modules
	 ************************************************/
	s32Ret = SAMPLE_PLAT_SYS_INIT(stSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "sys init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_PLAT_VI_INIT(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static void sys_vi_deinit(void)
{
	SAMPLE_COMM_VI_DestroyIsp(&g_stViConfig);

	SAMPLE_COMM_VI_DestroyVi(&g_stViConfig);

	SAMPLE_COMM_SYS_Exit();
}

static void signal_handler(int signo)
{
	if (g_bEnableRun) {
		signal(signo, SIG_IGN);
		g_bEnableRun = CVI_FALSE;
	} else {
		exit(-1);
	}
}

static void print_usage(char *sPrgNm)
{
	UNUSED(sPrgNm);
	printf("current AE AWB calibration Info :\n");
	printf("    ExpTime: %d AGain: %d DGain: %d ISPDGain: %d RGain:%d BGain:%d\n\n",g_caliInfo.u32ExpTime,
				g_caliInfo.u32AGain, g_caliInfo.u32DGain, g_caliInfo.u32ISPDGain,
				g_caliInfo.u32RGain, g_caliInfo.u32BGain);
	printf("mode:\n");
	printf("    0  Get current AE & AWB info.\n");
	printf("    1   Calibration.\n");
	printf("    255  exit.\n:");
}



static void get_ae_awb_info(VI_PIPE ViPipe)
{
	ISP_EXP_INFO_S aeInfo;
	ISP_WB_Q_INFO_S wbInfo;

	CVI_ISP_QueryExposureInfo(ViPipe, &aeInfo);
	CVI_AWB_QueryInfo(ViPipe, &wbInfo);
	g_caliInfo.u32ExpTime = aeInfo.u32ExpTime;
	g_caliInfo.u32AGain = aeInfo.u32AGain;
	g_caliInfo.u32DGain = aeInfo.u32DGain;
	g_caliInfo.u32ISPDGain = aeInfo.u32ISPDGain;
	g_caliInfo.u32RGain = wbInfo.u16Rgain;
	g_caliInfo.u32BGain = wbInfo.u16Bgain;
}


void stitchCalibration(VI_PIPE ViPipe, CVI_U8 group, CVI_U8 chnSum, CVI_U8 chn)
{
#define CALIBRATION_CHANNLE_NUM  6
	ISP_STITCH_ATTR_S stStitchAttr[CALIBRATION_CHANNLE_NUM];
	ISP_EXPOSURE_ATTR_S stExpAttr;
	ISP_WB_ATTR_S stWBAttr;

	CVI_ISP_GetExposureAttr(ViPipe, &stExpAttr);
	stExpAttr.enOpType = OP_TYPE_MANUAL;
	stExpAttr.stManual.enExpTimeOpType = OP_TYPE_MANUAL;
	stExpAttr.stManual.enAGainOpType = OP_TYPE_MANUAL;
	stExpAttr.stManual.enDGainOpType = OP_TYPE_MANUAL;
	stExpAttr.stManual.enGainType = AE_TYPE_GAIN;
	stExpAttr.stManual.u32ExpTime = g_caliInfo.u32ExpTime;
	stExpAttr.stManual.u32AGain =g_caliInfo.u32AGain;
	stExpAttr.stManual.u32DGain = g_caliInfo.u32DGain;
	stExpAttr.stManual.u32ISPDGain = g_caliInfo.u32ISPDGain;
	CVI_ISP_SetExposureAttr(ViPipe, &stExpAttr);

	CVI_ISP_GetWBAttr(ViPipe, &stWBAttr);
	stWBAttr.enOpType = OP_TYPE_MANUAL;
	stWBAttr.stManual.u16Rgain = g_caliInfo.u32RGain;
	stWBAttr.stManual.u16Gbgain = 1024;
	stWBAttr.stManual.u16Grgain = 1024;
	stWBAttr.stManual.u16Bgain = g_caliInfo.u32BGain;
	CVI_ISP_SetWBAttr(ViPipe, &stWBAttr);

	DELAY_500MS();
	stStitchAttr[ViPipe].u8Group = group;
	stStitchAttr[ViPipe].u8CombChnSum = chnSum;
	stStitchAttr[ViPipe].u8CombChn = chn;
	CVI_ISP_StitchCalibartion(ViPipe, stStitchAttr, CALIBRATION_CHANNLE_NUM);
	for (int i = 0; i < CALIBRATION_CHANNLE_NUM; i++) {
		printf("chn: %d Grp: %d LR: %d RR: %d BR: %d\n",
		i, stStitchAttr[i].u8Group, stStitchAttr[i].u32CalibLumaRatio,
		stStitchAttr[i].u32CalibRGainRatio, stStitchAttr[i].u32CalibBGainRatio);
	}
}

static int run_calibration(char **argv)
{

	CVI_S32 ViPipe = 0, mode = 0, para1 = 0, para2 = 0, para3 = 0;
	sleep(1);
	while (g_bEnableRun) {
		print_usage(argv[0]);

		scanf("%d", &mode);

		if (mode == 0) {
			printf("input :<vipipe>\n:");
			scanf("%d", &ViPipe);
			get_ae_awb_info(ViPipe);
		} else if (mode == 1) {
			printf("input :<vipipe> <group> <chnSum> <chn>\n:");
			scanf("%d %d %d %d", &ViPipe, &para1, &para2, &para3);
			stitchCalibration(ViPipe, para1, para2, para3);
		} else if (mode == 255){
			break;
		}
		DELAY_500MS();
		ViPipe = mode = para1 = para2 = para3 = 0;
	}

	return CVI_SUCCESS;
}


int main(int argc, char **argv)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(argc);
	s32Ret = sys_vi_init();
	if (s32Ret != CVI_SUCCESS) {
		printf("sys vi init failed!\n");
		return s32Ret;
	}

	g_bEnableRun = CVI_TRUE;

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	s32Ret = run_calibration(argv);

	sys_vi_deinit();

	return s32Ret;
}

