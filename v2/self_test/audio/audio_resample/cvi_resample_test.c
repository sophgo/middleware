/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: sample/cvi_resample_test.c
 * Description:This file show the example of (1)how to resample audio with resample
 * lib and api, user should focus on the api call flow. (2)The unit test bin for audio
 * resample, user can resample audio to other sampling rate by cvi_resample.
 *
 */

/* Include files */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#if defined(__CV181X__) || defined(__CV180X__)
#include <cvi_type.h>
#else
#include "cvi_type.h"
#endif
#ifndef CVIAUDIO_STATIC
#include "cvi_audio_dl_adp.h"
#endif
#include "cvi_resampler_api.h"
//#include "cvi_aud_internal.h"

#define RES_LIB_NAME "libcvi_RES1.so"
#define FILE_NAME_LEN	128
#define SAMPLE_RES_CHECK_NULL_PTR(ptr)\
	do {\
		if (NULL == (CVI_U8 *)ptr)\
			printf("[fatal error]ptr is NULL,fuc:%s,line:%d\n", __func__, __LINE__);\
	} while (0)


typedef CVI_VOID * (*pCVI_Resampler_Create_Callback)(CVI_S32 s32Inrate,
		CVI_S32 s32Outrate, CVI_S32 s32Chans);

typedef CVI_S32(*pCVI_Resampler_Process_Callback)(CVI_VOID *inst,
		CVI_S16 *s16Inbuf, CVI_S32 s32Insamps, CVI_S16 *s16Outbuf);

typedef CVI_VOID(*pCVI_Resampler_Destroy_Callback)(CVI_VOID *inst);

typedef CVI_S32(*pCVI_Resampler_GetMaxOutputNum_Callback)(CVI_VOID *inst,
		CVI_S32 s32Insamps);

struct SAMPLE_RES_FUN_S {
	CVI_VOID *pLibHandle;
	pCVI_Resampler_Create_Callback pCVI_Resampler_Create;
	pCVI_Resampler_Process_Callback pCVI_Resampler_Process;
	pCVI_Resampler_Destroy_Callback pCVI_Resampler_Destroy;
	pCVI_Resampler_GetMaxOutputNum_Callback pCVI_Resampler_GetMaxOutputNum;
};


static int _check_rate_valid(int rate)
{
	if ((rate != 8000) &&
		(rate != 11025) &&
		(rate != 12000) &&
		(rate != 16000) &&
		(rate != 22050) &&
		(rate != 24000) &&
		(rate != 32000) &&
		(rate != 44100) &&
		(rate != 64000) &&
		(rate != 96000) &&
		(rate != 48000))
		return CVI_FALSE;
	else
		return CVI_TRUE;
}

int main(int argc, const char *const argv[])
{

	FILE *pfd_in = NULL;
	FILE *pfd_out = NULL;

	/* Setup frame size information */
	CVI_S32 s32SamplePerFrame = 320;

	/* Setup bit depth information */
	/* cvitek audio resample only support 16 bit depth */
#if 0
	ST_RES_INFO stResInfo;

	stResInfo.dst_sample_fmt = AUDIO_BIT_WIDTH_16;
	if (set_resample_info(stResInfo) == CVI_FAILURE) {
		printf("[Error] Cannot set in resample info...\n");
		return CVI_FAILURE;
	}
#endif

	/* Step1 set up the usage */

	do {
		printf("====================================\n");
		printf("cvi_resample usage :\n");
		printf("sample_audio_resample  [input_file_name.wav] [intput sample_rate] [target sample_rate]\n");
		printf("[input_file_name.wav]: only support .raw data for now\n");
		printf("[input sample_rate]: support: 8000/11025/16000/22050/24000/32000/44100/48000\n");
		printf("[target sample_rate]: support: 8000/11025/16000/22050/24000/32000/44100/48000\n");
		printf(" the file will not be treated as wav file, but as raw data\n");
		printf("Test Result : create the file : [$(var2)_sample_rate.pcm]\n");
		printf("====================================\n");
	} while (0);
	printf("\n");

	if (argc < 3) {
		printf("[Error]Not enough input argument...\n");
		return 0;
	}

	const char *input_filename = argv[1];
	int current_rate = atoi(argv[2]);
	int target_rate = atoi(argv[3]);
	char output_filename[FILE_NAME_LEN] = {0};
	CVI_S32 s32Ret = CVI_FAILURE;
	CVI_S32 s32ReadCheck = 0;
	//CVI_S32 s32InPutSize = 0;
	CVI_U32 u32OutSample = 0;

	/* Step2 check input valid */
	s32Ret = _check_rate_valid(current_rate);

	if (s32Ret == CVI_FALSE) {
		printf("[Error]input sample rate param invalid...\n");
		return 0;
	}

	s32Ret = _check_rate_valid(target_rate);

	if (s32Ret == CVI_FALSE) {
		printf("[Error]target_rate param invalid...\n");
		return 0;
	}

	/* Step 3 link the resample function pointer */
	/* SAMPLE_AUDIO_InitExtResFun */
	struct SAMPLE_RES_FUN_S stSampleResFun;

	memset(&stSampleResFun, 0, sizeof(struct SAMPLE_RES_FUN_S));
#ifdef CVIAUDIO_STATIC
	stSampleResFun.pCVI_Resampler_Create = CVI_Resampler_Create;
	stSampleResFun.pCVI_Resampler_Process = CVI_Resampler_Process;
	stSampleResFun.pCVI_Resampler_Destroy = CVI_Resampler_Destroy;
	stSampleResFun.pCVI_Resampler_GetMaxOutputNum = CVI_Resampler_GetMaxOutputNum;

#else
	if (stSampleResFun.pLibHandle != CVI_NULL) {
		CVI_Audio_Dlclose(stSampleResFun.pLibHandle);
		memset(&stSampleResFun, 0, sizeof(struct SAMPLE_RES_FUN_S));
	}

	s32Ret = CVI_Audio_Dlopen(&(stSampleResFun.pLibHandle), RES_LIB_NAME);
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
		__func__, __LINE__, "load resample lib fail!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &(stSampleResFun.pCVI_Resampler_Create),
				stSampleResFun.pLibHandle,
				"CVI_Resampler_Create");


	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			__func__, __LINE__, "find symbol error!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &
				(stSampleResFun.pCVI_Resampler_Process),
				stSampleResFun.pLibHandle,
				"CVI_Resampler_Process");

	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			__func__, __LINE__, "find symbol error!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &
				(stSampleResFun.pCVI_Resampler_Destroy),
				stSampleResFun.pLibHandle,
				"CVI_Resampler_Destroy");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			__func__, __LINE__, "find symbol  [Error]!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &
				(stSampleResFun.pCVI_Resampler_GetMaxOutputNum),
				stSampleResFun.pLibHandle,
				"CVI_Resampler_GetMaxOutputNum");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			__func__, __LINE__, "find symbol [Error]!\n");
		return CVI_FAILURE;
	}
#endif

	/* Step 4 open file and file target */
	/* SAMPLE_AUDIO_OpenResFile */
	snprintf(output_filename, FILE_NAME_LEN, "outsample_file_%d.raw", target_rate);
	pfd_in = fopen(input_filename, "rb");
	pfd_out = fopen(output_filename, "wb");

	if (((pfd_in) == NULL) || ((pfd_out) == NULL)) {
		printf(" [Error]Cannot open input / output file\n");
		goto ERROR_HANDEL1;
	}

	SAMPLE_RES_CHECK_NULL_PTR(stSampleResFun.pCVI_Resampler_Create);
	SAMPLE_RES_CHECK_NULL_PTR(stSampleResFun.pCVI_Resampler_Process);
	SAMPLE_RES_CHECK_NULL_PTR(stSampleResFun.pCVI_Resampler_Destroy);
	SAMPLE_RES_CHECK_NULL_PTR(stSampleResFun.pCVI_Resampler_GetMaxOutputNum);


	/* Step5 open thread and do the resample */
	/* SAMPLE_COMM_AUDIO_CreatTrdAiExtRes */
	CVI_VOID *cviRes = CVI_NULL;
	int channels = 1;

	if (argc == 5) {
		int chkadditional = atoi(argv[4]);

		if (chkadditional == 1) {
			printf("------------------------------>channel = 1\n");
			channels = 1;
		} else if (chkadditional == 2) {
			printf("------------------------------>channel = 2\n");
			channels = 2;
		} else
			printf("---------------------------------------->go default chn[%d]\n", channels);

	}

	cviRes = stSampleResFun.pCVI_Resampler_Create(
		current_rate,
		target_rate,
		channels);

	if (cviRes == NULL) {
		printf("[%s][%d]Error:get the NULL from Resampler_Create ...force return\n",
		__func__, __LINE__);
		return 0;
	}

	CVI_S32 tmpp = stSampleResFun.pCVI_Resampler_GetMaxOutputNum(cviRes,
			s32SamplePerFrame);
	printf("[t]pCVI_Resampler_GetMaxOutputNum[%d]\n", tmpp);

	printf("[t][%d]\n", __LINE__);
	CVI_S16 *ps16OutBuf = malloc(stSampleResFun.pCVI_Resampler_GetMaxOutputNum(cviRes,
			s32SamplePerFrame) * sizeof(CVI_S16)*channels);
	printf("[t][%d]\n", __LINE__);
	CVI_CHAR *input_buffer = (CVI_CHAR *)malloc(10 * 160 * 1024);

	printf("[t][%d]\n", __LINE__);
	int vround = 1;

	while (1) {
		/* each sample 2 bytes, everytime read in 160 samples per frame */
		s32ReadCheck = fread(input_buffer, sizeof(short), s32SamplePerFrame * channels, pfd_in);
		//s32InPutSize = s32SamplePerFrame*sizeof(short);

		if (feof(pfd_in)) {
			printf("[t][%d]\n", __LINE__);
			printf("end of file ...leave\n");
			break;

		} else if (s32ReadCheck != s32SamplePerFrame * channels) {
			printf("[t][%d]\n", __LINE__);
			printf("file read abnormal\n");
			break;
		}


		u32OutSample = stSampleResFun.pCVI_Resampler_Process(cviRes,
			(CVI_S16 *)input_buffer, s32SamplePerFrame, ps16OutBuf);
	printf("[t]pCVI_Resampler_Process u32OutSample[%d]\n", u32OutSample * channels);

	printf("[t][%d]round[%d]\n", __LINE__, vround++);
		fwrite(ps16OutBuf, sizeof(short), u32OutSample * channels, pfd_out);
	}

	free(ps16OutBuf);
	free(input_buffer);
	fclose(pfd_in);
	fclose(pfd_out);
	printf("sample_audio_resample finished TEST-PASS\n");
	if (stSampleResFun.pLibHandle != CVI_NULL) {
		CVI_Audio_Dlclose(stSampleResFun.pLibHandle);
		memset(&stSampleResFun, 0, sizeof(struct SAMPLE_RES_FUN_S));
		printf("do the dlclose\n");
	}
	return 0;
ERROR_HANDEL1:
	printf(" [Error]error handle failure\n");
	if (stSampleResFun.pLibHandle != CVI_NULL) {
		CVI_Audio_Dlclose(stSampleResFun.pLibHandle);
		memset(&stSampleResFun, 0, sizeof(struct SAMPLE_RES_FUN_S));
		printf("do the dlclose\n");
	}
	fclose(pfd_in);
	fclose(pfd_out);
	return 0;
}
