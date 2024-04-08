// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_audio_loadcheck.c
 * Description: audio transcode function interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/times.h>
#include <string.h>
#include "cvi_audio_loadcheck.h"

#ifndef CHECK_TRUE
#define CHECK_TRUE 1
#endif

#ifndef CHECK_FALSE
#define CHECK_FALSE 0
#endif

#ifndef CVIAUDIO_CALLOC
#define CVIAUDIO_CALLOC(TYPE, COUNT) ((TYPE *)calloc(COUNT, sizeof(TYPE)))
#endif

#ifndef CVIAUDIO_FREE_BUF
#define CVIAUDIO_FREE_BUF(OBJ) {if (NULL != OBJ) {free(OBJ); OBJ = NULL; } }
#endif



typedef struct _ST_CVIAUDIO_LOAD_CHECK {
	//user config------------start
	int SampleRate;
	int ChnCnt;
	unsigned int MeasureInterval; //in seconds
	//user config------------end

	//internal param --------------start
	int OneSecData;
	unsigned int DataBytesAccumulate;
	unsigned int CurCnt;
	unsigned int DataIntervalCnt;
	int bResetStartTimer;
	struct tms vStartTime;
	struct tms vStopTime;
	char name[128];
	//internal param --------------end
} ST_CVIAUDIO_LOAD_CHECK;
void *CVI_AUDIO_LoadingCheck_Init(int samplerate,
				int channel_cnt,
				int measure_interval_sec,
				char *name)
{
	ST_CVIAUDIO_LOAD_CHECK *phandle;
	ST_CVIAUDIO_LOAD_CHECK *pLoadCheck;

	phandle = CVIAUDIO_CALLOC(ST_CVIAUDIO_LOAD_CHECK, 1);
	pLoadCheck = phandle;
	//update the parameters from user
	pLoadCheck->SampleRate = samplerate;
	pLoadCheck->ChnCnt = channel_cnt;
	pLoadCheck->MeasureInterval = measure_interval_sec;
	//update the default parameters internally
	pLoadCheck->OneSecData = samplerate * channel_cnt * 2;
	pLoadCheck->DataBytesAccumulate = 0;
	pLoadCheck->CurCnt = 0;
	pLoadCheck->DataIntervalCnt = 0;
	pLoadCheck->bResetStartTimer = CHECK_TRUE;
	strcpy(pLoadCheck->name, name);
	//printf("[cviaudio]sr:%d, chn_cnt:%d, interval:%dsec, onesec_data bytes:%d\n",
	//	samplerate, channel_cnt, pLoadCheck->MeasureInterval, pLoadCheck->OneSecData);

	return phandle;
}

int CVI_AUDIO_LoadingCheck_Begin(void *phandle)
{
	ST_CVIAUDIO_LOAD_CHECK *pLoadCheck = (ST_CVIAUDIO_LOAD_CHECK *)phandle;

	if (pLoadCheck == NULL) {
		//printf("[cviaudio][Error]Null pt detect[%s][%d]\n", __func__, __LINE__);
		return -1;
	}
	if (pLoadCheck->bResetStartTimer == CHECK_TRUE) {
		times(&pLoadCheck->vStartTime);
		pLoadCheck->bResetStartTimer = CHECK_FALSE;
	}
	return 0;
}

int CVI_AUDIO_LoadingCheck_End(void *phandle, unsigned int bytes_proceed)
{
	ST_CVIAUDIO_LOAD_CHECK *pLoadCheck = (ST_CVIAUDIO_LOAD_CHECK *)phandle;

	if (pLoadCheck == NULL) {
		//printf("[cviaudio][Error]Null pt detect[%s][%d]\n", __func__, __LINE__);
		return -1;
	}
	if (pLoadCheck->OneSecData == 0) {
		printf("[cviaudio][Error]Onesecond data abnormal[%d]\n", pLoadCheck->OneSecData);
		return -1;
	}
	pLoadCheck->DataBytesAccumulate += bytes_proceed;
	pLoadCheck->CurCnt = pLoadCheck->DataBytesAccumulate / pLoadCheck->OneSecData;
	if (pLoadCheck->CurCnt - pLoadCheck->DataIntervalCnt == pLoadCheck->MeasureInterval) {
		times(&pLoadCheck->vStopTime);
		clock_t time_tick_diff = (pLoadCheck->vStopTime.tms_utime - pLoadCheck->vStartTime.tms_utime);

		if ((time_tick_diff)/pLoadCheck->MeasureInterval != 0) {
			printf("name[%s]->bytes[%d] time interval[%ld]ticks cpu[%ld]%% measure every[%d]sec\n",
			pLoadCheck->name,
			pLoadCheck->OneSecData,
			time_tick_diff,
			(time_tick_diff)/pLoadCheck->MeasureInterval,
			pLoadCheck->MeasureInterval);
		} else {
			//for the precision
			float check = time_tick_diff;

			printf("name[%s]->bytes[%d] time interval[%ld]ticks cpu[%f]%% measure every[%d]sec\n",
			pLoadCheck->name,
			pLoadCheck->OneSecData,
			time_tick_diff,
			(check)/pLoadCheck->MeasureInterval,
			pLoadCheck->MeasureInterval);

		}

		pLoadCheck->DataIntervalCnt = pLoadCheck->CurCnt;
		pLoadCheck->bResetStartTimer = CHECK_TRUE;
	}
	return 0;

}
int CVI_AUDIO_LoadingCheck_DeInit(void *phandle)
{
	CVIAUDIO_FREE_BUF(phandle);

	return 0;
}


