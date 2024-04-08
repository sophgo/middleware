#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<pthread.h>
#include<math.h>
#include <unistd.h>
#include <stdint.h>
#include<pthread.h>
#include "cvi_audio_fader.h"

typedef enum {
	FADER_TYPE_LINE,
	FADER_TYPE_CUBIC,
} CVI_E_FADER_TYPE;
typedef struct CVI_ST_FADER_PARAMtag {
	//float attuationDb;
	CVI_E_FADER_TYPE type;
	unsigned long timeMs;
} CVI_ST_FADER_PARAM;

typedef struct CVI_ST_FADER_INFOtag {
	CVI_ST_FADER_PARAM faderParams;
	unsigned long timeInSample;
	float curVolumDb;
	float curGain;
	float startGain;
	float targetGain;
	unsigned long curSample;
	unsigned long sampleRate;
	float *segGain;
	unsigned short segNum;
	int channels;
	CVI_E_FADE_MODE mode;
	int bDone;
} CVI_ST_FADER_INFO, *CVI_ST_FADER_INFO_PTR;

CVI_ST_FADER_INFO gFaderHandle;
unsigned char fgEnd;


float cvi_mapSegGainToRealGain(CVI_ST_FADER_INFO *pFaderHandle, float segGain)
{
	float deltaGain = pFaderHandle->targetGain - pFaderHandle->startGain;
	float realGain = deltaGain * segGain + pFaderHandle->startGain;
	return realGain;
}
void cvi_faderPrepareShape(CVI_ST_FADER_INFO *pFaderHandle, unsigned short segNum)
{
	unsigned short segIdx;
	float tmp;

	pFaderHandle->segGain = (float *)malloc((segNum + 1) * sizeof(float));
	pFaderHandle->segNum = segNum;
	if (pFaderHandle->faderParams.type != FADER_TYPE_CUBIC)
		return;
	//0~1 divide into N seg.
	for (segIdx = 0; segIdx < segNum + 1; segIdx++) {
		tmp = (float)segIdx / segNum;
		pFaderHandle->segGain[segIdx] = tmp * tmp * tmp;
		pFaderHandle->segGain[segIdx] = cvi_mapSegGainToRealGain(pFaderHandle, pFaderHandle->segGain[segIdx]);
	}
}
static float cvi_dbToGain(float db)
{
	return pow(10, db/20);
}




void cvi_faderCalGain(CVI_ST_FADER_INFO *pFaderHandle)
{
	float startGainInCurSeg, endGainInCurSeg, step;
	float deltaGain = pFaderHandle->targetGain - pFaderHandle->startGain;
	unsigned long samplesInSeg = pFaderHandle->timeInSample / pFaderHandle->segNum;
	unsigned short curSeg = (float)pFaderHandle->curSample / samplesInSeg;
	unsigned long startSampleInCurSeg = samplesInSeg * curSeg;

	switch (pFaderHandle->faderParams.type) {
	case FADER_TYPE_LINE:
		step = deltaGain / pFaderHandle->timeInSample;
		pFaderHandle->curGain += deltaGain / pFaderHandle->timeInSample;
//pFaderHandle->curGain = pFaderHandle->startGain + deltaGain *
//pFaderHandle->curSample / pFaderHandle->timeInSample;
		break;
	case FADER_TYPE_CUBIC:
		startGainInCurSeg = pFaderHandle->segGain[curSeg];
		endGainInCurSeg = pFaderHandle->segGain[curSeg + 1];
		step = (endGainInCurSeg - startGainInCurSeg) / samplesInSeg;
		if (pFaderHandle->curSample == startSampleInCurSeg)
			pFaderHandle->curGain = startGainInCurSeg;
		else
			pFaderHandle->curGain += step;
		break;
	}
	if ((pFaderHandle->mode == FADE_MODE_FADEOUT)
		&& (pFaderHandle->curGain < 0.000000)) {
		pFaderHandle->curGain = 0.0f;
	} else if ((pFaderHandle->mode == FADE_MODE_FADEIN)
		&& (pFaderHandle->curGain > 1.000000)) {
		pFaderHandle->curGain = 1.0f;
	}
//printf("curGain:%f, curSample:%ld, timeInSample:%ld, curSeg:%d, startGain:%f, endGain:%f\n",
//pFaderHandle->curGain, pFaderHandle->curSample, pFaderHandle->timeInSample, curSeg,
//startGainInCurSeg, endGainInCurSeg);
}

CVI_FADER_HANDLE cvi_faderInit(CVI_E_FADE_MODE mode, int channels, unsigned long sampleRate, int fadeTimeMs)
{
	CVI_ST_FADER_INFO *pFaderHandle = (CVI_ST_FADER_INFO_PTR)malloc(sizeof(CVI_ST_FADER_INFO));
	int TargetDb;

	if (mode >= FADE_MODE_BUTT) {
		return NULL;
	}
	if (fadeTimeMs <= 0 || fadeTimeMs > 5000)
		fadeTimeMs = 500;
	if (mode == FADE_MODE_FADEOUT) {
		TargetDb = -127;
		pFaderHandle->curGain = pFaderHandle->startGain = 1;
	} else {
		TargetDb = 0;
		pFaderHandle->curGain = pFaderHandle->startGain = 0;
	}
	pFaderHandle->faderParams.type = FADER_TYPE_LINE;
	pFaderHandle->faderParams.timeMs = fadeTimeMs;
	pFaderHandle->timeInSample = fadeTimeMs * sampleRate / 1000;
	pFaderHandle->targetGain = cvi_dbToGain(TargetDb);
	pFaderHandle->curSample = 0;
	pFaderHandle->channels = channels;
	pFaderHandle->mode = mode;
	pFaderHandle->bDone = 0;
	cvi_faderPrepareShape(pFaderHandle, 20);

	return (void *)pFaderHandle;
}


int cvi_fader_reset(CVI_FADER_HANDLE pFaderHandle)
{
	CVI_ST_FADER_INFO *pstFaderInfo = (CVI_ST_FADER_INFO *)pFaderHandle;

	if (!pstFaderInfo) {
		printf("invalid params.\n");
		return -1;
	}
	pstFaderInfo->curSample = 0;
	if (pstFaderInfo->mode == FADE_MODE_FADEOUT) {
		pstFaderInfo->curGain = 1;
	} else if (pstFaderInfo->mode == FADE_MODE_FADEIN) {
		pstFaderInfo->curGain = 0;
	}
	pstFaderInfo->bDone = 0;

	return 0;
}



int cvi_fader_process(CVI_FADER_HANDLE pFaderHandle, short *data, int frames)
{
	unsigned short sampleIdx, chIdx;
	short *pData = data;

	CVI_ST_FADER_INFO *pstFaderInfo = (CVI_ST_FADER_INFO *)pFaderHandle;

	if (!pstFaderInfo || !data) {
		printf("invalid params.\n");
		return -1;
	}
	if (pstFaderInfo->bDone) {
		return 0;
	}
	for (sampleIdx = 0; sampleIdx < frames; sampleIdx++) {
		if (pstFaderInfo->curSample != pstFaderInfo->timeInSample) {
			cvi_faderCalGain(pstFaderInfo);
			pstFaderInfo->curSample++;
		} else {
			pstFaderInfo->bDone = 1;
		}
		for (chIdx = 0; chIdx < pstFaderInfo->channels; chIdx++) {
			pData[chIdx + pstFaderInfo->channels * sampleIdx] *= pstFaderInfo->curGain;
		}

	}

	return 0;
}


void cvi_fader_destroy(CVI_FADER_HANDLE pFaderHandle)
{
	CVI_ST_FADER_INFO *pstFaderInfo = (CVI_ST_FADER_INFO *)pFaderHandle;

	if (pstFaderInfo) {
		if (pstFaderInfo->segGain)
			free(pstFaderInfo->segGain);
		free(pFaderHandle);
	}

}

