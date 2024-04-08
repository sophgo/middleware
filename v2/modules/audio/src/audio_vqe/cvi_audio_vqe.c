/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_audio_vqe.c
 * Description:The VQE function API shall be implemented here
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "cvi_audio_arch.h"
#include "cvi_comm_aio.h"
#include "cvi_type.h"
#include "alog.h"
#include "cvi_audio_vqe.h"  //api release for uppper layer
#include "cvi_audio_vqe_load.h"
#include "cvi_audio_dl_adp.h"
#include "farend_buffer.h"
#ifdef CVIAUDIO_STATIC
#include "cviaudio_algo_interface.h"
#include "../../include/cvi_audio_interface_tinyalsa.h"
#endif
#ifdef RPC_MULTI_PROCESS_AUDIO
#include "cvi_audio_rpc.h"
#endif
#ifdef AUD_SUPPORT_KERNEL_MODE
#include "cviaudio_kernel_mode.h"
#endif
/* Debug function and Macro */
#define _VQE_VERSION_TAG_ "cvivqe_20220520_support_ao_speech_EQ_static"
#define VQE_UNUSED_REF(X)  ((X) = (X))


//#define MORAN_AEC_LIB_NAME "libaec_ext.so"//external algorithm ver1
#define CVITEK_AEC_LIB_NAME "libaec.so"
#define CVITEK_SSP_LIB_NAME "libssp.so"//new version for zkt
#define CVITEK_SSP_NOTCH_LIB_NAME "libcvi_ssp.so"
#define ONLINE_MODE_FILE_PATH "/mnt/data/audvqe.cfg"
#define VQE_CURRENT_PARAMETERS "/tmp/audvqe.cfg"
#define VQE_PATH_DEFAULT_MODE 0


//CVI_S32 cviaud_dbg_level = 2;

int cviaud_dbgg = 4;

#define ERR_PRINTF(fmt, args...) \
	do { \
		if (cviaud_dbgg > 0) \
			fprintf(stderr, "[cvi_AudVqe][err][%s][%d] "fmt, __func__, __LINE__, ##args);\
	} while (0)



#define DBG_PRINTF(fmt, args...) \
	do { \
		if (cviaud_dbgg > 1) \
			fprintf(stderr, "[cvi_AudVqe][info] "fmt, ##args);\
	} while (0)



#define TRA_PRINTF(fmt, args...) \
	do { \
		if (cviaud_dbgg > 2) \
			fprintf(stderr, "[cvi_AudVqe][%s][%d] "fmt, __func__, __LINE__, ##args);\
	} while (0)



#define DUM_PRINTF(fmt, args...) \
	do { \
		if (cviaud_dbgg > 3) \
			fprintf(stderr, "[cvi_AudVqe] "fmt, ##args);\
	} while (0)

#ifndef DEFAULT_BYTES_PER_SAMPLE
#define DEFAULT_BYTES_PER_SAMPLE 2
#endif

#ifndef AEC_FRAMES_LEN
#define AEC_FRAMES_LEN 160
#endif

#ifndef DEFAULT_NUM_OF_VQE_CHANNEL
#define DEFAULT_NUM_OF_VQE_CHANNEL 2
#endif

#ifndef MAX_VQE_MIC_IN_NUM
#define MAX_VQE_MIC_IN_NUM 8
#endif

#ifndef SAFE_FREE_BUF
#define SAFE_FREE_BUF(OBJ) {if (NULL != OBJ) {free(OBJ); OBJ = NULL; } }
#endif


AUDIO_VQE_REGISTER_S gstVqeReg = {0};
/*replace local variable with pointer & heap alloc*/
#define VQE_CALLOC(TYPE, COUNT) ((TYPE *)calloc(COUNT, sizeof(TYPE)))
//ex: VQE_CALLOC(FLOAT, numberOfNoneZero);
short *poutput_aec;
short *poutput_anr;
char *pinput_anr;
int bDevVqeStatus[CVI_MAX_AI_DEVICE_ID_NUM];
pthread_mutex_t glock = PTHREAD_MUTEX_INITIALIZER;
static int bVqeInited = CVI_FALSE;
static int gVqePath = VQE_PATH_DEFAULT_MODE;
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

#ifndef CHECK_NULL_PTR_WO_PRINT
#define CHECK_NULL_PTR_WO_PRINT(ptr)          \
	do {                                   \
		if (!(ptr)) {                                                                                     \
			return CVI_FAILURE;                                                                            \
		}                                                                                                      \
	} while (0)
#endif

#define DEBUG_VQE2 0
#if DEBUG_VQE2
	FILE *ptr;
	FILE *ptr_org;
#endif


#ifndef VQE_RET_FAILURE
#define VQE_RET_FAILURE(ret)	\
	do {	\
		if (ret != CVI_SUCCESS) {	\
			ERR_PRINTF("\n");	\
			return ret;	\
		}	\
	} while (0)
#endif
#define INPUT_VQE_TMP_BUFFER_BYTES 6400
//define for function pointer----------------------------start
typedef CVI_VOID * (*pCVIAUDIO_ALGO_INIT_CB)(CVI_S32 s32FunctMask,
												CVI_VOID *param_info);
typedef CVI_S32(*pCVIAUDIO_ALGO_PROCESS_CB)(CVI_VOID *pHandle,
											CVI_S16 *mic_in,
											CVI_S16 *ref_in,
											CVI_S16 *out,
											CVI_S32 iLength);
typedef CVI_S32(*pCVIAUDIO_SPK_ALGO_PROCESS_CB)(CVI_VOID *pHandle,
											CVI_S16 *spk_in,
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
	pCVIAUDIO_ALGO_INIT_CB pCviAud_SpkAlgo_Init;
	pCVIAUDIO_SPK_ALGO_PROCESS_CB pCviAud_SpkAlgo_Process;
	pCVIAUDIO_ALGO_DEINIT_CB pCviAud_SpkAlgo_DeInit;
} ST_CVIAUDIO_ALGO_FUNC;

typedef struct _ST_CVIAUDIO_SW_AEC {
	CVI_S32 s32SampleRate;
	CVI_S32 s32Channels;
	CVI_S32 s32PeriodSamples;
	CVI_S32 s32FarEndBufferLength;
	char *pDemuxBuffer;
	char *pTmpBuffer;
	void *FarendHandle;
	CVI_BOOL bEnableSwAec;
	CVI_BOOL bFirstReadRdy;
} ST_CVIAUDIO_SW_AEC;

typedef struct _ST_AI_VQE_CHN_CONFIG {
	CVI_BOOL bVqeEnable;
	CVI_BOOL bEnableRefAEC;
} ST_AI_VQE_CHN_CONFIG;
typedef struct _ST_CVIAUDIO_VQE_AINDEV {
	ST_AI_VQE_CHN_CONFIG stAinChnCfg[CVI_AUD_MAX_CHANNEL_NUM];
} ST_CVIAUDIO_VQE_AINDEV;

typedef struct _ST_CVIAUDIO_VQE_INTERNAL {
	CVI_VOID *gVqeAinHandle;
	AI_TALKVQE_CONFIG_S talkvqe;
	CVI_S16 *mic_in;
	CVI_S16 *ref_in;
	ST_CVIAUDIO_VQE_AINDEV stAinDevCfg[CVI_MAX_AI_DEVICE_ID_NUM];
} ST_CVIAUDIO_VQE_INTERNAL;


static ST_CVIAUDIO_VQE_INTERNAL gstVqeAinInternal = {0};
static ST_CVIAUDIO_ALGO_FUNC	gstVqeFunc = {0};
ST_CVIAUDIO_SW_AEC *pSwAecHandle;
//define for function pointer----------------------------end
//define the static function ----------------------------start
static CVI_S32 CVI_AUD_VQE_LoadFunc(CVI_CHAR *pChLibName);
static CVI_S32 CVI_AUD_VQE_UnLoadFunc(CVI_VOID);
static CVI_S32 _select_init_ssp(const AI_TALKVQE_CONFIG_S *pstVqeConfig);
static CVI_S32 _select_init_path(CVI_S32 eVqePath);


//define the static function ------------------
//new version end]----------stop
static CVI_S32 CVI_AUD_VQE_UnLoadFunc(CVI_VOID)
{
#ifdef CVIAUDIO_STATIC
	printf("cvi_audio_vqe not using dlopen\n");
	gstVqeFunc.pCviAud_Algo_Init = NULL;
	gstVqeFunc.pCviAud_Algo_Process = NULL;
	gstVqeFunc.pCviAud_Algo_DeInit = NULL;
	gstVqeFunc.pCviAud_Algo_GetVersion = NULL;
	gstVqeFunc.pCviAud_SpkAlgo_Init = NULL;
	gstVqeFunc.pCviAud_SpkAlgo_Process = NULL;
	gstVqeFunc.pCviAud_SpkAlgo_DeInit = NULL;
#else
	if (gstVqeFunc.pLibHandle != CVI_NULL) {
		CVI_Audio_Dlclose(gstVqeFunc.pLibHandle);
		memset(&gstVqeFunc, 0, sizeof(ST_CVIAUDIO_ALGO_FUNC));
	}
#endif
	return CVI_SUCCESS;
}

static CVI_S32 CVI_AUD_VQE_LoadFunc(CVI_CHAR *pChLibName)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ST_CVIAUDIO_ALGO_FUNC _stVqeFunc;

	cviAudioGetDbgMask(&cviaud_dbg);
	pChLibName = pChLibName;
	CVI_AUD_VQE_UnLoadFunc();

	memset(&_stVqeFunc, 0, sizeof(ST_CVIAUDIO_ALGO_FUNC));
#ifdef CVIAUDIO_STATIC
	_stVqeFunc.pCviAud_Algo_Init = CviAud_Algo_Init;
	_stVqeFunc.pCviAud_Algo_Process = CviAud_Algo_Process;
	_stVqeFunc.pCviAud_Algo_DeInit = CviAud_Algo_DeInit;
	_stVqeFunc.pCviAud_Algo_GetVersion = CviAud_Algo_GetVersion;
	_stVqeFunc.pCviAud_SpkAlgo_Init = CviAud_SpkAlgo_Init;
	_stVqeFunc.pCviAud_SpkAlgo_Process = CviAud_SpkAlgo_Process;
	_stVqeFunc.pCviAud_SpkAlgo_DeInit = CviAud_SpkAlgo_DeInit;
#else
	s32Ret = CVI_Audio_Dlopen(&(_stVqeFunc.pLibHandle), pChLibName);
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			   __func__, __LINE__, "load VQE lib fail!\n");
		return CVI_ERR_AI_VQE_ERR;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &
				 (_stVqeFunc.pCviAud_Algo_Init), _stVqeFunc.pLibHandle,
				 "CviAud_Algo_Init");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			   __func__, __LINE__, "find symbol error!\n");
		return CVI_ERR_AI_VQE_ERR;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &
				 (_stVqeFunc.pCviAud_Algo_Process), _stVqeFunc.pLibHandle,
				 "CviAud_Algo_Process");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			   __func__, __LINE__, "find symbol error!\n");
		return CVI_ERR_AI_VQE_ERR;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &
				 (_stVqeFunc.pCviAud_Algo_DeInit), _stVqeFunc.pLibHandle,
				 "CviAud_Algo_DeInit");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			   __func__, __LINE__, "find symbol error!\n");
		return CVI_ERR_AI_VQE_ERR;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &
				 (_stVqeFunc.pCviAud_Algo_GetVersion), _stVqeFunc.pLibHandle,
				 "CviAud_Algo_GetVersion");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			   __func__, __LINE__, "find symbol error!\n");
		return CVI_ERR_AI_VQE_ERR;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &
				 (_stVqeFunc.pCviAud_SpkAlgo_Init), _stVqeFunc.pLibHandle,
				 "CviAud_SpkAlgo_Init");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			   __func__, __LINE__, "find symbol error!\n");
		return CVI_ERR_AI_VQE_ERR;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &
				 (_stVqeFunc.pCviAud_SpkAlgo_Process), _stVqeFunc.pLibHandle,
				 "CviAud_SpkAlgo_Process");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			   __func__, __LINE__, "find symbol error!\n");
		return CVI_ERR_AI_VQE_ERR;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &
				 (_stVqeFunc.pCviAud_SpkAlgo_DeInit), _stVqeFunc.pLibHandle,
				 "CviAud_SpkAlgo_DeInit");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			   __func__, __LINE__, "find symbol error!\n");
		return CVI_ERR_AI_VQE_ERR;
	}
#endif

	memcpy(&gstVqeFunc, &_stVqeFunc, sizeof(ST_CVIAUDIO_ALGO_FUNC));

	return s32Ret;
}

CVI_S32 CVI_AudIn_AlgoInit(const AI_TALKVQE_CONFIG_S *pstVqeConfig)
{
#if DEBUG_VQE2
	ptr = fopen("DEBUG_VQE2save.raw", "wb");
	ptr_org = fopen("DEBUG_VQE2saveorg.raw", "wb");
#endif
	CVI_S32 s32Ret;
	CVI_VOID *_handle = CVI_NULL;
	CVI_CHAR _version[256] = {0};

	cviAudioGetDbgMask(&cviaud_dbg);
	DBG_PRINTF("xxxxxxxxxxxVQE version[%s]xxxxxxxxx\n", _VQE_VERSION_TAG_);

	CHECK_NULL_PTR(gstVqeFunc.pCviAud_Algo_Init);
	CHECK_NULL_PTR(gstVqeFunc.pCviAud_Algo_GetVersion);
	pthread_mutex_lock(&glock);
	if (bVqeInited) {
		printf("vqe has been already inited.\n");
		pthread_mutex_unlock(&glock);
		return CVI_SUCCESS;
	}
	bVqeInited = CVI_TRUE;
	gstVqeFunc.pCviAud_Algo_GetVersion(_version);
	printf("xxxxx[%s]\n", _version);
	//printf("[%s][%d] open mask[0x%x]\n", __func__, __LINE__, pstVqeConfig->u32OpenMask);
	_handle = gstVqeFunc.pCviAud_Algo_Init(pstVqeConfig->u32OpenMask, (CVI_VOID *)pstVqeConfig);

	if (_handle == CVI_NULL) {
		ERR_PRINTF("return NULL handle\n");
		s32Ret = CVI_ERR_AIO_NULL_PTR;
		bVqeInited = CVI_FALSE;
		pthread_mutex_unlock(&glock);
		return s32Ret;

	} else {
		CVI_S32 s32Size = DEFAULT_BYTES_PER_SAMPLE * AEC_FRAMES_LEN * DEFAULT_NUM_OF_VQE_CHANNEL;

		gstVqeAinInternal.gVqeAinHandle = _handle;
		gstVqeAinInternal.mic_in = (short *)malloc(s32Size);
		gstVqeAinInternal.ref_in = (short *)malloc(s32Size);
		//printf("gVqeAinHandle[0x%x]\n", gVqeAinHandle);
		s32Ret = CVI_SUCCESS;
	}

	if (poutput_aec == NULL)
		poutput_aec = VQE_CALLOC(short, AEC_FRAMES_LEN * DEFAULT_NUM_OF_VQE_CHANNEL * DEFAULT_BYTES_PER_SAMPLE);

	if (poutput_aec == NULL)
		ERR_PRINTF("Not enough mem for AEC buffer\n");

	if (poutput_anr == NULL)
		poutput_anr = VQE_CALLOC(short, AEC_FRAMES_LEN * DEFAULT_NUM_OF_VQE_CHANNEL * DEFAULT_BYTES_PER_SAMPLE);

	if (poutput_anr == NULL)
		ERR_PRINTF("Not enough mem for ANR buffer\n");
	pthread_mutex_unlock(&glock);
	return s32Ret;
}


CVI_S32 CVI_AudIn_AlgoProcess_AEC(CVI_CHAR *datain,
	CVI_CHAR *dataout,
	CVI_S32 s32SizeInBytes,
	CVI_S32 *s32SizeOutBytes)
{
#ifndef AUD_SUPPORT_KERNEL_MODE
	CHECK_NULL_PTR(gstVqeAinInternal.gVqeAinHandle);
	CHECK_NULL_PTR(gstVqeAinInternal.mic_in);
	CHECK_NULL_PTR(gstVqeAinInternal.ref_in);
	CHECK_NULL_PTR(gstVqeFunc.pCviAud_Algo_Process);
#endif
	pthread_mutex_lock(&glock);
//printf("[v][%s][%d]\n", __func__, __LINE__);
	if (!bVqeInited) {
		printf("vqe has not been already inited.\n");
		pthread_mutex_unlock(&glock);
		return CVI_FAILURE_ILLEGAL_PARAM;
	}
#ifdef AUD_SUPPORT_KERNEL_MODE
	if (gVqePath) {
		CVI_S32 s32Ret = CVI_FAILURE;
		AUDIO_FRAME_S stFrame;
//printf("[v][%s][%d]\n", __func__, __LINE__);
		if (gVqePath == E_VQE_KERNEL_BLOCK_MODE) {
			s32Ret = CVI_AI_GetFrameExtSsp_BlkMode(datain,
							dataout,
							s32SizeInBytes,
							s32SizeOutBytes);
//printf("[v][%s][%d]\n", __func__, __LINE__);
			pthread_mutex_unlock(&glock);
			return s32Ret;

		} else if (gVqePath == E_VQE_KERNEL_CO_BUFF_MODE) {
			CVI_S32 s32InputSamplesAll = s32SizeInBytes / DEFAULT_BYTES_PER_SAMPLE;
			CVI_S32 s32RemainSample = s32InputSamplesAll / DEFAULT_NUM_OF_VQE_CHANNEL;

			while (s32RemainSample >= AEC_FRAMES_LEN) {
				memset(&stFrame, 0, sizeof(AUDIO_FRAME_S));
				s32Ret = CVI_AI_GetFrameExtSsp(&stFrame);
				VQE_RET_FAILURE(s32Ret);
				s32RemainSample -= (AEC_FRAMES_LEN);
				memcpy(dataout, stFrame.u64VirAddr[0],  sizeof(CVI_S16) * AEC_FRAMES_LEN);
				if (s32RemainSample)
					dataout += (sizeof(CVI_S16) * AEC_FRAMES_LEN);
			}
			pthread_mutex_unlock(&glock);
			return s32Ret;

		} else {
			ERR_PRINTF("invalid mode in vqe[%d]\n", gVqePath);
			pthread_mutex_unlock(&glock);
			return CVI_FAILURE_ILLEGAL_PARAM;
		}
	}

#endif

	CVI_S32 s32InputSamplesAll = s32SizeInBytes / DEFAULT_BYTES_PER_SAMPLE;
	CVI_S32 s32RemainSizeSample = s32InputSamplesAll / DEFAULT_NUM_OF_VQE_CHANNEL; //toal samples in  mic_in
	CVI_S32 s32RetSamples = 0;
	//short output[AEC_FRAMES_LEN * DEFAULT_NUM_OF_VQE_CHANNEL * DEFAULT_BYTES_PER_SAMPLE] = {0};
	int i;
	CVI_S16 *ps16DataIn = (CVI_S16 *)datain;

	*s32SizeOutBytes = 0;

	CHECK_NULL_PTR(poutput_aec);

	while (s32RemainSizeSample >= AEC_FRAMES_LEN) {

		//printf("[%s][%d] handle add---REPLACING[%d]\n", __func__, __LINE__, s32RemainSizeSample);
		ps16DataIn = (CVI_S16 *)datain;
		for (i = 0; i < AEC_FRAMES_LEN; i++) {
			gstVqeAinInternal.mic_in[i] = ps16DataIn[i * 2];
			gstVqeAinInternal.ref_in[i] = ps16DataIn[i * 2 + 1];
		}
		s32RetSamples = gstVqeFunc.pCviAud_Algo_Process(gstVqeAinInternal.gVqeAinHandle,
								(CVI_S16 *)gstVqeAinInternal.mic_in,
								(CVI_S16 *)gstVqeAinInternal.ref_in,
								(CVI_S16 *)poutput_aec,
								AEC_FRAMES_LEN);

		s32RemainSizeSample -= (AEC_FRAMES_LEN);
		#if 0
		printf("data in step size bytes[%d] ret smp[%d] remain[%d]\n",
			AEC_FRAMES_LEN * DEFAULT_BYTES_PER_SAMPLE * DEFAULT_NUM_OF_VQE_CHANNEL,
			s32RetSamples,
			s32RemainSizeSample);
		#endif
		memcpy(dataout, poutput_aec,  sizeof(CVI_S16) * s32RetSamples);
		*s32SizeOutBytes += (sizeof(CVI_S16) * s32RetSamples);

		if (s32RemainSizeSample) {
			//if still reamin samples in buffer, then moving buffer pt
			//2 channel audio input in datain
			datain += AEC_FRAMES_LEN * DEFAULT_BYTES_PER_SAMPLE * DEFAULT_NUM_OF_VQE_CHANNEL;
			dataout += (sizeof(CVI_S16) * s32RetSamples);

		}
	}

	if (s32RemainSizeSample != 0) {
		ERR_PRINTF("remaining data ...not process AEC\n");
		if (s32RemainSizeSample != 0) {
			/* Not return */
			ERR_PRINTF("Please set frame sample 160x..remain[%d]smpls\n", s32RemainSizeSample);
		}
	}

	pthread_mutex_unlock(&glock);
	return CVI_SUCCESS;
}

CVI_S32 CVI_AudIn_AlgoProcess_AnrAgc(CVI_CHAR *datain,
				CVI_CHAR *dataout,
				CVI_S32 s32SizeInBytes,
				CVI_S32 *s32SizeOutBytes)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32SizeInSamples = s32SizeInBytes / DEFAULT_BYTES_PER_SAMPLE;
	CVI_S32 s32RetSamples = 0;
	CVI_S32 s32RemainSizeSample = s32SizeInSamples;
	//short output[AEC_FRAMES_LEN * DEFAULT_NUM_OF_VQE_CHANNEL * DEFAULT_BYTES_PER_SAMPLE] = {0};
	//char input[INPUT_VQE_TMP_BUFFER_BYTES] = {0};
	pthread_mutex_lock(&glock);
	if (!bVqeInited) {
		printf("vqe has not been already inited.\n");
		pthread_mutex_unlock(&glock);
		return CVI_FAILURE_ILLEGAL_PARAM;
	}
#ifdef AUD_SUPPORT_KERNEL_MODE
	if (gVqePath) {
		CVI_S32 s32Ret = CVI_FAILURE;
		AUDIO_FRAME_S stFrame;

		if (gVqePath == E_VQE_KERNEL_BLOCK_MODE) {
			s32Ret = CVI_AI_GetFrameExtSsp_BlkMode(datain,
							dataout,
							s32SizeInBytes,
							s32SizeOutBytes);
			pthread_mutex_unlock(&glock);
			return s32Ret;

		} else if (gVqePath == E_VQE_KERNEL_CO_BUFF_MODE) {
			CVI_S32 s32InputSamplesAll = s32SizeInBytes / DEFAULT_BYTES_PER_SAMPLE;
			CVI_S32 s32RemainSample = s32InputSamplesAll / DEFAULT_NUM_OF_VQE_CHANNEL;

			while (s32RemainSample >= AEC_FRAMES_LEN) {
				memset(&stFrame, 0, sizeof(AUDIO_FRAME_S));
				s32Ret = CVI_AI_GetFrameExtSsp(&stFrame);
				VQE_RET_FAILURE(s32Ret);
				s32RemainSample -= (AEC_FRAMES_LEN);
				memcpy(dataout, stFrame.u64VirAddr[0],  sizeof(CVI_S16) * AEC_FRAMES_LEN);
				if (s32RemainSample)
					dataout += (sizeof(CVI_S16) * AEC_FRAMES_LEN);
			}
			pthread_mutex_unlock(&glock);
			return s32Ret;

		} else {
			ERR_PRINTF("invalid mode in vqe[%d]\n", gVqePath);
			pthread_mutex_unlock(&glock);
			return CVI_FAILURE_ILLEGAL_PARAM;
		}
	}

#endif
	if (pinput_anr == NULL) {
		pinput_anr =  VQE_CALLOC(char, INPUT_VQE_TMP_BUFFER_BYTES);
		if (pinput_anr == NULL) {
			ERR_PRINTF("Not Enough Heap size\n");
			pthread_mutex_unlock(&glock);
			return CVI_ERR_AI_VQE_BUF_FULL;
		}
	}

	CVI_CHAR *ps8DataIn = (CVI_CHAR *)pinput_anr;
	CVI_CHAR *ps8DataOut = (CVI_CHAR *)dataout;

	if (s32SizeInBytes > INPUT_VQE_TMP_BUFFER_BYTES) {
		ERR_PRINTF("input size too large[%d]\n", s32SizeInBytes);
		pthread_mutex_unlock(&glock);
		return CVI_ERR_AI_VQE_BUF_FULL;

	} else {
		memcpy(pinput_anr, datain, s32SizeInBytes);
		ps8DataIn = pinput_anr;
	}
#if DEBUG_VQE2
	int count  = 0;
#endif
	CHECK_NULL_PTR(datain);
	CHECK_NULL_PTR(dataout);
	CHECK_NULL_PTR(gstVqeAinInternal.gVqeAinHandle);
	CHECK_NULL_PTR(gstVqeFunc.pCviAud_Algo_Process);
	CHECK_NULL_PTR(poutput_anr);
	*s32SizeOutBytes = 0;
#if DEBUG_VQE2
		printf("[v]ptr_orgxxxxx size[%d]\n", s32SizeInBytes);
		fwrite(ps8DataIn, 1, s32SizeInBytes, ptr_org);
#endif
	while (s32RemainSizeSample >= AEC_FRAMES_LEN) {
#if DEBUG_VQE2
		count += AEC_FRAMES_LEN*2;
		printf("count [%d][%d][0x%p]\n", count, AEC_FRAMES_LEN*2, ps8DataIn);
		fwrite(ps8DataIn, 1, AEC_FRAMES_LEN*2, ptr);
#endif
		s32RetSamples = gstVqeFunc.pCviAud_Algo_Process(
					gstVqeAinInternal.gVqeAinHandle,
					(CVI_S16 *)ps8DataIn,
					NULL,
					(CVI_S16 *)poutput_anr,
					AEC_FRAMES_LEN);
		if (s32RetSamples <= 0) {
			ERR_PRINTF("abnormal\n");
			s32Ret = CVI_ERR_AI_VQE_ERR;
		}

		s32RemainSizeSample -= (AEC_FRAMES_LEN);
		memcpy((char  *)ps8DataOut, (char *)poutput_anr, sizeof(CVI_S16) * s32RetSamples);
		*s32SizeOutBytes += (sizeof(CVI_S16) * s32RetSamples);

		if (s32RemainSizeSample) {
			datain += AEC_FRAMES_LEN * DEFAULT_BYTES_PER_SAMPLE;
			memcpy(ps8DataIn, datain, (AEC_FRAMES_LEN * DEFAULT_BYTES_PER_SAMPLE));
			ps8DataOut += (sizeof(CVI_S16) * s32RetSamples);
		}
	}
	if (s32RemainSizeSample != 0) {
		ERR_PRINTF("remaining data ...not process AEC\n");
		if (s32RemainSizeSample != 0) {
			/* Not return */
			ERR_PRINTF("Please set frame sample 160x..remain[%d]smpls\n", s32RemainSizeSample);
		}
	}
	pthread_mutex_unlock(&glock);

	return s32Ret;
}

CVI_S32 CVI_AudIn_AlgoDeInit(void)
{
	pthread_mutex_lock(&glock);
	if (!bVqeInited) {
		printf("vqe has not been already inited yet.\n");
		pthread_mutex_unlock(&glock);
		return CVI_FALSE;
	}
	bVqeInited = CVI_FALSE;
	SAFE_FREE_BUF(poutput_aec);
	SAFE_FREE_BUF(poutput_anr);
	SAFE_FREE_BUF(pinput_anr);
#ifdef AUD_SUPPORT_KERNEL_MODE
	if (gVqePath) {
		CVI_S32 s32Ret = CVI_FAILURE;

		if (gVqePath == E_VQE_KERNEL_BLOCK_MODE) {
			s32Ret = CVI_AI_DisableExtSsp_BlkMode();
			pthread_mutex_unlock(&glock);
			return s32Ret;

		} else if (gVqePath == E_VQE_KERNEL_CO_BUFF_MODE) {
			s32Ret = CVI_AI_DisableExtSsp();
			pthread_mutex_unlock(&glock);
			return s32Ret;

		} else {
			ERR_PRINTF("invalid mode in vqe[%d]\n", gVqePath);
			pthread_mutex_unlock(&glock);
			return CVI_FAILURE_ILLEGAL_PARAM;
		}
	}
#endif
	CHECK_NULL_PTR(gstVqeFunc.pCviAud_Algo_DeInit);
	gstVqeFunc.pCviAud_Algo_DeInit(gstVqeAinInternal.gVqeAinHandle);
	gstVqeAinInternal.gVqeAinHandle = NULL;
	SAFE_FREE_BUF(gstVqeAinInternal.mic_in);
	SAFE_FREE_BUF(gstVqeAinInternal.ref_in);

	pthread_mutex_unlock(&glock);
	DBG_PRINTF("deinit success\n");

	return CVI_SUCCESS;
}


CVI_S32 CVI_AUDIO_RegisterVQEModule(const AUDIO_VQE_REGISTER_S
					*pstVqeRegister)
{
	if (pstVqeRegister == NULL)
		return CVI_FAILURE;

	memcpy(&gstVqeReg, pstVqeRegister, sizeof(AUDIO_VQE_REGISTER_S));

	return CVI_SUCCESS;

}

CVI_S32 CVI_AI_SetRecordVqeAttr(AUDIO_DEV AiDevId, AI_CHN AiChn,
			const AI_RECORDVQE_CONFIG_S *pstVqeConfig)
{
	VQE_UNUSED_REF(AiDevId);
	VQE_UNUSED_REF(AiChn);
	VQE_UNUSED_REF(pstVqeConfig);
	ERR_PRINTF("Plz use CVI_AI_SetTalkVqeAttr instead\n");
	return CVI_FAILURE;
}

CVI_S32 CVI_AI_GetRecordVqeAttr(AUDIO_DEV AiDevId, AI_CHN AiChn,
				AI_RECORDVQE_CONFIG_S *pstVqeConfig)
{
	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		ERR_PRINTF("\n");
		return (CVI_S32)CVI_ERR_AI_INVALID_DEVID;
	}
	VQE_UNUSED_REF(AiDevId);
	VQE_UNUSED_REF(AiChn);
	VQE_UNUSED_REF(pstVqeConfig);

	ERR_PRINTF("Plz use CVI_AI_GetTalkVqeAttr instead\n");
	return CVI_FAILURE;
}

static CVI_S32 _select_init_ssp(const AI_TALKVQE_CONFIG_S *pstVqeConfig)
{
	VQE_UNUSED_REF(pstVqeConfig);
#ifndef AUD_SUPPORT_KERNEL_MODE
	ERR_PRINTF("Not support in this chip\n");
	return 0;
#else
	CVI_S32 s32Ret = CVI_FAILURE;

	if (gVqePath == E_VQE_KERNEL_BLOCK_MODE) {
		s32Ret = CVI_AI_EnableExtSSp_BlkMode(pstVqeConfig->s32WorkSampleRate,
						AEC_FRAMES_LEN,
						pstVqeConfig);
		bVqeInited = CVI_TRUE;
		VQE_RET_FAILURE(s32Ret);
	} else if (gVqePath == E_VQE_KERNEL_CO_BUFF_MODE) {
		s32Ret = CVI_AI_EnableExtSSp(pstVqeConfig->s32WorkSampleRate,
						AEC_FRAMES_LEN,
						pstVqeConfig);
		bVqeInited = CVI_TRUE;
		VQE_RET_FAILURE(s32Ret);
	} else {
		ERR_PRINTF("invalid parameters[%d]\n", gVqePath);
		return CVI_FAILURE;
	}

	return s32Ret;
#endif
}
static CVI_S32 _select_init_path(CVI_S32 eVqePath)
{
	VQE_UNUSED_REF(eVqePath);
#ifndef AUD_SUPPORT_KERNEL_MODE
	return 0;
#else

	if (eVqePath < 0 || eVqePath >= (CVI_S32)E_VQE_PATH_MAX) {
		ERR_PRINTF("abnomal path selection in kernel mode vqe[%d]\n", eVqePath);
		return 0;

	} else
		return eVqePath;
#endif
}

static CVI_VOID _vqe_dump_config(AI_TALKVQE_CONFIG_S *pstTalkVqeCfg)
{
	//dump mask and customer config
	printf("u32OpenMask[0x%x]\n", pstTalkVqeCfg->u32OpenMask);
	printf("s32WorkSampleRate[%d]\n", pstTalkVqeCfg->s32WorkSampleRate);
	printf("para_client_config[%d]\n", pstTalkVqeCfg->para_client_config);
	//dump aec config
	printf("stAecCfg.para_aec_filter_len[%d]\n", pstTalkVqeCfg->stAecCfg.para_aec_filter_len);
	printf("stAecCfg.para_aes_std_thrd[%d]\n", pstTalkVqeCfg->stAecCfg.para_aes_std_thrd);
	printf("stAecCfg.para_aes_supp_coeff[%d]\n", pstTalkVqeCfg->stAecCfg.para_aes_supp_coeff);
	//dump anr config
	printf("stAnrCfg.para_nr_init_sile_time[%d]\n", pstTalkVqeCfg->stAnrCfg.para_nr_init_sile_time);
	printf("stAnrCfg.para_nr_snr_coeff[%d]\n", pstTalkVqeCfg->stAnrCfg.para_nr_snr_coeff);
	//dump agc config
	printf("stAgcCfg.para_agc_max_gain[%d]\n", pstTalkVqeCfg->stAgcCfg.para_agc_max_gain);
	printf("stAgcCfg.para_agc_target_high[%d]\n", pstTalkVqeCfg->stAgcCfg.para_agc_target_high);
	printf("stAgcCfg.para_agc_target_low[%d]\n", pstTalkVqeCfg->stAgcCfg.para_agc_target_low);
	printf("stAgcCfg.para_agc_vad_ena[%d]\n", pstTalkVqeCfg->stAgcCfg.para_agc_vad_ena);
	//dump swaec delsy config
	printf("stAecDelayCfg.para_aec_init_filter_len[%d]\n", pstTalkVqeCfg->stAecDelayCfg.para_aec_init_filter_len);
	printf("stAecDelayCfg.para_dg_target[%d]\n", pstTalkVqeCfg->stAecDelayCfg.para_dg_target);
	printf("stAecDelayCfg.para_delay_sample[%d]\n", pstTalkVqeCfg->stAecDelayCfg.para_delay_sample);
	//dump reversion mask for customize
	printf("s32RevMask[0x%x]\n", pstTalkVqeCfg->s32RevMask);
	printf("para_notch_freq[%d]\n", pstTalkVqeCfg->para_notch_freq);
	printf("customize[%s]\n", pstTalkVqeCfg->customize);
}

CVI_S32 CVI_AI_SetTalkVqeAttr(AUDIO_DEV AiDevId, AI_CHN AiChn,
			AUDIO_DEV AoDevId, AO_CHN AoChn,
			const AI_TALKVQE_CONFIG_S *pstVqeConfig)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ai_set_talk_vqe_attr(AiDevId, AiChn,
			AoDevId, AoChn,
			pstVqeConfig);
	}
#endif
	CVI_S32 s32Ret = CVI_SUCCESS;
	AI_TALKVQE_CONFIG_S stTalkVqeCfg;
	FILE *fp = CVI_NULL;
	FILE *fp_save = CVI_NULL;

	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		ERR_PRINTF("\n");
		return CVI_ERR_AI_INVALID_DEVID;
	}
	VQE_UNUSED_REF(AiChn);
	VQE_UNUSED_REF(AoDevId);
	VQE_UNUSED_REF(AoChn);

	memset(&stTalkVqeCfg, 0, sizeof(AI_TALKVQE_CONFIG_S));
	memcpy(&stTalkVqeCfg, pstVqeConfig, sizeof(AI_TALKVQE_CONFIG_S));
	if (_select_init_path(gVqePath)) {
		s32Ret = _select_init_ssp(pstVqeConfig);
		if (s32Ret != CVI_SUCCESS) {
			ERR_PRINTF("Failure in vqe kernel mode\n");
			VQE_RET_FAILURE(s32Ret);

		} else {
			memcpy(&gstVqeAinInternal.talkvqe,
				&stTalkVqeCfg,
				sizeof(AI_TALKVQE_CONFIG_S));
			fp_save = fopen(VQE_CURRENT_PARAMETERS, "w");
			CVI_AUD_VQE_SaveParamToCfg(fp_save, (CVI_VOID *)&gstVqeAinInternal.talkvqe);
			if (fp_save != CVI_NULL)
				fclose(fp_save);
			return CVI_SUCCESS;
		}
	}

	s32Ret = CVI_AUD_VQE_LoadFunc(CVITEK_SSP_NOTCH_LIB_NAME);
	if (s32Ret != CVI_SUCCESS) {
		ERR_PRINTF("Cannot find algorithm lib:[%s]\n", CVITEK_SSP_NOTCH_LIB_NAME);
		ERR_PRINTF("\n");
		return CVI_ERR_AI_VQE_ERR;
	}
#ifdef CVIAUDIO_STATIC

#else
	CHECK_NULL_PTR(gstVqeFunc.pLibHandle);//handle from dlopen
#endif
	if (pstVqeConfig->s32RevMask == CVIAUDIO_ALGO_ONLINE_PARAM) {
		//online mode, using parameter in /mnt/data/audvqe.cfg
		if (access(ONLINE_MODE_FILE_PATH, F_OK) == 0) {
			fp = fopen(ONLINE_MODE_FILE_PATH, "r");
			if (fp != NULL) {
				CVI_AUD_VQE_LoadParamFromCfg(fp, (CVI_U8 *)&stTalkVqeCfg);
				printf("Load from VQE Online mode-----------------[begin]\n");
				_vqe_dump_config(&stTalkVqeCfg);
				printf("Load from VQE Online mode-----------------[End]\n");
				memcpy(&gstVqeAinInternal.talkvqe, &stTalkVqeCfg, sizeof(AI_TALKVQE_CONFIG_S));

			} else
				VQE_RET_FAILURE(CVI_FAILURE);
		} else {
			ERR_PRINTF("Online mode load vqe parameters failure\n");
			ERR_PRINTF("Cannot find online mode cfg[%s]\n", ONLINE_MODE_FILE_PATH);
			VQE_RET_FAILURE(CVI_ERR_AI_VQE_FILE_UNEXIST);
		}
	} else {
		if (strcmp(pstVqeConfig->customize, "ZKT") == 0) {
			printf("cviaudio VQE using ZKT parameters\n");
			stTalkVqeCfg.u32OpenMask =  ZKT_VQE_CONFIG.u32OpenMask;
			stTalkVqeCfg.para_notch_freq = ZKT_VQE_CONFIG.para_notch_freq;
			memcpy(&stTalkVqeCfg.stAecCfg, &ZKT_VQE_CONFIG.stAecCfg, sizeof(AI_AEC_CONFIG_S));
			memcpy(&stTalkVqeCfg.stAnrCfg, &ZKT_VQE_CONFIG.stAnrCfg, sizeof(AUDIO_ANR_CONFIG_S));
			memcpy(&stTalkVqeCfg.stAgcCfg, &ZKT_VQE_CONFIG.stAgcCfg, sizeof(AUDIO_AGC_CONFIG_S));
			printf("dump after using ZKT default config---------[begin]\n");
			_vqe_dump_config(&stTalkVqeCfg);
			printf("dump after using ZKT default config---------[end]\n");
			memcpy(&gstVqeAinInternal.talkvqe, &stTalkVqeCfg, sizeof(AI_TALKVQE_CONFIG_S));
		} else if (strcmp(pstVqeConfig->customize, "CEOP") == 0) {
			printf("cviaudio VQE using CEOP parameters\n");
			stTalkVqeCfg.u32OpenMask =  CEOP_VQE_CONFIG.u32OpenMask;
			stTalkVqeCfg.para_notch_freq = CEOP_VQE_CONFIG.para_notch_freq;
			memcpy(&stTalkVqeCfg.stAecCfg, &CEOP_VQE_CONFIG.stAecCfg, sizeof(AI_AEC_CONFIG_S));
			memcpy(&stTalkVqeCfg.stAnrCfg, &CEOP_VQE_CONFIG.stAnrCfg, sizeof(AUDIO_ANR_CONFIG_S));
			memcpy(&stTalkVqeCfg.stAgcCfg, &CEOP_VQE_CONFIG.stAgcCfg, sizeof(AUDIO_AGC_CONFIG_S));
			printf("dump after using CEOP default config---------[begin]\n");
			_vqe_dump_config(&stTalkVqeCfg);
			printf("dump after using CEOP default config---------[end]\n");
			memcpy(&gstVqeAinInternal.talkvqe, &stTalkVqeCfg, sizeof(AI_TALKVQE_CONFIG_S));
		} else if (strcmp(pstVqeConfig->customize, "HT") == 0) {
			printf("cviaudio VQE using HT parameters\n");
			stTalkVqeCfg.u32OpenMask =  HT_VQE_CONFIG.u32OpenMask;
			stTalkVqeCfg.para_notch_freq = HT_VQE_CONFIG.para_notch_freq;
			memcpy(&stTalkVqeCfg.stAecCfg, &HT_VQE_CONFIG.stAecCfg, sizeof(AI_AEC_CONFIG_S));
			memcpy(&stTalkVqeCfg.stAnrCfg, &HT_VQE_CONFIG.stAnrCfg, sizeof(AUDIO_ANR_CONFIG_S));
			memcpy(&stTalkVqeCfg.stAgcCfg, &HT_VQE_CONFIG.stAgcCfg, sizeof(AUDIO_AGC_CONFIG_S));
			printf("dump after using HT default config---------[begin]\n");
			_vqe_dump_config(&stTalkVqeCfg);
			printf("dump after using HT default config---------[end]\n");
			memcpy(&gstVqeAinInternal.talkvqe, &stTalkVqeCfg, sizeof(AI_TALKVQE_CONFIG_S));
		} else {
			printf("cviaudio VQE using user set parameters\n");
			memcpy(&gstVqeAinInternal.talkvqe, pstVqeConfig, sizeof(AI_TALKVQE_CONFIG_S));
		}
		//save current cfg to /tmp/audvqe.cfg
		fp_save = fopen(VQE_CURRENT_PARAMETERS, "w");
		CVI_AUD_VQE_SaveParamToCfg(fp_save, (CVI_VOID *)&gstVqeAinInternal.talkvqe);
	}

	s32Ret = CVI_AudIn_AlgoInit(&gstVqeAinInternal.talkvqe);
	VQE_RET_FAILURE(s32Ret);

	if (fp != CVI_NULL)
		fclose(fp);
	if (fp_save != CVI_NULL)
		fclose(fp_save);

	return CVI_SUCCESS;
}


CVI_S32 CVI_AI_GetTalkVqeAttr(AUDIO_DEV AiDevId, AI_CHN AiChn,
				AI_TALKVQE_CONFIG_S *pstVqeConfig)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ai_get_talk_vqe_attr(AiDevId, AiChn,
							pstVqeConfig);
	}
#endif
	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		ERR_PRINTF("\n");
		return CVI_ERR_AI_INVALID_DEVID;
	}
	VQE_UNUSED_REF(AiChn);
	memcpy(pstVqeConfig,
			&gstVqeAinInternal.talkvqe,
			sizeof(AI_TALKVQE_CONFIG_S));
	return CVI_SUCCESS;
}

CVI_S32 CVI_VQE_EnableSwAEC(CVI_S32 s32SampleRate, CVI_S32 s32OutChannel, CVI_S32 period_size)
{
	cviAudioGetDbgMask(&cviaud_dbg);
	if (s32SampleRate != 8000 && s32SampleRate != 16000) {
		ERR_PRINTF("Not available sample rate[%d]...only support 8k/16k\n", s32SampleRate);
		return	CVI_FAILURE;
	}

	if (s32OutChannel != 1 && s32OutChannel != 2) {
		ERR_PRINTF("Not support out channels[%d]\n", s32OutChannel);
		return	CVI_FAILURE;
	}

	if (!pSwAecHandle) {
		//Condition 1:
		pSwAecHandle = VQE_CALLOC(ST_CVIAUDIO_SW_AEC, 1);
		pSwAecHandle->s32Channels = s32OutChannel;
		pSwAecHandle->s32SampleRate = s32SampleRate;
		pSwAecHandle->s32PeriodSamples = period_size;
		pSwAecHandle->pDemuxBuffer =  VQE_CALLOC(char, period_size * DEFAULT_BYTES_PER_SAMPLE * 2);
		pSwAecHandle->pTmpBuffer =  VQE_CALLOC(char, period_size * DEFAULT_BYTES_PER_SAMPLE * 2);
		//500ms buffering
		// = (s32SampleRate / 2 ) * bytes_per_sample
		pSwAecHandle->s32FarEndBufferLength = ((s32SampleRate / 2) * DEFAULT_BYTES_PER_SAMPLE);
		FarEndBufferInit(&pSwAecHandle->FarendHandle, pSwAecHandle->s32FarEndBufferLength);
		pSwAecHandle->bEnableSwAec = CVI_TRUE;
		pSwAecHandle->bFirstReadRdy = CVI_FALSE;
	} else {
		//check the parameter is the same
		if (pSwAecHandle->s32Channels == s32OutChannel &&
			pSwAecHandle->s32SampleRate == s32SampleRate &&
			pSwAecHandle->s32PeriodSamples == period_size) {
			//Condition 2
			//same ...trigger enable
			if (pSwAecHandle->bEnableSwAec == CVI_FALSE) {
				printf("[cviaudio]Sw AEC enable...!!\n");
				pSwAecHandle->bEnableSwAec = CVI_TRUE;
			} else {
				printf("[cviaudio]Sw AEC already enable!!\n");
			}
		} else {
			//Condition 3
			//not same parameters while  handle exist ..disable and retrigger handle
			pSwAecHandle->bEnableSwAec = CVI_FALSE;
			//STEP 1:clear the handle and buffer
			SAFE_FREE_BUF(pSwAecHandle->pDemuxBuffer);
			SAFE_FREE_BUF(pSwAecHandle->pTmpBuffer);
			FarEndBufferDestory(pSwAecHandle->FarendHandle);
			pSwAecHandle->FarendHandle = CVI_NULL;
			//STEP 2: retrigger sw aec handle
			pSwAecHandle->s32Channels = s32OutChannel;
			pSwAecHandle->s32SampleRate = s32SampleRate;
			pSwAecHandle->s32PeriodSamples = period_size;
			pSwAecHandle->pDemuxBuffer =  VQE_CALLOC(char, 2 * period_size * DEFAULT_BYTES_PER_SAMPLE);
			pSwAecHandle->pTmpBuffer =  VQE_CALLOC(char, 2 * period_size * DEFAULT_BYTES_PER_SAMPLE);
			//500ms buffering
			// = (s32SampleRate / 2 ) * bytes_per_sample
			pSwAecHandle->s32FarEndBufferLength = ((s32SampleRate / 2) * DEFAULT_BYTES_PER_SAMPLE);
			FarEndBufferInit(&pSwAecHandle->FarendHandle, pSwAecHandle->s32FarEndBufferLength);
			pSwAecHandle->bEnableSwAec = CVI_TRUE;
		}
		pSwAecHandle->bFirstReadRdy = CVI_FALSE;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VQE_DisableSwAEC(void)
{
	cviAudioGetDbgMask(&cviaud_dbg);
	CHECK_NULL_PTR(pSwAecHandle);
	//check enable status ...is disable already ..print warning msg
	pSwAecHandle->bEnableSwAec = CVI_FALSE;
	pSwAecHandle->bFirstReadRdy = CVI_FALSE;

	return	CVI_SUCCESS;
}

CVI_S32 CVI_VQE_CheckSwAECEnable(void)
{
	if (!pSwAecHandle) {
		ERR_PRINTF("pSwAecHandle is Null\n");
		return	CVI_FALSE;
	}
	printf("[%s] ret[%d]\n", __func__, (int)pSwAecHandle->bEnableSwAec);
	return pSwAecHandle->bEnableSwAec;
}


static int _vqe_demux2chn(short *tmp,
			short *input_buff,
			CVI_U32 inputbytes)
{
	CVI_U32 i;
	int total_chn = 2;
	int channel_index = 1; //extract the right channel

	if (!tmp) {
		ERR_PRINTF("\n");
		return CVI_FAILURE;
	}
	//if (total_chn == 2 && dst_chn == 1) {
	for (i = 0;  i < (inputbytes / total_chn); i++) {
		tmp[i] = input_buff[total_chn * i + channel_index];
	}
	inputbytes = inputbytes / total_chn;

	//memcpy(input_buff, tmp, inputbytes);
	return inputbytes;

}

CVI_S32 CVI_VQE_SwAECWrite(char *pbuffer, unsigned int size_bytes)
{
	CVI_S32 s32BufDataLength, s32AvailLength, s32WriteInSize, s32Ret;

	CHECK_NULL_PTR(pSwAecHandle);
	if (!pSwAecHandle->bEnableSwAec) {
		ERR_PRINTF("SW AEC not enable\n");
		return CVI_FAILURE;
	}

	if (!pSwAecHandle->FarendHandle) {
		ERR_PRINTF("SW AEC far end buffer not create ...!!");
		return CVI_FAILURE;
	}

	//step 1: handle the two channel play out condition
	if (pSwAecHandle->s32Channels == 1)
		s32WriteInSize = size_bytes;
	else {

		//do the demux
		_vqe_demux2chn((short *)(pSwAecHandle->pDemuxBuffer), (short *)pbuffer, size_bytes);
		s32WriteInSize = size_bytes / 2;
	}


	//step 2: check the available farend buffer size and write
	s32BufDataLength = FarEndBufferDataLen(pSwAecHandle->FarendHandle);
	s32AvailLength = pSwAecHandle->s32FarEndBufferLength - s32BufDataLength;
	if (s32AvailLength < s32WriteInSize) {
		ERR_PRINTF("buffering full ...flush and write again[%d] < [%d]\n",
				s32AvailLength, s32WriteInSize);
		FarEndBufferRead(pSwAecHandle->FarendHandle,
				pSwAecHandle->pTmpBuffer,
				s32WriteInSize);
	}
	if (pSwAecHandle->s32Channels == 1) {
		s32Ret =  FarEndBufferWrite(pSwAecHandle->FarendHandle,
				(char *)pbuffer,
				s32WriteInSize);
	} else {
		s32Ret =  FarEndBufferWrite(pSwAecHandle->FarendHandle,
				(char *)pSwAecHandle->pDemuxBuffer,
				s32WriteInSize);
	}

	if (s32Ret != s32WriteInSize)
		ERR_PRINTF("Cannot write in farend buffer\n");

	return CVI_SUCCESS;
}

CVI_S32 CVI_VQE_SwAECRead(char *pbuffer, unsigned int size_bytes)
{
	CVI_S32 s32BufDataLength = 0;

	CHECK_NULL_PTR(pSwAecHandle);
	if (!pSwAecHandle->bEnableSwAec) {
		ERR_PRINTF("SW AEC not enable\n");
		return CVI_FAILURE;
	}

	if (!pSwAecHandle->FarendHandle) {
		ERR_PRINTF("SW AEC far end buffer not create ...!!");
		return CVI_FAILURE;
	}

	s32BufDataLength = FarEndBufferDataLen(pSwAecHandle->FarendHandle);
	if ((pSwAecHandle->bFirstReadRdy == CVI_FALSE) &&
		((CVI_S32)(size_bytes) < pSwAecHandle->s32FarEndBufferLength)) {
		if (s32BufDataLength >  (int)(size_bytes * 2)) { //at least two frame inside
			//buffer level is enough for reading first frame
			printf("SW AEC bFirstReadRdy\n");
			pSwAecHandle->bFirstReadRdy = CVI_TRUE;

		} else {
			usleep(40*1000);//sleep 40ms
			return CVI_FAILURE;
		}

	}

	if ((unsigned int)s32BufDataLength < size_bytes) {
		ERR_PRINTF("Not enough data in far end buffer\n");
		usleep(100*1000);//sleep 100 ms
		return CVI_FAILURE;

	} else {
		FarEndBufferRead(pSwAecHandle->FarendHandle,
				(char *)pbuffer,
				size_bytes);
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VQE_CloseSwAEC(void)
{
	CHECK_NULL_PTR_WO_PRINT(pSwAecHandle);

	SAFE_FREE_BUF(pSwAecHandle->pDemuxBuffer);
	SAFE_FREE_BUF(pSwAecHandle->pTmpBuffer);
	FarEndBufferDestory(pSwAecHandle->FarendHandle);
	pSwAecHandle->FarendHandle = CVI_NULL;
	SAFE_FREE_BUF(pSwAecHandle);

	return	CVI_SUCCESS;
}

CVI_S32 CVI_AI_EnableVqe(AUDIO_DEV AiDevId, AI_CHN AiChn)
{

	cviAudioGetDbgMask(&cviaud_dbg);
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ai_enable_vqe(AiDevId, AiChn);
	}
#endif
	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		ERR_PRINTF("\n");
		return CVI_ERR_AI_INVALID_DEVID;
	}
	VQE_UNUSED_REF(AiChn);
	//bDevVqeStatus[AiDevId] = CVI_TRUE;
	bDevVqeStatus[AiDevId] |= (0x01 << AiChn);
	gstVqeAinInternal.stAinDevCfg[AiDevId].stAinChnCfg[AiChn].bVqeEnable = CVI_TRUE;

	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_DisableVqe(AUDIO_DEV AiDevId, AI_CHN AiChn)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ai_disable_vqe(AiDevId, AiChn);
	}
#endif
	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		ERR_PRINTF("\n");
		return CVI_ERR_AI_INVALID_DEVID;
	}
	VQE_UNUSED_REF(AiChn);
	//CHECK_NULL_PTR(gstVqeFunc.pCviAud_Algo_DeInit);

	//gstVqeFunc.pCviAud_Algo_DeInit(gstVqeAinInternal.gVqeAinHandle);
	//bDevVqeStatus[AiDevId] = CVI_FALSE;
	bDevVqeStatus[AiDevId] &= ~(0x01 << AiChn);
	gstVqeAinInternal.stAinDevCfg[AiDevId].stAinChnCfg[AiChn].bVqeEnable = CVI_FALSE;
	gstVqeAinInternal.talkvqe.u32OpenMask = 0x00;

	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_DevVQECheckEnable(AUDIO_DEV AiDevId)
{

	cviAudioGetDbgMask(&cviaud_dbg);
	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		ERR_PRINTF("\n");
		return CVI_ERR_AI_INVALID_DEVID;
	}

	CVI_BOOL bRet = CVI_FALSE;

	bRet = bDevVqeStatus[AiDevId];
	return bRet;
}

CVI_BOOL CVI_AI_VQECheckEnable(AUDIO_DEV AiDevId, AI_CHN AiChn)
{

	cviAudioGetDbgMask(&cviaud_dbg);
	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		ERR_PRINTF("AiDevId:%d\n", AiDevId);
		return CVI_FALSE;
	}
	VQE_UNUSED_REF(AiChn);

	CVI_BOOL bRet = CVI_FALSE;

	bRet = gstVqeAinInternal.stAinDevCfg[AiDevId].stAinChnCfg[AiChn].bVqeEnable;
	return bRet;
}

CVI_S32 CVI_AI_VQECheckFlag(AUDIO_DEV AoDevId, AI_CHN AiChn)
{
	if (CHECK_AI_DEVID_VALID(AoDevId)) {
		ERR_PRINTF("\n");
		return CVI_ERR_AI_INVALID_DEVID;
	}
	VQE_UNUSED_REF(AiChn);
	return gstVqeAinInternal.talkvqe.u32OpenMask;
}

CVI_S32 CVI_AI_EnableAecRefFrame(AUDIO_DEV AiDevId, AI_CHN AiChn,
				     AUDIO_DEV AoDevId, AO_CHN AoChn)
{
	VQE_UNUSED_REF(AiDevId);
	VQE_UNUSED_REF(AiChn);
	VQE_UNUSED_REF(AoDevId);
	VQE_UNUSED_REF(AoChn);

	gstVqeAinInternal.stAinDevCfg[AiDevId].stAinChnCfg[AiChn].bEnableRefAEC = CVI_TRUE;
	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_DisableAecRefFrame(AUDIO_DEV AiDevId, AI_CHN AiChn)
{
	VQE_UNUSED_REF(AiDevId);
	VQE_UNUSED_REF(AiChn);

	gstVqeAinInternal.stAinDevCfg[AiDevId].stAinChnCfg[AiChn].bEnableRefAEC = CVI_FALSE;
	return CVI_SUCCESS;
}

#ifdef AUD_SUPPORT_KERNEL_MODE
CVI_S32 CVI_VQE_PathSelect(E_VQE_ALGO_PATH eVqePath)
{
	if (eVqePath >= E_VQE_PATH_MAX || eVqePath < 0) {
		ERR_PRINTF("invalid parameters [%d]\n", (int)eVqePath);
		return CVI_FAILURE;
	}

	gVqePath = eVqePath;
	return CVI_SUCCESS;
}
#endif
