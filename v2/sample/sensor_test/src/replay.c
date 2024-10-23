#include "replay.h"

// default is MIPI-CSI Bayer format sensor
VI_DEV_ATTR_S DEV_ATTR_SENSOR_DEFAULT = {
	VI_MODE_MIPI,
	VI_WORK_MODE_1Multiplex,
	VI_SCAN_PROGRESSIVE,
	{-1, -1, -1, -1},
	VI_DATA_SEQ_YUYV,

	{
	/*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
	VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH,
	VI_VSYNC_VALID_SIGNAL, VI_VSYNC_VALID_NEG_HIGH,

	/*hsync_hfb    hsync_act    hsync_hhb*/
	{0,            1920,        0,
	/*vsync0_vhb vsync0_act vsync0_hhb*/
	 0,            1080,        0,
	/*vsync1_vhb vsync1_act vsync1_hhb*/
	 0,            0,            0}
	},
	VI_DATA_TYPE_RGB,
	{1920, 1080},
	{
		WDR_MODE_NONE,
		1080
	},
	.enBayerFormat = BAYER_FORMAT_BG,
};

// default is output YUV420 image
VI_CHN_ATTR_S CHN_ATTR_DEFAULT = {
	{1920, 1080},
	PIXEL_FORMAT_YUV_PLANAR_420,
	DYNAMIC_RANGE_SDR8,
	VIDEO_FORMAT_LINEAR,
	COMPRESS_MODE_NONE,
	CVI_FALSE, CVI_FALSE,
	0,
	{ -1, -1},
	-1
};

 /* snsr_size, action_size, fps, bayer_format, wdr_mode, snsr_mode, data_lane_num, master_or_slave_mode */
ISP_PUB_ATTR_S ISP_PUB_ATTR_DEFAULT =
						{ { 0, 0, 1920, 1080 }, { 1920, 1080 }, 30, BAYER_RGGB, WDR_MODE_NONE, 0, 4, 2};

void _ERR_Exit(void)
{
	CVI_S32 ret;

	SAMPLE_COMM_ISP_Stop(0);

	ret = CVI_VI_DisableChn(0, 0);
	if (ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_DisableChn failed with %#x!\n",
						ret);
		exit(1);
	}
	ret = CVI_VI_StopPipe(0);
	if (ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_StopPipe failed with %#x!\n", ret);
		exit(1);
	}

	ret = CVI_VI_DestroyPipe(0);
	if (ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_DestroyPipe failed with %#x!\n", ret);
		exit(1);
	}
	ret = CVI_VI_DisableDev(0);
	if (ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VI_StopViPipe is fail\n");
		exit(1);
	}

	SAMPLE_COMM_SYS_Exit();
}


static void replay_sys_handle_signal(int nSignal, siginfo_t *si, void *arg)
{
	UNUSED(nSignal);
	UNUSED(si);
	UNUSED(arg);

	_ERR_Exit();

	exit(1);
}

CVI_S32 set_replay_mode_config(void)
{
	CVI_S32 s32Ret;
	VI_DEV_TIMING_ATTR_S stTimingAttr;
	CVI_S32 timing_enable = 0;

	SAMPLE_PRT("Auto trig vsync enable: ");
	scanf("%d", &timing_enable);
	stTimingAttr.bEnable = timing_enable;
	if (stTimingAttr.bEnable) {
		SAMPLE_PRT("update fps: ");
		scanf("%d", &stTimingAttr.s32FrmRate);
	} else {
		stTimingAttr.s32FrmRate = 0;
	}

	s32Ret = CVI_VI_SetPipeFrameSource(0, VI_PIPE_FRAME_SOURCE_USER_FE);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_SetPipeFrameSource failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_SetDevTimingAttr(0, &stTimingAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_SetDevTimingAttr failed with %#x\n", s32Ret);
		return s32Ret;
	}

	SAMPLE_PRT("set usr-pic TEST-PASS\n");

	return s32Ret;
}

CVI_S32 set_replay_pic_config(VIDEO_FRAME_INFO_S *stVideoFrame)
{
	CVI_S32 input_value;
	SAMPLE_PRT("input is RAW [0]\n");
	SAMPLE_PRT("input is YUYV [22]\n");
	SAMPLE_PRT("input is UYVY [23]\n");
	SAMPLE_PRT("input is YVYU [24]\n");
	SAMPLE_PRT("input is VYUY [25]\n");
	SAMPLE_PRT("select format : ");
	scanf("%d", &input_value);
	stVideoFrame->stVFrame.enPixelFormat = input_value;

	if (!stVideoFrame->stVFrame.enPixelFormat) {
		SAMPLE_PRT("select compressmode no [0] yes [1]:");
		scanf("%d", &input_value);
		stVideoFrame->stVFrame.enCompressMode = input_value;

		SAMPLE_PRT("select hdr mode no [0] yes [2]: ");
		scanf("%d", &input_value);
		stVideoFrame->stVFrame.enDynamicRange = input_value;

		SAMPLE_PRT("img width: ");
		scanf("%d", &stVideoFrame->stVFrame.u32Width);
		SAMPLE_PRT("img height: ");
		scanf("%d", &stVideoFrame->stVFrame.u32Height);
		SAMPLE_PRT("crop left offset: ");
		scanf("%hd", &stVideoFrame->stVFrame.s16OffsetLeft);
		SAMPLE_PRT("crop top offset: ");
		scanf("%hd", &stVideoFrame->stVFrame.s16OffsetTop);
		SAMPLE_PRT("crop right offset: ");
		scanf("%hd", &stVideoFrame->stVFrame.s16OffsetRight);
		SAMPLE_PRT("crop bottom offset: ");
		scanf("%hd", &stVideoFrame->stVFrame.s16OffsetBottom);
		SAMPLE_PRT("bayer format BG(0)/GB(1)/GR(2)/RG(3): ");
		scanf("%d", &input_value);
		stVideoFrame->stVFrame.enBayerFormat = input_value;
	} else {
		SAMPLE_PRT("img width: ");
		scanf("%d", &stVideoFrame->stVFrame.u32Width);
		SAMPLE_PRT("img height: ");
		scanf("%d", &stVideoFrame->stVFrame.u32Height);
#ifdef REPLAY_DEBUG
		SAMPLE_PRT("crop left offset: ");
		scanf("%hd", &stVideoFrame->stVFrame.s16OffsetLeft);
		SAMPLE_PRT("crop top offset: ");
		scanf("%hd", &stVideoFrame->stVFrame.s16OffsetTop);
		SAMPLE_PRT("crop right offset: ");
		scanf("%hd", &stVideoFrame->stVFrame.s16OffsetRight);
		SAMPLE_PRT("crop bottom offset: ");
		scanf("%hd", &stVideoFrame->stVFrame.s16OffsetBottom);
#endif
		stVideoFrame->stVFrame.s16OffsetLeft = 0;
		stVideoFrame->stVFrame.s16OffsetTop = 0;
		stVideoFrame->stVFrame.s16OffsetRight = stVideoFrame->stVFrame.u32Width;
		stVideoFrame->stVFrame.s16OffsetBottom = stVideoFrame->stVFrame.u32Height;
	}
#ifdef REPLAY_DEBUG
	u32BlkSize = VI_GetRawBufferSize(stVideoFrame->stVFrame.u32Width,
				 stVideoFrame->stVFrame.u32Height, PIXEL_FORMAT_RGB_BAYER_12BPP,
				 compress_mode, DEFAULT_ALIGN, 0);

	if (vi_usr_blk_le == VB_INVALID_HANDLE) {
		vi_usr_blk_le = CVI_VB_GetBlock(VB_INVALID_POOLID, u32BlkSize);
		if (vi_usr_blk_le == VB_INVALID_HANDLE) {
			fclose(fp_le);
			return CVI_FAILURE;
		}
	}
	u64PhyAddr_le = CVI_VB_Handle2PhysAddr(vi_usr_blk_le);
	puVirAddr_le = CVI_SYS_MmapCache(u64PhyAddr_le, u32BlkSize);
	u32len = fread(puVirAddr_le, u32BlkSize, 1, fp_le);
	if (u32len <= 0) {
		SAMPLE_PRT("fread raw error\n");
		fclose(fp_le);
		return CVI_FAILURE;
	}
	SAMPLE_PRT("load le img to bayer buffer: size(0x%x) paddr(0x%#"PRIx64") vaddr(%p)\n",
		   u32BlkSize, u64PhyAddr_le, puVirAddr_le);

	if (vi_ut_ctx.is_hdr_enable) {
		if (vi_usr_blk_se == VB_INVALID_HANDLE) {
			vi_usr_blk_se = CVI_VB_GetBlock(VB_INVALID_POOLID, u32BlkSize);
		}
		u64PhyAddr_se = CVI_VB_Handle2PhysAddr(vi_usr_blk_se);
		puVirAddr_se = CVI_SYS_MmapCache(u64PhyAddr_se, u32BlkSize);
		u32len = fread(puVirAddr_se, u32BlkSize, 1, fp_se);
		if (u32len <= 0) {
			SAMPLE_PRT("fread raw error\n");
			fclose(fp_se);
			return CVI_FAILURE;
		}
		SAMPLE_PRT("load se img to bayer buffer: size(0x%x) paddr(0x%#"PRIx64") vaddr(%p)\n",
			u32BlkSize, u64PhyAddr_se, puVirAddr_se);
	}

	stVideoFrame->stVFrame.u64PhyAddr[0] = u64PhyAddr_le;
	stVideoFrame->stVFrame.u64PhyAddr[1] = u64PhyAddr_se;
	pstVideoFrame[0] = &stVideoFrame;
	CVI_VI_SendPipeRaw(1, PipeId, pstVideoFrame, 80);

	fclose(fp_le);
	if (vi_ut_ctx.is_hdr_enable) {
		fclose(fp_se);
	}
	SAMPLE_PRT("Send usr-pic TEST-PASS\n");
#endif
	return CVI_SUCCESS;
}

CVI_S32 trig_replay_pic(VI_PIPE PipeId[], const VIDEO_FRAME_INFO_S *pstVideoFrame[])
{
	if (CVI_VI_SendPipeRaw(1, PipeId, pstVideoFrame, 0) == CVI_SUCCESS) {
		SAMPLE_PRT("Trig vsync success\n");
		return CVI_SUCCESS;
	} else {
		SAMPLE_PRT("Trig vsync failed\n");
		return CVI_FAILURE;
	}

}

CVI_S32 replay_vi_start_dev(VIDEO_FRAME_INFO_S *stVideoFrame)
{
	CVI_S32             s32Ret;
	VI_DEV_ATTR_S       stViDevAttr;
	VI_DEV_BIND_PIPE_S  stViDevBindAttr;

	memcpy(&stViDevAttr, &DEV_ATTR_SENSOR_DEFAULT, sizeof(VI_DEV_ATTR_S));

	if (!stVideoFrame->stVFrame.enPixelFormat) {
		stViDevAttr.stSize.u32Width = stVideoFrame->stVFrame.u32Width;
		stViDevAttr.stSize.u32Height = stVideoFrame->stVFrame.u32Height;
		if (stVideoFrame->stVFrame.enDynamicRange) {
			stViDevAttr.stWDRAttr.enWDRMode = WDR_MODE_2To1_LINE;
			stViDevAttr.stWDRAttr.u32CacheLine = stVideoFrame->stVFrame.u32Width;
		}
		// stViDevAttr.enDataSeq
		stViDevAttr.enInputDataType = VI_DATA_TYPE_RGB;
		// stViDevAttr.enIntfMode
		// stViDevAttr.enYuvSceneMode
		stViDevAttr.enBayerFormat = stVideoFrame->stVFrame.enBayerFormat;
	} else {
		stViDevAttr.stSize.u32Width = stVideoFrame->stVFrame.u32Width;
		stViDevAttr.stSize.u32Height = stVideoFrame->stVFrame.u32Height;
		stViDevAttr.stWDRAttr.u32CacheLine = stVideoFrame->stVFrame.u32Width;
		// stViDevAttr.stWDRAttr.enWDRMode
		stViDevAttr.enDataSeq = VI_DATA_SEQ_YUYV;
		stViDevAttr.enInputDataType = VI_DATA_TYPE_YUV;
		stViDevAttr.enIntfMode = VI_MODE_MIPI_YUV422;
		stViDevAttr.enYuvSceneMode = VI_ISP_YUV_SCENE_ISP;
	}
	stViDevAttr.snrFps = 25;
	stViDevBindAttr.PipeId[0] = 0;
	stViDevBindAttr.u32Num = 1;

	s32Ret = CVI_VI_SetDevAttr(0, &stViDevAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_SetDevAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_SetDevBindAttr(0, &stViDevBindAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_SetDevBindAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_EnableDev(0);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_EnableDev failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 replay_startIsp(VIDEO_FRAME_INFO_S *stVideoFrame)
{
	CVI_S32 s32Ret = 0;
	VI_PIPE ViPipe = 0;
	ISP_PUB_ATTR_S stPubAttr;
	ISP_STATISTICS_CFG_S stsCfg = {0};
	ISP_BIND_ATTR_S stBindAttr;

	SAMPLE_COMM_ISP_Aelib_Callback(ViPipe);
	SAMPLE_COMM_ISP_Awblib_Callback(ViPipe);
	#if ENABLE_AF_LIB
	SAMPLE_COMM_ISP_Aflib_Callback(ViPipe);
	#endif

	snprintf(stBindAttr.stAeLib.acLibName, sizeof(CVI_AE_LIB_NAME), "%s", CVI_AE_LIB_NAME);
	stBindAttr.stAeLib.s32Id = ViPipe;
	stBindAttr.sensorId = 0;
	snprintf(stBindAttr.stAwbLib.acLibName, sizeof(CVI_AWB_LIB_NAME), "%s", CVI_AWB_LIB_NAME);
	stBindAttr.stAwbLib.s32Id = ViPipe;
	#if ENABLE_AF_LIB
	snprintf(stBindAttr.stAfLib.acLibName, sizeof(CVI_AF_LIB_NAME), "%s", CVI_AF_LIB_NAME);
	stBindAttr.stAfLib.s32Id = ViPipe;
	#endif
	s32Ret = CVI_ISP_SetBindAttr(ViPipe, &stBindAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Bind Algo failed with %#x!\n", s32Ret);
	}
	s32Ret = CVI_ISP_MemInit(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Init Ext memory failed with %#x!\n", s32Ret);
		return s32Ret;
	}
	// SAMPLE_COMM_ISP_GetIspAttrBySns(pstViInfo->stSnsInfo.enSnsType, &stPubAttr);
	memcpy(&stPubAttr, &ISP_PUB_ATTR_DEFAULT, sizeof(ISP_PUB_ATTR_S));
	stPubAttr.stSnsSize.u32Width = stVideoFrame->stVFrame.u32Width;
	stPubAttr.stSnsSize.u32Height = stVideoFrame->stVFrame.u32Height;
	stPubAttr.stWndRect.u32Width = stVideoFrame->stVFrame.u32Width;
	stPubAttr.stWndRect.u32Height = stVideoFrame->stVFrame.u32Height;
	stPubAttr.stWndRect.s32X = 0;
	stPubAttr.stWndRect.s32Y = 0;
	stPubAttr.enWDRMode = stVideoFrame->stVFrame.enDynamicRange ? WDR_MODE_2To1_LINE : WDR_MODE_NONE;
	stPubAttr.f32FrameRate = 25;
	stPubAttr.enBayer = stVideoFrame->stVFrame.enBayerFormat;
	s32Ret = CVI_ISP_SetPubAttr(ViPipe, &stPubAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SetPubAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}
	CVI_ISP_GetStatisticsConfig(0, &stsCfg);
	stsCfg.stAECfg.stCrop[0].bEnable = 0;
	stsCfg.stAECfg.stCrop[0].u16X = stsCfg.stAECfg.stCrop[0].u16Y = 0;
	stsCfg.stAECfg.stCrop[0].u16W = stPubAttr.stWndRect.u32Width;
	stsCfg.stAECfg.stCrop[0].u16H = stPubAttr.stWndRect.u32Height;
	memset(stsCfg.stAECfg.au8Weight, 1,
		AE_WEIGHT_ZONE_ROW * AE_WEIGHT_ZONE_COLUMN * sizeof(CVI_U8));

#ifdef ARCH_CV183X
	stsCfg.stAECfg.stCrop[1].bEnable = 0;
	stsCfg.stAECfg.stCrop[1].u16X = stsCfg.stAECfg.stCrop[1].u16Y = 0;
	stsCfg.stAECfg.stCrop[1].u16W = stPubAttr.stWndRect.u32Width;
	stsCfg.stAECfg.stCrop[1].u16H = stPubAttr.stWndRect.u32Height;
#endif

	stsCfg.stWBCfg.u16ZoneRow = AWB_ZONE_ORIG_ROW;
	stsCfg.stWBCfg.u16ZoneCol = AWB_ZONE_ORIG_COLUMN;
	stsCfg.stWBCfg.stCrop.bEnable = 0;
	stsCfg.stWBCfg.stCrop.u16X = stsCfg.stWBCfg.stCrop.u16Y = 0;
	stsCfg.stWBCfg.stCrop.u16W = stPubAttr.stWndRect.u32Width;
	stsCfg.stWBCfg.stCrop.u16H = stPubAttr.stWndRect.u32Height;
	stsCfg.stWBCfg.u16BlackLevel = 0;
	stsCfg.stWBCfg.u16WhiteLevel = 4095;
	stsCfg.stFocusCfg.stConfig.bEnable = 1;
	stsCfg.stFocusCfg.stConfig.u8HFltShift = 1;
	stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[0] = 1;
	stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[1] = 2;
	stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[2] = 3;
	stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[3] = 5;
	stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[4] = 10;
	stsCfg.stFocusCfg.stConfig.stRawCfg.PreGammaEn = 0;
	stsCfg.stFocusCfg.stConfig.stPreFltCfg.PreFltEn = 1;
	stsCfg.stFocusCfg.stConfig.u16Hwnd = 17;
	stsCfg.stFocusCfg.stConfig.u16Vwnd = 15;
	stsCfg.stFocusCfg.stConfig.stCrop.bEnable = 0;
	// AF offset and size has some limitation.
	stsCfg.stFocusCfg.stConfig.stCrop.u16X = AF_XOFFSET_MIN;
	stsCfg.stFocusCfg.stConfig.stCrop.u16Y = AF_YOFFSET_MIN;
	stsCfg.stFocusCfg.stConfig.stCrop.u16W = stPubAttr.stWndRect.u32Width - AF_XOFFSET_MIN * 2;
	stsCfg.stFocusCfg.stConfig.stCrop.u16H = stPubAttr.stWndRect.u32Height - AF_YOFFSET_MIN * 2;
	//Horizontal HP0
	stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[0] = 0;
	stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[1] = 0;
	stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[2] = 13;
	stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[3] = 24;
	stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[4] = 0;
	//Horizontal HP1
	stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[0] = 1;
	stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[1] = 2;
	stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[2] = 4;
	stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[3] = 8;
	stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[4] = 0;
	//Vertical HP
	stsCfg.stFocusCfg.stVParam_FIR.s8VFltHpCoeff[0] = 13;
	stsCfg.stFocusCfg.stVParam_FIR.s8VFltHpCoeff[1] = 24;
	stsCfg.stFocusCfg.stVParam_FIR.s8VFltHpCoeff[2] = 0;
	stsCfg.unKey.bit1FEAeGloStat = stsCfg.unKey.bit1FEAeLocStat =
		stsCfg.unKey.bit1AwbStat1 = stsCfg.unKey.bit1AwbStat2 = stsCfg.unKey.bit1FEAfStat = 1;
#ifndef ARCH_CV183X
	//LDG
	stsCfg.stFocusCfg.stConfig.u8ThLow = 0;
	stsCfg.stFocusCfg.stConfig.u8ThHigh = 255;
	stsCfg.stFocusCfg.stConfig.u8GainLow = 30;
	stsCfg.stFocusCfg.stConfig.u8GainHigh = 20;
	stsCfg.stFocusCfg.stConfig.u8SlopLow = 8;
	stsCfg.stFocusCfg.stConfig.u8SlopHigh = 15;
#endif
	s32Ret = CVI_ISP_SetStatisticsConfig(ViPipe, &stsCfg);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("ISP Set Statistic failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_ISP_Init(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("ISP Init failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	/* TODO: AI ISP */

	return CVI_SUCCESS;
}

CVI_S32 replay_createIsp(VIDEO_FRAME_INFO_S *stVideoFrame)
{
	#define USE_OLDAPI_LOADPARA		0

	CVI_S32 s32ViNum = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;

	// s32Ret = SAMPLE_COMM_VI_StartIsp(pstViInfo);
	s32Ret = replay_startIsp(stVideoFrame);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("replay_startIsp failed !\n");
		return CVI_FAILURE;
	}

#if USE_OLDAPI_LOADPARA
	s32Ret = SAMPLE_COMM_BIN_ReadBlockParaFrombin(CVI_BIN_ID_ISP0 + i);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("read block(%d) para fail: %#x!\n", CVI_BIN_ID_ISP0 + i, s32Ret);
	}
#else
	s32Ret = SAMPLE_COMM_BIN_ReadParaFrombin();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("read para fail: %#x,use default para!\n", s32Ret);
	}
	#endif

	s32Ret = SAMPLE_COMM_ISP_Run(s32ViNum);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("ISP_Run failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 replay_startViChn(VIDEO_FRAME_INFO_S *stVideoFrame)
{
	CVI_S32             s32Ret = CVI_SUCCESS;
	VI_PIPE             ViPipe = 0;
	VI_CHN              ViChn = 0;
	VI_CHN_ATTR_S       stChnAttr;

	memcpy(&stChnAttr, &CHN_ATTR_DEFAULT, sizeof(VI_CHN_ATTR_S));
	stChnAttr.enPixelFormat = PIXEL_FORMAT_NV21;
	stChnAttr.stSize.u32Width = stVideoFrame->stVFrame.u32Width;
	stChnAttr.stSize.u32Height = stVideoFrame->stVFrame.u32Height;
	stChnAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
	stChnAttr.enVideoFormat  = VIDEO_FORMAT_LINEAR;
	stChnAttr.enCompressMode = stVideoFrame->stVFrame.enCompressMode;
	/* fill the sensor orientation */
	stChnAttr.bMirror = 0;
	stChnAttr.bFlip = 0;

	s32Ret = CVI_VI_SetChnAttr(ViPipe, ViChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_SetChnAttr failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VI_EnableChn(ViPipe, ViChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_EnableChn failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 replay_vi_isp_enable(VIDEO_FRAME_INFO_S *stVideoFrame)
{
	VI_PIPE ViPipe = 0;
	VI_PIPE_ATTR_S	   stPipeAttr;

	CVI_S32 s32Ret = CVI_SUCCESS;

	/************************************************
	 * step2:  Init VI ISP
	 ************************************************/
	s32Ret = replay_vi_start_dev(stVideoFrame);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("VI_StartDev failed with %#x!\n", s32Ret);
		goto error;
	}

	stPipeAttr.bYuvSkip = CVI_FALSE;
	stPipeAttr.u32MaxW = stVideoFrame->stVFrame.u32Width;
	stPipeAttr.u32MaxH = stVideoFrame->stVFrame.u32Height;
	stPipeAttr.enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stPipeAttr.enBitWidth = DATA_BITWIDTH_12;
	stPipeAttr.stFrameRate.s32SrcFrameRate = -1;
	stPipeAttr.stFrameRate.s32DstFrameRate = -1;
	stPipeAttr.bNrEn = CVI_TRUE;
	stPipeAttr.enCompressMode = stVideoFrame->stVFrame.enCompressMode;
	stPipeAttr.bYuvBypassPath = stVideoFrame->stVFrame.enPixelFormat ? 1 : 0;

	s32Ret = CVI_VI_CreatePipe(ViPipe, &stPipeAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_CreatePipe failed with %#x!\n", s32Ret);
		goto error;
	}

	s32Ret = CVI_VI_StartPipe(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_StartPipe failed with %#x!\n", s32Ret);
		goto error;
	}

	s32Ret = CVI_VI_GetPipeAttr(ViPipe, &stPipeAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_GetPipeAttr failed with %#x!\n", s32Ret);
		goto error;
	}

	s32Ret = replay_createIsp(stVideoFrame);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("VI_CreateIsp failed with %#x!\n", s32Ret);
		goto error;
	}

	s32Ret = replay_startViChn(stVideoFrame);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("VI_StartViChn failed with %#x!\n", s32Ret);
		goto error;
	}

	return s32Ret;
error:
	_ERR_Exit();
	return s32Ret;
}

CVI_S32 get_replay_yuv_pic_addr(VIDEO_FRAME_INFO_S *stVideoFrame, CVI_U64 *u64PhyAddr_le, CVI_U64 *u64PhyAddr_se)
{
	FILE *fp_le = NULL;
	clock_t start_time, end_time;
	double time_spent;
	CVI_CHAR filename_le[64] = "res/2k_ballon_bayer_12_GR.bin";
	CVI_U32 u32BlkSize = COMMON_GetPicBufferSize(stVideoFrame->stVFrame.u32Width,
					stVideoFrame->stVFrame.u32Height, PIXEL_FORMAT_YUYV,
					DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	CVI_U32 pixel_num = stVideoFrame->stVFrame.u32Width * stVideoFrame->stVFrame.u32Height * 2;
	/*YUYV input*/
	uint8_t *buffer = (uint8_t *)malloc(u32BlkSize);
	uint8_t pix_y0 = 0, pix_y1 = 0, pix_u = 0, pix_v = 0,
			temp_y0 = 0, temp_y1 = 0, temp_hy0 = 0, temp_hy1 = 0,
			temp_u = 0, temp_v = 0, temp_hu = 0, temp_hv = 0;
	static VB_BLK vi_usr_blk_y = VB_INVALID_HANDLE, vi_usr_blk_uv = VB_INVALID_HANDLE;
	CVI_U32 y_index = 0, uv_index = 0;
	CVI_VOID *puVirAddr_le = 0, *puVirAddr_se = 0;

	start_time = clock();

	SAMPLE_PRT("YUV filename: ");
	scanf("%s", filename_le);
	fp_le = fopen(filename_le, "r");
	if (fp_le == CVI_NULL) {
		SAMPLE_PRT("open data file error\n");
		return CVI_FAILURE;
	}

	if (buffer == NULL) {
		SAMPLE_PRT("Memory allocation failed");
		fclose(fp_le);
		return CVI_FAILURE;
	}

	if (fread(buffer, u32BlkSize, 1, fp_le) <= 0) {
		if (feof(fp_le)) {
			SAMPLE_PRT("file size error\n");
		} else if (ferror(fp_le)) {
			SAMPLE_PRT("fread file error\n");
		}
		fclose(fp_le);
		return CVI_FAILURE;
	}
	SAMPLE_PRT("load yuv img to buffer: size(0x%x) vaddr(%p)\n",
				u32BlkSize, buffer);

	fclose(fp_le);

	if (vi_usr_blk_y == VB_INVALID_HANDLE) {
		vi_usr_blk_y = CVI_VB_GetBlock(VB_INVALID_POOLID, u32BlkSize);
		if (vi_usr_blk_y == VB_INVALID_HANDLE) {
			free(buffer);
			return CVI_FAILURE;
		}
	}
	if (vi_usr_blk_uv == VB_INVALID_HANDLE) {
		vi_usr_blk_uv = CVI_VB_GetBlock(VB_INVALID_POOLID, u32BlkSize);
		if (vi_usr_blk_uv == VB_INVALID_HANDLE) {
			free(buffer);
			return CVI_FAILURE;
		}
	}
	*u64PhyAddr_le = CVI_VB_Handle2PhysAddr(vi_usr_blk_y);
	puVirAddr_le = CVI_SYS_MmapCache(*u64PhyAddr_le, u32BlkSize);
	*u64PhyAddr_se = CVI_VB_Handle2PhysAddr(vi_usr_blk_uv);
	puVirAddr_se = CVI_SYS_MmapCache(*u64PhyAddr_se, u32BlkSize);

	SAMPLE_PRT("load yuv to VB: pixnum = %d size(0x%x) y addr(0x%#"PRIx64") uv addr(0x%#"PRIx64")\n",
				pixel_num, u32BlkSize, *u64PhyAddr_le, *u64PhyAddr_se);
	/*temp_y0, temp_y1, temp_hy0, temp_hy1, temp_u, temp_v, temp_hu, temp_hv*/
	for (CVI_U32 i = 0; i < pixel_num; i += 4) {
		pix_y0 = (uint8_t)(buffer[i]);
		pix_u = (uint8_t)(buffer[i + 1]);
		pix_y1 = (uint8_t)(buffer[i + 2]);
		pix_v = (uint8_t)(buffer[i + 3]);
#ifdef REPLAY_DEBUG
		if (i == 0) {
			SAMPLE_PRT("y = 0x%x, u = 0x%x, y = 0x%x, v = 0x%x\n", pix_y0, pix_u, pix_y1, pix_v);
		}
#endif
		temp_y0 = pix_y0 >> 2;
		temp_hy0 = (pix_y0 & 0x03) << 2;

		temp_u = pix_u >> 2;
		temp_hu = (pix_u & 0x03) << 2;

		temp_y1 = pix_y1 >> 2;
		temp_hy1 = (pix_y1 & 0x03) << 6;

		temp_v = pix_v >> 2;
		temp_hv = (pix_v & 0x03) << 6;

		((uint8_t *)puVirAddr_le)[y_index++] = temp_y0;
		((uint8_t *)puVirAddr_le)[y_index++] = temp_y1;
		((uint8_t *)puVirAddr_le)[y_index++] = temp_hy0 | temp_hy1;
		((uint8_t *)puVirAddr_se)[uv_index++] = temp_u;
		((uint8_t *)puVirAddr_se)[uv_index++] = temp_v;
		((uint8_t *)puVirAddr_se)[uv_index++] = temp_hu | temp_hv;
#ifdef REPLAY_DEBUG
		if (i == 0) {
			SAMPLE_PRT("y[%d] = 0x%x, y[%d] = 0x%x, y[%d] = 0x%x, uv[%d] = 0x%x, uv[%d] = 0x%x, uv[%d] = 0x%x\n",
						y_index - 3, ((uint8_t *)puVirAddr_le)[y_index - 3],
						y_index - 2, ((uint8_t *)puVirAddr_le)[y_index - 2],
						y_index - 1, ((uint8_t *)puVirAddr_le)[y_index - 1],
						uv_index - 3, ((uint8_t *)puVirAddr_se)[uv_index - 3],
						uv_index - 2, ((uint8_t *)puVirAddr_se)[uv_index - 2],
						uv_index - 1, ((uint8_t *)puVirAddr_se)[uv_index - 1]);
		}
#endif
	}

	SAMPLE_PRT("YUYV8BIT -> Y12BITUV12BIT success\n");
	free(buffer);
	buffer = NULL;

	end_time = clock();
	time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
	SAMPLE_PRT("Function execution time: %f seconds\n", time_spent);

	return CVI_SUCCESS;
}

CVI_S32 get_replay_pic_addr(VIDEO_FRAME_INFO_S *stVideoFrame, CVI_U64 *u64PhyAddr_le, CVI_U64 *u64PhyAddr_se)
{
	FILE *fp_le = NULL, *fp_se = NULL;
	CVI_CHAR filename_le[64] = "res/2k_ballon_bayer_12_GR.bin";
	CVI_CHAR filename_se[64] = "res/2k_ballon_bayer_12_GR.bin";
	CVI_U32 u32BlkSize = 0, u32len = 0;
	CVI_VOID *puVirAddr_le = 0, *puVirAddr_se = 0;

	static VB_BLK vi_usr_blk_le = VB_INVALID_HANDLE, vi_usr_blk_se = VB_INVALID_HANDLE;

	if (!stVideoFrame->stVFrame.enPixelFormat) {
		SAMPLE_PRT("LE filename: ");
		scanf("%s", filename_le);
		fp_le = fopen(filename_le, "r");
		if (fp_le == CVI_NULL) {
			SAMPLE_PRT("open data file error\n");
			return CVI_FAILURE;
		}

		if (stVideoFrame->stVFrame.enDynamicRange == DYNAMIC_RANGE_HDR10) {
			SAMPLE_PRT("SE filename: ");
			scanf("%s", filename_se);
			fp_se = fopen(filename_se, "r");
			if (fp_se == CVI_NULL) {
				SAMPLE_PRT("open data file error\n");
				return CVI_FAILURE;
			}
		}

		u32BlkSize = VI_GetRawBufferSize(stVideoFrame->stVFrame.u32Width,
					 stVideoFrame->stVFrame.u32Height, PIXEL_FORMAT_RGB_BAYER_12BPP,
					 stVideoFrame->stVFrame.enCompressMode, DEFAULT_ALIGN, 0);
	} else {
		if (get_replay_yuv_pic_addr(stVideoFrame, u64PhyAddr_le, u64PhyAddr_se) == CVI_SUCCESS) {
			SAMPLE_PRT("Get pic addr PASS\n");
			return CVI_SUCCESS;
		} else {
			SAMPLE_PRT("get YUV data addr failed!\n");
			return CVI_FAILURE;
		}
#ifdef REPLAY_DEBUG
		SAMPLE_PRT("YUV filename Y: ");
		scanf("%s", filename_le);
		fp_le = fopen(filename_le, "r");
		if (fp_le == CVI_NULL) {
			SAMPLE_PRT("open data file error\n");
			return CVI_FAILURE;
		}

		SAMPLE_PRT("YUV filename UV: ");
		scanf("%s", filename_se);
		fp_se = fopen(filename_se, "r");
		if (fp_se == CVI_NULL) {
			SAMPLE_PRT("open data file error\n");
			return CVI_FAILURE;
		}

		u32BlkSize = COMMON_GetPicBufferSize(stVideoFrame->stVFrame.u32Width,
					stVideoFrame->stVFrame.u32Height, PIXEL_FORMAT_YUYV,
					DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
#endif
	}

	if (vi_usr_blk_le == VB_INVALID_HANDLE) {
		vi_usr_blk_le = CVI_VB_GetBlock(VB_INVALID_POOLID, u32BlkSize);
		if (vi_usr_blk_le == VB_INVALID_HANDLE) {
			fclose(fp_le);
			return CVI_FAILURE;
		}
	}

	*u64PhyAddr_le = CVI_VB_Handle2PhysAddr(vi_usr_blk_le);
	puVirAddr_le = CVI_SYS_MmapCache(*u64PhyAddr_le, u32BlkSize);
	u32len = fread(puVirAddr_le, u32BlkSize, 1, fp_le);
	if (u32len <= 0) {
		if (feof(fp_le)) {
			SAMPLE_PRT("file size error\n");
		} else if (ferror(fp_le)) {
			SAMPLE_PRT("fread file error\n");
		}
		fclose(fp_le);
		return CVI_FAILURE;
	}
	SAMPLE_PRT("load le img to bayer buffer: size(0x%x) paddr(0x%#"PRIx64") vaddr(%p)\n",
				u32BlkSize, *u64PhyAddr_le, puVirAddr_le);

	fclose(fp_le);

	if ((!stVideoFrame->stVFrame.enPixelFormat && stVideoFrame->stVFrame.enDynamicRange ==
					DYNAMIC_RANGE_HDR10) || stVideoFrame->stVFrame.enPixelFormat) {
		if (vi_usr_blk_se == VB_INVALID_HANDLE) {
			vi_usr_blk_se = CVI_VB_GetBlock(VB_INVALID_POOLID, u32BlkSize);
		}
		*u64PhyAddr_se = CVI_VB_Handle2PhysAddr(vi_usr_blk_se);
		puVirAddr_se = CVI_SYS_MmapCache(*u64PhyAddr_se, u32BlkSize);
		u32len = fread(puVirAddr_se, u32BlkSize, 1, fp_se);
		if (u32len <= 0) {
			if (feof(fp_se)) {
				SAMPLE_PRT("file size error\n");
			} else if (ferror(fp_se)) {
				SAMPLE_PRT("fread file error\n");
			}
			fclose(fp_se);
			return CVI_FAILURE;
		}
		SAMPLE_PRT("load se img to bayer buffer: size(0x%x) paddr(0x%#"PRIx64") vaddr(%p)\n",
					u32BlkSize, *u64PhyAddr_se, puVirAddr_se);

		fclose(fp_se);
	}

	if (!stVideoFrame->stVFrame.enPixelFormat &&
			stVideoFrame->stVFrame.enDynamicRange == DYNAMIC_RANGE_HDR10) {
		fclose(fp_se);
	}

	SAMPLE_PRT("Get pic addr PASS\n");
	return CVI_SUCCESS;
}

CVI_S32 replay_sys_init(VIDEO_FRAME_INFO_S *stVideoFrame)
{
	CVI_S32		s32Ret;
	VB_CONFIG_S	stVbConf;
	CVI_U32		u32BlkSize, u32BlkRotSize;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 0;

	if (!stVideoFrame->stVFrame.enPixelFormat) {
		u32BlkSize = COMMON_GetPicBufferSize(stVideoFrame->stVFrame.u32Width, stVideoFrame->stVFrame.u32Height,
				PIXEL_FORMAT_NV21, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkRotSize = COMMON_GetPicBufferSize(stVideoFrame->stVFrame.u32Width, stVideoFrame->stVFrame.u32Height,
				PIXEL_FORMAT_NV21, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkSize = u32BlkSize > u32BlkRotSize ? u32BlkSize : u32BlkRotSize;
	} else {
		u32BlkSize = COMMON_GetPicBufferSize(stVideoFrame->stVFrame.u32Width, stVideoFrame->stVFrame.u32Height,
				PIXEL_FORMAT_YUYV, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkRotSize = COMMON_GetPicBufferSize(stVideoFrame->stVFrame.u32Width, stVideoFrame->stVFrame.u32Height,
				PIXEL_FORMAT_YUYV, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkSize = u32BlkSize > u32BlkRotSize ? u32BlkSize : u32BlkRotSize;
	}

	stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkCnt = 5;
	stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].enRemapMode = VB_REMAP_MODE_CACHED;
	stVbConf.u32MaxPoolCnt++;

	if (stVbConf.u32MaxPoolCnt == 1) {
		stVbConf.astCommPool[0].u32BlkCnt += 2;
	}

	/************************************************
	 * step3:  Init modules
	 ************************************************/
	struct sigaction sa = {};

	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = replay_sys_handle_signal;
	sa.sa_flags = SA_SIGINFO|SA_RESETHAND;	// Reset signal handler to system default after signal triggered
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("system init failed with %#x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 replay_vi_init(void)
{
	CVI_S32 op;
	VIDEO_FRAME_INFO_S stVideoFrame;
	const VIDEO_FRAME_INFO_S *pstVideoFrame[1];
	VI_PIPE PipeId[] = {0};
	CVI_U64 u64PhyAddr_le = 0, u64PhyAddr_se = 0;

	memset(&stVideoFrame, 0, sizeof(stVideoFrame));
	pstVideoFrame[0] = NULL;

	if (set_replay_mode_config() != CVI_SUCCESS) {
		SAMPLE_PRT("set_replay_mode_config failed\n");
	}

	if (set_replay_pic_config(&stVideoFrame) != CVI_SUCCESS) {
		SAMPLE_PRT("set_replay_pic_config failed\n");
	}

	if (replay_sys_init(&stVideoFrame) != CVI_SUCCESS) {
		SAMPLE_PRT("replay_sys_init failed\n");
	}

	if (get_replay_pic_addr(&stVideoFrame, &u64PhyAddr_le, &u64PhyAddr_se) != CVI_SUCCESS) {
		SAMPLE_PRT("get_replay_pic_addr failed\n");
	}

	stVideoFrame.stVFrame.u64PhyAddr[0] = u64PhyAddr_le;
	stVideoFrame.stVFrame.u64PhyAddr[1] = u64PhyAddr_se;
	pstVideoFrame[0] = &stVideoFrame;

	if (trig_replay_pic(PipeId, pstVideoFrame) != CVI_SUCCESS) {
		SAMPLE_PRT("trig_replay_pic 1st failed\n");
	}

	if (replay_vi_isp_enable(&stVideoFrame) != CVI_SUCCESS) {
		SAMPLE_PRT("replay_vi_isp_enable failed\n");
	}

	do {
		SAMPLE_PRT("---Basic------------------------------------------------\n");
		SAMPLE_PRT("1: trig vsync\n");
		SAMPLE_PRT("2: dump vi yuv frame\n");
#ifdef REPLAY_DEBUG
		SAMPLE_PRT("253: get vi yuv for debug\n");
#endif
		SAMPLE_PRT("254: cat vi & vi_dbg\n");
		SAMPLE_PRT("255: exit\n");
		scanf("%d", &op);

		switch (op) {
		case 1:
			if (trig_replay_pic(PipeId, pstVideoFrame) != CVI_SUCCESS) {
				SAMPLE_PRT("op(%d) failed!\n", op);
				return CVI_SUCCESS;
			}
			break;
		case 2:
			if (sensor_dump_yuv() != CVI_SUCCESS) {
				SAMPLE_PRT("op(%d) failed!\n", op);
				return CVI_SUCCESS;
			}
			break;
#ifdef REPLAY_DEBUG
		case 253:
			if (get_vi_yuv_debug() != CVI_SUCCESS) {
				SAMPLE_PRT("op(%d) failed!\n", op);
				return CVI_SUCCESS;
			}
			break;
#endif
		case 254:
			system("cat /proc/soph/vi");
			system("cat /proc/soph/vi_dbg");
			break;
		default:
			break;
		}
	} while (op != 255);

	_ERR_Exit();

	return CVI_SUCCESS;
}