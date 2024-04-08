/**
 *Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 * \brief Application interface for cvitek audio
 * \author yike <ke.yi@cvitek.com>
 * \author cvitek
 * \date 2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include "cvi_hpf.h"
#include "alog.h"

#define Qfactor (20)

typedef enum {
	CVI_E_FILTER_LPF,
	CVI_E_FILTER_HPF,
	CVI_E_FILTER_LSF,
	CVI_E_FILTER_HSF,
	CVI_E_FILTER_PEF,
	CVI_E_FILTER_MAX,
} CVI_E_FILTER_TYPE;

typedef struct {
	float a[3];
	float b[3];
	int a_x[3];
	int b_x[3];
} CVI_ST_FILTER_COEFF;


#define CVI_FILTER_MAX_CH 2

typedef struct {
	float xHistory[CVI_FILTER_MAX_CH][3];
	float yHistory[CVI_FILTER_MAX_CH][3];
} CVI_ST_FILTER_HISTORY;



typedef struct {
	CVI_E_FILTER_TYPE eFilterType;
	CVI_ST_FILTER_PARAM filterParams;
	CVI_ST_FILTER_COEFF filterCoeff;
	CVI_ST_FILTER_HISTORY filterHistory;
} CVI_ST_FILTER_HANDLE;


typedef struct CVI_ST_FILTER_INFOtag {
	int channels;
	unsigned int freq;
	int samplerate;
	CVI_ST_FILTER_HANDLE filterHandle;
} CVI_ST_FILTER_INFO, *CVI_ST_FILTER_INFO_PTR;



static void __filter_init(CVI_ST_FILTER_PARAM *pFilterParams,
						int samplerate,
						CVI_ST_FILTER_HANDLE *pFilterHandle)
{
	float A = pow(10, pFilterParams->gainDb / 40);
	float w0 = 2 * 3.1415926 * pFilterParams->f0 / samplerate;
	float cos_w0 = cos(w0);
	float sin_w0 = sin(w0);
	float alpha = sin_w0 / (2 * pFilterParams->Q);
	float *a = pFilterHandle->filterCoeff.a;
	float *b = pFilterHandle->filterCoeff.b;

	pFilterHandle->eFilterType = pFilterParams->type;

	memcpy(&(pFilterHandle->filterParams), pFilterParams, sizeof(CVI_ST_FILTER_PARAM));
	memset(&(pFilterHandle->filterHistory), 0, sizeof(CVI_ST_FILTER_HISTORY));

	switch (pFilterParams->type) {
	case CVI_E_FILTER_LPF:
			b[0] = (1 - cos_w0) / 2;
			b[1] = 1 - cos_w0;
			b[2] = (1 - cos_w0) / 2;
			a[0] = 1 + alpha;
			a[1] =	-2 * cos_w0;
			a[2] = 1 - alpha;
			break;
	case CVI_E_FILTER_HPF:
			b[0] = (1 + cos_w0) / 2;
			b[1] = -(1 + cos_w0);
			b[2] = (1 + cos_w0) / 2;
			a[0] = 1 + alpha;
			a[1] =	-2 * cos_w0;
			a[2] = 1 - alpha;
			break;
	case CVI_E_FILTER_LSF:
			b[0] = A * ((A + 1) - (A - 1) * cos_w0 + 2 * sqrt(A) * alpha);
			b[1] = 2 * A * ((A - 1) - (A + 1) * cos_w0);
			b[2] = A * ((A + 1) - (A - 1) * cos_w0 - 2 * sqrt(A) * alpha);
			a[0] = (A + 1) + (A - 1) * cos_w0 + 2 * sqrt(A) * alpha;
			a[1] = -2 * ((A - 1) + (A + 1) * cos_w0);
			a[2] = (A + 1) + (A - 1) * cos_w0 - 2 * sqrt(A) * alpha;
			break;
	case CVI_E_FILTER_HSF:
			b[0] = A * ((A + 1) + (A - 1) * cos_w0 + 2 * sqrt(A) * alpha);
			b[1] = -2 * A * ((A - 1) + (A + 1) * cos_w0);
			b[2] = A * ((A + 1) + (A - 1) * cos_w0 - 2 * sqrt(A) * alpha);
			a[0] = (A + 1) - (A - 1) * cos_w0 + 2 * sqrt(A) * alpha;
			a[1] = 2 * ((A - 1) - (A + 1) * cos_w0);
			a[2] = (A + 1) - (A - 1) * cos_w0 - 2 * sqrt(A) * alpha;
			break;
	case CVI_E_FILTER_PEF:
			b[0] = 1 + alpha * A;
			b[1] = -2 * cos_w0;
			b[2] = 1 - alpha * A;
			a[0] = 1 + alpha / A;
			a[1] = -2 * cos_w0;
			a[2] = 1 - alpha / A;
			break;
	default:
			break;
	}
	b[0] /= a[0];
	b[1] /= a[0];
	b[2] /= a[0];
	a[1] /= a[0];
	a[2] /= a[0];
	a[0] = 1;

}

CVI_HPF_HANDLE cvitek_hpfilter_create(int samplerate, int channels, CVI_ST_FILTER_PARAM *pstFilterParams)
{
	CVI_ST_FILTER_INFO *pstFilterInfo = (CVI_ST_FILTER_INFO *)malloc(sizeof(CVI_ST_FILTER_INFO));
	CVI_ST_FILTER_PARAM stFilterParams;

	if (!pstFilterInfo) {
		log_error("malloc failed.\n");
		return NULL;
	}

	CVI_ST_FILTER_PARAM *pFilterParams = &stFilterParams;

	memcpy(pFilterParams, pstFilterParams, sizeof(CVI_ST_FILTER_PARAM));
	pstFilterInfo->samplerate = samplerate;
	pstFilterInfo->channels = channels;
	pstFilterInfo->freq = pFilterParams->f0;
	CVI_ST_FILTER_HANDLE *pFilterHandle = &pstFilterInfo->filterHandle;

	__filter_init(pFilterParams, pstFilterInfo->samplerate, pFilterHandle);

	return (void *)pstFilterInfo;
}


int cvitek_hpfilter_set_params(CVI_HPF_HANDLE handle, unsigned int freq)
{
	CVI_ST_FILTER_INFO *pstFilterInfo = (CVI_ST_FILTER_INFO *)handle;
	CVI_ST_FILTER_PARAM stFilterParams;
	CVI_ST_FILTER_PARAM *pFilterParams = &stFilterParams;

	if (!pstFilterInfo) {
		log_error("invalid params.\n");
		return -1;
	}
	if (pstFilterInfo->freq == freq) {
		log_error("current freq is %d.\n", freq);
		return -2;
	}

	pFilterParams->f0 = freq;
	pFilterParams->Q = 0.707;
	pFilterParams->gainDb = 0;
	pFilterParams->type = CVI_E_FILTER_HPF;

	CVI_ST_FILTER_HANDLE *pFilterHandle = &pstFilterInfo->filterHandle;

	pstFilterInfo->freq = freq;

	__filter_init(pFilterParams, pstFilterInfo->samplerate, pFilterHandle);

	return 0;
}


int cvitek_hpfilter_get_freq(CVI_HPF_HANDLE handle)
{
	CVI_ST_FILTER_INFO *pstFilterInfo = (CVI_ST_FILTER_INFO *)handle;

	if (!pstFilterInfo) {
		log_error("invalid params.\n");
		return -1;
	}

	return pstFilterInfo->freq;
}


static float cvi_filterCore(CVI_ST_FILTER_HANDLE *pFilterHandle, float curSampleValue, short curChIdx)
{
	float *x, *y;
	float *a, *b;
	float sum = 0;

	x = pFilterHandle->filterHistory.xHistory[curChIdx];
	y = pFilterHandle->filterHistory.yHistory[curChIdx];
	a = pFilterHandle->filterCoeff.a;
	b = pFilterHandle->filterCoeff.b;
	x[0] = curSampleValue;
	//y[0] = (b[0] * x[0] + b[1] * x[1] + b[2] * x[2] - a[1] * y[1] - a[2] * y[2]) / a[0];
	sum += b[0] * x[0];
	sum += b[1] * x[1];
	sum += b[2] * x[2];
	sum += -a[1] * y[1];
	sum += -a[2] * y[2];
	y[0] = sum;
	x[2] = x[1];
	x[1] = x[0];
	y[2] = y[1];
	y[1] = y[0];
	return y[0];
}

int cvitek_hpfilter_process(CVI_HPF_HANDLE handle, float *data, int frames)
{
	unsigned short sampleIdx, chIdx;
	CVI_ST_FILTER_INFO_PTR pFilterHandle = (CVI_ST_FILTER_INFO_PTR)handle;
	float *pData = data;

	if (!handle || !data) {
		log_error("invalid params.\n");
		return -1;
	}

	for (chIdx = 0; chIdx < pFilterHandle->channels; chIdx++) {
		for (sampleIdx = 0; sampleIdx < frames; sampleIdx++) {
			pData[chIdx + pFilterHandle->channels * sampleIdx] =
				cvi_filterCore(&pFilterHandle->filterHandle,
				pData[chIdx + pFilterHandle->channels * sampleIdx], chIdx);
		}
	}

	return 0;
}


void cvitek_hpfilter_destroy(CVI_HPF_HANDLE handle)
{
	if (!handle) {
		log_error("invalid params.\n");
		return;
	}
	free(handle);
}


