#include <stdio.h>
#include "cvi_ai_internal.h"
#include "cvi_audio_interface_tinyalsa.h"
#include "atomic.h"
#include "volume_ctrl.h"


ST_AI_INSTANCE gstAiInstance[CVI_MAX_AI_DEVICE_ID_NUM];

static pthread_mutex_t g_track_lock[CVI_AUD_MAX_CHANNEL_NUM] = {
	[0 ...(CVI_AUD_MAX_CHANNEL_NUM - 1)] = PTHREAD_MUTEX_INITIALIZER};

#ifdef CVIAUDIO_SOFTWARE_AEC
#if 0
static void _mux_with_right_channel(CVI_S16 *right_buff,
				    char **mux_buff,
				    int mono_bytes)
{
	int i = 0;

	short *tmp = (short *)(*mux_buff);

	for (i = 0;  i < (mono_bytes / 2); i++) {
		//tmp[2 * i] = mux_buff[i];
		tmp[2 * i + 1] = right_buff[i];
	}
}
#endif
#endif


static CVI_S32 _parsing_ain_channel_vqe_status(AUDIO_DEV AiDevId, AI_CHN AiChn, CVI_ST_AUD_TRACK_INFO *pastTrackInfo)
{
	CVI_S32 s32Ret = CVI_FAILURE;
	ST_AI_CHANNEL_CONFIG *pstAiChnCfg;

	if (CVIAUDIO_CHECK_NULL((void *)pastTrackInfo))
		return -1;
	if (!pastTrackInfo->direction) {
		return CVI_FAILURE;
	}
	pstAiChnCfg = &pastTrackInfo->stAiChnCfg;

	pstAiChnCfg->bEnableVqe = CVI_AI_VQECheckEnable(AiDevId, AiChn);

	if (pstAiChnCfg->bEnableVqe == CVI_FALSE) {
		s32Ret = CVI_FAILURE;
	} else {
		if (pastTrackInfo->bInitOk != CVI_TRUE) {
			log_error("Enable vqe wo channel enable devid[%d]chnid[%d]\n",
				  AiDevId, AiChn);
			s32Ret = CVI_FAILURE;
		} else {
			CVI_S32 s32VqeMask = 0x0;

			pastTrackInfo->iChannels = DEFAULT_AEC_OUT_CHN_COUNT;
			s32Ret = CVI_SUCCESS;
			s32VqeMask = CVI_AI_VQECheckFlag(AiDevId, AiChn);
			if ((s32VqeMask & AI_TALKVQE_MASK_AEC) == AI_TALKVQE_MASK_AEC)
				pstAiChnCfg->bVqeWithAecOn = CVI_TRUE;
			else if (((s32VqeMask & AI_TALKVQE_MASK_AGC) == AI_TALKVQE_MASK_AGC) ||
				 ((s32VqeMask & AI_TALKVQE_MASK_ANR) == AI_TALKVQE_MASK_ANR))
				pstAiChnCfg->bVqeWithAecOn = CVI_FALSE;
			else {
				log_error("vqe on but mask unidentified dev[%d]chn[%d]mask[0x%x]\n",
					  AiDevId, AiChn, s32VqeMask);
				pstAiChnCfg->bEnableVqe = CVI_FALSE;
				s32Ret = CVI_FAILURE;
			}

		}
	}

	if (s32Ret == CVI_SUCCESS)
		log_debug("VQE enable in dev[%d]chn[%d]\n", AiDevId, AiChn);
	return s32Ret;
}




CVI_S32 CVI_AI_QuickRecord(CVI_VOID)
{
	printf("Enter [%s] [%d] not support!\n", __func__, __LINE__);
	return CVI_SUCCESS;

}
CVI_S32 CVI_AI_Init(void)
{
	CVI_S32 i = 0;

	log_debug("cviaudio version[%s]\n", _VERSION_TAG_);
	//init the variable
	for (i = 0 ; i < CVI_MAX_AI_DEVICE_ID_NUM ; i++)
		memset(&gstAiInstance[i], 0, sizeof(ST_AI_INSTANCE));

#ifdef CVIAUDIO_MW_STR_MODE
	memset(&gstAudStr, 0, sizeof(ST_CVIAUDIO_MW_STR_MODE));
#endif
	return CVI_SUCCESS;
}

void CVI_AI_Deinit(void)
{
#if 0
	//clean up buffer from ai_enable
	CVI_S32 s32DevCnt = 0;

	for (s32DevCnt = 0; s32DevCnt < CVI_MAX_AI_DEVICE_ID_NUM; s32DevCnt++) {
		//ST_TINYALSA_CONFIG *pstTinyAlsaCfg = &gstAiInstance[s32DevCnt].stTinyAlsaCfg;
		ST_AI_INSTANCE *pain_instatnce =  &gstAiInstance[s32DevCnt];

#if 0//TODO: Need check pcm_close in ain
		if (pstTinyAlsaCfg->pcm) {
			log_debug("CVI_AUDIO_DEINIT:pcm_close ain pcm devid[%d]\n", s32DevCnt);
			pcm_close(pstTinyAlsaCfg->pcm);
			pstTinyAlsaCfg->pcm = CVI_NULL;
		}
#endif

		pain_instatnce->bThreadExist = CVI_FALSE;

		CVI_S32 s32ChnCnt = 0;

		for (s32ChnCnt = 0; s32ChnCnt < CVI_AUD_MAX_CHANNEL_NUM; s32ChnCnt++) {
			ST_AI_CHANNEL_CONFIG *pstChnConfig = &gstAiInstance[s32DevCnt].stChnConfig[s32ChnCnt];

			SAFE_FREE_BUF(pstChnConfig->ps16ResBuffer);
			SAFE_FREE_BUF(pstChnConfig->cviRes);
			SAFE_FREE_BUF(pstChnConfig->pCycleReadBuffer);
			SAFE_FREE_BUF(pstChnConfig->pVqeBuff);
		}
	}
#ifdef CVIAUDIO_SOFTWARE_AEC
	CVI_VQE_CloseSwAEC();
#endif
#ifdef RPC_MULTI_PROCESS_AUDIO
	rpc_server_audio_deinit();
#endif
	CVI_AudIn_AlgoFreeBuffer();
#endif
}



/* AI(audio input <arecord>) function api. */
CVI_S32 CVI_AI_SetPubAttr(AUDIO_DEV AiDevId, const AIO_ATTR_S *pstAttr)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ai_set_pub_attr(AiDevId, pstAttr);
	}
#endif
	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		log_error("!\n");
		return CVI_ERR_AI_INVALID_DEVID;
	}
	//for mark the status that ai attr been set
	gstAudStatus.AiIdxNow = AiDevId;
	if (pstAttr->u32ChnCnt > CVI_AUD_MAX_CHANNEL_NUM) {
		log_error("invaild params,ChnCnt:%d > max_chn:%d\n",
				pstAttr->u32ChnCnt, CVI_AUD_MAX_CHANNEL_NUM);
	}
	log_debug("index now[%d]\n", AiDevId);
	memcpy(&gstAiInstance[AiDevId].aio_attrs, pstAttr, sizeof(AIO_ATTR_S));

	log_debug("gstAiInstance AiDevId[%d]\n", (CVI_S32)AiDevId);
	log_debug("gstAiInstance.aio_attrs.enSamplerate[%d]\n",
		  (CVI_S32)gstAiInstance[AiDevId].aio_attrs.enSamplerate);
	log_debug("gstAiInstance.aio_attrs.enBitwidth[%d]\n",
		  (CVI_S32)gstAiInstance[AiDevId].aio_attrs.enBitwidth);
	log_debug("gstAiInstance.aio_attrs.enWorkmode[%d]\n",
		  (CVI_S32)gstAiInstance[AiDevId].aio_attrs.enWorkmode);
	log_debug("gstAiInstance.aio_attrs.enSoundmode[%d]\n",
		  (CVI_S32)gstAiInstance[AiDevId].aio_attrs.enSoundmode);
	log_debug("gstAiInstance.aio_attrs.u32EXFlag[%d]\n",
		  (CVI_S32)gstAiInstance[AiDevId].aio_attrs.u32EXFlag);
	log_debug("gstAiInstance.aio_attrs.u32FrmNum[%d]\n",
		  (CVI_S32)gstAiInstance[AiDevId].aio_attrs.u32FrmNum);
	log_debug("gstAiInstance.aio_attrs.u32PtNumPerFrm[%d]\n",
		  (CVI_S32)gstAiInstance[AiDevId].aio_attrs.u32PtNumPerFrm);
	log_debug("gstAiInstance.aio_attrs.u32ChnCnt[%d]\n",
		  (CVI_S32)gstAiInstance[AiDevId].aio_attrs.u32ChnCnt);
	log_debug("gstAiInstance.aio_attrs.u32ClkSel[%d]\n",
		  (CVI_S32)gstAiInstance[AiDevId].aio_attrs.u32ClkSel);
	log_debug("gstAiInstance.aio_attrs.enI2sType[%d]\n",
		  (CVI_S32)gstAiInstance[AiDevId].aio_attrs.enI2sType);

	return CVI_SUCCESS;
}
CVI_S32 CVI_AI_GetPubAttr(AUDIO_DEV AiDevId, AIO_ATTR_S *pstAttr)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ai_get_pub_attr(AiDevId, pstAttr);
	}
#endif
	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		log_error("!\n");
		return CVI_ERR_AI_INVALID_DEVID;
	}

	memcpy(pstAttr, &gstAiInstance[AiDevId].aio_attrs, sizeof(AIO_ATTR_S));
	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_Enable(AUDIO_DEV AiDevId)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ai_enable(AiDevId);
	}
#endif
	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		log_error("AiDevId:%d\n", AiDevId);
		return CVI_ERR_AI_INVALID_DEVID;
	}
	struct sched_param param;
	pthread_attr_t attr;
	ST_AI_INSTANCE *_ain_instatnce = &gstAiInstance[AiDevId];

	if (_ain_instatnce->bEnableAI == CVI_TRUE) {
		log_error("device %d has already been enabled.\n", AiDevId);
		return CVI_ERR_AI_BUSY;
	}

	if (!_ain_instatnce->aio_attrs.u32ChnCnt) {
		log_error("invalid order,ChnCnt has not been set.\n");
		return CVI_ERR_AI_NOT_CONFIG;
	}


	if (!_ain_instatnce->bThreadExist) {
		param.sched_priority = 80;
		pthread_attr_init(&attr);
		pthread_attr_setschedpolicy(&attr, SCHED_RR);
		pthread_attr_setschedparam(&attr, &param);
		pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
		pthread_create(&_ain_instatnce->AinThreadId,
			       &attr,
			       (CVI_VOID * (*)(CVI_VOID *))AudioPrimaryInputThread,
			       (void *)_ain_instatnce);

		_ain_instatnce->bThreadExist = CVI_TRUE;
		_ain_instatnce->bEnableAI = CVI_TRUE;
		_ain_instatnce->s32DevId = AiDevId;
		log_debug("AiDev:%d.--->success\n", AiDevId);
	}
	return CVI_SUCCESS;
}
CVI_S32 CVI_AI_Disable(AUDIO_DEV AiDevId)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ai_disable(AiDevId);
	}
#endif
	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		log_error("\n");
		return CVI_ERR_AI_INVALID_DEVID;
	}
	CVI_ST_THREAD_INFO *pThreadInfo;
	ST_AI_INSTANCE *pain_instatnce =  &gstAiInstance[AiDevId];

	//apparently the stop procedure should depned by DevId(input usage)
	if (pain_instatnce->bEnableAI == CVI_FALSE) {
		log_error("already disable\n");
		return CVI_SUCCESS;
	}

	pThreadInfo = &pain_instatnce->stThreadInfo;
	if (pThreadInfo->i32TrackRefCnt) {
		log_error("there's %d channels in busy state.\n", pThreadInfo->i32TrackRefCnt);
		return CVI_ERR_AI_BUSY;
	}

	if (pain_instatnce->bThreadExist) {
		pain_instatnce->stThreadInfo.i32ExitPending = CVI_TRUE;
		pthread_join(pain_instatnce->AinThreadId, NULL);
		pain_instatnce->bThreadExist = CVI_FALSE;
		memset(&pain_instatnce->stThreadInfo, 0, sizeof(CVI_ST_THREAD_INFO));
	}
	pain_instatnce->bEnableAI = CVI_FALSE;

	log_debug("AiDev:%d.--->success\n", AiDevId);
	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_EnableChn(AUDIO_DEV AiDevId, AI_CHN AiChn)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ai_enable_chn(AiDevId, AiChn);
	}
#endif
	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		log_error("\n");
		return CVI_ERR_AI_INVALID_DEVID;
	}

	ST_AI_INSTANCE *pstAiInstance = &gstAiInstance[AiDevId];
	AIO_ATTR_S *paio_attrs = &pstAiInstance->aio_attrs;
	CVI_ST_AUD_TRACK_INFO *pstTrackInfo = NULL;

	if (AiChn < 0 || AiChn >= CVI_AUD_MAX_CHANNEL_NUM) {
		log_error("invalid AiChn:%d\n", AiChn);
		return CVI_ERR_AI_INVALID_DEVID;
	}
	if (pstAiInstance->ppastTrackInfo[AiChn]) {
		log_error("channel already enable[%d]\n", AiChn);
		return CVI_ERR_AI_BUSY;
	}
	pthread_mutex_lock(&g_track_lock[AiChn]);
	pstAiInstance->ppastTrackInfo[AiChn] = (CVI_ST_AUD_TRACK_INFO *)malloc(sizeof(CVI_ST_AUD_TRACK_INFO));
	if (CVIAUDIO_CHECK_NULL((void *)pstAiInstance->ppastTrackInfo[AiChn]))
		return -1;

	pstTrackInfo = pstAiInstance->ppastTrackInfo[AiChn];
	memset(pstTrackInfo, 0, sizeof(CVI_ST_AUD_TRACK_INFO));
	pstTrackInfo->pBasePtr = (void *)pstAiInstance;
	pstTrackInfo->chnid = AiChn;
	pstTrackInfo->direction = 1;
	pstTrackInfo->iChannels = paio_attrs->enSoundmode + 1;
	pstTrackInfo->iSampleRate = paio_attrs->enSamplerate;
	pstTrackInfo->state = STATE_READY;
	pstTrackInfo->ThreadInfo = &pstAiInstance->stThreadInfo;

	//add to track vector
	CVI_ST_THREAD_INFO *pThreadInfo = pstTrackInfo->ThreadInfo;

	pThreadInfo->TrackVector[AiChn] = pstTrackInfo;
	pstTrackInfo->bInitOk = CVI_TRUE;
	cvitek_atomic_inc(&pThreadInfo->i32TrackRefCnt);

	if (!pstAiInstance->ppVolinstance[AiChn]) {
		pstAiInstance->ppVolinstance[AiChn] = cvitek_volume_ctrl_create();
		log_debug("creat sw volume[%p[%d]]\n", pstAiInstance->ppVolinstance[AiChn], AiChn);
	}

	pthread_mutex_unlock(&g_track_lock[AiChn]);
	log_debug("AiDev:%d %d.--->success\n", AiDevId, AiChn);
	return CVI_SUCCESS;
}
CVI_S32 CVI_AI_DisableChn(AUDIO_DEV AiDevId, AI_CHN AiChn)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ai_disable_chn(AiDevId, AiChn);
	}
#endif
	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		log_error("AiDevId:%d\n", AiDevId);
		return CVI_ERR_AI_INVALID_DEVID;
	}

	if (AiChn < 0 || AiChn >= CVI_AUD_MAX_CHANNEL_NUM) {
		log_error("invalid AiChn:%d\n", AiChn);
		return CVI_ERR_AI_INVALID_DEVID;
	}

	ST_AI_INSTANCE *pstAiInstance = &gstAiInstance[AiDevId];
	if (!pstAiInstance->bEnableAI) {
		log_error("device %d is close\n", AiDevId);
		return CVI_ERR_AI_NOT_ENABLED;
	}

	CVI_ST_AUD_TRACK_INFO *pstTrackInfo = pstAiInstance->ppastTrackInfo[AiChn];

	if (CVIAUDIO_CHECK_NULL((void *)pstTrackInfo))
		return -1;
	pstTrackInfo->state = STATE_CLOSE;
	pthread_mutex_lock(&g_track_lock[AiChn]);
	CVI_ST_THREAD_INFO *pThreadInfo = pstTrackInfo->ThreadInfo;
	ST_AI_CHANNEL_CONFIG *pstAiChnCfg = &pstTrackInfo->stAiChnCfg;

	if (!pstTrackInfo->bInitOk) {
		log_error("ai channel %d is not valuable\n", AiChn);
		pthread_mutex_unlock(&g_track_lock[AiChn]);
		return CVI_ERR_AI_NOT_ENABLED;
	}

	if (pstTrackInfo->iShmMemIndex > 0)
		share_cyclebuffer_destroy(pstTrackInfo->iShmMemIndex);

	pThreadInfo->TrackVector[AiChn] = NULL;
	cvitek_atomic_dec(&pThreadInfo->i32TrackRefCnt);

	SAFE_FREE_BUF(pstAiChnCfg->pRawData);
	memset(pstTrackInfo, 0, sizeof(CVI_ST_AUD_TRACK_INFO));
	SAFE_FREE_BUF(pstAiInstance->ppastTrackInfo[AiChn]);

	if (pstAiInstance->ppVolinstance[AiChn]) {
		cvitek_volume_ctrl_destroy(pstAiInstance->ppVolinstance[AiChn]);
		pstAiInstance->ppVolinstance[AiChn] = NULL;
		log_debug("destroy sw volume\n");
	}

	pthread_mutex_unlock(&g_track_lock[AiChn]);
	log_debug("AiDev:%d %d.--->success\n", AiDevId, AiChn);
	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_GetFrame(AUDIO_DEV AiDevId, AI_CHN AiChn,
			AUDIO_FRAME_S *pstFrm, AEC_FRAME_S *pstAecFrm, CVI_S32 s32MilliSec)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster())
		return rpc_client_aud_ai_get_frame(AiDevId,
						   AiChn,
						   pstFrm,
						   pstAecFrm,
						   s32MilliSec);

#endif
	CVI_S32 ret;
	UNUSED_REF(pstAecFrm);
	ST_AI_INSTANCE *pstAiInstance = &gstAiInstance[AiDevId];
	CVI_ST_AUD_TRACK_INFO *pstTrackInfo = pstAiInstance->ppastTrackInfo[AiChn];

	if (CVIAUDIO_CHECK_NULL((void *)pstTrackInfo))
		return -1;
	pthread_mutex_lock(&g_track_lock[AiChn]);
	ST_AI_CHANNEL_CONFIG *pstAiChnCfg = &pstTrackInfo->stAiChnCfg;

	if (pstTrackInfo->state == STATE_READY) {

		_parsing_ain_channel_vqe_status(AiDevId, AiChn, pstTrackInfo);
		int channels = pstTrackInfo->iChannels;
		int frame_cnt = pstTrackInfo->stAiChnCfg.stChnParams.u32UsrFrmDepth;
		int period_len = pstAiInstance->aio_attrs.u32PtNumPerFrm;
		int OutResLen = period_len * pstTrackInfo->iSampleRate / pstAiInstance->aio_attrs.enSamplerate;

		if (!frame_cnt)
			frame_cnt = pstAiInstance->aio_attrs.u32FrmNum;

		pstAiChnCfg->u32RawPeriodBytes = period_len * channels * DEFAULT_BYTES_PER_SAMPLE;
		if (pstTrackInfo->b_need_resample && pstTrackInfo->pResHandle && (period_len != 1024))
			pstAiChnCfg->u32RawPeriodBytes = OutResLen * channels * DEFAULT_BYTES_PER_SAMPLE;

		pstTrackInfo->u32ShareBufSize = frame_cnt * pstAiChnCfg->u32RawPeriodBytes * 2;
		pstAiChnCfg->pRawData = (CVI_S16 *)malloc(pstAiChnCfg->u32RawPeriodBytes);
		if (CVIAUDIO_CHECK_NULL((void *)pstAiChnCfg->pRawData)) {
			pthread_mutex_unlock(&g_track_lock[AiChn]);
			return -1;
		}

		pstTrackInfo->iShmMemIndex =
			share_cyclebuffer_init(pstTrackInfo->u32ShareBufSize, 0);
		if (pstTrackInfo->iShmMemIndex < 0) {
			log_error("query share buffer failed.\n");
			free(pstAiChnCfg->pRawData);
			pthread_mutex_unlock(&g_track_lock[AiChn]);
			return CVI_ERR_AI_NOMEM;
		}
		pstTrackInfo->state = STATE_RUNNING;
		log_info("AiIndex:%d\n", pstTrackInfo->iShmMemIndex);
	}

	if (pstTrackInfo->state != STATE_RUNNING) {
		log_error("current channel is close or not configed.\n");
		pthread_mutex_unlock(&g_track_lock[AiChn]);
		return CVI_ERR_AO_NOT_PERM;
	}

	ret = share_cyclebuffer_client_read(pstTrackInfo->iShmMemIndex,
					    pstAiChnCfg->pRawData, pstAiChnCfg->u32RawPeriodBytes, s32MilliSec);
	if (ret <= 0) {
		log_error("read failed,index:%d,ret:%d\n", pstTrackInfo->iShmMemIndex, ret);
		pthread_mutex_unlock(&g_track_lock[AiChn]);
		return ret;
	}
	pstTrackInfo->allDataByte += pstAiChnCfg->u32RawPeriodBytes;
	pstTrackInfo->dataByte = pstAiChnCfg->u32RawPeriodBytes;
	pstFrm->enBitwidth = pstAiInstance->aio_attrs.enBitwidth;
	pstFrm->enSoundmode = pstTrackInfo->iChannels - 1;
	pstFrm->u64TimeStamp = _get_current_pts();
	pstFrm->u32Len = pstAiChnCfg->u32RawPeriodBytes /
			 (DEFAULT_BYTES_PER_SAMPLE * pstTrackInfo->iChannels);
	pstFrm->u64VirAddr[0] = (CVI_U8 *)pstAiChnCfg->pRawData;
	pstFrm->u32Seq = pstTrackInfo->u32SeqCnt++;
	pthread_mutex_unlock(&g_track_lock[AiChn]);
	return CVI_SUCCESS;
}


CVI_S32 CVI_AI_ReleaseFrame(AUDIO_DEV AiDevId, AI_CHN AiChn,
			    const AUDIO_FRAME_S *pstFrm, const AEC_FRAME_S *pstAecFrm)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ai_release_frame(AiDevId, AiChn,
						       pstFrm,
						       pstAecFrm);
	}
#endif
	UNUSED_REF(AiDevId);
	UNUSED_REF(AiChn);
	UNUSED_REF(pstFrm);
	UNUSED_REF(pstAecFrm);
	/* [Notice]: */
	/* Do not need to release/free pstFrm address inside */
	/* Since the address already parts of the ring buffer */
	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_SetChnParam(AUDIO_DEV AiDevId, AI_CHN AiChn,
			   const AI_CHN_PARAM_S *pstChnParam)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ai_set_chn_param(AiDevId, AiChn,
						       pstChnParam);
	}
#endif
	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		log_error("\n");
		return CVI_ERR_AI_INVALID_DEVID;
	}
	if (CVIAUDIO_CHECK_NULL((void *)pstChnParam))
		return -1;

	ST_AI_INSTANCE *_ai_instance = &gstAiInstance[AiDevId];
	CVI_ST_AUD_TRACK_INFO *pstTrackInfo = _ai_instance->ppastTrackInfo[AiChn];

	if (CVIAUDIO_CHECK_NULL((void *)pstTrackInfo))
		return -1;
	if (_ai_instance->aio_attrs.u32FrmNum > pstChnParam->u32UsrFrmDepth) {
		log_warn("currnet u32UsrFrmDepth %d set is too small.\n", pstChnParam->u32UsrFrmDepth);
		_ai_instance->aio_attrs.u32FrmNum = 3;
	}

	memcpy(&pstTrackInfo->stAiChnCfg.stChnParams, pstChnParam, sizeof(AI_CHN_PARAM_S));

	return CVI_SUCCESS;
}
CVI_S32 CVI_AI_GetChnParam(AUDIO_DEV AiDevId, AI_CHN AiChn,
			   AI_CHN_PARAM_S *pstChnParam)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ai_get_chn_param(AiDevId, AiChn,
						       pstChnParam);
	}
#endif
	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		log_error("\n");
		return CVI_ERR_AI_INVALID_DEVID;
	}
	if (CVIAUDIO_CHECK_NULL((void *)pstChnParam))
		return -1;
	ST_AI_INSTANCE *_ai_instance = &gstAiInstance[AiDevId];
	CVI_ST_AUD_TRACK_INFO *pstTrackInfo = _ai_instance->ppastTrackInfo[AiChn];

	if (CVIAUDIO_CHECK_NULL((void *)pstTrackInfo))
		return -1;
	memcpy(pstChnParam, &pstTrackInfo->stAiChnCfg.stChnParams, sizeof(AI_CHN_PARAM_S));

	return CVI_SUCCESS;
}


CVI_S32 CVI_AI_EnableReSmp(AUDIO_DEV AiDevId, AI_CHN AiChn,
			   AUDIO_SAMPLE_RATE_E enOutSampleRate)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ai_enable_resmp(AiDevId, AiChn,
						      enOutSampleRate);
	}
#endif
	//while entering this funciton , means the resample enable is toggled.
	//need to send the info from gstAIInstance[AiDevId] to cvi_resample.c
	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		log_error("\n");
		return CVI_ERR_AI_INVALID_DEVID;
	}

	if (enOutSampleRate == AUDIO_SAMPLE_RATE_BUTT) {
		log_error("RATE_BUTT do not do the resample\n");
		return CVI_ERR_AI_NOT_SUPPORT;
	}
	pthread_mutex_lock(&g_track_lock[AiChn]);
	ST_AI_INSTANCE *_ain_instatnce = &gstAiInstance[AiDevId];
	CVI_ST_AUD_TRACK_INFO *pstTrackInfo = _ain_instatnce->ppastTrackInfo[AiChn];

	if (CVIAUDIO_CHECK_NULL((void *)pstTrackInfo)) {
		pthread_mutex_unlock(&g_track_lock[AiChn]);
		return -1;
	}
	if (enOutSampleRate == (AUDIO_SAMPLE_RATE_E) pstTrackInfo->iSampleRate) {
		log_error("same sr %d\n", enOutSampleRate);
		pthread_mutex_unlock(&g_track_lock[AiChn]);
		return CVI_ERR_AI_ILLEGAL_PARAM;
	}

	if (CVI_AI_VQECheckEnable(AiDevId, AiChn))
		pstTrackInfo->iChannels = 1;

	pstTrackInfo->iSampleRate = enOutSampleRate;
	pstTrackInfo->pResHandle =  CVI_Resampler_Create(
					    _ain_instatnce->aio_attrs.enSamplerate,
					    pstTrackInfo->iSampleRate,
					    pstTrackInfo->iChannels);
	if (pstTrackInfo->pResHandle == CVI_NULL) {
		pthread_mutex_unlock(&g_track_lock[AiChn]);
		log_error("Create resample func failure..\n");
		return CVI_ERR_AI_NOMEM;
	}
	pstTrackInfo->b_need_resample = CVI_TRUE;
	pthread_mutex_unlock(&g_track_lock[AiChn]);
	log_info("[%d] to [%d]\n", _ain_instatnce->aio_attrs.enSamplerate, pstTrackInfo->iSampleRate);
	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_DisableReSmp(AUDIO_DEV AiDevId, AI_CHN AiChn)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ai_disable_resmp(AiDevId, AiChn);
	}
#endif
	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		log_error("\n");
		return CVI_ERR_AI_INVALID_DEVID;
	}

	if (AiChn < 0 || AiChn >= CVI_AUD_MAX_CHANNEL_NUM) {
		log_error("invalid AiChn:%d\n", AiChn);
		return CVI_ERR_AI_INVALID_CHNID;
	}
	pthread_mutex_lock(&g_track_lock[AiChn]);
	ST_AI_INSTANCE *_ain_instatnce = &gstAiInstance[AiDevId];
	CVI_ST_AUD_TRACK_INFO *pstTrackInfo = _ain_instatnce->ppastTrackInfo[AiChn];

	if (!_ain_instatnce->bEnableAI) {
		log_error("device %d is close\n", AiDevId);
		pthread_mutex_unlock(&g_track_lock[AiChn]);
		return CVI_ERR_AI_NOT_ENABLED;
	}

	if (CVIAUDIO_CHECK_NULL((void *)pstTrackInfo)) {
		pthread_mutex_unlock(&g_track_lock[AiChn]);
		return -1;
	}

	if (!pstTrackInfo->b_need_resample) {
		pthread_mutex_unlock(&g_track_lock[AiChn]);
		return CVI_ERR_AI_NOT_CONFIG;
	}

	if (pstTrackInfo->bInitOk) {
		if (pstTrackInfo->pResHandle) {
			CVI_Resampler_Destroy(pstTrackInfo->pResHandle);
			pstTrackInfo->pResHandle = NULL;
			pstTrackInfo->b_need_resample = CVI_FALSE;
		}
		pstTrackInfo->b_need_resample = CVI_FALSE;
	} else {
		log_error("chn %d is close\n", AiChn);
		pthread_mutex_unlock(&g_track_lock[AiChn]);
		return CVI_ERR_AI_NOT_ENABLED;
	}

	pthread_mutex_unlock(&g_track_lock[AiChn]);
	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_SetTrackMode(AUDIO_DEV AiDevId, AUDIO_TRACK_MODE_E enTrackMode)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32Retf = CVI_SUCCESS;
	CVI_S32 fdAcodec_adc = -1;
	CVI_S32 fdAcodec_dac = -1;

	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		log_error("\n");
		return CVI_ERR_AI_INVALID_DEVID;
	}

	/* Step 1 save the track mode to global instance */
	gstAiInstance[AiDevId].enTrackMode = enTrackMode;
	log_debug("settrack mode[%d]\n", (int)enTrackMode);

	/* Step 2 set the track mode through ioctl */
	fdAcodec_adc = open(ACODEC_ADC, O_RDWR);

	if (fdAcodec_adc < 0) {
		log_error("can't open Acodec,%s\n", ACODEC_ADC);
		return CVI_ERR_AI_SYS_NOTREADY;
	}

	fdAcodec_dac = open(ACODEC_DAC, O_RDWR);

	if (fdAcodec_dac < 0) {
		printf("%s: can't open Acodec,%s\n", __func__, ACODEC_DAC);
		return CVI_FAILURE;
	}

	switch (enTrackMode) {
	case AUDIO_TRACK_NORMAL:
		s32Ret = ioctl(fdAcodec_adc, ACODEC_SOFT_RESET_CTRL);

		if (s32Ret != CVI_SUCCESS) {
			/* failure */
			log_error("failure..ret[%d]\n", s32Ret);
			s32Retf = CVI_ERR_AI_SYS_NOTREADY;
		}

		break;

	case AUDIO_TRACK_BOTH_LEFT: {
		/* TODO: Need confirm usage */

		CVI_U32 input_mode = 0;

		if (ioctl(fdAcodec_adc, ACODEC_SET_MIXER_MIC, &input_mode)) {
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

		s32Ret = ioctl(fdAcodec_adc, ACODEC_SET_MIXER_MIC, &input_mode);

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

		s32Ret = ioctl(fdAcodec_adc, ACODEC_SET_MICL_MUTE, &mute_ctrl);

		if (s32Ret != CVI_SUCCESS) {
			/* failure */
			log_error("failure..ret[%d]\n", s32Ret);
			s32Retf = CVI_ERR_AI_SYS_NOTREADY;
		}

		break;
	}

	case AUDIO_TRACK_RIGHT_MUTE: {
		CVI_U32 mute_ctrl = 0x1;

		s32Ret = ioctl(fdAcodec_adc, ACODEC_SET_MICR_MUTE, &mute_ctrl);

		if (s32Ret != CVI_SUCCESS) {
			/* failure */
			log_error("failure..ret[%d]\n", s32Ret);
			s32Retf = CVI_ERR_AI_SYS_NOTREADY;
		}

		break;
	}

	case AUDIO_TRACK_BOTH_MUTE: {
		CVI_U32 mute_ctrl = 0x1;

		s32Ret = ioctl(fdAcodec_adc, ACODEC_SET_MICR_MUTE, &mute_ctrl);
		s32Ret = ioctl(fdAcodec_adc, ACODEC_SET_MICL_MUTE, &mute_ctrl);

		if (s32Ret != CVI_SUCCESS) {
			/* failure */
			log_error("failure..ret[%d]\n", s32Ret);
			s32Retf = CVI_ERR_AI_SYS_NOTREADY;
		}

		break;
	}

	case AUDIO_TRACK_BUTT:
		log_error("invalid option ...return\n");
		close(fdAcodec_adc);
		return CVI_ERR_AI_ILLEGAL_PARAM;

	/* break; */
	default:
		log_error("Not support this mode\n");
		break;
	}

	close(fdAcodec_adc);
	return s32Retf;
}

CVI_S32 CVI_AI_GetTrackMode(AUDIO_DEV AiDevId, AUDIO_TRACK_MODE_E *penTrackMode)
{

	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		log_error("\n");
		return CVI_ERR_AI_INVALID_DEVID;
	}

	*penTrackMode = gstAiInstance[AiDevId].enTrackMode;
	return CVI_SUCCESS;
}


CVI_S32 CVI_AI_SaveFile(AUDIO_DEV AiDevId, AI_CHN AiChn,
			const AUDIO_SAVE_FILE_INFO_S *pstSaveFileInfo)
{
	UNUSED_REF(AiDevId);
	UNUSED_REF(AiChn);
	UNUSED_REF(pstSaveFileInfo);
	return CVI_SUCCESS;
}
CVI_S32 CVI_AI_QueryFileStatus(AUDIO_DEV AiDevId, AI_CHN AiChn,
			       AUDIO_FILE_STATUS_S *pstFileStatus)
{
	UNUSED_REF(AiDevId);
	UNUSED_REF(AiChn);
	UNUSED_REF(pstFileStatus);
	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_GetFd(AUDIO_DEV AiDevId, AI_CHN AiChn)
{
	UNUSED_REF(AiDevId);
	UNUSED_REF(AiChn);

	log_error("This api not support\n");
	return CVI_FAILURE;
}

CVI_S32 CVI_AI_ClrPubAttr(AUDIO_DEV AiDevId)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ai_clr_pub_attr(AiDevId);
	}
#endif
	if (CHECK_AI_DEVID_VALID(AiDevId)) {
		log_error("\n");
		return CVI_ERR_AI_INVALID_DEVID;
	}

	ST_AI_INSTANCE *pain_instatnce = &gstAiInstance[AiDevId];

	memset(&pain_instatnce->aio_attrs, 0, sizeof(AIO_ATTR_S));
	return CVI_SUCCESS;
}



CVI_S32 CVI_AI_GetVolume(AUDIO_DEV AiDevId, CVI_S32 *ps32VolumeStep)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ai_get_volume(AiDevId, ps32VolumeStep);
	}
#endif
	ACODEC_VOL_CTRL vol_ctrl_L;
	ACODEC_VOL_CTRL vol_ctrl_R;
	CVI_S32 fdAcodec_adc = -1;

	UNUSED_REF(AiDevId);
	fdAcodec_adc = open(ACODEC_ADC, O_RDWR);

	if (fdAcodec_adc < 0) {
		log_error("%s: can't open Acodec,%s\n", __func__, ACODEC_ADC);
		if (fdAcodec_adc != -1)
			close(fdAcodec_adc);

		return CVI_FAILURE;
	}

	if (ioctl(fdAcodec_adc, ACODEC_GET_ADCL_VOL, &vol_ctrl_L))
		log_error("ioctl err!\n");

	if (ioctl(fdAcodec_adc, ACODEC_GET_ADCR_VOL, &vol_ctrl_R))
		log_error("ioctl err!\n");


	printf("fdAcodec_adc ACODEC_GET_ADCL_VOL mute[%d] [%d]ok!\n",
	       vol_ctrl_L.vol_ctrl_mute, vol_ctrl_L.vol_ctrl);
	printf("fdAcodec_adc ACODEC_GET_ADCR_VOL mute[%d] [%d]ok!\n",
	       vol_ctrl_R.vol_ctrl_mute, vol_ctrl_R.vol_ctrl);

	*ps32VolumeStep =  vol_ctrl_R.vol_ctrl;

	if (fdAcodec_adc)
		close(fdAcodec_adc);

	return CVI_SUCCESS;
}


CVI_S32 CVI_AI_SetVolume(AUDIO_DEV AiDevId, CVI_S32 s32VolumeStep)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_ai_set_volume(AiDevId, s32VolumeStep);
	}
#endif

	CVI_S32 fdAcodec_adc = -1;
	ACODEC_VOL_CTRL vol_ctrl;

	UNUSED_REF(AiDevId);
	printf("enter  ACODEC_SET_ADCL_VOL / ACODEC_SET_ADCR_VOL\n");

#ifdef ARCH_CV183X
	printf("(mic in) volume[7-0, 0:mute]: [%d]\n", s32VolumeStep);
	if (s32VolumeStep < 0 || s32VolumeStep > 7) {
		log_error("Invalid usage\n");
		log_error("s32VolumeStep  0(low)~7(max) integer number\n");
		return CVI_FAILURE;
	}
#else
	printf("(mic in) volume[24-0, 0:mute]: [%d]\n", s32VolumeStep);
	if (s32VolumeStep < 0 || s32VolumeStep > 24) {
		log_error("Invalid usage\n");
		log_error("s32VolumeStep  0(low)~24(max) integer number\n");
		return CVI_FAILURE;
	}
#endif

	fdAcodec_adc = open(ACODEC_ADC, O_RDWR);

	if (fdAcodec_adc < 0) {
		log_error("%s: can't open Acodec,%s\n", __func__, ACODEC_ADC);
		if (fdAcodec_adc != -1)
			close(fdAcodec_adc);

		return CVI_FAILURE;
	}

	vol_ctrl.vol_ctrl_mute = 0x0;

	if (s32VolumeStep == 0)
		vol_ctrl.vol_ctrl_mute = 1;
	else
		vol_ctrl.vol_ctrl_mute = 0;

	vol_ctrl.vol_ctrl = s32VolumeStep;
	if (ioctl(fdAcodec_adc, ACODEC_SET_ADCL_VOL, &vol_ctrl))
		log_error("ioctl err!\n");
	else {
		printf("fdAcodec_dac ACODEC_SET_ADCLR_VOL [%d]ok!\n", s32VolumeStep);
	}

	if (ioctl(fdAcodec_adc, ACODEC_SET_ADCR_VOL, &vol_ctrl))
		log_error("ioctl err!\n");
	else
		printf("fdAcodec_dac ACODEC_SET_ADCR_VOL [%d]ok!\n", s32VolumeStep);

	/* use command: ACODEC_SET_DACL_VOL, ACODEC_SET_DACR_VOL --end */
	if (fdAcodec_adc > 0)
		close(fdAcodec_adc);

	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_SetVqeVolume(AUDIO_DEV AiDevId, AI_CHN AiChn, CVI_S32 s32VolumeDb)
{

	int s32Ret = CVI_SUCCESS;
	ST_AI_INSTANCE *pstAiInstance = &gstAiInstance[AiDevId];

	if (!pstAiInstance->ppVolinstance[AiChn]) {
		log_warn("please call CVI_AI_EnableChn first\n");
		return CVI_FAILURE;
	}

	s32Ret = cvitek_set_volume_index(pstAiInstance->ppVolinstance[AiChn], s32VolumeDb);
	if (s32Ret != CVI_SUCCESS) {
		log_error("set vol error .ppVolinstance[%d], vol:%d\n", AiChn, s32VolumeDb);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_GetVqeVolume(AUDIO_DEV AiDevId, AI_CHN AiChn, CVI_S32 *ps32VolumeDb)
{

	ST_AI_INSTANCE *pstAiInstance = &gstAiInstance[AiDevId];

	if (!pstAiInstance->ppVolinstance[AiChn]) {
		log_warn("please call CVI_AI_EnableChn first\n");
		return CVI_FAILURE;
	}

	*ps32VolumeDb = cvitek_get_volume_index(pstAiInstance->ppVolinstance[AiChn]);
	if (*ps32VolumeDb < 0) {
		log_error("get vol error .ppVolinstance[%d], vol:%d\n", AiChn, *ps32VolumeDb);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}


