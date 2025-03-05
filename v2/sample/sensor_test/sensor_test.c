
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <md5sum.h>
#include <inttypes.h>

#include <fcntl.h>		/* low-level i/o */
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
#include "ae_test.h"
#include "replay.h"

static SAMPLE_VI_CONFIG_S g_stViConfig;
static SAMPLE_INI_CFG_S g_stIniCfg;

void _PLAT_ERR_Exit(void)
{
	if (g_stViConfig.s32WorkingViNum != 0) {
		SAMPLE_COMM_VI_DestroyIsp(&g_stViConfig);
		SAMPLE_COMM_VI_DestroyVi(&g_stViConfig);
	}
	SAMPLE_COMM_SYS_Exit();
}

static void sys_handle_signal(int nSignal, siginfo_t *si, void *arg)
{
	UNUSED(nSignal);
	UNUSED(si);
	UNUSED(arg);

	_PLAT_ERR_Exit();

	exit(1);
}

CVI_S32 vpss_config_online_mode(SIZE_S *sns_size)
{
	VPSS_GRP	   VpssGrp	  = VPSS_ONLINE_GRP_0;
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_CHN	   VpssChn	  = VPSS_CHN0;
	CVI_BOOL	   abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM] = {0};
	CVI_S32 s32Ret = CVI_SUCCESS;

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate			= -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate			= -1;
	stVpssGrpAttr.enPixelFormat							= SAMPLE_PIXEL_FORMAT;
	stVpssGrpAttr.u32MaxW								= sns_size->u32Width;
	stVpssGrpAttr.u32MaxH								= sns_size->u32Height;

	astVpssChnAttr[VpssChn].u32Width					= 1280;
	astVpssChnAttr[VpssChn].u32Height					= 720;
	astVpssChnAttr[VpssChn].enVideoFormat				= VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat				= SAMPLE_PIXEL_FORMAT;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth					= 0;
	astVpssChnAttr[VpssChn].bMirror						= CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip						= CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	astVpssChnAttr[VpssChn].stNormalize.bEnable			= CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 sys_config_online_mode(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_VPSS_MODE_S	stVIVPSSMode;

	/************************************************
	 * Config vpss online mode
	 ************************************************/
	stVIVPSSMode.aenMode[0] = stVIVPSSMode.aenMode[1] = VI_OFFLINE_VPSS_ONLINE;

	s32Ret = CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_SetVIVPSSMode failed with %#x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 vi_start_dev(SAMPLE_VI_INFO_S *pstViInfo, CVI_U32 isp_mode)
{
	CVI_S32             s32Ret;
	VI_DEV              ViDev;
	SAMPLE_SNS_TYPE_E   enSnsType;
	VI_DEV_ATTR_S       stViDevAttr;
	VI_DEV_BIND_PIPE_S  stViDevBindAttr;
	ISP_PUB_ATTR_S      pstPubAttr;

	ViDev       = pstViInfo->stDevInfo.ViDev;
	enSnsType   = pstViInfo->stSnsInfo.enSnsType;

	SAMPLE_COMM_VI_GetDevAttrBySns(enSnsType, &stViDevAttr);
	SAMPLE_COMM_ISP_GetIspAttrBySns(enSnsType, &pstPubAttr);
	stViDevAttr.stWDRAttr.enWDRMode = pstViInfo->stDevInfo.enWDRMode;
	stViDevAttr.snrFps = (CVI_U32)pstPubAttr.f32FrameRate;
	stViDevBindAttr.PipeId[0] = (CVI_S32)pstViInfo->stSnsInfo.MipiDev;
	stViDevBindAttr.u32Num = 1;

	if (isp_mode == 2) {
		stViDevAttr.enYuvSceneMode = VI_ISP_YUV_SCENE_BYPASS;
	} else {
		stViDevAttr.enYuvSceneMode = VI_ISP_YUV_SCENE_ISP;
	}

	s32Ret = CVI_VI_SetDevAttr(ViDev, &stViDevAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_SetDevAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_SetDevBindAttr(ViDev, &stViDevBindAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_SetDevBindAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_EnableDev(ViDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_EnableDev failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 vi_start_cfg(SAMPLE_VI_CONFIG_S *pstViConfig, CVI_S32 isp_mode)
{
	PIC_SIZE_E	   enPicSize;
	SIZE_S		   stSize;

	VI_DEV ViDev = 0;
	VI_PIPE ViPipe = 0;
	VI_PIPE_ATTR_S	   stPipeAttr;

	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i = 0, j = 0;
	CVI_S32 s32DevNum;

	memcpy((void *)&g_stViConfig, (void *)pstViConfig, sizeof(SAMPLE_VI_CONFIG_S));

	/************************************************
	 * step1:  Get input size
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(pstViConfig->astViInfo[ViDev].stSnsInfo.enSnsType, &enPicSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VI_GetSizeBySensor failed with %#x\n", s32Ret);
		goto error;
	}

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
		goto error;
	}

	/************************************************
	 * step2:  Init VI ISP
	 ************************************************/
#if USE_USER_SEN_DRIVER
	s32Ret = SAMPLE_COMM_VI_StartSensor(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "system start sensor failed with %#x\n", s32Ret);
		goto error;
	}
#endif
	for (i = 0; i < pstViConfig->s32WorkingViNum; i++) {
		ViDev = i;

		s32Ret = vi_start_dev(&pstViConfig->astViInfo[ViDev], isp_mode);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "VI_StartDev failed with %#x!\n", s32Ret);
			goto error;
		}
	}

#if USE_USER_SEN_DRIVER
	s32Ret = SAMPLE_COMM_VI_StartMIPI(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "system start MIPI failed with %#x\n", s32Ret);
		goto error;
	}

	s32Ret = SAMPLE_COMM_VI_SensorProbe(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "system sensor probe failed with %#x\n", s32Ret);
		goto error;
	}
#endif

	stPipeAttr.bYuvSkip = CVI_FALSE;
	stPipeAttr.u32MaxW = stSize.u32Width;
	stPipeAttr.u32MaxH = stSize.u32Height;
	stPipeAttr.enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stPipeAttr.enBitWidth = DATA_BITWIDTH_12;
	stPipeAttr.stFrameRate.s32SrcFrameRate = -1;
	stPipeAttr.stFrameRate.s32DstFrameRate = -1;
	stPipeAttr.bNrEn = CVI_TRUE;
	stPipeAttr.bYuvBypassPath = CVI_FALSE;
	stPipeAttr.enCompressMode = pstViConfig->astViInfo[0].stChnInfo.enCompressMode;

	for (i = 0; i < pstViConfig->s32WorkingViNum; i++) {
		SAMPLE_VI_INFO_S *pstViInfo = NULL;

		s32DevNum  = pstViConfig->as32WorkingViId[i];
		pstViInfo = &pstViConfig->astViInfo[s32DevNum];
		stPipeAttr.bYuvBypassPath = SAMPLE_COMM_VI_GetYuvBypassSts(pstViInfo->stSnsInfo.enSnsType);

		for (j = 0; j < WDR_MAX_PIPE_NUM; j++) {
			if (pstViInfo->stPipeInfo.aPipe[j] >= 0 && pstViInfo->stPipeInfo.aPipe[j] < VI_MAX_PIPE_NUM) {
				ViPipe = pstViInfo->stPipeInfo.aPipe[j];
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

				s32Ret = CVI_VI_GetPipeAttr(ViPipe, &stPipeAttr);
				if (s32Ret != CVI_SUCCESS) {
					CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_GetPipeAttr failed with %#x!\n", s32Ret);
					goto error;
				}
			}
		}
	}

	s32Ret = SAMPLE_COMM_VI_CreateIsp(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "VI_CreateIsp failed with %#x!\n", s32Ret);
		goto error;
	}

	s32Ret = SAMPLE_COMM_VI_StartViChn(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "VI_StartViChn failed with %#x!\n", s32Ret);
		goto error;
	}

	return s32Ret;
error:
	_PLAT_ERR_Exit();
	return s32Ret;
}

static int sys_vi_init(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MMF_VERSION_S stVersion;
	SAMPLE_INI_CFG_S stIniCfg;
	SAMPLE_VI_CONFIG_S stViConfig;
	LOG_LEVEL_CONF_S log_conf;
	VI_DEV_ATTR_S stVidevAttr;
	CVI_U32 Vb_cnt;
	CVI_S32 isp_mode;

	memset(&stVersion, 0, sizeof(MMF_VERSION_S));
	memset(&stIniCfg, 0, sizeof(SAMPLE_INI_CFG_S));
	memset(&stViConfig, 0, sizeof(SAMPLE_VI_CONFIG_S));
	memset(&log_conf, 0, sizeof(LOG_LEVEL_CONF_S));
	memset(&stVidevAttr, 0, sizeof(VI_DEV_ATTR_S));

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

	for (CVI_S32 i = 0; i < stViConfig.s32WorkingViNum; i++) {
		if (SAMPLE_COMM_VI_GetYuvBypassSts(stIniCfg.enSnsType[i])) {
			SAMPLE_PRT("NOW is YUV sensor\n");
			SAMPLE_PRT("Set yuv sns tuning and vi-vpss online : [0]\n");
			SAMPLE_PRT("Set yuv sns tuning and vi-vpss offline : [1]\n");
			SAMPLE_PRT("Set yuv sns bypass isp and vi-vpss offline : [2]\n");
			SAMPLE_PRT("Input option: ");
			scanf("%d", &isp_mode);
			if (isp_mode == 2) {
				stViConfig.astViInfo[i].stChnInfo.enPixFormat = PIXEL_FORMAT_YUYV;
			} else {
				stViConfig.astViInfo[i].stChnInfo.enPixFormat = PIXEL_FORMAT_NV21;
			}
		} else {
			isp_mode = 1;
		}
	}

	memcpy(&g_stViConfig, &stViConfig, sizeof(SAMPLE_VI_CONFIG_S));
	memcpy(&g_stIniCfg, &stIniCfg, sizeof(SAMPLE_INI_CFG_S));

	/************************************************
	 * step2:  Get input size
	 ************************************************/
	CVI_U32 u32BlkSize, u32BlkRotSize;
	PIC_SIZE_E enPicSize;
	VB_CONFIG_S stVbConf;
	SIZE_S stSize;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 0;

	for (CVI_S32 i = 0; i < stViConfig.s32WorkingViNum; i++) {
		Vb_cnt = 0;
		bool createNewPool = true;

		s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stIniCfg.enSnsType[i], &enPicSize);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VI_GetSizeBySensor failed with %#x\n", s32Ret);
			return s32Ret;
		}

		s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
			return s32Ret;
		}

		s32Ret = SAMPLE_COMM_VI_GetDevAttrBySns(stIniCfg.enSnsType[i], &stVidevAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VI_GetDevAttrBySns failed with %#x\n", s32Ret);
			return s32Ret;
		}
		if (stVidevAttr.enInputDataType == VI_DATA_TYPE_YUV) {
			if (stVidevAttr.enWorkMode == VI_WORK_MODE_2Multiplex) {
				Vb_cnt = 6;
			} else if (stVidevAttr.enWorkMode == VI_WORK_MODE_3Multiplex) {
				Vb_cnt = 9;
			} else if (stVidevAttr.enWorkMode == VI_WORK_MODE_4Multiplex) {
				Vb_cnt = 12;
			} else {
				Vb_cnt = 3;
			}
		} else {
			Vb_cnt = 3;
		}

		u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
					stViConfig.astViInfo[i].stChnInfo.enPixFormat,
					DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkRotSize = COMMON_GetPicBufferSize(stSize.u32Height, stSize.u32Width,
					stViConfig.astViInfo[i].stChnInfo.enPixFormat,
					DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkSize = u32BlkSize > u32BlkRotSize ? u32BlkSize : u32BlkRotSize;

		for (CVI_U32 j = 0; j < stVbConf.u32MaxPoolCnt; j++) {
			if (stVbConf.astCommPool[j].u32BlkSize == u32BlkSize) {
				stVbConf.astCommPool[j].u32BlkCnt += Vb_cnt;
				createNewPool = false;
				break;
			}
		}
		if (createNewPool) {
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkSize = u32BlkSize;
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkCnt = Vb_cnt;
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].enRemapMode = VB_REMAP_MODE_CACHED;
			SAMPLE_PRT("set VBpool [%d] %d:%d, BlkCnt= %d, Size = %d\n",
						stVbConf.u32MaxPoolCnt, stSize.u32Width, stSize.u32Height,
						stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkCnt,
						stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkSize);
			stVbConf.u32MaxPoolCnt++;
		} else {
			SAMPLE_PRT("set VBpool [%d] %d:%d, BlkCnt= %d, Size = %d\n",
						stVbConf.u32MaxPoolCnt, stSize.u32Width, stSize.u32Height,
						stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkCnt,
						stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkSize);
		}
	}

	if (stVbConf.u32MaxPoolCnt == 1) {
		stVbConf.astCommPool[0].u32BlkCnt += 2;
	}

	/************************************************
	 * step3:  Init modules
	 ************************************************/
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = sys_handle_signal;
	sa.sa_flags = SA_SIGINFO|SA_RESETHAND; // Reset signal handler to system default after signal triggered
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "system init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * Config vpss online mode
	 ************************************************/
	if (!isp_mode) {
		s32Ret = sys_config_online_mode();
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "sys_config_online_mode failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	s32Ret = vi_start_cfg(&stViConfig, isp_mode);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * Config and init VPSS
	 ************************************************/
	if (!isp_mode) {
		s32Ret = vpss_config_online_mode(&stSize);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vpss_config_online_mode failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	return CVI_SUCCESS;
}

static void sys_vi_deinit(void)
{
	SAMPLE_COMM_VI_DestroyIsp(&g_stViConfig);

	SAMPLE_COMM_VI_DestroyVi(&g_stViConfig);

	SAMPLE_COMM_SYS_Exit();
}

static CVI_S32 _vi_get_chn_frame(CVI_U8 chn)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	VI_CROP_INFO_S crop_info = {0};

	if (CVI_VI_GetChnFrame(0, chn, &stVideoFrame, 3000) == 0) {
		FILE *output;
		size_t image_size = stVideoFrame.stVFrame.u32Length[0] + stVideoFrame.stVFrame.u32Length[1]
				  + stVideoFrame.stVFrame.u32Length[2];
		CVI_VOID *vir_addr;
		CVI_U32 plane_offset, u32LumaSize, u32ChromaSize;
		CVI_CHAR img_name[128] = {0, };

		CVI_TRACE_LOG(CVI_DBG_WARN, "width: %d, height: %d, total_buf_length: %zu\n",
			   stVideoFrame.stVFrame.u32Width,
			   stVideoFrame.stVFrame.u32Height, image_size);

		snprintf(img_name, sizeof(img_name), "sample_%d.yuv", chn);

		output = fopen(img_name, "wb");
		if (output == NULL) {
			memset(img_name, 0x0, sizeof(img_name));
			snprintf(img_name, sizeof(img_name), "/mnt/data/sample_%d.yuv", chn);
			output = fopen(img_name, "wb");
			if (output == NULL) {
				CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame);
				CVI_TRACE_LOG(CVI_DBG_ERR, "fopen fail\n");
				return CVI_FAILURE;
			}
		}

		u32LumaSize =  stVideoFrame.stVFrame.u32Stride[0] * stVideoFrame.stVFrame.u32Height;
		u32ChromaSize =  stVideoFrame.stVFrame.u32Stride[1] * stVideoFrame.stVFrame.u32Height / 2;
		CVI_VI_GetChnCrop(0, chn, &crop_info);
		if (crop_info.bEnable) {
			u32LumaSize = ALIGN((crop_info.stCropRect.u32Width * 8 + 7) >> 3, DEFAULT_ALIGN) *
				ALIGN(crop_info.stCropRect.u32Height, 2);
			u32ChromaSize = (ALIGN(((crop_info.stCropRect.u32Width >> 1) * 8 + 7) >> 3, DEFAULT_ALIGN) *
				ALIGN(crop_info.stCropRect.u32Height, 2)) >> 1;
		}
		vir_addr = CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], image_size);
		CVI_SYS_IonInvalidateCache(stVideoFrame.stVFrame.u64PhyAddr[0], vir_addr, image_size);
		plane_offset = 0;
		for (int i = 0; i < 3; i++) {
			if (stVideoFrame.stVFrame.u32Length[i] != 0) {
				stVideoFrame.stVFrame.pu8VirAddr[i] = vir_addr + plane_offset;
				plane_offset += stVideoFrame.stVFrame.u32Length[i];
				CVI_TRACE_LOG(CVI_DBG_WARN,
					   "plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) length(%d)\n",
					   i, stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Stride[i],
					   stVideoFrame.stVFrame.u32Length[i]);
				fwrite((void *)stVideoFrame.stVFrame.pu8VirAddr[i]
					, (i == 0) ? u32LumaSize : u32ChromaSize, 1, output);
			}
		}
		CVI_SYS_Munmap(vir_addr, image_size);

		if (CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame) != 0)
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_ReleaseChnFrame NG\n");

		fclose(output);
		return CVI_SUCCESS;
	}
	CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_GetChnFrame NG\n");
	return CVI_FAILURE;
}

static long diff_in_us(struct timespec t1, struct timespec t2)
{
	struct timespec diff;

	if (t2.tv_nsec-t1.tv_nsec < 0) {
		diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
	} else {
		diff.tv_sec  = t2.tv_sec - t1.tv_sec;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
	}
	return (diff.tv_sec * 1000000.0 + diff.tv_nsec / 1000.0);
}

static void ViConfigReInit(SAMPLE_VI_CONFIG_S *p_stViConfig, SAMPLE_INI_CFG_S *p_stIniCfg)
{
	int s32WorkSnsId = 0;

	for (s32WorkSnsId = 0; s32WorkSnsId < p_stIniCfg->devNum; s32WorkSnsId++) {
		p_stViConfig->s32WorkingViNum					= 1 + s32WorkSnsId;
		p_stViConfig->as32WorkingViId[s32WorkSnsId]			= s32WorkSnsId;
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.enSnsType	=
			p_stIniCfg->enSnsType[s32WorkSnsId];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.MipiDev		=
			p_stIniCfg->MipiDev[s32WorkSnsId];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.s32BusId	=
			p_stIniCfg->s32BusId[s32WorkSnsId];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[0]   =
			p_stIniCfg->as16LaneId[s32WorkSnsId][0];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[1]   =
			p_stIniCfg->as16LaneId[s32WorkSnsId][1];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[2]   =
			p_stIniCfg->as16LaneId[s32WorkSnsId][2];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[3]   =
			p_stIniCfg->as16LaneId[s32WorkSnsId][3];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[4]   =
			p_stIniCfg->as16LaneId[s32WorkSnsId][4];

		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[0] =
			p_stIniCfg->as8PNSwap[s32WorkSnsId][0];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[1] =
			p_stIniCfg->as8PNSwap[s32WorkSnsId][1];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[2] =
			p_stIniCfg->as8PNSwap[s32WorkSnsId][2];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[3] =
			p_stIniCfg->as8PNSwap[s32WorkSnsId][3];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[4] =
			p_stIniCfg->as8PNSwap[s32WorkSnsId][4];
		p_stViConfig->astViInfo[s32WorkSnsId].stDevInfo.enWDRMode =
			p_stIniCfg->enWDRMode[s32WorkSnsId];
	}
}

CVI_S32 sensor_dump_yuv(void)
{
	CVI_S32 loop = 0;
	CVI_U32 ok = 0, ng = 0;
	CVI_U8  chn = 0;
	int tmp;
	struct timespec start, end;

	CVI_TRACE_LOG(CVI_DBG_WARN, "Get frm from which chn(0~7): ");
	scanf("%d", &tmp);
	chn = tmp;
	CVI_TRACE_LOG(CVI_DBG_WARN, "how many loops to do(11111) is infinite: ");
	scanf("%d", &loop);
	while (loop > 0) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		if (_vi_get_chn_frame(chn) == CVI_SUCCESS) {
			++ok;
			clock_gettime(CLOCK_MONOTONIC, &end);
			CVI_TRACE_LOG(CVI_DBG_WARN, "ms consumed: %f\n",
						(CVI_FLOAT)diff_in_us(start, end)/1000);
		} else
			++ng;
		//sleep(1);
		if (loop != 11111)
			loop--;
	}
	CVI_TRACE_LOG(CVI_DBG_WARN, "VI GetChnFrame OK(%d) NG(%d)\n", ok, ng);

	CVI_TRACE_LOG(CVI_DBG_WARN, "Dump VI yuv TEST-PASS\n");

	return CVI_SUCCESS;
}

static CVI_S32 _vi_get_chn_md5(CVI_S32 frm_num, CVI_U8 chn)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	VI_CROP_INFO_S crop_info = {0};
	unsigned char md5_result[MD5_DIGEST_LENGTH];
	char md5_str[MD5_DIGEST_LENGTH * 2 + 1];

	if (CVI_VI_GetChnFrame(0, chn, &stVideoFrame, 3000) == 0) {
		FILE *output;
		FILE* md5_output;
		size_t image_size = stVideoFrame.stVFrame.u32Length[0] + stVideoFrame.stVFrame.u32Length[1]
				  + stVideoFrame.stVFrame.u32Length[2];
		CVI_VOID *vir_addr;
		CVI_U32 plane_offset, u32LumaSize, u32ChromaSize;
		CVI_CHAR img_name[128] = {0, };
		CVI_CHAR buffer[64];

		CVI_TRACE_LOG(CVI_DBG_WARN, "width: %d, height: %d, total_buf_length: %zu\n",
			   stVideoFrame.stVFrame.u32Width,
			   stVideoFrame.stVFrame.u32Height, image_size);

		snprintf(img_name, sizeof(img_name), "sample_%d.yuv", chn);

		output = fopen(img_name, "wb");
		if (output == NULL) {
			memset(img_name, 0x0, sizeof(img_name));
			snprintf(img_name, sizeof(img_name), "/mnt/data/sample_%d.yuv", chn);
			output = fopen(img_name, "wb");
			if (output == NULL) {
				CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame);
				CVI_TRACE_LOG(CVI_DBG_ERR, "fopen fail\n");
				return CVI_FAILURE;
			}
		}

		strcpy(buffer, "frame_info.txt");
		md5_output = fopen(buffer, "a");
		if (md5_output == NULL) {
			snprintf(buffer, sizeof(buffer), "/mnt/data/md5_info.txt");
			md5_output = fopen("frame_info.txt", "a");
		}

		u32LumaSize =  stVideoFrame.stVFrame.u32Stride[0] * stVideoFrame.stVFrame.u32Height;
		u32ChromaSize =  stVideoFrame.stVFrame.u32Stride[1] * stVideoFrame.stVFrame.u32Height / 2;
		CVI_VI_GetChnCrop(0, chn, &crop_info);
		if (crop_info.bEnable) {
			u32LumaSize = ALIGN((crop_info.stCropRect.u32Width * 8 + 7) >> 3, DEFAULT_ALIGN) *
				ALIGN(crop_info.stCropRect.u32Height, 2);
			u32ChromaSize = (ALIGN(((crop_info.stCropRect.u32Width >> 1) * 8 + 7) >> 3, DEFAULT_ALIGN) *
				ALIGN(crop_info.stCropRect.u32Height, 2)) >> 1;
		}
		vir_addr = CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], image_size);
		CVI_SYS_IonInvalidateCache(stVideoFrame.stVFrame.u64PhyAddr[0], vir_addr, image_size);
		plane_offset = 0;
		for (int i = 0; i < 3; i++) {
			if (stVideoFrame.stVFrame.u32Length[i] != 0) {
				stVideoFrame.stVFrame.pu8VirAddr[i] = vir_addr + plane_offset;
				plane_offset += stVideoFrame.stVFrame.u32Length[i];
				CVI_TRACE_LOG(CVI_DBG_WARN,
					   "plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) length(%d)\n",
					   i, stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Stride[i],
					   stVideoFrame.stVFrame.u32Length[i]);
				fwrite((void *)stVideoFrame.stVFrame.pu8VirAddr[i]
					, (i == 0) ? u32LumaSize : u32ChromaSize, 1, output);
				MD5((void *)stVideoFrame.stVFrame.pu8VirAddr[i]
					,	(i == 0) ? u32LumaSize : u32ChromaSize, md5_result);
				for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
					sprintf(&md5_str[i*2], "%02x", (unsigned int)md5_result[i]);
				}
				fprintf(md5_output, "frame %d: md5=%s\n", frm_num, md5_str);
			}
		}
		CVI_SYS_Munmap(vir_addr, image_size);

		if (CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame) != 0)
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_ReleaseChnFrame NG\n");

		fclose(output);
		fclose(md5_output);
		return CVI_SUCCESS;
	}
	CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_GetChnFrame NG\n");
	return CVI_FAILURE;
}

static CVI_S32 sensor_slt_test(void)
{
	CVI_S32 loop = 0;
	CVI_U32 ok = 0, ng = 0;
	CVI_U8  chn = 0;
	int tmp;
	struct timespec start, end;

	CVI_TRACE_LOG(CVI_DBG_WARN, "Get frm md5 from which chn(0~7): ");
	scanf("%d", &tmp);
	chn = tmp;
	CVI_TRACE_LOG(CVI_DBG_WARN, "how many loops to do(11111) is infinite: ");
	scanf("%d", &loop);
	while (loop > 0) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		if (_vi_get_chn_md5(loop, chn) == CVI_SUCCESS) {
			++ok;
			clock_gettime(CLOCK_MONOTONIC, &end);
			CVI_TRACE_LOG(CVI_DBG_WARN, "ms consumed: %f\n",
						(CVI_FLOAT)diff_in_us(start, end)/1000);
		} else
			++ng;
		//sleep(1);
		if (loop != 11111)
			loop--;
	}
	CVI_TRACE_LOG(CVI_DBG_WARN, "Get frame success\n");

	return CVI_SUCCESS;
}

static CVI_S32 sensor_flip_mirror(void)
{
	int flip;
	int mirror;
	int chnID;
	int pipeID;

	CVI_TRACE_LOG(CVI_DBG_WARN, "chn(0~5): ");
	scanf("%d", &chnID);
	CVI_TRACE_LOG(CVI_DBG_WARN, "Flip enable/disable(1/0): ");
	scanf("%d", &flip);
	CVI_TRACE_LOG(CVI_DBG_WARN, "Mirror enable/disable(1/0): ");
	scanf("%d", &mirror);
	pipeID = chnID;
	CVI_VI_SetChnFlipMirror(pipeID, chnID, flip, mirror);

	return CVI_SUCCESS;
}

static CVI_S32 sensor_dump_raw(void)
{
	VIDEO_FRAME_INFO_S stVideoFrame[2];
	VI_DUMP_ATTR_S attr;
	struct timeval tv1;
	int frm_num = 1, j = 0;
	CVI_U32 dev = 0, loop = 0;
	struct timespec start, end;
	CVI_S32 s32Ret = CVI_SUCCESS;

	memset(stVideoFrame, 0, sizeof(stVideoFrame));

	stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stVideoFrame[1].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

	CVI_TRACE_LOG(CVI_DBG_WARN, "To get raw dump from dev(0~5): ");
	scanf("%d", &dev);

	attr.bEnable = 1;
	attr.u32Depth = 0;
	attr.enDumpType = VI_DUMP_TYPE_RAW;

	CVI_VI_SetPipeDumpAttr(dev, &attr);

	attr.bEnable = 0;
	attr.enDumpType = VI_DUMP_TYPE_IR;

	CVI_VI_GetPipeDumpAttr(dev, &attr);

	CVI_TRACE_LOG(CVI_DBG_WARN, "Enable(%d), DumpType(%d):\n", attr.bEnable, attr.enDumpType);
	CVI_TRACE_LOG(CVI_DBG_WARN, "how many loops to do (1~60)");
	scanf("%d", &loop);

	if (loop > 60)
		return s32Ret;

	while (loop > 0) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		frm_num = 1;

		CVI_VI_GetPipeFrame(dev, stVideoFrame, 1000);

		if (stVideoFrame[1].stVFrame.u64PhyAddr[0] != 0)
			frm_num = 2;

		gettimeofday(&tv1, NULL);

		for (j = 0; j < frm_num; j++) {
			size_t image_size = stVideoFrame[j].stVFrame.u32Length[0];
			unsigned char *ptr = calloc(1, image_size);
			FILE *output;
			char img_name[128] = {0,}, order_id[8] = {0,};

			if (attr.enDumpType == VI_DUMP_TYPE_RAW) {
				stVideoFrame[j].stVFrame.pu8VirAddr[0]
					= CVI_SYS_Mmap(stVideoFrame[j].stVFrame.u64PhyAddr[0]
					  , stVideoFrame[j].stVFrame.u32Length[0]);
				CVI_TRACE_LOG(CVI_DBG_WARN, "paddr(%#"PRIx64") vaddr(%p)\n",
							stVideoFrame[j].stVFrame.u64PhyAddr[0],
							stVideoFrame[j].stVFrame.pu8VirAddr[0]);

				memcpy(ptr, (const void *)stVideoFrame[j].stVFrame.pu8VirAddr[0],
					stVideoFrame[j].stVFrame.u32Length[0]);
				CVI_SYS_Munmap((void *)stVideoFrame[j].stVFrame.pu8VirAddr[0],
						stVideoFrame[j].stVFrame.u32Length[0]);

				switch (stVideoFrame[j].stVFrame.enBayerFormat) {
				default:
				case BAYER_FORMAT_BG:
					snprintf(order_id, sizeof(order_id), "BG");
					break;
				case BAYER_FORMAT_GB:
					snprintf(order_id, sizeof(order_id), "GB");
					break;
				case BAYER_FORMAT_GR:
					snprintf(order_id, sizeof(order_id), "GR");
					break;
				case BAYER_FORMAT_RG:
					snprintf(order_id, sizeof(order_id), "RG");
					break;
				}

				snprintf(img_name, sizeof(img_name),
						"./vi_%d_%s_%s_w_%d_h_%d_x_%d_y_%d_tv_%ld_%ld.raw",
						dev, (j == 0) ? "LE" : "SE", order_id,
						stVideoFrame[j].stVFrame.u32Width,
						stVideoFrame[j].stVFrame.u32Height,
						stVideoFrame[j].stVFrame.s16OffsetLeft,
						stVideoFrame[j].stVFrame.s16OffsetTop,
						tv1.tv_sec, tv1.tv_usec);

				CVI_TRACE_LOG(CVI_DBG_WARN, "dump image %s\n", img_name);

				output = fopen(img_name, "wb");

				fwrite(ptr, image_size, 1, output);
				fclose(output);
				free(ptr);
			}
		}

		CVI_VI_ReleasePipeFrame(dev, stVideoFrame);

		clock_gettime(CLOCK_MONOTONIC, &end);
		CVI_TRACE_LOG(CVI_DBG_WARN, "ms consumed: %f\n",
					(CVI_FLOAT)diff_in_us(start, end) / 1000);

		loop--;
	}

	CVI_TRACE_LOG(CVI_DBG_WARN, "Dump VI raw TEST-PASS\n");

	return s32Ret;
}

static CVI_S32 sensor_linear_wdr_switch(void)
{
	int tmp;
	CVI_U8 wdrMode = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;

	SAMPLE_COMM_VI_DestroyIsp(&g_stViConfig);
	// Stop VI.
	SAMPLE_COMM_VI_DestroyVi(&g_stViConfig);
	// Close ISP device.
	s32Ret = SAMPLE_COMM_VI_CLOSE();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi close failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}
	// select which mode want to switch.
	printf("Please select sensor input mode (0:linear/1:wdr) :");
	scanf("%d", &tmp);
	wdrMode = tmp;
	if (wdrMode == 0) {
		// Reset main sensor initial config to linear setting.
		g_stIniCfg.enSnsType[0] = SONY_IMX327_2L_MIPI_2M_30FPS_12BIT;
		g_stIniCfg.enWDRMode[0] = WDR_MODE_NONE;
		// Reset slave sensor initial config to linear setting.
		g_stIniCfg.enSnsType[1] = SONY_IMX327_SLAVE_MIPI_2M_30FPS_12BIT;
		g_stIniCfg.enWDRMode[1] = WDR_MODE_NONE;
	} else {
		// Reset main sensor initial config to wdr setting.
		g_stIniCfg.enSnsType[0] = SONY_IMX327_2L_MIPI_2M_30FPS_12BIT_WDR2TO1;
		g_stIniCfg.enWDRMode[0] = WDR_MODE_2To1_LINE;
		// Reset slave sensor initial config to wdr setting.
		g_stIniCfg.enSnsType[1] = SONY_IMX327_SLAVE_MIPI_2M_30FPS_12BIT_WDR2TO1;
		g_stIniCfg.enWDRMode[1] = WDR_MODE_2To1_LINE;
	}
	// Reconfig VI setting for different mode Re-initial correctly.
	ViConfigReInit(&g_stViConfig, &g_stIniCfg);
	// open Isp device.
	s32Ret = SAMPLE_COMM_VI_OPEN();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi open failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}
	// Initial VI & ISP.
	s32Ret = vi_start_cfg(&g_stViConfig, 2);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

int sensor_dump(void)
{
	CVI_U64 addr = 0;
	CVI_U32 size = 0;
	FILE *output;
	char img_name[128] = {0, };

	CVI_TRACE_LOG(CVI_DBG_WARN, "dump addr:\n");
	scanf("%lx", &addr);
	CVI_TRACE_LOG(CVI_DBG_WARN, "dump size(0\1):\n");
	scanf("%x", &size);

	snprintf(img_name, sizeof(img_name), "register_%lx.bin", addr);

	output = fopen(img_name, "wb");
	if (output == NULL) {
		memset(img_name, 0x0, sizeof(img_name));
		snprintf(img_name, sizeof(img_name), "register_%lx.bin", addr);
		output = fopen(img_name, "wb");
		if (output == NULL) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "fopen fail\n");
			return CVI_FAILURE;
		}
	}

	void *vir_addr = CVI_SYS_Mmap(addr, size);

	fwrite(vir_addr, size, 1, output);
	fflush(output);

	CVI_SYS_Munmap(vir_addr, size);

	fclose(output);

	return CVI_SUCCESS;
}

int sensor_proc(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 op;

	SAMPLE_PRT("---debug_info------------------------------------------------\n");
	SAMPLE_PRT("1: /proc/soph/vi_dbg\n");
	SAMPLE_PRT("2: /proc/soph/vi\n");
	SAMPLE_PRT("3: /proc/soph/mipi-rx\n");
	scanf("%d", &op);

	switch (op) {
	case 1:
		system("cat /proc/soph/vi_dbg");
		break;
	case 2:
		system("cat /proc/soph/vi");
		break;
	case 3:
		system("cat /proc/soph/mipi-rx");
		break;
	default:
		break;
	}

	return s32Ret;
}

static CVI_S32 get_yuv_from_addr(CVI_U64 phy_addr_y, CVI_U64 phy_addr_uv)
{
	CVI_U8 chn = 0;
	CVI_S32 weigth = 1280;
	CVI_S32 heigth = 720;
	size_t image_size = phy_addr_uv ? weigth * heigth * 1.5 : weigth * heigth * 2;
	CVI_VOID *vir_addr;
	CVI_U32 plane_offset, u32LumaSize, u32ChromaSize;
	CVI_CHAR img_name[128] = {0, };
	CVI_U64 u64PhyAddr[3] = {phy_addr_y, phy_addr_uv, 0};
	CVI_U32 u32Length[3] = {phy_addr_uv ? weigth * heigth : weigth * heigth * 2,
							phy_addr_uv ? (weigth * heigth / 2) : 0,
							0};
	CVI_U32 u32Stride[3] = {phy_addr_uv ? weigth : weigth * 2,
							phy_addr_uv ? weigth : 0,
							0};
	CVI_U8 *pu8VirAddr[3];
	FILE *output;

	CVI_TRACE_LOG(CVI_DBG_WARN, "heigth: %d, height: %d, total_buf_length: %zu\n", weigth, heigth, image_size);

	snprintf(img_name, sizeof(img_name), "sample_%d.yuv", chn);
	output = fopen(img_name, "wb");
	if (output == NULL) {
		memset(img_name, 0x0, sizeof(img_name));
		snprintf(img_name, sizeof(img_name), "/mnt/data/sample_%d.yuv", chn);
		output = fopen(img_name, "wb");
		if (output == NULL) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "fopen fail\n");
			return CVI_FAILURE;
		}
	}

	u32LumaSize = u32Stride[0] * heigth;
	u32ChromaSize = u32Stride[1] * heigth / 2;
	vir_addr = CVI_SYS_Mmap(u64PhyAddr[0], image_size);
	CVI_SYS_IonInvalidateCache(u64PhyAddr[0], vir_addr, image_size);
	plane_offset = 0;
	for (int i = 0; i < 3; i++) {
		if (u32Length[i] != 0) {
			pu8VirAddr[i] = vir_addr + plane_offset;
			plane_offset += u32Length[i];
			CVI_TRACE_LOG(CVI_DBG_WARN,
				   "plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) weigth(%d)\n",
				   i, u64PhyAddr[i],
				   pu8VirAddr[i],
				   u32Stride[i],
				   u32Length[i]);
			fwrite((void *)pu8VirAddr[i] , (i == 0) ? u32LumaSize : u32ChromaSize, 1, output);
		}
	}
	CVI_SYS_Munmap(vir_addr, image_size);

	fclose(output);
	return CVI_SUCCESS;
}

CVI_S32 get_vi_yuv_debug(void)
{
	CVI_U64 phy_addr_y, phy_addr_uv;
	CVI_U32 is_nv21;

	printf("Want dump nv21 [1] or yuyv[0]: ");
	scanf("%d", &is_nv21);
	if (is_nv21) {
		printf("Enter the Y address in hexadecimal: ");
		scanf("%lx", &phy_addr_y);
		printf("Enter the UV address in hexadecimal: ");
		scanf("%lx", &phy_addr_uv);
	} else {
		printf("Enter the Y address in hexadecimal: ");
		scanf("%lx", &phy_addr_y);
		phy_addr_uv = 0;
	}

	return get_yuv_from_addr(phy_addr_y, phy_addr_uv);
}
//#define ENABLE_ISP_TOOL_DAEMON 1
//#define JSONRPC_PORT	(5566)
//extern void isp_daemon2_init(unsigned int port);
//extern void isp_daemon2_uninit(void);

int main(int argc, char **argv)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 op;

	UNUSED(argc);
	UNUSED(argv);

	SAMPLE_PRT("select is replay yes [1] no [0]:");
	scanf("%d", &op);
	if (op) {
		s32Ret = replay_vi_init();
		if (s32Ret != CVI_SUCCESS)
			return s32Ret;
		goto REPLAY_PATH;
	}

	s32Ret = sys_vi_init();
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

#ifdef ENABLE_ISP_TOOL_DAEMON
	isp_daemon2_init(JSONRPC_PORT);
#endif

	usleep(100 * 1000);

	system("stty erase ^H");

	do {
		SAMPLE_PRT("---Basic------------------------------------------------\n");
		SAMPLE_PRT("1: dump vi raw frame\n");
		SAMPLE_PRT("2: dump vi yuv frame\n");
		SAMPLE_PRT("3: set chn flip/mirror\n");
		SAMPLE_PRT("4: linear wdr switch\n");
		SAMPLE_PRT("5: AE debug\n");
		SAMPLE_PRT("6: sensor dump\n");
		SAMPLE_PRT("7: sensor proc\n");
		SAMPLE_PRT("8: sensor md5 test(for slt_test lt6911)\n");
		SAMPLE_PRT("9: dump vi yuv frame from phyaddr\n");
		SAMPLE_PRT("255: exit\n");
		scanf("%d", &op);

		switch (op) {
		case 1:
			s32Ret = sensor_dump_raw();
			break;
		case 2:
			s32Ret = sensor_dump_yuv();
			break;
		case 3:
			s32Ret = sensor_flip_mirror();
			break;
		case 4:
			s32Ret = sensor_linear_wdr_switch();
			break;
		case 5:
			s32Ret = sensor_ae_test();
			break;
		case 6:
			s32Ret = sensor_dump();
			break;
		case 7:
			s32Ret = sensor_proc();
			break;
		case 8:
			s32Ret = sensor_slt_test();
			break;
		case 9:
			s32Ret = get_vi_yuv_debug();
			break;
		default:
			break;
		}
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "op(%d) failed with %#x!\n", op, s32Ret);
			break;
		}
	} while (op != 255);

#ifdef ENABLE_ISP_TOOL_DAEMON
	isp_daemon2_uninit();
#endif

REPLAY_PATH:
	sys_vi_deinit();

	return s32Ret;
}

