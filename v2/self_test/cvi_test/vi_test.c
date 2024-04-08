#include "vi_test.h"

#define NONE	"\033[m"
#define RED	"\033[0;32;31m"
#define GREEN	"\033[0;32;32m"

CVI_S32 VI_Test_Open(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = SAMPLE_COMM_VI_OPEN();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi open failed. s32Ret: 0x%x !\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 VI_Test_Init(SAMPLE_VI_CONFIG_S *viCfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = SAMPLE_PLAT_VI_INIT(viCfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi init failed. s32Ret: 0x%x !\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 VI_Test_Close(SAMPLE_VI_CONFIG_S *viCfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	SAMPLE_COMM_VI_DestroyIsp(viCfg);
	SAMPLE_COMM_VI_DestroyVi(viCfg);

	s32Ret = SAMPLE_COMM_VI_CLOSE();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi close failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 VI_Test_OFL_OL_VPSS(SAMPLE_VI_CONFIG_S *viCfg, SAMPLE_INI_CFG_S *iniCfg)
{
	CVI_S32		s32Ret = CVI_SUCCESS;
	SIZE_S		stSizeSns0, stSizeSns1, stSizeOut = {.u32Width = 1280, .u32Height = 720};
	PIC_SIZE_E	enPicSize;
	VI_VPSS_MODE_S	stVIVPSSMode;
	int		master_snr_online, slave_snr_online = 1, is_two_devs;

	is_two_devs = (viCfg->s32WorkingViNum == 2) ? true : false;

	/************************************************
	 * step1:  Get input size
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(iniCfg->enSnsType[0], &enPicSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VI_GetSizeBySensor failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSizeSns0);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
		return s32Ret;
	}

	if (is_two_devs) {
		s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(iniCfg->enSnsType[1], &enPicSize);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VI_GetSizeBySensor failed with %#x\n", s32Ret);
			return s32Ret;
		}

		s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSizeSns1);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
			return s32Ret;
		}
	}

	/************************************************
	 * step2:  Config vpss offline/online mode
	 ************************************************/
	SAMPLE_PRT("Master snr vpss Offline(0)/Online(1):\n");
	scanf("%d", &master_snr_online);
	if (is_two_devs) {
		SAMPLE_PRT("Slave sensor Offline(0)/Online(1):\n");
		scanf("%d", &slave_snr_online);
	}

	if (master_snr_online) {
		stVIVPSSMode.aenMode[0] = stVIVPSSMode.aenMode[1] = VI_OFFLINE_VPSS_ONLINE;
	} else {
		stVIVPSSMode.aenMode[0] = stVIVPSSMode.aenMode[1] = VI_OFFLINE_VPSS_OFFLINE;
	}

	if (is_two_devs) {
		if (slave_snr_online) {
			stVIVPSSMode.aenMode[1] = VI_OFFLINE_VPSS_ONLINE;
		} else {
			stVIVPSSMode.aenMode[1] = VI_OFFLINE_VPSS_OFFLINE;
		}
	}

	s32Ret = CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_SetVIVPSSMode failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = VI_Test_Init(viCfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * step3:  Config and init VPSS
	 ************************************************/
	VPSS_GRP	   VpssGrp	  = VPSS_ONLINE_GRP_0;
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_CHN	   VpssChn	  = VPSS_CHN0;
	CVI_BOOL	   abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM] = {0};
	VI_PIPE ViPipe = 0;
	VI_CHN ViChn = 0;

	// snr0
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = SAMPLE_PIXEL_FORMAT;
	stVpssGrpAttr.u32MaxW			     = stSizeSns0.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSizeSns0.u32Height;

	astVpssChnAttr[VpssChn].u32Width		    = stSizeOut.u32Width;
	astVpssChnAttr[VpssChn].u32Height		    = stSizeOut.u32Height;
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
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (!master_snr_online) { //offline
		ViPipe = 0;
		ViChn = 0;
		s32Ret = SAMPLE_COMM_VI_Bind_VPSS(ViPipe, ViChn, VpssGrp);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	if (is_two_devs) {
		// snr1
		VpssGrp = VPSS_ONLINE_GRP_1;
		stVpssGrpAttr.enPixelFormat	= SAMPLE_PIXEL_FORMAT;
		stVpssGrpAttr.u32MaxW		= stSizeSns1.u32Width;
		stVpssGrpAttr.u32MaxH		= stSizeSns1.u32Height;

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

		if (!slave_snr_online) { //offline
			ViPipe = 0;
			ViChn = 1;
			s32Ret = SAMPLE_COMM_VI_Bind_VPSS(ViPipe, ViChn, VpssGrp);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
				return s32Ret;
			}
		}
	}

	/************************************************
	 * step4:  Config and init VO
	 ************************************************/
	SAMPLE_VO_CONFIG_S stVoConfig;
	RECT_S stDefDispRect  = {0, 0, stSizeOut.u32Height, stSizeOut.u32Width};
	SIZE_S stDefImageSize = {stSizeOut.u32Height, stSizeOut.u32Width};
	VO_CHN VoChn = 0;

	s32Ret = SAMPLE_COMM_VO_GetDefConfig(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VO_GetDefConfig failed with %#x\n", s32Ret);
		return s32Ret;
	}

	stVoConfig.VoDev	 = 0;
	stVoConfig.stVoPubAttr.enIntfType  = VO_INTF_MIPI;
	stVoConfig.stVoPubAttr.enIntfSync  = VO_OUTPUT_720x1280_60;
	stVoConfig.stDispRect	 = stDefDispRect;
	stVoConfig.stImageSize	 = stDefImageSize;
	stVoConfig.enPixFormat	 = SAMPLE_PIXEL_FORMAT;
	stVoConfig.enVoMode	 = VO_MODE_1MUX;

	s32Ret = SAMPLE_COMM_VO_StartVO(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VO_StartVO failed with %#x\n", s32Ret);
		return s32Ret;
	}

	VpssGrp = VPSS_ONLINE_GRP_0;
	VpssChn = 0;
	CVI_VO_SetChnRotation(stVoConfig.VoDev, VoChn, ROTATION_90);
	SAMPLE_COMM_VPSS_Bind_VO(VpssGrp, VpssChn, stVoConfig.VoDev, VoChn);

	int sel_snr = 0;

	do {
		SAMPLE_PRT(GREEN "\nSelect vo output sensor:0/1, or 255 exit!\n" NONE);

		scanf("%d", &sel_snr);
		if (sel_snr == 255) {
			break;
		}
		VpssGrp = (sel_snr == 0) ? 0 : 1;
		SAMPLE_COMM_VPSS_UnBind_VO((VpssGrp ^ 1), VpssChn, stVoConfig.VoDev, VoChn);
		SAMPLE_COMM_VPSS_Bind_VO(VpssGrp, VpssChn, stVoConfig.VoDev, VoChn);
	} while (1);

	SAMPLE_COMM_VPSS_UnBind_VO(0, VpssChn, stVoConfig.VoDev, VoChn);
	SAMPLE_COMM_VPSS_UnBind_VO(1, VpssChn, stVoConfig.VoDev, VoChn);
	SAMPLE_COMM_VO_StopVO(&stVoConfig);

	SAMPLE_COMM_VI_UnBind_VPSS(0, 0, 0);
	SAMPLE_COMM_VI_UnBind_VPSS(0, 1, 0);
	SAMPLE_COMM_VI_UnBind_VPSS(0, 0, 1);
	SAMPLE_COMM_VI_UnBind_VPSS(0, 1, 1);

	SAMPLE_COMM_VPSS_Stop(0, abChnEnable);
	SAMPLE_COMM_VPSS_Stop(1, abChnEnable);

	s32Ret = VI_Test_Close(viCfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi close failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

struct vi_test_ops viOps = {
	.vi_open	= VI_Test_Open,
	.vi_init	= VI_Test_Init,
	.vi_close	= VI_Test_Close,
	.vi_ofl_ol_vpss	= VI_Test_OFL_OL_VPSS,
};
