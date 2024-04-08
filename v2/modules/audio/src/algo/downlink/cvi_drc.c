/**
 *Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 * \brief Application interface for cvitek audio
 * \author yike <ke.yi@cvitek.com>
 * \author cvitek
 * \date 2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#include "cvi_drc.h"
#include "alog.h"

typedef struct {
	int bInit;
	CVI_E_DRC_TYPE eDrcType;
	pthread_mutex_t lock;
	int chNum;
	union {
		CVI_ST_DRC_COMPRESSOR_PARAM compressorParams;
		CVI_ST_DRC_LIMITER_PARAM limiterParams;
		CVI_ST_DRC_EXPANDER_PARAM expanderParams;
	} uDrcParams;
	float curGain;
	float curSmoothGainDb;
	float alphaAttack;
	float alphaRelease;
	unsigned long attackHoldCounter;
	unsigned long releaseHoldCounter;
	int samplerate;
	float postGain;
} DRC_HANDLE_T;

typedef struct {
	float sampleValue;
	short bytesPerSample;
} SAMPLE_INFO_T;


DRC_HANDLE cvitek_drc_create(int channel, int samplerate, void *pDrcParams, CVI_E_DRC_TYPE eDrcType)
{
	CVI_ST_DRC_COMPRESSOR_PARAM *pCompressorParams;
	CVI_ST_DRC_LIMITER_PARAM *pLimiterParams;
	CVI_ST_DRC_EXPANDER_PARAM *pExpanderParams;

	if (pDrcParams == NULL || eDrcType >= CVI_E_DRC_TYPE_BUTT) {
		log_error("invalid params.\n");
		return NULL;
	}

	DRC_HANDLE_T *pDrcHandle = (DRC_HANDLE_T *)malloc(sizeof(DRC_HANDLE_T));

	if (pDrcHandle == NULL) {
		log_error("malloc faild.\n");
		return NULL;
	}

	memset(pDrcHandle, 0, sizeof(DRC_HANDLE_T));
	pthread_mutex_init(&pDrcHandle->lock, NULL);
	pthread_mutex_lock(&pDrcHandle->lock);
	pDrcHandle->eDrcType = eDrcType;
	pDrcHandle->samplerate = samplerate;
	pDrcHandle->postGain = 1.0f;

	switch (eDrcType) {
	case CVI_E_DRC_TYPE_COMPRESSOR:
		pCompressorParams = (CVI_ST_DRC_COMPRESSOR_PARAM *)pDrcParams;
		memcpy(&pDrcHandle->uDrcParams.compressorParams, pCompressorParams,
				sizeof(CVI_ST_DRC_COMPRESSOR_PARAM));
		pDrcHandle->alphaAttack = expf(-logf(9) /
				(samplerate * pCompressorParams->attackTimeMs / 1000));
		pDrcHandle->alphaRelease = expf(-logf(9) /
				(samplerate * pCompressorParams->releaseTimeMs / 1000));
		break;
	case CVI_E_DRC_TYPE_LIMITER:
		pLimiterParams = (CVI_ST_DRC_LIMITER_PARAM *)pDrcParams;
		memcpy(&pDrcHandle->uDrcParams.limiterParams, pLimiterParams,
				sizeof(CVI_ST_DRC_LIMITER_PARAM));
		pDrcHandle->alphaAttack = expf(-logf(9) /
				(samplerate * pLimiterParams->attackTimeMs / 1000));
		pDrcHandle->alphaRelease = expf(-logf(9) /
				(samplerate * pLimiterParams->releaseTimeMs / 1000));
		pDrcHandle->postGain = pLimiterParams->postGain;
		break;
	case CVI_E_DRC_TYPE_EXPANDER:
		pExpanderParams = (CVI_ST_DRC_EXPANDER_PARAM *)pDrcParams;
		memcpy(&pDrcHandle->uDrcParams.expanderParams, pExpanderParams,
				sizeof(CVI_ST_DRC_EXPANDER_PARAM));
		pDrcHandle->alphaAttack = expf(-logf(9) /
				(samplerate * pExpanderParams->attackTimeMs / 1000));
		pDrcHandle->alphaRelease = expf(-logf(9) /
				(samplerate * pExpanderParams->releaseTimeMs / 1000));
		break;
	case CVI_E_DRC_TYPE_BUTT:
		break;
	}

	pDrcHandle->curGain = 1;
	pDrcHandle->curSmoothGainDb = 0;
	pDrcHandle->attackHoldCounter = 0;
	pDrcHandle->releaseHoldCounter = 0;
	pDrcHandle->chNum = channel;
	pDrcHandle->bInit = 1;

	pthread_mutex_unlock(&pDrcHandle->lock);
	return (void *)pDrcHandle;
}

int cvitek_set_drc_params(DRC_HANDLE DrcHandle, void *pDrcParams, CVI_E_DRC_TYPE eDrcType)
{
	DRC_HANDLE_T *pDrcHandle = (DRC_HANDLE_T *)DrcHandle;
	CVI_ST_DRC_COMPRESSOR_PARAM *pCompressorParams;
	CVI_ST_DRC_LIMITER_PARAM *pLimiterParams;
	CVI_ST_DRC_EXPANDER_PARAM *pExpanderParams;

	if (pDrcHandle == NULL) {
		log_error("invalid params.\n");
		return -1;
	}
	pthread_mutex_lock(&pDrcHandle->lock);
	if (!pDrcHandle->bInit) {
		log_error("not init.\n");
		pthread_mutex_unlock(&pDrcHandle->lock);
		return -2;
	}

	if (eDrcType != pDrcHandle->eDrcType) {
		log_error("current type:%d can not be changed to %d\n", pDrcHandle->eDrcType, eDrcType);
		pthread_mutex_unlock(&pDrcHandle->lock);
		return -3;
	}

	switch (eDrcType) {
	case CVI_E_DRC_TYPE_COMPRESSOR:
		pCompressorParams = (CVI_ST_DRC_COMPRESSOR_PARAM *)pDrcParams;
		memcpy(&pDrcHandle->uDrcParams.compressorParams, pCompressorParams,
				sizeof(CVI_ST_DRC_COMPRESSOR_PARAM));
		pDrcHandle->alphaAttack = expf(-logf(9) /
				(pDrcHandle->samplerate * pCompressorParams->attackTimeMs / 1000));
		pDrcHandle->alphaRelease = expf(-logf(9) /
				(pDrcHandle->samplerate * pCompressorParams->releaseTimeMs / 1000));
		break;
	case CVI_E_DRC_TYPE_LIMITER:
		pLimiterParams = (CVI_ST_DRC_LIMITER_PARAM *)pDrcParams;
		memcpy(&pDrcHandle->uDrcParams.limiterParams, pLimiterParams,
				sizeof(CVI_ST_DRC_LIMITER_PARAM));
		pDrcHandle->alphaAttack = expf(-logf(9) /
				(pDrcHandle->samplerate * pLimiterParams->attackTimeMs / 1000));
		pDrcHandle->alphaRelease = expf(-logf(9) /
				(pDrcHandle->samplerate * pLimiterParams->releaseTimeMs / 1000));
		pDrcHandle->postGain = pLimiterParams->postGain;
		break;
	case CVI_E_DRC_TYPE_EXPANDER:
		pExpanderParams = (CVI_ST_DRC_EXPANDER_PARAM *)pDrcParams;
		memcpy(&pDrcHandle->uDrcParams.expanderParams, pExpanderParams,
				sizeof(CVI_ST_DRC_EXPANDER_PARAM));
		pDrcHandle->alphaAttack = expf(-logf(9) /
				(pDrcHandle->samplerate * pExpanderParams->attackTimeMs / 1000));
		pDrcHandle->alphaRelease = expf(-logf(9) /
				(pDrcHandle->samplerate * pExpanderParams->releaseTimeMs / 1000));
		break;
	case CVI_E_DRC_TYPE_BUTT:
		break;
	}

	pthread_mutex_unlock(&pDrcHandle->lock);
	return 0;
}

int cvitek_get_drc_params(DRC_HANDLE DrcHandle, void *pDrcParams, CVI_E_DRC_TYPE eDrcType)
{
	DRC_HANDLE_T *pDrcHandle = (DRC_HANDLE_T *)DrcHandle;
	CVI_ST_DRC_COMPRESSOR_PARAM *pCompressorParams;
	CVI_ST_DRC_LIMITER_PARAM *pLimiterParams;
	CVI_ST_DRC_EXPANDER_PARAM *pExpanderParams;

	if (pDrcHandle == NULL) {
		log_error("invalid params.\n");
		return -1;
	}

	pthread_mutex_lock(&pDrcHandle->lock);

	switch (eDrcType) {
	case CVI_E_DRC_TYPE_COMPRESSOR:
		pCompressorParams = &pDrcHandle->uDrcParams.compressorParams;
		memcpy(pDrcParams, pCompressorParams, sizeof(CVI_ST_DRC_COMPRESSOR_PARAM));
		//printf("compressor attack time ms= %d\n", pCompressorParams->attackTimeMs);
		//printf("compressor release time ms= %d\n", pCompressorParams->releaseTimeMs);
		//printf("compressor ratio = %d\n", pCompressorParams->ratio);
		//printf("compressor threDb = %f\n", pCompressorParams->thresholdDb);
		break;
	case CVI_E_DRC_TYPE_LIMITER:
		pLimiterParams = &pDrcHandle->uDrcParams.limiterParams;
		memcpy(pDrcParams, pLimiterParams, sizeof(CVI_ST_DRC_LIMITER_PARAM));
		//printf("Limiter attack time ms= %d\n", pLimiterParams->attackTimeMs);
		//printf("Limiter release time ms= %d\n", pLimiterParams->releaseTimeMs);
		//printf("Limiter threDb = %f\n", pLimiterParams->thresholdDb);
		break;
	case CVI_E_DRC_TYPE_EXPANDER:
		pExpanderParams = &pDrcHandle->uDrcParams.expanderParams;
		memcpy(pDrcParams, pExpanderParams, sizeof(CVI_ST_DRC_EXPANDER_PARAM));
		//printf("expander attack time ms= %d\n", pExpanderParams->attackTimeMs);
		//printf("expander release time ms= %d\n", pExpanderParams->releaseTimeMs);
		//printf("expander hold time ms = %d\n", pExpanderParams->holdTimeMs);
		//printf("expander ratio = %d\n", pExpanderParams->ratio);
		//printf("expander minDb = %f\n", pExpanderParams->minDb);
		//printf("expander threDb = %f\n", pExpanderParams->thresholdDb);
		break;
	case CVI_E_DRC_TYPE_BUTT:
		break;
	}

	pthread_mutex_unlock(&pDrcHandle->lock);
	return 0;
}


CVI_E_DRC_TYPE cvitek_get_drc_type(DRC_HANDLE DrcHandle)
{
	DRC_HANDLE_T *pDrcHandle = (DRC_HANDLE_T *)DrcHandle;

	if (pDrcHandle == NULL) {
		log_error("invalid params.\n");
		return CVI_E_DRC_TYPE_BUTT;
	}

	return pDrcHandle->eDrcType;
}

static float cvi_sampleValueToDb(DRC_HANDLE DrcHandle, SAMPLE_INFO_T *pSampleInfo)
{
    DRC_HANDLE_T *pDrcHandle = (DRC_HANDLE_T *)DrcHandle;
    if (pDrcHandle == NULL || pSampleInfo == NULL) {
		log_error("invalid params. pDrcHandle:%p, pSampleInfo:%p\n", pDrcHandle, pSampleInfo);
		return -1;
	}

	//short maxSampleValue = ((1 << (pSampleInfo->bytesPerSample * 8)) - 1) / 2;
	////float db = 20 * log10f((float)abs(pSampleInfo->sampleValue) / maxSampleValue);
	float db;

	if (pDrcHandle->eDrcType == CVI_E_DRC_TYPE_LIMITER) {
		db = 20 * log10f(fabs(pSampleInfo->sampleValue * pDrcHandle->postGain
					* pDrcHandle->curGain + 0.000000001));
	} else {
		db = 20 * log10f(fabs(pSampleInfo->sampleValue + 0.000000001));
	}
	//printf("maxSampleValue:%d, sampleValue:%d, db:%f\n", maxSampleValue, pSampleInfo->sampleValue, db);
	return db;
}

static float cvi_drcComputeGainDb(DRC_HANDLE_T *pDrcHandle, float sampleDb)
{
	if (pDrcHandle == NULL) {
		log_error("invalid params.\n");
		return -1;
	}

	float staticChract = sampleDb;

	switch (pDrcHandle->eDrcType) {
	case CVI_E_DRC_TYPE_COMPRESSOR:
		if (sampleDb < pDrcHandle->uDrcParams.compressorParams.thresholdDb) {
			staticChract = sampleDb;
		} else {
			staticChract = pDrcHandle->uDrcParams.compressorParams.thresholdDb +
			(sampleDb - pDrcHandle->uDrcParams.compressorParams.thresholdDb) /
			pDrcHandle->uDrcParams.compressorParams.ratio;
		}
		break;
	case CVI_E_DRC_TYPE_LIMITER:
		if (sampleDb < pDrcHandle->uDrcParams.limiterParams.thresholdDb) {
			staticChract = sampleDb;
		} else {
			staticChract = pDrcHandle->uDrcParams.limiterParams.thresholdDb;
		}
		break;
	case CVI_E_DRC_TYPE_EXPANDER:
		if ((sampleDb >= pDrcHandle->uDrcParams.expanderParams.thresholdDb)
				|| (sampleDb < pDrcHandle->uDrcParams.expanderParams.minDb
				&& pDrcHandle->curGain > 1)) {
			staticChract = sampleDb;
		} else {
			staticChract = pDrcHandle->uDrcParams.expanderParams.thresholdDb +
			(sampleDb - pDrcHandle->uDrcParams.expanderParams.thresholdDb) /
			pDrcHandle->uDrcParams.expanderParams.ratio;
		}
		break;
	case CVI_E_DRC_TYPE_BUTT:
		break;
	}
	//printf("staticChract:%f, sampleDb:%f\n", staticChract, sampleDb);
	return staticChract - sampleDb;
}

static float cvi_drcCompressorSmoothGain(DRC_HANDLE_T *pDrcHandle, float computeGainDb)
{
	float smoothGainDb;

	if (computeGainDb < pDrcHandle->curSmoothGainDb) {
		smoothGainDb = pDrcHandle->alphaAttack * pDrcHandle->curSmoothGainDb +
		(1 - pDrcHandle->alphaAttack) * computeGainDb;
	} else {
		smoothGainDb = pDrcHandle->alphaRelease * pDrcHandle->curSmoothGainDb +
		(1 - pDrcHandle->alphaRelease) * computeGainDb;
	}

	return smoothGainDb;
}

static float cvi_drcExpanderSmoothGain(DRC_HANDLE_T *pDrcHandle, float computeGainDb)
{
	float smoothGainDb = computeGainDb;
	unsigned long holdTimeInSample = pDrcHandle->uDrcParams.expanderParams.holdTimeMs *
		pDrcHandle->samplerate / 1000;

	if (pDrcHandle->attackHoldCounter >= holdTimeInSample && computeGainDb > pDrcHandle->curSmoothGainDb) {
		smoothGainDb = pDrcHandle->alphaAttack * pDrcHandle->curSmoothGainDb +
			(1 - pDrcHandle->alphaAttack) * computeGainDb;
	} else if (pDrcHandle->attackHoldCounter < holdTimeInSample && computeGainDb > pDrcHandle->curSmoothGainDb) {
		smoothGainDb = pDrcHandle->curSmoothGainDb;
		pDrcHandle->attackHoldCounter++;
		pDrcHandle->releaseHoldCounter = 0;
	} else if (pDrcHandle->releaseHoldCounter >= holdTimeInSample && computeGainDb <= pDrcHandle->curSmoothGainDb) {
		smoothGainDb = pDrcHandle->alphaRelease * pDrcHandle->curSmoothGainDb +
			(1 - pDrcHandle->alphaRelease) * computeGainDb;
	} else if (pDrcHandle->releaseHoldCounter < holdTimeInSample && computeGainDb <= pDrcHandle->curSmoothGainDb) {
		smoothGainDb = pDrcHandle->curSmoothGainDb;
		pDrcHandle->releaseHoldCounter++;
		pDrcHandle->attackHoldCounter = 0;
	}
	return smoothGainDb;
}

static float cvi_drcSmoothGain(DRC_HANDLE_T *pDrcHandle, float computeGainDb)
{
	if (pDrcHandle == NULL) {
		log_error("invalid params.\n");
		return -1;
	}
	float smoothGainDb = computeGainDb;

	switch (pDrcHandle->eDrcType) {
	case CVI_E_DRC_TYPE_COMPRESSOR:
	case CVI_E_DRC_TYPE_LIMITER:
		smoothGainDb = cvi_drcCompressorSmoothGain(pDrcHandle, computeGainDb);
		break;
	case CVI_E_DRC_TYPE_EXPANDER:
		smoothGainDb = cvi_drcExpanderSmoothGain(pDrcHandle, computeGainDb);
		break;
	case CVI_E_DRC_TYPE_BUTT:
		break;
	}
	return smoothGainDb;
}

float cvi_dbToGain(float db)
{
	return pow(10, db / 20);
}


static void cvi_drcCalGain(DRC_HANDLE_T *pDrcHandle, SAMPLE_INFO_T *pSampleInfo)
{
	if (pDrcHandle == NULL || pSampleInfo == NULL) {
		log_error("invalid params.\n");
		return;
	}
	float sampleDb = cvi_sampleValueToDb(pDrcHandle, pSampleInfo);
	float computeGainDb = cvi_drcComputeGainDb(pDrcHandle, sampleDb);

	pDrcHandle->curSmoothGainDb = cvi_drcSmoothGain(pDrcHandle, computeGainDb);
	pDrcHandle->curGain = cvi_dbToGain(pDrcHandle->curSmoothGainDb);
	if (pDrcHandle->eDrcType == CVI_E_DRC_TYPE_LIMITER) {
		pDrcHandle->curGain = pDrcHandle->curGain * pDrcHandle->postGain;
	} else {
		pDrcHandle->curGain = pDrcHandle->curGain * 1.0f;
	}
	//printf("sampleDb:%f, computeGainDb:%f, smoothGainDb:%f, curGain:%f\n",
	//   sampleDb, computeGainDb, pDrcHandle->curSmoothGainDb, pDrcHandle->curGain);
}

int cvitek_drc_process(DRC_HANDLE DrcHandle, float *data, int frames)
{
	float *pData = data;
	DRC_HANDLE_T *pDrcHandle = (DRC_HANDLE_T *)DrcHandle;
	unsigned short sampleIdx, chIdx;
	SAMPLE_INFO_T sampleInfo;

	if (pDrcHandle == NULL) {
		log_error("invalid params.\n");
		return -1;
	}
	pthread_mutex_lock(&pDrcHandle->lock);

	if (!pDrcHandle->bInit) {
		log_error("not init.\n");
		pthread_mutex_unlock(&pDrcHandle->lock);
		return -2;
	}
	for (chIdx = 0; chIdx < pDrcHandle->chNum; chIdx++) {
		for (sampleIdx = 0; sampleIdx < frames; sampleIdx++) {
			sampleInfo.bytesPerSample = 2;
			sampleInfo.sampleValue = pData[chIdx + pDrcHandle->chNum * sampleIdx];
			cvi_drcCalGain(pDrcHandle, &sampleInfo);
			pData[chIdx + pDrcHandle->chNum * sampleIdx] *= pDrcHandle->curGain;
		}
	}
	pthread_mutex_unlock(&pDrcHandle->lock);
	return 0;
}

void cvitek_drc_destroy(DRC_HANDLE DrcHandle)
{
	DRC_HANDLE_T *pDrcHandle = (DRC_HANDLE_T *)DrcHandle;

	if (pDrcHandle == NULL) {
		log_error("invalid params.\n");
		return;
	}
	pthread_mutex_lock(&pDrcHandle->lock);
	pDrcHandle->bInit = 0;
	pthread_mutex_unlock(&pDrcHandle->lock);
	pthread_mutex_destroy(&pDrcHandle->lock);
	free(DrcHandle);
}



