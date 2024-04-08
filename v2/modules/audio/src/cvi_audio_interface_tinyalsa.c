/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_audio_interface.c
 * Description:
 *	 audio main api interface  functions.
 */

#include<stdio.h>
#include <sys/prctl.h>
#include "cvi_audio_arch.h"
#include "cvi_audio_common.h"
#include "cvi_ao_internal.h"
#include "cvi_ai_internal.h"
#include "cvi_audio_interface_tinyalsa.h"

//global variable need to use share memory--------------->start

ST_CODEC_CONFIG gstCodecConfig;
AudInnerStatus gstAudStatus;
//extern ST_AI_INSTANCE gstAiInstance[CVI_MAX_AI_DEVICE_ID_NUM];
//global variable need to use share memory---------------->end

//global variable as specific config, no need share memory----->start

ST_AIN_THREAD_CFG stThreadCfg[CVI_MAX_AI_DEVICE_ID_NUM];
//global variable as specific config, no need share memory------->end
CVI_S32 cviaud_dbg = CVI_AUD_MASK_ERR;
//extern ST_AO_INSTANCE gstAoInstance[CVI_MAX_AO_DEVICE_ID_NUM];
//-------define toolkit function for measurement or debug

//support STR in cviaudio-----------------start
//#define CVIAUDIO_MW_STR_MODE 1 //turn off in default
#ifdef CVIAUDIO_MW_STR_MODE
typedef struct _ST_CVIAUDIO_STR {
	ST_AI_INSTANCE AinStr[CVI_MAX_AI_DEVICE_ID_NUM];
	ST_AO_INSTANCE AoutStr[CVI_MAX_AO_DEVICE_ID_NUM];
	ST_AENC_INSTANCE AencStr[AENC_MAX_CHN_NUM];
	ST_ADEC_INSTANCE AdecStr[ADEC_MAX_CHN_NUM];
	ST_BIND_CONFIG AudBindStr;
	AI_TALKVQE_CONFIG_S uplinkVqeAttr;
	ST_BIND_CONFIG bindconfig;
} ST_CVIAUDIO_MW_STR_MODE;

ST_CVIAUDIO_MW_STR_MODE gstAudStr;
#endif
//support STR in cviaudio------------------end

#ifdef BIND_AAC_VQE_RES
static CVI_VOID _RecalculateAAC_EncodeSize(AENC_CHN AeChn, CVI_S32 *pAacEncFormatSizeBytes);
#endif

CVI_S32 CVI_AUDIO_Suspend(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	printf("Enter[CVI_AUDIO_Suspend]------------------->\n");
#ifndef CVIAUDIO_MW_STR_MODE
	printf("CVI_AUDIO_Suspend not support in MW ...\n");
	printf("User should turn off / on Aud on application while suspend / resume\n");
#else
	CVI_S32 s32DevIndex = 0;
	CVI_S32 s32ChnIndex = 0;
	//step 1 check uplink(aenc/ain), destroy channel(aenc/ain)
	for (s32ChnIndex = 0; s32ChnIndex < AENC_MAX_CHN_NUM; s32ChnIndex++) {
		ST_AENC_INSTANCE *_aenc_instance = &gstAencInstance[s32ChnIndex];

		memcpy(&gstAudStr.AencStr[s32ChnIndex], &gstAencInstance[s32ChnIndex], sizeof(ST_AENC_INSTANCE));
		if (_aenc_instance->bEnableAenc == CVI_TRUE) {
			printf("cviaudio suspend aenc dev[%d] bEnableAenc[%d] bVqeOn[%d] bVqeWithAecOn[%d]\n",
			       s32ChnIndex,
			       _aenc_instance->bEnableAenc,
			       _aenc_instance->bVqeOn,
			       _aenc_instance->bVqeWithAecOn);
			CVI_AENC_DestroyChn(s32ChnIndex);
		}
	}

	for (s32DevIndex = 0; s32DevIndex < CVI_MAX_AI_DEVICE_ID_NUM; s32DevIndex++) {
		ST_AI_INSTANCE *_ai_instance = &gstAiInstance[s32DevIndex];

		memcpy(&gstAudStr.AinStr[s32DevIndex], _ai_instance, sizeof(ST_AI_INSTANCE));
		if (_ai_instance->bEnableAI == CVI_TRUE) {
			printf("cviaudio suspend ain devId[%d]=========\n", s32DevIndex);
			for (s32ChnIndex = 0; s32ChnIndex < CVI_AUD_MAX_CHANNEL_NUM; s32ChnIndex++) {
				ST_AI_CHANNEL_CONFIG *_pstChnConfig = &_ai_instance->stChnConfig[s32ChnIndex];

				if (_pstChnConfig->bChannelEnable == CVI_TRUE) {
					printf("ain enable chn[%d] resmp[%d] bEnableVqe[%d] bVqeWithAecOn[%d]\n",
					       s32ChnIndex,
					       _pstChnConfig->bResampleChn,
					       _pstChnConfig->bEnableVqe,
					       _pstChnConfig->bVqeWithAecOn);
					if (_pstChnConfig->bResampleChn)
						CVI_AI_DisableReSmp(s32DevIndex, s32ChnIndex);
					if (_pstChnConfig->bEnableVqe) {
						CVI_AI_GetTalkVqeAttr(s32DevIndex, s32ChnIndex,
								      &gstAudStr.uplinkVqeAttr);
						CVI_AI_DisableVqe(s32DevIndex, s32ChnIndex);
					}
					CVI_AI_DisableChn(s32DevIndex, s32ChnIndex);
				}
			}
			CVI_AI_Disable(s32DevIndex);
			printf("cviaudio suspend ain===================\n");
		}
	}
	//step 2 check downlink(aout/adec), destroy channel(aout/adec)
	for (s32DevIndex = 0; s32DevIndex < CVI_MAX_AO_DEVICE_ID_NUM; s32DevIndex++) {
		ST_AO_INSTANCE *_aout_instance = &gstAoInstance[s32DevIndex];

		memcpy(&gstAudStr.AoutStr[s32DevIndex], _aout_instance, sizeof(ST_AO_INSTANCE));
		if (_aout_instance->bEnableAO == CVI_TRUE) {
			printf("cviaudio suspend aout devId[%d]=========\n", s32DevIndex);
			for (s32ChnIndex = 0; s32ChnIndex < CVI_AUD_MAX_CHANNEL_NUM; s32ChnIndex++) {
				ST_AO_CHANNEL_CONFIG *_pstChnConfig = &_aout_instance->stChnConfig[s32ChnIndex];

				if (_pstChnConfig->bChannelEnable == CVI_TRUE) {
					printf("aout enable chn[%d] resmp[%d]\n",
					       s32ChnIndex,
					       _pstChnConfig->bResampleChn);
					if (_pstChnConfig->bResampleChn == CVI_TRUE)
						CVI_AO_DisableReSmp(s32DevIndex, s32ChnIndex);

					CVI_AO_DisableChn(s32DevIndex, s32ChnIndex);
				}

			}
			CVI_AO_Disable(s32DevIndex);
			printf("cviaudio suspend aout===================\n");
		}
	}
	for (s32ChnIndex = 0; s32ChnIndex < ADEC_MAX_CHN_NUM; s32ChnIndex++) {
		ST_ADEC_INSTANCE *_adec_instance = &gstAdecInstance[s32ChnIndex];

		memcpy(&gstAudStr.AdecStr[s32ChnIndex], _adec_instance, sizeof(ST_ADEC_INSTANCE));
		if (_adec_instance->bEnableAdec == CVI_TRUE) {
			printf("cviaudio suspend adec dev[%d] bEnableAdec[%d]\n",
			       s32ChnIndex,
			       _adec_instance->bEnableAdec);
			CVI_ADEC_DestroyChn(s32ChnIndex);
		}
	}
	//step 3  check bind mode
	//no need to check bind config
	memcpy(&gstAudStr.bindconfig, &gstBindConfig, sizeof(ST_BIND_CONFIG));

	//step 4 check the ain/aout volume

#endif
	printf("Leave[CVI_AUDIO_Suspend]<-------------------\n");

	return s32Ret;
}

CVI_S32 CVI_AUDIO_Resume(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	printf("Enter[CVI_AUDIO_Resume]---------------->\n");

#ifndef CVIAUDIO_MW_STR_MODE
	printf("CVI_AUDIO_Resume not support in MW...\n");
	printf("User should turn off / on Aud on application while suspend / resume\n");
#else

	CVI_S32 s32DevIndex = 0;
	CVI_S32 s32ChnIndex = 0;

	//step 1 check uplink(ain/aenc), create channel(ain/aenc)
	for (s32DevIndex = 0; s32DevIndex < CVI_MAX_AI_DEVICE_ID_NUM; s32DevIndex++) {
		ST_AI_INSTANCE *_ai_instance = &gstAudStr.AinStr[s32DevIndex];

		if (_ai_instance->bEnableAI)
			CVI_AI_SetPubAttr(s32DevIndex, &gstAudStr.AinStr[s32DevIndex].aio_attrs);

		for (s32ChnIndex = 0; s32ChnIndex < CVI_AUD_MAX_CHANNEL_NUM; s32ChnIndex++) {
			ST_AI_CHANNEL_CONFIG *_pstChnConfig = &_ai_instance->stChnConfig[s32ChnIndex];

			if (_pstChnConfig->bChannelEnable == CVI_TRUE) {
				CVI_AI_EnableChn(s32DevIndex, s32ChnIndex);
				printf("cviaudio resume ain chn[%d] resmp[%d] bEnableVqe[%d] bVqeWithAecOn[%d]\n",
				       s32ChnIndex,
				       _pstChnConfig->bResampleChn,
				       _pstChnConfig->bEnableVqe,
				       _pstChnConfig->bVqeWithAecOn);
			}

			if (_pstChnConfig->bEnableVqe == CVI_TRUE) {
				CVI_AI_SetTalkVqeAttr(
					s32DevIndex,
					s32ChnIndex,
					0,
					0,
					(AI_TALKVQE_CONFIG_S *)&gstAudStr.uplinkVqeAttr);
				CVI_AI_EnableVqe(s32DevIndex, s32ChnIndex);
			}

			if (_pstChnConfig->bResampleChn == CVI_TRUE) {
				CVI_AI_EnableReSmp(s32DevIndex,
						   s32ChnIndex,
						   _pstChnConfig->outSmpRate);
			}
		}

		if (_ai_instance->bEnableAI == CVI_TRUE) {
			CVI_AI_Enable(s32DevIndex);
			printf("cviaudio resume ain dev[%d]\n", s32DevIndex);
		}
	}

	for (s32ChnIndex = 0; s32ChnIndex < AENC_MAX_CHN_NUM; s32ChnIndex++) {
		ST_AENC_INSTANCE *_aenc_instance = &gstAudStr.AencStr[s32ChnIndex];

		if (_aenc_instance->bEnableAenc == CVI_TRUE) {
			printf("cviaudio resume aenc chn[%d] bEnableAenc[%d] bVqeOn[%d] bVqeWithAecOn[%d]\n",
			       s32ChnIndex,
			       _aenc_instance->bEnableAenc,
			       _aenc_instance->bVqeOn,
			       _aenc_instance->bVqeWithAecOn);
			CVI_AENC_CreateChn(s32ChnIndex, &_aenc_instance->aenc_attrs);
		}
	}
	//step 2 check downlink(adec/aout), create channel(adec/aout)
	for (s32ChnIndex = 0; s32ChnIndex < ADEC_MAX_CHN_NUM; s32ChnIndex++) {
		ST_ADEC_INSTANCE *_adec_instance = &gstAudStr.AdecStr[s32ChnIndex];

		if (_adec_instance->bEnableAdec == CVI_TRUE) {
			printf("cviaudio resume adec chn[%d]\n", s32ChnIndex);
			CVI_ADEC_CreateChn(s32ChnIndex, &_adec_instance->adec_attrs);
		}
	}

	for (s32DevIndex = 0; s32DevIndex < CVI_MAX_AO_DEVICE_ID_NUM; s32DevIndex++) {
		ST_AO_INSTANCE *_aout_instance = &gstAudStr.AoutStr[s32DevIndex];

		if (_aout_instance->bEnableAO == CVI_TRUE) {
			printf("cviaudio resume aout dev[%d]\n", s32DevIndex);
			CVI_AO_SetPubAttr(s32DevIndex, &_aout_instance->ao_attrs);
			CVI_AO_Enable(s32DevIndex);
			for (s32ChnIndex = 0; s32ChnIndex < CVI_AUD_MAX_CHANNEL_NUM; s32ChnIndex++) {
				ST_AO_CHANNEL_CONFIG *_pstChnConfig = &_aout_instance->stChnConfig[s32ChnIndex];

				if (_pstChnConfig->bChannelEnable == CVI_TRUE) {
					printf("cviaudio resume chn[%d] resmp[%d]\n",
					       s32ChnIndex,
					       _pstChnConfig->bResampleChn);

					if (_pstChnConfig->bResampleChn == CVI_TRUE)
						CVI_AO_EnableReSmp(s32DevIndex, s32ChnIndex, _pstChnConfig->inSmpRate);

					CVI_AO_EnableChn(s32DevIndex, s32ChnIndex);
				}

			}
		}
	}
	//step 3  check bind mode
	ST_BIND_CONFIG *pAudBindStrCfg = (ST_BIND_CONFIG *)&gstAudStr.bindconfig;
	MMF_CHN_S stSrcChn, stDestChn;

	if (pAudBindStrCfg->bAIbindAO == CVI_TRUE) {
		log_error("Not support ain bind aout resume flow\n");
	}

	if (pAudBindStrCfg->bAIbindAENC == CVI_TRUE) {
		printf("cviaudio resume Ain[%d] bind Aenc[%d]\n",
		       pAudBindStrCfg->SrcDevId,
		       pAudBindStrCfg->DestDevId);
		stSrcChn.enModId = CVI_ID_AI;
		stSrcChn.s32DevId = pAudBindStrCfg->SrcDevId;
		stSrcChn.s32ChnId = pAudBindStrCfg->SrcChnId;
		stDestChn.enModId = CVI_ID_AENC;
		stDestChn.s32DevId = pAudBindStrCfg->DestDevId;
		stDestChn.s32ChnId = pAudBindStrCfg->DestChnId;
		CVI_AUD_SYS_Bind(&stSrcChn, &stDestChn);
	}

	if (pAudBindStrCfg->bADECbindAO == CVI_TRUE) {
		printf("cviaudio resume Adec[%d] bind Aout[%d]\n",
		       pAudBindStrCfg->SrcDevId,
		       pAudBindStrCfg->DestDevId);
		ST_BIND_CONFIG *pChnBindCfg = &gstAudStr.AoutStr[pAudBindStrCfg->DestDevId].ChnBindCfg;

		stSrcChn.enModId = CVI_ID_ADEC;
		stSrcChn.s32DevId = pChnBindCfg->SrcDevId;
		stSrcChn.s32ChnId = pChnBindCfg->SrcChnId;
		stDestChn.enModId = CVI_ID_AO;
		stDestChn.s32DevId = pChnBindCfg->DestDevId;
		stDestChn.s32ChnId = pChnBindCfg->DestChnId;
		CVI_AUD_SYS_Bind(&stSrcChn, &stDestChn);
	}
	//step 4 set back the ain/aout volume
	//No Need
#endif
	printf("Enter[CVI_AUDIO_Resume]<----------------\n");
	return s32Ret;
}


/* audio api implementation */
CVI_S32 CVI_AUDIO_SetModParam(const AUDIO_MOD_PARAM_S *pstModParam)
{

	log_debug("===================================================\n");
	log_debug("CVI_AUDIO_SetModParam is not effective in this SDK release\n");
	log_debug("User can toggle the audio module by CVI_AUDIO_INIT api\n");
	log_debug("===================================================\n");

	UNUSED_REF(pstModParam);
	//memset (pstModParam, 0, sizeof(AUDIO_MOD_PARAM_S));
	//TODO: Test mod parameter

	return CVI_SUCCESS;
}
CVI_S32 CVI_AUDIO_GetModParam(AUDIO_MOD_PARAM_S *pstModParam)
{
	log_debug("===================================================\n");
	log_debug("CVI_AUDIO_GetModParam is not effective in this SDK release\n");
	log_debug("User can close the audio module by CVI_AUDIO_DEINIT api\n");
	log_debug("===================================================\n");

	memset(pstModParam, 0, sizeof(AUDIO_MOD_PARAM_S));
	return CVI_SUCCESS;
}



#ifdef AiMultiMic
static int _audio_divide_multichannel(short *tmp,
				      CVI_S16 *input_buff,
				      CVI_U32 *inputbytes,
				      CVI_U32 total_chn,
				      CVI_U32 channel_index)
{
	CVI_U32 i;

	if (total_chn == 1) {
		return *inputbytes;
	}

	if (channel_index >= total_chn) {
		log_error("channel_index[%d] >= total_chn[%d]\n", channel_index, total_chn);
		return CVI_FAILURE;
	}

	if (!tmp) {
		log_error("\n");
		return CVI_FAILURE;
	}
	//if (total_chn == 2 && dst_chn == 1) {
	for (i = 0;  i < (*inputbytes / total_chn); i++) {
		tmp[i] = input_buff[total_chn * i + channel_index];
	}
	*inputbytes = *inputbytes / total_chn;

	memcpy(input_buff, tmp, *inputbytes);
	return *inputbytes;

}


CVI_S32 CVI_AI_EnableMultiMic(AUDIO_DEV AiDevId, CVI_S32 s32MicInNumbers)
{
	UNUSED_REF(AiDevId);
	UNUSED_REF(s32MicInNumbers);
	ST_AIN_MULTI_MIC *_pstMultiMic = &gstAiInstance[AiDevId].stMultiMic;
	CVI_S32 s32Index = 0;
	CVI_S32 s32Ret = 0;

#define MULTI_MIC_CYCLE_BUFFER_SIZE 3840
	log_debug("Dev[%d]Chn[%d]\n", AiDevId, s32MicInNumbers);
	if (s32MicInNumbers > CVI_MAX_INPUT_MIC_NUMBERS) {
		log_error("mic in numbers too large [%d]>[%d]\n",
			  s32MicInNumbers, CVI_MAX_INPUT_MIC_NUMBERS);
		return CVI_ERR_AIO_ILLEGAL_PARAM;
	}

	_pstMultiMic->bEnableMultiMic = CVI_TRUE;
	_pstMultiMic->s32MultiChn = s32MicInNumbers;
	for (s32Index = 0; s32Index < s32MicInNumbers; s32Index++) {

		s32Ret = CycleBufferInit((void **)&_pstMultiMic->pMultiMicCb[s32Index], 3840);
		if (s32Ret < 0) {
			log_error("\n");
			return CVI_FAILURE;
		}
		_pstMultiMic->pMultiMicBuffer[s32Index] =
			(CVI_CHAR *)malloc(MULTI_MIC_CYCLE_BUFFER_SIZE * s32MicInNumbers);
		_pstMultiMic->pMultiMicTmp[s32Index] =
			(short *)malloc(MULTI_MIC_CYCLE_BUFFER_SIZE * s32MicInNumbers);
		memset(_pstMultiMic->pMultiMicBuffer[s32Index], 0,
		       sizeof(MULTI_MIC_CYCLE_BUFFER_SIZE) * s32MicInNumbers);
		memset(_pstMultiMic->pMultiMicTmp[s32Index], 0,
		       sizeof(MULTI_MIC_CYCLE_BUFFER_SIZE) * s32MicInNumbers);
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_DisableMultiMic(AUDIO_DEV AiDevId)
{
	UNUSED_REF(AiDevId);
	ST_AIN_MULTI_MIC *_pstMultiMic = &gstAiInstance[AiDevId].stMultiMic;
	CVI_S32 s32Index = 0;

	log_debug("CVI_AI_DisableMultiMic Dev[%d]\n", AiDevId);
	_pstMultiMic->bEnableMultiMic = CVI_FALSE;


	for (s32Index = 0; s32Index < _pstMultiMic->s32MultiChn; s32Index++) {

		if (_pstMultiMic->pMultiMicCb[s32Index]) {
			CycleBufferDestroy(_pstMultiMic->pMultiMicCb[s32Index]);
			_pstMultiMic->pMultiMicCb[s32Index] = CVI_NULL;
		}
		SAFE_FREE_BUF(_pstMultiMic->pMultiMicBuffer[s32Index]);
		SAFE_FREE_BUF(_pstMultiMic->pMultiMicTmp[s32Index]);
	}
	_pstMultiMic->s32MultiChn = 0;

	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_MultiMicGetFrame(AUDIO_DEV AiDevId, AI_CHN AiChn,
				AUDIO_FRAME_S *pstFrm, CVI_S32 s32MilliSec)
{
	UNUSED_REF(AiDevId);
	UNUSED_REF(AiChn);
	UNUSED_REF(pstFrm);
	UNUSED_REF(s32MilliSec);
	E_AUDIO_DATA_TIMEMODE  eTimeMode = E_AUDIO_DATA_BLOCK_MODE;
	ST_AI_INSTANCE *_ai_instance = &gstAiInstance[AiDevId];
	ST_AIN_MULTI_MIC *_pstMultiMic = &gstAiInstance[AiDevId].stMultiMic;
	ST_TINYALSA_CONFIG *pstTinyAlsaCfg = &_ai_instance->stTinyAlsaCfg;
	CVI_U32 u32PeriodBytes = pstTinyAlsaCfg->periodbytes;
	CVI_BOOL bHasFrame = CVI_FALSE;
	CVI_S32 s32TimeOutCnt = 0;
	CVI_S32 s32Ret = CVI_FAILURE;

	if (_pstMultiMic == CVI_FALSE) {
		log_error("Not success before CVI_AI_EnableMultiMic\n");
		return CVI_ERR_AI_NOT_CONFIG;
	}


	if (s32MilliSec < 0)
		eTimeMode = E_AUDIO_DATA_BLOCK_MODE;
	else if (s32MilliSec == 0)
		eTimeMode = E_AUDIO_DATA_NONE_BLOCK_MODE;
	else if (s32MilliSec  > 0)
		eTimeMode = E_AUDIO_DATA_TIMEOUT_MODE;

	do {
		if (_pstMultiMic->pMultiMicCb[AiChn]) {
			if (CycleBufferDataLen(_pstMultiMic->pMultiMicCb[AiChn]) >= (int)u32PeriodBytes) {
				s32Ret = CycleBufferRead(_pstMultiMic->pMultiMicCb[AiChn],
							 _pstMultiMic->pMultiMicBuffer[AiChn],
							 u32PeriodBytes);
				if (s32Ret <= 0)
					log_error("\n");
				bHasFrame = CVI_TRUE;
			} else {
				bHasFrame = CVI_FALSE;
				usleep(1000 * 10); //sleep 10ms
			}
		} else {
			log_error("DEV[%d]Chn[%d] not enable yet\n", AiDevId, AiChn);
			return CVI_ERR_AI_NOT_ENABLED;
		}

		if (eTimeMode == E_AUDIO_DATA_TIMEOUT_MODE) {
			if (s32TimeOutCnt > s32MilliSec) {
				log_debug("AudIn_GetFrame Timeout [%d] > [%d]\n",
					  s32TimeOutCnt,
					  s32MilliSec);
				break;

			} else {
				usleep(1000);
				s32TimeOutCnt++;
				continue;
			}
		} else if (eTimeMode == E_AUDIO_DATA_BLOCK_MODE) {
			if (_pstMultiMic->pMultiMicBuffer[AiChn] == NULL) {
				log_error("Null Buffer------>[%d] force break\n", __LINE__);
				break;
				//audio total deinit will enter this condition
			} else {
				//printf("[%s]Block mode NOT null DEV[%d]CHN[%d]\n", __func__ ,AiDevId, AiChn);
			}
			if (s32TimeOutCnt > (CVI_AUDIO_DATA_TEN_SEC_TIMEOUT * 1000)) {
				printf("AudIn_GetFrame Timeout [%d] > [%d]\n",
				       (s32TimeOutCnt),
				       (int)CVI_AUDIO_DATA_TEN_SEC_TIMEOUT * 1000);
				log_error("15 second cannot get audio in frame\n");
				//dump the reg
				printf("[cviaudio]ch_status....\n");
				system("cat /proc/sysDMA/ch_status");
				printf("[cviaudio]dma_status....\n");
				system("cat /proc/sysDMA/dma_status ");
				printf("[cviaudio]log_level....\n");
				system("echo 1 > /proc/sysDMA/log_level");
				break;

			} else {
				usleep(1000);
				s32TimeOutCnt++;
				continue;
			}
		}
	} while ((bHasFrame == CVI_FALSE) && (eTimeMode != E_AUDIO_DATA_NONE_BLOCK_MODE));

	CVI_U32 s32TotalBytes = u32PeriodBytes;
	//do the demux for multi channels----------------start
	_audio_divide_multichannel(_pstMultiMic->pMultiMicTmp[AiChn],
				   (CVI_S16 *)_pstMultiMic->pMultiMicBuffer[AiChn],
				   &s32TotalBytes,
				   _pstMultiMic->s32MultiChn,
				   AiChn);
	//do the demux for multi channels----------------end
	pstFrm->enBitwidth = _ai_instance->aio_attrs.enBitwidth;
	pstFrm->enSoundmode = _ai_instance->aio_attrs.enSoundmode;
	pstFrm->u32Len = u32PeriodBytes /
			 (DEFAULT_BYTES_PER_SAMPLE * (_pstMultiMic->s32MultiChn));
	pstFrm->u64TimeStamp = _get_current_pts();
	pstFrm->u32Seq = 0;
	pstFrm->u64VirAddr[0] = (CVI_U8 *)_pstMultiMic->pMultiMicBuffer[AiChn];

	if (bHasFrame == CVI_FALSE) {
		log_debug("Not getting enough frame size[%d] timemode[%d]\n",
			  u32PeriodBytes,
			  eTimeMode);
		pstFrm->u32Len = 0;
		return CVI_SUCCESS;
	}

	return CVI_SUCCESS;
}
#endif


