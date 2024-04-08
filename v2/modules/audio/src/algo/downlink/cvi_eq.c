#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include "cvi_eq.h"
#include "alog.h"

#define EQ_PROCESS_SAMPLES 160

#undef Qfactor
#define Qfactor (20)

typedef struct {
	float a[3];
	float b[3];
	int a_x[3];
	int b_x[3];
} FILTER_COEFF_T;

typedef enum {
	FILTER_TYPE_LPF,
	FILTER_TYPE_HPF,
	FILTER_TYPE_LSF,
	FILTER_TYPE_HSF,
	FILTER_TYPE_PEF,
	FILTER_TYPE_MAX,
} FILTER_TYPE_E;

typedef struct {
	int type;
	float f0;
	float Q;
	float gainDb;
} FILTER_PARAM_T;

#define FILTER_MAX_CH 2
typedef struct {
	float xHistory[FILTER_MAX_CH][3];
	float yHistory[FILTER_MAX_CH][3];
} FILTER_HISTORY_T;

typedef struct {
	FILTER_TYPE_E eFilterType;
	FILTER_PARAM_T filterParams;
	FILTER_COEFF_T filterCoeff;
	FILTER_HISTORY_T filterHistory;
} FILTER_HANDLE_T;

typedef enum {
	EQ_MODE_ROCK,
	EQ_MODE_POP,
	EQ_MODE_MAX
} EQ_MODE_E;


#define EQ_MAX_BAND 6
typedef struct CVI_ST_EQ_INFOtag {
	int samplerate;
	int channels;
	int samples;
	EQ_MODE_E eEqMode;
	int bParamsChanged;
	pthread_mutex_t lock;
	FILTER_HANDLE_T filterHandles[EQ_MAX_BAND];
	FILTER_PARAM_T	EqFilterParams[EQ_MAX_BAND];
} CVI_ST_EQ_INFO, *CVI_ST_EQ_INFO_PTR;


static FILTER_TYPE_E gEqFilterTypes[EQ_MAX_BAND] = {
	FILTER_TYPE_LSF,
	FILTER_TYPE_PEF,
	FILTER_TYPE_PEF,
	FILTER_TYPE_PEF,
	FILTER_TYPE_PEF,
	FILTER_TYPE_HSF
};

static FILTER_PARAM_T gEqFilterParams[EQ_MODE_MAX][EQ_MAX_BAND] = {
	{
		{FILTER_TYPE_LSF, 100, 2, 9},
		{FILTER_TYPE_PEF, 600, 8, 3},
		{FILTER_TYPE_PEF, 1000, 8, -1},
		{FILTER_TYPE_PEF, 3000, 8, 3},
		{FILTER_TYPE_PEF, 5000, 8, 6},
		{FILTER_TYPE_HSF, 10000, 2, 9},
	},
	{
		{FILTER_TYPE_LSF, 150, 0.707, 6},
		{FILTER_TYPE_PEF, 500, 7, 9},
		{FILTER_TYPE_PEF, 1000, 7, 3},
		{FILTER_TYPE_PEF, 3000, 7, 1},
		{FILTER_TYPE_PEF, 5000, 7, -3},
		{FILTER_TYPE_HSF, 10000, 0.707, -5},
	}
};

static void filterInit(FILTER_HANDLE_T *pFilterHandle, int samplerate, FILTER_PARAM_T *pFilterParams)
{
	float A = pow(10, pFilterParams->gainDb / 40);
	float w0 = 2 * 3.1415926 * pFilterParams->f0 / samplerate;
	float cos_w0 = cos(w0);
	float sin_w0 = sin(w0);
	float alpha = sin_w0 / (2 * pFilterParams->Q);
	float *a = pFilterHandle->filterCoeff.a;
	float *b = pFilterHandle->filterCoeff.b;

	pFilterHandle->eFilterType = pFilterParams->type;
	memcpy(&(pFilterHandle->filterParams), pFilterParams, sizeof(FILTER_PARAM_T));
	memset(&(pFilterHandle->filterHistory), 0, sizeof(FILTER_HISTORY_T));
	switch (pFilterParams->type) {
	case FILTER_TYPE_LPF:
		b[0] = (1 - cos_w0) / 2;
		b[1] = 1 - cos_w0;
		b[2] = (1 - cos_w0) / 2;
		a[0] = 1 + alpha;
		a[1] =  -2 * cos_w0;
		a[2] = 1 - alpha;
		break;
	case FILTER_TYPE_HPF:
		b[0] = (1 + cos_w0) / 2;
		b[1] = -(1 + cos_w0);
		b[2] = (1 + cos_w0) / 2;
		a[0] = 1 + alpha;
		a[1] =  -2 * cos_w0;
		a[2] = 1 - alpha;
		break;
	case FILTER_TYPE_LSF:
		b[0] = A * ((A + 1) - (A - 1) * cos_w0 + 2 * sqrt(A) * alpha);
		b[1] = 2 * A * ((A - 1) - (A + 1) * cos_w0);
		b[2] = A * ((A + 1) - (A - 1) * cos_w0 - 2 * sqrt(A) * alpha);
		a[0] = (A + 1) + (A - 1) * cos_w0 + 2 * sqrt(A) * alpha;
		a[1] = -2 * ((A - 1) + (A + 1) * cos_w0);
		a[2] = (A + 1) + (A - 1) * cos_w0 - 2 * sqrt(A) * alpha;
		break;
	case FILTER_TYPE_HSF:
		b[0] = A * ((A + 1) + (A - 1) * cos_w0 + 2 * sqrt(A) * alpha);
		b[1] = -2 * A * ((A - 1) + (A + 1) * cos_w0);
		b[2] = A * ((A + 1) + (A - 1) * cos_w0 - 2 * sqrt(A) * alpha);
		a[0] = (A + 1) - (A - 1) * cos_w0 + 2 * sqrt(A) * alpha;
		a[1] = 2 * ((A - 1) - (A + 1) * cos_w0);
		a[2] = (A + 1) - (A - 1) * cos_w0 - 2 * sqrt(A) * alpha;
		break;
	case FILTER_TYPE_PEF:
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


float cvi_filterCore(FILTER_HANDLE_T *pFilterHandle, float curSampleValue, short curChIdx)
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

static void cvi_eqFilterInit(CVI_ST_EQ_INFO *pEqHandle)
{
	if (pEqHandle == NULL)
		return;
	FILTER_HANDLE_T *pFilterHandle;
	FILTER_PARAM_T *pFilterParam;
	short bandIdx;

	for (bandIdx = 0; bandIdx < EQ_MAX_BAND; bandIdx++) {
		pFilterHandle = &pEqHandle->filterHandles[bandIdx];
		pFilterParam = &pEqHandle->EqFilterParams[bandIdx];
		filterInit(pFilterHandle, pEqHandle->samplerate, pFilterParam);
	}
}

EQ_HANDLE cvitek_eq_create(int samplerate, int channels)
{
	CVI_ST_EQ_INFO_PTR pstEqHandle = (CVI_ST_EQ_INFO_PTR)malloc(sizeof(CVI_ST_EQ_INFO));

	if (!pstEqHandle) {
		log_error("malloc failed.\n");
		return NULL;
	}
	memset(pstEqHandle, 0, sizeof(CVI_ST_EQ_INFO));

	pthread_mutex_init(&pstEqHandle->lock, NULL);
	pthread_mutex_lock(&pstEqHandle->lock);
	pstEqHandle->eEqMode = EQ_MODE_POP;
	pstEqHandle->samplerate = samplerate;
	pstEqHandle->channels = channels;
	pstEqHandle->samples = EQ_PROCESS_SAMPLES;
	for (int idx = 0; idx < EQ_MAX_BAND; idx++) {
		pstEqHandle->EqFilterParams[idx] = gEqFilterParams[EQ_MODE_POP][idx];
	}
	cvi_eqFilterInit(pstEqHandle);
	pthread_mutex_unlock(&pstEqHandle->lock);

	return pstEqHandle;
}

int cvitek_eq_set_params(EQ_HANDLE EqHandle, CVI_ST_EQ_PARAMS_PTR pstEqParams)
{
	CVI_ST_EQ_INFO_PTR pstEqHandle = (CVI_ST_EQ_INFO_PTR)EqHandle;

	if (!pstEqHandle || !pstEqParams) {
		log_error("invalid params.\n");
		return -1;
	}

	pthread_mutex_lock(&pstEqHandle->lock);
	if (pstEqParams->bandIdx < 0
		|| pstEqParams->bandIdx >= EQ_MAX_BAND) {
		pthread_mutex_unlock(&pstEqHandle->lock);
		log_error("invalid params,bindidx[0-5]\n");
		return -2;
	}

	pstEqHandle->EqFilterParams[pstEqParams->bandIdx].gainDb
		= pstEqParams->gainDb;
	pstEqHandle->EqFilterParams[pstEqParams->bandIdx].f0
		= pstEqParams->freq;
	pstEqHandle->EqFilterParams[pstEqParams->bandIdx].Q
		= pstEqParams->QValue;

	pstEqHandle->bParamsChanged = 1;

	cvi_eqFilterInit(pstEqHandle);
	pthread_mutex_unlock(&pstEqHandle->lock);

	return 0;
}

int cvitek_eq_get_params(EQ_HANDLE EqHandle, CVI_ST_EQ_PARAMS_PTR pstEqParams)
{
	CVI_ST_EQ_INFO_PTR pstEqHandle = (CVI_ST_EQ_INFO_PTR)EqHandle;

	if (!pstEqHandle || !pstEqParams) {
		log_error("invalid params.\n");
		return -1;
	}

	pthread_mutex_lock(&pstEqHandle->lock);

	pstEqParams->gainDb =
		pstEqHandle->EqFilterParams[pstEqParams->bandIdx].gainDb;

	pstEqParams->freq =
		pstEqHandle->EqFilterParams[pstEqParams->bandIdx].f0;
	pstEqParams->QValue =
		pstEqHandle->EqFilterParams[pstEqParams->bandIdx].Q;

	pthread_mutex_unlock(&pstEqHandle->lock);

	return 0;
}

int cvitek_eq_process(EQ_HANDLE EqHandle, float *data, int frames)
{
	unsigned short sampleIdx, chIdx, bandIdx;
	FILTER_HANDLE_T *pFilterHandle;
	float *pData = data;
	int loop = 0;
	CVI_ST_EQ_INFO_PTR pstEqHandle = (CVI_ST_EQ_INFO_PTR)EqHandle;

	if (!pstEqHandle || !data) {
		log_error("invalid params.\n");
		return -1;
	}
	pthread_mutex_lock(&pstEqHandle->lock);

	for (bandIdx = 0; bandIdx < EQ_MAX_BAND; bandIdx++) {
		for (chIdx = 0; chIdx < pstEqHandle->channels; chIdx++) {
			for (sampleIdx = 0; sampleIdx < frames; sampleIdx++) {
				pData[chIdx + pstEqHandle->channels * sampleIdx] =
					cvi_filterCore(&pstEqHandle->filterHandles[bandIdx],
						pData[chIdx + pstEqHandle->channels * sampleIdx], chIdx);
			}
		}
	}
	pthread_mutex_unlock(&pstEqHandle->lock);
	return 0;

}

void cvitek_eq_destroy(EQ_HANDLE EqHandle)
{
	CVI_ST_EQ_INFO_PTR pstEqHandle = (CVI_ST_EQ_INFO_PTR)EqHandle;

	if (!pstEqHandle) {
		log_error("malloc failed.\n");
		return;
	}
	pthread_mutex_destroy(&pstEqHandle->lock);
	free(pstEqHandle);
}

int cvitek_eq_get_bind_cnt(void)
{
	return EQ_MAX_BAND;
}

