/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_audio_vqe.c
 * Description:The VQE function API shall be implemented here
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "cvi_audio_arch.h"
#include "cvi_comm_aio.h"
//#include "cvi_audio_dnvqe.h"
#include "alog.h"
#include "cvi_audio_dnvqe_adp.h"
#include "cvi_audio_common.h"
#include "cvi_ao_internal.h"
#include "cvi_aud_threads.h"
#include "sharebuffer.h"
#ifdef CVIAUDIO_STATIC
#include "cviaudio_algo_interface2.h"
#endif
/* Debug function and Macro */
#define _VQE_VERSION_TAG_ "cviDnVqe_20211027_support_wo_static"
#define _DNVQE_CFG_FILE "/tmp/downvqe.cfg"
#define DNVQE_UNUSED_REF(X)  ((X) = (X))

#define CVITEK_DOWNLINK_SSP_LIB_NAME "libcvi_ssp2.so"

//CVI_S32 dnvqe_dbg_level = 2;

#define ERR_PRINTF(fmt, args...) \
	do { \
		if (cviaud_dbg > 0) \
			fprintf(stderr, "[Aud_DnVqe][err][%s][%d] "fmt, __func__, __LINE__, ##args);\
	} while (0)

#define DBG_PRINTF(fmt, args...) \
	do { \
		if (cviaud_dbg > 1) \
			fprintf(stderr, "[Aud_DnVqe][info] "fmt, ##args);\
	} while (0)

#define TRA_PRINTF(fmt, args...) \
	do { \
		if (cviaud_dbg > 2) \
			fprintf(stderr, "[Aud_DnVqe][%s][%d] "fmt, __func__, __LINE__, ##args);\
	} while (0)

#define DUM_PRINTF(fmt, args...) \
	do { \
		if (cviaud_dbg > 3) \
			fprintf(stderr, "[Aud_DnVqe] "fmt, ##args);\
	} while (0)

#ifndef DEFAULT_BYTES_PER_SAMPLE
#define DEFAULT_BYTES_PER_SAMPLE 2
#endif

#ifndef DN_FRAMES_LEN
#define DN_FRAMES_LEN 160
#endif

#ifndef DEFAULT_NUM_OF_VQE_CHANNEL
#define DEFAULT_NUM_OF_VQE_CHANNEL 2
#endif

#ifndef SAFE_FREE_BUF
#define SAFE_FREE_BUF(OBJ) {if (NULL != OBJ) {free(OBJ); OBJ = NULL; } }
#endif

//-new version start]
#ifndef CHECK_NULL_PTR
#define CHECK_NULL_PTR(ptr)                                                                                            \
	do {                                                                                                           \
		if (!(ptr)) {                                                                                     \
			printf("func:%s,line:%d, NULL pointer\n", __func__, __LINE__);                                 \
			return CVI_FAILURE;                                                                            \
		}                                                                                                      \
	} while (0)
#endif


#ifndef DNVQE_RET_FAILURE
#define DNVQE_RET_FAILURE(RET, FUNC, LINE)	\
	do {	\
		if (RET != CVI_SUCCESS) {	\
			ERR_PRINTF("[%d][%s]line[%d]\n", RET, FUNC, LINE);	\
			return CVI_FAILURE;	\
		}	\
	} while (0)
#endif

/*replace local variable with pointer & heap alloc*/
#define DNVQE_CALLOC(TYPE, COUNT) ((TYPE *)calloc(COUNT, sizeof(TYPE)))
#define DEFAULT_DNVQE_OUTBUFFER_SIZE 1280
short *poutput;
#define DEFAULT_DNVQE_INBUFFER_SIZE 6400
//char input[6400] = {0};
char *pinput;


//define for function pointer----------------------------start
typedef CVI_VOID * (*pCVIAUDIO_ALGO_INIT_CB)(CVI_S32 s32FunctMask,
					CVI_VOID * param_info);
typedef CVI_S32(*pCVIAUDIO_ALGO_PROCESS_CB)(CVI_VOID *pHandle,
						CVI_S16 *in,
						CVI_S16 *out,
						CVI_S32 iLength);
typedef CVI_VOID(*pCVIAUDIO_ALGO_DEINIT_CB)(CVI_VOID *pHandle);
typedef CVI_VOID(*pCVIAUDIO_ALGO_GETVERSION_CB)(CVI_CHAR *pstrVersion);

typedef struct _ST_CVIAUDIO_ALGO_FUNC {
	CVI_VOID *pLibHandle;
	pCVIAUDIO_ALGO_INIT_CB pCviAud_Algo_Init;
	pCVIAUDIO_ALGO_PROCESS_CB pCviAud_Algo_Process;
	pCVIAUDIO_ALGO_DEINIT_CB pCviAud_Algo_DeInit;
	pCVIAUDIO_ALGO_GETVERSION_CB pCviAud_Algo_GetVersion;
} ST_CVIAUDIO_ALGO_FUNC;

typedef struct _ST_AO_VQE_CHN_CONFIG {
	CVI_BOOL bVqeEnable;
} ST_AO_VQE_CHN_CONFIG;

typedef struct _ST_CVIAUDIO_VQE_AODEV {
	ST_AO_VQE_CHN_CONFIG stAoutChnCfg[CVI_AUD_MAX_CHANNEL_NUM];
} ST_CVIAUDIO_VQE_AODEV;

typedef struct _ST_CVIAUDIO_VQE_INTERNAL {
	CVI_VOID *gVqeAoutHandle;
	AO_VQE_CONFIG_S VqeConfig;
	CVI_S16 *data_in;
	ST_CVIAUDIO_VQE_AODEV stAoutDevCfg[CVI_MAX_AI_DEVICE_ID_NUM];
} ST_CVIAUDIO_VQE_INTERNAL;

static ST_CVIAUDIO_VQE_INTERNAL gstVqeAoInternal = {0};
static ST_CVIAUDIO_ALGO_FUNC	gstDnVqeFunc = {0};

static CVI_S32 CVI_AUD_DnVQE_LoadFunc(CVI_CHAR *pChLibName);
static CVI_S32 CVI_AUD_DnVQE_UnLoadFunc(CVI_VOID);


static CVI_S32 CVI_AUD_DnVQE_UnLoadFunc(CVI_VOID)
{
#ifdef CVIAUDIO_STATIC
	gstDnVqeFunc.pCviAud_Algo_Init = NULL;
	gstDnVqeFunc.pCviAud_Algo_Process = NULL;
	gstDnVqeFunc.pCviAud_Algo_DeInit = NULL;
	gstDnVqeFunc.pCviAud_Algo_GetVersion = NULL;
#else
	if (gstDnVqeFunc.pLibHandle != CVI_NULL) {
		CVI_AudDnVqe_Dlclose(gstDnVqeFunc.pLibHandle);
		memset(&gstDnVqeFunc, 0, sizeof(ST_CVIAUDIO_ALGO_FUNC));
	}
#endif
	return CVI_SUCCESS;
}

static CVI_S32 CVI_AUD_DnVQE_LoadFunc(CVI_CHAR *pChLibName)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ST_CVIAUDIO_ALGO_FUNC _stVqeFunc;
	cviAudioGetDbgMask(&cviaud_dbg);

	pChLibName = pChLibName;
	CVI_AUD_DnVQE_UnLoadFunc();

	memset(&_stVqeFunc, 0, sizeof(ST_CVIAUDIO_ALGO_FUNC));
#ifdef CVIAUDIO_STATIC
	_stVqeFunc.pCviAud_Algo_Init = CviAud_DnAlgo_Init;
	_stVqeFunc.pCviAud_Algo_Process = CviAud_DnAlgo_Process;
	_stVqeFunc.pCviAud_Algo_DeInit = CviAud_DnAlgo_DeInit;
	_stVqeFunc.pCviAud_Algo_GetVersion = CviAud_DnAlgo_GetVersion;
#else
	s32Ret = CVI_AudDnVqe_Dlopen(&(_stVqeFunc.pLibHandle), pChLibName);
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			   __func__, __LINE__, "load VQE lib fail!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_AudDnVqe_Dlsym((CVI_VOID **) &
				 (_stVqeFunc.pCviAud_Algo_Init), _stVqeFunc.pLibHandle,
				 "CviAud_DnAlgo_Init");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			   __func__, __LINE__, "find symbol error!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_AudDnVqe_Dlsym((CVI_VOID **) &
				 (_stVqeFunc.pCviAud_Algo_Process), _stVqeFunc.pLibHandle,
				 "CviAud_DnAlgo_Process");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			   __func__, __LINE__, "find symbol error!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_AudDnVqe_Dlsym((CVI_VOID **) &
				 (_stVqeFunc.pCviAud_Algo_DeInit), _stVqeFunc.pLibHandle,
				 "CviAud_DnAlgo_DeInit");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			   __func__, __LINE__, "find symbol error!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_AudDnVqe_Dlsym((CVI_VOID **) &
				 (_stVqeFunc.pCviAud_Algo_GetVersion), _stVqeFunc.pLibHandle,
				 "CviAud_DnAlgo_GetVersion");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			   __func__, __LINE__, "find symbol error!\n");
		return CVI_FAILURE;
	}
#endif
	memcpy(&gstDnVqeFunc, &_stVqeFunc, sizeof(ST_CVIAUDIO_ALGO_FUNC));

	return s32Ret;
}




CVI_S32 CVI_AUD_DNVQE_SaveParamToCfg(char *filename, CVI_VOID *param)
{
	FILE *fp = fopen(filename, "w");
	if (fp == CVI_NULL) {
		printf("cviaudio Error[%s][%d]\n", __func__, __LINE__);
		return -1;
	}

	fwrite(param, sizeof(AO_VQE_CONFIG_S), 1, fp);
	fclose(fp);
	return CVI_SUCCESS;
}

CVI_S32 CVI_AUD_DNVQE_LoadParamFromCfg(char *filename, CVI_U8 *buf)
{

	FILE *fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		printf("cviaudio Error[%s][%d]\n", __func__, __LINE__);
		return -1;
	}
	fread(buf, sizeof(AO_VQE_CONFIG_S), 1, fp);
	fclose(fp);
	return CVI_SUCCESS;
}


CVI_S32 CVI_AO_SetVqeAttr(AUDIO_DEV AoDevId, AO_CHN AoChn,
			const AO_VQE_CONFIG_S *pstVqeConfig)
{
	DNVQE_UNUSED_REF(AoDevId);
	DNVQE_UNUSED_REF(AoChn);
	DNVQE_UNUSED_REF(pstVqeConfig);

	cviAudioGetDbgMask(&cviaud_dbg);
	//step 1: update vqe config
	if (access(_DNVQE_CFG_FILE, F_OK) == 0) {
		printf("load dnvqe param %s\n", _DNVQE_CFG_FILE);
		CVI_AUD_DNVQE_SaveParamToCfg(_DNVQE_CFG_FILE, (CVI_VOID *)&gstVqeAoInternal.VqeConfig);
		CVI_AUD_DNVQE_LoadParamFromCfg(_DNVQE_CFG_FILE, (CVI_U8 *)&gstVqeAoInternal.VqeConfig);
	} else
		memcpy(&gstVqeAoInternal.VqeConfig, pstVqeConfig, sizeof(AO_VQE_CONFIG_S));
	return CVI_SUCCESS;
}

CVI_S32 CVI_AO_GetVqeAttr(AUDIO_DEV AoDevId, AO_CHN AoChn,
			AO_VQE_CONFIG_S *pstVqeConfig)
{
	DNVQE_UNUSED_REF(AoDevId);
	DNVQE_UNUSED_REF(AoChn);
	DNVQE_UNUSED_REF(pstVqeConfig);
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		ERR_PRINTF("\n");
		return CVI_FAILURE;
	}

	memcpy(pstVqeConfig, &gstVqeAoInternal.VqeConfig, sizeof(AO_VQE_CONFIG_S));

	return CVI_FAILURE;
}


CVI_S32 CVI_AO_EnableVqe(AUDIO_DEV AoDevId, AO_CHN AoChn)
{
	cviAudioGetDbgMask(&cviaud_dbg);
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		ERR_PRINTF("\n");
		return CVI_FAILURE;
	}
	DNVQE_UNUSED_REF(AoChn);
	char _version[256] = {0};
	CVI_VOID *_handle = CVI_NULL;
	AO_VQE_CONFIG_S *_pstVqeConfig = NULL;
	CVI_S32 s32Ret = CVI_SUCCESS;

	_pstVqeConfig = (AO_VQE_CONFIG_S *)(&gstVqeAoInternal.VqeConfig);
	if (_pstVqeConfig->s32channels <= 0) {
		ERR_PRINTF("dnvqe channel:%d\n", _pstVqeConfig->s32channels);
		return CVI_FAILURE;
	}

	//step 3: load libcvi_ssp2.so function
	s32Ret = CVI_AUD_DnVQE_LoadFunc(CVITEK_DOWNLINK_SSP_LIB_NAME);
	DNVQE_RET_FAILURE(s32Ret, __func__, __LINE__);
	CHECK_NULL_PTR(gstDnVqeFunc.pCviAud_Algo_Init);
	CHECK_NULL_PTR(gstDnVqeFunc.pCviAud_Algo_GetVersion);
	CHECK_NULL_PTR(gstDnVqeFunc.pCviAud_Algo_Process);
	CHECK_NULL_PTR(gstDnVqeFunc.pCviAud_Algo_DeInit);
	gstDnVqeFunc.pCviAud_Algo_GetVersion(_version);
	printf("xxxxxx downlink vqe for Aout[%s]\n", _version);
	//step 4: trigger algo init function after loading success!!
	_handle = gstDnVqeFunc.pCviAud_Algo_Init(gstVqeAoInternal.VqeConfig.u32OpenMask,
						(CVI_VOID *)_pstVqeConfig);
	if (_handle == CVI_NULL) {
		ERR_PRINTF("return NULL handle\n");
		return CVI_FAILURE;
	}
	gstVqeAoInternal.gVqeAoutHandle = _handle;
	gstVqeAoInternal.stAoutDevCfg[AoDevId].stAoutChnCfg[AoChn].bVqeEnable = CVI_TRUE;
	//gstVqeAoInternal.data_in = (short *)malloc(s32Size);
	if (poutput == NULL)
		poutput = DNVQE_CALLOC(short, DEFAULT_DNVQE_OUTBUFFER_SIZE);
	if (poutput == NULL)
		ERR_PRINTF("Not enough mem for dnvqe output buffer\n");

	if (pinput == NULL)
		pinput = DNVQE_CALLOC(char, DEFAULT_DNVQE_INBUFFER_SIZE);
	if (pinput == NULL)
		ERR_PRINTF("Not enough mem for dnvqe input buffer\n");
	return CVI_SUCCESS;
}

CVI_S32 CVI_AO_DisableVqe(AUDIO_DEV AoDevId, AO_CHN AoChn)
{
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		ERR_PRINTF("\n");
		return CVI_FAILURE;
	}
	int timeoutCnt = 0;
	ST_AO_INSTANCE *pstAoInstance = &gstAoInstance[AoDevId];
	CVI_ST_AUD_TRACK_INFO *pstTrackInfo = pstAoInstance->pastTrackInfo[AoChn];
	int iFrameBytes = DEFAULT_BYTES_PER_SAMPLE * pstTrackInfo->iChannels *
								pstAoInstance->ao_attrs.u32PtNumPerFrm;

	DNVQE_UNUSED_REF(AoChn);
	CHECK_NULL_PTR(gstDnVqeFunc.pCviAud_Algo_DeInit);

	while ((share_cyclebuffer_frameready_size(pstTrackInfo->iShmMemIndex) >= iFrameBytes) && timeoutCnt < 50) {
		usleep(10 * 1000);
		timeoutCnt++;
	}
	printf("[rachel][CVI_AO_DisableVqe]AoDevId:%d, AoChn:%d, timeoutCnt:%d, iFrameBytes:%d\n", AoDevId, AoChn, timeoutCnt, iFrameBytes);
	gstDnVqeFunc.pCviAud_Algo_DeInit(gstVqeAoInternal.gVqeAoutHandle);
	gstVqeAoInternal.stAoutDevCfg[AoDevId].stAoutChnCfg[AoChn].bVqeEnable = CVI_FALSE;
	//SAFE_FREE_BUF(gstVqeAoInternal.data_in);
	SAFE_FREE_BUF(poutput);
	SAFE_FREE_BUF(pinput);
	return CVI_SUCCESS;
}


CVI_BOOL CVI_AO_VQECheckEnable(AUDIO_DEV AoDevId, AO_CHN AoChn)
{
	cviAudioGetDbgMask(&cviaud_dbg);
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		ERR_PRINTF("\n");
		return CVI_FAILURE;
	}
	DNVQE_UNUSED_REF(AoChn);
	DNVQE_UNUSED_REF(AoDevId);
	CVI_BOOL bCheck = CVI_FALSE;

	bCheck = gstVqeAoInternal.stAoutDevCfg[AoDevId].stAoutChnCfg[AoChn].bVqeEnable;
	return bCheck;
}

//BELOW function is for unit test treated dnvqe as single modules----start

CVI_S32 CVI_AudOut_AlgoProcess(CVI_CHAR *datain,
				CVI_CHAR *dataout,
				CVI_S32 s32SizeInBytes,
				CVI_S32 *s32SizeOutBytes)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32SizeInSamples = s32SizeInBytes / DEFAULT_BYTES_PER_SAMPLE / gstVqeAoInternal.VqeConfig.s32channels;
	CVI_S32 s32RetSamples = 0;
	CVI_S32 s32RemainSizeSample = s32SizeInSamples;
	CVI_S32 s32PerByte = gstVqeAoInternal.VqeConfig.s32channels * DEFAULT_BYTES_PER_SAMPLE;
	//short output[1280] = {0};
	//char input[6400] = {0};
	CVI_CHAR *ps8DataIn = (CVI_CHAR *)pinput;
	CVI_CHAR *ps8DataOut = (CVI_CHAR *)dataout;

	CHECK_NULL_PTR(poutput);
	CHECK_NULL_PTR(pinput);

	if (s32SizeInBytes > 6400 || (s32RemainSizeSample % DN_FRAMES_LEN)) {
		ERR_PRINTF("pinput size too large[%d]\n", s32SizeInBytes);
		return CVI_FAILURE;

	} else {
		memcpy(pinput, datain, s32SizeInBytes);
		ps8DataIn = pinput;
	}

	CHECK_NULL_PTR(datain);
	CHECK_NULL_PTR(dataout);
	CHECK_NULL_PTR(gstVqeAoInternal.gVqeAoutHandle);
	CHECK_NULL_PTR(gstDnVqeFunc.pCviAud_Algo_Process);

	*s32SizeOutBytes = 0;

	while (s32RemainSizeSample >= DN_FRAMES_LEN) {
		s32RetSamples = gstDnVqeFunc.pCviAud_Algo_Process(
					gstVqeAoInternal.gVqeAoutHandle,
					(CVI_S16 *)ps8DataIn,
					(CVI_S16 *)poutput,
					DN_FRAMES_LEN);
		if (s32RetSamples <= 0) {
			ERR_PRINTF("abnormal\n");
			s32Ret = CVI_FAILURE;
		}

		s32RemainSizeSample -= (DN_FRAMES_LEN);
		memcpy((char  *)ps8DataOut, (char *)poutput, s32PerByte * s32RetSamples);
		*s32SizeOutBytes += (s32PerByte * s32RetSamples);

		if (s32RemainSizeSample) {
			ps8DataIn += DN_FRAMES_LEN * s32PerByte;
			ps8DataOut += (s32PerByte * s32RetSamples);
		}
	}
	if (s32RemainSizeSample != 0) {
		ERR_PRINTF("remaining data ...not process AoutAlgo\n");
		if (s32RemainSizeSample != 0) {
			/* Not return */
			ERR_PRINTF("Please set frame sample 160x..remain[%d]smpls\n", s32RemainSizeSample);
		}
	}

	return s32Ret;
}
//stop
