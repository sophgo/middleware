#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>// for close
#include <fcntl.h> // for open
#include <sys/ioctl.h>//for ioctl
#include "acodec.h"
#include "asoundlib.h"
#include "cyclebuffer.h"
#include "cvi_audio_uac.h"
#include "cvi_mp3_decode.h"
#include "cvi_aac_dec.h"
#include "cvi_resampler_api.h"
#include "cvi_audio_dl_adp.h"

#define RES_LIB_NAME "libcvi_RES1.so"
#define CVI_LITE_AUDIO_INTERFACE
#ifdef CVI_LITE_AUDIO_INTERFACE
#define __VERSION_TAG__   "CVI_AUDIO_UAC_20210809"
//---global struct------------------------------start
#ifdef THIS_IS_32
#define AUDIO_PERIOD_SIZE 480
#define CAP_PERIOD_COUNT 4
#define PLAY_PERIOD_COUNT 4
#else
#define AUDIO_PERIOD_SIZE 960
#define CAP_PERIOD_COUNT 10
#define PLAY_PERIOD_COUNT 4
#endif

#ifndef BYTES_PER_SAMPLE
#define BYTES_PER_SAMPLE 2
#endif

#ifndef DEFAULT_CHANNEL_COUNT
#define DEFAULT_CHANNEL_COUNT 2
#endif

#ifndef SAFE_FREE_BUF
#define SAFE_FREE_BUF(OBJ) {if (NULL != OBJ) {free(OBJ); OBJ = NULL; } }
#endif

//buffer setting
#define CVIAUDIO_UAC_FILE_MODE_FIFO_SIZE (1152*60)
#define CVIAUDIO_UAC_FIFO_SIZE (1024*12)
#define CVIAUDIO_UAC_FIFO_SIZE_RESAMPLE (1024 * 48)
#define CVIAUDIO_UAC_FIFO_THRESHOLD_RESAMPLE (1024 * 32)

//function pointer
typedef CVI_VOID * (*pCVI_Resampler_Create_Callback)(CVI_S32 s32Inrate,
		CVI_S32 s32Outrate, CVI_S32 s32Chans);

typedef CVI_S32(*pCVI_Resampler_Process_Callback)(CVI_VOID *inst,
		CVI_S16 *s16Inbuf, CVI_S32 s32Insamps, CVI_S16 *s16Outbuf);

typedef CVI_VOID(*pCVI_Resampler_Destroy_Callback)(CVI_VOID *inst);


typedef CVI_S32(*pCVI_Resampler_GetMaxOutputNum_Callback)(CVI_VOID *inst,
		CVI_S32 s32Insamps);

typedef struct SAMPLE_RES_FUN_S {
	CVI_VOID *pLibHandle;
	pCVI_Resampler_Create_Callback pCVI_Resampler_Create;
	pCVI_Resampler_Process_Callback pCVI_Resampler_Process;
	pCVI_Resampler_Destroy_Callback pCVI_Resampler_Destroy;
	pCVI_Resampler_GetMaxOutputNum_Callback pCVI_Resampler_GetMaxOutputNum;
} ST_SAMPLE_RES_FUN_S;


typedef struct _uac_threadEnable {
	bool bEnableSrc_MicIn;
	bool bEnableSrc_UsbIn;
	bool bEnableSrc_FileIn;
	bool bEnableSrc_StreamIn;
	bool bEnableDst_SpkOut;
	bool bEnableDst_UsbOut;
	bool bEnableDst_FileSave;

} ST_UAC_THREAD_ENABLE;

typedef struct _cviaudo_fifo_buffer {
	void *pcviaudio_fifo_handle;
	char *pcviaudio_fifo_buf;
	int s32fifo_size;
	bool bEnable;
} st_cviaudio_fifo_buffer;

typedef struct _ST_CVI_UAC_PCM_INFO_UPDATE {
	unsigned int channels;
	unsigned int rate;
	unsigned int period_size;
} ST_CVI_UAC_PCM_INFO_UPDATE;


typedef struct _cviuac_src_context {
	pthread_t *src_thread;
	E_CVIAUDIO_UAC_CODETYPE codec_type;
	ST_MP3_DEC_INFO stMp3DecInfo;
	void *pMp3DecHandler;
	ST_AAC_DEC_INFO stAacDecInfo;
	void *pAacDecHandler;
} cviuac_src_context;


typedef struct _cviuac_dst_context {
	pthread_t *dst_thread;
	ST_CVI_UAC_PCM_INFO_UPDATE update_pcm_info;
	ST_SAMPLE_RES_FUN_S stSampleResFun;
	void *cviRes;
	FILE *pfd_dst_save;
} cviuac_dst_context;

typedef struct _cviuac_module_param_s {
	//add the global parameters here
	int temp_value;
} cviuac_module_params;

typedef struct _cviuac_context {
	cviuac_src_context * cviuac_src_handle[CVIAUDIO_SRC_MAX_NUM];
	cviuac_dst_context * cviuac_dst_handle[CVIAUDIO_DST_MAX_NUM];
	cviuac_module_params ModParams;
	bool bValid;
} cviuac_context;
//---global struct------------------------------end

//define print level -------------------------------------[start]
#define CVI_UAC_MASK_ERR	(0x01)
#define CVI_UAC_MASK_INFO	(0x01)
#define CVI_UAC_MASK_WARN	(0x02)
#define CVI_UAC_MASK_DBG	(0x04)
#define CVI_UAC_MASK_TRACE	(0x08)
#define  USB_AS_SRC_STABLE_THRESHOLD 5

int  gUAC_level = 0x04;

#define UAC_UNUSED_REF(X)  ((X) = (X))//TODO:this case should not exist!

#define ERR_UAC_PRINT(msg, ...)	\
	do { \
		if (gUAC_level >= CVI_UAC_MASK_ERR) \
		printf("[UAC][ERR] %s = %d, "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define INFO_UAC_PRINT(msg, ...)	\
	do { \
		if (gUAC_level >= CVI_UAC_MASK_INFO) \
		printf("[UAC][INFO] %s = %d, "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define WARN_UAC_PRINT(msg, ...)	\
	do { \
		if (gUAC_level >= CVI_UAC_MASK_WARN) \
		printf("[UAC][WARN] %s = %d, "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define DBG_UAC_PRINT(msg, ...)	\
	do {  \
		if (gUAC_level >= CVI_UAC_MASK_DBG) \
		printf("[UAC][DBG] %s = %d, "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define TRACE_UAC_PRINT(msg, ...)	\
	do { \
		if (gUAC_level == CVI_UAC_MASK_TRACE) \
		printf("[UAC][DBG] %s = %d, "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

static int  cviUacGetEnv(char *env, char *fmt, void *param);
static void cvi_audio_uac_getDbgMask(void);
//define print level .................................................[end]

//----uac macro---------------------------------start
#ifndef UAC_RET_FAILURE
#define UAC_RET_FAILURE(ret)	\
	do {	\
		if (ret != CVI_SUCCESS) {	\
			ERR_UAC_PRINT("\n");	\
			return CVI_FAILURE;	\
		}	\
	} while (0)
#endif

#ifndef CHECK_NULL_PTR
#define CHECK_NULL_PTR(ptr)                                                                                            \
	do {                                                                                                           \
		if (!(ptr)) {                                                                                     \
			ERR_UAC_PRINT("NULL POINTER , force return\n");\
			return CVI_FAILURE;                                                                            \
		}                                                                                                      \
	} while (0)
#endif


#ifndef CHECK_NULL_PTR_RET
#define CHECK_NULL_PTR_RET(ptr, RET) \
	do {       \
		if (!(ptr)) {              \
			ERR_UAC_PRINT("NULL POINTER , force return\n");\
			return RET;                             \
		}                                            \
	} while (0)
#endif


#ifndef CHECK_NULL_PTR_RET_NONE
#define CHECK_NULL_PTR_RET_NONE(ptr) \
	do {       \
		if (!(ptr)) {              \
			ERR_UAC_PRINT("NULL POINTER , force return\n");\
			return;                             \
		}                                            \
	} while (0)
#endif
//----uac macro---------------------------------stop

//----uac global variable-----------------------start
cviuac_context *handle;
st_cviaudio_fifo_buffer gstCviaudioFifo[CVIAUDIO_DST_MAX_NUM];
bool gstBindCfg[CVIAUDIO_SRC_MAX_NUM][CVIAUDIO_DST_MAX_NUM];
ST_UAC_THREAD_ENABLE gThreadEnable;
void *streamInput_fifo_handle;
//----uac global variable-----------------------stop

//-----uac static function---------end
static void _clear_bind_config(void);
static void _create_uac_src_thread(pthread_t *input_thread,
					void *thread_func,
					void *thread_param);
static void _create_uac_dst_thread(pthread_t *output_thread,
					void *thread_func,
					void *thread_param);
static void *thread_src_mic_in_audio(void *arg);
static void *thread_src_usb_in_audio(void *arg);
static void *thread_src_file_in_audio(void *arg);
static void *thread_src_stream_in_audio(void *arg);

static void *thread_dst_spk_out_audio(void *arg);
static void *thread_dst_usb_out_audio(void *arg);
static void *thread_dst_file_save_audio(void *arg);
//for mp3 decoder: parse header callback
static int _mp3_dec_callback(void *inst,
				ST_MP3_DEC_INFO *pMp3_DecInfo,
				char *pBuff, int size);
static int _mp3_dec_callback_stream(void *inst,
				ST_MP3_DEC_INFO *pMp3_DecInfo,
				char *pBuff, int size);
//for aac adts decoder: parse adts header callback
static int _aac_dec_callback(void *paac_handle,
				ST_AAC_DEC_INFO *aac_info,
				char *pBuff, int byte);
static int _aac_dec_callback_stream(void *paac_handle,
				ST_AAC_DEC_INFO *aac_info,
				char *pBuff, int byte);
static int _attach_dst_handle_resampler(cviuac_dst_context *pUacDstHandle);

void uac_dump_audiodata(char *filename, char *buf, unsigned int len)
{
	FILE *fp;

	if (filename == NULL) {
		return;
	}

	fp = fopen(filename, "ab+");
	fwrite(buf, 1, len, fp);
	fclose(fp);

}

static int  cviUacGetEnv(char *env, char *fmt, void *param)
{
#define NOT_SET -2
#define INPUT_ERR -1
	char *pEnv;
	int val = NOT_SET;

	pEnv = getenv(env);
	if (pEnv) {
		if (strcmp(fmt, "%s") == 0) {
			strcpy(param, pEnv);
		} else {
			if (sscanf(pEnv, fmt, &val) != 1)
				return INPUT_ERR;
			printf("[UAC]%s = %d\n", env, val);
		}
	} else
		printf("[Err][%s][%d]getenv failure\n", __func__, __LINE__);

	return val;
}
static void cvi_audio_uac_getDbgMask(void)
{
#define NOT_SET -2
#define INPUT_ERR -1
	int  *pUAC_level = &gUAC_level;
	int getValue;

	*pUAC_level = 0;

	getValue = cviUacGetEnv("uac_level", "%d", NULL);
	if (getValue == NOT_SET || getValue == INPUT_ERR) {
		printf("uac level not set or input err..force level=0x04\n");
		*pUAC_level  = CVI_UAC_MASK_DBG;
	} else
		*pUAC_level = getValue;

	printf("[UAC]uac_level = [%d]\n", (*pUAC_level));
}


static void _clear_bind_config(void)
{
	for (int i = 0; i < CVIAUDIO_DST_MAX_NUM; i++) {
		gstBindCfg[CVIAUDIO_SRC_MIC_IN][i] = false;
		gstBindCfg[CVIAUDIO_SRC_USB_IN][i] = false;
		gstBindCfg[CVIAUDIO_SRC_FILE_IN][i] = false;

	}
}

static void _cviaudio_uac_InitParam(cviuac_context *pUacHandle)
{
	if (!pUacHandle)
		return;
	//TODO: check the chip id  to decide the valid
	pUacHandle->bValid = CVI_TRUE;
}

static void _create_uac_src_thread(pthread_t *input_thread,
					void *thread_func,
					void *thread_param)
{
	struct sched_param param;
	pthread_attr_t attr;
	int ret = 0;

	param.sched_priority = 80;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	ret = pthread_create(input_thread,
			&attr,
			thread_func,
			thread_param);

	if (ret < 0)
		ERR_UAC_PRINT("create thread failure\n");

	pthread_detach(*input_thread);
}



static int _attach_dst_handle_resampler(cviuac_dst_context *pUacDstHandle)
{
	CHECK_NULL_PTR_RET(pUacDstHandle, CVI_FAILURE);
	CVI_S32 s32Ret = CVI_FAILURE;

	memset(&pUacDstHandle->stSampleResFun, 0, sizeof(ST_SAMPLE_RES_FUN_S));

	ST_SAMPLE_RES_FUN_S *pstSampleResFun = &pUacDstHandle->stSampleResFun;

	if (pstSampleResFun->pLibHandle != CVI_NULL) {
		CVI_Audio_Dlclose(pstSampleResFun->pLibHandle);
		memset(pstSampleResFun, 0, sizeof(ST_SAMPLE_RES_FUN_S));
	}

	s32Ret = CVI_Audio_Dlopen(&(pstSampleResFun->pLibHandle), RES_LIB_NAME);
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
		__func__, __LINE__, "load resample lib fail!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &(pstSampleResFun->pCVI_Resampler_Create),
				pstSampleResFun->pLibHandle,
				"CVI_Resampler_Create");


	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			__func__, __LINE__, "find symbol error!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &
				(pstSampleResFun->pCVI_Resampler_Process),
				pstSampleResFun->pLibHandle,
				"CVI_Resampler_Process");

	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			__func__, __LINE__, "find symbol error!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &
				(pstSampleResFun->pCVI_Resampler_Destroy),
				pstSampleResFun->pLibHandle,
				"CVI_Resampler_Destroy");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			__func__, __LINE__, "find symbol  [Error]!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_Audio_Dlsym((CVI_VOID **) &
				(pstSampleResFun->pCVI_Resampler_GetMaxOutputNum),
				pstSampleResFun->pLibHandle,
				"CVI_Resampler_GetMaxOutputNum");
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n",
			__func__, __LINE__, "find symbol [Error]!\n");
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}


static void _create_uac_dst_thread(pthread_t *output_thread,
					void *thread_func,
					void *thread_param)
{
	struct sched_param param;
	pthread_attr_t attr;
	int ret = 0;

	param.sched_priority = 80;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	ret = pthread_create(output_thread,
			&attr,
			thread_func,
			thread_param);

	if (ret < 0)
		ERR_UAC_PRINT("create thread failure\n");
	else
		DBG_UAC_PRINT("create thread success!!\n");

	pthread_detach(*output_thread);
}

#define CVI_AAC_CB_STATUS_START 0x2
#define CVI_AAC_CB_STATUS_SEND_DATA 0x3
#define CVI_AAC_CB_CHANGE_INFO 0x4


static int _aac_dec_callback(void *paac_handle, ST_AAC_DEC_INFO *aac_info,
			     char *pBuff, int byte)
{
	UAC_UNUSED_REF(paac_handle);
	UAC_UNUSED_REF(pBuff);
	UAC_UNUSED_REF(byte);
	if (aac_info->cbState == CVI_AAC_CB_STATUS_START) {//ao init

		printf("AAC ADTS parsing=========================\n");
		printf("[channel]:[%d]\n", aac_info->channel_num);
		printf("[sample rate]:[%d]\n", aac_info->sample_rate);
		printf("[callback state] [0x%x]\n", aac_info->cbState);
		printf("[one aac data byte include ADTS]:[%d]\n", aac_info->frame_byte);
		printf("=========================================\n");

		cviuac_context *pUacHandle = handle;
		cviuac_src_context *pUacSrcHandle = NULL;

		pUacSrcHandle = pUacHandle->cviuac_src_handle[CVIAUDIO_SRC_FILE_IN];

		if (pUacHandle != NULL && pUacSrcHandle != NULL) {
			memcpy(&pUacSrcHandle->stAacDecInfo, aac_info, sizeof(ST_AAC_DEC_INFO));
			printf("[%s][%d] get the dec header info\n",  __func__, __LINE__);
		} else
			printf("[Warn][%s][%d] get the dec header info..not update\n",  __func__, __LINE__);

		aac_info->cbState = CVI_AAC_CB_STATUS_SEND_DATA;

	} else if (aac_info->cbState == CVI_AAC_CB_STATUS_SEND_DATA) {
#if 0
		memcpy(pCbBuffer, pBuff, byte);
		Ret = _send_cvi_ao(aac_info->channel_num, pCbBuffer, byte);
		if (Ret != 0)
			printf("[Error]_send_cvi_ao failure\n");

#endif
	}


	return 0;
}

static int _aac_dec_callback_stream(void *paac_handle, ST_AAC_DEC_INFO *aac_info,
			     char *pBuff, int byte)
{
	UAC_UNUSED_REF(paac_handle);
	UAC_UNUSED_REF(pBuff);
	UAC_UNUSED_REF(byte);
	if (aac_info->cbState == CVI_AAC_CB_STATUS_START) {//ao init

		printf("AAC ADTS parsing=========================\n");
		printf("[channel]:[%d]\n", aac_info->channel_num);
		printf("[sample rate]:[%d]\n", aac_info->sample_rate);
		printf("[callback state] [0x%x]\n", aac_info->cbState);
		printf("[one aac data byte include ADTS]:[%d]\n", aac_info->frame_byte);
		printf("=========================================\n");

		cviuac_context *pUacHandle = handle;
		cviuac_src_context *pUacSrcHandle = NULL;

		pUacSrcHandle = pUacHandle->cviuac_src_handle[CVIAUDIO_SRC_STREAM_IN];

		if (pUacHandle != NULL && pUacSrcHandle != NULL) {
			memcpy(&pUacSrcHandle->stAacDecInfo, aac_info, sizeof(ST_AAC_DEC_INFO));
			printf("[%s][%d] get the dec header info\n",  __func__, __LINE__);
		} else
			printf("[Warn][%s][%d] get the dec header info..not update\n",  __func__, __LINE__);

		aac_info->cbState = CVI_AAC_CB_STATUS_SEND_DATA;

	} else if (aac_info->cbState == CVI_AAC_CB_STATUS_SEND_DATA) {
#if 0
		memcpy(pCbBuffer, pBuff, byte);
		Ret = _send_cvi_ao(aac_info->channel_num, pCbBuffer, byte);
		if (Ret != 0)
			printf("[Error]_send_cvi_ao failure\n");

#endif
	}


	return 0;
}




//callback function should exist in sample code
static int _mp3_dec_callback_stream(void *inst,
				ST_MP3_DEC_INFO *pMp3_DecInfo,
				char *pBuff, int size)
{

	if (inst == NULL) {
		printf("Null pt [%s][%d]\n", __func__, __LINE__);
		printf("Null pt in callback function force return\n");
		return -1;
	}

	if (pMp3_DecInfo == NULL) {
		printf("Null pt [%s][%d]\n", __func__, __LINE__);
		printf("Null pt in callback pMp3_DecInfo force return\n");
		return -1;
	}

	cviuac_context *pUacHandle = handle;
	cviuac_src_context *pUacSrcHandle = NULL;

	pUacSrcHandle = pUacHandle->cviuac_src_handle[CVIAUDIO_SRC_STREAM_IN];

	//save the data or play out to AO
	if (pMp3_DecInfo->cbState == CVI_MP3_DEC_CB_STATUS_GET_FIRST_INFO) {


		printf("stage:CVI_MP3_DEC_CB_STATUS_GET_FIRST_INFO\n");
		printf("========================================\n");
		printf("[channel]:[%d]\n", pMp3_DecInfo->channel_num);
		printf("[sample rate]:[%d]\n", pMp3_DecInfo->sample_rate);
		printf("[bit rate]:[%d]\n", pMp3_DecInfo->bit_rate);
		printf("[frame cnt]:[%d]\n", pMp3_DecInfo->frame_cnt);
		printf("[callback state] [0x%x]\n", pMp3_DecInfo->cbState);
		printf("=========================================\n");
		//create ao channel


		if (pUacHandle != NULL && pUacSrcHandle != NULL) {
			memcpy(&pUacSrcHandle->stMp3DecInfo, pMp3_DecInfo, sizeof(ST_MP3_DEC_INFO));
			printf("[%s][%d] get the dec header info\n",  __func__, __LINE__);
		} else
			printf("[Warn][%s][%d] get the dec header info..not update\n",  __func__, __LINE__);
	} else if (pMp3_DecInfo->cbState == CVI_MP3_DEC_CB_STATUS_STABLE) {
		pBuff = pBuff;
		size = size;
		//dont send file in small file mode
	} else if (pMp3_DecInfo->cbState == CVI_MP3_DEC_CB_STATUS_SMP_RATE_CHG ||
		pMp3_DecInfo->cbState == CVI_MP3_DEC_CB_STATUS_INFO_CHG) {
		printf("stage:CVI_MP3_DEC_CB_STATUS_INFO_CHG\n");
		printf("new chn[%d]\n", pMp3_DecInfo->channel_num);
		printf("new rate[%d]\n", pMp3_DecInfo->sample_rate);

		if (pUacHandle != NULL && pUacSrcHandle != NULL) {
			printf("[%s][%d] get the dec header chg info\n",  __func__, __LINE__);
			memcpy(&pUacSrcHandle->stMp3DecInfo, pMp3_DecInfo, sizeof(ST_MP3_DEC_INFO));
		} else {
			printf("[%s][%d][Warn] NotUpdate get the dec header chg info\n",  __func__, __LINE__);
		}
	} else if (pMp3_DecInfo->cbState == CVI_MP3_DEC_CB_STATUS_INIT) {
		printf("stage:CVI_MP3_DEC_CB_STATUS_INIT\n");
		printf("Not decode any mp3 frame info yet........\n");
	}
	return 0;
}

//callback function should exist in sample code
static int _mp3_dec_callback(void *inst, ST_MP3_DEC_INFO *pMp3_DecInfo, char *pBuff, int size)
{

	if (inst == NULL) {
		printf("Null pt [%s][%d]\n", __func__, __LINE__);
		printf("Null pt in callback function force return\n");
		return -1;
	}

	if (pMp3_DecInfo == NULL) {
		printf("Null pt [%s][%d]\n", __func__, __LINE__);
		printf("Null pt in callback pMp3_DecInfo force return\n");
		return -1;
	}

	cviuac_context *pUacHandle = handle;
	cviuac_src_context *pUacSrcHandle = NULL;

	pUacSrcHandle = pUacHandle->cviuac_src_handle[CVIAUDIO_SRC_FILE_IN];

	//save the data or play out to AO
	if (pMp3_DecInfo->cbState == CVI_MP3_DEC_CB_STATUS_GET_FIRST_INFO) {


		printf("stage:CVI_MP3_DEC_CB_STATUS_GET_FIRST_INFO\n");
		printf("========================================\n");
		printf("[channel]:[%d]\n", pMp3_DecInfo->channel_num);
		printf("[sample rate]:[%d]\n", pMp3_DecInfo->sample_rate);
		printf("[bit rate]:[%d]\n", pMp3_DecInfo->bit_rate);
		printf("[frame cnt]:[%d]\n", pMp3_DecInfo->frame_cnt);
		printf("[callback state] [0x%x]\n", pMp3_DecInfo->cbState);
		printf("=========================================\n");
		//create ao channel


		if (pUacHandle != NULL && pUacSrcHandle != NULL) {
			memcpy(&pUacSrcHandle->stMp3DecInfo, pMp3_DecInfo, sizeof(ST_MP3_DEC_INFO));
			printf("[%s][%d] get the dec header info\n",  __func__, __LINE__);
		} else
			printf("[Warn][%s][%d] get the dec header info..not update\n",  __func__, __LINE__);
	} else if (pMp3_DecInfo->cbState == CVI_MP3_DEC_CB_STATUS_STABLE) {
		pBuff = pBuff;
		size = size;
		//dont send file in small file mode
	} else if (pMp3_DecInfo->cbState == CVI_MP3_DEC_CB_STATUS_SMP_RATE_CHG ||
		pMp3_DecInfo->cbState == CVI_MP3_DEC_CB_STATUS_INFO_CHG) {
		printf("stage:CVI_MP3_DEC_CB_STATUS_INFO_CHG\n");
		printf("new chn[%d]\n", pMp3_DecInfo->channel_num);
		printf("new rate[%d]\n", pMp3_DecInfo->sample_rate);

		if (pUacHandle != NULL && pUacSrcHandle != NULL) {
			printf("[%s][%d] get the dec header chg info\n",  __func__, __LINE__);
			memcpy(&pUacSrcHandle->stMp3DecInfo, pMp3_DecInfo, sizeof(ST_MP3_DEC_INFO));
		} else {
			printf("[%s][%d][Warn] NotUpdate get the dec header chg info\n",  __func__, __LINE__);
		}
	} else if (pMp3_DecInfo->cbState == CVI_MP3_DEC_CB_STATUS_INIT) {
		printf("stage:CVI_MP3_DEC_CB_STATUS_INIT\n");
		printf("Not decode any mp3 frame info yet........\n");
	}
	return 0;
}
static void *thread_src_stream_in_audio(void *arg)
{

#define CVIUAC_STREAM_SRC_OPEN_STATE 0xA1
#define CVIUAC_STREAM_SRC_PCM_STATE 0xA2
#define CVIUAC_STREAM_SRC_DECODE_MP3_STATE 0xA3
#define CVIUAC_STREAM_SRC_DECODE_AAC_STATE 0xA4
#define CVIUAC_STREAM_SRC_CLOSE_FD_STATE 0xA5

#define CVIUAC_STREAM_SRC_INPUT_BUFFER_SIZE (1024*2)
//#define CVIUAC_FILEIN_SRC_OUTPUT_BUFFER_SIZE (1024*10)
//mp3 outputbuffer acquire 10*1024
//aac outputbuffer acquire 25~30 *1024
#define CVIUAC_STREAM_SRC_OUTPUT_BUFFER_SIZE (40 * 1024)
#define CVIUAC_STREAM_SRC_PCM_READ_SIZE 480
#define CVIUAC_STREAM_SRC_MP3_READ_SIZE (1024*2)
#define CVIUAC_STREAM_SRC_AAC_READ_SIZE (1024*2) //in bytes

	if (arg == NULL) {
		ERR_UAC_PRINT("\n");
		return (void *)0;
	}

	ST_CVI_UAC_FILE_PARAM *pFileParam = (ST_CVI_UAC_FILE_PARAM *)arg;
	int StreamInSrcState = CVIUAC_STREAM_SRC_OPEN_STATE;
	cviuac_context *pUacHandle = handle;
	cviuac_src_context *pUacSrcHandle = NULL;

	pUacSrcHandle = pUacHandle->cviuac_src_handle[CVIAUDIO_SRC_STREAM_IN];
	char *input_buffer = NULL;
	char *output_buffer = NULL;
	int read_size = 0;
	int s32Ret = 0;
	int s32OutLen = 0;
	int tmp = 0;
	int bufferlevel = 0;
	int buffer_write_count = 0;

	if (pUacSrcHandle == NULL) {
		ERR_UAC_PRINT("\n");
		return (void *)0;
	}

	while (gThreadEnable.bEnableSrc_StreamIn) {

		switch (StreamInSrcState) {
		case CVIUAC_STREAM_SRC_OPEN_STATE:
		{

			SAFE_FREE_BUF(input_buffer);
			SAFE_FREE_BUF(output_buffer);
			input_buffer = (char *)malloc(CVIUAC_STREAM_SRC_INPUT_BUFFER_SIZE);
			output_buffer = (char *)malloc(CVIUAC_STREAM_SRC_OUTPUT_BUFFER_SIZE);
			if (pUacSrcHandle->codec_type == CVIAUDIO_UAC_TYPE_MP3) {
				StreamInSrcState = CVIUAC_STREAM_SRC_DECODE_MP3_STATE;
			} else if (pUacSrcHandle->codec_type == CVIAUDIO_UAC_TYPE_AAC)
				StreamInSrcState = CVIUAC_STREAM_SRC_DECODE_AAC_STATE;
			else if (pUacSrcHandle->codec_type == CVIAUDIO_UAC_TYPE_PCM)
				StreamInSrcState = CVIUAC_STREAM_SRC_PCM_STATE;
			else {
				ERR_UAC_PRINT("Wrong Type[%d]\n", pUacSrcHandle->codec_type);
				StreamInSrcState = CVIUAC_STREAM_SRC_CLOSE_FD_STATE;
			}

		}
		break;

		case CVIUAC_STREAM_SRC_PCM_STATE:
		{
			bool bOutFiFoExist = false;
			int buffer_writeBytes = 0;

			st_cviaudio_fifo_buffer *pfifo = NULL;

			if (streamInput_fifo_handle == NULL) {
				ERR_UAC_PRINT("\n");
				break;
			}
			bufferlevel = CycleBufferDataLen(streamInput_fifo_handle);
			if (bufferlevel > CVIUAC_STREAM_SRC_PCM_READ_SIZE) {
				read_size = CycleBufferRead(streamInput_fifo_handle,
							input_buffer,
							CVIUAC_STREAM_SRC_PCM_READ_SIZE);
				if (read_size != CVIUAC_STREAM_SRC_PCM_READ_SIZE)
					ERR_UAC_PRINT("\n");
				StreamInSrcState = CVIUAC_STREAM_SRC_PCM_STATE;
			} else {
				usleep(50*1000);
				StreamInSrcState = CVIUAC_STREAM_SRC_PCM_STATE;
			}

			for (int dstIndex = 0; dstIndex < CVIAUDIO_DST_MAX_NUM; dstIndex++) {
			if (gstBindCfg[CVIAUDIO_SRC_STREAM_IN][dstIndex] == true) {
				bOutFiFoExist = true;
				//put data in the fifo
				pfifo = &gstCviaudioFifo[dstIndex];
				if (pfifo->pcviaudio_fifo_handle && pfifo->bEnable) {
				buffer_writeBytes = CycleBufferWrite(pfifo->pcviaudio_fifo_handle,
									input_buffer,
									read_size);

				tmp = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
				if (buffer_writeBytes != read_size) {
					ERR_UAC_PRINT("writein[%d] trueWriteIn[%d] level[%d] size[%d]\n",
					read_size, buffer_writeBytes, tmp, pfifo->s32fifo_size);
				}

				if (tmp > (pfifo->s32fifo_size / 2)) {
					//incase the playout thread is too late to create..
					usleep(100*1000);//sleep 100ms for fifo level
				}



				if (!buffer_writeBytes) {
				//buffer abnormal
				buffer_write_count++;
				usleep(1000 * 20);
				if (buffer_write_count >= 10) {
					//tmp = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
					ERR_UAC_PRINT("STREAM in fifo full bufferlevel[%d] of [%d]\n",
					tmp, pfifo->s32fifo_size);
					usleep(500*1000);
				}
				} else {
					//buffer ok
					buffer_write_count = 0;
				}

				} else {
					ERR_UAC_PRINT("stream in src fifo not create for dstPath[%d]\n", dstIndex);
					sleep(1);
				}

				if (handle != NULL) {
					cviuac_context *pUacHandle = handle;
					cviuac_dst_context *pUacDstHandle = NULL;

					if (pUacHandle->cviuac_dst_handle[dstIndex] != NULL) {
					pUacDstHandle = pUacHandle->cviuac_dst_handle[dstIndex];
					pUacDstHandle->update_pcm_info.channels = pFileParam->pcmInfo.channels;
					pUacDstHandle->update_pcm_info.period_size = pFileParam->pcmInfo.period_size;
					pUacDstHandle->update_pcm_info.rate = pFileParam->pcmInfo.rate;
					if (pFileParam->pfd_save != NULL && dstIndex == CVIAUDIO_DST_FILE_SAVE)
						pUacDstHandle->pfd_dst_save = pFileParam->pfd_save;
					} else {
						ERR_UAC_PRINT("DstPath[%d]not create with bind mode on..\n",
								dstIndex);
					}

				}
			}
			}
			if (bOutFiFoExist == false)
				sleep(1);

		}
		break;

		case CVIUAC_STREAM_SRC_DECODE_MP3_STATE:
		{
			s32OutLen = 0;
			s32Ret = 0;
			int buffer_writeBytes = 0;
			int buffer_write_count = 0;
			bool bOutFiFoExist = false;
			st_cviaudio_fifo_buffer *pfifo = NULL;

			read_size = 0;

			bufferlevel = CycleBufferDataLen(streamInput_fifo_handle);
			if (bufferlevel > CVIUAC_STREAM_SRC_MP3_READ_SIZE) {
				read_size = CycleBufferRead(streamInput_fifo_handle,
							input_buffer,
							CVIUAC_STREAM_SRC_MP3_READ_SIZE);
				if (read_size != CVIUAC_STREAM_SRC_MP3_READ_SIZE)
					ERR_UAC_PRINT("\n");
				StreamInSrcState = CVIUAC_STREAM_SRC_DECODE_MP3_STATE;
			} else {
				usleep(50*1000);
				StreamInSrcState = CVIUAC_STREAM_SRC_DECODE_MP3_STATE;
			}

			if (read_size > 0) {
			s32Ret = CVI_MP3_Decode((CVI_VOID *)pUacSrcHandle->pMp3DecHandler,
						(CVI_VOID *)input_buffer,
						(CVI_VOID *)output_buffer,
						(CVI_S32)read_size,
						(CVI_S32 *)&s32OutLen);

			if (s32Ret != CVI_SUCCESS)
				ERR_UAC_PRINT("mp3 decode not success!!\n");
			if (s32OutLen > CVIUAC_STREAM_SRC_OUTPUT_BUFFER_SIZE)
				ERR_UAC_PRINT("mp3 decode out buffer not large enough\n");
			}

			//check update info to Dst
			if (pUacSrcHandle->stMp3DecInfo.sample_rate != 0) {

			//update to cviuac_dst_context for  update_pcm_info
			for (int dstIndex = 0; dstIndex < CVIAUDIO_DST_MAX_NUM; dstIndex++) {
			if (gstBindCfg[CVIAUDIO_SRC_STREAM_IN][dstIndex] == true) {
				if (handle != NULL) {
					cviuac_context *pUacHandle = handle;
					cviuac_dst_context *pUacDstHandle = NULL;

					if (pUacHandle->cviuac_dst_handle[dstIndex] != NULL) {
					pUacDstHandle = pUacHandle->cviuac_dst_handle[dstIndex];
					pUacDstHandle->update_pcm_info.channels =
							pUacSrcHandle->stMp3DecInfo.channel_num;
					pUacDstHandle->update_pcm_info.period_size = 480;
					pUacDstHandle->update_pcm_info.rate = pUacSrcHandle->stMp3DecInfo.sample_rate;
					if (pFileParam->pfd_save != NULL && dstIndex == CVIAUDIO_DST_FILE_SAVE)
						pUacDstHandle->pfd_dst_save = pFileParam->pfd_save;
					} else {
						ERR_UAC_PRINT("DstPath[%d]not create with bind mode on..\n",
								dstIndex);
					}

				}

			}
			}
			}

			//block mode to write in cycle buffer
			for (int dstIndex = 0; dstIndex < CVIAUDIO_DST_MAX_NUM; dstIndex++) {
			if (gstBindCfg[CVIAUDIO_SRC_STREAM_IN][dstIndex] == true) {
				bOutFiFoExist = true;
				//put data in the fifo
				pfifo = &gstCviaudioFifo[dstIndex];
				if (pfifo->pcviaudio_fifo_handle && pfifo->bEnable) {
				if (s32OutLen != 0) {

				buffer_writeBytes = CycleBufferWriteWait(pfifo->pcviaudio_fifo_handle,
									output_buffer,
									s32OutLen,
									1000);

				tmp = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
				if (tmp > (pfifo->s32fifo_size / 2))
					usleep(100*1000);//sleep 100ms for fifo level


				if (!buffer_writeBytes) {
					ERR_UAC_PRINT("\n");
					buffer_write_count++;
					usleep(1000 * 50);
					if (buffer_write_count >= 5) {
						tmp = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
						ERR_UAC_PRINT("STREAM in fifo full mp3 bufferlevel[%d] of [%d]\n",
						tmp, pfifo->s32fifo_size);
						sleep(1);
					}
				} else {
					//buffer ok
					buffer_write_count = 0;
					if (s32OutLen > buffer_writeBytes)
						DBG_UAC_PRINT("mp3 decoded data buffer overrun\n");
				}
				}
				} else {
					ERR_UAC_PRINT("File in src fifo not create for dstPath[%d]\n", dstIndex);
					sleep(1);
				}
			}
			}

			if (bOutFiFoExist == false) {
				sleep(1);
				printf("CVIUAC_STREAM_SRC_DECODE_MP3_STATE fifo not exist yet\n");
			}


		}
		break;

		case CVIUAC_STREAM_SRC_DECODE_AAC_STATE:
		{
			s32OutLen = 0;
			s32Ret = 0;
			int buffer_writeBytes = 0;
			int buffer_write_count = 0;
			bool bOutFiFoExist = false;
			st_cviaudio_fifo_buffer *pfifo = NULL;

			read_size = 0;
#if 0
			read_size = fread(input_buffer, 1,
						CVIUAC_STREAM_SRC_AAC_READ_SIZE,
						pFileParam->pfd);
			if (read_size == 0) {
				DBG_UAC_PRINT("read AAC file end ...prepare to leave\n");
				StreamInSrcState = CVIUAC_STREAM_SRC_CLOSE_FD_STATE;
			} else {
				if (read_size < CVIUAC_STREAM_SRC_AAC_READ_SIZE)
					DBG_UAC_PRINT("read AAC file, last input[%d]\n", read_size);
				StreamInSrcState = CVIUAC_STREAM_SRC_DECODE_AAC_STATE;
			}
#endif
			bufferlevel = CycleBufferDataLen(streamInput_fifo_handle);
			if (bufferlevel > CVIUAC_STREAM_SRC_AAC_READ_SIZE) {
				read_size = CycleBufferRead(streamInput_fifo_handle,
							input_buffer,
							CVIUAC_STREAM_SRC_AAC_READ_SIZE);
				if (read_size != CVIUAC_STREAM_SRC_AAC_READ_SIZE)
					ERR_UAC_PRINT("\n");
				StreamInSrcState = CVIUAC_STREAM_SRC_DECODE_AAC_STATE;
			} else {
				usleep(50*1000);
				StreamInSrcState = CVIUAC_STREAM_SRC_DECODE_AAC_STATE;
			}
			//send into aac decode
			if (read_size != 0) {
				CHECK_NULL_PTR_RET(pUacSrcHandle->pAacDecHandler, (void *)0);
				s32Ret = CVI_AAC_Decode(pUacSrcHandle->pAacDecHandler,
							(unsigned char *)input_buffer,
							(INT_PCM *)output_buffer,
							read_size,
							(int *)&s32OutLen);
				if (s32Ret != 0) {
					ERR_UAC_PRINT("aac decode faile\n");
					StreamInSrcState = CVIUAC_STREAM_SRC_CLOSE_FD_STATE;
				}
			}
			s32OutLen = s32OutLen * BYTES_PER_SAMPLE;
			if (s32OutLen > CVIUAC_STREAM_SRC_OUTPUT_BUFFER_SIZE) {
				ERR_UAC_PRINT("[Fatal]AAC decode out buffer not large enough\n");
				break;
			}
			//send header info to cviuac_dst_context for  channel num/rate
			for (int dstIndex = 0; dstIndex < CVIAUDIO_DST_MAX_NUM; dstIndex++) {
			if (gstBindCfg[CVIAUDIO_SRC_STREAM_IN][dstIndex] == true) {
				if (handle != NULL) {
					cviuac_context *pUacHandle = handle;
					cviuac_dst_context *pUacDstHandle = NULL;

					if (pUacHandle->cviuac_dst_handle[dstIndex] != NULL) {
					pUacDstHandle = pUacHandle->cviuac_dst_handle[dstIndex];
					pUacDstHandle->update_pcm_info.channels =
							pUacSrcHandle->stAacDecInfo.channel_num;
					pUacDstHandle->update_pcm_info.period_size = 480;
					pUacDstHandle->update_pcm_info.rate = pUacSrcHandle->stAacDecInfo.sample_rate;
					if (pFileParam->pfd_save != NULL && dstIndex == CVIAUDIO_DST_FILE_SAVE)
						pUacDstHandle->pfd_dst_save = pFileParam->pfd_save;

					} else {
						ERR_UAC_PRINT("DstPath[%d]not create with bind mode on..\n",
								dstIndex);
					}

				}

			}
			}


			//write data block mode in fifo
			for (int dstIndex = 0; dstIndex < CVIAUDIO_DST_MAX_NUM; dstIndex++) {
			if (gstBindCfg[CVIAUDIO_SRC_STREAM_IN][dstIndex] == true) {
				bOutFiFoExist = true;
				//put data in the fifo
				pfifo = &gstCviaudioFifo[dstIndex];
				if (pfifo->pcviaudio_fifo_handle && pfifo->bEnable) {
				if (s32OutLen != 0) {
				buffer_writeBytes = CycleBufferWriteWait(pfifo->pcviaudio_fifo_handle,
									output_buffer,
									s32OutLen,
									1000);


				tmp = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
				if (tmp > (pfifo->s32fifo_size / 2))
					usleep(200*1000);//sleep 100ms for fifo level

				if (!buffer_writeBytes) {
					ERR_UAC_PRINT("\n");
					buffer_write_count++;
					usleep(1000 * 500);//sleep 500 ms
					if (buffer_write_count >= 5) {
						tmp = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
						ERR_UAC_PRINT("USB in fifo full AAC bufferlevel[%d] of [%d]\n",
						tmp, pfifo->s32fifo_size);
						sleep(1);
					}
				} else {
					//buffer ok
					buffer_write_count = 0;
					if (s32OutLen > buffer_writeBytes)
						DBG_UAC_PRINT("AAC decoded overrun buflevel[%d] bufsize[%d]\n",
								tmp, pfifo->s32fifo_size);
				}
				}
				} else {
					ERR_UAC_PRINT("File in src fifo not create for dstPath[%d]\n", dstIndex);
					sleep(1);
				}
			}
			}

			if (bOutFiFoExist == false) {
				sleep(1);
				printf("CVIUAC_STREAM_SRC_DECODE_AAC_STATE fifo not exist yet\n");
			}
		}
		break;

		case CVIUAC_STREAM_SRC_CLOSE_FD_STATE:
		{
			if (pFileParam->pfd != NULL)
				fclose(pFileParam->pfd);
			SAFE_FREE_BUF(input_buffer);
			SAFE_FREE_BUF(output_buffer);
		}
		break;

		default:
			ERR_UAC_PRINT("[Error]file in as src abnormal...[%d]\n", StreamInSrcState);
			sleep(2);
			break;
		}

	}

	SAFE_FREE_BUF(input_buffer);
	SAFE_FREE_BUF(output_buffer);
	printf("[%s] close\n", __func__);
	return (void *)0;
}


static void *thread_src_file_in_audio(void *arg)
{

#define CVIUAC_FILEIN_SRC_OPEN_STATE 0x81
#define CVIUAC_FILEIN_SRC_PCM_STATE 0x82
#define CVIUAC_FILEIN_SRC_DECODE_MP3_STATE 0x83
#define CVIUAC_FILEIN_SRC_DECODE_AAC_STATE 0x84
#define CVIUAC_FILEIN_SRC_CLOSE_FD_STATE 0x85

#define CVIUAC_FILEIN_SRC_INPUT_BUFFER_SIZE (1024*2)
//#define CVIUAC_FILEIN_SRC_OUTPUT_BUFFER_SIZE (1024*10)
//mp3 outputbuffer acquire 10*1024
//aac outputbuffer acquire 25~30 *1024
#define CVIUAC_FILEIN_SRC_OUTPUT_BUFFER_SIZE (40 * 1024)
#define CVIUAC_FILEIN_SRC_PCM_READ_SIZE 480
#define CVIUAC_FILEIN_SRC_MP3_READ_SIZE 512
#define CVIUAC_FILEIN_SRC_AAC_READ_SIZE (1024*2) //in bytes

	if (arg == NULL) {
		ERR_UAC_PRINT("\n");
		return (void *)0;
	}

	ST_CVI_UAC_FILE_PARAM *pFileParam = (ST_CVI_UAC_FILE_PARAM *)arg;
	int FileInSrcState = CVIUAC_FILEIN_SRC_OPEN_STATE;
	cviuac_context *pUacHandle = handle;
	cviuac_src_context *pUacSrcHandle = NULL;

	pUacSrcHandle = pUacHandle->cviuac_src_handle[CVIAUDIO_SRC_FILE_IN];
	char *input_buffer = NULL;
	char *output_buffer = NULL;
	int read_size = 0;
	int s32Ret = 0;
	int s32OutLen = 0;
	int tmp = 0;


	if (pUacSrcHandle == NULL) {
		ERR_UAC_PRINT("\n");
		return (void *)0;
	}

	while (gThreadEnable.bEnableSrc_FileIn) {

		switch (FileInSrcState) {
		case CVIUAC_FILEIN_SRC_OPEN_STATE:
		{
			if (pFileParam->pfd == NULL) {
				ERR_UAC_PRINT("\n");
				return (void *)0;

			} else {
				SAFE_FREE_BUF(input_buffer);
				SAFE_FREE_BUF(output_buffer);
				input_buffer = (char *)malloc(CVIUAC_FILEIN_SRC_INPUT_BUFFER_SIZE);
				output_buffer = (char *)malloc(CVIUAC_FILEIN_SRC_OUTPUT_BUFFER_SIZE);

				if (pUacSrcHandle->codec_type == CVIAUDIO_UAC_TYPE_MP3)
					FileInSrcState = CVIUAC_FILEIN_SRC_DECODE_MP3_STATE;
				else if (pUacSrcHandle->codec_type == CVIAUDIO_UAC_TYPE_AAC)
					FileInSrcState = CVIUAC_FILEIN_SRC_DECODE_AAC_STATE;
				else if (pUacSrcHandle->codec_type == CVIAUDIO_UAC_TYPE_PCM)
					FileInSrcState = CVIUAC_FILEIN_SRC_PCM_STATE;
				else {
					ERR_UAC_PRINT("Wrong Type[%d]\n", pUacSrcHandle->codec_type);
					FileInSrcState = CVIUAC_FILEIN_SRC_CLOSE_FD_STATE;
				}
			}
		}
		break;

		case CVIUAC_FILEIN_SRC_PCM_STATE:
		{
			bool bOutFiFoExist = false;
			int buffer_writeBytes = 0;
			int buffer_write_count = 0;
			st_cviaudio_fifo_buffer *pfifo = NULL;

			read_size = fread(input_buffer, 1, CVIUAC_FILEIN_SRC_PCM_READ_SIZE, pFileParam->pfd);
			if (read_size == 0) {
				DBG_UAC_PRINT("read file end ...break\n");
				FileInSrcState = CVIUAC_FILEIN_SRC_CLOSE_FD_STATE;
			} else {
				FileInSrcState = CVIUAC_FILEIN_SRC_PCM_STATE;
			}

			for (int dstIndex = 0; dstIndex < CVIAUDIO_DST_MAX_NUM; dstIndex++) {
			if (gstBindCfg[CVIAUDIO_SRC_FILE_IN][dstIndex] == true) {
				bOutFiFoExist = true;
				//put data in the fifo
				pfifo = &gstCviaudioFifo[dstIndex];
				if (pfifo->pcviaudio_fifo_handle && pfifo->bEnable) {
				buffer_writeBytes = CycleBufferWrite(pfifo->pcviaudio_fifo_handle,
									input_buffer,
									read_size);

				tmp = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
				if (buffer_writeBytes != read_size) {
					ERR_UAC_PRINT("writein[%d] trueWriteIn[%d] level[%d] size[%d]\n",
					read_size, buffer_writeBytes, tmp, pfifo->s32fifo_size);
				}

				if (tmp > (pfifo->s32fifo_size / 2)) {
					//incase the playout thread is too late to create..
					usleep(100*1000);//sleep 100ms for fifo level
				}



				if (!buffer_writeBytes) {
				//buffer abnormal
				buffer_write_count++;
				usleep(1000 * 20);
				if (buffer_write_count >= 10) {
					//tmp = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
					ERR_UAC_PRINT("USB in fifo full bufferlevel[%d] of [%d]\n",
					tmp, pfifo->s32fifo_size);
					sleep(1);
				} else {
					//buffer ok
					buffer_write_count = 0;
				}
				}

				} else {
					ERR_UAC_PRINT("File in src fifo not create for dstPath[%d]\n", dstIndex);
					sleep(1);
				}

				if (handle != NULL) {
					cviuac_context *pUacHandle = handle;
					cviuac_dst_context *pUacDstHandle = NULL;

					if (pUacHandle->cviuac_dst_handle[dstIndex] != NULL) {
					pUacDstHandle = pUacHandle->cviuac_dst_handle[dstIndex];
					pUacDstHandle->update_pcm_info.channels = pFileParam->pcmInfo.channels;
					pUacDstHandle->update_pcm_info.period_size = pFileParam->pcmInfo.period_size;
					pUacDstHandle->update_pcm_info.rate = pFileParam->pcmInfo.rate;
					if (pFileParam->pfd_save != NULL && dstIndex == CVIAUDIO_DST_FILE_SAVE)
						pUacDstHandle->pfd_dst_save = pFileParam->pfd_save;
					} else {
						ERR_UAC_PRINT("DstPath[%d]not create with bind mode on..\n",
								dstIndex);
					}

				}
			}
			}
			if (bOutFiFoExist == false)
				sleep(1);

		}
		break;

		case CVIUAC_FILEIN_SRC_DECODE_MP3_STATE:
		{
			s32OutLen = 0;
			s32Ret = 0;
			int buffer_writeBytes = 0;
			int buffer_write_count = 0;
			bool bOutFiFoExist = false;
			st_cviaudio_fifo_buffer *pfifo = NULL;

			read_size = fread(input_buffer, 1,
						CVIUAC_FILEIN_SRC_MP3_READ_SIZE,
						pFileParam->pfd);
			if (read_size == 0) {
				DBG_UAC_PRINT("read mp3 file end ...prepare to leave\n");
				FileInSrcState = CVIUAC_FILEIN_SRC_CLOSE_FD_STATE;
			} else
				FileInSrcState = CVIUAC_FILEIN_SRC_DECODE_MP3_STATE;

			s32Ret = CVI_MP3_Decode((CVI_VOID *)pUacSrcHandle->pMp3DecHandler,
						(CVI_VOID *)input_buffer,
						(CVI_VOID *)output_buffer,
						(CVI_S32)read_size,
						(CVI_S32 *)&s32OutLen);

			if (s32Ret != CVI_SUCCESS)
				ERR_UAC_PRINT("mp3 decode not success!!\n");
			if (s32OutLen > CVIUAC_FILEIN_SRC_OUTPUT_BUFFER_SIZE)
				ERR_UAC_PRINT("mp3 decode out buffer not large enough\n");


			//check update info to Dst
			if (pUacSrcHandle->stMp3DecInfo.sample_rate != 0) {

			//update to cviuac_dst_context for  update_pcm_info
			for (int dstIndex = 0; dstIndex < CVIAUDIO_DST_MAX_NUM; dstIndex++) {
			if (gstBindCfg[CVIAUDIO_SRC_FILE_IN][dstIndex] == true) {
				if (handle != NULL) {
					cviuac_context *pUacHandle = handle;
					cviuac_dst_context *pUacDstHandle = NULL;

					if (pUacHandle->cviuac_dst_handle[dstIndex] != NULL) {
					pUacDstHandle = pUacHandle->cviuac_dst_handle[dstIndex];
					pUacDstHandle->update_pcm_info.channels =
							pUacSrcHandle->stMp3DecInfo.channel_num;
					pUacDstHandle->update_pcm_info.period_size = 480;
					pUacDstHandle->update_pcm_info.rate = pUacSrcHandle->stMp3DecInfo.sample_rate;
					if (pFileParam->pfd_save != NULL && dstIndex == CVIAUDIO_DST_FILE_SAVE)
						pUacDstHandle->pfd_dst_save = pFileParam->pfd_save;
					} else {
						ERR_UAC_PRINT("DstPath[%d]not create with bind mode on..\n",
								dstIndex);
					}

				}

			}
			}
			}

			//block mode to write in cycle buffer
			for (int dstIndex = 0; dstIndex < CVIAUDIO_DST_MAX_NUM; dstIndex++) {
			if (gstBindCfg[CVIAUDIO_SRC_FILE_IN][dstIndex] == true) {
				bOutFiFoExist = true;
				//put data in the fifo
				pfifo = &gstCviaudioFifo[dstIndex];
				if (pfifo->pcviaudio_fifo_handle && pfifo->bEnable) {
				if (s32OutLen != 0) {

				buffer_writeBytes = CycleBufferWriteWait(pfifo->pcviaudio_fifo_handle,
									output_buffer,
									s32OutLen,
									1000);

				tmp = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
				if (tmp > (pfifo->s32fifo_size / 2))
					usleep(100*1000);//sleep 100ms for fifo level


				if (!buffer_writeBytes) {
					ERR_UAC_PRINT("\n");
					buffer_write_count++;
					usleep(1000 * 50);
					if (buffer_write_count >= 5) {
						tmp = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
						ERR_UAC_PRINT("USB in fifo full mp3 bufferlevel[%d] of [%d]\n",
						tmp, pfifo->s32fifo_size);
						sleep(1);
					}
				} else {
					//buffer ok
					buffer_write_count = 0;
					if (s32OutLen > buffer_writeBytes)
						DBG_UAC_PRINT("mp3 decoded data buffer overrun\n");
				}
				}
				} else {
					ERR_UAC_PRINT("File in src fifo not create for dstPath[%d]\n", dstIndex);
					sleep(1);
				}
			}
			}

			if (bOutFiFoExist == false) {
				sleep(1);
				printf("CVIUAC_FILEIN_SRC_DECODE_MP3_STATE fifo not exist yet\n");
			}


		}
		break;

		case CVIUAC_FILEIN_SRC_DECODE_AAC_STATE:
		{
			s32OutLen = 0;
			s32Ret = 0;
			int buffer_writeBytes = 0;
			int buffer_write_count = 0;
			bool bOutFiFoExist = false;
			st_cviaudio_fifo_buffer *pfifo = NULL;

			read_size = fread(input_buffer, 1,
						CVIUAC_FILEIN_SRC_AAC_READ_SIZE,
						pFileParam->pfd);
			if (read_size == 0) {
				DBG_UAC_PRINT("read AAC file end ...prepare to leave\n");
				FileInSrcState = CVIUAC_FILEIN_SRC_CLOSE_FD_STATE;
			} else {
				if (read_size < CVIUAC_FILEIN_SRC_AAC_READ_SIZE)
					DBG_UAC_PRINT("read AAC file, last input[%d]\n", read_size);
				FileInSrcState = CVIUAC_FILEIN_SRC_DECODE_AAC_STATE;
			}
			//send into aac decode
			if (read_size != 0) {
				CHECK_NULL_PTR_RET(pUacSrcHandle->pAacDecHandler, (void *)0);
				s32Ret = CVI_AAC_Decode(pUacSrcHandle->pAacDecHandler,
							(unsigned char *)input_buffer,
							(INT_PCM *)output_buffer,
							read_size,
							(int *)&s32OutLen);
				if (s32Ret != 0) {
					ERR_UAC_PRINT("aac decode faile\n");
					FileInSrcState = CVIUAC_FILEIN_SRC_CLOSE_FD_STATE;
				}
			}
			s32OutLen = s32OutLen * BYTES_PER_SAMPLE;
			if (s32OutLen > CVIUAC_FILEIN_SRC_OUTPUT_BUFFER_SIZE) {
				ERR_UAC_PRINT("[Fatal]AAC decode out buffer not large enough\n");
				break;
			}
			//send header info to cviuac_dst_context for  channel num/rate
			for (int dstIndex = 0; dstIndex < CVIAUDIO_DST_MAX_NUM; dstIndex++) {
			if (gstBindCfg[CVIAUDIO_SRC_FILE_IN][dstIndex] == true) {
				if (handle != NULL) {
					cviuac_context *pUacHandle = handle;
					cviuac_dst_context *pUacDstHandle = NULL;

					if (pUacHandle->cviuac_dst_handle[dstIndex] != NULL) {
					pUacDstHandle = pUacHandle->cviuac_dst_handle[dstIndex];
					pUacDstHandle->update_pcm_info.channels =
							pUacSrcHandle->stAacDecInfo.channel_num;
					pUacDstHandle->update_pcm_info.period_size = 480;
					pUacDstHandle->update_pcm_info.rate = pUacSrcHandle->stAacDecInfo.sample_rate;
					if (pFileParam->pfd_save != NULL && dstIndex == CVIAUDIO_DST_FILE_SAVE)
						pUacDstHandle->pfd_dst_save = pFileParam->pfd_save;

					} else {
						ERR_UAC_PRINT("DstPath[%d]not create with bind mode on..\n",
								dstIndex);
					}

				}

			}
			}


			//write data block mode in fifo
			for (int dstIndex = 0; dstIndex < CVIAUDIO_DST_MAX_NUM; dstIndex++) {
			if (gstBindCfg[CVIAUDIO_SRC_FILE_IN][dstIndex] == true) {
				bOutFiFoExist = true;
				//put data in the fifo
				pfifo = &gstCviaudioFifo[dstIndex];
				if (pfifo->pcviaudio_fifo_handle && pfifo->bEnable) {
				if (s32OutLen != 0) {
				buffer_writeBytes = CycleBufferWriteWait(pfifo->pcviaudio_fifo_handle,
									output_buffer,
									s32OutLen,
									1000);


				tmp = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
				if (tmp > (pfifo->s32fifo_size / 2))
					usleep(200*1000);//sleep 100ms for fifo level

				if (!buffer_writeBytes) {
					ERR_UAC_PRINT("\n");
					buffer_write_count++;
					usleep(1000 * 500);//sleep 500 ms
					if (buffer_write_count >= 5) {
						tmp = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
						ERR_UAC_PRINT("USB in fifo full AAC bufferlevel[%d] of [%d]\n",
						tmp, pfifo->s32fifo_size);
						sleep(1);
					}
				} else {
					//buffer ok
					buffer_write_count = 0;
					if (s32OutLen > buffer_writeBytes)
						DBG_UAC_PRINT("AAC decoded overrun buflevel[%d] bufsize[%d]\n",
								tmp, pfifo->s32fifo_size);
				}
				}
				} else {
					ERR_UAC_PRINT("File in src fifo not create for dstPath[%d]\n", dstIndex);
					sleep(1);
				}
			}
			}

			if (bOutFiFoExist == false) {
				sleep(1);
				printf("CVIUAC_FILEIN_SRC_DECODE_AAC_STATE fifo not exist yet\n");
			}
		}
		break;

		case CVIUAC_FILEIN_SRC_CLOSE_FD_STATE:
		{
			if (pFileParam->pfd != NULL)
				fclose(pFileParam->pfd);
			SAFE_FREE_BUF(input_buffer);
			SAFE_FREE_BUF(output_buffer);
			goto LEAVE_FILE;
		}
		break;

		default:
			ERR_UAC_PRINT("[Error]file in as src abnormal...[%d]\n", FileInSrcState);
			sleep(2);
			break;
		}
	}
LEAVE_FILE:
	DBG_UAC_PRINT("Leave file play\n");
	printf("[%s] close\n", __func__);
	return (void *)0;
}



static void *thread_src_usb_in_audio(void *arg)
{

#define CVIUAC_USBIN_SRC_OPEN_STATE 0x71
#define CVIUAC_USBIN_DETECT_STATE 0x72
#define CVIUAC_USBIN_SRC_UPDATE_TO_DIST_INFO 0x73
#define CVIUAC_USBIN_SEND_DATA_STATE 0x74
#define CVIUAC_USBIN_FORCE_FLUSH_STATE 0x75
#define CVIUAC_USBIN_CLOSE_AND_RETRY_STATE 0x76


	struct pcm_config stDefaultUsbInConfig = {
		.channels = 1,
		.rate = 48000,
		.period_size = AUDIO_PERIOD_SIZE,
		.period_count = CAP_PERIOD_COUNT,
		.format = PCM_FORMAT_S16_LE,
		.start_threshold = 0,
		.stop_threshold = INT_MAX,
	};
	UAC_UNUSED_REF(arg);
	UAC_UNUSED_REF(stDefaultUsbInConfig);

	ST_CVI_UAC_USB_PARAM *pUacSrcUsbParam = (ST_CVI_UAC_USB_PARAM *)arg;

	DBG_UAC_PRINT("usb in channel[%d] rate[%d] period_size[%d]\n",
		stDefaultUsbInConfig.channels,
		stDefaultUsbInConfig.rate,
		stDefaultUsbInConfig.period_size);

	int  err_read = -1;
	struct pcm *pcm_usb_in = NULL;
	char *pcm_read_buffer = NULL;
	int read_bytes = AUDIO_PERIOD_SIZE * BYTES_PER_SAMPLE * stDefaultUsbInConfig.channels;
	int UsbInSrcState = CVIUAC_USBIN_SRC_OPEN_STATE;
	int usb_in_src_StableCnt = 0;
	int usb_stable_send_cnt = 0;


	if (read_bytes != 0) {
		pcm_read_buffer = (char *)calloc(1, (read_bytes));
		if (pcm_read_buffer == NULL) {
			ERR_UAC_PRINT("FATAL error\n");
			return NULL;
		}
	} else {
		ERR_UAC_PRINT("FATAL error\n");
		return NULL;
	}

	while (gThreadEnable.bEnableSrc_UsbIn) {

		switch (UsbInSrcState) {
		case CVIUAC_USBIN_SRC_OPEN_STATE:
		{
			if (err_read != 0) {
				pcm_usb_in = pcm_open(UAC_SRC_USB_IN_CARD_ID,
							0,
							PCM_IN, &stDefaultUsbInConfig);
				if (!pcm_usb_in || !pcm_is_ready(pcm_usb_in)) {
					INFO_UAC_PRINT("Unable to open pcm_usb_in (%s)\n",
						pcm_get_error(pcm_usb_in));
					ERR_UAC_PRINT("\n");
					usleep(500*1000);

				} else {
					err_read = 0;
					DBG_UAC_PRINT("pcm_usb_in recordhandle success\n");
				}
			}

			UsbInSrcState = CVIUAC_USBIN_DETECT_STATE;
		}
		break;

		case CVIUAC_USBIN_DETECT_STATE:
		{
			err_read = pcm_read(pcm_usb_in, pcm_read_buffer, read_bytes);
			if (err_read == 0) {
				usb_in_src_StableCnt++;
				if (usb_in_src_StableCnt > USB_AS_SRC_STABLE_THRESHOLD) {
					DBG_UAC_PRINT("USB as in source stable\n");
					UsbInSrcState = CVIUAC_USBIN_SRC_UPDATE_TO_DIST_INFO;
					usb_in_src_StableCnt = 0;
				} else {
					UsbInSrcState = CVIUAC_USBIN_DETECT_STATE;
				}
			} else {
				usb_in_src_StableCnt = 0;
				UsbInSrcState = CVIUAC_USBIN_CLOSE_AND_RETRY_STATE;
				 sleep(1);
				 usleep(500*1000);
				 DBG_UAC_PRINT("DOWNLINK_SRC_DETECT_STATE failure ...go to retry\n");
			}

		}
		break;

		case CVIUAC_USBIN_SRC_UPDATE_TO_DIST_INFO:
		{
			UsbInSrcState = CVIUAC_USBIN_SEND_DATA_STATE;
			//update to cviuac_dst_context for  update_pcm_info
			for (int dstIndex = 0; dstIndex < CVIAUDIO_DST_MAX_NUM; dstIndex++) {
			if (gstBindCfg[CVIAUDIO_SRC_USB_IN][dstIndex] == true) {
				if (handle != NULL) {
					cviuac_context *pUacHandle = handle;
					cviuac_dst_context *pUacDstHandle = NULL;

					if (pUacHandle->cviuac_dst_handle[dstIndex] != NULL) {
					pUacDstHandle = pUacHandle->cviuac_dst_handle[dstIndex];
					pUacDstHandle->update_pcm_info.channels = stDefaultUsbInConfig.channels;
					pUacDstHandle->update_pcm_info.period_size = stDefaultUsbInConfig.period_size;
					pUacDstHandle->update_pcm_info.rate = stDefaultUsbInConfig.rate;
					if (pUacSrcUsbParam != NULL  &&
						pUacSrcUsbParam->pfd_usb_save != NULL &&
						(dstIndex == CVIAUDIO_DST_FILE_SAVE))
						pUacDstHandle->pfd_dst_save = pUacSrcUsbParam->pfd_usb_save;
					}

				}

			}
			}
		}
		break;
		case CVIUAC_USBIN_SEND_DATA_STATE:
		{
			err_read = pcm_read(pcm_usb_in, pcm_read_buffer, read_bytes);
			if (err_read != 0) {
				ERR_UAC_PRINT("pcm_usb_in not get data from usb src\n");
				ERR_UAC_PRINT("card [%d][%s]\n", UAC_SRC_USB_IN_CARD_ID,
							pcm_get_error(pcm_usb_in));
				INFO_UAC_PRINT("[pcm usb in  src]go close and retry....\n");
				sleep(1);
				usleep(500*1000);
				UsbInSrcState = CVIUAC_USBIN_CLOSE_AND_RETRY_STATE;
			} else {
				UsbInSrcState = CVIUAC_USBIN_SRC_UPDATE_TO_DIST_INFO;
				bool bOutFiFoExist = false;
				st_cviaudio_fifo_buffer *pfifo = NULL;
				int buffer_writeBytes = 0;
				int buffer_write_count = 0;

				for (int dstIndex = 0; dstIndex < CVIAUDIO_DST_MAX_NUM; dstIndex++) {
				if (gstBindCfg[CVIAUDIO_SRC_USB_IN][dstIndex] == true) {
					bOutFiFoExist = true;
					//put data in the fifo
					pfifo = &gstCviaudioFifo[dstIndex];
					if (pfifo->pcviaudio_fifo_handle && pfifo->bEnable) {
					buffer_writeBytes = CycleBufferWrite(pfifo->pcviaudio_fifo_handle,
										pcm_read_buffer, read_bytes);
					if (!buffer_writeBytes) {
					//buffer abnormal
					buffer_write_count++;
					usleep(1000*read_bytes/(64));
					if (buffer_write_count >= 10) {
						int tmp = 0;

						tmp = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
						ERR_UAC_PRINT("USB in fifo full bufferlevel[%d] of [%d]\n",
						tmp, pfifo->s32fifo_size);
					}
					} else {
					//buffer ok
					buffer_write_count = 0;
					usb_stable_send_cnt++;
					if (usb_stable_send_cnt % 20 == 0)
						UsbInSrcState = CVIUAC_USBIN_SRC_UPDATE_TO_DIST_INFO;
					}
					} else
						ERR_UAC_PRINT("fifo not create yet for dstPath[%d]\n", dstIndex);
				}
				}

				if (bOutFiFoExist == false)
					usleep(500 * 1000);
			}
		}
		break;

		case CVIUAC_USBIN_CLOSE_AND_RETRY_STATE:
		{
			DBG_UAC_PRINT("pcm_close pcm_recordhandle\n");
			if (pcm_usb_in) {
				pcm_close(pcm_usb_in);
				pcm_usb_in = NULL;
			}

			DBG_UAC_PRINT("USB_IN_SRC_CLOSE_AND_RETRY_STATE\n");
			sleep(1);
			UsbInSrcState = CVIUAC_USBIN_SRC_OPEN_STATE;
		}
		break;

		default:
			ERR_UAC_PRINT("[Error]usb in as src abnormal...[%d]\n", UsbInSrcState);
			sleep(2);
			break;
		}

	}

	SAFE_FREE_BUF(pcm_read_buffer);
	if (pcm_usb_in)
		pcm_close(pcm_usb_in);
	printf("[%s] close\n", __func__);


	return (void *)0;
}

static void *thread_dst_spk_out_audio(void *arg)
{
#define CVIAUDIO_UAC_DST_WAIT_INFO_STATE 0xE1
#define CVIAUDIO_UAC_DST_START_AO_STATE 0xE2
#define CVIAUDIO_UAC_DST_DETECT_AO_STATE 0xE3
#define CVIAUDIO_UAC_DST_CHECK_INFO_CHANGE_STATE 0xE4
#define CVIAUDIO_UAC_DST_SEND_AO_DATA_STATE 0xE5
#define CVIAUDIO_UAC_DST_SENDDATA_MUTE_STATE 0xE6
#define CVIAUDIO_UAC_DST_AO_CLOSE_AND_RETRY_STATE 0xE7
#define CVUADIO_UAC_SPK_OUT_STABLE_THRESHOLD 2


	UAC_UNUSED_REF(arg);
struct pcm_config stPcmPrevious = {
	.channels = 1,//CHANNEL_COUNT,//host chncnt = 1, guest chn count = 2
	.rate = 48000,
	.period_size = AUDIO_PERIOD_SIZE,
	.period_count = PLAY_PERIOD_COUNT,
	.format = PCM_FORMAT_S16_LE,
	.start_threshold = 0,
	.stop_threshold = INT_MAX,
};
	int err_write = -1;
	struct pcm *pcm_spk_out = NULL;
	int zero_buffer_size = 480 * 2 * 2;
	char *zero_buffer = (char *)malloc(zero_buffer_size);

	memset(zero_buffer, 0, zero_buffer_size);
	int SpkOutDstState = CVIAUDIO_UAC_DST_WAIT_INFO_STATE;
	cviuac_context *pUacHandle = handle;
	cviuac_dst_context *pUacDstHandle = NULL;
	int size_bytes = 0;
	int detect_spk_count = 0;
	char *pcm_writebuffer = (char *)malloc(zero_buffer_size);
	int ret_b;

	pUacDstHandle = pUacHandle->cviuac_dst_handle[CVIAUDIO_DST_SPK_OUT];
	if (pUacDstHandle == NULL)
		ERR_UAC_PRINT("Null pointer detect in spk out\n");

	while (gThreadEnable.bEnableDst_SpkOut) {
		switch (SpkOutDstState) {
		case CVIAUDIO_UAC_DST_WAIT_INFO_STATE:
		{
			if ((pUacDstHandle->update_pcm_info.rate != 0) &&
			(pUacDstHandle->update_pcm_info.channels != 0) &&
			(pUacDstHandle->update_pcm_info.period_size != 0)) {
				DBG_UAC_PRINT("Get the first info\n");
				SpkOutDstState = CVIAUDIO_UAC_DST_START_AO_STATE;
				DBG_UAC_PRINT("spk out..first rate[%d] channel[%d] period_size[%d]\n",
					pUacDstHandle->update_pcm_info.rate,
					pUacDstHandle->update_pcm_info.channels,
					pUacDstHandle->update_pcm_info.period_size);
				stPcmPrevious.channels = pUacDstHandle->update_pcm_info.channels;
				stPcmPrevious.period_size = pUacDstHandle->update_pcm_info.period_size;
				stPcmPrevious.rate = pUacDstHandle->update_pcm_info.rate;
				size_bytes = stPcmPrevious.channels * stPcmPrevious.period_size * 2;
				SAFE_FREE_BUF(pcm_writebuffer);
				pcm_writebuffer =  (char *)malloc(size_bytes);
			} else {
				usleep(500*1000);
				SpkOutDstState = CVIAUDIO_UAC_DST_WAIT_INFO_STATE;
			}

		}
		break;

		case CVIAUDIO_UAC_DST_START_AO_STATE:
		{
			if (err_write != 0) {
				pcm_spk_out = pcm_open(UAC_DST_SPK_OUT_CARD_ID,
							 0, PCM_OUT, &stPcmPrevious);
				if (!pcm_spk_out || !pcm_is_ready(pcm_spk_out)) {
					ERR_UAC_PRINT("Unable to open pcm_spk_out (%s)\n",
							pcm_get_error(pcm_spk_out));
					ERR_UAC_PRINT("\n");
					usleep(500*1000);
				} else {
					err_write = 0;
					DBG_UAC_PRINT("pcm_down_playhandle success\n");
					#if 0
					int  u32PeriodBytes =
					pcm_frames_to_bytes(pcm_spk_out, stPcmPrevious.period_size);

					printf("recommand size bytes..[%d]\n", u32PeriodBytes);
					#endif
				}
			} else {
				err_write = 0;
				DBG_UAC_PRINT("pcm_playhandle success\n");
				usleep(10*1000);
			}
			SpkOutDstState = CVIAUDIO_UAC_DST_DETECT_AO_STATE;
		}
		break;

		case CVIAUDIO_UAC_DST_DETECT_AO_STATE:
		{

			if (size_bytes > zero_buffer_size) {
				SAFE_FREE_BUF(zero_buffer);
				zero_buffer_size = size_bytes;
				zero_buffer = (char *)malloc(zero_buffer_size);
				memset(zero_buffer, 0, zero_buffer_size);
			}
			err_write = pcm_write(pcm_spk_out, zero_buffer, size_bytes);
			if (err_write != 0) {
				ERR_UAC_PRINT("pcm_spk_out[%s]\n", pcm_get_error(pcm_spk_out));
				detect_spk_count = 0;
				sleep(1);
				SpkOutDstState = CVIAUDIO_UAC_DST_AO_CLOSE_AND_RETRY_STATE;
			} else {
				detect_spk_count++;
				if (detect_spk_count > CVUADIO_UAC_SPK_OUT_STABLE_THRESHOLD) {
					DBG_UAC_PRINT("spk out stable\n");
					SpkOutDstState = CVIAUDIO_UAC_DST_SEND_AO_DATA_STATE;
					detect_spk_count = 0;
				} else {
					SpkOutDstState = CVIAUDIO_UAC_DST_DETECT_AO_STATE;
				}
			}
		}
		break;

		case CVIAUDIO_UAC_DST_CHECK_INFO_CHANGE_STATE:
		{
			if ((stPcmPrevious.rate != pUacDstHandle->update_pcm_info.rate) &&
				(pUacDstHandle->update_pcm_info.rate != 0)) {
				DBG_UAC_PRINT("audio info change rate[%d]->rate[%d]\n",
						stPcmPrevious.rate,
						pUacDstHandle->update_pcm_info.rate);
				//close and reopen
				err_write = -1;

				SpkOutDstState = CVIAUDIO_UAC_DST_WAIT_INFO_STATE;

				SAFE_FREE_BUF(pcm_writebuffer);
				DBG_UAC_PRINT("pcm_close pcm_spk_out\n");
				if (pcm_spk_out) {
					pcm_close(pcm_spk_out);
					pcm_spk_out = NULL;
				}
				pcm_writebuffer = (char *)malloc(size_bytes);

			} else {
				SpkOutDstState = CVIAUDIO_UAC_DST_SEND_AO_DATA_STATE;
			}
		}
		break;

		case CVIAUDIO_UAC_DST_SEND_AO_DATA_STATE:
		{
			st_cviaudio_fifo_buffer *pfifo = NULL;

			pfifo = &gstCviaudioFifo[CVIAUDIO_DST_SPK_OUT];
			if (pfifo->pcviaudio_fifo_handle && pfifo->bEnable) {


			ret_b = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
			if (ret_b >= size_bytes * 2) {
				//sufficient with buffering 2 frame in cycble buffer
				//send the true data

				ret_b = CycleBufferRead(pfifo->pcviaudio_fifo_handle,
							pcm_writebuffer,
							size_bytes);
				if (access("/tmp/spk_out_pcm", F_OK) == 0) {
					uac_dump_audiodata((char *)"/tmp/vincent_spk.pcm",
					(char *)pcm_writebuffer,
					(unsigned int)size_bytes);
				}

				if (ret_b != size_bytes) {
					//...cycle buffer abnormal
					DBG_UAC_PRINT("Error not enough in cycle buffer ...abnormal[%d]\n", __LINE__);
				}
				err_write = pcm_write(pcm_spk_out, pcm_writebuffer, size_bytes);

				if (err_write != 0) {
					ERR_UAC_PRINT("spk_out[%s]\n", pcm_get_error(pcm_spk_out));
					ERR_UAC_PRINT("Error pcm_write failure  dst(spk)...go retry\n");
					sleep(2);
					SpkOutDstState = CVIAUDIO_UAC_DST_AO_CLOSE_AND_RETRY_STATE;
				}  else {
					//send data success ...keep sending from cycle buffer
					SpkOutDstState = CVIAUDIO_UAC_DST_CHECK_INFO_CHANGE_STATE;
				}
			} else {
				//buffer not enough go to mute state
				SpkOutDstState = CVIAUDIO_UAC_DST_SENDDATA_MUTE_STATE;
			}

			} else {
				//cyclebuffer not ready
				SpkOutDstState = CVIAUDIO_UAC_DST_CHECK_INFO_CHANGE_STATE;
				sleep(1);
			}
		}
		break;

		case CVIAUDIO_UAC_DST_SENDDATA_MUTE_STATE:
		{
			st_cviaudio_fifo_buffer *pfifo = NULL;

			pfifo = &gstCviaudioFifo[CVIAUDIO_DST_SPK_OUT];

			if (size_bytes > zero_buffer_size) {
				SAFE_FREE_BUF(zero_buffer);
				zero_buffer_size = size_bytes;
				zero_buffer = (char *)malloc(zero_buffer_size);
				memset(zero_buffer, 0, zero_buffer_size);
			}

			err_write = pcm_write(pcm_spk_out, zero_buffer, size_bytes);
#if 0
				fwrite(zero_buffer, 1, size_bytes, pvincent);
#endif
			if (err_write != 0) {
				ERR_UAC_PRINT("Error pcm_write Mute failure in downlink dst(spk)...go retry\n");
				sleep(2);
				SpkOutDstState = CVIAUDIO_UAC_DST_AO_CLOSE_AND_RETRY_STATE;
			}  else {
				if (pfifo->pcviaudio_fifo_handle != NULL) {
					ret_b = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
					if (ret_b < size_bytes * 2) {
						//cycle buffer underrun
						SpkOutDstState = CVIAUDIO_UAC_DST_SENDDATA_MUTE_STATE;
					} else {
						//cycle buffer enough data
						//go to send data
						SpkOutDstState = CVIAUDIO_UAC_DST_SEND_AO_DATA_STATE;
					}
				} else {
					usleep(500 * 1000);
					SpkOutDstState = CVIAUDIO_UAC_DST_SENDDATA_MUTE_STATE;
					DBG_UAC_PRINT("cycle buffer fifo for CVIAUDIO_DST_SPK_OUT not ready\n");
				}
			}
		}
		break;

		case CVIAUDIO_UAC_DST_AO_CLOSE_AND_RETRY_STATE:
		{
			//close and reopen
			SAFE_FREE_BUF(pcm_writebuffer);
			if (err_write != 0) {
				DBG_UAC_PRINT("pcm_close pcm_spk_out\n");
				if  (pcm_spk_out) {
					pcm_close(pcm_spk_out);
					pcm_spk_out = NULL;
				}
			}
			pcm_writebuffer = (char *)malloc(size_bytes);
			usleep(500*1000);
			DBG_UAC_PRINT("CVIAUDIO_UAC_DST_AO_CLOSE_AND_RETRY_STATE\n");
			SpkOutDstState = CVIAUDIO_UAC_DST_WAIT_INFO_STATE;

		}
		break;

		default:
			ERR_UAC_PRINT("thread_dst_spkout_audio[%d]\n", SpkOutDstState);
			sleep(2);
			break;
		}
	}

	SAFE_FREE_BUF(zero_buffer);
	SAFE_FREE_BUF(pcm_writebuffer);
	if (pcm_spk_out) {
		pcm_close(pcm_spk_out);
		pcm_spk_out = NULL;
	}
	printf("[%s] close\n", __func__);
	return (void *)0;
}


typedef struct _ResState {
	bool bNeedResample;
	bool bResOn;
	int target_rate;
	int target_size;//for pcm_writebuffer size in bytes
	int target_channel;
	char *target_buffer;

	int source_rate;
	int source_size;//for pcm_writebuffer size in bytes
	int source_channel;
	char *source_buffer;

	void *ResampleFifo;
} ST_ResState;

static void _mono_mux_to_stereo(CVI_S16 *mono_buff,
				char **mux_buff,
				int mono_bytes)
{
	int i = 0;

	short *tmp = (short *)(*mux_buff);

	for (i = 0;  i < (mono_bytes / 2); i++) {
		tmp[2 * i] = mono_buff[i];
		tmp[2 * i + 1] = mono_buff[i];
	}
}



static void _update_ResState(struct pcm_config  *p_defaultUsbPcm,
			   struct pcm_config *p_updateUsbPcm,
			   ST_ResState *pstResState,
			   cviuac_dst_context *pUacDstHandle)
{
	CHECK_NULL_PTR_RET_NONE(p_defaultUsbPcm);
	CHECK_NULL_PTR_RET_NONE(p_updateUsbPcm);
	CHECK_NULL_PTR_RET_NONE(pstResState);
	CHECK_NULL_PTR_RET_NONE(pUacDstHandle);
	CHECK_NULL_PTR_RET_NONE(pUacDstHandle->stSampleResFun.pCVI_Resampler_Create);

	if (p_updateUsbPcm->rate != 0 &&
		p_defaultUsbPcm->rate != p_updateUsbPcm->rate) {
		pstResState->bNeedResample = true;
	} else
		pstResState->bNeedResample = false;

	if (!pstResState->bNeedResample)
		return;
	//Need to do the Resample-----start
	if (pstResState->bResOn == false) {

		pstResState->target_channel = p_defaultUsbPcm->channels;
		pstResState->target_rate = p_defaultUsbPcm->rate;
		pstResState->source_channel = p_updateUsbPcm->channels;
		pstResState->source_rate = p_updateUsbPcm->rate;
		if (pUacDstHandle->cviRes != NULL)
			pUacDstHandle->stSampleResFun.pCVI_Resampler_Destroy(pUacDstHandle->cviRes);

		pUacDstHandle->cviRes = pUacDstHandle->stSampleResFun.pCVI_Resampler_Create(
					pstResState->source_rate,
					pstResState->target_rate,
					pstResState->source_channel);

		pstResState->source_size = 480 * BYTES_PER_SAMPLE * pstResState->source_channel;
		pstResState->target_size = pUacDstHandle->stSampleResFun.pCVI_Resampler_GetMaxOutputNum(
						pUacDstHandle->cviRes, pstResState->source_size);

		SAFE_FREE_BUF(pstResState->target_buffer);
		SAFE_FREE_BUF(pstResState->source_buffer);
		pstResState->target_buffer = (char *)malloc(pstResState->target_size);
		pstResState->source_buffer = (char *)malloc(pstResState->source_size);
		if (pstResState->ResampleFifo != NULL) {
			CycleBufferDestory(pstResState->ResampleFifo);
			pstResState->ResampleFifo = NULL;
		}
		CycleBufferInit(&pstResState->ResampleFifo,
				CVIAUDIO_UAC_FIFO_SIZE_RESAMPLE);//48K

		DBG_UAC_PRINT("First trigger resampler ------------------------>\n");
		pstResState->bResOn = true;
	} else {
		//bResOn == true
		if (((int)p_updateUsbPcm->channels != (int)pstResState->source_channel) ||
		     ((int)p_updateUsbPcm->rate != (int)pstResState->source_rate)) {
			//resample is need, but is different from previous resample
			//need retrigger resampler
			pstResState->target_channel = p_defaultUsbPcm->channels;
			pstResState->target_rate = p_defaultUsbPcm->rate;
			pstResState->source_channel = p_updateUsbPcm->channels;
			pstResState->source_rate = p_updateUsbPcm->rate;
			if (pUacDstHandle->cviRes != NULL)
				pUacDstHandle->stSampleResFun.pCVI_Resampler_Destroy(pUacDstHandle->cviRes);

			pUacDstHandle->cviRes = pUacDstHandle->stSampleResFun.pCVI_Resampler_Create(
						pstResState->source_rate,
						pstResState->target_rate,
						pstResState->source_channel);

			pstResState->source_size = 480 * BYTES_PER_SAMPLE * pstResState->source_channel;
			pstResState->target_size = pUacDstHandle->stSampleResFun.pCVI_Resampler_GetMaxOutputNum(
							pUacDstHandle->cviRes, pstResState->source_size);

			SAFE_FREE_BUF(pstResState->target_buffer);
			SAFE_FREE_BUF(pstResState->source_buffer);
			pstResState->target_buffer = (char *)malloc(pstResState->target_size);
			pstResState->source_buffer = (char *)malloc(pstResState->source_size);
			if (pstResState->ResampleFifo != NULL) {
				CycleBufferDestory(pstResState->ResampleFifo);
				pstResState->ResampleFifo = NULL;
			}
			CycleBufferInit(&pstResState->ResampleFifo,
					CVIAUDIO_UAC_FIFO_SIZE * 4);//48K

			pstResState->bResOn = true;
			DBG_UAC_PRINT("Retrigger resampler ------------------------>\n");


		}
	}


}


static void *thread_dst_usb_out_audio(void *arg)
{

#define CVIUAC_DST_USB_OUT_WAIT_INFO_STATE 0x91
#define CVIUAC_DST_USB_OUT_OPEN_STATE 0x92
#define CVIUAC_DST_USB_OUT_DETECT_STATE 0x93
#define CVIUAC_DST_USB_OUT_INFO_CHANGE_STATE 0x94
#define CVIUAC_DST_USB_OUT_SEND_DATA_STATE 0x95
#define CVIUAC_DST_USB_OUT_SEND_RESAMPLE_DATA 0x96
#define CVIUAC_DST_USB_OUT_SEND_DATA_MUTE_STATE 0x97
#define CVIUAC_DST_USB_OUT_CLOSE_AND_RETRY_STATE 0x98
#define CVIUAC_DST_USB_OUT_ERROR_AND_FORCE_LEAVE 0xFF
#define  USB_AS_SPK_STABLE_THRESHOLD 2

struct pcm_config stDefaultUsbOutConfig = {
	.channels = 2,
	.rate = 48000,
	.period_size = AUDIO_PERIOD_SIZE,
	.period_count = PLAY_PERIOD_COUNT,
	.format = PCM_FORMAT_S16_LE,
	.start_threshold = 0,
	.stop_threshold = INT_MAX,
};

struct pcm_config stPcmUpdate = {
	.channels = 2,
	.rate = 48000,
	.period_size = AUDIO_PERIOD_SIZE,
	.period_count = PLAY_PERIOD_COUNT,
	.format = PCM_FORMAT_S16_LE,
	.start_threshold = 0,
	.stop_threshold = INT_MAX,
};
	UAC_UNUSED_REF(arg);
	UAC_UNUSED_REF(stDefaultUsbOutConfig);

	int err_write = -1;
	struct pcm *pcm_usb_out = NULL;
	int zero_buffer_size = AUDIO_PERIOD_SIZE * 2 * BYTES_PER_SAMPLE;
	char *zero_buffer = (char *)malloc(zero_buffer_size);
	int fix_write_size = zero_buffer_size;
	char *pcm_write_buffer = (char *)malloc(fix_write_size);
	int ret_b;
	cviuac_context *pUacHandle = handle;
	cviuac_dst_context *pUacDstHandle = NULL;
	ST_ResState stResState;
	int UsbOutDstState = CVIUAC_DST_USB_OUT_WAIT_INFO_STATE;
	int detect_usb_count = 0;
	int mono_size = 0;
	char *mux_buffer = NULL;
	int mux_buffer_size = 0;

	CHECK_NULL_PTR_RET(pUacHandle, (void *)0);
	memset(zero_buffer, 0, zero_buffer_size);
	memset(&stResState, 0, sizeof(ST_ResState));
	pUacDstHandle = pUacHandle->cviuac_dst_handle[CVIAUDIO_DST_USB_OUT];
	if (pUacDstHandle == NULL)
		ERR_UAC_PRINT("Null pointer detect in usb_out\n");

	while (gThreadEnable.bEnableDst_UsbOut) {
		switch (UsbOutDstState) {
		case CVIUAC_DST_USB_OUT_WAIT_INFO_STATE:
		{
			if ((pUacDstHandle->update_pcm_info.rate != 0) &&
			(pUacDstHandle->update_pcm_info.channels != 0) &&
			(pUacDstHandle->update_pcm_info.period_size != 0)) {
				DBG_UAC_PRINT("Get the first info\n");

				DBG_UAC_PRINT("USB out..first rate[%d] channel[%d] period_size[%d]\n",
					pUacDstHandle->update_pcm_info.rate,
					pUacDstHandle->update_pcm_info.channels,
					pUacDstHandle->update_pcm_info.period_size);
				stPcmUpdate.channels = pUacDstHandle->update_pcm_info.channels;
				stPcmUpdate.period_size = pUacDstHandle->update_pcm_info.period_size;
				stPcmUpdate.rate = pUacDstHandle->update_pcm_info.rate;

				_update_ResState(&stDefaultUsbOutConfig,
						&stPcmUpdate,
						&stResState,
						pUacDstHandle);
				if (pcm_usb_out == NULL)
					UsbOutDstState = CVIUAC_DST_USB_OUT_OPEN_STATE;
				else
					UsbOutDstState = CVIUAC_DST_USB_OUT_DETECT_STATE;

			} else {
				usleep(500*1000);
				UsbOutDstState = CVIUAC_DST_USB_OUT_WAIT_INFO_STATE;
			}
		}
		break;

		case CVIUAC_DST_USB_OUT_OPEN_STATE:
		{
			if (err_write != 0) {
				pcm_usb_out = pcm_open(UAC_DST_USB_OUT_CARD_ID
							, 0, PCM_OUT, &stDefaultUsbOutConfig);
				if (!pcm_usb_out || !pcm_is_ready(pcm_usb_out)) {
					ERR_UAC_PRINT("Unable to open pcm_usb_out (%s)\n",
							pcm_get_error(pcm_usb_out));
					ERR_UAC_PRINT("\n");
					usleep(500*1000);
					//return 0;
					//force return for the open failure
				} else {
					err_write = 0;
					DBG_UAC_PRINT("pcm_usb_out success rate[%d] chn[%d]\n",
							stDefaultUsbOutConfig.rate,
							stDefaultUsbOutConfig.channels);
				}
			}
			UsbOutDstState = CVIUAC_DST_USB_OUT_DETECT_STATE;
		}
		break;

		case CVIUAC_DST_USB_OUT_DETECT_STATE:
		{
			err_write = pcm_write(pcm_usb_out, zero_buffer, zero_buffer_size);
			if (err_write != 0) {
				INFO_UAC_PRINT("usb out failure\n");
				sleep(2);
				detect_usb_count = 0;
				UsbOutDstState = CVIUAC_DST_USB_OUT_CLOSE_AND_RETRY_STATE;
				err_write = -1;
			} else {
				detect_usb_count++;
				if (detect_usb_count > USB_AS_SPK_STABLE_THRESHOLD) {
					DBG_UAC_PRINT("USB as usb out  .....ok[%d]\n", detect_usb_count);
					UsbOutDstState = CVIUAC_DST_USB_OUT_SEND_DATA_STATE;
					detect_usb_count = 0;
				} else {
					UsbOutDstState = CVIUAC_DST_USB_OUT_DETECT_STATE;
				}
			}
		}
		break;

		case CVIUAC_DST_USB_OUT_INFO_CHANGE_STATE:
		{
			if ((stPcmUpdate.rate != pUacDstHandle->update_pcm_info.rate) &&
				(pUacDstHandle->update_pcm_info.rate != 0)) {
				DBG_UAC_PRINT("audio info change rate[%d]->rate[%d]\n",
						stPcmUpdate.rate,
						pUacDstHandle->update_pcm_info.rate);

				UsbOutDstState = CVIUAC_DST_USB_OUT_WAIT_INFO_STATE;

			} else {
				UsbOutDstState = CVIUAC_DST_USB_OUT_SEND_DATA_STATE;
			}
		}
		break;

		case CVIUAC_DST_USB_OUT_SEND_DATA_STATE:
		{
			st_cviaudio_fifo_buffer *pfifo = NULL;

			pfifo = &gstCviaudioFifo[CVIAUDIO_DST_USB_OUT];
			if (pfifo->pcviaudio_fifo_handle && pfifo->bEnable) {
			if (stResState.bNeedResample == false) {

			ret_b = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
			if (ret_b >= fix_write_size * 2) {
				//sufficient with buffering 2 frame in cycble buffer
				//send the true data

				ret_b = CycleBufferRead(pfifo->pcviaudio_fifo_handle,
							pcm_write_buffer,
							fix_write_size);
				if (access("/tmp/usb_out_pcm", F_OK) == 0) {
					uac_dump_audiodata((char *)"/tmp/usb_out.pcm",
					(char *)pcm_write_buffer,
					(unsigned int)fix_write_size);
				}

				if (ret_b != fix_write_size) {
					//...cycle buffer abnormal
					DBG_UAC_PRINT("Error not enough in cycle buffer ...abnormal[%d]\n", __LINE__);
				}
				err_write = pcm_write(pcm_usb_out, pcm_write_buffer, fix_write_size);

				if (err_write != 0) {
					ERR_UAC_PRINT("usb_out[%s]\n", pcm_get_error(pcm_usb_out));
					ERR_UAC_PRINT("Error pcm_write failure  dst(usb_out)...go retry\n");
					sleep(2);
					err_write = -1;
					UsbOutDstState = CVIUAC_DST_USB_OUT_CLOSE_AND_RETRY_STATE;
				}  else {
					//send data success ...keep sending from cycle buffer
					UsbOutDstState = CVIUAC_DST_USB_OUT_INFO_CHANGE_STATE;
				}
			} else {
				//buffer not enough go to mute state
				UsbOutDstState = CVIUAC_DST_USB_OUT_SEND_DATA_MUTE_STATE;
			}

			} else {
				//resample is needed when fifo on
				UsbOutDstState = CVIUAC_DST_USB_OUT_SEND_RESAMPLE_DATA;

			}
			} else {
				//cyclebuffer not ready
				UsbOutDstState = CVIUAC_DST_USB_OUT_INFO_CHANGE_STATE;
				sleep(1);
			}
		}
		break;

		case CVIUAC_DST_USB_OUT_SEND_RESAMPLE_DATA:
		{
			st_cviaudio_fifo_buffer *pfifo = NULL;
			int samples_per_frame = 0;
			int u32OutSample = 0;
			int write_in_size = 0;
			int write_in_check = 0;
			int resample_fifo_level = 0;
			//error handle check 1
			pfifo = &gstCviaudioFifo[CVIAUDIO_DST_USB_OUT];
			if (!pfifo->pcviaudio_fifo_handle || !pfifo->bEnable) {
				ERR_UAC_PRINT("Wrong state of USB_OUT\n");
				UsbOutDstState = CVIUAC_DST_USB_OUT_ERROR_AND_FORCE_LEAVE;
				break;
			}
			//error handle check 2
			if (stResState.bNeedResample == true) {
				if (stResState.bResOn == false) {
					ERR_UAC_PRINT("Need resample but resample not On\n");
					UsbOutDstState = CVIUAC_DST_USB_OUT_ERROR_AND_FORCE_LEAVE;
					break;

				} else {
					if (stResState.ResampleFifo == NULL) {
					ERR_UAC_PRINT("\n");
					UsbOutDstState = CVIUAC_DST_USB_OUT_ERROR_AND_FORCE_LEAVE;
					break;
					}
				}
			}
			//do the resample
			samples_per_frame = (stResState.source_size / BYTES_PER_SAMPLE) / stResState.source_channel;
			ret_b = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
			resample_fifo_level = CycleBufferDataLen(stResState.ResampleFifo);
			if  ((ret_b >= stResState.source_size) &&
				(resample_fifo_level < CVIAUDIO_UAC_FIFO_THRESHOLD_RESAMPLE)) {
				//enough data and resample fifo to do the resample
				ret_b = CycleBufferRead(pfifo->pcviaudio_fifo_handle,
							stResState.source_buffer,
							stResState.source_size);
				if (ret_b != stResState.source_size)
					ERR_UAC_PRINT("\n");


				u32OutSample = pUacDstHandle->stSampleResFun.pCVI_Resampler_Process(
						pUacDstHandle->cviRes,
						(CVI_S16 *)stResState.source_buffer,
						samples_per_frame,
						(CVI_S16 *)stResState.target_buffer);
				write_in_size = u32OutSample * BYTES_PER_SAMPLE * stResState.source_channel;
				write_in_check = CycleBufferWrite(stResState.ResampleFifo,
						stResState.target_buffer,
						write_in_size);

				if (write_in_check != write_in_size)
					ERR_UAC_PRINT("\n");
			}
			//read out the resample data and pcm_write
			ret_b = CycleBufferDataLen(stResState.ResampleFifo);
			if (ret_b >= fix_write_size * 2) {
				//sufficient with buffering 2 frame in cycble buffer
				//send the true data
				if (stResState.source_channel == 1 &&
					stResState.target_channel == 2) {
					//mux mono to stereo
					mono_size = fix_write_size / 2;
					ret_b = CycleBufferRead(stResState.ResampleFifo,
								pcm_write_buffer,
								mono_size);
					if (access("/tmp/usb_out_res_mono", F_OK) == 0) {
						uac_dump_audiodata((char *)"/tmp/usb_res_mono.pcm",
						(char *)pcm_write_buffer,
						(unsigned int)mono_size);
					}

					if (ret_b != mono_size) {
						//...cycle buffer abnormal
						ERR_UAC_PRINT("Error not enough data[%d]\n", __LINE__);
					}
					if (mux_buffer == NULL || (fix_write_size > mux_buffer_size)) {
						SAFE_FREE_BUF(mux_buffer);
						mux_buffer_size = fix_write_size;
						mux_buffer = calloc(1, (mux_buffer_size));
					}

					_mono_mux_to_stereo((CVI_S16 *)pcm_write_buffer,
								&mux_buffer,
								mono_size);
					if (access("/tmp/usb_out_res_mux", F_OK) == 0) {
						uac_dump_audiodata((char *)"/tmp/usb_res_mux.pcm",
						(char *)mux_buffer,
						(unsigned int)fix_write_size);
					}
					err_write = pcm_write(pcm_usb_out, mux_buffer, fix_write_size);

				} else {
				ret_b = CycleBufferRead(stResState.ResampleFifo,
								pcm_write_buffer,
								fix_write_size);
				if (access("/tmp/usb_out_res_pcm", F_OK) == 0) {
					uac_dump_audiodata((char *)"/tmp/usb_res_out.pcm",
					(char *)pcm_write_buffer,
					(unsigned int)fix_write_size);
				}

				if (ret_b != fix_write_size) {
					//...cycle buffer abnormal
					ERR_UAC_PRINT("Error not enough in cycle buffer ...abnormal[%d]\n", __LINE__);
				}
				err_write = pcm_write(pcm_usb_out, pcm_write_buffer, fix_write_size);
				}

				if (err_write != 0) {
					ERR_UAC_PRINT("usb_out[%s]\n", pcm_get_error(pcm_usb_out));
					ERR_UAC_PRINT("usb_out[%d][%d]\n", stResState.source_channel,
									stResState.target_channel);
					ERR_UAC_PRINT("Error pcm_write failure  dst(usb_out)...go retry\n");
					sleep(2);
					err_write = -1;
					UsbOutDstState = CVIUAC_DST_USB_OUT_CLOSE_AND_RETRY_STATE;
				}  else {
					//send data success ...keep sending from cycle buffer
					UsbOutDstState = CVIUAC_DST_USB_OUT_INFO_CHANGE_STATE;
				}
			} else {
				//buffer not enough go to mute state
				UsbOutDstState = CVIUAC_DST_USB_OUT_SEND_DATA_MUTE_STATE;
			}

		}
		break;

		case CVIUAC_DST_USB_OUT_SEND_DATA_MUTE_STATE:
		{
			st_cviaudio_fifo_buffer *pfifo = NULL;

			pfifo = &gstCviaudioFifo[CVIAUDIO_DST_USB_OUT];
			int target_threshold = 0;

			err_write = pcm_write(pcm_usb_out, zero_buffer, fix_write_size);
			if (err_write != 0) {
				ERR_UAC_PRINT("Error pcm_write Mute failure in usb_out...go retry\n");
				sleep(2);
				err_write = -1;
				UsbOutDstState = CVIUAC_DST_USB_OUT_CLOSE_AND_RETRY_STATE;
			}  else {
				if (pfifo->pcviaudio_fifo_handle != NULL) {
					ret_b = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
					if (stResState.bNeedResample)
						target_threshold = stResState.source_size;
					else
						target_threshold = fix_write_size;

					if (ret_b < target_threshold * 2) {
						//cycle buffer underrun
						UsbOutDstState = CVIUAC_DST_USB_OUT_SEND_DATA_MUTE_STATE;
					} else {
						//cycle buffer enough data
						//go to send data
						UsbOutDstState = CVIUAC_DST_USB_OUT_SEND_DATA_STATE;
					}
				} else {
					usleep(500 * 1000);
					UsbOutDstState = CVIAUDIO_UAC_DST_SENDDATA_MUTE_STATE;
					DBG_UAC_PRINT("cycle buffer fifo for CVIAUDIO_DST_USB_OUT not ready\n");
				}
			}
		}
		break;

		case CVIUAC_DST_USB_OUT_CLOSE_AND_RETRY_STATE:
		{
			//close and reopen
			SAFE_FREE_BUF(pcm_write_buffer);
			if (err_write != 0) {
				DBG_UAC_PRINT("pcm_close pcm_usb_out\n");
				if (pcm_usb_out) {
					pcm_close(pcm_usb_out);
					pcm_usb_out = NULL;
				}
			}
			pcm_write_buffer = (char *)malloc(fix_write_size);
			usleep(500*1000);
			DBG_UAC_PRINT("CVIAUDIO_UAC_DST_AO_CLOSE_AND_RETRY_STATE\n");
			UsbOutDstState = CVIUAC_DST_USB_OUT_WAIT_INFO_STATE;
		}
		break;

		default:
			sleep(1);
			ERR_UAC_PRINT("thread_dst_usb_out_audio ERR_state[%d]\n", UsbOutDstState);
			break;
		}
	}

	SAFE_FREE_BUF(zero_buffer);
	SAFE_FREE_BUF(pcm_write_buffer);
	SAFE_FREE_BUF(mux_buffer);
	if (pcm_usb_out) {
		pcm_close(pcm_usb_out);
		pcm_usb_out = NULL;
	}
	SAFE_FREE_BUF(stResState.ResampleFifo);
	SAFE_FREE_BUF(stResState.source_buffer);
	SAFE_FREE_BUF(stResState.target_buffer);
	printf("[%s] close\n", __func__);
	return (void *)0;
}

static void *thread_dst_file_save_audio(void *arg)
{
	UAC_UNUSED_REF(arg);

#define CVIUAC_DST_FILE_SAVE_WAIT_INFO_STATE 0xF1
#define CVIUAC_DST_FILE_SAVE_SAVING_DATA_STATE 0xF2
#define CVIUAC_DST_FILE_SAVE_FINISHED_STATE 0xF3
	int FileSaveDstState = CVIUAC_DST_FILE_SAVE_WAIT_INFO_STATE;
	cviuac_context *pUacHandle = handle;
	cviuac_dst_context *pUacDstHandle = NULL;

	CHECK_NULL_PTR_RET(pUacHandle, (void *)0);
	pUacDstHandle = pUacHandle->cviuac_dst_handle[CVIAUDIO_DST_FILE_SAVE];
	CHECK_NULL_PTR_RET(pUacDstHandle, (void *)0);
	int wait_cnt = 1000;
#define CVIUAC_FILESAVE_READ_BUFFER_SIZE (1024*10)
	char *file_buffer = (char *)malloc(CVIUAC_FILESAVE_READ_BUFFER_SIZE);
	int no_data_count = 0;

	while (gThreadEnable.bEnableDst_FileSave) {
		switch (FileSaveDstState) {
		case CVIUAC_DST_FILE_SAVE_WAIT_INFO_STATE:
		{
			if (pUacDstHandle->pfd_dst_save == NULL) {
				if (wait_cnt--) {
					usleep(1000);
					FileSaveDstState = CVIUAC_DST_FILE_SAVE_WAIT_INFO_STATE;
				} else {
					pUacDstHandle->pfd_dst_save = fopen("/tmp/savefile.raw", "wb+");
					CHECK_NULL_PTR_RET(pUacDstHandle->pfd_dst_save, (void *)0);
				}


			} else
				FileSaveDstState = CVIUAC_DST_FILE_SAVE_SAVING_DATA_STATE;

		}
		break;

		case CVIUAC_DST_FILE_SAVE_SAVING_DATA_STATE:
		{
			FileSaveDstState = CVIUAC_DST_FILE_SAVE_SAVING_DATA_STATE;

			st_cviaudio_fifo_buffer *pfifo = NULL;
			int read_bytes = 0;
			int read_out = 0;

			pfifo = &gstCviaudioFifo[CVIAUDIO_DST_FILE_SAVE];
			if (pfifo->pcviaudio_fifo_handle && pfifo->bEnable) {
				read_bytes = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
				if (read_bytes > 0) {
					if (read_bytes > CVIUAC_FILESAVE_READ_BUFFER_SIZE)
						read_bytes = CVIUAC_FILESAVE_READ_BUFFER_SIZE;

					read_out = CycleBufferRead(pfifo->pcviaudio_fifo_handle,
							file_buffer,
							read_bytes);
					if (read_out != read_bytes)
						ERR_UAC_PRINT("\n");

					if (access("/tmp/mic_in_save", F_OK) == 0) {
						uac_dump_audiodata((char *)"/tmp/mic_in_save.pcm",
						(char *)file_buffer,
						(unsigned int)read_out);
					}
					fwrite(file_buffer, 1, read_out,
						pUacDstHandle->pfd_dst_save);
					no_data_count = 0;
				} else if (read_bytes == 0) {
					if (no_data_count > 30) {
						DBG_UAC_PRINT("dst file save 3 seconds no data\n");
						FileSaveDstState = CVIUAC_DST_FILE_SAVE_FINISHED_STATE;
					}

					no_data_count++;
					usleep(100*1000);
				}
			} else {
				//cyclebuffer not ready
				usleep(100*1000);
			}
		}
		break;

		case CVIUAC_DST_FILE_SAVE_FINISHED_STATE:
		{
			if (pUacDstHandle->pfd_dst_save != NULL)
				fclose(pUacDstHandle->pfd_dst_save);
			SAFE_FREE_BUF(file_buffer);
		}
		break;

		default:
			ERR_UAC_PRINT("thread_dst_file_save_audio[%d]\n", FileSaveDstState);
			sleep(2);
			break;
		}

	}

	if (pUacDstHandle->pfd_dst_save != NULL)
		fclose(pUacDstHandle->pfd_dst_save);
	SAFE_FREE_BUF(file_buffer);


	printf("[%s] close\n", __func__);
	return (void *)0;
}

static void *thread_src_mic_in_audio(void *arg)
{

#define CVIUAC_SRC_MICIN_OPEN_STATE 0x61
#define CVIUAC_SRC_MICIN_UPDATE_TO_DST_INFO 0x62
#define CVIUAC_SRC_MICIN_SEND_DATA_STATE 0x63
#define CVIUAC_SRC_MICIN_CLOSE_AND_RETRY_STATE 0x64
	ST_CVI_UAC_MIC_IN_PARAM *pUacMicInParam = (ST_CVI_UAC_MIC_IN_PARAM *)arg;
	struct pcm_config stDefaultMicInConfig = {
		.channels = 1,
		.rate = 48000,
		.period_size = AUDIO_PERIOD_SIZE,
		.period_count = CAP_PERIOD_COUNT,
		.format = PCM_FORMAT_S16_LE,
		.start_threshold = 0,
		.stop_threshold = INT_MAX,
	};

	stDefaultMicInConfig.channels = pUacMicInParam->channels;
	stDefaultMicInConfig.rate = pUacMicInParam->rate;
	stDefaultMicInConfig.period_size = pUacMicInParam->period_size;
	DBG_UAC_PRINT("mic in channel[%d] rate[%d] period_size[%d]\n",
		stDefaultMicInConfig.channels,
		stDefaultMicInConfig.rate,
		stDefaultMicInConfig.period_size);

	int err_read = 0;
	struct pcm *pcm_mic_in = NULL;
	char *pcm_read_buffer = NULL;
	int read_bytes = stDefaultMicInConfig.period_size*BYTES_PER_SAMPLE*stDefaultMicInConfig.channels;
	int MicInSrcState = CVIUAC_SRC_MICIN_OPEN_STATE;
	int tmp = 0;
	int pcm_open_retry = 0;
	int buffer_write_count = 0;

	if (read_bytes != 0) {
		pcm_read_buffer = (char *)malloc(read_bytes);
		if (pcm_read_buffer == NULL) {
			ERR_UAC_PRINT("FATAL error\n");
			return NULL;
		}
	} else {
		ERR_UAC_PRINT("FATAL error\n");
		return NULL;
	}

	while (gThreadEnable.bEnableSrc_MicIn) {
		switch (MicInSrcState) {
		case CVIUAC_SRC_MICIN_OPEN_STATE:
		{
			pcm_mic_in = pcm_open(UAC_SRC_MIC_IN_CARD_ID,
						0,
						PCM_IN,
						&stDefaultMicInConfig);
			if (!pcm_mic_in || !pcm_is_ready(pcm_mic_in)) {
				ERR_UAC_PRINT("Unable to pcm_mic_in (%s)\n",
						pcm_get_error(pcm_mic_in));
				ERR_UAC_PRINT("\n");
				usleep(500*1000);
				sleep(1);
				//force return for the open failure;
				ERR_UAC_PRINT("pcm_mic_in failure...retry\n");
				pcm_open_retry++;
				if (pcm_open_retry > 10) {
					ERR_UAC_PRINT("retry pcm_open mic in card[%d]dev[0]..failure\n",
						UAC_SRC_MIC_IN_CARD_ID);
					break;
				}
				MicInSrcState = CVIUAC_SRC_MICIN_CLOSE_AND_RETRY_STATE;
			} else {
				DBG_UAC_PRINT("pcm_mic_in success\n");
				pcm_open_retry = 0;
				MicInSrcState = CVIUAC_SRC_MICIN_UPDATE_TO_DST_INFO;
			}
		}
		break;

		case CVIUAC_SRC_MICIN_UPDATE_TO_DST_INFO:
		{


			MicInSrcState = CVIUAC_SRC_MICIN_SEND_DATA_STATE;
			//update to cviuac_dst_context for  update_pcm_info
			for (int dstIndex = 0; dstIndex < CVIAUDIO_DST_MAX_NUM; dstIndex++) {
			if (gstBindCfg[CVIAUDIO_SRC_MIC_IN][dstIndex] == true) {
				if (handle != NULL) {
					cviuac_context *pUacHandle = handle;
					cviuac_dst_context *pUacDstHandle = NULL;

					if (pUacHandle->cviuac_dst_handle[dstIndex] != NULL) {
					pUacDstHandle = pUacHandle->cviuac_dst_handle[dstIndex];
					pUacDstHandle->update_pcm_info.channels = stDefaultMicInConfig.channels;
					pUacDstHandle->update_pcm_info.period_size = stDefaultMicInConfig.period_size;
					pUacDstHandle->update_pcm_info.rate = stDefaultMicInConfig.rate;
					if (pUacMicInParam->pfd_MicInSave != NULL && dstIndex == CVIAUDIO_DST_FILE_SAVE)
						pUacDstHandle->pfd_dst_save = pUacMicInParam->pfd_MicInSave;
					}

				}

			}
			}

		}
		break;


		case CVIUAC_SRC_MICIN_SEND_DATA_STATE:
		{
			//check the pcm_read
			err_read = pcm_read(pcm_mic_in, pcm_read_buffer, read_bytes);

			if (err_read == 0) {
				MicInSrcState = CVIUAC_SRC_MICIN_UPDATE_TO_DST_INFO;
				//check the bind and send to fifo
				//check the bind and send to fifo
				bool bOutFiFoExist = false;
				st_cviaudio_fifo_buffer *pfifo = NULL;
				int buffer_writeBytes = 0;
				int level = 0;

				for (int dstIndex = 0; dstIndex < CVIAUDIO_DST_MAX_NUM; dstIndex++) {
				if (gstBindCfg[CVIAUDIO_SRC_MIC_IN][dstIndex] == true) {
					bOutFiFoExist = true;
					//put data in the fifo
					pfifo = &gstCviaudioFifo[dstIndex];
					if (pfifo->pcviaudio_fifo_handle) {
					buffer_writeBytes = CycleBufferWriteWait(pfifo->pcviaudio_fifo_handle,
										pcm_read_buffer, read_bytes, 500);

					level = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
					if (buffer_writeBytes != read_bytes) {
						ERR_UAC_PRINT("writein[%d] trueWriteIn[%d] level[%d] size[%d]\n",
						read_bytes, buffer_writeBytes, level, pfifo->s32fifo_size);
					} else {
						//no data will missing in the fifo
					}
					if (access("/tmp/mic_in", F_OK) == 0) {
						uac_dump_audiodata((char *)"/tmp/mic_in.pcm",
						(char *)pcm_read_buffer,
						(unsigned int)buffer_writeBytes);
					}

					if (!buffer_writeBytes) {
					//buffer abnormal
					buffer_write_count++;
					usleep(1000*10);
					if (buffer_write_count >= 5) {
						tmp = CycleBufferDataLen(pfifo->pcviaudio_fifo_handle);
						ERR_UAC_PRINT("Mic in fifo full bufferlevel[%d] of [%d]\n",
						tmp, pfifo->s32fifo_size);
					}
					} else {
						//buffer ok
						buffer_write_count = 0;
					}

					} else
						ERR_UAC_PRINT("fifo not create yet for dstPath[%d]\n", dstIndex);
				}
				}

				if (bOutFiFoExist == false)
					usleep(250 * 1000);

			} else {
				ERR_UAC_PRINT("CVIUAC_SRC_MICIN_SEND_DATA_STATE(mic in)  failure ...go to retry\n");
				MicInSrcState = CVIUAC_SRC_MICIN_CLOSE_AND_RETRY_STATE;
			}

		}
		break;

		case CVIUAC_SRC_MICIN_CLOSE_AND_RETRY_STATE:
		{
			DBG_UAC_PRINT("pcm_mic_in close and retry\n");
			sleep(2);
			if (pcm_mic_in) {
				pcm_close(pcm_mic_in);
				pcm_mic_in = NULL;
			}
			MicInSrcState = CVIUAC_SRC_MICIN_OPEN_STATE;
		}
		break;

		default:
			ERR_UAC_PRINT("thread_src_mic_in_audio[%d]\n", MicInSrcState);
			sleep(2);
			break;
		}
	}


	SAFE_FREE_BUF(pcm_read_buffer);
	if (pcm_mic_in) {
		pcm_close(pcm_mic_in);
		pcm_mic_in = NULL;
	}
	printf("[%s] close\n", __func__);

	return (void *)0;
}

//-----uac static function---------end

int cviaudio_uac_init(void)
{
	cviuac_context *pUacHandle;

	printf("init...easy uac audio api[%s]\n", __VERSION_TAG__);
	cvi_audio_uac_getDbgMask();
	memset(&gThreadEnable, 0, sizeof(ST_UAC_THREAD_ENABLE));

	if (handle == NULL) {
		handle = calloc(1, sizeof(cviuac_context));
		if (handle == NULL) {
			printf("Error cviaudio uac init handle failure\n");
			UAC_RET_FAILURE(CVI_FAILURE);

		} else
			printf("cviaudio uac init handle success\n");
	} else
		printf("Error cviaudio uac handle already exist\n");

	pUacHandle = handle;
	_cviaudio_uac_InitParam(pUacHandle);

	//reset the bind config
	_clear_bind_config();

	return CVI_SUCCESS;
}

int cviaudio_uac_stream_mode(char *input_buffer, int input_size)
{

	CHECK_NULL_PTR_RET(streamInput_fifo_handle, CVI_FAILURE);

	int fifo_level = 0;
	int fifo_size = (CVIAUDIO_UAC_FIFO_SIZE * 4);
	int ret = 0;

	fifo_level = CycleBufferDataLen(streamInput_fifo_handle);
	if ((fifo_level + (input_size * 2)) > fifo_size) {
		//DBG_UAC_PRINT("wait\n");
		usleep(100*1000);
	}
	usleep(10*1000);

	ret = CycleBufferWriteWait(streamInput_fifo_handle,
				input_buffer,
				input_size, 1000);

	if (ret != input_size) {
		ERR_UAC_PRINT("level[%d] inputsize[%d] ret[%d]\n",
			fifo_level, input_size, ret);
	}
	return CVI_SUCCESS;

}

int cviaudio_uac_createSrc(E_CVIAUDIO_UAC_SRC_PATH eSrcPath,
			E_CVIAUDIO_UAC_CODETYPE eCodeType,
			ST_UAC_SRC_PARAM *pstUacSrcParam)
{
	if (handle == NULL) {
		printf("Error cviaudio uac handle is Null...not init yet!!\n");
		return CVI_FAILURE;
	}

	cviuac_context *pUacHandle = handle;
	cviuac_src_context *pUacSrcHandle = NULL;


	CHECK_NULL_PTR(pstUacSrcParam);
	if (CHECK_UAC_SRC_VALID(eSrcPath))
		UAC_RET_FAILURE(CVI_FAILURE_ILLEGAL_PARAM);

	if (CHECK_UAC_CODEC_VALID(eCodeType))
		UAC_RET_FAILURE(CVI_FAILURE_ILLEGAL_PARAM);


	CHECK_NULL_PTR(pUacHandle);

	if (pUacHandle->cviuac_src_handle[eSrcPath] == NULL) {
		pUacHandle->cviuac_src_handle[eSrcPath] = calloc(1, sizeof(cviuac_src_context));
		if (pUacHandle->cviuac_src_handle[eSrcPath] == NULL) {
			ERR_UAC_PRINT("create handle failure\n");
			return CVI_FAILURE_NOMEM;
		}

	} else
		ERR_UAC_PRINT("already create audio srcpath[%d] handle\n", eSrcPath);

	//init the src path param
	pUacSrcHandle = pUacHandle->cviuac_src_handle[eSrcPath];
	pUacSrcHandle->codec_type = eCodeType;
	switch (eSrcPath) {
	case CVIAUDIO_SRC_MIC_IN:
		if (pUacSrcHandle->src_thread != NULL) {
			ERR_UAC_PRINT("srcpath[%d] already create thread\n", eSrcPath);
		} else {
			gThreadEnable.bEnableSrc_MicIn = true;
			pUacSrcHandle->src_thread = (pthread_t *)malloc(sizeof(pthread_t));
			CHECK_NULL_PTR(pUacSrcHandle->src_thread);
			DBG_UAC_PRINT("UAC mic in src channels[%d]\n", pstUacSrcParam->stMicInParam.channels);
			DBG_UAC_PRINT("UAC mic in src period_size[%d]\n", pstUacSrcParam->stMicInParam.period_size);
			DBG_UAC_PRINT("UAC mic in src rate[%d]\n", pstUacSrcParam->stMicInParam.rate);

			_create_uac_src_thread(pUacSrcHandle->src_thread,
						thread_src_mic_in_audio,
						(void *)&pstUacSrcParam->stMicInParam);

		}
		break;
	case CVIAUDIO_SRC_USB_IN:
		if (pUacSrcHandle->src_thread != NULL) {
			ERR_UAC_PRINT("srcpath[%d] already create thread\n", eSrcPath);
		} else {
			gThreadEnable.bEnableSrc_UsbIn = true;
			pUacSrcHandle->src_thread = (pthread_t *)malloc(sizeof(pthread_t));
			CHECK_NULL_PTR(pUacSrcHandle->src_thread);
			_create_uac_src_thread(pUacSrcHandle->src_thread,
						thread_src_usb_in_audio,
						(void *)&pstUacSrcParam->stUsbInParam);
		}
		break;
	case CVIAUDIO_SRC_FILE_IN:
		if (eCodeType == CVIAUDIO_UAC_TYPE_MP3) {
			if (pUacSrcHandle->pMp3DecHandler != NULL)
				ERR_UAC_PRINT("mp3 dec handler already exist\n");
			else {
				pUacSrcHandle->pMp3DecHandler = CVI_MP3_Decode_Init(NULL);
				CHECK_NULL_PTR(pUacSrcHandle->pMp3DecHandler);
				memset(&pUacSrcHandle->stMp3DecInfo, 0, sizeof(ST_MP3_DEC_INFO));
				CVI_MP3_Decode_InstallCb(pUacSrcHandle->pMp3DecHandler, _mp3_dec_callback);
			}
		} else if (eCodeType == CVIAUDIO_UAC_TYPE_AAC) {
			if (pUacSrcHandle->pAacDecHandler !=  NULL)
				ERR_UAC_PRINT("AAC dec handler already exist\n");
			else {
				pUacSrcHandle->pAacDecHandler = CVI_AAC_Decode_Init(CVI_DECODE_AAC_TYPE, NULL);
				CHECK_NULL_PTR(pUacSrcHandle->pAacDecHandler);
				memset(&pUacSrcHandle->stAacDecInfo, 0, sizeof(ST_AAC_DEC_INFO));
				CVI_AAC_Decode_InstallCb(pUacSrcHandle->pAacDecHandler, _aac_dec_callback);
			}

		} else if (eCodeType == CVIAUDIO_UAC_TYPE_PCM) {
			DBG_UAC_PRINT("UAC src codec type filein CVIAUDIO_UAC_TYPE_PCM\n");
		}

		if (pUacSrcHandle->src_thread != NULL) {
			ERR_UAC_PRINT("srcpath[%d] already create thread\n", eSrcPath);
		} else {
			gThreadEnable.bEnableSrc_FileIn = true;
			pUacSrcHandle->src_thread = (pthread_t *)malloc(sizeof(pthread_t));
			CHECK_NULL_PTR(pUacSrcHandle->src_thread);
			_create_uac_src_thread(pUacSrcHandle->src_thread,
						thread_src_file_in_audio,
						(void *)&pstUacSrcParam->stFileInParam);
		}
		break;
	case CVIAUDIO_SRC_STREAM_IN:
		if (eCodeType == CVIAUDIO_UAC_TYPE_MP3) {
			if (pUacSrcHandle->pMp3DecHandler != NULL)
				ERR_UAC_PRINT("mp3 dec handler already exist\n");
			else {
				pUacSrcHandle->pMp3DecHandler = CVI_MP3_Decode_Init(NULL);
				CHECK_NULL_PTR(pUacSrcHandle->pMp3DecHandler);
				memset(&pUacSrcHandle->stMp3DecInfo, 0, sizeof(ST_MP3_DEC_INFO));
				CVI_MP3_Decode_InstallCb(pUacSrcHandle->pMp3DecHandler, _mp3_dec_callback_stream);
			}
		} else if (eCodeType == CVIAUDIO_UAC_TYPE_AAC) {
			if (pUacSrcHandle->pAacDecHandler !=  NULL)
				ERR_UAC_PRINT("AAC dec handler already exist\n");
			else {
				pUacSrcHandle->pAacDecHandler = CVI_AAC_Decode_Init(CVI_DECODE_AAC_TYPE, NULL);
				CHECK_NULL_PTR(pUacSrcHandle->pAacDecHandler);
				memset(&pUacSrcHandle->stAacDecInfo, 0, sizeof(ST_AAC_DEC_INFO));
				CVI_AAC_Decode_InstallCb(pUacSrcHandle->pAacDecHandler, _aac_dec_callback_stream);
			}

		} else if (eCodeType == CVIAUDIO_UAC_TYPE_PCM) {
			DBG_UAC_PRINT("UAC src codec type filein CVIAUDIO_UAC_TYPE_PCM\n");
		}

		if (pUacSrcHandle->src_thread != NULL) {
			ERR_UAC_PRINT("srcpath[%d] already create thread\n", eSrcPath);
		} else {
			gThreadEnable.bEnableSrc_StreamIn = true;
			if (streamInput_fifo_handle != NULL)
				CycleBufferDestory(streamInput_fifo_handle);

			CycleBufferInit(&streamInput_fifo_handle, CVIAUDIO_UAC_FIFO_SIZE * 4);
			pUacSrcHandle->src_thread = (pthread_t *)malloc(sizeof(pthread_t));
			CHECK_NULL_PTR(pUacSrcHandle->src_thread);
			_create_uac_src_thread(pUacSrcHandle->src_thread,
						thread_src_stream_in_audio,
						(void *)&pstUacSrcParam->stFileInParam);

		}
		break;
	default:
		ERR_UAC_PRINT("invalid eSrcPath[%d]\n", eSrcPath);
		UAC_RET_FAILURE(CVI_FAILURE_ILLEGAL_PARAM);
		break;
	}

	return CVI_SUCCESS;
}

int cviaudio_uac_createDst(E_CVIAUDIO_UAC_DST_PATH eDstPath)
{
	if (handle == NULL) {
		printf("Error cviaudio uac handle is Null...not init yet!!\n");
		return CVI_FAILURE;
	}

	cviuac_context *pUacHandle = handle;
	cviuac_dst_context *pUacDstHandle = NULL;

	if (CHECK_UAC_DST_VALID(eDstPath))
		UAC_RET_FAILURE(CVI_FAILURE_ILLEGAL_PARAM);

	CHECK_NULL_PTR(pUacHandle);

	if (pUacHandle->cviuac_dst_handle[eDstPath] == NULL) {
		pUacHandle->cviuac_dst_handle[eDstPath] = calloc(1, sizeof(cviuac_src_context));
		if (pUacHandle->cviuac_dst_handle[eDstPath] == NULL) {
			ERR_UAC_PRINT("create handle failure\n");
			return CVI_FAILURE_NOMEM;
		}

	} else
		ERR_UAC_PRINT("already create audio dstpath[%d] handle\n", eDstPath);

	pUacDstHandle = pUacHandle->cviuac_dst_handle[eDstPath];
	switch (eDstPath) {
	case CVIAUDIO_DST_SPK_OUT:
		if (pUacDstHandle->dst_thread != NULL)
			ERR_UAC_PRINT("dstpath[%d] already create thread\n", eDstPath);
		else {
			gThreadEnable.bEnableDst_SpkOut = true;
			pUacDstHandle->dst_thread = (pthread_t *)malloc(sizeof(pthread_t));
			CHECK_NULL_PTR(pUacDstHandle->dst_thread);
			memset(&pUacDstHandle->update_pcm_info, 0, sizeof(ST_CVI_UAC_PCM_INFO_UPDATE));
			_create_uac_dst_thread(pUacDstHandle->dst_thread,
						thread_dst_spk_out_audio,
						NULL);
		}
		break;
	case CVIAUDIO_DST_USB_OUT:
		if (pUacDstHandle->dst_thread != NULL)
			ERR_UAC_PRINT("dstpath[%d] already create thread\n", eDstPath);
		else {
			int ret;

			gThreadEnable.bEnableDst_UsbOut = true;
			pUacDstHandle->dst_thread = (pthread_t *)malloc(sizeof(pthread_t));
			CHECK_NULL_PTR(pUacDstHandle->dst_thread);
			memset(&pUacDstHandle->update_pcm_info, 0, sizeof(ST_CVI_UAC_PCM_INFO_UPDATE));

			ret = _attach_dst_handle_resampler(pUacDstHandle);
			if (ret != CVI_SUCCESS)
				ERR_UAC_PRINT("\n");

			_create_uac_dst_thread(pUacDstHandle->dst_thread,
						thread_dst_usb_out_audio,
						NULL);

		}
		break;
	case CVIAUDIO_DST_FILE_SAVE:
		if (pUacDstHandle->dst_thread != NULL)
			ERR_UAC_PRINT("dstpath[%d] already create thread\n", eDstPath);
		else {
			gThreadEnable.bEnableDst_FileSave = true;
			pUacDstHandle->dst_thread = (pthread_t *)malloc(sizeof(pthread_t));
			CHECK_NULL_PTR(pUacDstHandle->dst_thread);
			memset(&pUacDstHandle->update_pcm_info, 0, sizeof(ST_CVI_UAC_PCM_INFO_UPDATE));
			_create_uac_dst_thread(pUacDstHandle->dst_thread,
						thread_dst_file_save_audio,
						NULL);
		}
		break;
	default:
		ERR_UAC_PRINT("invalid eDstPath[%d]\n", eDstPath);
		UAC_RET_FAILURE(CVI_FAILURE_ILLEGAL_PARAM);
		break;

	}

	return CVI_SUCCESS;
}

int cviaudio_uac_bind(E_CVIAUDIO_UAC_SRC_PATH eSrcPath, E_CVIAUDIO_UAC_DST_PATH eDstPath)
{
	if (CHECK_UAC_SRC_VALID(eSrcPath))
		UAC_RET_FAILURE(CVI_FAILURE_ILLEGAL_PARAM);


	if (CHECK_UAC_DST_VALID(eDstPath))
		UAC_RET_FAILURE(CVI_FAILURE_ILLEGAL_PARAM);


	st_cviaudio_fifo_buffer *pDstFiFo = (st_cviaudio_fifo_buffer *)&gstCviaudioFifo[eDstPath];

	if (pDstFiFo->pcviaudio_fifo_handle == NULL) {
		if ((eSrcPath == CVIAUDIO_SRC_FILE_IN) || (eSrcPath == CVIAUDIO_SRC_STREAM_IN)) {
			CycleBufferInit(&pDstFiFo->pcviaudio_fifo_handle, CVIAUDIO_UAC_FILE_MODE_FIFO_SIZE);
			pDstFiFo->pcviaudio_fifo_buf = (char *)malloc(CVIAUDIO_UAC_FILE_MODE_FIFO_SIZE);
			pDstFiFo->s32fifo_size = CVIAUDIO_UAC_FILE_MODE_FIFO_SIZE;
			pDstFiFo->bEnable = true;
			DBG_UAC_PRINT("cycle buffer init...........................xxxx\n");
		} else {
			CycleBufferInit(&pDstFiFo->pcviaudio_fifo_handle, CVIAUDIO_UAC_FIFO_SIZE);
			pDstFiFo->pcviaudio_fifo_buf = (char *)malloc(CVIAUDIO_UAC_FIFO_SIZE);
			pDstFiFo->s32fifo_size = CVIAUDIO_UAC_FIFO_SIZE;
			pDstFiFo->bEnable = true;
		}
	} else {
		DBG_UAC_PRINT("fifo already create in eDstPath[%d]\n", eDstPath);
	}
	gstBindCfg[eSrcPath][eDstPath] = true;
	DBG_UAC_PRINT("gstBindCfg src[%d] dst[%d] bindcfg true\n", eSrcPath, eDstPath);


	if (eSrcPath == CVIAUDIO_SRC_MIC_IN)
		gThreadEnable.bEnableSrc_MicIn = true;
	else if (eSrcPath == CVIAUDIO_SRC_USB_IN)
		gThreadEnable.bEnableSrc_UsbIn = true;
	else if (eSrcPath == CVIAUDIO_SRC_FILE_IN)
		gThreadEnable.bEnableSrc_FileIn = true;
	else if (eSrcPath == CVIAUDIO_SRC_STREAM_IN)
		gThreadEnable.bEnableSrc_StreamIn = true;

	if (eDstPath == CVIAUDIO_DST_SPK_OUT)
		gThreadEnable.bEnableDst_SpkOut = true;
	else if (eDstPath == CVIAUDIO_DST_USB_OUT)
		gThreadEnable.bEnableDst_UsbOut = true;
	else if (eDstPath == CVIAUDIO_DST_FILE_SAVE)
		gThreadEnable.bEnableDst_FileSave = true;

	return CVI_SUCCESS;
}

int cviaudio_uac_unbind(E_CVIAUDIO_UAC_SRC_PATH eSrcPath, E_CVIAUDIO_UAC_DST_PATH eDstPath)
{
	if (CHECK_UAC_SRC_VALID(eSrcPath))
		UAC_RET_FAILURE(CVI_FAILURE_ILLEGAL_PARAM);


	if (CHECK_UAC_DST_VALID(eDstPath))
		UAC_RET_FAILURE(CVI_FAILURE_ILLEGAL_PARAM);

	st_cviaudio_fifo_buffer *pfifo = (st_cviaudio_fifo_buffer *)&gstCviaudioFifo[eDstPath];

	gstBindCfg[eSrcPath][eDstPath] = false;

	if (eSrcPath == CVIAUDIO_SRC_MIC_IN)
		gThreadEnable.bEnableSrc_MicIn = false;
	else if (eSrcPath == CVIAUDIO_SRC_USB_IN)
		gThreadEnable.bEnableSrc_UsbIn = false;
	else if (eSrcPath == CVIAUDIO_SRC_FILE_IN)
		gThreadEnable.bEnableSrc_FileIn = false;
	else if (eSrcPath == CVIAUDIO_SRC_STREAM_IN)
		gThreadEnable.bEnableSrc_StreamIn = false;

	if (eDstPath == CVIAUDIO_DST_SPK_OUT)
		gThreadEnable.bEnableDst_SpkOut = false;
	else if (eDstPath == CVIAUDIO_DST_USB_OUT)
		gThreadEnable.bEnableDst_UsbOut = false;
	else if (eDstPath == CVIAUDIO_DST_FILE_SAVE)
		gThreadEnable.bEnableDst_FileSave = false;

	pfifo->bEnable = false;
	pfifo->s32fifo_size = 0;
	SAFE_FREE_BUF(pfifo->pcviaudio_fifo_buf);
	if (pfifo->pcviaudio_fifo_handle)
		CycleBufferDestory(pfifo->pcviaudio_fifo_handle);
	return CVI_SUCCESS;
}

int cviaudio_uac_destroy(E_CVIAUDIO_UAC_SRC_PATH eSrcPath,
			E_CVIAUDIO_UAC_DST_PATH eDstPath)
{
	UAC_UNUSED_REF(eSrcPath);
	UAC_UNUSED_REF(eDstPath);
	cviuac_context *pUacHandle = handle;
	cviuac_src_context *pUacSrcHandle = NULL;
	cviuac_dst_context *pUacDstHandle = NULL;

	CHECK_NULL_PTR(pUacHandle);
	//destroy eSrcPath handle
	if (pUacHandle->cviuac_src_handle[eSrcPath] == NULL) {
		printf("UAC SRC path[%d]..already destroy\n", eSrcPath);
	} else {
		pUacSrcHandle = pUacHandle->cviuac_src_handle[eSrcPath];
		if (pUacSrcHandle->src_thread != NULL) {
			printf("destroy src thread\n");
			pthread_cancel(*pUacSrcHandle->src_thread);
			pthread_join(*pUacSrcHandle->src_thread, NULL);

		}
		if (pUacSrcHandle->pMp3DecHandler)
			CVI_MP3_Decode_DeInit(pUacSrcHandle->pMp3DecHandler);
		if (pUacSrcHandle->pAacDecHandler)
			CVI_AAC_Decode_Dinit(pUacSrcHandle->pAacDecHandler);

		SAFE_FREE_BUF(pUacSrcHandle);

	}

	//destroy eDstPath handle
	if (pUacHandle->cviuac_dst_handle[eDstPath] == NULL) {
		printf("UAC DST path[%d]..already destroy\n", eDstPath);
	} else {
		pUacDstHandle = pUacHandle->cviuac_dst_handle[eDstPath];
		if (pUacDstHandle->dst_thread != NULL) {
			printf("destroy dst thread\n");
			pthread_cancel(*pUacDstHandle->dst_thread);
			pthread_join(*pUacDstHandle->dst_thread, NULL);

		}

		if (pUacDstHandle->cviRes)
			CVI_Resampler_Destroy(pUacDstHandle->cviRes);

		if (pUacDstHandle->pfd_dst_save)
			fclose(pUacDstHandle->pfd_dst_save);

		if (pUacDstHandle->stSampleResFun.pLibHandle)
			CVI_Audio_Dlclose(pUacDstHandle->stSampleResFun.pLibHandle);

		SAFE_FREE_BUF(pUacDstHandle);

	}

	return CVI_SUCCESS;
}

int cviaudio_uac_deinit(void)
{
	cviuac_context *pUacHandle = handle;

	_clear_bind_config();
	SAFE_FREE_BUF(pUacHandle);
	//TODO: check and destroy all the fifo

	return CVI_SUCCESS;
}

int cviaudio_set_spk_volume(int volstep)
{
	DBG_UAC_PRINT("range 0~32[32-0, 0:mute], enter[%d]\n", volstep);

	int fdAcodec_dac = -1;
	ACODEC_VOL_CTRL vol_ctrl;

	fdAcodec_dac = open(ACODEC_DAC, O_RDWR);
	if (fdAcodec_dac < 0) {
		ERR_UAC_PRINT("%s: can't open Acodec,%s\n", __func__, ACODEC_DAC);

		if (fdAcodec_dac != -1)
			close(fdAcodec_dac);

		return CVI_FAILURE;
	}
	/* use command: ACODEC_SET_DACL_VOL, ACODEC_SET_DACR_VOL --start */
	memset(&vol_ctrl, 0, sizeof(ACODEC_VOL_CTRL));

	if (volstep == 0) {
		DBG_UAC_PRINT("vol_ctrl.vol_ctrl_mute set ON\n");
		vol_ctrl.vol_ctrl_mute = 1;
	} else {
		DBG_UAC_PRINT("vol_ctrl.vol_ctrl_mute set Off\n");
		vol_ctrl.vol_ctrl_mute = 0;
	}

	vol_ctrl.vol_ctrl = volstep;

	if (ioctl(fdAcodec_dac, ACODEC_SET_DACL_VOL, &vol_ctrl)) {
		ERR_UAC_PRINT("ioctl err!\n");
		close(fdAcodec_dac);
		return CVI_FAILURE;

	} else
		DBG_UAC_PRINT("fdAcodec_dac ACODEC_SET_DACL_VOL [%d]ok!\n", volstep);

	if (ioctl(fdAcodec_dac, ACODEC_SET_DACR_VOL, &vol_ctrl)) {
		ERR_UAC_PRINT("ioctl err!\n");
		close(fdAcodec_dac);
		return CVI_FAILURE;

	} else
		DBG_UAC_PRINT("fdAcodec_dac ACODEC_SET_DACR_VOL [%d]ok!\n", volstep);

	/* use command: ACODEC_SET_DACL_VOL, ACODEC_SET_DACR_VOL --end */
	int u32Mute = 0;


	if (volstep == 0)
		u32Mute = 1;
	else
		u32Mute = 0;

	if (ioctl(fdAcodec_dac, ACODEC_SET_DACL_MUTE, &u32Mute))
		printf("ioctl err!\n");
	else
		printf("fdAcodec_dac ACODEC_SET_DACL_MUTE [%d]ok!\n", u32Mute);
	if (ioctl(fdAcodec_dac, ACODEC_SET_DACR_MUTE, &u32Mute))
		printf("ioctl err!\n");
	else
		printf("fdAcodec_dac ACODEC_SET_DACR_MUTE [%d]ok!\n", u32Mute);

	close(fdAcodec_dac);
	return CVI_SUCCESS;
}

int cviaudio_set_mic_volume(int volstep)
{
	int fdAcodec_adc = -1;
	ACODEC_VOL_CTRL vol_ctrl;

	DBG_UAC_PRINT("(mic in) volume[24-0, 0:mute]: [%d]\n", volstep);
	if (volstep < 0 || volstep > 24) {
		ERR_UAC_PRINT("Invalid usage\n");
		ERR_UAC_PRINT("volstep  0(low)~24(max) integer number\n");
		return CVI_FAILURE;
	}

	memset(&vol_ctrl, 0, sizeof(ACODEC_VOL_CTRL));

	fdAcodec_adc = open(ACODEC_ADC, O_RDWR);

	if (fdAcodec_adc < 0) {
		ERR_UAC_PRINT("can't open Acodec,%s\n", ACODEC_ADC);
		if (fdAcodec_adc != -1)
			close(fdAcodec_adc);

		return CVI_FAILURE;
	}


	if (volstep == 0)
		vol_ctrl.vol_ctrl_mute = 1;
	else
		vol_ctrl.vol_ctrl_mute = 0;

	vol_ctrl.vol_ctrl = volstep;
	if (ioctl(fdAcodec_adc, ACODEC_SET_ADCL_VOL, &vol_ctrl))
		ERR_UAC_PRINT("ioctl err!\n");
	else {
		DBG_UAC_PRINT("fdAcodec_dac ACODEC_SET_ADCLR_VOL [%d]ok!\n", volstep);
	}

	if (ioctl(fdAcodec_adc, ACODEC_SET_ADCR_VOL, &vol_ctrl))
		ERR_UAC_PRINT("ioctl err!\n");
	else
		DBG_UAC_PRINT("fdAcodec_dac ACODEC_SET_ADCR_VOL [%d]ok!\n", volstep);

	/* use command: ACODEC_SET_DACL_VOL, ACODEC_SET_DACR_VOL --end */
	if (fdAcodec_adc > 0)
		close(fdAcodec_adc);

	return CVI_SUCCESS;
}

#endif



