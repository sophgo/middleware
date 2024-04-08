/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: sample/cvi_sample_audio.c
 * Description:example for audio api flow
 * such as audio in, audio out, audio trancode flow
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
//#include "cvi_sample_comm.h"
#if !(defined(__CV181X__) || defined(__CV180X__) || defined(__CV186X__))
#include "sample_comm.h"
#endif
#include "acodec.h"
#ifndef CVIAUDIO_STATIC
#include "cvi_audio_dl_adp.h"
#endif
#include "cvi_resampler_api.h"
#include "cvi_audio_parse_param.h"
#ifdef SUPPORT_EXTERNAL_AAC
#include "cvi_audio_aac_adp.h"
#endif
#include "cvi_audio_internal_test.h"
#include "cvi_audio_loadcheck.h"

#if defined(__CV181X__) || defined(__CV180X__) || defined(__CV186X__)
#ifndef SAMPLE_AUDIO_INNER_AI_DEV
#define SAMPLE_AUDIO_INNER_AI_DEV 0
#endif

#ifndef FILE_NAME_LEN
#define FILE_NAME_LEN 128
#endif

#ifndef SAMPLE_AUDIO_INNER_AO_DEV
#define SAMPLE_AUDIO_INNER_AO_DEV 0
#endif
#endif


static PAYLOAD_TYPE_E gs_enPayloadType = PT_G726;//PT_D_MP2P;
static int iOutSampleRateSave;
static CVI_BOOL gs_bAioReSample  = CVI_FALSE;
static CVI_BOOL gs_bUserGetMode  = CVI_FALSE;
static CVI_BOOL gs_bAoVolumeCtrl = CVI_FALSE;
static AUDIO_SAMPLE_RATE_E enInSampleRate  = AUDIO_SAMPLE_RATE_BUTT;
static AUDIO_SAMPLE_RATE_E enOutSampleRate = AUDIO_SAMPLE_RATE_BUTT;
//static PAYLOAD_TYPE_E _select_audio_codec(CVI_VOID);
static PAYLOAD_TYPE_E _select_audio_codec2(CVI_VOID);
//static CVI_BOOL _select_audio_USER_GET_MODE(CVI_VOID);
static CVI_BOOL _user_get_mode_or_bind_mode(CVI_VOID);
//static CVI_BOOL _select_audio_VqeOnOff(AI_TALKVQE_CONFIG_S *pstAiVqeTalkAttr);
int g_cap_time;
static int _console_request(char *printout, int defalut_val);
static int GET_CONSOLE_REQ(int audio_status, char *printout, int default_val);
static CVI_BOOL _update_vqe_setting(AI_TALKVQE_CONFIG_S *pstAiVqeTalkAttr);
static CVI_BOOL _update_aec_setting(AI_TALKVQE_CONFIG_S *pstAiVqeTalkAttr);
static CVI_BOOL _update_downlink_vqe_setting(AO_VQE_CONFIG_S *pstAoVqeAttr);
CVI_BOOL bRecordingForAEC = CVI_FALSE;

/* 0: close, 1: record VQE, 2: talk VQE */
/* CVI_AUDIO only support talk VQE. Please choose only	0 otherwise 2 */
#define CVITEK_AUDIO_TALK_VQE 2
static CVI_U32 u32AiVqeType = CVITEK_AUDIO_TALK_VQE;
CVI_BOOL bDnVqeOn = CVI_FALSE;
#define SAMPLE_DBG(s32Ret) printf("s32Ret=%#x,fuc:%s,line:%d\n", s32Ret, __func__, __LINE__)
#define MAX_DBG_TEST_OPTIONS 256
#define _CONSOLE_REQ(X, Y) GET_CONSOLE_REQ(_parsing_audio_status(), (char *)X, (int)Y)
#define _INPUT_ARG_BY_LIST_MODE _parsing_audio_status()
#ifndef AACLC_SAMPLES_PER_FRAME
#define AACLC_SAMPLES_PER_FRAME 1024
#endif

//----------------------------------------------------------------user option function[start]
ST_AudioUnitTestCfg  stAudTestCfg;
#define TEST_MODE_BIND_AENC_ALSO_GET_AIN_FRAME 1
pthread_t stTestModeThread;
pthread_t stAdecToAo_NoBind;
typedef struct _ST_TestModeParam {
	CVI_S32 s32DeviceId;
	CVI_S32 s32ChnId;
	CVI_S32 s32Cnt;
} ST_TestModeParam;
//---------------------------------------------------------------user option function[end]
#define SMP_AUD_UNUSED_REF(X)  ((X) = (X))
#define SAMPLE_RES_CHECK_NULL_PTR(ptr)\
	do {\
		if (NULL == (CVI_U8 *)ptr)\
			printf("[fatal error]ptr is NULL,fuc:%s,line:%d\n", __func__, __LINE__);\
	} while (0)

#define RES_LIB_NAME "libcvi_RES1.so"

#define SAMPLE_RES_CHECK_NULL_PTR(ptr)\
	do {\
		if (NULL == (CVI_U8 *)ptr)\
			printf("[fatal error]ptr is NULL,fuc:%s,line:%d\n", __func__, __LINE__);\
	} while (0)

/******************************************************************************/
/* function : PT Number to String**********************************************/
/******************************************************************************/

CVI_VOID *SAMPLE_AUDIO_AdecToAo_NoBind_PROC(CVI_VOID *parg)
{
#define CVI_AUDIO_BLOCK_MODE -1
	ST_TestModeParam *pTestMode = (ST_TestModeParam *)parg;
	CVI_S32 s32Count = pTestMode->s32Cnt;
	CVI_S32 s32Dev = pTestMode->s32DeviceId;
	CVI_S32 s32Chn = pTestMode->s32ChnId;
	AUDIO_FRAME_S stFrame;
	CVI_S32 s32ChannelCnt = 0;
	AEC_FRAME_S   stAecFrm;
	FILE		*fd_aud_testmode;
	CVI_S32 s32Ret;

	fd_aud_testmode = fopen("TestMode.raw", "wb");
	if (!fd_aud_testmode) {
		printf("[Error] Cannot create test mode file\n");
		return 0;
	}

	printf("s32Dev[%d] s32Chn[%d] Cnt[%d]\n", s32Dev, s32Chn, s32Count);


	while (s32Count--) {
		s32Ret = CVI_AI_GetFrame(s32Dev, s32Chn,
					 &stFrame,
					 &stAecFrm,
					 CVI_AUDIO_BLOCK_MODE);

		if (s32Ret != CVI_SUCCESS) {
			printf("[Error]CVI_AI_GetFrame none!!\n");
			break;
		}

		if (stFrame.u32Len == 0)
			printf(" block mode return size 0...\n");

		s32ChannelCnt = stFrame.enSoundmode + 1;
		//printf("test mode size[%d][%d]\n", stFrame.u32Len , s32ChannelCnt);
		if (stFrame.enBitwidth == AUDIO_BIT_WIDTH_16)
			fwrite(stFrame.u64VirAddr[0], 1, (stFrame.u32Len * s32ChannelCnt * 2),
			       fd_aud_testmode);
		else if (stFrame.enBitwidth == AUDIO_BIT_WIDTH_32)
			fwrite(stFrame.u64VirAddr[0], 1, (stFrame.u32Len * s32ChannelCnt * 4),
			       fd_aud_testmode);
		else
			printf("Not support format bitwidth\n");
	}
	printf("file close in test mode -----------------------------------------------------\n");
	fclose(fd_aud_testmode);
	return 0;
}

CVI_VOID *SAMPLE_AUDIO_TEST_MODE_PROC(CVI_VOID *parg)
{
#define CVI_AUDIO_BLOCK_MODE -1
	ST_TestModeParam *pTestMode = (ST_TestModeParam *)parg;
	CVI_S32 s32Count = pTestMode->s32Cnt;
	CVI_S32 s32Dev = pTestMode->s32DeviceId;
	CVI_S32 s32Chn = pTestMode->s32ChnId;
	AUDIO_FRAME_S stFrame;
	CVI_S32 s32ChannelCnt = 0;
	AEC_FRAME_S   stAecFrm;
	FILE		*fd_aud_testmode;
	CVI_S32 s32Ret;

	fd_aud_testmode = fopen("TestMode.raw", "wb");
	if (!fd_aud_testmode) {
		printf("[Error] Cannot create test mode file\n");
		return 0;
	}

	printf("s32Dev[%d] s32Chn[%d] Cnt[%d]\n", s32Dev, s32Chn, s32Count);


	while (s32Count--) {
		s32Ret = CVI_AI_GetFrame(s32Dev, s32Chn,
					 &stFrame,
					 &stAecFrm,
					 CVI_AUDIO_BLOCK_MODE);

		if (s32Ret != CVI_SUCCESS) {
			printf("[Error]CVI_AI_GetFrame none!!\n");
			break;
		}

		if (stFrame.u32Len == 0)
			printf(" block mode return size 0...\n");

		s32ChannelCnt = stFrame.enSoundmode + 1;
		//printf("test mode size[%d][%d]\n", stFrame.u32Len , s32ChannelCnt);
		if (stFrame.enBitwidth == AUDIO_BIT_WIDTH_16)
			fwrite(stFrame.u64VirAddr[0], 1, (stFrame.u32Len * s32ChannelCnt * 2),
			       fd_aud_testmode);
		else if (stFrame.enBitwidth == AUDIO_BIT_WIDTH_32)
			fwrite(stFrame.u64VirAddr[0], 1, (stFrame.u32Len * s32ChannelCnt * 4),
			       fd_aud_testmode);
		else
			printf("Not support format bitwidth\n");
	}
	printf("file close in test mode -----------------------------------------------------\n");
	fclose(fd_aud_testmode);
	return 0;
}

static char *SAMPLE_AUDIO_Pt2Str(PAYLOAD_TYPE_E enType)
{
	if (enType == PT_G711A)
		return "g711a";
	else if (enType == PT_G711U)
		return "g711u";
	else if (enType == PT_ADPCMA)
		return "adpcm";
	else if (enType == PT_G726)
		return "g726";
	else if (enType == PT_LPCM)
		return "pcm";
	else if (enType == PT_AAC)
		return "aac";
	else
		return "data";
}
static int GET_CONSOLE_REQ(int audio_status, char *printout, int default_val)
{
	if (audio_status) {
		return _parsing_request(printout, default_val);
	} else {
		return _console_request(printout, default_val);
	}
	return 0;
}

static int _console_request(char *printout, int default_val)
{
	char s_option[128];
	int s32Ret = -1;

	printf("\e[0;32m=====================================\n");
	fflush(stdout);
	fflush(stdin);
	if (printout != NULL) {
		printf(printout);
		fgets(s_option, 10, stdin);
		if (s_option[0] == '\n') {
			s32Ret = default_val;
			printf("input default val[%d]\n", s32Ret);
		} else {
			s32Ret = atoi(s_option);
			printf("input [%d]\n", s32Ret);
		}
	} else
		printf("Error input type[%s][%d]\n", __func__, __LINE__);

	printf("\e[0;32m=====================================\n");
	fflush(stdin);
	return s32Ret;
}


struct SAMPLE_AI2EXTRES_S {
	CVI_BOOL bStart;
	CVI_S32  AiDev;
	CVI_S32  AiChn;
	AUDIO_SAMPLE_RATE_E enInSample;
	AUDIO_SAMPLE_RATE_E enOutSample;
	CVI_U32 u32PerFrame;
	FILE *pfd;
	pthread_t stAiPid;
};

struct SAMPLE_EXTRES2AOUT_S {
	CVI_BOOL bStart;
	CVI_S32  AoDev;
	CVI_S32  AoChn;
	AUDIO_SAMPLE_RATE_E enSrcSample;
	AUDIO_SAMPLE_RATE_E enAoutSample;
	CVI_U32 u32PerFrame;
	CVI_U32 u32ChnCnt;
	FILE *pfd;
	CVI_BOOL bEnInternalResmp;
	pthread_t stAoutPid;
};

typedef CVI_VOID *(*pCVI_Resampler_Create_Callback)(CVI_S32 s32Inrate,
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

static struct SAMPLE_AI2EXTRES_S   gs_stSampleAiExtRes[AI_DEV_MAX_NUM *
				       AI_MAX_CHN_NUM];
static struct SAMPLE_RES_FUN_S	   gs_stSampleResFun = {0};
static struct SAMPLE_EXTRES2AOUT_S  gs_stSampleExtResAout;

/******************************************************************************/
/* function : get frame from Ai, send it  to Resampler*/
/******************************************************************************/

static CVI_S32 SAMPLE_AUDIO_VQE_Setting(AI_TALKVQE_CONFIG_S *pConfig)
{
	if (pConfig == CVI_NULL) {
		printf("Not valid pointer for VQE setting\n");
		return CVI_FAILURE;
	}

	AI_AEC_CONFIG_S default_AEC_Setting;
	AUDIO_ANR_CONFIG_S	default_ANR_Setting;
	AUDIO_AGC_CONFIG_S default_AGC_Setting;

	memset(&default_AEC_Setting, 0, sizeof(AI_AEC_CONFIG_S));
	memset(&default_AGC_Setting, 0, sizeof(AUDIO_AGC_CONFIG_S));
	memset(&default_ANR_Setting, 0, sizeof(AUDIO_ANR_CONFIG_S));

#ifdef NEXT_SSP_ALGO
	default_ANR_Setting.para_nr_init_sile_time = 0;
	default_ANR_Setting.para_nr_snr_coeff = 15;
	default_AGC_Setting.para_agc_max_gain = 0;
	default_AGC_Setting.para_agc_target_high = 2;
	default_AGC_Setting.para_agc_target_low = 72;
	default_AGC_Setting.para_agc_vad_ena = 1;
	pConfig->para_notch_freq = 0;
#else
	default_AGC_Setting.para_agc_max_gain = 1;
	default_AGC_Setting.para_agc_target_high = 2;
	default_AGC_Setting.para_agc_target_low = 6;
	default_AGC_Setting.para_agc_vad_enable = CVI_TRUE;
	default_AGC_Setting.para_agc_vad_cnt = 13;
	default_AGC_Setting.para_agc_cut6_enable = CVI_TRUE;

	default_ANR_Setting.para_nr_snr_coeff = 15;
	default_ANR_Setting.para_nr_noise_coeff = 2;
	pConfig->enWorkstate = VQE_WORKSTATE_COMMON;
	pConfig->s32BytesPerSample = 2;
#endif
	memcpy(&pConfig->stAgcCfg,
	       &default_AGC_Setting,
	       sizeof(AUDIO_AGC_CONFIG_S));
	memcpy(&pConfig->stAnrCfg,
	       &default_ANR_Setting,
	       sizeof(AUDIO_ANR_CONFIG_S));
	memcpy(&pConfig->stAecCfg,
	       &default_AEC_Setting,
	       sizeof(AI_AEC_CONFIG_S));

	pConfig->u32OpenMask = 0;
	pConfig->u32OpenMask |= (AI_TALKVQE_MASK_AGC);
	pConfig->u32OpenMask |= (AI_TALKVQE_MASK_ANR);
	//pConfig->u32OpenMask |= (AI_TALKVQE_MASK_AEC);



	return CVI_SUCCESS;
}

#define DEBUG_MODE_SAVE_BEFORE_AOUT  0
void *SAMPLE_COMM_AUDIO_ExtResAoutProc(void *parg)
{
	struct SAMPLE_EXTRES2AOUT_S *pstRes2Ao = (struct SAMPLE_EXTRES2AOUT_S *)parg;
	CVI_S32 s32ReadCheck = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ChnCnt = pstRes2Ao->u32ChnCnt;
	CVI_VOID *hRes = NULL;
	AUDIO_FRAME_S stFrame;
	CVI_S32  AoDev = pstRes2Ao->AoDev;
	CVI_S32  AoChn = pstRes2Ao->AoChn;
	CVI_S32 s32ZeroCnt = 0;
	CVI_S32 s32TotalBytesSize = 0;
	CVI_CHAR *input_buffer;
	CVI_S16 *ps16OutBuf;
	CVI_U32 u32OutSample;
	CVI_S32 s32ReadInBytes;



	if (pstRes2Ao->bEnInternalResmp == CVI_FALSE) {
		SAMPLE_RES_CHECK_NULL_PTR(gs_stSampleResFun.pCVI_Resampler_Create);
		SAMPLE_RES_CHECK_NULL_PTR(gs_stSampleResFun.pCVI_Resampler_Process);
		SAMPLE_RES_CHECK_NULL_PTR(gs_stSampleResFun.pCVI_Resampler_Destroy);
		SAMPLE_RES_CHECK_NULL_PTR(gs_stSampleResFun.pCVI_Resampler_GetMaxOutputNum);

		hRes = gs_stSampleResFun.pCVI_Resampler_Create(pstRes2Ao->enSrcSample,
				pstRes2Ao->enAoutSample,
				u32ChnCnt);

		if (hRes == NULL) {
			printf("[%s][%d]Error:get the NULL from Resampler_Create ...force return\n",
			       __func__, __LINE__);
			return NULL;
		}

		s32TotalBytesSize = gs_stSampleResFun.pCVI_Resampler_GetMaxOutputNum(hRes,
				    pstRes2Ao->u32PerFrame) * sizeof(CVI_S16) * u32ChnCnt;
		s32ReadInBytes = (pstRes2Ao->u32PerFrame) * sizeof(CVI_S16) * u32ChnCnt;
	} else {
		s32TotalBytesSize = pstRes2Ao->u32PerFrame * sizeof(CVI_S16) * u32ChnCnt;
		s32ReadInBytes = s32TotalBytesSize;
	}
	input_buffer = (CVI_CHAR *)malloc(s32ReadInBytes);
	ps16OutBuf = (CVI_S16 *)malloc(s32TotalBytesSize *
				       12); //8k -> 96k , maximum 12 times
	u32OutSample = 0;

#if DEBUG_MODE_SAVE_BEFORE_AOUT
	FILE *pfd_savein;

	pfd_savein = fopen("save_ao_input.raw", "w+");
#endif
	while (pstRes2Ao->bStart) {

		s32ReadCheck = fread(input_buffer, 1, s32ReadInBytes, pstRes2Ao->pfd);
		if (s32ReadCheck != s32ReadInBytes) {
			printf("[%s][%d]End of file\n", __func__, __LINE__);
			break;
		}
		if (feof(pstRes2Ao->pfd)) {
			printf("[%s][%d]\n", __func__, __LINE__);
			printf("end of file ...leave\n");
			break;
		}

		if (pstRes2Ao->bEnInternalResmp == CVI_FALSE) {
			u32OutSample = gs_stSampleResFun.pCVI_Resampler_Process(hRes,
					(CVI_S16 *)input_buffer, pstRes2Ao->u32PerFrame, ps16OutBuf);
			printf("xxx save sample_audio_internal 8 u32OutSample[%d] u32ChnCnt[%d]\n", u32OutSample,
			       u32ChnCnt);
			stFrame.u64VirAddr[0] = (CVI_U8 *)ps16OutBuf;
		} else {
			//using internal resampler --just send in period size per channel
			u32OutSample = pstRes2Ao->u32PerFrame;
			stFrame.u64VirAddr[0] = (CVI_U8 *)input_buffer;
		}
#if DEBUG_MODE_SAVE_BEFORE_AOUT
		(CVI_VOID)fwrite(ps16OutBuf,  1, u32OutSample * u32ChnCnt * sizeof(CVI_S16),
				 pfd_savein);
#endif

		stFrame.u32Len = u32OutSample;
		stFrame.u64TimeStamp = 0;
		stFrame.enSoundmode =
			u32ChnCnt;//always send in single channe period size(samples)
		stFrame.enBitwidth = AUDIO_BIT_WIDTH_16;

		if (stFrame.u32Len != 0) {
			s32Ret = CVI_AO_SendFrame(AoDev, AoChn, (const AUDIO_FRAME_S *)&stFrame, 5000);

			if (s32Ret != CVI_SUCCESS) {
				printf("[%s]CVI_AO_SendFrame failed with %#x!\n", __func__, s32Ret);
				break;
			}
		} else {
			s32ZeroCnt++;
			printf("test breakxxxx[%d]\n", s32ZeroCnt);
			//break;
		}
	}

#if DEBUG_MODE_SAVE_BEFORE_AOUT
	fclose(pfd_savein);
#endif
	if (ps16OutBuf != NULL)
		free(ps16OutBuf);
	if (input_buffer != NULL)
		free(input_buffer);
	pstRes2Ao->bStart = CVI_FALSE;
	if (pstRes2Ao->bEnInternalResmp == CVI_FALSE)
		gs_stSampleResFun.pCVI_Resampler_Destroy(hRes);

	return NULL;
}

#define DEBUG_MODE_SAVE_INPUT 0
void *SAMPLE_COMM_AUDIO_AiExtResProc(void *parg)
{

	CVI_S32 s32Ret;
	struct SAMPLE_AI2EXTRES_S *pstAiCtl = (struct SAMPLE_AI2EXTRES_S *)parg;
	AUDIO_FRAME_S stFrame;
	AEC_FRAME_S   stAecFrm;
	//AI_CHN_PARAM_S stAiChnPara;
	CVI_VOID *hRes = NULL;
	AIO_ATTR_S	AudAttr = {0};
	CVI_S16 *ps16OutBuf = CVI_NULL;
	CVI_U32 u32OutSample = 0;
	CVI_U32 u32ChannelsNum = 0;
#if DEBUG_MODE_SAVE_INPUT
	FILE *pfd_savein;
	pfd_savein = fopen("save_resample_input.raw", "w+");
#endif
	SAMPLE_RES_CHECK_NULL_PTR(gs_stSampleResFun.pCVI_Resampler_Create);
	SAMPLE_RES_CHECK_NULL_PTR(gs_stSampleResFun.pCVI_Resampler_Process);
	SAMPLE_RES_CHECK_NULL_PTR(gs_stSampleResFun.pCVI_Resampler_Destroy);
	SAMPLE_RES_CHECK_NULL_PTR(gs_stSampleResFun.pCVI_Resampler_GetMaxOutputNum);

	s32Ret =  CVI_AI_GetPubAttr(pstAiCtl->AiDev, &AudAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("[Error]%s: CVI_AI_GetPubAttr  failed\n", __func__);
		return NULL;
	}

	u32ChannelsNum = AudAttr.u32ChnCnt;
	if (u32ChannelsNum == 0) {
		printf("[Error]%s:  channel cnt abnormal, make sure SAMPLE_COMM_AUDIO_StartAi ok\n",
		       __func__);
		return NULL;

	} else
		printf("[%s] channel count = [%d]\n", __func__, u32ChannelsNum);

	/*Create Resample.*/
	/* only support mono channel. */
	printf("input smp[%d] output smp[%d] chnnum[%d] perfrm[%d]\n",
	       pstAiCtl->enInSample, pstAiCtl->enOutSample, u32ChannelsNum,
	       pstAiCtl->u32PerFrame);
	hRes = gs_stSampleResFun.pCVI_Resampler_Create(pstAiCtl->enInSample,
			pstAiCtl->enOutSample, u32ChannelsNum);

	if (hRes == NULL) {
		printf("[%s][%d]Error:get the NULL from Resampler_Create ...force return\n",
		       __func__, __LINE__);
		return NULL;
	}

	CVI_S32 s32TotalSampleNum = gs_stSampleResFun.pCVI_Resampler_GetMaxOutputNum(
					    hRes,
					    pstAiCtl->u32PerFrame) * u32ChannelsNum;


	CVI_S32 s32TotalBytesSize = gs_stSampleResFun.pCVI_Resampler_GetMaxOutputNum(
					    hRes,
					    pstAiCtl->u32PerFrame) * sizeof(CVI_S16) * u32ChannelsNum;


	ps16OutBuf = malloc(gs_stSampleResFun.pCVI_Resampler_GetMaxOutputNum(hRes,
			    pstAiCtl->u32PerFrame) * sizeof(CVI_S16) * u32ChannelsNum);


	CVI_S32 s32Mulit = 1;

	if (pstAiCtl->enInSample % pstAiCtl->enOutSample == 0) {
		s32Mulit = pstAiCtl->enInSample / pstAiCtl->enOutSample;
		if (pstAiCtl->u32PerFrame % s32Mulit == 0)
			s32Mulit = 1;
	}
	//AiFd = CVI_AI_GetFd(pstAiCtl->AiDev, pstAiCtl->AiChn);

	while (pstAiCtl->bStart) {
		/* get frame from ai chn */
		memset(&stAecFrm, 0, sizeof(AEC_FRAME_S));
		s32Ret = CVI_AI_GetFrame(pstAiCtl->AiDev, pstAiCtl->AiChn, &stFrame,
					 &stAecFrm, -1);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_AI_GetFrame none!!\n");
			continue;
		}
		/* send frame to encoder */
		if (s32Mulit != 1) {
			//-------Not an option
			printf(" This should not enter\n");

		} else {
#if DEBUG_MODE_SAVE_INPUT
			printf("[dbg]save input size  total samples[%d] per channel\n", stFrame.u32Len);
			(CVI_VOID)fwrite(stFrame.u64VirAddr[0],
					 sizeof(CVI_S16),
					 stFrame.u32Len * u32ChannelsNum,
					 pfd_savein);
#endif
			u32OutSample = gs_stSampleResFun.pCVI_Resampler_Process(hRes,
					(CVI_S16 *)stFrame.u64VirAddr[0], pstAiCtl->u32PerFrame, ps16OutBuf);

			printf("frm[%d] maxout[%d] bufsize[%d] outsmpnum[%d] save_smpnum[%d] insmpnum[%d]\n",
			       pstAiCtl->u32PerFrame,
			       s32TotalSampleNum,
			       s32TotalBytesSize,
			       u32OutSample,
			       u32OutSample * u32ChannelsNum,
			       pstAiCtl->u32PerFrame);

			(CVI_VOID)fwrite(ps16OutBuf,  sizeof(CVI_S16), u32OutSample * u32ChannelsNum,
					 pstAiCtl->pfd);
		}
		fflush(pstAiCtl->pfd);
#if DEBUG_MODE_SAVE_INPUT
		fflush(pfd_savein);
#endif
		/* finally you must release the stream */
		s32Ret = CVI_AI_ReleaseFrame(pstAiCtl->AiDev, pstAiCtl->AiChn, &stFrame,
					     &stAecFrm);
		if (s32Ret != CVI_SUCCESS) {
			printf("%s: CVI_AI_ReleaseFrame(%d, %d), failed with %#x!\n",
			       __func__, pstAiCtl->AiDev, pstAiCtl->AiChn, s32Ret);
			pstAiCtl->bStart = CVI_FALSE;
			free(ps16OutBuf);
			gs_stSampleResFun.pCVI_Resampler_Destroy(hRes);
			return NULL;
		}
	}

	pstAiCtl->bStart = CVI_FALSE;
	free(ps16OutBuf);
	gs_stSampleResFun.pCVI_Resampler_Destroy(hRes);

	return NULL;
}
/******************************************************************************/
/* function : Create the thread to resample from file input and send to aout*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_CreatTrdExtResAout(AIO_ATTR_S *pstAioAttr,
		AUDIO_DEV AoDev, AI_CHN AoChn,	AUDIO_SAMPLE_RATE_E eSrcSampleRate,
		FILE *pResFd, CVI_BOOL bEnInternalResmp)
{
	struct SAMPLE_EXTRES2AOUT_S *pstRes2Ao = NULL;

	pstRes2Ao = &gs_stSampleExtResAout;
	pstRes2Ao->AoDev = AoDev;
	pstRes2Ao->AoChn = AoChn;
	pstRes2Ao->enSrcSample = eSrcSampleRate;
	pstRes2Ao->enAoutSample = pstAioAttr->enSamplerate;
	pstRes2Ao->u32PerFrame =  pstAioAttr->u32PtNumPerFrm;
	pstRes2Ao->pfd = pResFd;
	pstRes2Ao->bStart = CVI_TRUE;
	pstRes2Ao->u32ChnCnt = pstAioAttr->u32ChnCnt;
	pstRes2Ao->bEnInternalResmp = bEnInternalResmp;

	pthread_create(&pstRes2Ao->stAoutPid,
		       0,
		       SAMPLE_COMM_AUDIO_ExtResAoutProc,
		       pstRes2Ao);

	return CVI_SUCCESS;
}
/******************************************************************************/
/* function : Create the thread to get frame from ai and send to aenc*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_CreatTrdAiExtRes(AIO_ATTR_S *pstAioAttr,
		AUDIO_DEV AiDev, AI_CHN AiChn,	AUDIO_SAMPLE_RATE_E enOutSampleRate,
		FILE *pResFd)
{
	struct SAMPLE_AI2EXTRES_S *pstAi2ExtRes = NULL;

	pstAi2ExtRes = &gs_stSampleAiExtRes[AiDev * AI_MAX_CHN_NUM + AiChn];
	pstAi2ExtRes->AiDev = AiDev;
	pstAi2ExtRes->AiChn = AiChn;
	pstAi2ExtRes->enInSample = pstAioAttr->enSamplerate;
	pstAi2ExtRes->enOutSample = enOutSampleRate;
	pstAi2ExtRes->u32PerFrame = pstAioAttr->u32PtNumPerFrm;
	pstAi2ExtRes->pfd = pResFd;
	pstAi2ExtRes->bStart = CVI_TRUE;
	pthread_create(&pstAi2ExtRes->stAiPid, 0, SAMPLE_COMM_AUDIO_AiExtResProc,
		       pstAi2ExtRes);
	return CVI_SUCCESS;
}

/******************************************************************************/
/* function : Destroy the thread to get frame from ai and send to extern resampler*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_DestoryTrdAiExtRes(AUDIO_DEV AiDev, AI_CHN AiChn)
{
	struct SAMPLE_AI2EXTRES_S *pstAi = NULL;

	pstAi = &gs_stSampleAiExtRes[AiDev * AI_MAX_CHN_NUM + AiChn];
	if (pstAi->bStart) {
		pstAi->bStart = CVI_FALSE;
		//pthread_cancel(pstAi->stAiPid);
		pthread_join(pstAi->stAiPid, 0);
	}
	fclose(pstAi->pfd);

	return CVI_SUCCESS;
}

/******************************************************************************/
/* function : Add dynamic load path*/
/******************************************************************************/
static CVI_VOID SAMPLE_AUDIO_AddLibPath(CVI_VOID)
{
#ifndef CVIAUDIO_STATIC
	CVI_S32 s32Ret;
	CVI_CHAR aszLibPath[FILE_NAME_LEN] = {0};

	s32Ret = CVI_Audio_Dlpath(aszLibPath);
	if (s32Ret != CVI_SUCCESS)
		printf("%s: add lib path %s failed\n", __func__, aszLibPath);
#endif
}

/******************************************************************************/
/* function : DeInit resamle functions */
/******************************************************************************/
static CVI_S32 SAMPLE_AUDIO_DeInitExtResFun(CVI_VOID)
{
	if (gs_stSampleResFun.pLibHandle != CVI_NULL) {
#ifndef CVIAUDIO_STATIC
		CVI_Audio_Dlclose(gs_stSampleResFun.pLibHandle);
#endif
	}
	memset(&gs_stSampleResFun, 0, sizeof(struct SAMPLE_RES_FUN_S));
	return CVI_SUCCESS;
}

/******************************************************************************/
/* function : Init resamle functions */
/******************************************************************************/
static CVI_S32 SAMPLE_AUDIO_InitExtResFun(CVI_VOID)
{
	CVI_S32 s32Ret;
	struct SAMPLE_RES_FUN_S stSampleResFun;

	s32Ret = SAMPLE_AUDIO_DeInitExtResFun();
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
		       __func__, __LINE__, "Unload resample lib fail!\n");
		return CVI_FAILURE;
	}

	memset(&stSampleResFun, 0, sizeof(struct SAMPLE_RES_FUN_S));
#ifdef CVIAUDIO_STATIC
	stSampleResFun.pCVI_Resampler_Create = CVI_Resampler_Create;
	stSampleResFun.pCVI_Resampler_Process = CVI_Resampler_Process;
	stSampleResFun.pCVI_Resampler_Destroy = CVI_Resampler_Destroy;
	stSampleResFun.pCVI_Resampler_GetMaxOutputNum = CVI_Resampler_GetMaxOutputNum;
	SMP_AUD_UNUSED_REF(s32Ret);
	s32Ret = CVI_SUCCESS;
#else
	s32Ret = CVI_Audio_Dlopen(&(stSampleResFun.pLibHandle), RES_LIB_NAME);
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
		       __func__, __LINE__, "load resample lib fail!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &(stSampleResFun.pCVI_Resampler_Create),
				 stSampleResFun.pLibHandle, "CVI_Resampler_Create");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
		       __func__, __LINE__, "find symbol error!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &
				 (stSampleResFun.pCVI_Resampler_Process), stSampleResFun.pLibHandle,
				 "CVI_Resampler_Process");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
		       __func__, __LINE__, "find symbol error!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &
				 (stSampleResFun.pCVI_Resampler_Destroy), stSampleResFun.pLibHandle,
				 "CVI_Resampler_Destroy");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
		       __func__, __LINE__, "find symbol error!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &
				 (stSampleResFun.pCVI_Resampler_GetMaxOutputNum), stSampleResFun.pLibHandle,
				 "CVI_Resampler_GetMaxOutputNum");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
		       __func__, __LINE__, "find symbol error!\n");
		return CVI_FAILURE;
	}

	memcpy(&gs_stSampleResFun, &stSampleResFun, sizeof(struct SAMPLE_RES_FUN_S));
#endif
	return CVI_SUCCESS;
}

/******************************************************************************/
/* function : Open Aenc File */
/******************************************************************************/
static FILE *SAMPLE_AUDIO_OpenAencFile(AENC_CHN AeChn, PAYLOAD_TYPE_E enType)
{
	FILE *pfd;
	CVI_CHAR aszFileName[FILE_NAME_LEN] = {0};

	/* create file for save stream*/

	snprintf(aszFileName, FILE_NAME_LEN, "audio_chn%d.%s", AeChn,
		 SAMPLE_AUDIO_Pt2Str(enType));

#ifdef CVI_MODIFIED
	snprintf(aszFileName, FILE_NAME_LEN, "fd_aenc_out.raw.%s",
		 SAMPLE_AUDIO_Pt2Str(enType));
#endif
	pfd = fopen(aszFileName, "w+");
	if (pfd == NULL) {
		printf("%s: open file %s failed\n", __func__, aszFileName);
		return NULL;
	}
	printf("open stream file:\"%s\" for aenc ok\n", aszFileName);

	return pfd;
}

/******************************************************************************/
/* function : Open ExtResample File*/
/******************************************************************************/
static FILE *SAMPLE_AUDIO_OpenResFile(CVI_S32 s32ChnCnt)
{
	FILE *pfd;
	CVI_CHAR aszFileName[FILE_NAME_LEN] = {0};

//rename the output sample file
	if (s32ChnCnt > 1) {
		snprintf(aszFileName, FILE_NAME_LEN,
			 "outsample_file_%d_ch%d.raw", iOutSampleRateSave, s32ChnCnt);
	} else {
		snprintf(aszFileName, FILE_NAME_LEN,
			 "outsample_file_%d.raw", iOutSampleRateSave);
	}

	pfd = fopen(aszFileName, "w+");
	if (pfd == NULL) {
		printf("%s: open file %s failed\n", __func__, aszFileName);
		return NULL;
	}
	printf("open stream file:\"%s\" for Resample ok\n", aszFileName);
	return pfd;
}

/******************************************************************************/
/* function : Open Adec File */
/******************************************************************************/
static FILE *SAMPLE_AUDIO_OpenAdecFile(ADEC_CHN AdChn, PAYLOAD_TYPE_E enType)
{
	FILE *pfd;
	CVI_CHAR aszFileName[FILE_NAME_LEN] = {0};

	/* create file for save stream*/
	SMP_AUD_UNUSED_REF(AdChn);
	snprintf(aszFileName, FILE_NAME_LEN, "fd_aenc_out_chn0.%s",
		 SAMPLE_AUDIO_Pt2Str(enType));
	if (access(aszFileName, F_OK) != -1) {
		printf("[%s]  exist ..\n", aszFileName);
		pfd = fopen(aszFileName, "rb");
		if (pfd != NULL) {
			printf("====================>open stream file:\"%s\" for adec ok\n",
			       aszFileName);
			/* null check for file */

		} else {
			printf("%s: xxxxxxxxxxxxxxxxxxxxopen file %s failed\n", __func__,
			       aszFileName);
			return NULL;
		}

	} else {
		printf("fd_aenc_out_chn0.%s file not exist leaving..\n",
		       SAMPLE_AUDIO_Pt2Str(enType));
		return NULL;
	}
	return pfd;
}


/******************************************************************************/
/* function : file -> Adec -> Ao*/
/******************************************************************************/
CVI_S32 SAMPLE_AUDIO_AdecAo(CVI_VOID)
{
	CVI_S32 s32Ret;
	AO_CHN AoChn = 0;
	ADEC_CHN AdChn = 0;
	FILE *pfd = NULL;
	AIO_ATTR_S stAioAttr;
	int AudMaxChnCnt = 3;
	CVI_BOOL bEnableResample = CVI_FALSE;
	AUDIO_DEV AoDev = SAMPLE_AUDIO_INNER_AO_DEV;

#if 0//	SUPPORT_EXTERNAL_AAC
	stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_32000;
	stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
	stAioAttr.u32EXFlag      = 0;
	stAioAttr.u32FrmNum      = 30;
	stAioAttr.u32PtNumPerFrm = AACLC_SAMPLES_PER_FRAME;
	stAioAttr.u32ChnCnt      = 1;
	printf("decode  example aac\n");
	gs_enPayloadType = PT_AAC;
#endif
	if (stAudTestCfg.bOptCfg == CVI_TRUE) {
		s32Ret = CVI_SUCCESS;
		printf("force skip unittest..require using new unit test script\n");
		goto ADECAO_ERR3;
	}
	/********************************************/
	/* step 0-1: setup audout/adec playing config*/
	/********************************************/

	stAioAttr.enSamplerate =
		(AUDIO_SAMPLE_RATE_E)_CONSOLE_REQ("Enter play-out sample rate(default:8000)\n",
				8000);
	stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;
	stAioAttr.u32EXFlag = 0;
	stAioAttr.u32FrmNum = 20;
	stAioAttr.u32PtNumPerFrm =
		320;  //160 for basic test / 320 for g726 / 1152 for mp2/mp3
	stAioAttr.u32ChnCnt = AudMaxChnCnt;
	int channels = _CONSOLE_REQ("Enter channel numbers(1 or 2)(default:1)\n",
					   1);
	if (channels == 1)
		stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
	else if (channels == 2)
		stAioAttr.enSoundmode = AUDIO_SOUND_MODE_STEREO;
	else {
		printf("do not support other channel count[%d]\n", channels);
		return CVI_FAILURE;
	}
	stAioAttr.u32ClkSel = 0;
	stAioAttr.enI2sType = AIO_I2STYPE_INNERCODEC;
	gs_enPayloadType = _select_audio_codec2();
	if (gs_enPayloadType == PT_AAC) {
		printf("PT_AAC LC default setting audio out frm size[%d]\n",
		       stAioAttr.u32PtNumPerFrm);
		//stAioAttr.u32PtNumPerFrm = AACLC_SAMPLES_PER_FRAME;//1152 for mp2 //320 for g726
		//stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_32000;
		//stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;  //change to mono
		//stAioAttr.u32ChnCnt = 1; //channel one
	}

	printf("SAMPLE_AUDIO_AdecAo\n");
	//MP3/mp2：编码一帧，一般是1152个采样点，这样其数据大小是1152x2x2=4608字节
	//AAC：编码一帧，一般是1024个采样点，这样其数据大小是1024x2x2=4096字节
	/********************************************/
	/*	step 0-2: Do application need Resample */
	/*	setup Resampler config for play out */
	/********************************************/
	bEnableResample = (CVI_BOOL)_CONSOLE_REQ("do resample (0/1)(default:0)\n", 0);
	enInSampleRate	= 48000;
	enOutSampleRate = stAioAttr.enSamplerate;
	if (bEnableResample == CVI_TRUE) {
		printf("target sample rate = [%d]\n", enOutSampleRate);
		enInSampleRate =
			(AUDIO_SAMPLE_RATE_E)
			_CONSOLE_REQ("Enter input out sample_rate(default:48000)\n",
				     48000);
	}
	gs_bUserGetMode = _user_get_mode_or_bind_mode();
	g_cap_time = _CONSOLE_REQ("adec ao playout time (default:20)\n", 20);
	s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(&stAioAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto ADECAO_ERR3;
	}
	ADEC_CHN_ATTR_S AdecAttr;

	AdecAttr.s32Sample_rate = stAioAttr.enSamplerate;
	AdecAttr.s32ChannelNums = channels;
	AdecAttr.s32frame_size = stAioAttr.u32PtNumPerFrm;
	AdecAttr.s32BytesPerSample = 2;
	if ((bEnableResample == CVI_TRUE) && (gs_enPayloadType == PT_AAC)) {
		AdecAttr.s32Sample_rate = enInSampleRate;
		AdecAttr.s32frame_size = AACLC_SAMPLES_PER_FRAME;
	}

	s32Ret = SAMPLE_COMM_AUDIO_StartAdec(AdChn, gs_enPayloadType, &AdecAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto ADECAO_ERR3;
	}


	s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, AoChn, &stAioAttr,
					   enInSampleRate, bEnableResample);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto ADECAO_ERR2;
	}

	pfd = SAMPLE_AUDIO_OpenAdecFile(AdChn, gs_enPayloadType);

	if (pfd == NULL) {
		printf("pfd == NULL force stop!!\n");
		return 0;
	}

	if (!pfd) {
		SAMPLE_DBG(CVI_FAILURE);
		goto ADECAO_ERR0;
	}

	s32Ret = SAMPLE_COMM_AUDIO_CreatTrdFileAdec(AdChn, pfd);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto ADECAO_ERR1;
	} else {
		printf("start sending encode frame to adec module\n");
		printf(">>>>>>>>>>SAMPLE_AUDIO_AdecAo<<<<<<<<<<start playing\n");
	}

	if (gs_bUserGetMode == CVI_TRUE) {
		printf("userget mode\n");
		s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAdecAo(AdChn, AoDev, NULL);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_DBG(s32Ret);
			goto ADECAO_ERR1;
		}
	} else {
		printf("bind mode\n");
		s32Ret = SAMPLE_COMM_AUDIO_AoBindAdec(AoDev, AoChn, AdChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_DBG(s32Ret);
			goto ADECAO_ERR1;
		}
		printf("bind adec:%d to ao(%d,%d) ok\n", AdChn, AoDev, AoChn);
	}


	if (stAudTestCfg.bOptCfg == CVI_TRUE) {
		printf("user option mode ...unit test mode\n");
		CVI_S32 s32TestCount = 6;

		while (s32TestCount--)
			sleep(1);

		printf("user option mode ...unit test mode...leave\n");
	} else {
		//printf("\nplease press twice ENTER to exit this sample\n");
		//getchar();
		//getchar();
		SAMPLE_COMM_AUDIO_GET_AdecSend_ExitFlag();
	}

	s32Ret = SAMPLE_COMM_AUDIO_DestoryTrdFileAdec(AdChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		return CVI_FAILURE;
	}

ADECAO_ERR0:
	if (gs_bUserGetMode != CVI_TRUE) {
		printf("unbind adec to ao\n");
		s32Ret = SAMPLE_COMM_AUDIO_AoUnbindAdec(AoDev, AoChn, AdChn);
		if (s32Ret != CVI_SUCCESS)
			SAMPLE_DBG(s32Ret);

	} else {
		printf("release adec to ao thread\n");
		s32Ret = SAMPLE_COMM_AUDIO_DestoryTrdAdecAo(AdChn);
		if (s32Ret != CVI_SUCCESS)
			SAMPLE_DBG(s32Ret);
	}

ADECAO_ERR1:
	s32Ret |= SAMPLE_COMM_AUDIO_StopAo(AoDev, AoChn, bEnableResample);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_DBG(s32Ret);
ADECAO_ERR2:
	s32Ret |= SAMPLE_COMM_AUDIO_StopAdec(AdChn);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_DBG(s32Ret);

ADECAO_ERR3:
	printf("SAMPLE_AUDIO_AdecAo TEST-PASS\n");
	return s32Ret;
}


static CVI_BOOL _update_aec_setting(AI_TALKVQE_CONFIG_S *pstAiVqeTalkAttr)
{
	if (pstAiVqeTalkAttr == NULL)
		return CVI_FALSE;

	AI_AEC_CONFIG_S default_AEC_Setting;

	memset(&default_AEC_Setting, 0, sizeof(AI_AEC_CONFIG_S));

	default_AEC_Setting.para_aec_filter_len = 13;
	default_AEC_Setting.para_aes_std_thrd = 37;
	default_AEC_Setting.para_aes_supp_coeff = 60;
	pstAiVqeTalkAttr->stAecCfg = default_AEC_Setting;
	pstAiVqeTalkAttr->u32OpenMask = LP_AEC_ENABLE | NLP_AES_ENABLE | NR_ENABLE | AGC_ENABLE |
					NOTCH_ENABLE | DCREMOVER_ENABLE | DG_ENABLE | DELAY_ENABLE;
	printf("pstAiVqeTalkAttr:u32OpenMask[0x%x]\n", pstAiVqeTalkAttr->u32OpenMask);
	return CVI_FALSE;
}

static CVI_BOOL _update_vqe_setting(AI_TALKVQE_CONFIG_S *pstAiVqeTalkAttr)
{
	if (pstAiVqeTalkAttr == NULL)
		return CVI_FALSE;

	pstAiVqeTalkAttr->u32OpenMask |= AI_TALKVQE_MASK_ANR;
	pstAiVqeTalkAttr->u32OpenMask |= AI_TALKVQE_MASK_AGC;

	AUDIO_AGC_CONFIG_S st_AGC_Setting;
	AUDIO_ANR_CONFIG_S st_ANR_Setting;


#ifdef NEXT_SSP_ALGO
	st_AGC_Setting.para_agc_max_gain = 0;
	st_AGC_Setting.para_agc_target_high = 2;
	st_AGC_Setting.para_agc_target_low = 72;
	st_AGC_Setting.para_agc_vad_ena = CVI_TRUE;
	st_ANR_Setting.para_nr_snr_coeff = 15;
	st_ANR_Setting.para_nr_init_sile_time = 0;

#else
	st_AGC_Setting.para_agc_vad_enable = CVI_TRUE;
	st_ANR_Setting.para_nr_snr_coeff = 15;
	st_AGC_Setting.para_agc_max_gain = 1;
	st_AGC_Setting.para_agc_target_high = 2;
	st_AGC_Setting.para_agc_target_low = 6;
#endif

	pstAiVqeTalkAttr->stAgcCfg = st_AGC_Setting;
	pstAiVqeTalkAttr->stAnrCfg = st_ANR_Setting;

	pstAiVqeTalkAttr->para_notch_freq = 0;
	printf("pstAiVqeTalkAttr:u32OpenMask[0x%x]\n", pstAiVqeTalkAttr->u32OpenMask);
	return CVI_TRUE;
}

static CVI_BOOL _update_downlink_vqe_setting(AO_VQE_CONFIG_S *pstAoVqeAttr)
{
	if (pstAoVqeAttr == NULL)
		return CVI_FALSE;

	//pstAoVqeAttr->u32OpenMask |= AI_TALKVQE_MASK_ANR;

	printf("[%s][%d]..mask[0x%x]\n", __func__, __LINE__,
	       pstAoVqeAttr->u32OpenMask);

#ifdef NEXT_SSP_ALGO
	pstAoVqeAttr->u32OpenMask |= 0x01;
	AUDIO_SPK_AGC_CONFIG_S st_AGC_Setting;

	st_AGC_Setting.para_agc_max_gain = 0;
	st_AGC_Setting.para_agc_target_high = 8;
	st_AGC_Setting.para_agc_target_low = 72;

	pstAoVqeAttr->stAgcCfg = st_AGC_Setting;
#else
	pstAoVqeAttr->u32OpenMask |= AO_VQE_MASK_AGC;

	AUDIO_ANR_CONFIG_S st_ANR_Setting;
	AUDIO_AGC_CONFIG_S st_AGC_Setting;

	st_AGC_Setting.para_agc_max_gain = 4;
	st_AGC_Setting.para_agc_target_high = 2;
	st_AGC_Setting.para_agc_target_low = 6;
	st_AGC_Setting.para_agc_vad_enable = CVI_TRUE;
	pstAoVqeAttr->stAgcCfg = st_AGC_Setting;
	pstAoVqeAttr->stAnrCfg = st_ANR_Setting;
#endif

	printf("pstAoVqeAttr:u32OpenMask[0x%x]\n", pstAoVqeAttr->u32OpenMask);
	return CVI_TRUE;
}
#if 0
static CVI_BOOL _select_audio_VqeOnOff(AI_TALKVQE_CONFIG_S *pstAiVqeTalkAttr)
{
	fd_set          set;
	struct          timeval timeout = {0};
	CVI_S32 s32Input = 0;
	CVI_BOOL bRet = CVI_FALSE;
	CVI_S32 s32SelectRet;

	printf("-----------------------------------------------------------------\n");
	printf("Do you need VQE(Voice Quality Enhancement before audio encode?)\n");
	printf("0:No  1:Yes\n");
	printf("------------------------------------------------------------------\n");
	printf("Enter option(0/1):\t");
	fflush(stdin);
	FD_ZERO(&set);
	while (1) {
		timeout.tv_sec = 4;
		FD_SET(fileno(stdin), &set);
		printf("enter a number:");
		fflush(stdout);
		s32SelectRet = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
		if (s32SelectRet == -1) {
			printf("[Error]Select function error codec: skip VQE\n");
			return CVI_FALSE;
		}

		if (s32SelectRet != 0) {
			scanf("%d", &s32Input);
			printf("VQE ON/OFF:%d\r\n", s32Input);
			if (s32Input == 0) {
				printf("VQE OFF\n");
				bRet = CVI_FALSE;
			} else if (s32Input == 1) {
				printf("VQE ON\n");
				bRet = CVI_TRUE;
			} else {
				printf("Wrong input[%d]\n", s32Input);
				bRet = CVI_FALSE;
			}
			break;

		} else {
			printf("\r\nTimeout: Select VQE off\r\n");
			break;

		}
	}

	if (bRet == CVI_TRUE) {
		//update vqe setting
		bRet = _update_vqe_setting(pstAiVqeTalkAttr);
	} else
		bRet = CVI_FALSE;


	return bRet;
}

static CVI_BOOL _select_audio_USER_GET_MODE(CVI_VOID)
{
	CVI_BOOL bRet = CVI_FALSE;//bind mode in default
	CVI_S32 s32Input;
	fd_set	set;
	struct	timeval timeout = {0};
	CVI_S32 s32SelectRet;

	printf("-----------------------------------------------------------------\n");
	printf("bind mode : 0 / user get mode : 1\n");
	printf("------------------------------------------------------------------\n");
	printf("Enter option(0/1):\t");
	fflush(stdin);
	fflush(stdout);
	FD_ZERO(&set);
	while (1) {
		timeout.tv_sec = 3;
		FD_SET(fileno(stdin), &set);
		printf("enter a number:");
		fflush(stdout);
		s32SelectRet = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
		if (s32SelectRet == -1) {
			printf("[Error]Select function error codec return g726\n");
			return bRet;
		}

		if (s32SelectRet != 0) {
			scanf("%d", &s32Input);
			printf("INPUT:%d\r\n", s32Input);
			if (s32Input == 0) {
				printf("bind mode\n");
				bRet = CVI_FALSE;
			} else if (s32Input == 1) {
				printf("user mode\n");
				bRet = CVI_TRUE;
			} else {
				printf("Enter invalid num ....select BIND MODE\n");
				bRet = CVI_FALSE;
			}
			break;

		} else {
			printf("\r\nTimeout: Select bind mode\r\n");
			bRet = CVI_FALSE;
			break;

		}
	}

	return bRet;
}
#endif

int _get_record_time(void)
{
	return g_cap_time;
}


static CVI_BOOL _user_get_mode_or_bind_mode(CVI_VOID)
{
	CVI_BOOL bRet = CVI_FALSE;
	CVI_S32 s32Input;

	printf("-----------------------------------------------------------------\n");
	printf("bind mode : 0 / user get mode : 1\n");
	printf("------------------------------------------------------------------\n");
	s32Input =  _CONSOLE_REQ("bind mode: Enter option(0/1):[default:0]:\n", 0);
	if (s32Input == 0) {
		printf("select bindmode\n");
		bRet = CVI_FALSE;
	} else if (s32Input == 1) {
		printf("select user get mode\n");
		bRet = CVI_TRUE;
	} else {
		printf("unrecognized selection[%d]\n", s32Input);
		printf("select bind mode\n");
		bRet = CVI_FALSE;
	}

	return bRet;
}

static PAYLOAD_TYPE_E _select_audio_codec2(CVI_VOID)
{
	PAYLOAD_TYPE_E eType = PT_G726;
	CVI_S32 s32Opt = 0;

	printf("-----------------------------------------------------------------\n");
	printf("select audio codec type:\n");
	printf("0:g726    1:g711A   2:g711Mu   3: adpcm  4.AAC\n");
	printf("------------------------------------------------------------------\n");
	s32Opt =  _CONSOLE_REQ("codec:Enter option(0~4) [default:0]:\n", 0);
	if (s32Opt == 0) {
		printf("Codec G726\n");
		eType = PT_G726;
	} else if (s32Opt == 1) {
		printf("Codec G711A\n");
		eType = PT_G711A;
	} else if (s32Opt == 2) {
		printf("Codec G711Mu\n");
		eType = PT_G711U;
	} else if (s32Opt == 3) {
		printf("Codec PT_ADPCMA\n");
		eType = PT_ADPCMA;
	} else if (s32Opt == 4) {
		printf("Codec AAC_LC\n");
		eType = PT_AAC;
	} else {
		printf("Enter invalid num ....select g726\n");
		eType = PT_G726;
	}

	return eType;
}

#if 0
static PAYLOAD_TYPE_E _select_audio_codec(CVI_VOID)
{
	CVI_S32 s32Input;
	fd_set          set;
	CVI_S32 s32SelectRet;
	struct          timeval timeout = {0};
	PAYLOAD_TYPE_E eType = PT_G726;

	printf("-----------------------------------------------------------------\n");
	printf("select audio codec type:\n");
	printf("0:g726     1:g711A    2:g711Mu    3: adpcm\n");
	printf("------------------------------------------------------------------\n");
	printf("Enter option(0~4):\t");
	fflush(stdin);
	fflush(stdout);

	FD_ZERO(&set);
	while (1) {
		timeout.tv_sec = 5;
		FD_SET(fileno(stdin), &set);
		printf("enter a number:");
		fflush(stdout);
		s32SelectRet = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
		if (s32SelectRet == -1) {
			printf("[Error]Select function error codec return g726\n");
			return PT_G726;
		}

		if (s32SelectRet != 0) {
			scanf("%d", &s32Input);
			printf("INPUT:%d\r\n", s32Input);
			if (s32Input == 0) {
				printf("Codec G726\n");
				eType = PT_G726;
			} else if (s32Input == 1) {
				printf("Codec G711A\n");
				eType = PT_G711A;
			} else if (s32Input == 2) {
				printf("Codec G711Mu\n");
				eType = PT_G711U;
			} else if (s32Input == 3) {
				printf("Codec PT_ADPCMA\n");
				eType = PT_ADPCMA;
			} else if (s32Input == 4) {
				printf("Codec LPCM...not support-> chg to g726\n");
				eType = PT_G726;
			} else {
				printf("Enter invalid num ....select g726\n");
				eType = PT_G726;
			}
			break;

		} else {
			printf("\r\nTimeout: Select codec g726\r\n");
			break;

		}
	}
	fflush(stdin);
	return eType;
}
#endif

/******************************************************************************/
/* function : Ai -> Aenc -> file*/
/*						 -> Adec -> Ao*/
/******************************************************************************/
CVI_S32 SAMPLE_AUDIO_AiAenc(CVI_VOID)
{
	CVI_S32 s32Ret;
	AI_CHN AiChn = 0;
	AO_CHN AoChn = 0;
	ADEC_CHN AdChn = 0;
	AENC_CHN AeChn = 0;
	CVI_BOOL bSendAdec = CVI_FALSE;
	int AudMaxChnCnt = 3;
	FILE *pfd = NULL;
	AIO_ATTR_S stAioAttr;
	CVI_BOOL bEnableResample = CVI_FALSE;
	AUDIO_DEV	AiDev = SAMPLE_AUDIO_INNER_AI_DEV;
	AUDIO_DEV	AoDev = SAMPLE_AUDIO_INNER_AO_DEV;
	//g726 only support mono with 8000 sample rate
	if (stAudTestCfg.bOptCfg == CVI_TRUE) {
		s32Ret = CVI_SUCCESS;
		printf("force skip Unittest...Please using new unittest\n");
		printf("SAMPLE_COMM_AUDIO_StartAenc success\n");
		sleep(1);
		goto AIAENC_ERR6;
	}
	//bSendAdec = (CVI_BOOL)_CONSOLE_REQ("send to adec (0/1)(default:0)\n", 0);
	/********************************************/
	/* step 0-1: setup audin/aenc in recording config*/
	/********************************************/
	stAioAttr.enSamplerate =
		(AUDIO_SAMPLE_RATE_E)
		_CONSOLE_REQ("Enter (Ain/Record)sample rate(default:8000)\n", 8000);
	stAioAttr.u32ChnCnt = AudMaxChnCnt;
	int chnannels = _CONSOLE_REQ("Enter channel numbers(1 or 2)(default:1)\n",
					   1);
	if (chnannels == 1)
		stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
	else if (chnannels == 2)
		stAioAttr.enSoundmode = AUDIO_SOUND_MODE_STEREO;
	else {
		printf("do not support other channel count[%d]\n", chnannels);
		return CVI_FAILURE;
	}
	stAioAttr.u32PtNumPerFrm = 320;//1152 for mp2 //320 for g726
	gs_enPayloadType = _select_audio_codec2();

	stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;
	stAioAttr.u32EXFlag = 0;
	stAioAttr.u32FrmNum = 30;
	stAioAttr.u32ClkSel = 0;
	stAioAttr.enI2sType = AIO_I2STYPE_INNERCODEC;
	/********************************************/
	/*	step 0-2: Do application need VQE */
	/*	setup VQE config for record in */
	/********************************************/
	AI_TALKVQE_CONFIG_S stAiVqeTalkAttr;
	AI_TALKVQE_CONFIG_S *pstAiVqeTalkAttr = (AI_TALKVQE_CONFIG_S *)&stAiVqeTalkAttr;
	CVI_BOOL bVqeOn = CVI_FALSE;
	CVI_BOOL bAecOn = CVI_FALSE;

	memset(&stAiVqeTalkAttr, 0, sizeof(AI_TALKVQE_CONFIG_S));
	printf("Do you need VQE(Voice Quality Enhancement before audio encode?)\n");
	//bVqeOn = _select_audio_VqeOnOff(pstAiVqeTalkAttr);
	bVqeOn = (CVI_BOOL)_CONSOLE_REQ("bVqeOn 0:No  1:Yes (default:0)\n", 0);
	if (bVqeOn == CVI_FALSE)
		pstAiVqeTalkAttr = NULL;
	else {

		if ((stAioAttr.enSamplerate == AUDIO_SAMPLE_RATE_8000) ||
		    (stAioAttr.enSamplerate == AUDIO_SAMPLE_RATE_16000)) {
			_update_vqe_setting(pstAiVqeTalkAttr);

			pstAiVqeTalkAttr->s32WorkSampleRate = stAioAttr.enSamplerate;

			printf("Turn On AEC?\n");
			bAecOn = (CVI_BOOL)_CONSOLE_REQ("bAecOn 0:No  1:Yes (default:0)\n", 0);
			if (bAecOn == CVI_TRUE) {
				_update_aec_setting(pstAiVqeTalkAttr);
				printf("----------------notice-------------------------------\n");
				printf("AEC will need to setup record in to channel Count = 2\n");
				printf("AEC will output only one single channel with 2 channels in\n");
				printf("-----------------------------------------------------\n");

				if (chnannels != 2) {
					printf("AEC channel Count !=2,please reset the parameters\n");
					return CVI_FAILURE;
				}
			}
		} else {
			//not support VQE aside of 8k/16k input
			printf("[ERROR]VQE only support on 8k/16k sample rate\n");
			bVqeOn = CVI_FALSE;
			bAecOn = CVI_FALSE;
		}
	}

	if (gs_enPayloadType == PT_AAC) {
		printf("PT_AAC LC default setting default frm length[%d]\n",
		       AACLC_SAMPLES_PER_FRAME);
		if (bVqeOn == CVI_TRUE) {
			stAioAttr.u32PtNumPerFrm = stAioAttr.u32PtNumPerFrm;//320/640
			printf("AAC_VQE only support on 160 times frame length[%d]\n",
			       stAioAttr.u32PtNumPerFrm);
		} else {
			//cv182x not support period size > 1024 in pcm
			//stAioAttr.u32PtNumPerFrm = AACLC_SAMPLES_PER_FRAME;
		}
	}
	/********************************************/
	/*	step 0-3: Do application need Resample */
	/*	setup Resampler config for record in */
	/********************************************/
	enInSampleRate	= stAioAttr.enSamplerate;
	bEnableResample = (CVI_BOOL)_CONSOLE_REQ("do resample (0/1)(default:0)\n", 0);
	if (bEnableResample == CVI_TRUE) {
		enOutSampleRate =
			(AUDIO_SAMPLE_RATE_E)
			_CONSOLE_REQ("Enter target out sample_rate(default:48000)\n", 48000);
	}
	/********************************************/
	/*	step 0-4: Transfer audio in data in */
	/*	bind mode or user-get mode */
	/********************************************/
	gs_bUserGetMode = _user_get_mode_or_bind_mode();
	g_cap_time = _CONSOLE_REQ("record time (default:20)\n", 20);
	/********************************************/

	/*	step 1: config audio codec*/
	/********************************************/
	s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(&stAioAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto AIAENC_ERR6;
	}

	/********************************************/
	/*	step 2: start Ai*/
	/********************************************/
	u32AiVqeType = CVITEK_AUDIO_TALK_VQE;

	s32Ret = SAMPLE_COMM_AUDIO_StartAi(AiDev, AiChn, &stAioAttr,
					   enOutSampleRate, bEnableResample, pstAiVqeTalkAttr, u32AiVqeType);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto AIAENC_ERR6;
	}

	/********************************************/
	/*step 3: start Aenc*/
	/********************************************/
	if (bAecOn == CVI_TRUE)
		stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
	s32Ret = SAMPLE_COMM_AUDIO_StartAenc(AeChn, &stAioAttr,
					     gs_enPayloadType);
	if (s32Ret != CVI_SUCCESS) {
		printf("error[%s][%d]\n", __func__, __LINE__);
		SAMPLE_DBG(s32Ret);
		goto AIAENC_ERR5;
	}
	/********************************************/
	/*step 4: Aenc bind Ai Chn*/
	/********************************************/
	//strategy : first thread sending data or set to bind mode
	if (gs_bUserGetMode == CVI_TRUE) {
		printf("user mode[get frame by frame]\n");
		s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAiAenc(AiDev, AiChn, AeChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_DBG(s32Ret);
			goto AIAENC_ERR4;
		}
	} else {
		printf("bind mode\n");
		s32Ret = SAMPLE_COMM_AUDIO_AencBindAi(0, 0, 0);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_DBG(s32Ret);
			goto AIAENC_ERR4;
		}
		printf("Ai(%d,%d) bind to AencChn:%d ok!\n", AiDev, AiChn, AeChn);
		if (stAudTestCfg.s32TestMode == TEST_MODE_BIND_AENC_ALSO_GET_AIN_FRAME) {
			//testmode == 1]
			printf("Enter bind and get frame at the same time mode\n");
			ST_TestModeParam stTestMode;

			stTestMode.s32DeviceId = 0;
			stTestMode.s32ChnId = 0;
			stTestMode.s32Cnt = 600;//record 12 seconds for test output
			pthread_create(&stTestModeThread, 0, SAMPLE_AUDIO_TEST_MODE_PROC, &stTestMode);
			pthread_detach(stTestModeThread);
		}
	}

	pfd = SAMPLE_AUDIO_OpenAencFile(AeChn, gs_enPayloadType);
	if (!pfd) {
		SAMPLE_DBG(CVI_FAILURE);
		printf("[Error][%s][%d]SAMPLE_AUDIO_OpenAencFile\n", __func__, __LINE__);
		goto AIAENC_ERR1;
	}

	if (bSendAdec == CVI_FALSE) {
		printf(" not enter decoder...get aenc data out and save to file\n");
		s32Ret =  SAMPLE_COMM_AUDIO_GetAenc(0, pfd);

		if (s32Ret != CVI_SUCCESS) {
			printf("[error]GetAenc thread fail[%s][%d]\n", __func__, __LINE__);
			SAMPLE_DBG(s32Ret);
			goto AIAENC_ERR3;
		}
		printf(">>>>>>>>>>SAMPLE_AUDIO_AiAenc<<<<<<<<<<start recording\n");
	}
	/********************************************/
	/*step 5: start Adec & Ao. ( if you want )*/
	/********************************************/

	if (bSendAdec == CVI_TRUE) {
		printf(" go to decoder\n");
			if (bAecOn == CVI_TRUE) {
				printf("decode AEC out(1 chn)\n");
				chnannels = 1;
				stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
		}
	}

	if (bSendAdec == CVI_TRUE) {
		ADEC_CHN_ATTR_S AdecAttr;


		AdecAttr.s32Sample_rate = stAioAttr.enSamplerate;
		AdecAttr.s32ChannelNums = chnannels;
		AdecAttr.s32frame_size = stAioAttr.u32PtNumPerFrm;
		AdecAttr.s32BytesPerSample = 2;
		if ((bEnableResample == CVI_TRUE) && (gs_enPayloadType == PT_AAC)) {
			AdecAttr.s32Sample_rate = enOutSampleRate;
			AdecAttr.s32frame_size = AACLC_SAMPLES_PER_FRAME;
		}

		s32Ret = SAMPLE_COMM_AUDIO_StartAdec(AdChn, gs_enPayloadType, &AdecAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_DBG(s32Ret);
			goto AIAENC_ERR3;
		}

		s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, AoChn, &stAioAttr,
						   enOutSampleRate, bEnableResample);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_DBG(s32Ret);
			goto AIAENC_ERR2;
		}

		//printf(" check point[%s][%d]\n",__func__,__LINE__);
		s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAencAdec(AeChn, AdChn, pfd);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_DBG(s32Ret);
			goto AIAENC_ERR1;
		}

		if (gs_bUserGetMode == CVI_TRUE) {
			printf("userget mode\n");
			s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAdecAo(AdChn, AoDev, NULL);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_DBG(s32Ret);
				goto AIAENC_ERR0;
			}
		} else {
			printf("bind mode\n");
			s32Ret = SAMPLE_COMM_AUDIO_AoBindAdec(AoDev, AoChn, AdChn);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_DBG(s32Ret);
				goto AIAENC_ERR0;
			}
		}
	}

	if (stAudTestCfg.bOptCfg == CVI_TRUE) {
		printf("user option mode ...unit test mode\n");
		CVI_S32 s32TestCount = 10;

		while (s32TestCount--)
			sleep(1);

		printf("user option mode ...unit test mode...leave\n");
	} else {
		//printf("\nplease press twice ENTER to exit this sample\n");
		//getchar();
		//getchar();
		SAMPLE_COMM_AUDIO_GET_AencGet_ExitFlag();
	}

	/********************************************/
	/*step 6: exit the process*/
	/********************************************/
	if (bSendAdec == CVI_TRUE) {

		if (gs_bUserGetMode != CVI_TRUE) {
			printf("unbind adec to ao\n");
			s32Ret = SAMPLE_COMM_AUDIO_AoUnbindAdec(AoDev, AoChn, AdChn);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_DBG(s32Ret);
				return CVI_FAILURE;
			}
		} else {
			printf("release adec ao thread\n");
			s32Ret = SAMPLE_COMM_AUDIO_DestoryTrdAdecAo(AdChn);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_DBG(s32Ret);
				return CVI_FAILURE;
			}
		}
AIAENC_ERR0:
		//s32Ret |= SAMPLE_COMM_AUDIO_DestoryGetAenc(AeChn);
		s32Ret |= SAMPLE_COMM_AUDIO_DestoryTrdAencAdec(AdChn);

AIAENC_ERR1:
		s32Ret |= SAMPLE_COMM_AUDIO_StopAo(AoDev, AoChn, bEnableResample);
		if (s32Ret != CVI_SUCCESS)
			SAMPLE_DBG(s32Ret);

AIAENC_ERR2:
		s32Ret |= SAMPLE_COMM_AUDIO_StopAdec(AdChn);
		if (s32Ret != CVI_SUCCESS)
			SAMPLE_DBG(s32Ret);

	} else {
		s32Ret = SAMPLE_COMM_AUDIO_DestoryGetAenc(AeChn);
		if (s32Ret != CVI_SUCCESS)
			SAMPLE_DBG(s32Ret);
	}

AIAENC_ERR3:
	if (gs_bUserGetMode == CVI_TRUE) {
		s32Ret |= SAMPLE_COMM_AUDIO_DestoryTrdAi(AiDev, AiChn);
		if (s32Ret != CVI_SUCCESS)
			SAMPLE_DBG(s32Ret);
	} else {
		s32Ret |= SAMPLE_COMM_AUDIO_AencUnbindAi(AiDev, AiChn, AeChn);

		if (stAudTestCfg.s32TestMode == TEST_MODE_BIND_AENC_ALSO_GET_AIN_FRAME) {
			//do not need to call join for testmode
			//pthread_join(stTestModeThread, 0);
		}
		if (s32Ret != CVI_SUCCESS)
			SAMPLE_DBG(s32Ret);
		else
			printf("AiAenc Unbind success\n");
	}
AIAENC_ERR4:
	printf(" go to AIAENC_ERR4\n");
	s32Ret |= SAMPLE_COMM_AUDIO_StopAenc(AeChn);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_DBG(s32Ret);

AIAENC_ERR5:

	s32Ret |= SAMPLE_COMM_AUDIO_StopAi(AiDev, AiChn, bEnableResample,
					   CVI_FALSE);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_DBG(s32Ret);
AIAENC_ERR6:
	printf("SAMPLE_AUDIO_AiAenc  success TEST-PASS\n");
	return s32Ret;
}

/******************************************************************************/
/* function : Ai -> ExtResample -> file*/
/******************************************************************************/
static CVI_S32 _select_audio_channels(CVI_VOID)
{
	CVI_S32 s32Input;
	fd_set          set;
	CVI_S32 s32SelectRet;
	struct          timeval timeout = {0};
	CVI_S32 RetChn = 1;

	printf("-----------------------------------------------------------------\n");
	printf("select audio channel numbers:\n");
	printf("1: channel num =1    2:  channel num =2 , others force set channel num=1\n");
	printf("------------------------------------------------------------------\n");
	printf("Enter option(1 ,2 ):\t");


	FD_ZERO(&set);
	while (1) {
		timeout.tv_sec = 5;
		FD_SET(fileno(stdin), &set);
		printf("enter a number:");
		fflush(stdout);
		s32SelectRet = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
		if (s32SelectRet == -1) {
			printf("[Error]Select function error codec return channel 1\n");
			return 1;
		}

		if (s32SelectRet != 0) {
			scanf("%d", &s32Input);
			printf("INPUT:%d\r\n", s32Input);

			if (s32Input == 1)
				RetChn = 1;
			else if (s32Input == 2)
				RetChn = 2;
			else {
				printf("Enter invalid num ....select channel 1\n");
				RetChn = 1;
			}
			break;

		} else {
			printf("\r\nTimeout: Select channels = 1 \r\n");
			break;

		}
	}

	return RetChn;
}

CVI_S32 SAMPLE_AUDIO_ExtResampleToAout(CVI_VOID)
{
	CVI_S32 s32Ret;
	AO_CHN AoChn = 0;
	FILE *pfd = NULL;
	AIO_ATTR_S stAioAttr;
	AUDIO_DEV AoDev = SAMPLE_AUDIO_INNER_AO_DEV;
	AUDIO_SAMPLE_RATE_E eSrcSampleRate;
	CVI_BOOL bEnInternalResmp = CVI_FALSE;//use external resampler
	char input_filename[256] = {0};
	int input_cmd = 0;
	CVI_U32 u32ChnCnt = 1;
#if 0
	printf("==================\n");
	printf("\n Enter select internal or external(1:internal/ 0: external):");
	scanf("%d", &input_cmd);
	if (input_cmd == 1) {
		printf("internal\n");
		bEnInternalResmp = CVI_TRUE;
	} else if (input_cmd == 0) {
		printf("external\n");
		bEnInternalResmp = CVI_FALSE;
	}
#endif
	printf("\n");
	//[step 1]source raw file/ original sample rate/ channels
	printf("\n Enter src files(in audio raw format):");
	scanf("%s", input_filename);
	printf("\n");
	printf("Enter file sample rate\n");
	printf("sample rate support:\n");
	printf("8000\n");
	printf("11025\n");
	printf("16000\n");
	printf("22050\n");
	printf("24000\n");
	printf("32000\n");
	printf("44100\n");
	printf("48000\n");
	printf("64000\n");
	printf("96000\n\n");
	scanf("%d", &input_cmd);
	eSrcSampleRate = (AUDIO_SAMPLE_RATE_E)input_cmd;
	printf("Enter channel nums of src(1 / 2)\n\n");
	scanf("%d", &u32ChnCnt);
	//[step 2]speaker output sample rate/ channel
	//setup Aout speaker out attribute
	printf("\n Enter speaker out sample rate\n");
	printf("sample rate support:\n");
	printf("8000\n");
	printf("11025\n");
	printf("16000\n");
	printf("22050\n");
	printf("24000\n");
	printf("32000\n");
	printf("44100\n");
	printf("48000\n");
	printf("64000\n");
	printf("96000\n\n");
	scanf("%d", &input_cmd);
	stAioAttr.enSamplerate = (AUDIO_SAMPLE_RATE_E)input_cmd;
	stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;
	stAioAttr.u32EXFlag = 0;
	stAioAttr.u32FrmNum = 30;
	stAioAttr.u32PtNumPerFrm = 320;//160//320//640//1600
	stAioAttr.u32ChnCnt = u32ChnCnt;
	if (stAioAttr.u32ChnCnt  == 1)
		stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
	else
		stAioAttr.enSoundmode = AUDIO_SOUND_MODE_STEREO;
	stAioAttr.u32ClkSel = 0;
	stAioAttr.enI2sType = AIO_I2STYPE_INNERCODEC;

	/********************************************/
	/*step 3: config audio codec*/
	/********************************************/
	s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(&stAioAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto RESAO_ERR3;
	}

	/********************************************/
	/*step 4: open file of audio raw data*/
	/********************************************/
	pfd = fopen(input_filename, "rb");
	if (pfd == NULL) {
		printf("Error [%s]open file[%s]\n", __func__, input_filename);
		s32Ret = CVI_FAILURE;
		goto RESAO_ERR2;
	}

	if (bEnInternalResmp == CVI_FALSE) {
		/********************************************/
		/*step 5: enable extern resample.*/
		/********************************************/
		s32Ret = SAMPLE_AUDIO_InitExtResFun();
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_DBG(s32Ret);
			goto RESAO_ERR2;
		}
		/********************************************/
		/*step 6: setup for audio out--using external resasmpler*/
		/********************************************/
		s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, AoChn, &stAioAttr,
						   eSrcSampleRate, bEnInternalResmp);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_DBG(s32Ret);
			goto RESAO_ERR2;
		}
	} else {
		/********************************************/
		/*step 5/6: setup for audio out--using internal resampler*/
		/********************************************/
		s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, AoChn, &stAioAttr,
						   eSrcSampleRate, bEnInternalResmp);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_DBG(s32Ret);
			goto RESAO_ERR2;
		}
	}

	/********************************************/
	/*step 7: Create thread to resample.*/
	/********************************************/
	s32Ret = SAMPLE_COMM_AUDIO_CreatTrdExtResAout(&stAioAttr, AoDev, AoChn,
			eSrcSampleRate, pfd, bEnInternalResmp);

	struct SAMPLE_EXTRES2AOUT_S *pstRes2Ao =
		(struct SAMPLE_EXTRES2AOUT_S *)&gs_stSampleExtResAout;

	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		printf("[Error]create thread error[%s][%d]\n", __func__, __LINE__);


		if (pstRes2Ao->bStart) {
			pstRes2Ao->bStart = CVI_FALSE;
			pthread_cancel(pstRes2Ao->stAoutPid);
			pthread_join(pstRes2Ao->stAoutPid, 0);
		}
		goto RESAO_ERR2;
	}

	printf("Aout(%d,%d) extern resample ok!\n", AoDev, AoChn);
	do {
		//wait file playiend
		sleep(1);
	} while (pstRes2Ao->bStart);

	if (pstRes2Ao->bStart == CVI_FALSE)
		printf("file play finished!!!!\n");
	else
		printf("stopping resampler to Aout\n");

RESAO_ERR2:
	s32Ret |= SAMPLE_AUDIO_DeInitExtResFun();
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_DBG(s32Ret);

RESAO_ERR3:
	printf("SAMPLE_AUDIO_ExtResampleToAout TEST-PASS\n");
	return s32Ret;
}

CVI_S32 SAMPLE_AUDIO_AiToExtResample(CVI_VOID)
{
	CVI_S32 i = 0, s32Ret;
	AI_CHN	AiChn = 0;
	//AO_CHN AoChn = 0;
	FILE *pfd = NULL;
	AIO_ATTR_S stAioAttr;
	AUDIO_DEV	AiDev = SAMPLE_AUDIO_INNER_AI_DEV;

	stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_16000;
	stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;
	stAioAttr.u32EXFlag = 0;
	stAioAttr.u32FrmNum = 30;
	stAioAttr.u32PtNumPerFrm = 320;//160//320//640//1600
	stAioAttr.u32ChnCnt = _select_audio_channels();
	if (stAioAttr.u32ChnCnt  == 1)
		stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
	else
		stAioAttr.enSoundmode = AUDIO_SOUND_MODE_STEREO;
	stAioAttr.u32ClkSel = 0;
	stAioAttr.enI2sType = AIO_I2STYPE_INNERCODEC;

	//If gs_bAioReSample not true, the output sample rate will not be set.
	//gs_bAioReSample = CVI_TRUE;


	enOutSampleRate = AUDIO_SAMPLE_RATE_48000;
	printf("sample rate support:\n");
	printf("8000\n");
	printf("11025\n");
	printf("16000\n");
	printf("22050\n");
	printf("24000\n");
	printf("32000\n");
	printf("44100\n");
	printf("48000\n");
	printf("64000\n");
	printf("96000\n");
	printf("Current SampleRate[%d]:\n", (int)stAioAttr.enSamplerate);

	if (stAudTestCfg.bOptCfg == CVI_FALSE) {
		printf("Enter output sample rate:");
		scanf("%d", &iOutSampleRateSave);
		enOutSampleRate = (AUDIO_SAMPLE_RATE_E)iOutSampleRateSave;
		printf("\n--->target sample rate from ain:[%d]\n", iOutSampleRateSave);
	} else {
		printf("user option mode / unit test mode\n");
		enOutSampleRate = (AUDIO_SAMPLE_RATE_E)32000;
	}

	/********************************************/
	/*step 1: config audio codec*/
	/********************************************/
	s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(&stAioAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto AIRES_ERR4;
	}

	/********************************************/
	/* step 2: start Ai, disable inner resample.*/
	/********************************************/
	gs_bAioReSample = CVI_FALSE;
	printf("Using external resample mechanism\n");
	s32Ret = SAMPLE_COMM_AUDIO_StartAi(AiDev, AiChn, &stAioAttr,
					   enOutSampleRate, gs_bAioReSample, NULL, 0);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto AIRES_ERR3;
	}

	/********************************************/
	/*step 3: enable extern resample.*/
	/********************************************/
	s32Ret = SAMPLE_AUDIO_InitExtResFun();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto AIRES_ERR2;
	}

	/********************************************/
	/*step 4: Create thread to resample.*/
	/********************************************/
//	for (i = 0; i < s32AiChnCnt; i++) {
	AiChn = i;

	pfd = SAMPLE_AUDIO_OpenResFile(AiChn);
	if (!pfd) {
		SAMPLE_DBG(CVI_FAILURE);
		goto AIRES_ERR1;
	}

	s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAiExtRes(&stAioAttr, AiDev, AiChn,
			enOutSampleRate, pfd);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		SAMPLE_COMM_AUDIO_DestoryTrdAi(AiDev, AiChn);
		goto AIRES_ERR1;
	}

	printf("Ai(%d,%d) extern resample ok!\n", AiDev, AiChn);
//	}


	if (stAudTestCfg.bOptCfg == CVI_TRUE) {
		printf("user option mode ...unit test mode\n");
		CVI_S32 s32TestCount = 10;

		while (s32TestCount--)
			sleep(1);

		printf("user option mode ...unit test mode...leave\n");
	} else {
		printf("\nplease press twice ENTER to exit this sample\n");
		getchar();
		getchar();
	}

	/********************************************/
	/*step 6: exit the process*/
	/********************************************/
AIRES_ERR1:
//	for (i = 0; i < s32AiChnCnt; i++) {
	AiChn = i;

	s32Ret |= SAMPLE_COMM_AUDIO_DestoryTrdAiExtRes(AiDev, AiChn);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_DBG(s32Ret);
//	}

AIRES_ERR2:
	s32Ret |= SAMPLE_AUDIO_DeInitExtResFun();
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_DBG(s32Ret);
AIRES_ERR3:
	s32Ret |= SAMPLE_COMM_AUDIO_StopAi(AiDev, AiChn, gs_bAioReSample,
					   CVI_FALSE);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_DBG(s32Ret);

AIRES_ERR4:
	printf("SAMPLE_AUDIO_AiToExtResample TEST-PASS\n");
	return s32Ret;
}


/******************************************************************************/
/* function : Ai -> Ao(with fade in/out and volume adjust)*/
/******************************************************************************/
CVI_S32 SAMPLE_AUDIO_AiAo(CVI_VOID)
{

	CVI_S32 s32Ret;
	AI_CHN		AiChn = 0;
	AO_CHN		AoChn = 0;
	AIO_ATTR_S stAioAttr;

	AUDIO_DEV	AiDev = SAMPLE_AUDIO_INNER_AI_DEV;
	AUDIO_DEV	AoDev = SAMPLE_AUDIO_INNER_AO_DEV;

	stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_16000;  //change in 20190701
	stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;
	stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;// AUDIO_SOUND_MODE_STEREO;
	stAioAttr.u32EXFlag = 0;

//by testing : single thread mode 128 ok / double thread mode 2048 ok
//by testing :/double thread mode 160 failure
	stAioAttr.u32PtNumPerFrm = 320;//960;//640; //target 160
	printf("update sample audio 0 to 320\n");
	stAioAttr.u32FrmNum = 5;
	//stAioAttr.u32PtNumPerFrm = AACLC_SAMPLES_PER_FRAME;
	//stAioAttr.u32FrmNum = 30;
	stAioAttr.u32ChnCnt = 1;
	stAioAttr.u32ClkSel = 0;
	stAioAttr.enI2sType = AIO_I2STYPE_INNERCODEC;
	gs_bAioReSample = CVI_FALSE;
	/* config ao resample attr if needed */
	if (gs_bAioReSample == CVI_TRUE) {
		/* ai 48k -> 32k */
		enOutSampleRate = AUDIO_SAMPLE_RATE_32000;

		/* ao 32k -> 48k */
		enInSampleRate	= AUDIO_SAMPLE_RATE_32000;
	} else {
		enInSampleRate	= AUDIO_SAMPLE_RATE_BUTT;
		enOutSampleRate = AUDIO_SAMPLE_RATE_BUTT;
	}

	/* resample and anr should be user get mode */
	gs_bUserGetMode = (gs_bAioReSample == CVI_TRUE) ? CVI_TRUE : CVI_FALSE;

	/* config internal audio codec */
	s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(&stAioAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto AIAO_ERR3;
	}

	CVI_S32 s32EnableVqe = 0;
	AI_TALKVQE_CONFIG_S stAiVqeTalkAttr;
	AI_TALKVQE_CONFIG_S *pstAiVqeTalkAttr = (AI_TALKVQE_CONFIG_S *)&stAiVqeTalkAttr;
	CVI_BOOL bAEC_On = (pstAiVqeTalkAttr->u32OpenMask & AI_TALKVQE_MASK_AEC) ?
			   CVI_TRUE : CVI_FALSE;

	s32EnableVqe = 0;
	if (s32EnableVqe) {
		printf("enter default  vqe setting\n");
		SAMPLE_AUDIO_VQE_Setting(pstAiVqeTalkAttr);
		/* For AEC, it must record the two channel */
		stAioAttr.u32ChnCnt = 1;
		stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
#ifndef NEXT_SSP_ALGO
		pstAiVqeTalkAttr->s32FrameSample = stAioAttr.u32PtNumPerFrm;
#endif
		pstAiVqeTalkAttr->s32WorkSampleRate = stAioAttr.enSamplerate;
		pstAiVqeTalkAttr->u32OpenMask = (AI_TALKVQE_MASK_ANR | AI_TALKVQE_MASK_AGC);
	} else
		pstAiVqeTalkAttr = CVI_NULL;

	/* enable AI channel */
	if ((bAEC_On == CVI_TRUE) && (s32EnableVqe > 0)) {
		printf("AEC on ... input channel must be 2\n");
		stAioAttr.u32ChnCnt  = 2;
		stAioAttr.enSoundmode  = AUDIO_SOUND_MODE_STEREO;
	}

	s32Ret = SAMPLE_COMM_AUDIO_StartAi(AiDev, AiChn, &stAioAttr,
					   enOutSampleRate, gs_bAioReSample, pstAiVqeTalkAttr, 0);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto AIAO_ERR3;
	}

	/* enable AO channel */
	if ((bAEC_On == CVI_TRUE) && (s32EnableVqe > 0)) {
		printf("AEC on ... output  channel must be 1\n");
		stAioAttr.u32ChnCnt  = 1;
		stAioAttr.enSoundmode  = AUDIO_SOUND_MODE_MONO;
	}
	usleep(100 * 1000);
	s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, AoChn, &stAioAttr,
					   enInSampleRate, gs_bAioReSample);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto AIAO_ERR2;
	}

	/* bind AI to AO channel */
	if (gs_bUserGetMode == CVI_TRUE) {
		s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAiAo(AiDev, AiChn, AoDev, AoChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_DBG(s32Ret);
			goto AIAO_ERR1;
		}
	} else {
		s32Ret = SAMPLE_COMM_AUDIO_AoBindAi(AiDev, AiChn, AoDev, AoChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_DBG(s32Ret);
			goto AIAO_ERR1;
		}
	}
	printf("ai(%d,%d) bind to ao(%d,%d) ok\n", AiDev, AiChn, AoDev, AoChn);
	gs_bAoVolumeCtrl = CVI_FALSE;
	printf("turn on volume control\n");
	if (gs_bAoVolumeCtrl == CVI_TRUE) {
		s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAoVolCtrl(AoDev);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_DBG(s32Ret);
			printf("volume control test failure\n");
			goto AIAO_ERR0;
		}
	}


	if (gs_bAoVolumeCtrl == CVI_TRUE) {
		s32Ret = SAMPLE_COMM_AUDIO_DestoryTrdAoVolCtrl(AoDev);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_DBG(s32Ret);
			return CVI_FAILURE;
		}
	}

	printf("volume control test finish\n");
	printf("\nplease press twice ENTER to exit this sample\n");
	getchar();
	getchar();

AIAO_ERR0:
	if (gs_bUserGetMode == CVI_TRUE) {
		s32Ret = SAMPLE_COMM_AUDIO_DestoryTrdAi(AiDev, AiChn);
		if (s32Ret != CVI_SUCCESS)
			SAMPLE_DBG(s32Ret);
	} else {
		s32Ret = SAMPLE_COMM_AUDIO_AoUnbindAi(AiDev, AiChn, AoDev, AoChn);
		if (s32Ret != CVI_SUCCESS)
			SAMPLE_DBG(s32Ret);
	}

AIAO_ERR1:
	s32Ret |= SAMPLE_COMM_AUDIO_StopAo(AoDev, AoChn, gs_bAioReSample);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_DBG(s32Ret);
AIAO_ERR2:
	s32Ret |= SAMPLE_COMM_AUDIO_StopAi(AiDev, AiChn, gs_bAioReSample,
					   CVI_FALSE);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_DBG(s32Ret);
AIAO_ERR3:
	printf("SAMPLE_AUDIO_AiAo Test finished\n");
	return s32Ret;
}

/******************************************************************************/
/* function : Ai -> Ao(with fade in/out and volume adjust)*/
/* SAMPLE_AUDIO_AiToAoSysChn is design for	UserGetMode mode */
/******************************************************************************/
CVI_S32 SAMPLE_AUDIO_AiToAoSysChn(CVI_VOID)
{
	CVI_S32 s32Ret;
	AI_CHN AiChn = 0;
	AO_CHN AoChn = 0;
	AIO_ATTR_S stAioAttr;
	AUDIO_DEV	AiDev = SAMPLE_AUDIO_INNER_AI_DEV;
	AUDIO_DEV	AoDev = SAMPLE_AUDIO_INNER_AO_DEV;

	stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_16000;  //change in 20190701
	stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
	//stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_48000;
	//stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_STEREO;
	stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;

	stAioAttr.u32EXFlag = 0;
//by testing : single thread mode 128 ok / double thread mode 2048 ok
//by testing :/double thread mode 160 failure
	stAioAttr.u32FrmNum = 10;
	stAioAttr.u32PtNumPerFrm = 320; //target 160
	stAioAttr.u32ChnCnt = 1;
	//stAioAttr.u32PtNumPerFrm = AACLC_SAMPLES_PER_FRAME;
	//stAioAttr.u32ChnCnt = 2;

	stAioAttr.u32ClkSel = 0;
	stAioAttr.enI2sType = AIO_I2STYPE_INNERCODEC;

	gs_bAioReSample = CVI_FALSE;

	CVI_S32 s32DoResampleTest = 0;

	printf("Need to do the resample test ?\n");
	printf("Yes: 1	/ No: 0    please enter option:\t");
	scanf("%d", &s32DoResampleTest);
	printf("\n");
	printf("\t choose : %d\n", s32DoResampleTest);
	if (s32DoResampleTest)
		gs_bAioReSample = CVI_TRUE;
	else
		gs_bAioReSample = CVI_FALSE;




	/* config ao resample attr if needed */
	if (gs_bAioReSample == CVI_TRUE) {
		/* ai 16k -> 32k */
		enOutSampleRate = AUDIO_SAMPLE_RATE_32000;

		/* ao 32k -> 16k */
		enInSampleRate	= AUDIO_SAMPLE_RATE_32000;
	} else {
		enInSampleRate	= AUDIO_SAMPLE_RATE_BUTT;
		enOutSampleRate = AUDIO_SAMPLE_RATE_BUTT;
	}

	/* resample and anr should be user get mode */
	gs_bUserGetMode = (gs_bAioReSample == CVI_TRUE) ? CVI_TRUE : CVI_FALSE;

	/* config internal audio codec */
	s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(&stAioAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto AIAO_ERR3;
	}

	/* enable AI channel */
	s32Ret = SAMPLE_COMM_AUDIO_StartAi(AiDev, AiChn, &stAioAttr,
					   enOutSampleRate, gs_bAioReSample, NULL, 0);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto AIAO_ERR3;
	}

	/* enable AO channel */
	s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, AoChn, &stAioAttr,
					   enInSampleRate, gs_bAioReSample);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto AIAO_ERR2;
	}

	/* bind AI to AO channel */
	s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAiAo(AiDev, AiChn, AoDev, AO_SYSCHN_CHNID);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto AIAO_ERR1;
	}
	printf("ai(%d,%d) bind to ao(%d,%d) ok\n", AiDev, AiChn, AoDev,
	       AO_SYSCHN_CHNID);


	printf("\nplease press twice ENTER to exit this sample\n");
	getchar();
	getchar();

	s32Ret = SAMPLE_COMM_AUDIO_DestoryTrdAi(AiDev, AiChn);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_DBG(s32Ret);
AIAO_ERR1:
	s32Ret |= SAMPLE_COMM_AUDIO_StopAo(AoDev, AoChn, gs_bAioReSample);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_DBG(s32Ret);
AIAO_ERR2:
	s32Ret |= SAMPLE_COMM_AUDIO_StopAi(AiDev, AiChn, gs_bAioReSample,
					   CVI_FALSE);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_DBG(s32Ret);
AIAO_ERR3:
	printf("SAMPLE_AUDIO_AiToAoSysChn Test finished\n");
	return s32Ret;
}



/******************************************************************************/
/* function : Ai -> Ao*/
/******************************************************************************/
CVI_S32 SAMPLE_AUDIO_AiVqeProcessAo(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	AUDIO_DEV AiDev = SAMPLE_AUDIO_INNER_AI_DEV;
	AUDIO_DEV AoDev = SAMPLE_AUDIO_INNER_AO_DEV;
	AI_CHN AiChn = 0;
	AO_CHN AoChn = 0;
	AIO_ATTR_S stAioAttr;

	CVI_VOID	 *pAiVqeAttr = NULL;

	stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_16000;  //change in 20190701
	stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
	stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;

	stAioAttr.u32EXFlag  = 0;
	stAioAttr.u32FrmNum  = 10;
	stAioAttr.u32PtNumPerFrm = 320; //target 160
	stAioAttr.u32ChnCnt  = 1;
	stAioAttr.u32ClkSel = 0;
	stAioAttr.enI2sType = AIO_I2STYPE_INNERCODEC;

	gs_bAioReSample = CVI_FALSE;
	enInSampleRate	= AUDIO_SAMPLE_RATE_BUTT;
	enOutSampleRate = AUDIO_SAMPLE_RATE_BUTT;

	AI_AEC_CONFIG_S default_AEC_Setting;

	AUDIO_AGC_CONFIG_S default_AGC_Setting;


#ifdef NEXT_SSP_ALGO
	default_AGC_Setting.para_agc_max_gain = 0;
	default_AGC_Setting.para_agc_target_high = 2;
	default_AGC_Setting.para_agc_target_low = 72;
	default_AGC_Setting.para_agc_vad_ena = CVI_TRUE;
#else
	default_AGC_Setting.para_agc_max_gain = 4;
	default_AGC_Setting.para_agc_target_high = 2;
	default_AGC_Setting.para_agc_target_low = 6;
	default_AGC_Setting.para_agc_vad_enable = CVI_TRUE;
#endif

	AUDIO_ANR_CONFIG_S	default_ANR_Setting;


#ifdef NEXT_SSP_ALGO
	default_ANR_Setting.para_nr_snr_coeff = 15;
	default_ANR_Setting.para_nr_init_sile_time = 0;

#else
	default_ANR_Setting.para_nr_snr_coeff = 15;
#endif
	AI_TALKVQE_CONFIG_S stAiVqeTaklAttr;
	AI_RECORDVQE_CONFIG_S stAiVqeRecordAttr;


	if (u32AiVqeType == 1) {
		memset(&stAiVqeRecordAttr, 0, sizeof(AI_RECORDVQE_CONFIG_S));
		stAiVqeRecordAttr.s32WorkSampleRate    = AUDIO_SAMPLE_RATE_16000;

		stAiVqeRecordAttr.s32FrameSample	   = stAioAttr.u32PtNumPerFrm;
		stAiVqeRecordAttr.s32InChNum		   = 1;
		stAiVqeRecordAttr.s32OutChNum			= 1;
		stAiVqeRecordAttr.enWorkstate		   = VQE_WORKSTATE_COMMON;
		stAiVqeRecordAttr.enRecordType			 = VQE_RECORD_NORMAL;
		stAiVqeRecordAttr.u32OpenMask = AI_RECORDVQE_MASK_DRC | AI_RECORDVQE_MASK_HDR |
						AI_RECORDVQE_MASK_HPF | AI_RECORDVQE_MASK_RNR;

		pAiVqeAttr = (CVI_VOID *)&stAiVqeRecordAttr;
		printf("Not support Record VQE in CV1835 ... / Only support upstream(talk) VQE\n");
		goto VQE_ERR2;
	} else if (u32AiVqeType == 2) {
		memset(&stAiVqeTaklAttr, 0, sizeof(AI_TALKVQE_CONFIG_S));

		stAiVqeTaklAttr.s32WorkSampleRate = AUDIO_SAMPLE_RATE_16000;
#ifndef NEXT_SSP_ALGO
		stAiVqeTaklAttr.s32FrameSample = stAioAttr.u32PtNumPerFrm;
		stAiVqeTaklAttr.enWorkstate = VQE_WORKSTATE_COMMON;
#endif
		stAiVqeTaklAttr.stAgcCfg = default_AGC_Setting;
		stAiVqeTaklAttr.stAnrCfg = default_ANR_Setting;
		stAiVqeTaklAttr.stAecCfg = default_AEC_Setting;
		stAiVqeTaklAttr.u32OpenMask = (AI_TALKVQE_MASK_AGC | AI_TALKVQE_MASK_ANR);

		pAiVqeAttr = (CVI_VOID *)&stAiVqeTaklAttr;
	} else {
		/* Not setting attr for vqe */
		pAiVqeAttr = CVI_NULL;
	}

	/********************************************/
	/*step 1: config audio codec*/
	/********************************************/
	s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(&stAioAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto VQE_ERR2;
	}

	/********************************************/
	/*step 2: start Ai*/
	/********************************************/
	s32Ret = SAMPLE_COMM_AUDIO_StartAi(AiDev, AiChn, &stAioAttr,
					   enOutSampleRate, gs_bAioReSample, pAiVqeAttr, u32AiVqeType);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto VQE_ERR2;
	}

	/********************************************/
	/*step 3: start Ao */
	/********************************************/
	s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, AoChn, &stAioAttr,
					   enInSampleRate, gs_bAioReSample);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		goto VQE_ERR1;
	}

	/********************************************/
	/*step 4: Ao bind Ai Chn*/
	/********************************************/
	s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAiAo(AiDev, AiChn, AoDev, AoChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_DBG(s32Ret);
		SAMPLE_COMM_AUDIO_DestoryTrdAi(AiDev, AiChn);
		goto VQE_ERR0;
	}

	printf("bind ai(%d,%d) to ao(%d,%d) ok\n", AiDev, AiChn, AoDev, AoChn);
	printf("\nplease press twice ENTER to exit this sample\n");
	getchar();
	getchar();

	/********************************************/
	/*step 6: exit the process*/
	/********************************************/
	s32Ret = SAMPLE_COMM_AUDIO_DestoryTrdAi(AiDev, AiChn);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_DBG(s32Ret);

VQE_ERR0:
	s32Ret = SAMPLE_COMM_AUDIO_StopAo(AoDev, AoChn, gs_bAioReSample);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_DBG(s32Ret);

VQE_ERR1:
	s32Ret |= SAMPLE_COMM_AUDIO_StopAi(AiDev, AiChn, gs_bAioReSample,
					   CVI_TRUE);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_DBG(s32Ret);
VQE_ERR2:
	printf("SAMPLE_AUDIO_AiVqeProcessAo Test finished\n");
	return s32Ret;
}

/******************************************************************************/
/* function : Ai -> Ao(Hdmi) */
/******************************************************************************/
CVI_S32 SAMPLE_AUDIO_AiHdmiAo(CVI_VOID)
{
	printf("enter function[%s]\n", __func__);
	printf("This path not support in CV18xx\n");
	return CVI_FAILURE;
}

CVI_VOID SAMPLE_AUDIO_Usage(CVI_VOID)
{
	printf("\n\n/Usage:./sample_audio_internal <index>/\n");
	printf("\tindex and its function list below\n");
	printf("\t0:  start AI to AO loop\n");
	printf("\t1:  send AudIn frame to AENC , save as file fd_aenc_out\n");
	printf("\t2:  read encoded file, decode stream & send AudOut & save as fd_adec_output\n");
	printf("\t3:  start AI(VQE process), then send to AO\n");
	printf("\t4:  [Not Support]start AI to AO(Hdmi) loop\n");
	printf("\t5:  start AI to AO(Syschn) loop\n");
	printf("\t6:  start AI to Extern Resampler\n");
	printf("\t7:  cvi_audio version\n");
	printf("\t8:  start Extern Resampler to Aout\n");
	printf("\t9: record file\n");
	printf("\t10: play record-file\n");
	printf("\t11: play record VQE file\n");
	printf("\t12: pcm_io test\n");
	printf("\t14: SetVolume db test\n");
	printf("\t15: GetVolume db test\n");
	printf("\t16: IOCTL Test...set in all the ioctl command\n");
	printf("\t17: Quick test: record 6 seconds and playout 6 seconds\n");
	//printf("\t18: AEC onboard unit test (please put receivein.raw in board first)\n");
	printf("\t19: [sample code]recording frame by frame\n");
	printf("\t20: [sample code]playing audio frame by frame\n");
	printf("\t21: [sample code]sample_audio AEC internal test\n");

}

//----------------------------------------------------------------user option function[start]
#define no_argument		0
#define required_argument	1
#define optional_argument	2

static CVI_VOID SAMPLE_AUDIO_UsageOption(char *const *argv)
{
	printf("\n");
	printf("// ------------------------------------------------\n");
	printf("User can type in sample_audio_internal #num to enter test case by number\n");
	printf("Or user can type in option after test case 9:\n");
	printf("EX.\n");
	printf("sample_audio_internal 9 -c 2 -r 8000 --time	10\n");
	printf("// %s -c codec -w width -h height -i src.yuv -o enc\n", argv[0]);
	printf("// ------------------------------------------------\n");
	printf("\n");
}


typedef struct _optionExt_ {
	struct option opt;
	const char *help;
} st_optionExt;


static st_optionExt options_help[] = {
	{	{ "samplerate", optional_argument, NULL, 'r'},
		"--samplerate audio sampling rate\n"
	},
	{	{ "time", optional_argument, NULL, 't'},
		"--time audio record or play time in seconds\n"
	},
	{	{ "period", optional_argument, NULL, 'p'},
		"--period audio sample size/ period size\n"
	},
	{	{ "format", optional_argument, NULL, 'f'},
		"--format codec format:[pcm, g726, g711u, g711a...]\n"
	},
	{	{ "bitdepth", optional_argument, NULL, 'b'},
		"--bitdepth bitwidth of sample point [16/24/32]\n"
	},
	{	{ "channels", optional_argument, NULL, 'c'},
		"--channels channel numbers of test file\n"
	},
	{	{ "name", required_argument, NULL, 0},
		"--name file you want to record or play\n"
	},
	{	{ "unittest", no_argument, NULL, 'u'},
		"--unittest of test file\n"
	},
	{	{ "testmode", required_argument, NULL, 0},
		"--testmode  for customize verification\n"
	},
	{{ NULL, 0, NULL, 0 }, ""}
};



static CVI_S32	InitUnitTestCfg(ST_AudioUnitTestCfg *cfg, int argc, char **argv)
{

	bool bParse = true;
	CVI_S32 index = 0;
	CVI_S32 ch;
	struct option longOptions[MAX_DBG_TEST_OPTIONS + 1];

	memset((void *)longOptions, 0, sizeof(longOptions));

	if (argc == 3) {
		//snprintf(cfg->filename, "%s", argv[2]);
		if (strncmp(argv[2], "--", 2) == 0) {
			printf("go to option mode with input argc==3\n");
			goto SMP_AUD_OPTION;
		} else
			printf("parsing filename\n");

		memcpy(cfg->filename, argv[2], strlen(argv[2]));
		printf("cvg->filename[%s]\n", cfg->filename);
		cfg->bOptCfg = CVI_FALSE;
		return 1;
	}


SMP_AUD_OPTION:
	printf("sample_audio_internal user option mode\n");
	for (index = 0; index < MAX_DBG_TEST_OPTIONS; index++) {

		if (options_help[index].opt.name == NULL) {
			//printf("[Error]Do not have option name ...\n");
			printf("index[%d]\n", index);
			break;
		}

		if (index >= MAX_DBG_TEST_OPTIONS) {
			printf("[Error]too many options[%d]\n", index);
			return -1;
		}

		memcpy(&longOptions[index], &options_help[index].opt, sizeof(struct option));
	}

	cfg->bOptCfg = CVI_TRUE;
	printf("version 2021  index[%d]\n", index);
	while ((ch = getopt_long(argc, argv, "r:t:p:f:b:c:", longOptions,
				 &index)) != -1) {
		printf("ch = %c,  index = %d, optarg = %s\n", ch, index, optarg);
		switch (ch) {
		case 'r': //sample rate
			cfg->sample_rate =	atoi(optarg);
			break;
		case 't': //record or playout time in second
			cfg->Time_in_second = atoi(optarg);
			break;
		case 'p': //preiod size
			cfg->period_size = atoi(optarg);
			break;
		case 'f': //format (pcm...)
			strcpy(cfg->format, optarg);
			break;
		case 'b': //bit format (16/24/32)
			cfg->bitdepth = atoi(optarg);
			break;
		case 'c': //channel numbers
			cfg->channels = atoi(optarg);
			break;
		case 'u': //channel numbers
			cfg->unit_test = 1;
			break;
		case 0:
			printf("enter option 000000000\n");
			sleep(1);
			if (!strcmp(longOptions[index].name, "name")) {
				//memcpy(cfg->filename, optarg, strlen(optarg));
				//printf("option filename[%s]\n", cfg->filename);
				memcpy(cfg->filename, optarg, strlen(optarg));
			} else if (!strcmp(longOptions[index].name, "samplerate")) {
				cfg->sample_rate = atoi(optarg);
				printf("option samplerate[%d]\n", cfg->sample_rate);
			} else if (!strcmp(longOptions[index].name, "time")) {
				cfg->Time_in_second = atoi(optarg);
				printf("option Time_in_second[%d]\n", cfg->Time_in_second);
			} else if (!strcmp(longOptions[index].name, "period")) {
				cfg->period_size = atoi(optarg);
				printf("option period_size[%d]\n", cfg->period_size);
			} else if (!strcmp(longOptions[index].name, "format")) {
				strcpy(cfg->format, optarg);
				printf("option format[%s]\n", cfg->format);
			} else if (!strcmp(longOptions[index].name, "bitdepth")) {
				cfg->bitdepth = atoi(optarg);
				printf("option bitdepth[%d]\n", cfg->bitdepth);
			} else if (!strcmp(longOptions[index].name, "channels")) {
				cfg->channels = atoi(optarg);
				printf("option channels[%d]\n", cfg->channels);
			} else if (!strcmp(longOptions[index].name, "unittest")) {
				cfg->unit_test = 1;
				printf("option unit_test[%d]\n", cfg->unit_test);
			} else if (!strcmp(longOptions[index].name, "testmode")) {
				printf("option testmode\n");
				cfg->s32TestMode = atoi(optarg);
				printf("option testmode[%d]\n", cfg->s32TestMode);
			} else {
				//not exist
				printf("[Waring]option not exist...\n");
			}

			break;
		default:
			cfg->bOptCfg = CVI_FALSE;
			SAMPLE_AUDIO_UsageOption(argv);
			printf("[Error]No suitable option with input....\n");
			printf("[%s][%d]\n", __func__, __LINE__);
			bParse = false;
			break;
		}

		if (bParse == false)
			break;
	}

	return 1;
}
//----------------------------------------------------------------user option function[end]
/******************************************************************************/
/* function : to process abnormal case*/
/******************************************************************************/
void SAMPLE_AUDIO_HandleSig(CVI_S32 signo)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (SIGINT == signo || SIGTERM == signo) {

		SAMPLE_COMM_AUDIO_DestoryAllTrd();
		//SAMPLE_COMM_SYS_Exit();
		printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
	}

	exit(0);
}


pthread_t stAEC_RecThread;
pthread_t stAEC_PlayThread;

typedef struct _AEC_TEST_STRUCT {
	CVI_S32 s32DevId;
	CVI_S32 s32ChnCnt;
	FILE *fp_rec;
	CVI_S32 s32RecSeconds;
	CVI_S32 s32RecLoops;
	CVI_BOOL bAecOn;
} ST_AEC_TEST_STRUCT;

ST_AEC_TEST_STRUCT gstAecTestStruct;

CVI_VOID *SAMPLE_AUDIO_AEC_LOOP_TEST_GET_FRAME(CVI_VOID *parg)
{
	ST_AEC_TEST_STRUCT *pAecTestStruct = (ST_AEC_TEST_STRUCT *)parg;
	CVI_S32 s32RecLoops = pAecTestStruct->s32RecLoops;
	CVI_S32 s32ChnCnt = pAecTestStruct->s32ChnCnt;
	FILE *fp_record = pAecTestStruct->fp_rec;

	printf("-------->record loops[%d] sec[%d] chn[%d]\n",
	       pAecTestStruct->s32RecLoops,
	       pAecTestStruct->s32RecSeconds,
	       pAecTestStruct->s32ChnCnt);

	AUDIO_FRAME_S stFrame;
	AEC_FRAME_S   stAecFrm;
	CVI_S32 s32Ret = CVI_SUCCESS;
	void *ploadcheck = NULL;

	bRecordingForAEC = CVI_TRUE;


	if (_INPUT_ARG_BY_LIST_MODE) {
		ploadcheck = CVI_AUDIO_LoadingCheck_Init(16000, 2, 5, "sample_audio_aec_all");
	}

	while (s32RecLoops--) {
		//CVI_SYS_TraceBegin("CVI_AI_GetFrame");
		CVI_AUDIO_LoadingCheck_Begin(ploadcheck);
		s32Ret = CVI_AI_GetFrame(0, 0,
					 &stFrame,
					 &stAecFrm,
					 -1);
		//CVI_SYS_TraceEnd();
		CVI_AUDIO_LoadingCheck_End(ploadcheck, (stFrame.u32Len * s32ChnCnt * 2));
		if (s32Ret != CVI_SUCCESS) {
			printf("[Error]CVI_AI_GetFrame none!!\n");
			break;
		}

		if (stFrame.u32Len == 0)
			printf(" block mode return size 0...\n");

		//printf("CVI_AI_GetFrame frm len[%d]\n", stFrame.u32Len);
		if (pAecTestStruct->bAecOn == CVI_TRUE)
			fwrite(stFrame.u64VirAddr[0], 1, (stFrame.u32Len * 1 * 2), fp_record);
		else
			fwrite(stFrame.u64VirAddr[0], 1, (stFrame.u32Len * s32ChnCnt * 2), fp_record);

	}
	printf("--------->record [%d]s finished !!!\n", pAecTestStruct->s32RecSeconds);
	CVI_AUDIO_LoadingCheck_DeInit(ploadcheck);
	bRecordingForAEC = CVI_FALSE;
	return 0;
}

typedef struct _AEC_TEST_SEND_STRUCT {
	AIO_ATTR_S stAioAttr;
	FILE *fp;
} ST_AEC_TEST_SEND_STRUCT;

ST_AEC_TEST_SEND_STRUCT gstAecTestSendStruct;

CVI_VOID *SAMPLE_AUDIO_AEC_LOOP_TEST_SEND_FRAME(CVI_VOID *parg)
{
	ST_AEC_TEST_SEND_STRUCT *pAecTestSendStruct = (ST_AEC_TEST_SEND_STRUCT *)parg;
	AIO_ATTR_S *pstAioAttr = &pAecTestSendStruct->stAioAttr;
	CVI_S32 s32FrameBytes = pstAioAttr->u32ChnCnt * pstAioAttr->u32PtNumPerFrm * 2;
	char *pBuffer = malloc(s32FrameBytes);
	FILE *fp_playfile = pAecTestSendStruct->fp;
	int num_readbytes;
	AUDIO_FRAME_S stFrameSend;
	CVI_S32 s32Ret = CVI_SUCCESS;

	memset(pBuffer, 0, s32FrameBytes);
	printf("enter play thread\n");
	printf("sr:%d,channels:%d,period:%d,s32FrameBytes:%d.\n",
	       pstAioAttr->enSamplerate,
	       pstAioAttr->u32ChnCnt,
	       pstAioAttr->u32PtNumPerFrm,
	       s32FrameBytes);
	printf("first sleep 5 seconds\n");
	sleep(5);
	while (1) {
		memset(pBuffer, 0, s32FrameBytes);
		num_readbytes = fread(pBuffer, 1, s32FrameBytes, fp_playfile);
		if (num_readbytes > 0) {
			stFrameSend.u64VirAddr[0] = (CVI_U8 *)pBuffer;
			stFrameSend.u32Len = pstAioAttr->u32PtNumPerFrm;//samples size for each channel
			stFrameSend.u64TimeStamp = 0;
			stFrameSend.enSoundmode = pstAioAttr->enSoundmode;
			stFrameSend.enBitwidth = AUDIO_BIT_WIDTH_16;
			//printf("ao send u32len[%d]\n", stFrame.u32Len);
			s32Ret = CVI_AO_SendFrame(0, 0, (const AUDIO_FRAME_S *)&stFrameSend, 1000);
			if (s32Ret != CVI_SUCCESS)
				printf("CVI_AO_SendFrame failed with %#x!\n", s32Ret);
		} else {
			printf("num_framebytes %d.\n", num_readbytes);
			(CVI_VOID)fseek(fp_playfile, 0, SEEK_SET);/*read file again*/
			printf("audio replay.....loopback file\n");
			//break;
		}
		if (bRecordingForAEC == CVI_FALSE) {
			printf("stop playing file!!!\n");
			break;
		}
	}

	if (!pBuffer)
		free(pBuffer);
	if (!fp_playfile)
		fclose(fp_playfile);
	printf("play file thread finished!!!\n");
	return 0;
}

CVI_S32 SAMPLE_AUDIO_AEC_LOOP_TEST(CVI_VOID)
{
	printf("Entering AEC loop test\n");
	CVI_AUDIO_INIT();
	printf("Preparing parameters\n");
	CVI_S32 s32DevId = 0;
	AIO_ATTR_S	AudinAttr;
	CVI_S32 s32RecSeconds = 0;
	CVI_S32 s32RecLoops = 0;
	FILE *fp_rec = CVI_NULL;
	FILE *fp_play = CVI_NULL;
	char playfilename[128] = {0};
	char *p_play = playfilename;
	CVI_S32 s32Ret = CVI_SUCCESS;


	memset(&gstAecTestStruct, 0, sizeof(ST_AEC_TEST_STRUCT));
	memset(&gstAecTestSendStruct, 0, sizeof(ST_AEC_TEST_SEND_STRUCT));
	AudinAttr.enSamplerate =
		(AUDIO_SAMPLE_RATE_E)_CONSOLE_REQ("Enter sample rate(default:16000)\n", 16000);
	AudinAttr.u32ChnCnt = _CONSOLE_REQ("Enter channel numbers(1 or 2)(default:2)\n",
					   2);
	if (AudinAttr.u32ChnCnt == 1)
		AudinAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
	else if (AudinAttr.u32ChnCnt == 2)
		AudinAttr.enSoundmode = AUDIO_SOUND_MODE_STEREO;
	else {
		printf("do not support other channel count[%d]\n", AudinAttr.u32ChnCnt);
		return CVI_FAILURE;
	}

	AudinAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	AudinAttr.enWorkmode = AIO_MODE_I2S_MASTER;
	AudinAttr.u32EXFlag = 0;
	AudinAttr.u32FrmNum = 10; /* only use in bind mode */
	AudinAttr.u32PtNumPerFrm =
		_CONSOLE_REQ("Enter period size(samples per frame)(default:320)\n", 320);
	AudinAttr.u32ClkSel = 0;
	AudinAttr.enI2sType = AIO_I2STYPE_INNERCODEC;
	//[Requiring VQE]start---------------------------------------------------------------
	AI_TALKVQE_CONFIG_S stAiVqeTalkAttr;
	AI_TALKVQE_CONFIG_S *pstAiVqeTalkAttr = (AI_TALKVQE_CONFIG_S *)&stAiVqeTalkAttr;
	CVI_BOOL bVqeOn = CVI_FALSE;
	CVI_BOOL bAecOn = CVI_FALSE;

	memset(&stAiVqeTalkAttr, 0, sizeof(AI_TALKVQE_CONFIG_S));//init the value
	bVqeOn = (CVI_BOOL)_CONSOLE_REQ("bVqeOn? 0:No  1:Yes (default:0)\n", 0);
	if (bVqeOn == CVI_FALSE)
		pstAiVqeTalkAttr = NULL;
	else {
		if ((AudinAttr.enSamplerate == AUDIO_SAMPLE_RATE_8000) ||
		    (AudinAttr.enSamplerate == AUDIO_SAMPLE_RATE_16000)) {
			_update_vqe_setting(pstAiVqeTalkAttr);
#ifndef NEXT_SSP_ALGO
			pstAiVqeTalkAttr->s32FrameSample = AudinAttr.u32PtNumPerFrm;
			pstAiVqeTalkAttr->s32BytesPerSample = (AUDIO_BIT_WIDTH_16 + 1);
#endif
			pstAiVqeTalkAttr->s32WorkSampleRate = AudinAttr.enSamplerate;

			//printf("Turn On AEC?\n");
			bAecOn = CVI_TRUE;
			if (bAecOn == CVI_TRUE) {
				_update_aec_setting(pstAiVqeTalkAttr);
				printf("----------------notice-------------------------------\n");
				printf("AEC will need to setup record in to channel Count = 2\n");
				printf("AEC will output only one single channel with 2 channels in\n");
				printf("-----------------------------------------------------\n");
			}
		} else {
			//not support VQE aside of 8k/16k input
			printf("VQE only support on 8k/16k sample rate\n");
			bVqeOn = CVI_FALSE;
			bAecOn = CVI_FALSE;
		}
	}
	//[Requiring VQE]end---------------------------------------------------------------
	s32Ret = CVI_AI_SetPubAttr(s32DevId, &AudinAttr);
	s32Ret = CVI_AI_EnableChn(s32DevId, 0);
	if (bVqeOn == CVI_TRUE) {
		s32Ret = CVI_AI_SetTalkVqeAttr(
				 s32DevId,
				 0,
				 0,
				 0,
				 (AI_TALKVQE_CONFIG_S *)pstAiVqeTalkAttr);
		s32Ret = CVI_AI_EnableVqe(s32DevId, 0);
		if (s32Ret != CVI_SUCCESS) {
			printf("%s: CVI_AI_EnableVqe(%d,0) failed\n",
			       __func__,
			       s32DevId);
			return s32Ret;
		}

	}
	s32Ret = CVI_AI_Enable(s32DevId);
	if (s32Ret == CVI_FAILURE)
		printf("uplink audio setup failure\n");
	s32RecSeconds =
		_CONSOLE_REQ("How many seconds you want to record(default:60s)\n", 60);
	printf("Enter play out filename\n");
	if (_INPUT_ARG_BY_LIST_MODE)
		p_play = (char *)(_parsing_request_name("filename\n", 0));
	else
		scanf("%s", playfilename);

	printf("your file name is[%s]\n", playfilename);
	if (access(p_play, 0) < 0) {
		printf("Error filename not exist\n");
		return CVI_FAILURE;

	} else
		fp_play = fopen(p_play, "rb");

	s32RecLoops = (s32RecSeconds * AudinAttr.enSamplerate) /
		      AudinAttr.u32PtNumPerFrm;
	fp_rec = fopen("sample_record2ch.raw", "wb");
	if (!fp_rec) {
		perror("sample_record2ch.raw open fail");
		return CVI_FAILURE;
	}

	printf("--------------------------------->start recording...\n");
	//create a thread to record
	gstAecTestStruct.s32DevId = s32DevId;
	gstAecTestStruct.s32ChnCnt = 2;
	gstAecTestStruct.fp_rec = fp_rec;
	gstAecTestStruct.s32RecSeconds = s32RecSeconds;
	gstAecTestStruct.s32RecLoops = s32RecLoops;
	if (bAecOn == CVI_TRUE)
		gstAecTestStruct.bAecOn = CVI_TRUE;
	else
		gstAecTestStruct.bAecOn = CVI_FALSE;
	pthread_create(&stAEC_RecThread, 0, SAMPLE_AUDIO_AEC_LOOP_TEST_GET_FRAME,
		       &gstAecTestStruct);
	pthread_detach(stAEC_RecThread);
	//setup record loop to play
	s32Ret |= CVI_AO_SetPubAttr(s32DevId, &AudinAttr);
	s32Ret |= CVI_AO_Enable(s32DevId);
	s32Ret |= CVI_AO_EnableChn(s32DevId, 0);
	if (s32Ret == CVI_FAILURE)
		printf("[Error]in downlink audio setup\n");

	//play_audio_file thread
	memcpy(&gstAecTestSendStruct.stAioAttr, &AudinAttr, sizeof(AIO_ATTR_S));
	gstAecTestSendStruct.fp = fp_play;
	pthread_create(&stAEC_PlayThread, 0, SAMPLE_AUDIO_AEC_LOOP_TEST_SEND_FRAME,
		       &gstAecTestSendStruct);
	pthread_detach(stAEC_PlayThread);

	usleep(1000);
	printf("Ready to exit ....->\n");
	printf("Press Enter to Exit---------------------->\n");
	getchar();
	getchar();
	s32Ret = CVI_AO_DisableChn(0, 0);
	s32Ret = CVI_AO_Disable(0);
	CVI_AUDIO_DEINIT();
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_AUDIO_GET_AUDIO_FRAME_BY_FRAME(CVI_VOID)
{
	CVI_S32 s32Ret;
	CVI_S32 s32DevId = 0;
	CVI_S32 s32RecSeconds = 0;
	CVI_S32 s32RecLoops = 0;
	AIO_ATTR_S	AudinAttr;
	AUDIO_FRAME_S stFrame;
	AEC_FRAME_S   stAecFrm;
	FILE *fp_rec = CVI_NULL;
	int AudMaxChn = 3;

	CVI_AUDIO_INIT();

	AudinAttr.enSamplerate =
		(AUDIO_SAMPLE_RATE_E)_CONSOLE_REQ("Enter sample rate(default:16000)\n", 16000);

	AudinAttr.u32ChnCnt = AudMaxChn;
	int Channels = _CONSOLE_REQ("Enter channel numbers(1 or 2)(default:2)\n",
					   2);

	if (Channels == 1)
		AudinAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
	else if (Channels == 2)
		AudinAttr.enSoundmode = AUDIO_SOUND_MODE_STEREO;
	else {
		printf("do not support other channel count[%d]\n", Channels);
		return CVI_FAILURE;
	}

	AudinAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	AudinAttr.enWorkmode = AIO_MODE_I2S_MASTER;
	AudinAttr.u32EXFlag = 0;
	AudinAttr.u32FrmNum = 10; /* only use in bind mode */
	AudinAttr.u32PtNumPerFrm =
		_CONSOLE_REQ("Enter period size(samples per frame)(default:320)\n", 320);
	AudinAttr.u32ClkSel = 0;
	AudinAttr.enI2sType = AIO_I2STYPE_INNERCODEC;
	//[Requiring VQE]start---------------------------------------------------------------
	AI_TALKVQE_CONFIG_S stAiVqeTalkAttr;
	AI_TALKVQE_CONFIG_S *pstAiVqeTalkAttr = (AI_TALKVQE_CONFIG_S *)&stAiVqeTalkAttr;
	CVI_BOOL bVqeOn = CVI_FALSE;
	CVI_BOOL bAecOn = CVI_FALSE;

	memset(&stAiVqeTalkAttr, 0, sizeof(AI_TALKVQE_CONFIG_S));//init the value
	bVqeOn = (CVI_BOOL)_CONSOLE_REQ("bVqeOn? 0:No  1:Yes (default:0)\n", 0);
	if (bVqeOn == CVI_FALSE)
		pstAiVqeTalkAttr = NULL;
	else {
		if ((AudinAttr.enSamplerate == AUDIO_SAMPLE_RATE_8000) ||
		    (AudinAttr.enSamplerate == AUDIO_SAMPLE_RATE_16000)) {
			_update_vqe_setting(pstAiVqeTalkAttr);
#ifndef NEXT_SSP_ALGO
			pstAiVqeTalkAttr->s32FrameSample = AudinAttr.u32PtNumPerFrm;
			pstAiVqeTalkAttr->s32BytesPerSample = (AUDIO_BIT_WIDTH_16 + 1);
#endif
			pstAiVqeTalkAttr->s32WorkSampleRate = AudinAttr.enSamplerate;

			//printf("Turn On AEC?\n");
			bAecOn = CVI_FALSE;
			if (bAecOn == CVI_TRUE) {
				_update_aec_setting(pstAiVqeTalkAttr);
				printf("----------------notice-------------------------------\n");
				printf("AEC will need to setup record in to channel Count = 2\n");
				printf("AEC will output only one single channel with 2 channels in\n");
				printf("-----------------------------------------------------\n");
			}
		} else {
			//not support VQE aside of 8k/16k input
			printf("VQE only support on 8k/16k sample rate\n");
			bVqeOn = CVI_FALSE;
			bAecOn = CVI_FALSE;
		}
	}
	//[Requiring VQE]end---------------------------------------------------------------
	s32Ret = CVI_AI_SetPubAttr(s32DevId, &AudinAttr);
	s32Ret = CVI_AI_EnableChn(s32DevId, 0);

	if (bVqeOn == CVI_TRUE) {
		s32Ret = CVI_AI_SetTalkVqeAttr(
				 s32DevId,
				 0,
				 0,
				 0,
				 (AI_TALKVQE_CONFIG_S *)pstAiVqeTalkAttr);
		s32Ret = CVI_AI_EnableVqe(s32DevId, 0);
		if (s32Ret != CVI_SUCCESS) {
			printf("%s: CVI_AI_EnableVqe(%d,0) failed\n",
			       __func__,
			       s32DevId);
			return s32Ret;
		}

	}
	s32Ret = CVI_AI_Enable(s32DevId);
	if (s32Ret == CVI_FAILURE)
		printf("uplink audio setup failure\n");
	s32RecSeconds =
		_CONSOLE_REQ("How many seconds you want to record(default:10s)\n", 10);
	s32RecLoops = (s32RecSeconds * AudinAttr.enSamplerate) /
		      AudinAttr.u32PtNumPerFrm;
	fp_rec = fopen("sample_record.raw", "wb");
	printf("--------------------------------->start recording...\n");
	CVI_S32 s32OutputChnCnt = Channels;

	if (bAecOn == CVI_TRUE) {
		//AEC input two channel with one channel output
		s32OutputChnCnt = 1;
	}

	while (s32RecLoops--) {

		s32Ret = CVI_AI_GetFrame(s32DevId, 0,
					 &stFrame,
					 &stAecFrm,
					 CVI_AUDIO_BLOCK_MODE);
		if (s32Ret != CVI_SUCCESS) {
			printf("[Error]CVI_AI_GetFrame none!!\n");
			break;
		}

		if (stFrame.u32Len == 0)
			printf(" block mode return size 0...\n");

		//printf("CVI_AI_GetFrame frm len[%d]\n", stFrame.u32Len);
		if (stFrame.enBitwidth == AUDIO_BIT_WIDTH_16)
			fwrite(stFrame.u64VirAddr[0], 1, (stFrame.u32Len * s32OutputChnCnt * 2),
			       fp_rec);
		else if (stFrame.enBitwidth == AUDIO_BIT_WIDTH_32)
			fwrite(stFrame.u64VirAddr[0], 1, (stFrame.u32Len * s32OutputChnCnt * 4),
			       fp_rec);
		else
			printf("Not support format bitwidth\n");
	}
	printf("rate[%d] chn[%d] outchn[%d]saveing filename[sample_record.raw]\n",
	       AudinAttr.enSamplerate,
	       Channels, s32OutputChnCnt);
	printf("file close SAMPLE_AUDIO_GET_AUDIO_FRAME_BY_FRAME\n");
	//CVI_AUDIO_DEINIT();
	//Fix: Segmentation fault,because xxx_Thread is running and using CycleBuffer.
	SAMPLE_COMM_AUDIO_StopAi(s32DevId, 0, false, bVqeOn);
	fclose(fp_rec);
#if 0
	pthread_t pcm_output_thread;

	pthread_create(&pcm_output_thread, NULL, thread_uplink_audio, NULL);
	pthread_detach(pcm_output_thread);
#endif
	return CVI_SUCCESS;
}

void *play_audio_file(AIO_ATTR_S *pstAioAttr, FILE *fp)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	AUDIO_FRAME_S stFrame;
	//int bStart = CVI_FALSE;
	//uint32_t sleepUs = 0;
	char szThreadName[20] = "downlink";
	int num_readbytes;
	int channel_count = pstAioAttr->enSoundmode + 1;

	CVI_S32 s32FrameBytes = channel_count * pstAioAttr->u32PtNumPerFrm * 2;

	char *pBuffer = malloc(s32FrameBytes);//2ch 16bit 160 samples
	char *pVqeBuffer = malloc(s32FrameBytes);//2ch 16bit 160 samples
	int s32SizeOutBytes = 0;

	if (bDnVqeOn == CVI_TRUE)
		printf("Do the downlink vqe function\n");


	prctl(PR_SET_NAME, szThreadName, 0, 0, 0);
	memset(pBuffer, 0, s32FrameBytes);
	memset(pVqeBuffer, 0, s32FrameBytes);

	printf("sr:%d,channels:%d,period:%d,s32FrameBytes:%d.\n",
	       pstAioAttr->enSamplerate,
	       channel_count, pstAioAttr->u32PtNumPerFrm, s32FrameBytes);
	while (1) {
		memset(pBuffer, 0, s32FrameBytes);
		num_readbytes = fread(pBuffer, 1, s32FrameBytes, fp);
		if (num_readbytes > 0) {
			stFrame.u64VirAddr[0] = (CVI_U8 *)pBuffer;
			stFrame.u32Len = pstAioAttr->u32PtNumPerFrm;//samples size for each channel
			stFrame.u64TimeStamp = 0;
			stFrame.enSoundmode = channel_count - 1;
			stFrame.enBitwidth = AUDIO_BIT_WIDTH_16;
			if (bDnVqeOn == CVI_TRUE) {
				CVI_AudOut_AlgoProcess(pBuffer, pVqeBuffer,
						       num_readbytes,
						       &s32SizeOutBytes);
				if (s32SizeOutBytes != num_readbytes) {
					printf("[Error][%s][%d]\n", __func__, __LINE__);
				}
				stFrame.u64VirAddr[0] = (CVI_U8 *)pVqeBuffer;
			}
			//printf("ao send u32len[%d]\n", stFrame.u32Len);
			s32Ret = CVI_AO_SendFrame(0, 0, (const AUDIO_FRAME_S *)&stFrame, 1000);
			if (s32Ret != CVI_SUCCESS)
				printf("CVI_AO_SendFrame failed with %#x!\n", s32Ret);
		} else {
			printf("num_framebytes %d.\n", num_readbytes);
			break;
		}
	}

	printf("play_audio_file exit.\n");
	if (!pBuffer)
		free(pBuffer);
	if (!pVqeBuffer)
		free(pVqeBuffer);
	if (!fp)
		fclose(fp);

	return 0;
}


#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT	0x20746d66
#define ID_DATA 0x61746164

struct riff_wave_header {
	unsigned int riff_id;
	unsigned int riff_sz;
	unsigned int wave_id;
};

struct chunk_header {
	unsigned int id;
	unsigned int sz;
};

struct chunk_fmt {
	unsigned short audio_format;
	unsigned short num_channels;
	unsigned int sample_rate;
	unsigned int byte_rate;
	unsigned short block_align;
	unsigned short bits_per_sample;
};

static FILE *audio_open_wavfile(const char *filename, int *channels,
				int *sample_rate)
{
	FILE *file;
	struct riff_wave_header riff_wave_header;
	struct chunk_header chunk_header;
	struct chunk_fmt chunk_fmt;
	int more_chunks = 1;

	file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "Unable to open file '%s'\n", filename);
		return NULL;
	}

	fread(&riff_wave_header, sizeof(riff_wave_header), 1, file);
	if ((riff_wave_header.riff_id != ID_RIFF) ||
	    (riff_wave_header.wave_id != ID_WAVE)) {
		fprintf(stderr, "Error: '%s' is not a riff/wave file\n", filename);
		fclose(file);
		return NULL;
	}

	do {
		fread(&chunk_header, sizeof(chunk_header), 1, file);

		switch (chunk_header.id) {
		case ID_FMT:
			fread(&chunk_fmt, sizeof(chunk_fmt), 1, file);
			*sample_rate = chunk_fmt.sample_rate;
			*channels = chunk_fmt.num_channels;
			/* If the format header is larger, skip the rest */
			if (chunk_header.sz > sizeof(chunk_fmt))
				fseek(file, chunk_header.sz - sizeof(chunk_fmt), SEEK_CUR);
			break;
		case ID_DATA:
			/* Stop looking for chunks */
			more_chunks = 0;
			chunk_header.sz = le32toh(chunk_header.sz);
			break;
		default:
			/* Unknown chunk, skip bytes */
			fseek(file, chunk_header.sz, SEEK_CUR);
		}
	} while (more_chunks);
	return file;
}


CVI_S32 SAMPLE_AUDIO_SEND_AUDIO_FRAME_BY_FRAME(ST_AudioUnitTestCfg *testCfg)
{
	CVI_S32 s32Ret = 0;
	FILE *fp_play;
	AIO_ATTR_S AudoutAttr;
	CVI_S32 s32InputFileLen = 0;
	char *p_reqName = NULL;
	int AudMaxChn = 3;
	int channels = 2;

	bDnVqeOn = CVI_FALSE;
	CVI_AUDIO_INIT();

	if (!testCfg->filename && (!_INPUT_ARG_BY_LIST_MODE)) {
		printf("filename is NULL.\n");
		printf("Usage: sample_audio_internal 20 --name $(filename.raw)\n");
		return -1;
	} else if (_INPUT_ARG_BY_LIST_MODE) {
		p_reqName = (char *)(_parsing_request_name("filename\n", 0));
		if (p_reqName == 0) {
			printf("--list mode wo input filename\n");

		} else {
			printf("--list mode filename[%s]\n", p_reqName);
			if (access(p_reqName, 0) < 0) {
				//printf("file not exit ...\n");
				printf("list mode filename not exist\n");
			} else {
				memcpy(testCfg->filename, p_reqName, CVIAUDIO_PARSE_FILE_LENGTH);
				printf("set file name from --list mode\n");
			}
		}
	}

	printf("[%s]filename:%s\n", __func__, testCfg->filename);
	if (access(testCfg->filename, 0) < 0) {
		printf("file not exit ...\n");
		printf("Usage: sample_audio 20 --name $(filename.raw)\n");
		return -1;
	}

	s32InputFileLen = strlen(testCfg->filename);
	if (testCfg->filename[s32InputFileLen - 4] == '.' &&
	    (testCfg->filename[s32InputFileLen - 3] == 'W' ||
	     testCfg->filename[s32InputFileLen - 3] == 'w') &&
	    (testCfg->filename[s32InputFileLen - 2] == 'A' ||
	     testCfg->filename[s32InputFileLen - 2] == 'a') &&
	    (testCfg->filename[s32InputFileLen - 1] == 'V' ||
	     testCfg->filename[s32InputFileLen - 1] == 'v')) {
		printf("Enter wav file\n");
		fp_play = audio_open_wavfile(testCfg->filename,
					     (int *)&channels,
					     (int *)&AudoutAttr.enSamplerate);
	} else {

		printf("Play raw format file(Not a wav file)\n");
		fp_play = fopen(testCfg->filename, "rb");

		channels =
			_CONSOLE_REQ("Enter channel numbers(1 or 2)(default:2)\n", 2);
		AudoutAttr.enSamplerate =
			(AUDIO_SAMPLE_RATE_E)_CONSOLE_REQ("Enter sample rate(default:16000)\n", 16000);

	}

	AudoutAttr.u32ChnCnt = AudMaxChn;
	AudoutAttr.enSoundmode	  = (AUDIO_SOUND_MODE_E)(channels - 1);
	AudoutAttr.enWorkmode	  = AIO_MODE_I2S_MASTER;
	AudoutAttr.u32EXFlag	  = 0;
	AudoutAttr.u32FrmNum	  = 10; /* only use in bind mode */
	AudoutAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	AudoutAttr.u32PtNumPerFrm =
		_CONSOLE_REQ("Enter period size(samples per frame)(default:320)\n", 320);
	//20*AudoutAttr.enSamplerate/1000(320);
	AudoutAttr.u32ClkSel	  = 0;
	AudoutAttr.enI2sType = AIO_I2STYPE_INNERCODEC;
	bDnVqeOn = (CVI_BOOL)_CONSOLE_REQ("Down link VQE on? 0:No  1:Yes (default:0)\n",
					  0);
	if (bDnVqeOn == CVI_TRUE) {
		AO_VQE_CONFIG_S stAoVqeAttr;
		AO_VQE_CONFIG_S *pstAoVqeAttr = (AO_VQE_CONFIG_S *)&stAoVqeAttr;

		memset(pstAoVqeAttr, 0, sizeof(AO_VQE_CONFIG_S));
		if ((AudoutAttr.enSamplerate != AUDIO_SAMPLE_RATE_8000) &&
		    (AudoutAttr.enSamplerate != AUDIO_SAMPLE_RATE_16000)) {
			printf("Downlink VQE only support 8k/16k sample rate\n");
			return CVI_FAILURE;

		} else {
			_update_downlink_vqe_setting(pstAoVqeAttr);
#ifndef NEXT_SSP_ALGO
			pstAoVqeAttr->s32FrameSample = AudoutAttr.u32PtNumPerFrm;
#endif
			pstAoVqeAttr->s32WorkSampleRate = AudoutAttr.enSamplerate;
			s32Ret = CVI_AO_SetVqeAttr(0, 0, pstAoVqeAttr);
			if (s32Ret != CVI_SUCCESS) {
				printf("Error in downlink audio vqe\n");
				return CVI_FAILURE;

			}
			s32Ret =  CVI_AO_EnableVqe(0, 0);
			if (s32Ret != CVI_SUCCESS) {
				printf("Error in CVI_AO_EnableVqe\n");
				return CVI_FAILURE;

			}
		}
	}
	s32Ret |= CVI_AO_SetPubAttr(0, &AudoutAttr);
	s32Ret |= CVI_AO_Enable(0);
	s32Ret |= CVI_AO_EnableChn(0, 0);

	if (s32Ret == CVI_FAILURE)
		printf("[Error]in downlink audio setup\n");


	printf("--------------------------------->start playing...\n");
	play_audio_file(&AudoutAttr, fp_play);
	printf("--------------------------------->stop playing...\n");
	s32Ret = CVI_AO_DisableChn(0, 0);
	s32Ret = CVI_AO_Disable(0);
	CVI_AUDIO_DEINIT();
	bDnVqeOn = CVI_FALSE;
	return 1;
}

int printf_parse_usage(void)
{
	printf("use: sample_audio_internal 1 --list -r [sample_rate]");
	printf(" -c [channel] -p [preiod_size] -C [codec 0:g726 1:g711A 2:g711Mu 3: adpcm 4.AAC]");
	printf(" -V [bVqeOn] -A [bAecOn] -R [bResmp] -b [bind_mode] -T [record time]\n");
	printf("eg:./sample_audio_internal 1 --list -r 8000 -c 2 -p 320 -C 0 -V 1 -A 1 -R 0 -b 0 -T 20\n");
	return 0;
}

/******************************************************************************/
/* function : main*/
/******************************************************************************/

CVI_S32 main(int argc, char *argv[])
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32Index = 0;

	if (argc  <  2) {
		SAMPLE_AUDIO_Usage();
		return CVI_FAILURE;
	}

	u32Index = atoi(argv[1]);

	if (u32Index > 30) {
		SAMPLE_AUDIO_Usage();
		return CVI_FAILURE;
	}

	stAudTestCfg.bOptCfg = CVI_FALSE;

	if (argc >= 3) {
		if (strncmp(argv[2], "--list", 6) == 0) {
			printf_parse_usage();
			if (audio_parse(argc, argv)) {
				printf("input parse error\n");
				return -1;
			}
		} else {
			printf("User option mode...parsing user option....\n");
			InitUnitTestCfg(&stAudTestCfg, argc, argv);
		}

	} else {
		printf("User console mode\n");
		stAudTestCfg.bOptCfg = CVI_FALSE;
		stAudTestCfg.unit_test = 0;
		strcpy(stAudTestCfg.filename, "NULL");
	}


	signal(SIGINT, SAMPLE_AUDIO_HandleSig);
	signal(SIGTERM, SAMPLE_AUDIO_HandleSig);


	//memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	//s32Ret = SAMPLE_COMM_SYS_Init(NULL);
//
	//if (s32Ret != CVI_SUCCESS) {
	//	printf("%s: system init failed with %d!\n", __func__, s32Ret);
	//	return CVI_FAILURE;
	//}

	SAMPLE_AUDIO_AddLibPath();
	printf("cvi_sample_audio:Enter command id =[%d]\n", u32Index);

#ifdef SUPPORT_EXTERNAL_AAC
	s32Ret = CVI_MPI_AENC_AacInit();
	if (s32Ret != CVI_SUCCESS) {
		printf("[Warning]Not support exteranl AAC Encoder load into cviaudio in this version\n");
		printf("Please using middleware/sample/audio/aac_sample to use external AAC!!\n");
	}
	s32Ret = CVI_MPI_ADEC_AacInit();
	if (s32Ret != CVI_SUCCESS) {
		printf("[Warning]Not support exteranl AAC Decoder load into cviaudio in this version\n");
		printf("Please using middleware/sample/audio/aac_sample to use external AAC!!\n");
	}
#endif



	switch (u32Index) {
	case 0: {
		SAMPLE_AUDIO_AiAo();
		break;
	}
	case 1: {
		SAMPLE_AUDIO_AiAenc();
		break;
	}
	case 2: {
		SAMPLE_AUDIO_AdecAo();
		break;
	}
	case 3: {
		SAMPLE_AUDIO_AiVqeProcessAo();
		break;
	}
	case 4: {
		SAMPLE_AUDIO_AiHdmiAo();
		break;
	}
	case 5: {
		SAMPLE_AUDIO_AiToAoSysChn();
		break;
	}
	case 6: {
		SAMPLE_AUDIO_AiToExtResample();
		break;
	}
	//for audio unit test -------------------------------------start
	case 7: {
		printf("[cviaudio]enter debug function!!\n");
		SAMPLE_AUDIO_DEBUG();
		break;
	}
	case 8: {
		printf("[cviaudio]play raw file with External resampler to Aout\n");
		SAMPLE_AUDIO_ExtResampleToAout();
		break;
	}
	case 9: {
		printf("[cviaudio]RECORD FILE !\n");
		SAMPLE_AUDIO_DEBUG_RECORD(&stAudTestCfg);
		break;
	}
	case 10: {
		printf("[cviaudio]PLAY RECORD FILE\n");
		if (strcmp(stAudTestCfg.filename,  "NULL"))
			printf("PLAY_REACORD FILE  name[%s]\n", stAudTestCfg.filename);

		SAMPLE_AUDIO_DEBUG_PLAY(&stAudTestCfg);
		break;
	}
	case 11: {
		printf("[cviaudio]PLAY RECORD FILE AFTER VQE!\n");
		SAMPLE_AUDIO_DEBUG_VQE_PLAY(&stAudTestCfg);
		break;
	}
	case 12: {
		SAMPLE_AUDIO_DEBUG_LEVEL(&stAudTestCfg);
		break;
	}
	case 13: {
		printf("[cviaudio]This is an empty function---start\n");
		printf("[cviaudio]This is an empty function---end\n");
		break;
	}
	case 14: {
		printf("[cviaudio] Set Volume!\n");
		if (_INPUT_ARG_BY_LIST_MODE) {
			//set ain volume
			printf("input set volume by --list mode\n");
			CVI_S32 ain_volume = _parsing_request("ain_volume", -1);
			CVI_S32 aout_volume = _parsing_request("aout_volume", -1);

			if (ain_volume != -1)
				CVI_AI_SetVolume(0, ain_volume);
			if (aout_volume != -1)
				CVI_AO_SetVolume(1, aout_volume);
		} else
			SAMPLE_AUDIO_DEBUG_SET_VOLUME(&stAudTestCfg);

		printf("[cviaudio]SET VOLUME!...end\n");
		break;

	}
	case 15: {
		printf("[cviaudio] Get Volume!\n");
		SAMPLE_AUDIO_DEBUG_GET_VOLUME(&stAudTestCfg);
		printf("[cviaudio]GET VOLUME!...end\n");
		break;
	}
	case 16: {
		printf("[cviaudio] IOCTL Test!\n");
		SAMPLE_AUDIO_DEBUG_IOCTL_TEST();
		printf("[cviaudio]IOCTL Test!...end\n");
		break;
	}
	case 17: {
		printf("[cviaudio] Quick Test!\n");
		SAMPLE_AUDIO_DEBUG_QUICK_TEST();
		printf("[cviaudio] Quick Test!...end\n");
		break;
	}
	case 18: {
		printf("[cviaudio]not support!\n");
		break;
	}
	//for audio unit test -------------------------------------end
	case 19: {
		printf("[sample code]recording frame by frame\n");
		SAMPLE_AUDIO_GET_AUDIO_FRAME_BY_FRAME();
		break;
	}
	case 20: {
		printf("[sample code]playing audio frame by frame\n");
		SAMPLE_AUDIO_SEND_AUDIO_FRAME_BY_FRAME(&stAudTestCfg);
		break;
	}
	case 21: {
		printf("[sample code]AEC self loop test\n");
		SAMPLE_AUDIO_AEC_LOOP_TEST();
		break;
	}

	default: {
		break;
	}
	}
#ifdef SUPPORT_EXTERNAL_AAC
	CVI_MPI_AENC_AacDeInit();
	CVI_MPI_ADEC_AacDeInit();
#endif
	//SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}


