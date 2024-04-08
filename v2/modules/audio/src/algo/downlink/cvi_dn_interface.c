/**
 *Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
 * \brief Application interface for cvitek audio
 * \author cvitek
 * \date 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include "cvi_dn_interface.h"
#include "cvi_comm_aio.h"

void *cvi_audio_pp_init(void *pstdnalgoparam)
{
	AO_VQE_CONFIG_S *param = (AO_VQE_CONFIG_S *)pstdnalgoparam;
	int samplerate = param->s32WorkSampleRate;

	pp_handle_t pHandle = (pp_handle_t)malloc(sizeof(pp_handle));
	if (pHandle == NULL) {
		printf("malloc handle error\n");
		return NULL;
	}

	pHandle->channel = 1;
	pHandle->in_channel = param->s32channels;
	pHandle->input = (float *)malloc(sizeof(float) * FRAME_SIZE * pHandle->in_channel);
	pHandle->output = (float *)malloc(sizeof(float) * FRAME_SIZE * pHandle->channel);
	pHandle->input_gain = 1.0;
	pHandle->flag = param->u32OpenMask;

	CVI_ST_FILTER_PARAM stFilterParams;
	stFilterParams.type = param->stHpfParam.type;
	stFilterParams.f0 = param->stHpfParam.f0;
	stFilterParams.Q = param->stHpfParam.Q;
	stFilterParams.gainDb = param->stHpfParam.gainDb;

	printf("[rachel][dnvqe] param\n");
	printf("[rachel] flag:%d\n", pHandle->flag);
	printf("[rachel] channel_in:%d\n", pHandle->in_channel);
	printf("[rachel] channel_out:%d\n", pHandle->channel);
	printf("[rachel] samplerate:%d\n", samplerate);
	printf("\n");
	printf("[rachel] HPF:\n");
	printf("[rachel] f0/freq:%f\n", stFilterParams.f0);
	printf("[rachel] Q:%f\n", stFilterParams.Q);
	printf("[rachel] gainDb:%f\n", stFilterParams.gainDb);
	printf("[rachel] type:%d\n", stFilterParams.type);

	pHandle->cvi_hpf = cvitek_hpfilter_create(samplerate, pHandle->channel, &stFilterParams);

	CVI_ST_EQ_PARAMS stEqParams;
	stEqParams.bandIdx = param->stEqParam.bandIdx;
	stEqParams.freq = param->stEqParam.freq;
	stEqParams.QValue = param->stEqParam.QValue;
	stEqParams.gainDb = param->stEqParam.gainDb;

	printf("[rachel] EQ:\n");
	printf("[rachel] bandIdx:%d\n", stEqParams.bandIdx);
	printf("[rachel] freq:%d\n", stEqParams.freq);
	printf("[rachel] QValue:%f\n", stEqParams.QValue);
	printf("[rachel] gainDb:%f\n", stEqParams.gainDb);

	pHandle->cvi_eq = cvitek_eq_create(samplerate, pHandle->channel);
	cvitek_eq_set_params(pHandle->cvi_eq, &stEqParams);

	CVI_ST_DRC_LIMITER_PARAM stDrcLimiterParams;

	stDrcLimiterParams.attackTimeMs = param->stDrcLimiter.attackTimeMs;
	stDrcLimiterParams.releaseTimeMs = param->stDrcLimiter.releaseTimeMs;
	stDrcLimiterParams.thresholdDb = param->stDrcLimiter.thresholdDb;
	stDrcLimiterParams.postGain = param->stDrcLimiter.postGain;
	printf("\n");
	printf("[rachel] DRC-Limiter:\n");
	printf("[rachel] attackTimeMs:%d\n", stDrcLimiterParams.attackTimeMs);
	printf("[rachel] releaseTimeMs:%d\n", stDrcLimiterParams.releaseTimeMs);
	printf("[rachel] thresholdDb:%f\n", stDrcLimiterParams.thresholdDb);
	printf("[rachel] postGain:%f\n", stDrcLimiterParams.postGain);
	pHandle->cvi_limiter =  cvitek_drc_create(pHandle->channel, samplerate,
			(void *)&stDrcLimiterParams, CVI_E_DRC_TYPE_LIMITER);

	CVI_ST_DRC_COMPRESSOR_PARAM stDrcCompressParams;

	stDrcCompressParams.attackTimeMs = param->stDrcCompressor.attackTimeMs;
	stDrcCompressParams.releaseTimeMs = param->stDrcCompressor.releaseTimeMs;
	stDrcCompressParams.thresholdDb = param->stDrcCompressor.thresholdDb;
	stDrcCompressParams.ratio = param->stDrcCompressor.ratio;
	printf("\n");
	printf("[rachel] DRC-Compressor:\n");
	printf("[rachel] attackTimeMs:%d\n", stDrcCompressParams.attackTimeMs);
	printf("[rachel] releaseTimeMs:%d\n", stDrcCompressParams.releaseTimeMs);
	printf("[rachel] thresholdDb:%f\n", stDrcCompressParams.thresholdDb);
	printf("[rachel] ratio:%d\n", stDrcCompressParams.ratio);
	pHandle->cvi_compress = cvitek_drc_create(pHandle->channel, samplerate,
			(void *)&stDrcCompressParams, CVI_E_DRC_TYPE_COMPRESSOR);

	CVI_ST_DRC_EXPANDER_PARAM stDrcExpParams;

	stDrcExpParams.attackTimeMs = param->stDrcExpander.attackTimeMs;
	stDrcExpParams.releaseTimeMs = param->stDrcExpander.releaseTimeMs;
	stDrcExpParams.thresholdDb = param->stDrcExpander.thresholdDb;
	stDrcExpParams.minDb = param->stDrcExpander.minDb;
	stDrcExpParams.ratio = param->stDrcExpander.ratio;
	stDrcExpParams.holdTimeMs = param->stDrcExpander.holdTimeMs;
	printf("\n");
	printf("[rachel] DRC-Expander:\n");
	printf("[rachel] attackTimeMs:%d\n", stDrcExpParams.attackTimeMs);
	printf("[rachel] releaseTimeMs:%d\n", stDrcExpParams.releaseTimeMs);
	printf("[rachel] thresholdDb:%f\n", stDrcExpParams.thresholdDb);
	printf("[rachel] minDb:%f\n", stDrcExpParams.minDb);
	printf("[rachel] ratio:%d\n", stDrcExpParams.ratio);
	printf("[rachel] holdTimeMs:%d\n", stDrcExpParams.holdTimeMs);
	pHandle->cvi_expander = cvitek_drc_create(pHandle->channel, samplerate,
			(void *)&stDrcExpParams, CVI_E_DRC_TYPE_EXPANDER);

	return pHandle;
}
int cvi_audio_pp_process(void *handle, void *input, void *output, int frame_size)
{
	pp_handle_t pHandle = (pp_handle_t)handle;
	short *data = (short *)input;
	short *out = (short *)output;
    int ret = 0;

	for (int i = 0; i < frame_size * pHandle->in_channel; i++) {
		pHandle->input[i] = data[i] / 32767.0;
	}
	if (pHandle->in_channel == 2) {
		for (int i = 0; i < frame_size; i++) {
			pHandle->output[i] = (pHandle->input[2 * i + 0] + pHandle->input[2 * i + 1])/2;
		}
	} else {
		for (int i = 0; i < frame_size; i++) {
			pHandle->output[i] = pHandle->input[i];
		}
	}
	//master volume
	for (int i = 0; i < frame_size * pHandle->channel; i++) {
		pHandle->output[i] = pHandle->output[i] * pHandle->input_gain;
	}
	if ((pHandle->flag & 0x1) == 0x1) {//hpf
		ret |= cvitek_hpfilter_process(pHandle->cvi_hpf, pHandle->output, frame_size);
	}
	if ((pHandle->flag & 0x2) == 0x2) {//EQ
		ret |= cvitek_eq_process(pHandle->cvi_eq, pHandle->output, frame_size);
	}
	if ((pHandle->flag & 0x4) == 0x4) {//DRC
		ret |= cvitek_drc_process(pHandle->cvi_expander, pHandle->output, frame_size);
		ret |= cvitek_drc_process(pHandle->cvi_compress, pHandle->output, frame_size);
	}
	if ((pHandle->flag & 0x8) == 0x8) {//Limiter
		cvitek_drc_process(pHandle->cvi_limiter, pHandle->output, frame_size);
	}
	short temp;
	for (int i = 0; i < frame_size * pHandle->channel; i++) {
		if (pHandle->output[i] > 1.0) {
			pHandle->output[i] = 1.0;
		} else if (pHandle->output[i] < -1.0) {
			pHandle->output[i] = -1.0;
		}
		temp = pHandle->output[i] * 32767.0;
		if (pHandle->in_channel == 2) {
			out[2 * i + 0] = temp;
			out[2 * i + 1] = temp;
		} else {
			out[i] = temp;
		}
	}

    return ret;
}
void cvi_audio_pp_deinit(void *handle)
{
	pp_handle_t pHandle = (pp_handle_t)handle;
	EQ_HANDLE pHpf = (EQ_HANDLE)pHandle->cvi_hpf;

	cvitek_hpfilter_destroy(pHpf);

	EQ_HANDLE pEq = (EQ_HANDLE)pHandle->cvi_eq;

	cvitek_eq_destroy(pEq);

	DRC_HANDLE pLimiter = (DRC_HANDLE)pHandle->cvi_limiter;

	cvitek_drc_destroy(pLimiter);

	DRC_HANDLE pCompress = (DRC_HANDLE)pHandle->cvi_compress;

	cvitek_drc_destroy(pCompress);

	DRC_HANDLE pExpander = (DRC_HANDLE)pHandle->cvi_expander;

	cvitek_drc_destroy(pExpander);
	free(pHandle->input);
	free(pHandle->output);
	free(handle);
}

int cvi_audio_pp_set_inputgain(void *handle, float gain)
{
	pp_handle_t pHandle = (pp_handle_t)handle;

	pHandle->input_gain = gain;
	printf("intput gain = %f\n", gain);

	return 0;
}

int cvi_audio_pp_set_flag(void *handle, int flag)
{
	pp_handle_t pHandle = (pp_handle_t)handle;

	pHandle->flag = flag;
	printf("pp flag = 0x%x\n", flag);

	return 0;
}

int cvi_audio_pp_set_eq_params(void *handle, CVI_ST_EQ_PARAMS_PTR pstEqParams)
{
	pp_handle_t pHandle = (pp_handle_t)handle;
	EQ_HANDLE pEq = (EQ_HANDLE)pHandle->cvi_eq;

	cvitek_eq_set_params(pEq, pstEqParams);
	return 0;
}
int cvi_audio_pp_get_eq_params(void *handle, CVI_ST_EQ_PARAMS_PTR pstEqParams)
{
	pp_handle_t pHandle = (pp_handle_t)handle;
	EQ_HANDLE pEq = (EQ_HANDLE)pHandle->cvi_eq;

	cvitek_eq_get_params(pEq, pstEqParams);

	return 0;
}
int cvi_audio_pp_set_drc_params(void *handle, void *pstDrcParams, int eDrcType)
{
	pp_handle_t pHandle = (pp_handle_t)handle;
	DRC_HANDLE pDrc = (DRC_HANDLE)pHandle->cvi_compress;

	switch (eDrcType) {
	case CVI_E_DRC_TYPE_COMPRESSOR:
		pDrc = (DRC_HANDLE)pHandle->cvi_compress;
		break;
	case CVI_E_DRC_TYPE_LIMITER:
		pDrc = (DRC_HANDLE)pHandle->cvi_limiter;
		break;
	case CVI_E_DRC_TYPE_EXPANDER:
		pDrc = (DRC_HANDLE)pHandle->cvi_expander;
		break;
	case CVI_E_DRC_TYPE_BUTT:
		break;
	}

	cvitek_set_drc_params(pDrc, pstDrcParams, eDrcType);

	return 0;
}

int cvi_audio_pp_get_drc_params(void *handle, void *pstDrcParams, int eDrcType)
{
	pp_handle_t pHandle = (pp_handle_t)handle;
	DRC_HANDLE pDrc = (DRC_HANDLE)pHandle->cvi_compress;

	switch (eDrcType) {
	case CVI_E_DRC_TYPE_COMPRESSOR:
		pDrc = (DRC_HANDLE)pHandle->cvi_compress;
		break;
	case CVI_E_DRC_TYPE_LIMITER:
		pDrc = (DRC_HANDLE)pHandle->cvi_limiter;
		break;
	case CVI_E_DRC_TYPE_EXPANDER:
		pDrc = (DRC_HANDLE)pHandle->cvi_expander;
		break;
	case CVI_E_DRC_TYPE_BUTT:
		break;
	}

	cvitek_get_drc_params(pDrc, pstDrcParams, eDrcType);

	return 0;
}

int cvi_audio_set_hpf_params(void *handle, unsigned int freq)
{
	pp_handle_t pHandle = (pp_handle_t)handle;

	CVI_HPF_HANDLE pHpf = (CVI_HPF_HANDLE)pHandle->cvi_hpf;

	cvitek_hpfilter_set_params(pHpf, freq);

	return 0;
}

int cvi_audio_get_hpf_freq(void *handle)
{
	pp_handle_t pHandle = (pp_handle_t)handle;

	CVI_HPF_HANDLE pHpf = (CVI_HPF_HANDLE)pHandle->cvi_hpf;

	return cvitek_hpfilter_get_freq(pHpf);
}

