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

#include "cvi_buffer.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_comm_isp.h"

#include "sample_comm.h"


#define NONE	"\033[m"
#define RED	"\033[0;32;31m"
#define GREEN	"\033[0;32;32m"

CVI_S32 VIP_FPGA_1822_TEST(void)
{
	SAMPLE_INI_CFG_S	   stIniCfg = {0};

	COMPRESS_MODE_E    enCompressMode   = COMPRESS_MODE_NONE;
	SAMPLE_VI_CONFIG_S stViConfig;
	CVI_S32            s32WorkSnsId = 0;

	VB_CONFIG_S	stVbConf;
	PIC_SIZE_E	enPicSize;
	CVI_U32		u32BlkSize;
	SIZE_S		stSize;
	CVI_S32		s32Ret = CVI_SUCCESS;
	LOG_LEVEL_CONF_S log_conf;
	int		op = 0;

	stIniCfg = (SAMPLE_INI_CFG_S) {
		.enSource  = VI_PIPE_FRAME_SOURCE_DEV,
		.devNum    = 1,
		.enSnsType[0] = SONY_IMX327_MIPI_1M_30FPS_10BIT,
		.enWDRMode[0] = WDR_MODE_NONE,
		.MipiDev[0]   = 0xff,
		.s32BusId[0]  = 3,
		.u8UseMultiSns = 0,
	};

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
	/************************************************
	 * step1:  Config VI
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_IniToViCfg(&stIniCfg, &stViConfig);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

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
	 * step3:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt		= 1;

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height, PIXEL_FORMAT_NV21,
						DATA_BITWIDTH_8, enCompressMode, 32);
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 4;
	SAMPLE_PRT("common pool[0] (%d,%d) BlkSize %d\n", stSize.u32Width, stSize.u32Height, u32BlkSize);

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("system init failed with %#x\n", s32Ret);
		goto error;
	}


	VI_DEV ViDev = 0;
	VI_PIPE ViPipe = 0;
	VI_PIPE_ATTR_S	   stPipeAttr;
	VI_DEV_ATTR_S      stViDevAttr;
	CVI_S32 s32ViNum;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	/************************************************
	 * step2:  Init VI ISP
	 ************************************************/
	SAMPLE_COMM_VI_GetDevAttrBySns(stIniCfg.enSnsType[0], &stViDevAttr);
	stViDevAttr.stWDRAttr.enWDRMode = stIniCfg.enWDRMode[0];

	s32Ret = SAMPLE_COMM_VI_StartSensor(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "system start sensor failed with %#x\n", s32Ret);
		goto error;
	}

	s32Ret = CVI_VI_SetDevAttr(ViDev, &stViDevAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_SetDevAttr failed with %#x!\n", s32Ret);
		goto error;
	}

	s32Ret = CVI_VI_EnableDev(ViDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_EnableDev failed with %#x!\n", s32Ret);
		goto error;
	}

	s32Ret = SAMPLE_COMM_VI_StartDev(&stViConfig.astViInfo[ViDev]);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "VI_StartDev failed with %#x!\n", s32Ret);
		goto error;
	}

	s32Ret = SAMPLE_COMM_VI_StartMIPI(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "system start MIPI failed with %#x\n", s32Ret);
		goto error;
	}

	stPipeAttr.bYuvSkip = CVI_FALSE;
	stPipeAttr.u32MaxW = stSize.u32Width;
	stPipeAttr.u32MaxH = stSize.u32Height;
	stPipeAttr.enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stPipeAttr.enBitWidth = DATA_BITWIDTH_12;
	stPipeAttr.stFrameRate.s32SrcFrameRate = -1;
	stPipeAttr.stFrameRate.s32DstFrameRate = -1;
	stPipeAttr.bNrEn = CVI_TRUE;
	stPipeAttr.bYuvBypassPath = CVI_FALSE;

	s32Ret = CVI_VI_CreatePipe(ViPipe, &stPipeAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_CreatePipe failed with %#x!\n", s32Ret);
		goto error;
	}

	s32Ret = CVI_VI_StartPipe(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_StartPipe failed with %#x!\n", s32Ret);
		goto error;
	}

	for (s32WorkSnsId = 0; s32WorkSnsId < stViConfig.s32WorkingViNum; s32WorkSnsId++) {
		s32ViNum  = stViConfig.as32WorkingViId[s32WorkSnsId];
		pstViInfo = &stViConfig.astViInfo[s32ViNum];

		s32Ret = SAMPLE_COMM_VI_StartIsp(pstViInfo);

		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VI_StartIsp failed !\n");
			goto error;
		}
	}

	//_CVI_VI_CFG_CTRL_TEST();

	s32Ret = SAMPLE_COMM_VI_StartViChn(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "VI_StartViChn failed with %#x!\n", s32Ret);
		goto error;
	}


	do {
		SAMPLE_PRT("---Basic------------------------------------------------\n");
		SAMPLE_PRT("255: exit\n");
		scanf("%d", &op);
	} while (op != 255);


	//SAMPLE_COMM_VI_DestroyIsp(&stViConfig);

	s32Ret = CVI_VI_DisableChn(0, 0);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_DisableChn failed with %#x!\n", s32Ret);
		goto error;
	}

	s32Ret = CVI_VI_StopPipe(0);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_StopPipe failed with %#x!\n", s32Ret);
		goto error;
	}

	s32Ret = CVI_VI_DestroyPipe(0);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_DestroyPipe failed with %#x!\n", s32Ret);
		goto error;
	}

	s32Ret	= CVI_VI_DisableDev(0);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_DisableDev failed with %#x!\n", s32Ret);
		goto error;
	}

error:
	SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}

