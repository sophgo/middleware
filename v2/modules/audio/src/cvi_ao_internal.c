#include <stdio.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <stdint.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "cvi_audio_interface_tinyalsa.h"
#include "cvi_ao_internal.h"
#include "atomic.h"

//global variable need to use share memory--------------->start
struct pcm_config aout_tinyconfig;
struct pcm *aout_pcm;
ST_AOUTPROC_PARAM AoutProcParam_forThread;
ST_AOUTPROC_PARAM gstAoutProcParam;
//global variable need to use share memory--------------->end

ST_AO_INSTANCE gstAoInstance[CVI_MAX_AO_DEVICE_ID_NUM];

static pthread_mutex_t g_track_lock[CVI_AUD_MAX_CHANNEL_NUM] = {
	[0 ...(CVI_AUD_MAX_CHANNEL_NUM - 1)] = PTHREAD_MUTEX_INITIALIZER};

#ifdef ARCH_CV182X
CVI_BOOL gMuteFade2Zero;
CVI_BOOL gMuteFade2Nornal;
CVI_CHAR gMuteFadeCount;
#define MUTEFILE "/tmp/mute"
#endif

void ao_dump_audiodata(char *filename, char *buf, unsigned int len)
{
	FILE *fp;

	if (filename == NULL) {
		return;
	}

	fp = fopen(filename, "ab+");
	fwrite(buf, 1, len, fp);
	fclose(fp);

}


int get_not_used_track_index(CVI_ST_THREAD_INFO *pThreadInfo)
{
	for (int i = 0; i < TRACK_MAX; i++) {
		if (!pThreadInfo->TrackVector[i]) {
			return i;
		}
	}
	return -1;
}

CVI_S32 CVI_AO_Init(void)
{
	CVI_S32 i = 0;
	memset(gstAoInstance, 0, sizeof(ST_AO_INSTANCE)*CVI_MAX_AO_DEVICE_ID_NUM);
	for (i = 0 ; i < CVI_MAX_AO_DEVICE_ID_NUM ; i++) {
		memset(&gstAoInstance[i], 0, sizeof(ST_AO_INSTANCE));
		gstAoInstance[i].stThreadInfo.card = -1;
	}
	return CVI_SUCCESS;
}

CVI_VOID CVI_AO_Deinit(void)
{
	//return;
}

CVI_S32 CVI_AO_SetPubAttr(AUDIO_DEV AoDevId, const AIO_ATTR_S *pstAttr)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ao_set_pub_attr(AoDevId, pstAttr);
	}
#endif
	if (CVIAUDIO_CHECK_NULL((void *)pstAttr))
		return -1;
	if (pstAttr->u32ChnCnt > CVI_AUD_MAX_CHANNEL_NUM) {
		log_error("invaild params,ChnCnt:%d > max_chn:%d\n",
				pstAttr->u32ChnCnt, CVI_AUD_MAX_CHANNEL_NUM);
	}
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("\n");
		return CVI_ERR_AO_INVALID_DEVID;
	}

	if (gstAoInstance[AoDevId].bEnableAO) {
		log_error("please dialbe ao first.\n");
		return CVI_ERR_AO_BUSY;
	}

	memcpy(&gstAoInstance[AoDevId].ao_attrs, pstAttr, sizeof(AIO_ATTR_S));

	return 0;
}
CVI_S32 CVI_AO_GetPubAttr(AUDIO_DEV AoDevId, AIO_ATTR_S *pstAttr)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ao_get_pub_attr(AoDevId, pstAttr);
	}
#endif
	if (CVIAUDIO_CHECK_NULL((void *)pstAttr))
		return -1;
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("AoDevId:%d\n", AoDevId);
		return CVI_ERR_AO_INVALID_DEVID;
	}
	memcpy(pstAttr, &gstAoInstance[AoDevId].ao_attrs, sizeof(AIO_ATTR_S));

	return 0;
}


CVI_S32 CVI_AO_ClrPubAttr(AUDIO_DEV AoDevId)
{

	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("AoDevId:%d\n", AoDevId);
		return CVI_ERR_AO_INVALID_DEVID;
	}

	memset(&gstAoInstance[AoDevId].ao_attrs, 0, sizeof(AIO_ATTR_S));
	return CVI_SUCCESS;
}


CVI_S32 CVI_AO_SetCard(AUDIO_DEV AoDevId, CVI_S32 AoCardId)
{
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("AoDevId:%d\n", AoDevId);
		return CVI_ERR_AO_INVALID_DEVID;
	}
	if (CHECK_AO_CARD_VALID(AoCardId)) {
		log_error("AoCardId:%d\n", AoCardId);
		return CVI_ERR_AO_INVALID_CARDID;
	}
	gstAoInstance[AoDevId].stThreadInfo.card = AoCardId;

	return 0;
}

CVI_S32 CVI_AO_Enable(AUDIO_DEV AoDevId)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ao_enable(AoDevId);
	}
#endif
	struct sched_param param;
	pthread_attr_t attr;

	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("AoDevId:%d\n", AoDevId);
		return CVI_ERR_AO_INVALID_DEVID;
	}

	ST_AO_INSTANCE *pstAoInstance = &gstAoInstance[AoDevId];

	if (pstAoInstance->bEnableAO) {
		log_error("device %d has already been enabled.\n", AoDevId);
		return CVI_ERR_AO_BUSY;
	}

	if (!pstAoInstance->ao_attrs.u32ChnCnt) {
		log_error("invalid order,ChnCnt has not been set.\n");
		return CVI_ERR_AO_NOT_CONFIG;
	}
	pstAoInstance->s32PeriodMs = pstAoInstance->ao_attrs.u32PtNumPerFrm * 1000 /
				     pstAoInstance->ao_attrs.enSamplerate;
	pstAoInstance->s32PeriodFrameLen =
		pstAoInstance->s32PeriodMs * pstAoInstance->ao_attrs.enSamplerate / 1000;

	if (!pstAoInstance->bThreadExist) {
		pstAoInstance->stThreadInfo.i32ExitPending = CVI_FALSE;
		param.sched_priority = 80;
		pthread_attr_init(&attr);
		pthread_attr_setschedpolicy(&attr, SCHED_RR);
		pthread_attr_setschedparam(&attr, &param);
		pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
		pstAoInstance->s32DevId = AoDevId;
		pthread_create(&pstAoInstance->AoutThreadId, &attr,
			       (CVI_VOID * (*)(CVI_VOID *))AudioPrimaryOutputThread, (void *)pstAoInstance);
		pstAoInstance->bThreadExist = CVI_TRUE;
		pstAoInstance->bEnableAO = CVI_TRUE;
		log_debug("AoDev:%d.--->success\n", AoDevId);
	}
	return 0;
}

CVI_S32 CVI_AO_Disable(AUDIO_DEV AoDevId)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ao_disable(AoDevId);
	}
#endif
	CVI_ST_THREAD_INFO *pThreadInfo;

	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("AoDevId:%d\n", AoDevId);
		return CVI_ERR_AO_INVALID_DEVID;
	}

	ST_AO_INSTANCE *pstAoInstance = &gstAoInstance[AoDevId];

	pThreadInfo = &pstAoInstance->stThreadInfo;
	if (pThreadInfo->i32TrackRefCnt) {
		log_error("there's %d channels in busy state.\n", pThreadInfo->i32TrackRefCnt);
		return CVI_ERR_AO_BUSY;
	}

	if (pstAoInstance->bThreadExist) {
		pstAoInstance->stThreadInfo.i32ExitPending = CVI_TRUE;
		pthread_join(pstAoInstance->AoutThreadId, NULL);
	}

	pstAoInstance->AoutThreadId = 0;
	pstAoInstance->bThreadExist = CVI_FALSE;
	pstAoInstance->bEnableAO = CVI_FALSE;
	log_debug("AoDev:%d.--->success\n", AoDevId);
	return 0;
}

CVI_S32 CVI_AO_EnableChn(AUDIO_DEV AoDevId, AO_CHN AoChn)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ao_enable_chn(AoDevId, AoChn);
	}
#endif
	CVI_ST_THREAD_INFO *pThreadInfo;

	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("AoDevId:%d\n", AoDevId);
		return CVI_ERR_AO_INVALID_DEVID;
	}

	if (AoChn < 0 || AoChn >= CVI_AUD_MAX_CHANNEL_NUM) {
		log_error("invalid AoChn:%d\n", AoChn);
		return CVI_ERR_AO_INVALID_CHNID;
	}

	ST_AO_INSTANCE *pstAoInstance = &gstAoInstance[AoDevId];

	if (!pstAoInstance->bEnableAO) {
		log_error("device %d is close\n", AoDevId);
		return CVI_ERR_AO_NOT_ENABLED;
	}

	if (pstAoInstance->pastTrackInfo[AoChn]) {
		log_error("invalid AoChn:%d,busy now.\n", AoChn);
		return CVI_ERR_AO_NOT_SUPPORT;
	}

	pthread_mutex_lock(&g_track_lock[AoChn]);
	CVI_ST_AUD_TRACK_INFO *pstTrackInfo =
		(CVI_ST_AUD_TRACK_INFO *)malloc(sizeof(CVI_ST_AUD_TRACK_INFO));
	if (!pstTrackInfo) {
		log_error("malloc failed.\n");
		pthread_mutex_unlock(&g_track_lock[AoChn]);
		return CVI_ERR_AO_NOMEM;
	}
	memset(pstTrackInfo, 0, sizeof(CVI_ST_AUD_TRACK_INFO));
	pstTrackInfo->state = STATE_IDLE;
	pstTrackInfo->lastState = STATE_IDLE;
	pstAoInstance->pastTrackInfo[AoChn] = pstTrackInfo;
	pstTrackInfo->iSampleRate = pstAoInstance->ao_attrs.enSamplerate;
	pstTrackInfo->iChannels = pstAoInstance->ao_attrs.enSoundmode + 1;
	pstTrackInfo->pBasePtr = (void *)pstAoInstance;
	pstTrackInfo->chnid = AoChn;

	pstTrackInfo->ThreadInfo = &pstAoInstance->stThreadInfo;
	pThreadInfo = pstTrackInfo->ThreadInfo;
	cvitek_atomic_inc(&pThreadInfo->i32TrackRefCnt);
	pstTrackInfo->direction = 0;

	pthread_mutex_unlock(&g_track_lock[AoChn]);
	log_debug("AoDev:%d %d.--->success\n", AoDevId, AoChn);
	return 0;
}

CVI_S32 CVI_AO_DisableChn(AUDIO_DEV AoDevId, AO_CHN AoChn)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ao_disable_chn(AoDevId, AoChn);
	}
#endif
	int timeoutCnt = 0;
	CVI_ST_THREAD_INFO *pThreadInfo;

	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("AoDevId:%d\n", AoDevId);
		return CVI_ERR_AO_INVALID_DEVID;
	}

	if (AoChn < 0 || AoChn >= CVI_AUD_MAX_CHANNEL_NUM) {
		log_error("invalid AoChn:%d\n", AoChn);
		return CVI_ERR_AO_INVALID_CHNID;
	}

	ST_AO_INSTANCE *pstAoInstance = &gstAoInstance[AoDevId];

	if (!pstAoInstance->bEnableAO) {
		log_error("device %d is close\n", AoDevId);
		return CVI_ERR_AO_NOT_ENABLED;
	}

	pthread_mutex_lock(&g_track_lock[AoChn]);
	CVI_ST_AUD_TRACK_INFO_PTR pstTrackInfo = pstAoInstance->pastTrackInfo[AoChn];

	if (!pstTrackInfo) {
		log_error("ao channel %d is not valuable\n", AoDevId);
		pthread_mutex_unlock(&g_track_lock[AoChn]);
		return CVI_ERR_AO_NOT_ENABLED;
	}

	pstTrackInfo->lastState = pstTrackInfo->state;
	pstTrackInfo->state = STATE_STOP;
	while (pstTrackInfo->refFlag && timeoutCnt < 50) {
		usleep(10 * 1000);
		timeoutCnt++;
	}
	if (pstTrackInfo->refFlag || pstTrackInfo->state != STATE_CLOSE) {
		log_warn("check the thread status,refFlag(%d) state(%d)\n",
			 pstTrackInfo->refFlag, pstTrackInfo->state);
	}

	if (pstTrackInfo->bInitOk) {
		if (pstTrackInfo->pResHandle) {
			pstTrackInfo->b_need_resample = CVI_FALSE;
			CVI_Resampler_Destroy(pstTrackInfo->pResHandle);
			pstTrackInfo->pResHandle = NULL;
		}

		share_cyclebuffer_destroy(pstTrackInfo->iShmMemIndex);
		pstTrackInfo->bInitOk = CVI_FAILURE;
	}
	pThreadInfo = pstTrackInfo->ThreadInfo;
	cvitek_atomic_dec(&pThreadInfo->i32TrackRefCnt);
	free(pstTrackInfo);
	pstAoInstance->pastTrackInfo[AoChn] = CVI_NULL;
	pthread_mutex_unlock(&g_track_lock[AoChn]);
	log_debug("AoDev:%d %d.--->success\n", AoDevId, AoChn);
	return 0;
}


CVI_S32 _audio_sendframe(CVI_ST_AUD_TRACK_INFO_PTR pstTrackInfo,
			 const AUDIO_FRAME_S *pstData, CVI_S32 s32MilliSec)
{
	int ret = 0;

	if (CVIAUDIO_CHECK_NULL((void *)pstData))
		return -1;
	if (CVIAUDIO_CHECK_NULL((void *)pstTrackInfo))
		return -1;
	ST_AO_INSTANCE *pstAoInstance = (ST_AO_INSTANCE *)pstTrackInfo->pBasePtr;

	if (!pstTrackInfo->bInitOk) {
		log_info("[start] create ao chnid:%d, channels:%d,iShmMemIndex:%d,chnsr:%d, sr:%d\n",
			 pstTrackInfo->chnid, pstTrackInfo->iChannels, pstTrackInfo->iShmMemIndex,
			 pstTrackInfo->iSampleRate, pstAoInstance->ao_attrs.enSamplerate);
		pstTrackInfo->iChannels = pstData->enSoundmode + 1;
		log_info("[start] CVI_Resampler_Create b_need_resample = %d\n",
				pstTrackInfo->b_need_resample);
		if (pstTrackInfo->b_need_resample) {
			pstTrackInfo->pResHandle = CVI_Resampler_Create(
							   pstTrackInfo->iSampleRate,
							   pstAoInstance->ao_attrs.enSamplerate,
							   pstTrackInfo->iChannels);
			if (pstTrackInfo->pResHandle == CVI_NULL) {
				log_error("Create resample func failure..\n");
				return CVI_ERR_AO_SYS_NOTREADY;
			}
		}

		int channels = pstData->enSoundmode + 1;
		int period_cnt = pstAoInstance->ao_attrs.u32FrmNum;
		int period_len = pstAoInstance->s32PeriodMs * pstTrackInfo->iSampleRate / 1000;

		CVI_U32 frame_bytes = pstData->u32Len * (pstData->enSoundmode + 1)
							* DEFAULT_BYTES_PER_SAMPLE;

		pstTrackInfo->u32ShareBufSize = period_len
				* period_cnt * channels * DEFAULT_BYTES_PER_SAMPLE * 8;//for 32k 44.1k aac
		if (pstTrackInfo->u32ShareBufSize < frame_bytes) {
			pstTrackInfo->u32ShareBufSize = frame_bytes * 2;
		}
		pstTrackInfo->iShmMemIndex =
			share_cyclebuffer_init(pstTrackInfo->u32ShareBufSize, 0);
		if (pstTrackInfo->iShmMemIndex < 0) {
			log_error("query share buffer failed.\n");
			return CVI_ERR_AO_NOMEM;
		}
		//add to track vector
		CVI_ST_THREAD_INFO *pThreadInfo = pstTrackInfo->ThreadInfo;
		int i = get_not_used_track_index(pThreadInfo);

		if (i < 0) {
			log_error("no valid track vector.\n");
			return -1;
		}
		pThreadInfo->TrackVector[i] = pstTrackInfo;

		printf("[end] create ao chnid:%d, channels:%d,iShmMemIndex:%d,sr:%d,bufsize:%d\n",
			 pstTrackInfo->chnid, pstTrackInfo->iChannels, pstTrackInfo->iShmMemIndex,
			 pstTrackInfo->iSampleRate, pstTrackInfo->u32ShareBufSize);
		pstTrackInfo->bInitOk = CVI_TRUE;
	}

	ret = share_cyclebuffer_client_write(pstTrackInfo->iShmMemIndex,
					     pstData->u64VirAddr[0],
					     pstData->u32Len * (pstData->enSoundmode + 1)
						 * DEFAULT_BYTES_PER_SAMPLE,
						 s32MilliSec);
	if (ret <= 0) {
		log_error("write failed,index:%d,bytelen:%d, bufbyte:%d bufflen:%d ret:%d\n",
			  pstTrackInfo->iShmMemIndex,
			  pstData->u32Len * (pstData->enSoundmode + 1) * DEFAULT_BYTES_PER_SAMPLE,
			  pstTrackInfo->u32ShareBufSize,
			  share_cyclebuffer_frameready_size(pstTrackInfo->iShmMemIndex), ret);
		return ret;
	}
	pstTrackInfo->dataByte = (pstData->u32Len * (pstData->enSoundmode + 1) * DEFAULT_BYTES_PER_SAMPLE);
	pstTrackInfo->allDataByte += pstTrackInfo->dataByte;

	if (pstTrackInfo->state == STATE_IDLE) {
		pstTrackInfo->lastState = pstTrackInfo->state;
		pstTrackInfo->state = STATE_READY;
	}

	return CVI_SUCCESS;
}


CVI_S32 CVI_AO_SendFrame(AUDIO_DEV AoDevId, AO_CHN AoChn,
			 const AUDIO_FRAME_S *pstData, CVI_S32 s32MilliSec)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		//printf("[rpc][chn:%d] %s\n", AoChn, __func__);
		return rpc_client_aud_ao_send_frame(AoDevId, AoChn, pstData, s32MilliSec);
	}
#endif
	int ret = 0;
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("AoDevId:%d\n", AoDevId);
		return CVI_ERR_AO_INVALID_DEVID;
	}

	if (CVIAUDIO_CHECK_NULL((void *)pstData))
		return -1;
	if (pstData->enSoundmode >= AUDIO_SOUND_MODE_BUTT) {
		log_error("invalid AoChn:%d  channel mode:%d\n", AoChn, pstData->enSoundmode);
		return CVI_ERR_AO_ILLEGAL_PARAM;
	}

	if (pstData->enBitwidth != AUDIO_BIT_WIDTH_16) {
		log_error("AoChn[%d] bit width error,enBitwidth:%d\n", AoChn, pstData->enBitwidth);
		return CVI_ERR_AO_ILLEGAL_PARAM;
	}

	if (AoChn < 0 || AoChn >= CVI_AUD_MAX_CHANNEL_NUM) {
		log_error("invalid AoChn:%d\n", AoChn);
		return CVI_ERR_AO_INVALID_CHNID;
	}

	ST_AO_INSTANCE *pstAoInstance = &gstAoInstance[AoDevId];

	if (!pstAoInstance->bEnableAO) {
		log_error("device %d is close\n", AoDevId);
		return CVI_ERR_AO_NOT_ENABLED;
	}

	pthread_mutex_lock(&g_track_lock[AoChn]);
	CVI_ST_AUD_TRACK_INFO_PTR pstTrackInfo = pstAoInstance->pastTrackInfo[AoChn];

	if (!pstTrackInfo) {
		pthread_mutex_unlock(&g_track_lock[AoChn]);
		log_error("ao channel %d is not valuable\n", AoDevId);
		return CVI_ERR_AO_NOT_ENABLED;
	}

	if (pstTrackInfo->stBindinfo.bBind) {
		log_error("bind mode, can not be called by app.\n");
		pthread_mutex_unlock(&g_track_lock[AoChn]);
		return CVI_ERR_AO_BUSY;
	}
	ret = _audio_sendframe(pstTrackInfo, pstData, s32MilliSec);

	pthread_mutex_unlock(&g_track_lock[AoChn]);

	return ret;
}


CVI_S32 CVI_AO_EnableReSmp(AUDIO_DEV AoDevId, AO_CHN AoChn,
			   AUDIO_SAMPLE_RATE_E enInSampleRate)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ao_enable_resmp(AoDevId, AoChn, enInSampleRate);
	}
#endif
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("AoDevId:%d\n", AoDevId);
		return CVI_ERR_AO_INVALID_DEVID;
	}

	if (AoChn < 0 || AoChn >= CVI_AUD_MAX_CHANNEL_NUM) {
		log_error("invalid AoChn:%d\n", AoChn);
		return CVI_ERR_AO_INVALID_CHNID;
	}
	ST_AO_INSTANCE *pstAoInstance = &gstAoInstance[AoDevId];

	if (!pstAoInstance->bEnableAO) {
		log_error("device %d is close\n", AoDevId);
		return CVI_ERR_AO_NOT_ENABLED;
	}
	pthread_mutex_lock(&g_track_lock[AoChn]);
	CVI_ST_AUD_TRACK_INFO_PTR pstTrackInfo = pstAoInstance->pastTrackInfo[AoChn];

	if (!pstTrackInfo) {
		pthread_mutex_unlock(&g_track_lock[AoChn]);
		log_error("ao channel %d is not valuable\n", AoDevId);
		return CVI_ERR_AO_NOT_ENABLED;
	}

	if (pstTrackInfo->bInitOk) {
		pthread_mutex_unlock(&g_track_lock[AoChn]);
		log_error("ao channel %d is busy now.\n", AoDevId);
		return CVI_ERR_AO_BUSY;
	}
	pstTrackInfo->iSampleRate = enInSampleRate;
	pstTrackInfo->b_need_resample = CVI_TRUE;
	log_info("track sr:%d\n", enInSampleRate);
	pthread_mutex_unlock(&g_track_lock[AoChn]);

	return 0;
}

CVI_S32 CVI_AO_DisableReSmp(AUDIO_DEV AoDevId, AO_CHN AoChn)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ao_disable_resmp(AoDevId, AoChn);
	}
#endif
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("AoDevId:%d\n", AoDevId);
		return CVI_ERR_AO_INVALID_DEVID;
	}

	if (AoChn < 0 || AoChn >= CVI_AUD_MAX_CHANNEL_NUM) {
		log_error("invalid AoChn:%d\n", AoChn);
		return CVI_ERR_AO_INVALID_CHNID;
	}
	ST_AO_INSTANCE *pstAoInstance = &gstAoInstance[AoDevId];

	if (!pstAoInstance->bEnableAO) {
		log_error("device %d is close\n", AoDevId);
		return CVI_ERR_AO_NOT_ENABLED;
	}
	pthread_mutex_lock(&g_track_lock[AoChn]);
	CVI_ST_AUD_TRACK_INFO_PTR pstTrackInfo = pstAoInstance->pastTrackInfo[AoChn];

	if (!pstTrackInfo) {
		pthread_mutex_unlock(&g_track_lock[AoChn]);
		log_error("ao channel %d is not valuable\n", AoDevId);
		return CVI_ERR_AO_NOT_ENABLED;
	}

	pstTrackInfo->b_need_resample = CVI_FALSE;
	pthread_mutex_unlock(&g_track_lock[AoChn]);

	return 0;
}

CVI_S32 CVI_AO_ClearChnBuf(AUDIO_DEV AoDevId, AO_CHN AoChn)
{
	CVI_S32 ret;

	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("AoDevId:%d\n", AoDevId);
		return CVI_ERR_AO_INVALID_DEVID;
	}

	if (AoChn < 0 || AoChn >= CVI_AUD_MAX_CHANNEL_NUM) {
		log_error("invalid AoChn:%d\n", AoChn);
		return CVI_ERR_AO_INVALID_CHNID;
	}
	ST_AO_INSTANCE *pstAoInstance = &gstAoInstance[AoDevId];

	if (!pstAoInstance->bEnableAO) {
		log_error("device %d is close\n", AoDevId);
		return CVI_ERR_AO_NOT_ENABLED;
	}
	pthread_mutex_lock(&g_track_lock[AoChn]);
	CVI_ST_AUD_TRACK_INFO_PTR pstTrackInfo = pstAoInstance->pastTrackInfo[AoChn];

	if (!pstTrackInfo) {
		pthread_mutex_unlock(&g_track_lock[AoChn]);
		log_error("ao channel %d is not valuable\n", AoDevId);
		return CVI_ERR_AO_NOT_ENABLED;
	}

	if (pstTrackInfo->iShmMemIndex < 0) {
		pthread_mutex_unlock(&g_track_lock[AoChn]);
		log_error("invalid track buffer.\n");
		return CVI_ERR_AO_NOT_CONFIG;
	}

	ret = share_cyclebuffer_clear(pstTrackInfo->iShmMemIndex);
	pthread_mutex_unlock(&g_track_lock[AoChn]);

	return ret;
}
CVI_S32 CVI_AO_QueryChnStat(AUDIO_DEV AoDevId, AO_CHN AoChn,
			    AO_CHN_STATE_S *pstStatus)
{
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("AoDevId:%d\n", AoDevId);
		return CVI_ERR_AO_INVALID_DEVID;
	}
	if (CVIAUDIO_CHECK_NULL((void *)pstStatus))
		return -1;

	if (AoChn < 0 || AoChn >= CVI_AUD_MAX_CHANNEL_NUM) {
		log_error("invalid AoChn:%d\n", AoChn);
		return CVI_ERR_AO_INVALID_CHNID;
	}

	ST_AO_INSTANCE *pstAoInstance = &gstAoInstance[AoDevId];

	if (!pstAoInstance->bEnableAO) {
		log_error("device %d is close\n", AoDevId);
		return CVI_ERR_AO_NOT_ENABLED;
	}
	pthread_mutex_lock(&g_track_lock[AoChn]);
	CVI_ST_AUD_TRACK_INFO_PTR pstTrackInfo = pstAoInstance->pastTrackInfo[AoChn];

	if (!pstTrackInfo) {
		pthread_mutex_unlock(&g_track_lock[AoChn]);
		log_error("ao channel %d is not valuable\n", AoDevId);
		return CVI_ERR_AO_NOT_ENABLED;
	}

	if (pstTrackInfo->iShmMemIndex < 0) {
		pthread_mutex_unlock(&g_track_lock[AoChn]);
		log_error("invaild track buffer.\n");
		return CVI_ERR_AO_NOT_CONFIG;
	}

	pstStatus->u32ChnTotalNum = pstTrackInfo->u32ShareBufSize;
	pstStatus->u32ChnBusyNum =
		share_cyclebuffer_frameready_size(pstTrackInfo->iShmMemIndex);
	pstStatus->u32ChnFreeNum = pstStatus->u32ChnTotalNum
				   - pstStatus->u32ChnBusyNum;
	pthread_mutex_unlock(&g_track_lock[AoChn]);

	return CVI_SUCCESS;
}

CVI_S32 CVI_AO_PauseChn(AUDIO_DEV AoDevId, AO_CHN AoChn)
{
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("AoDevId:%d\n", AoDevId);
		return CVI_ERR_AO_INVALID_DEVID;
	}

	if (AoChn < 0 || AoChn >= CVI_AUD_MAX_CHANNEL_NUM) {
		log_error("invalid AoChn:%d\n", AoChn);
		return CVI_ERR_AO_INVALID_CHNID;
	}
	ST_AO_INSTANCE *pstAoInstance = &gstAoInstance[AoDevId];

	if (!pstAoInstance->bEnableAO) {
		log_error("device %d is close\n", AoDevId);
		return CVI_ERR_AO_NOT_ENABLED;
	}
	pthread_mutex_lock(&g_track_lock[AoChn]);
	CVI_ST_AUD_TRACK_INFO_PTR pstTrackInfo = pstAoInstance->pastTrackInfo[AoChn];

	if (!pstTrackInfo) {
		pthread_mutex_unlock(&g_track_lock[AoChn]);
		log_error("ao channel %d is not valuable\n", AoDevId);
		return CVI_ERR_AO_NOT_ENABLED;
	}

	if (pstTrackInfo->iShmMemIndex < 0) {
		pthread_mutex_unlock(&g_track_lock[AoChn]);
		log_error("invaild track buffer.\n");
		return CVI_ERR_AO_NOT_CONFIG;
	}

	pstTrackInfo->lastState = pstTrackInfo->state;
	pstTrackInfo->state = STATE_PAUSE;
	pthread_mutex_unlock(&g_track_lock[AoChn]);

	return 0;
}

CVI_S32 CVI_AO_ResumeChn(AUDIO_DEV AoDevId, AO_CHN AoChn)
{
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("AoDevId:%d\n", AoDevId);
		return CVI_ERR_AO_INVALID_DEVID;
	}

	if (AoChn < 0 || AoChn >= CVI_AUD_MAX_CHANNEL_NUM) {
		log_error("invalid AoChn:%d\n", AoChn);
		return CVI_ERR_AO_INVALID_CHNID;
	}
	ST_AO_INSTANCE *pstAoInstance = &gstAoInstance[AoDevId];

	if (!pstAoInstance->bEnableAO) {
		log_error("device %d is close\n", AoDevId);
		return CVI_ERR_AO_NOT_ENABLED;
	}
	pthread_mutex_lock(&g_track_lock[AoChn]);
	CVI_ST_AUD_TRACK_INFO_PTR pstTrackInfo = pstAoInstance->pastTrackInfo[AoChn];

	if (!pstTrackInfo) {
		pthread_mutex_unlock(&g_track_lock[AoChn]);
		log_error("ao channel %d is not valuable\n", AoDevId);
		return CVI_ERR_AO_NOT_ENABLED;
	}

	if (pstTrackInfo->iShmMemIndex < 0) {
		pthread_mutex_unlock(&g_track_lock[AoChn]);
		log_error("invaild track buffer.\n");
		return CVI_ERR_AO_NOT_CONFIG;
	}

	if (pstTrackInfo->state == STATE_PAUSE) {
		pstTrackInfo->lastState = pstTrackInfo->state;
		pstTrackInfo->state = STATE_READY;
	}
	pthread_mutex_unlock(&g_track_lock[AoChn]);

	return 0;
}


CVI_S32 CVI_AO_SetVolume(AUDIO_DEV AoDevId, CVI_S32 s32VolumeDb)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ao_set_volume(AoDevId,
						    s32VolumeDb);
	}
#endif
	CVI_S32 fdAcodec_dac = -1;
	ACODEC_VOL_CTRL vol_ctrl;
#ifdef ARCH_CV183X
	printf("range 0~15 (each step 1.5db)[15-0, 0:mute], enter[%d]\n", s32VolumeDb);
#else
	printf("range 0~32[32-0, 0:mute], enter[%d]\n", s32VolumeDb);
#endif

	UNUSED_REF(AoDevId);
	fdAcodec_dac = open(ACODEC_DAC, O_RDWR);

	if (fdAcodec_dac < 0) {
		log_error("%s: can't open Acodec,%s\n", __func__, ACODEC_DAC);

		if (fdAcodec_dac != -1)
			close(fdAcodec_dac);

		return CVI_FAILURE;
	}

	/* use command: ACODEC_SET_DACL_VOL, ACODEC_SET_DACR_VOL --start */
	vol_ctrl.vol_ctrl_mute = 0x0;

	if (s32VolumeDb == 0) {
		log_debug("vol_ctrl.vol_ctrl_mute set 1\n");
#ifdef ARCH_CV183X
		vol_ctrl.vol_ctrl_mute = 1;
#else
		gstCodecConfig.bMute = CVI_TRUE;
#endif
	} else {
		log_debug("vol_ctrl.vol_ctrl_mute set 0\n");
#ifdef ARCH_CV183X
		vol_ctrl.vol_ctrl_mute = 0;
#else
		gstCodecConfig.bMute = CVI_FALSE;
#endif
	}

	vol_ctrl.vol_ctrl = s32VolumeDb;

	if (ioctl(fdAcodec_dac, ACODEC_SET_DACL_VOL, &vol_ctrl)) {
		log_error("ioctl err!\n");
		close(fdAcodec_dac);
		return CVI_FAILURE;

	} else
		printf("fdAcodec_dac ACODEC_SET_DACL_VOL [%d]ok!\n", s32VolumeDb);

	if (ioctl(fdAcodec_dac, ACODEC_SET_DACR_VOL, &vol_ctrl)) {
		log_error("ioctl err!\n");
		close(fdAcodec_dac);
		return CVI_FAILURE;

	} else
		printf("fdAcodec_dac ACODEC_SET_DACR_VOL [%d]ok!\n", s32VolumeDb);

	/* use command: ACODEC_SET_DACL_VOL, ACODEC_SET_DACR_VOL --end */

	close(fdAcodec_dac);

	gstCodecConfig.vol_ctrl = vol_ctrl;

	return CVI_SUCCESS;
}

CVI_S32 CVI_AO_GetVolume(AUDIO_DEV AoDevId, CVI_S32 *ps32VolumeDb)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ao_get_volume(AoDevId,
						    ps32VolumeDb);
	}
#endif
	ACODEC_VOL_CTRL vol_ctrl_L;
	ACODEC_VOL_CTRL vol_ctrl_R;
	CVI_S32 fdAcodec_dac = -1;

	UNUSED_REF(AoDevId);
#ifdef ARCH_CV183X
	printf("range 0~15, each step 1.5db\n");
#else
	printf("range 0~32\n");
#endif
	fdAcodec_dac = open(ACODEC_DAC, O_RDWR);

	if (fdAcodec_dac < 0) {
		log_error("%s: can't open Acodec,%s\n", __func__, ACODEC_DAC);
		return CVI_FAILURE;
	}

	/* Using command ACODEC_GET_DACL_VOL, ACODEC_GET_DACL_VOL---start */
	if (ioctl(fdAcodec_dac, ACODEC_GET_DACL_VOL, &vol_ctrl_L)) {
		log_error("ioctl err!\n");
		close(fdAcodec_dac);
		return CVI_FAILURE;

	} else
		printf("fdAcodec_dac ACODEC_GET_DACL_VOL mute[%d] [%d]ok!\n",
		       vol_ctrl_L.vol_ctrl_mute, vol_ctrl_L.vol_ctrl);

	if (ioctl(fdAcodec_dac, ACODEC_GET_DACR_VOL, &vol_ctrl_R)) {
		log_error("ioctl err!\n");
		close(fdAcodec_dac);
		return CVI_FAILURE;

	} else
		printf("fdAcodec_dac ACODEC_GET_DACR_VOL mute[%d] [%d]ok!\n",
		       vol_ctrl_R.vol_ctrl_mute, vol_ctrl_R.vol_ctrl);


	if (vol_ctrl_R.vol_ctrl != vol_ctrl_L.vol_ctrl)
		log_warn("Left & Right sound out volume not the same [%d] [%d]\n",
			  vol_ctrl_L.vol_ctrl, vol_ctrl_R.vol_ctrl);

	/* ---end */

	*ps32VolumeDb = vol_ctrl_R.vol_ctrl;

	close(fdAcodec_dac);
	return CVI_SUCCESS;

}


CVI_S32 CVI_AO_QueryFileStatus(AUDIO_DEV AoDevId, AO_CHN AoChn,
			       AUDIO_FILE_STATUS_S *pstFileStatus)
{
	UNUSED_REF(AoDevId);
	UNUSED_REF(AoChn);
	UNUSED_REF(pstFileStatus);
	return CVI_SUCCESS;
}

CVI_S32 CVI_AO_SaveFile(AUDIO_DEV AoDevId, AO_CHN AoChn,
			AUDIO_SAVE_FILE_INFO_S *pstSaveFileInfo)
{
	CVI_S32 s32Ret = CVI_FAILURE;

	UNUSED_REF(AoDevId);
	UNUSED_REF(AoChn);
	UNUSED_REF(pstSaveFileInfo);
	return s32Ret;
}

CVI_S32 CVI_AO_GetMute(AUDIO_DEV AoDevId, CVI_BOOL *pbEnable,
		       AUDIO_FADE_S *pstFade)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ao_get_mute(AoDevId, pbEnable,
						  pstFade);
	}
#endif
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("\n");
		return CVI_ERR_AO_INVALID_DEVID;
	}

	*pbEnable = gstCodecConfig.bMute;
	*pstFade = gstCodecConfig.stFade;

	return CVI_SUCCESS;

}

CVI_S32 CVI_AO_GetTrackMode(AUDIO_DEV AoDevId,
			    AUDIO_TRACK_MODE_E *penTrackMode)
{
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("\n");
		return CVI_ERR_AO_INVALID_DEVID;
	}

	*penTrackMode = gstAoInstance[AoDevId].enTrackMode;

	return CVI_SUCCESS;

}



CVI_S32 CVI_AO_SetTrackMode(AUDIO_DEV AoDevId,
			    AUDIO_TRACK_MODE_E enTrackMode)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32Retf = CVI_SUCCESS;
	CVI_S32 fdAcodec_dac = -1;

	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("\n");
		return CVI_ERR_AO_INVALID_DEVID;
	}

	/* Step 1 save the track mode to global instance */
	gstAoInstance[AoDevId].enTrackMode = enTrackMode;

	/* Step 2 set the track mode through ioctl */
	fdAcodec_dac = open(ACODEC_DAC, O_RDWR);

	if (fdAcodec_dac < 0) {
		printf("%s: can't open Acodec,%s\n", __func__, ACODEC_DAC);
		return CVI_FAILURE;
	}

	switch (enTrackMode) {
	case AUDIO_TRACK_NORMAL:
		s32Ret = ioctl(fdAcodec_dac, ACODEC_SOFT_RESET_CTRL);

		if (s32Ret != CVI_SUCCESS) {
			/* failure */
			log_error("failure..ret[%d]\n", s32Ret);
			s32Retf = CVI_ERR_AI_SYS_NOTREADY;
		}
		break;

	case AUDIO_TRACK_BOTH_LEFT: {
		/* TODO: Need confirm usage */

		CVI_U32 input_mode = 0;

		if (ioctl(fdAcodec_dac, ACODEC_SET_MIXER_MIC, &input_mode)) {
			//if (s32Ret != CVI_SUCCESS) {
			/* failure */
			log_error("failure..ret[%d]\n", s32Ret);
			s32Retf = CVI_ERR_AI_SYS_NOTREADY;
		}

		break;
	}

	case AUDIO_TRACK_BOTH_RIGHT: {
		/* TODO: Need confirm usage */
		CVI_U32 input_mode = 1;

		s32Ret = ioctl(fdAcodec_dac, ACODEC_SET_MIXER_MIC, &input_mode);

		if (s32Ret != CVI_SUCCESS) {
			/* failure */
			log_error("failure..ret[%d]\n", s32Ret);
			s32Retf = CVI_ERR_AI_SYS_NOTREADY;
		}

		break;
	}

	case AUDIO_TRACK_EXCHANGE:
		/* TODO: check the method ??? */
		log_error("How to ....\n");
		break;

	case AUDIO_TRACK_MIX:
		/* TODO: check the method ??? */
		log_error("How to ....\n");
		break;

	case AUDIO_TRACK_LEFT_MUTE: {
		CVI_U32 mute_ctrl = 0x1;

		s32Ret = ioctl(fdAcodec_dac, ACODEC_SET_MICL_MUTE, &mute_ctrl);

		if (s32Ret != CVI_SUCCESS) {
			/* failure */
			log_error("failure..ret[%d]\n", s32Ret);
			s32Retf = CVI_ERR_AI_SYS_NOTREADY;
		}

		break;
	}

	case AUDIO_TRACK_RIGHT_MUTE: {
		CVI_U32 mute_ctrl = 0x1;

		s32Ret = ioctl(fdAcodec_dac, ACODEC_SET_MICR_MUTE, &mute_ctrl);

		if (s32Ret != CVI_SUCCESS) {
			/* failure */
			log_error("failure..ret[%d]\n", s32Ret);
			s32Retf = CVI_ERR_AI_SYS_NOTREADY;
		}

		break;
	}

	case AUDIO_TRACK_BOTH_MUTE: {
		CVI_U32 mute_ctrl = 0x1;

		s32Ret = ioctl(fdAcodec_dac, ACODEC_SET_MICR_MUTE, &mute_ctrl);
		s32Ret = ioctl(fdAcodec_dac, ACODEC_SET_MICL_MUTE, &mute_ctrl);

		if (s32Ret != CVI_SUCCESS) {
			/* failure */
			log_error("failure..ret[%d]\n", s32Ret);
			s32Retf = CVI_ERR_AI_SYS_NOTREADY;
		}

		break;
	}

	case AUDIO_TRACK_BUTT:
		log_error("invalid option ...return\n");
		close(fdAcodec_dac);
		return CVI_ERR_AI_ILLEGAL_PARAM;

	/* break; */
	default:
		log_error("Not support this mode\n");
		break;
	}

	close(fdAcodec_dac);
	return s32Retf;
}


CVI_S32 CVI_AO_SetMute(AUDIO_DEV AoDevId, CVI_BOOL bEnable,
		       const AUDIO_FADE_S *pstFade)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ao_set_mute(AoDevId, bEnable,
						  pstFade);
	}
#endif
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("\n");
		return CVI_ERR_AO_INVALID_DEVID;
	}


	CVI_S32 s32Ret = CVI_FAILURE;
	CVI_S32 fdAcodec_adc = -1;
	CVI_S32 fdAcodec_dac = -1;
	CVI_U32 u32Mute = 1;

	if (pstFade != CVI_NULL)
		memcpy(&gstCodecConfig.stFade, pstFade, sizeof(AUDIO_FADE_S));
	else {
		gstCodecConfig.stFade.bFade = CVI_FALSE;
		//set the default fade to false
	}


	fdAcodec_adc = open(ACODEC_ADC, O_RDWR);

	if (fdAcodec_adc < 0) {
		log_error("%s: can't open Acodec,%s\n", __func__, ACODEC_ADC);
		return CVI_ERR_AIO_NOT_PERM;
	}

	fdAcodec_dac = open(ACODEC_DAC, O_RDWR);

	if (fdAcodec_dac < 0) {
		log_error("%s: can't open Acodec,%s\n", __func__, ACODEC_DAC);
		return CVI_ERR_AIO_NOT_PERM;
	}

	if (bEnable == CVI_TRUE) {
		gstCodecConfig.bMute = CVI_TRUE;
		u32Mute = 1;
		printf("set mute on ------------------------------>\n");
	} else {
		gstCodecConfig.bMute = CVI_FALSE;
		u32Mute = 0;
		printf("set mute off <------------------------------\n");
	}

	/* Fade Setting Check --- start */
	if (gstCodecConfig.stFade.bFade == CVI_TRUE) {
		CVI_S32 s32Delay = gstCodecConfig.stFade.enFadeInRate;
		CVI_S32 s32VolumeDb;

		if (u32Mute == CVI_TRUE && gstCodecConfig.stFade.enFadeOutRate == AUDIO_FADE_RATE_BUTT) {
			log_error(" conflict fade setting [%d][%d]\n",
				  u32Mute,
				  gstCodecConfig.stFade.enFadeOutRate);
			close(fdAcodec_adc);
			close(fdAcodec_dac);
			return CVI_ERR_AO_ILLEGAL_PARAM;

		} else
			s32Delay = gstCodecConfig.stFade.enFadeOutRate;

		if ((u32Mute == CVI_FALSE) && (gstCodecConfig.stFade.enFadeInRate == AUDIO_FADE_RATE_BUTT)) {
			log_error(" conflict fade setting [%d][%d]\n",
				  u32Mute,
				  gstCodecConfig.stFade.enFadeOutRate);
			close(fdAcodec_adc);
			close(fdAcodec_dac);
			return CVI_ERR_AO_ILLEGAL_PARAM;

		} else
			s32Delay = gstCodecConfig.stFade.enFadeInRate;


		s32Ret = CVI_AO_GetVolume(AoDevId, &s32VolumeDb);

		if (s32Ret != CVI_SUCCESS || s32VolumeDb < 0) {
			log_error("\n");
			return CVI_FAILURE;
		}


		/* range 0~15 (each step 1.5db)[15-0, 0:mute] */
		if (u32Mute == CVI_TRUE) {
			/* Start to fadeout */
			while (s32VolumeDb != 0) {
				usleep(s32Delay * 1000);
				s32Ret = CVI_AO_SetVolume(AoDevId, s32VolumeDb);
				s32VolumeDb--;

				if (s32Ret != CVI_SUCCESS) {
					log_error("\n");
					close(fdAcodec_adc);
					close(fdAcodec_dac);
					return CVI_ERR_AO_ILLEGAL_PARAM;
				}
			}

		} else {
			/* Start to fadeIn to previous target volume */
			CVI_S32 s32InitVol = 0;

			if ((int)gstCodecConfig.vol_ctrl.vol_ctrl >= 0)
				log_debug("target volume[%d]\n", gstCodecConfig.vol_ctrl.vol_ctrl);

			while (s32InitVol != (CVI_S32)gstCodecConfig.vol_ctrl.vol_ctrl) {
				usleep(s32Delay * 1000);
				s32Ret = CVI_AO_SetVolume(AoDevId, s32InitVol);
				s32InitVol++;

				if (s32Ret != CVI_SUCCESS) {
					log_error("\n");
					close(fdAcodec_adc);
					close(fdAcodec_dac);
					return CVI_ERR_AO_ILLEGAL_PARAM;
				}
			}
		}

	}

	/* Fade Setting Check --- stop */

#ifdef ARCH_CV183X
	if (ioctl(fdAcodec_dac, ACODEC_SET_DACL_MUTE, &u32Mute))
		printf("ioctl err!\n");
	else
		printf("fdAcodec_dac ACODEC_SET_DACL_MUTE [%d]ok!\n", u32Mute);

	if (ioctl(fdAcodec_dac, ACODEC_SET_DACR_MUTE, &u32Mute))
		printf("ioctl err!\n");
	else
		printf("fdAcodec_dac ACODEC_SET_DACR_MUTE [%d]ok!\n", u32Mute);
#endif

	close(fdAcodec_adc);
	close(fdAcodec_dac);

	return CVI_SUCCESS;
}

