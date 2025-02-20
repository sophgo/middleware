#include "vi_ut.h"
#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <inttypes.h>

int _vi_set_hdr(void)
{
	S_CTRL_VALUE(vi_ut_ctx.fd, vi_ut_ctx.is_hdr_enable, VI_IOCTL_HDR);
}

int _vi_set_be_online(void)
{
	S_CTRL_VALUE(vi_ut_ctx.fd, vi_ut_ctx.is_be_online, VI_IOCTL_BE_ONLINE);
}

int _vi_set_post_online(void)
{
	S_CTRL_VALUE(vi_ut_ctx.fd, vi_ut_ctx.is_post_online, VI_IOCTL_ONLINE);
}

int _vi_set_snr_info(uint8_t dev)
{
	struct cvi_isp_snr_info cfg;
	CVI_U8 i = 0;

	cfg.raw_num = dev;
	cfg.color_mode = BAYER_RGGB;
	cfg.snr_fmt.frm_num = vi_ut_ctx.is_hdr_enable ? 2 : 1;
	for (i = 0; i < cfg.snr_fmt.frm_num; i++) {
		cfg.snr_fmt.img_size[i].max_width = vi_ut_ctx.stSize[dev].u32Width;
		cfg.snr_fmt.img_size[i].max_height = vi_ut_ctx.stSize[dev].u32Height;
		cfg.snr_fmt.img_size[i].start_x = 0;
		cfg.snr_fmt.img_size[i].start_y = 0;
		cfg.snr_fmt.img_size[i].active_w = vi_ut_ctx.stSize[dev].u32Width;
		cfg.snr_fmt.img_size[i].active_h = vi_ut_ctx.stSize[dev].u32Height;
		cfg.snr_fmt.img_size[i].width = vi_ut_ctx.stSize[dev].u32Width;
		cfg.snr_fmt.img_size[i].height = vi_ut_ctx.stSize[dev].u32Height;
	}

	S_CTRL_PTR(vi_ut_ctx.fd, &cfg, VI_IOCTL_SET_SNR_INFO);
}

int _vi_set_3dnr(bool is_on)
{
	S_CTRL_VALUE(vi_ut_ctx.fd, is_on, VI_IOCTL_3DNR);
}

int _vi_set_patgen(bool enable)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	pid_t status;
	char cmd[128] = {0};

	sprintf(cmd, "echo %d,%d,%d,%d,%d,%d > /sys/module/%s/parameters/csi_patgen_en",
			enable, enable, enable, enable, enable, enable, CHIP_TYPE);
	VI_UT_PRT("%s\n", cmd);
	status = system(cmd);

	if (status == -1) {
		VI_UT_PRT("system call error\n");
		s32Ret =  CVI_FAILURE;
	} else {
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 0) {
				VI_UT_PRT("run shell script successfully.\n");
			} else {
				VI_UT_PRT("run shell script fail, script exit code: %d\n",
						WEXITSTATUS(status));
				s32Ret =  CVI_FAILURE;
			}
		} else {
			VI_UT_PRT("exit status = [%d]\n", WEXITSTATUS(status));
		}
	}

	return s32Ret;
}

static void _vi_ut_sys_handle_signal(int nSignal, siginfo_t *si, void *arg)
{
	UNUSED(nSignal);
	UNUSED(si);
	UNUSED(arg);

	if (stViConfig.s32WorkingViNum != 0) {
		SAMPLE_COMM_VI_DestroyIsp(&stViConfig);
		SAMPLE_COMM_VI_DestroyVi(&stViConfig);
	}
	SAMPLE_COMM_SYS_Exit();
	exit(1);
}

long vi_ut_diff_in_us(struct timespec t1, struct timespec t2)
{
	struct timespec diff;

	if (t2.tv_nsec - t1.tv_nsec < 0) {
		diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
	} else {
		diff.tv_sec  = t2.tv_sec - t1.tv_sec;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
	}
	return (diff.tv_sec * 1000000.0 + diff.tv_nsec / 1000.0);
}

CVI_S32 vi_ut_set_rawdump_crop(CVI_U32 dev, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VI_DEV_ATTR_S  dev_attr;
	CVI_U32 j, crop_x, crop_y, crop_w, crop_h;

	CVI_VI_GetDevAttr(dev, &dev_attr);
	VI_UT_PRT("dev(%d) input size : w(%d), h(%d)\n",
		dev, dev_attr.stSize.u32Width, dev_attr.stSize.u32Height);

	if (vi_ut_ctx.is_test_mode && vi_ut_ctx.is_crop_raw) {
		crop_x = 0;
		crop_y = 0;
		crop_w = 960;
		crop_h = 540;
	} else {
		VI_UT_PRT("To set rawdump crop x: ");
		scanf("%d", &crop_x);
		VI_UT_PRT("To set rawdump crop y: ");
		scanf("%d", &crop_y);
		VI_UT_PRT("To set rawdump crop w: ");
		scanf("%d", &crop_w);
		VI_UT_PRT("To set rawdump crop h: ");
		scanf("%d", &crop_h);
	}

	if (crop_x % 2 || crop_y % 2 || crop_w % 2 || crop_h % 2) {
		VI_UT_PRT("crop_x(%d)_y(%d)_w(%d)_h(%d) must be multiple of 2.\n",
			crop_x, crop_y, crop_w, crop_h);
		return CVI_FAILURE;
	}

	if ((crop_x + crop_w) > dev_attr.stSize.u32Width ||
		(crop_y + crop_h) > dev_attr.stSize.u32Height) {
		VI_UT_PRT("crop_x(%d)+w(%d) or y(%d)+h(%d) is bigger than dev_w(%d)_h(%d)\n",
			crop_x, crop_w, crop_y, crop_h, dev_attr.stSize.u32Width, dev_attr.stSize.u32Height);
		return CVI_FAILURE;
	}

	for (j = 0; j < 2; j++) {
		pstVideoFrame[j].stVFrame.s16OffsetTop = crop_y;
		pstVideoFrame[j].stVFrame.s16OffsetBottom = dev_attr.stSize.u32Height - crop_y - crop_h;
		pstVideoFrame[j].stVFrame.s16OffsetLeft = crop_x;
		pstVideoFrame[j].stVFrame.s16OffsetRight = dev_attr.stSize.u32Width - crop_x - crop_w;
	}

	return CVI_SUCCESS;
}

CVI_S32 vi_ut_plat_sys_init(void)
{
	CVI_S32		s32Ret;
	PIC_SIZE_E	enPicSize;
	CVI_U32		is_dpcm_on = 0;
	CVI_S32		i;
	VB_CONFIG_S	stVbConf;
	CVI_U32		u32BlkSize, u32BlkRotSize;
	SAMPLE_INI_CFG_S stIniCfg = {0};

	stIniCfg = (SAMPLE_INI_CFG_S) {
		.enSource  = VI_PIPE_FRAME_SOURCE_DEV,
		.devNum    = 1,
		.enSnsType[0] = SONY_IMX327_MIPI_2M_30FPS_12BIT,
		.enWDRMode[0] = WDR_MODE_NONE,
		.s32BusId[0]  = 3,
		.s32SnsI2cAddr[0] = -1,
		.MipiDev[0]   = 0xFF,
		.u8UseMultiSns = 0,
	};

	vi_ut_ctx.enCompressMode = COMPRESS_MODE_NONE;
	vi_ut_ctx.enPixelFormat  = VI_PIXEL_FORMAT;
	vi_ut_ctx.u32Align       = DEFAULT_ALIGN;

	if (!vi_ut_ctx.is_test_mode) {
		if (vi_ut_ctx.is_get_dpcm_mode) {
			VI_UT_PRT("set dpcm on ? (0:no, 1:yes): ");
			scanf("%d", &is_dpcm_on);
		}
	} else {
		is_dpcm_on = vi_ut_ctx.is_dpcm_on;
	}

	vi_ut_ctx.enCompressMode = (is_dpcm_on == 0) ? COMPRESS_MODE_NONE : COMPRESS_MODE_TILE;
	VI_UT_PRT("vi_ut_ctx.enCompressMode (%d)\n", vi_ut_ctx.enCompressMode);

	// Get config from ini if found.
	s32Ret = SAMPLE_COMM_VI_ParseIni(&stIniCfg);
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("Parse fail\n");
	} else {
		VI_UT_PRT("Parse complete\n");
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
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 0;

	for (i = 0; i < stViConfig.s32WorkingViNum; i++) {
		bool createNewPool = true;

		s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stIniCfg.enSnsType[i], &enPicSize);
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("SAMPLE_COMM_VI_GetSizeBySensor failed with %#x\n", s32Ret);
			return s32Ret;
		}

		s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &vi_ut_ctx.stSize[i]);
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
			return s32Ret;
		}

		u32BlkSize = COMMON_GetPicBufferSize(vi_ut_ctx.stSize[i].u32Width, vi_ut_ctx.stSize[i].u32Height,
				SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkRotSize = COMMON_GetPicBufferSize(vi_ut_ctx.stSize[i].u32Height, vi_ut_ctx.stSize[i].u32Width,
				SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkSize = u32BlkSize > u32BlkRotSize ? u32BlkSize : u32BlkRotSize;

		for (CVI_U32 j = 0; j < stVbConf.u32MaxPoolCnt; j++) {
			if (stVbConf.astCommPool[j].u32BlkSize == u32BlkSize) {
				stVbConf.astCommPool[j].u32BlkCnt += 3;
				createNewPool = false;
				break;
			}
		}
		if (createNewPool) {
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkSize = u32BlkSize;
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkCnt = 4;
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].enRemapMode = VB_REMAP_MODE_CACHED;
			stVbConf.u32MaxPoolCnt++;
		}
	}

	if (stVbConf.u32MaxPoolCnt == 1) {
		stVbConf.astCommPool[0].u32BlkCnt += 2;
	}

	/************************************************
	 * step3:  Init modules
	 ************************************************/
	struct sigaction sa = {};

	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = _vi_ut_sys_handle_signal;
	sa.sa_flags = SA_SIGINFO|SA_RESETHAND;	// Reset signal handler to system default after signal triggered
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("system init failed with %#x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 vi_ut_vpss_config_online_mode(void)
{
	/************************************************
	 * Config and init VPSS
	 ************************************************/
	VPSS_GRP	   VpssGrp	  = VPSS_ONLINE_GRP_0;
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_CHN	   VpssChn	  = VPSS_CHN0;
	CVI_BOOL	   abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM] = {0};
	CVI_S32 s32Ret = CVI_SUCCESS;

	// snr0
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = SAMPLE_PIXEL_FORMAT;
	stVpssGrpAttr.u32MaxW			     = vi_ut_ctx.stSize[0].u32Width;
	stVpssGrpAttr.u32MaxH			     = vi_ut_ctx.stSize[0].u32Height;

	astVpssChnAttr[VpssChn].u32Width		    = 1280;
	astVpssChnAttr[VpssChn].u32Height		    = 720;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = SAMPLE_PIXEL_FORMAT;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 0;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_NONE;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 vi_ut_sys_config_online_mode(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_VPSS_MODE_S	stVIVPSSMode;
	int i = 0;

	/************************************************
	 * Config vpss online mode
	 ************************************************/
	for (i = 0; i < VI_MAX_PIPE_NUM; ++i) {
		stVIVPSSMode.aenMode[i] = VI_OFFLINE_VPSS_ONLINE;
	}

	s32Ret = CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("CVI_SYS_SetVIVPSSMode failed with %#x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 vi_ut_plat_vi_startchn(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32             i;
	CVI_S32             s32Ret = CVI_SUCCESS;
	VI_PIPE             ViPipe = 0;
	VI_CHN              ViChn = 0;
	VI_DEV              ViDev = 0;
	CVI_U32             u32SnsId = 0;
	VI_CHN_ATTR_S       stChnAttr;
	ISP_SNS_OBJ_S       *pstSnsObj;

	for (i = 0; i < pstViConfig->s32WorkingViNum; i++) {
		if (i < VI_MAX_DEV_NUM) {
			ViPipe	    = pstViConfig->astViInfo[i].stPipeInfo.aPipe[0];
			ViChn	    = pstViConfig->astViInfo[i].stChnInfo.ViChn;
			ViDev	    = pstViConfig->astViInfo[i].stDevInfo.ViDev;
			u32SnsId    = pstViConfig->astViInfo[i].stSnsInfo.s32SnsId;
			pstSnsObj   = (ISP_SNS_OBJ_S *)SAMPLE_COMM_ISP_GetSnsObj(u32SnsId);

			SAMPLE_COMM_VI_GetChnAttrBySns(pstViConfig->astViInfo[i].stSnsInfo.enSnsType, &stChnAttr);
			stChnAttr.enDynamicRange = pstViConfig->astViInfo[i].stChnInfo.enDynamicRange;
			stChnAttr.enVideoFormat  = pstViConfig->astViInfo[i].stChnInfo.enVideoFormat;
			stChnAttr.enCompressMode = pstViConfig->astViInfo[i].stChnInfo.enCompressMode;
			stChnAttr.enPixelFormat = pstViConfig->astViInfo[i].stChnInfo.enPixFormat;
			stChnAttr.u32Depth = 1;
			/* fill the sensor orientation */
			if (pstViConfig->astViInfo[i].stSnsInfo.u8Orien <= 3) {
				stChnAttr.bMirror = pstViConfig->astViInfo[i].stSnsInfo.u8Orien & 0x1;
				stChnAttr.bFlip = pstViConfig->astViInfo[i].stSnsInfo.u8Orien & 0x2;
			}

			s32Ret = CVI_VI_SetChnAttr(ViPipe, ViChn, &stChnAttr);
			if (s32Ret != CVI_SUCCESS) {
				VI_UT_PRT("CVI_VI_SetChnAttr failed with %#x!\n", s32Ret);
				return CVI_FAILURE;
			}

			if (pstSnsObj && pstSnsObj->pfnMirrorFlip)
				CVI_VI_RegChnFlipMirrorCallBack(ViPipe, ViDev, (void *)pstSnsObj->pfnMirrorFlip);

			s32Ret = CVI_VI_EnableChn(ViPipe, ViChn);
			if (s32Ret != CVI_SUCCESS) {
				VI_UT_PRT("CVI_VI_EnableChn failed with %#x!\n", s32Ret);
				return CVI_FAILURE;
			}
		}
	}

	return s32Ret;
}

CVI_S32 vi_ut_plat_vi_init(void)
{
	CVI_S32 s32Ret;
	VI_PIPE ViPipe = 0;
	VI_PIPE_ATTR_S stPipeAttr;
	CVI_S32 i = 0, j = 0;
	if (g_is_patgen) {stViConfig.astViInfo[0].stSnsInfo.MipiDev = 0;}
	vi_ut_ctx.fd = CVI_VI_GetPipeFd(0);

	stPipeAttr.enCompressMode = vi_ut_ctx.enCompressMode;
	VI_UT_PRT("stPipeAttr.enCompressMode (%d)\n", stPipeAttr.enCompressMode);

	if (vi_ut_ctx.is_enable_sensor) {
		s32Ret = SAMPLE_COMM_VI_StartSensor(&stViConfig);
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("system start sensor failed with %#x\n", s32Ret);
			goto error;
		}
	}

	for (i = 0; i < stViConfig.s32WorkingViNum; i++) {
		s32Ret = SAMPLE_COMM_VI_StartDev(&stViConfig.astViInfo[i]);
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("VI_StartDev failed with %#x!\n", s32Ret);
			goto error;
		}
	}

	if (vi_ut_ctx.is_enable_sensor) {
		s32Ret = SAMPLE_COMM_VI_StartMIPI(&stViConfig);
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("system start MIPI failed with %#x\n", s32Ret);
			goto error;
		}

		s32Ret = SAMPLE_COMM_VI_SensorProbe(&stViConfig);
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("system sensor probe failed with %#x\n", s32Ret);
			goto error;
		}
	}

	for (i = 0; i < stViConfig.s32WorkingViNum; i++) {
		CVI_S32 s32DevNum;
		SAMPLE_VI_INFO_S *pstViInfo = NULL;

		s32DevNum = stViConfig.as32WorkingViId[i];
		pstViInfo = &stViConfig.astViInfo[s32DevNum];

		if ((pstViInfo->stSnsInfo.enSnsType == PICO640_THERMAL_479P) ||
			(pstViInfo->stSnsInfo.enSnsType == TECHPOINT_TP2850_MIPI_2M_30FPS_8BIT) ||
			(pstViInfo->stSnsInfo.enSnsType == TECHPOINT_TP2850_MIPI_4M_30FPS_8BIT) ||
			(pstViInfo->stSnsInfo.enSnsType == VIVO_MCS369Q_4M_30FPS_12BIT) ||
			(pstViInfo->stSnsInfo.enSnsType == VIVO_MCS369_2M_30FPS_12BIT) ||
			(pstViInfo->stSnsInfo.enSnsType == VIVO_MM308M2_2M_25FPS_8BIT) ||
			(pstViInfo->stSnsInfo.enSnsType == NEXTCHIP_N5_2M_25FPS_8BIT) ||
			(pstViInfo->stSnsInfo.enSnsType == PIXELPLUS_PR2020_1M_25FPS_8BIT) ||
			(pstViInfo->stSnsInfo.enSnsType == PIXELPLUS_PR2020_1M_30FPS_8BIT) ||
			(pstViInfo->stSnsInfo.enSnsType == PIXELPLUS_PR2020_2M_25FPS_8BIT) ||
			(pstViInfo->stSnsInfo.enSnsType == PIXELPLUS_PR2020_2M_30FPS_8BIT) ||
			(pstViInfo->stSnsInfo.enSnsType == PIXELPLUS_PR2100_2M_25FPS_8BIT) ||
			(pstViInfo->stSnsInfo.enSnsType == PIXELPLUS_PR2100_2M_2CH_25FPS_8BIT) ||
			(pstViInfo->stSnsInfo.enSnsType == PIXELPLUS_PR2100_2M_2CH_2L_25FPS_8BIT) ||
			(pstViInfo->stSnsInfo.enSnsType == PIXELPLUS_PR2100_2M_4CH_25FPS_8BIT) ||
			(pstViInfo->stSnsInfo.enSnsType == PIXELPLUS_PR2100_2M_4CH_30FPS_8BIT)) {
			stPipeAttr.bYuvBypassPath = CVI_TRUE;
		} else {
			stPipeAttr.bYuvBypassPath = CVI_FALSE;
		}

		stPipeAttr.u32MaxW = vi_ut_ctx.stSize[i].u32Width;
		stPipeAttr.u32MaxH = vi_ut_ctx.stSize[i].u32Height;
		stPipeAttr.enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
		stPipeAttr.enBitWidth = DATA_BITWIDTH_12;
		stPipeAttr.stFrameRate.s32SrcFrameRate = -1;
		stPipeAttr.stFrameRate.s32DstFrameRate = -1;
		stPipeAttr.bNrEn = CVI_TRUE;
		stPipeAttr.bYuvBypassPath = CVI_FALSE;

		for (j = 0; j < WDR_MAX_PIPE_NUM; j++) {
			if (pstViInfo->stPipeInfo.aPipe[j] >= 0 && pstViInfo->stPipeInfo.aPipe[j] < VI_MAX_PIPE_NUM) {
				ViPipe = pstViInfo->stPipeInfo.aPipe[j];
				s32Ret = CVI_VI_CreatePipe(ViPipe, &stPipeAttr);
				if (s32Ret != CVI_SUCCESS) {
					VI_UT_PRT("CVI_VI_CreatePipe failed with %#x!\n", s32Ret);
					goto error;
				}

				s32Ret = CVI_VI_StartPipe(ViPipe);
				if (s32Ret != CVI_SUCCESS) {
					VI_UT_PRT("CVI_VI_StartPipe failed with %#x!\n", s32Ret);
					goto error;
				}
			}
		}
	}

	if (vi_ut_ctx.is_enable_sensor) {
		s32Ret = SAMPLE_COMM_VI_CreateIsp(&stViConfig);
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("VI_CreateIsp failed with %#x!\n", s32Ret);
			goto error;
		}
	}

	s32Ret = _vi_set_be_online();
	if (s32Ret < 0) {
		VI_UT_PRT("BE online failed\n");
		goto error;
	}

	s32Ret = _vi_set_post_online();
	if (s32Ret < 0) {
		VI_UT_PRT("Post online failed\n");
		goto error;
	}

	s32Ret = _vi_set_hdr();
	if (s32Ret < 0) {
		VI_UT_PRT("set hdr failed\n");
		goto error;
	}

	if (vi_ut_ctx.is_3dnr_off) {
		s32Ret = _vi_set_3dnr(false);
		if (s32Ret < 0) {
			VI_UT_PRT("set 3dnr failed\n");
			goto error;
		}
	}

	if (vi_ut_ctx.is_patgen_enable) {
		for (i = 0; i < stViConfig.s32WorkingViNum; i++) {
			s32Ret = _vi_set_snr_info(i);
			if (s32Ret < 0) {
				VI_UT_PRT("set snr[%d] info failed\n", i);
				goto error;
			}
		}
	}

	s32Ret = _vi_set_patgen(vi_ut_ctx.is_patgen_enable);
	if (s32Ret < 0) {
		VI_UT_PRT( "set patgen failed\n");
		goto error;
	}

	s32Ret = vi_ut_plat_vi_startchn(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("VI_StartViChn failed with %#x!\n", s32Ret);
		goto error;
	}
error:
	return s32Ret;
}

CVI_S32 vi_ut_plat_vi_deinit(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	SAMPLE_COMM_VI_DestroyIsp(&stViConfig);
	SAMPLE_COMM_VI_DestroyVi(&stViConfig);

	s32Ret = _vi_set_patgen(0);
	if (s32Ret < 0) {
		VI_UT_PRT( "set patgen failed\n");
	}

	return s32Ret;
}
