/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_audio_vqe_load.c
 * Description:The VQE function API shall be implemented here
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "cvi_type.h"
#include "cvi_comm_aio.h"
#include "cvi_audio_vqe_load.h"

#define VQE_DEFAULT_PARAM_VAL_UNSET (0XF)
#define VQE_DEFAULT_PARAM_ENUM_UNSET (0)
#define VQE_DEFAULT_PARAM_STRING_UNSET "None"

AI_TALKVQE_CONFIG_S  ZKT_VQE_CONFIG = {
	.para_client_config = 4,
	.u32OpenMask = 207,
	.s32WorkSampleRate = 8000,
	.stAecCfg = {13, 37, 60},
	.stAnrCfg = {15, 0},
	.stAgcCfg = {0, 2, 72, 1},
	.stAecDelayCfg = {13, 6, 450},
	.s32RevMask = VQE_DEFAULT_PARAM_VAL_UNSET,//turn this flag to default 0x11
	.para_notch_freq = 0,//user can ignore this flag
	.customize = VQE_DEFAULT_PARAM_STRING_UNSET,
};

AI_TALKVQE_CONFIG_S  CEOP_VQE_CONFIG = {
	.para_client_config = 1,
	.u32OpenMask = 0x3C,
	.s32WorkSampleRate = 8000,
	.stAecCfg = {13, 37, 60},
	.stAnrCfg = {10, 0},
	.stAgcCfg = {0, 2, 6, 1},
	.stAecDelayCfg = {2, 1, 1},
	.s32RevMask = VQE_DEFAULT_PARAM_VAL_UNSET,
	.para_notch_freq = 1,
	.customize = VQE_DEFAULT_PARAM_STRING_UNSET,
};

AI_TALKVQE_CONFIG_S  HT_VQE_CONFIG = {
	.para_client_config = 2,
	.u32OpenMask = 15,
	.s32WorkSampleRate = 8000,
	.stAecCfg = {2, 37, 60},
	.stAnrCfg = {15, 0},
	.stAgcCfg = {0, 18, 66, 1},
	.stAecDelayCfg = {2, 1, 1},
	.s32RevMask = VQE_DEFAULT_PARAM_VAL_UNSET,
	.para_notch_freq = 0,
	.customize = VQE_DEFAULT_PARAM_STRING_UNSET,
};


CVI_S32 CVI_AUD_VQE_SaveParamToCfg(FILE *fp, CVI_VOID *param)
{
	if (fp == CVI_NULL) {
		printf("cviaudio Error[%s][%d]\n", __func__, __LINE__);
		return CVI_FAILURE;
	}

	if (fp != CVI_NULL) {
		fwrite(param, sizeof(AI_TALKVQE_CONFIG_S), 1, fp);
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_AUD_VQE_LoadParamFromCfg(FILE *fp, CVI_U8 *buf)
{
	if (fp == CVI_NULL) {
		printf("cviaudio Error[%s][%d]\n", __func__, __LINE__);
		return CVI_FAILURE;
	}

	fread(buf, sizeof(AI_TALKVQE_CONFIG_S), 1, fp);
	return CVI_SUCCESS;

}
