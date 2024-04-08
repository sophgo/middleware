#include<stdio.h>
#include <sys/prctl.h>
#include "cvi_audio_interface_tinyalsa.h"
#ifdef RPC_MULTI_PROCESS_AUDIO
#include "cvi_audio_rpc.h"
#endif
#include "cvi_audio_transcode.h"
#include "cvi_audio_common.h"
#include "cvi_ai_internal.h"

CVI_S32 s32SizeBytesAACEnc;
ST_AENC_INSTANCE gstAencInstance[AENC_MAX_CHN_NUM];
AAC_AENC_ENCODER_S gAACenc;
ST_AAC_ENC_INST  *gstpAacEncInstance;
//extern ST_AI_INSTANCE gstAiInstance[CVI_MAX_AI_DEVICE_ID_NUM];


static CVI_CHAR *_audio_type2Str(PAYLOAD_TYPE_E enType)
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
	else if (enType == PT_D_MP2P)
		return "mp2";
	else
		return "data";
}


CVI_S32 _parsing_aenc_channel_vqe_status(AUDIO_DEV AiDevId, AI_CHN AiChn, ST_AENC_INSTANCE *_aenc_instance)
{
	CVI_S32 s32CheckVqeFlag = 0x00;
	CVI_S32 s32Ret = CVI_SUCCESS;

	_aenc_instance->bVqeOn = CVI_AI_VQECheckEnable(AiDevId, AiChn);
	if (_aenc_instance->bVqeOn == CVI_FALSE) {
		s32Ret = CVI_FAILURE;
	} else {

		s32CheckVqeFlag = CVI_AI_VQECheckFlag(AiDevId, AiChn);
		if ((s32CheckVqeFlag & AI_TALKVQE_MASK_AEC) == AI_TALKVQE_MASK_AEC)
			_aenc_instance->bVqeWithAecOn = CVI_TRUE;
		else if (((s32CheckVqeFlag & AI_TALKVQE_MASK_AGC) == AI_TALKVQE_MASK_AGC) ||
			 ((s32CheckVqeFlag & AI_TALKVQE_MASK_ANR) == AI_TALKVQE_MASK_ANR))
			_aenc_instance->bVqeWithAecOn = CVI_FALSE;
		else {
			log_error("vqe on but mask unidentified dev[%d], AiChn[%d] mask[0x%x]\n",
				  AiDevId, AiChn, s32CheckVqeFlag);
			_aenc_instance->bVqeOn = CVI_FALSE;
			s32Ret = CVI_FAILURE;
		}

	}

	return s32Ret;
}


CVI_S32 CVI_AENC_Init(void)
{
	for (int i = 0 ; i < AENC_MAX_CHN_NUM; i++)
		memset(&gstAencInstance[i], 0, sizeof(ST_AENC_INSTANCE));


	return CVI_SUCCESS;
}

void CVI_AENC_Deinit(void)
{
	//CycleBufferDestroy(gpstCircleBuffer_AiEnc);
	//gpstCircleBuffer_AiEnc = CVI_NULL;

}


CVI_S32 CVI_AENC_SetMute(AENC_CHN AeChn, CVI_BOOL bEnable)
{
	gstAencInstance[AeChn].bMute = bEnable;
	log_debug("SET MUTE [%d]\n", (CVI_S32)bEnable);
	return CVI_SUCCESS;
}

CVI_S32 CVI_AENC_GetMute(AENC_CHN AeChn, CVI_BOOL *pbEnable)
{
	*pbEnable = gstAencInstance[AeChn].bMute;
	log_debug("Get MUTE [%d]\n", (CVI_S32)gstAencInstance[AeChn].bMute);
	return CVI_SUCCESS;
}

CVI_S32 CVI_AENC_CreateChn(AENC_CHN AeChn, const AENC_CHN_ATTR_S *pstAttr)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_aenc_create_chn(AeChn, pstAttr);
	}
#endif
	if (CHECK_AENC_DEVID_VALID(AeChn)) {
		log_error("\n");
		return CVI_ERR_AENC_INVALID_DEVID;
	}

	CVI_S32 s32Ret;

	ST_AENC_INSTANCE *_aenc_instance = &gstAencInstance[AeChn];

	_aenc_instance->AencHandle = CVI_NULL;
	_aenc_instance->EncInBuffSize = 0;

	memcpy(&gstAencInstance[AeChn].aenc_attrs, (AENC_CHN_ATTR_S *)pstAttr,
	       sizeof(AENC_CHN_ATTR_S));

	PAYLOAD_TYPE_E eInputType = gstAencInstance[AeChn].aenc_attrs.enType;

	_aenc_instance->EncBuff =
		(CVI_U8 *) malloc(_aenc_instance->aenc_attrs.u32BufSize * DEFAULT_MAX_FRM_SIZE);
	_aenc_instance->EncBuff_aec =
		(CVI_U8 *) malloc(_aenc_instance->aenc_attrs.u32BufSize * DEFAULT_MAX_FRM_SIZE);

	_aenc_instance->EncInBuffSize = CYCLE_BUFFER_SIZE;
	if (_aenc_instance->EncInBuff == CVI_NULL) {
		log_debug("EncInBuff allocate ...\n");
		_aenc_instance->EncInBuff = malloc(_aenc_instance->EncInBuffSize);
	} else
		log_debug("EncInBuff already allocate in aenc_ch_id[%d]!!!\n", AeChn);


	/* setup default bitrate to 16bit */
	_aenc_instance->cviAencConfig.bitrate = Rate16kBits;
	_aenc_instance->cviAencConfig.channel_num =
		gstAiInstance[gstAudStatus.AiIdxNow].aio_attrs.enSoundmode + 1;
	_aenc_instance->cviAencConfig.sample_rate =
		(int)gstAiInstance[gstAudStatus.AiIdxNow].aio_attrs.enSamplerate;


	//checking vqe status with this channel------------------------START
	CVI_BOOL bVqeOn = CVI_FALSE;
	CVI_S32 s32CheckVqeFlag = 0x00;

	for (CVI_S32 s32DevId = 0; s32DevId < CVI_MAX_AI_DEVICE_ID_NUM; s32DevId++) {
		bVqeOn |= CVI_AI_VQECheckEnable(s32DevId, AeChn);
		if (bVqeOn) {
			_aenc_instance->bVqeOn = CVI_TRUE;
			s32CheckVqeFlag = CVI_AI_VQECheckFlag(s32DevId, AeChn);
			if ((s32CheckVqeFlag & AI_TALKVQE_MASK_AEC) == AI_TALKVQE_MASK_AEC) {
				_aenc_instance->bVqeWithAecOn = CVI_TRUE;
				_aenc_instance->cviAencConfig.channel_num = DEFAULT_AEC_OUT_CHN_COUNT;
			} else if (((s32CheckVqeFlag & AI_TALKVQE_MASK_AGC) == AI_TALKVQE_MASK_AGC) ||
				((s32CheckVqeFlag & AI_TALKVQE_MASK_ANR) == AI_TALKVQE_MASK_ANR))
				_aenc_instance->bVqeWithAecOn = CVI_FALSE;
			else {
				//VQE MAKS is not valid(with unsupport mask) even when vqe flag on
				_aenc_instance->bVqeOn = CVI_FALSE;
			}
		}
	}
	//checking vqe status with this channel------------------------END


	if (eInputType == PT_G711A) {
		/* g-series speech codec*/
		_aenc_instance->eACodecType = AUD_CODEC_G711_ALAW;
	} else if (eInputType == PT_G711U) {
		/* g-series speech codec*/
		_aenc_instance->eACodecType = AUD_CODEC_G711_MLAW;
	} else if (eInputType == PT_G726) {
		/* g-series speech codec*/
		_aenc_instance->eACodecType = AUD_CODEC_G726;
	} else if (eInputType == PT_ADPCMA) {
		/* ADPCM speech codec*/
		_aenc_instance->eACodecType = AUD_CODEC_ADPCM_IMA;
	} else if ((eInputType == PT_DVI4_8K) || (eInputType == PT_DVI4_16K) ||
		   (eInputType == PT_DVI4_3) || (eInputType == PT_DVI4_4)) {
		/* ADPCM speech codec*/
		_aenc_instance->eACodecType = AUD_CODEC_ADPCM_DVI4;
	} else if (eInputType == PT_AAC) {
		/* AAC codec */
		log_debug("AAC  venc create\n");
		_aenc_instance->eACodecType = AUD_CODEC_AAC;
	} else {

		log_error("Not support transcode codec[%d][%d]\n",
			  (int)eInputType, (int)pstAttr->enType);
		goto RET_AENC_CREATE_FAIL;
	}


	switch (pstAttr->enType) {
	case PT_G711A:
		break;

	case PT_G711U:
		break;

	case PT_G726: {
		AENC_ATTR_G726_S *pValue = (AENC_ATTR_G726_S *)pstAttr->pValue;

		if ((pValue->enG726bps == MEDIA_G726_16K) || (pValue->enG726bps == G726_16K)) {
			log_debug("G726 bit 16k\n");
			_aenc_instance->cviAencConfig.bitrate = Rate16kBits;
		}

		if ((pValue->enG726bps == MEDIA_G726_24K) || (pValue->enG726bps == G726_24K)) {
			log_debug("G726 bit 24k\n");
			_aenc_instance->cviAencConfig.bitrate = Rate24kBits;
		}

		if ((pValue->enG726bps == G726_32K) || (pValue->enG726bps == MEDIA_G726_32K)) {
			log_debug("G726 bit 32k\n");
			_aenc_instance->cviAencConfig.bitrate = Rate32kBits;
		}

		if ((pValue->enG726bps == G726_40K) || (pValue->enG726bps == MEDIA_G726_40K)) {
			log_debug("G726 bit 40k\n");
			_aenc_instance->cviAencConfig.bitrate = Rate40kBits;
		}

		break;
	}

	case PT_ADPCMA:
		break;

	case PT_D_VDVI:
		break;

	case PT_AAC: {
		//check if AAC encoder / decoder handle exist
		//AENC_ATTR_AAC_S  *pstAencAac = (AENC_ATTR_AAC_S  *)pstAttr->pValue;
		ST_AENC_ATTR_AAC_S  *pstAencAac = (ST_AENC_ATTR_AAC_S *)pstAttr->pValue;

		log_debug("[%s][%d]aac type[%d]\n", __func__, __LINE__, pstAencAac->enAACType);
		log_debug("[%s][%d]aac enSmpRate[%d]\n", __func__, __LINE__, pstAencAac->enSmpRate);
		//adjust the type and size
		if (pstAencAac->enAACType == AAC_TYPE_AACLC) {
			s32SizeBytesAACEnc = AACLC_SAMPLES_PER_FRAME_DEFAULT;
			log_debug("AAC_TYPE_AACLC s32SizeBytes[%d]\n", s32SizeBytesAACEnc);
		} else if (pstAencAac->enAACType == AAC_TYPE_EAAC ||
			   pstAencAac->enAACType == AAC_TYPE_EAACPLUS) {
			s32SizeBytesAACEnc = AACPLUS_SAMPLES_PER_FRAME_DEFAULT;
			log_debug("AAC_TYPE_EAAC s32SizeBytes[%d]\n", s32SizeBytesAACEnc);
		} else if (pstAencAac->enAACType == AAC_TYPE_AACLD ||
			   pstAencAac->enAACType == AAC_TYPE_AACELD) {
			s32SizeBytesAACEnc = AACLD_SAMPLES_PER_FRAME_DEFAULT;
			log_debug("AAC_TYPE_AACELD s32SizeBytes[%d]\n", s32SizeBytesAACEnc);
		} else {
			printf("[Error][%s][%d].....\n", __func__, __LINE__);
			sleep(1);
		}

		s32SizeBytesAACEnc = s32SizeBytesAACEnc * DEFAULT_BYTES_PER_SAMPLE;

		if (gstpAacEncInstance != NULL &&  gAACenc.pfnOpenEncoder != NULL) {
			log_debug("Prepare Create AAC aenc channel\n");
		} else {
			log_debug("create AAC enc channel failure\n");
			goto RET_AENC_CREATE_FAIL;
		}

		break;
	}

	case PT_DVI4_8K:
	case PT_DVI4_16K:
		break;

	default:
		log_error("Not support encode type\n");
		break;
	}

	//test for aenc file mode
	if (_aenc_instance->aenc_attrs.bFileDbgMode == CVI_TRUE) {

		_aenc_instance->fd_aenc_input = CVI_NULL;

		_aenc_instance->fd_aenc_input = open("fd_aenc_in.raw", O_WRONLY + O_CREAT, 0644);

		if ((_aenc_instance->fd_aenc_input == -1) || (_aenc_instance->fd_aenc_input == CVI_NULL)) {
			log_error("fd_aenc_input failure !!\n");
			goto RET_AENC_CREATE_FAIL;
		}

		CVI_CHAR FileName[256] = {0};

		snprintf(FileName, 256, "fd_aenc_out_chn%d.%s",
			 (int)AeChn,
			 _audio_type2Str(_aenc_instance->aenc_attrs.enType));
		_aenc_instance->fd_aenc_out = fopen(FileName, "wb");
		if (!_aenc_instance->fd_aenc_out) {
			log_error("fd_aenc failure !!\n");
			goto RET_AENC_CREATE_FAIL;
		}
	}

	if (pstAttr->enType != PT_AAC) {
		_aenc_instance->AencHandle = CVI_AUDIO_Transcode_Init(NULL,
					     _aenc_instance->eACodecType,
					     &_aenc_instance->cviAencConfig);

		if (_aenc_instance->AencHandle == CVI_NULL) {
			log_error("transcode INIT setting error ....\n");
			goto RET_AENC_CREATE_FAIL;
		}
	} else {
		//parse aac config
		ST_AENC_ATTR_AAC_S	*stAencAac = (ST_AENC_ATTR_AAC_S *)pstAttr->pValue;
		ST_AENC_ATTR_AAC_S	aacenc_config;

		aacenc_config.enTransType = stAencAac->enTransType;
		aacenc_config.enSmpRate = stAencAac->enSmpRate;
		aacenc_config.enSoundMode = stAencAac->enSoundMode;
		aacenc_config.enBitWidth = stAencAac->enBitWidth;
		aacenc_config.enBitRate = stAencAac->enBitRate;
		aacenc_config.enAACType = stAencAac->enAACType;
		aacenc_config.s16BandWidth = (int)aacenc_config.enSmpRate / 2;

		if (aacenc_config.enSmpRate > 32000)
			aacenc_config.enBitRate = 48000;

		printf("[%s][%d]create start\n", __func__, __LINE__);
		printf("gAACenc.enType[%d]\n", gAACenc.enType);
		log_info("soundmode:%d, sample_rate:%d, enBitWidth:%d, enBitRate:%d, enAACType:%d, s16BandWidth:%d\n",
			 aacenc_config.enSoundMode, aacenc_config.enSmpRate, aacenc_config.enBitWidth,
			 aacenc_config.enBitRate, aacenc_config.enAACType, aacenc_config.s16BandWidth);

		s32Ret = gAACenc.pfnOpenEncoder((CVI_VOID *)(&aacenc_config)
						, (CVI_VOID **)(&_aenc_instance->AencHandle));


		if (s32Ret != CVI_SUCCESS) {
			log_error("AAC openEncoder failure\n");
			//return CVI_FAILURE;
			goto RET_AENC_CREATE_FAIL;
		}

	}

	_aenc_instance->bEnableAenc = CVI_TRUE;
	_aenc_instance->s32EncodedDataBytes = 0;


	_aenc_instance->iAencShmMemIndex = share_cyclebuffer_init(CYCLE_BUFFER_SIZE, 0);
	if (_aenc_instance->iAencShmMemIndex < 0) {
		log_error("query share buffer failed.\n");
		share_cyclebuffer_destroy(_aenc_instance->iAencShmMemIndex);
		goto RET_AENC_CREATE_FAIL;
	}
	log_info("AencIndex:%d, AeChn:%d\n", _aenc_instance->iAencShmMemIndex, AeChn);
	return CVI_SUCCESS;

RET_AENC_CREATE_FAIL:
	SAFE_FREE_BUF(_aenc_instance->EncBuff);
	SAFE_FREE_BUF(_aenc_instance->EncBuff_aec);
	SAFE_FREE_BUF(_aenc_instance->EncInBuff);
	return CVI_FAILURE;
}

CVI_S32 CVI_AENC_DestroyChn(AENC_CHN AeChn)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_aenc_destroy_chn(AeChn);
	}
#endif

	if (CHECK_AENC_DEVID_VALID(AeChn)) {
		log_error("\n");
		return CVI_ERR_AENC_INVALID_DEVID;
	}

	CVI_S32 s32Ret = CVI_FAILURE;
	ST_AENC_INSTANCE *_aenc_instance = &gstAencInstance[AeChn];

	_aenc_instance->bEnableAenc = CVI_FALSE;
	_aenc_instance->bVqeOn = CVI_FALSE;
	_aenc_instance->bVqeWithAecOn = CVI_FALSE;

	if (_aenc_instance->aenc_attrs.bFileDbgMode == CVI_TRUE) {
		close(_aenc_instance->fd_aenc_input);
		fclose(_aenc_instance->fd_aenc_out);
		_aenc_instance->fd_aenc_input = CVI_NULL;
		_aenc_instance->fd_aenc_out = CVI_NULL;
	}

	SAFE_FREE_BUF(_aenc_instance->EncBuff);
	SAFE_FREE_BUF(_aenc_instance->EncBuff_aec);
	SAFE_FREE_BUF(_aenc_instance->EncInBuff);
	//SAFE_FREE_BUF(_aenc_instance->EncVqeBuff);
	//SAFE_FREE_BUF(_aenc_instance->aenc_attrs.pValue); ?
	_aenc_instance->EncInBuffSize = 0;

	memset(&_aenc_instance->aenc_attrs, 0, sizeof(AENC_CHN_ATTR_S));

	if (_aenc_instance->eACodecType == AUD_CODEC_AAC) {
#ifndef CVIAUDIO_MW_STR_MODE
		if (gstpAacEncInstance != NULL && gAACenc.pfnCloseEncoder != NULL) {

			s32Ret = gAACenc.pfnCloseEncoder(_aenc_instance->AencHandle);

			if (s32Ret != CVI_SUCCESS)
				log_error("\n");
			else
				log_debug("AAC enc destroy\n");
		} else {
			//AAC encoder instance not create yet ...
			log_error("AAC Aenc not create success yet... cannot destroy\n");
		}
#endif
	} else {
		s32Ret = CVI_AUDIO_Transcode_DeInit(_aenc_instance->AencHandle,
						    _aenc_instance->eACodecType);
	}

	if (s32Ret != CVI_SUCCESS) {
		log_error("[%d]\n", s32Ret);
		return CVI_FAILURE;
	}

	if (_aenc_instance->iAencShmMemIndex > 0)
		share_cyclebuffer_destroy(_aenc_instance->iAencShmMemIndex);
	else {
		log_error("Aechn[%d], index[%d] please call CVI_AENC_CreateChn first.\n",
				AeChn, _aenc_instance->iAencShmMemIndex);
		return CVI_ERR_AENC_NOT_CONFIG;
	}

	_aenc_instance->AencHandle = CVI_NULL;
	memset(&gstAencInstance[AeChn], 0, sizeof(ST_AENC_INSTANCE));

	log_debug("AeChn[%d]----------------------->\n", AeChn);
	return CVI_SUCCESS;
}

CVI_S32 CVI_AENC_SendFrame(AENC_CHN AeChn, const AUDIO_FRAME_S *pstFrm,
			   const AEC_FRAME_S *pstAecFrm)
{
	//log_debug("enter [%d]\n", AeChn);
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		return rpc_client_aud_aenc_send_frame(AeChn, pstFrm, pstAecFrm);
	}
#endif

	if (CHECK_AENC_DEVID_VALID(AeChn)) {
		log_error("\n");
		return CVI_ERR_AENC_INVALID_DEVID;
	}

	UNUSED_REF(pstAecFrm);
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32TimeOutCnt = 0;
	CVI_S32 s32InputBytes;
	ST_AENC_INSTANCE *_aenc_instance = &gstAencInstance[AeChn];


	if (CVIAUDIO_CHECK_NULL((void *)_aenc_instance->AencHandle))
		return -1;

	if (_aenc_instance->stBindinfo.bBind == CVI_TRUE) {
		log_error("[you can not call this function] Ain bind AENC ongoing..\n");
		return CVI_ERR_AENC_NOT_PERM;
	}

	s32InputBytes  = _aenc_instance->cviAencConfig.channel_num * DEFAULT_BYTES_PER_SAMPLE;
	s32InputBytes = s32InputBytes * pstFrm->u32Len;

	_aenc_instance->s32SendBytePeriod = s32InputBytes;

	if (_aenc_instance->s32SendBytePeriod == 0) {
		log_info("Resample first Data.\n");
		return CVI_SUCCESS;
	}

	do {
		s32Ret = share_cyclebuffer_server_write(_aenc_instance->iAencShmMemIndex,
							(char *)pstFrm->u64VirAddr[0], s32InputBytes);
		if (s32Ret == 0) {
			usleep(1000 * 10);
			s32TimeOutCnt++;

			if (s32TimeOutCnt >= 3) {
				log_error("soundmode:%d,framelen:%d\n", pstFrm->enSoundmode, pstFrm->u32Len);
				break;
			}
		} else if (s32Ret < 0) {
			log_error("write error, shmMem:%d ret:%d\n", _aenc_instance->iAencShmMemIndex, s32Ret);
			return CVI_FALSE;
		}

	} while (s32Ret == 0);

	return CVI_SUCCESS;
}


CVI_S32 CVI_AENC_SaveFile(AENC_CHN AeChn, const AUDIO_SAVE_FILE_INFO_S *pstSaveFileInfo)
{
	if (CHECK_AENC_DEVID_VALID(AeChn)) {
		log_error("\n");
		return CVI_ERR_AENC_INVALID_DEVID;
	}
	UNUSED_REF(AeChn);
	UNUSED_REF(pstSaveFileInfo);
	return CVI_SUCCESS;
}

CVI_S32 CVI_AENC_QueryFileStatus(AENC_CHN AeChn, AUDIO_FILE_STATUS_S *pstFileStatus)
{
	if (CHECK_AENC_DEVID_VALID(AeChn)) {
		log_error("\n");
		return CVI_ERR_AENC_INVALID_DEVID;
	}

	UNUSED_REF(AeChn);
	UNUSED_REF(pstFileStatus);
	return CVI_SUCCESS;
}


CVI_S32 CVI_AENC_GetStream(AENC_CHN AeChn, AUDIO_STREAM_S *pstStream,
			   CVI_S32 s32MilliSec)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		return rpc_client_aud_aenc_get_stream(AeChn, pstStream, s32MilliSec);
	}
#endif
	if (CHECK_AENC_DEVID_VALID(AeChn)) {
		log_error("\n");
		return CVI_ERR_AENC_INVALID_DEVID;
	}

	CVI_BOOL bBlockMOde = CVI_FALSE;
	ST_AENC_INSTANCE *_aenc_instance = &gstAencInstance[AeChn];
	CVI_S32 s32Ret = 0, s32SizeBytes = 0, s32TimeOut = 0;
	CVI_S32 s32Buffbyte = 0;
	CVI_S32 u32SleepMs = 0;
	unsigned long long ullTimePivotStart = 0;

	pstStream->u32Len = 0;
	ullTimePivotStart = _get_current_time(); /* in milisecond */

	s32MilliSec = -1;
	if (s32MilliSec < 0) {
		bBlockMOde = CVI_TRUE;
		s32TimeOut = 20 * 1000;
	} else if (s32MilliSec == 0) {
		bBlockMOde = CVI_FALSE;
		s32TimeOut = 0;
	} else {
		bBlockMOde = CVI_TRUE;
		s32TimeOut = s32MilliSec;
	}


	if (_aenc_instance->eACodecType == AUD_CODEC_AAC) {
		s32SizeBytes = s32SizeBytesAACEnc * _aenc_instance->cviAencConfig.channel_num;
	} else {
		s32SizeBytes = _aenc_instance->s32SendBytePeriod;
		if (s32SizeBytes == 0)
			goto NO_DATA_HANDLE;
	}

	_aenc_instance->encInbyte = s32SizeBytes;
	s32Buffbyte = share_cyclebuffer_client_read(_aenc_instance->iAencShmMemIndex,
			_aenc_instance->EncInBuff, s32SizeBytes, s32TimeOut);

	if (s32Buffbyte < 0) {
		log_error("read failed,index:%d,ret:%d,buflen:%d\n", _aenc_instance->iAencShmMemIndex,
				s32Buffbyte, share_cyclebuffer_frameready_size(_aenc_instance->iAencShmMemIndex));
		goto NO_DATA_HANDLE;
	}

	//step 3: start audio encode process
	if (s32Buffbyte > 0) {

		CVI_S32 s32InputBytes  = s32SizeBytes;
		CVI_S32 s32OutLenBytes = 0;


		_check_and_dump("/tmp/dump_aenc_in", _aenc_instance->EncInBuff, s32SizeBytes);
		if (_aenc_instance->eACodecType == AUD_CODEC_AAC) {
			//AAC using external lib & api
			s32Ret = gAACenc.pfnEncodeFrm(_aenc_instance->AencHandle,
						      (CVI_S16 *)_aenc_instance->EncInBuff,
						      (CVI_U8 *)_aenc_instance->EncBuff,
						      s32InputBytes,
						      (CVI_U32 *)&s32OutLenBytes);
			if ((s32OutLenBytes == 0) && (s32InputBytes != 0)) {
				log_error("aac get size 0,s32InputBytes:%d\n", s32InputBytes);
				pstStream->u32Len = s32OutLenBytes; // size in bytes;
				pstStream->pStream = _aenc_instance->EncBuff;
				return CVI_SUCCESS;
			}

			if (s32Ret != CVI_SUCCESS) {
				log_error("AAC_ENCODER error\n");
				return CVI_FAILURE;
			}

		} else {
			//not AAC encode
			s32Ret = CVI_AUDIO_Encode(_aenc_instance->AencHandle,
						  _aenc_instance->eACodecType,
						  (CVI_VOID *)_aenc_instance->EncInBuff,
						  (CVI_VOID *)_aenc_instance->EncBuff,
						  s32InputBytes,
						  &s32OutLenBytes);

			if (s32Ret != CVI_SUCCESS) {
				log_error("encode error[%d]\n", s32Ret);
				return CVI_ERR_AENC_ENCODER_ERR;
			}

			if ((s32OutLenBytes == 0) && (s32InputBytes != 0)) {
				log_error("Fatal error in aenc..return size 0\n");
				return CVI_ERR_AENC_ENCODER_ERR;
			}


		}

		/* Check the aenc total size and decide to save to file or not --start */
		_aenc_instance->s32EncodedDataBytes += s32OutLenBytes;
		_aenc_instance->encOutbyte = s32OutLenBytes;
		_check_and_dump("/tmp/dump_aenc_out", (char *)_aenc_instance->EncBuff, s32OutLenBytes);
		pstStream->u32Len = s32OutLenBytes; // size in bytes;
		pstStream->pStream = _aenc_instance->EncBuff;

		if (_aenc_instance->aenc_attrs.bFileDbgMode) {
			write(_aenc_instance->fd_aenc_input, _aenc_instance->EncInBuff, s32InputBytes);
			//log_debug("[%s] write [%d]\n", __func__, s32OutLenBytes);
			fwrite(_aenc_instance->EncBuff, 1, s32OutLenBytes, _aenc_instance->fd_aenc_out);
		}
		//encode the original data to encoded data ------ end

	}

NO_DATA_HANDLE:
	pstStream->u64TimeStamp = _get_current_pts();

	if (pstStream->u32Len == 0) {
		if (bBlockMOde == CVI_FALSE) {
			//log_error("Non-block mode, size 0 ...return\n");
			log_info("no block mode\n");
			return CVI_SUCCESS;
			/* check condition */
		} else {
			/* block mode */
			if (s32TimeOut > 0) {
				if ((CVI_S32)(_get_current_time() - ullTimePivotStart) > s32TimeOut) {
					log_error("TIME OUT > [%d]\n", s32TimeOut);
					if (_aenc_instance->aenc_attrs.enType == PT_AAC)
						printf("type AAC check 1\n");
					if (_aenc_instance->eACodecType == AUD_CODEC_AAC)
						printf("type AAC check 2\n");

					log_error("TimeOut info req[%d] bufflevel[%d] u32SleepMs[%d]\n",
						  s32SizeBytes,
						  share_cyclebuffer_frameready_size(_aenc_instance->iAencShmMemIndex),
						  u32SleepMs);
					return CVI_FAILURE;
				}
			} else {
				log_error("Not suggetst using blocking mode\n");
				/* go check again */
			}

		}
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_AENC_ReleaseStream(AENC_CHN AeChn,
			       const AUDIO_STREAM_S *pstStream)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		return rpc_client_aud_aenc_release_stream(AeChn, pstStream);
	}
#endif
	if (CHECK_AENC_DEVID_VALID(AeChn)) {
		log_error("\n");
		return CVI_ERR_AENC_INVALID_DEVID;
	}

	ST_AENC_INSTANCE *_aenc_instance = (ST_AENC_INSTANCE *)&gstAencInstance[AeChn];

	if (_aenc_instance->AencHandle == CVI_NULL) {
		log_debug("AencHandle already NULL\n");
		return CVI_ERR_AENC_NULL_PTR;
	}

	if (pstStream->u32Len == 0) {
		//PRINTF("CVI_AENC_ReleaseStream size 0 !!!!\n");
		usleep(1000);
		return CVI_SUCCESS;
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_AENC_GetFd(AENC_CHN AeChn)
{
	/* No actual hw device, this api do not need to exist */
	UNUSED_REF(AeChn);
	log_error("This function not support\n");
	return CVI_FAILURE;
}

CVI_S32 CVI_AENC_RegisterEncoder(CVI_S32 *ps32Handle,
				 const AAC_AENC_ENCODER_S *pstEncoder)
{
	/* This is for AAC usage, other codec did not go this function */
	/* For AAC attach function pointer	*/
	UNUSED_REF(ps32Handle);
	UNUSED_REF(pstEncoder);

	log_error("Please using api:CVI_AENC_RegisterExternalEncoder instead\n");
	return CVI_SUCCESS;
}
CVI_S32 CVI_AENC_UnRegisterEncoder(CVI_S32 s32Handle)
{
	/* This is for AAC usage, other codec did not go this function */
	/* For AAC detach function pointer	*/
	UNUSED_REF(s32Handle);
	log_error("Please using api:CVI_ADEC_UnRegisterExternalDecoder instead\n");

	return CVI_SUCCESS;
}

CVI_S32 CVI_AENC_GetStreamBufInfo(AENC_CHN AeChn, CVI_U64 *pu64PhysAddr,
				  CVI_U32 *pu32Size)
{
	ST_AENC_INSTANCE *_aenc_instance = &gstAencInstance[AeChn];
	UNUSED_REF(pu64PhysAddr);
	*pu32Size = share_cyclebuffer_frameready_size(_aenc_instance->iAencShmMemIndex);
	if (*pu32Size == 0)
		log_debug("Not data in encoder cycle buffer\n");

	return CVI_SUCCESS;
}


CVI_S32 CVI_AENC_RegisterExternalEncoder(CVI_S32 *ps32Handle,
		const AAC_AENC_ENCODER_S *pstEncoder)
{
	if (gstpAacEncInstance == NULL)
		gstpAacEncInstance = calloc(1, sizeof(ST_AAC_ENC_INST));


	AAC_AENC_ENCODER_S *plocal = (AAC_AENC_ENCODER_S *)pstEncoder;

	gAACenc.enType = plocal->enType;
	gAACenc.u32MaxFrmLen =  plocal->u32MaxFrmLen;
	gAACenc.pfnOpenEncoder = plocal->pfnOpenEncoder;
	gAACenc.pfnEncodeFrm = plocal->pfnEncodeFrm;
	gAACenc.pfnCloseEncoder = plocal->pfnCloseEncoder;

	*ps32Handle = plocal->enType;
	//printf("entype[%d]\n", plocal->enType);
	//printf("u32MaxFrmLen[%d]\n", plocal->u32MaxFrmLen);
	//printf("[%s][%d]\n", __func__, __LINE__);
	//sleep(1);
	return CVI_SUCCESS;
}

CVI_S32 CVI_AENC_UnRegisterExternalEncoder(CVI_S32 s32Handle)
{
	UNUSED_REF(s32Handle);
	if (gstpAacEncInstance != NULL) {
		free(gstpAacEncInstance);
		gstpAacEncInstance = NULL;
	}
	s32Handle = -1;

	return CVI_SUCCESS;
}

#ifdef BIND_AAC_VQE_RES
static CVI_VOID _RecalculateAAC_EncodeSize(AENC_CHN AeChn, CVI_S32 *pAacEncFormatSizeBytes)
{
	CVI_S32 s32SrcSampleRate = 0;
	CVI_S32 s32TargetSampleRate = 0;
	CVI_S32 s32InputSize = *pAacEncFormatSizeBytes;

	if ((gstBindConfig.bAIbindAENC == CVI_TRUE) && (gstBindConfig.DestChnId == (CVI_S32)AeChn)) {
		ST_AI_INSTANCE *_ai_instance = &gstAiInstance[gstBindConfig.SrcDevId];
		ST_AI_CHANNEL_CONFIG *pstAinChnCfg = &_ai_instance->stChnConfig[gstBindConfig.SrcChnId];

		if (pstAinChnCfg->bResampleChn) {
			s32SrcSampleRate = (CVI_S32)pstAinChnCfg->inSmpRate;
			s32TargetSampleRate = (CVI_S32)pstAinChnCfg->outSmpRate;
			if (s32SrcSampleRate != s32TargetSampleRate) {
				*pAacEncFormatSizeBytes = (s32InputSize * s32SrcSampleRate) / s32TargetSampleRate;
			}
		}
	}
}
#endif


CVI_S32 CVI_BitOut_Transfer(CVI_CHAR **InPt, CVI_U32 *pu32InSizeByte, CVI_CHAR **OutPt,
			    CVI_U32 *pu32OutSizeByte, CVI_S32 s32BitOut)
{
	/* (*InPt)	=  input buffer addr*/
	/* (*OutPt) = output buffer addr */
	/* *pu32InSize = input buffer size */
	/* *pu32OutSize = output buffer size */
	/*	s32BitOut target bitdepth to transfer only support 24 bit / 32 bit */
	/* if success return  0, else return < 0 */
	CVI_U32 *pt32 = (CVI_U32 *)(*OutPt);
	CVI_U16 *pt16 = (CVI_U16 *)(*InPt);
	CVI_U8	*pt8 = (CVI_U8 *)(*OutPt);
	CVI_U32 u32ArraySize32 = 0;
	CVI_U32 u32ArraySize16 = 0;
	//CVI_U32 u32ArraySize24 = 0;


	if (*InPt == NULL) {
		log_error("Send in Null buffer in [%s]\n", __func__);
		return CVI_ERR_AI_NULL_PTR;
	}

	if (*pu32InSizeByte == 0) {
		log_error("No data size require to transfer [%s]\n", __func__);
		return CVI_ERR_AIO_ILLEGAL_PARAM;
	}

	u32ArraySize32 = (*pu32InSizeByte) / 2;
	u32ArraySize16 = (*pu32InSizeByte) / 2;
	//u32ArraySize24 = (u32ArraySize16 / 4) * 3;

	if (s32BitOut == 24) {
		*pu32OutSizeByte = ((*pu32InSizeByte) / 2) * 3;
		CVI_U32 index = 0;

		while (index < u32ArraySize16) {
			*pt32 = *pt16;
			*pt32 = *pt32 << 8;
			pt8 = pt8 + 3; /* shift 24bit */
			pt32 = (CVI_U32 *)pt8;
			pt16 += 1;
			index += 1;
		}

		goto SUC_RET;
	} else if (s32BitOut == 32) {
		*pu32OutSizeByte = (*pu32InSizeByte) * 2;

		while (u32ArraySize32 > 0) {
			*pt32 = *pt16;
			*pt32 = *pt32 << 16;/* little endian */
			pt16 += 1;
			pt32 += 1;
			u32ArraySize32 -= 1;
		}

		goto SUC_RET;
	} else {
		log_error("s32BitOut only support 24 or 32\n");
		return CVI_ERR_AIO_ILLEGAL_PARAM;
	}

SUC_RET:
	return CVI_SUCCESS;
}

