#include "cvi_auto_test.h"
#include "sample_comm.h"
#include "cvi_audio_aac_adp.h"



int AoDev;
int AiDev;


int running = 1;
int run_count = 100;

static AAC_TYPE_E     gs_enAacType = AAC_TYPE_AACLC;
static AAC_BPS_E     gs_enAacBps  = AAC_BPS_32K;
static AAC_TRANS_TYPE_E gs_enAacTransType = AAC_TRANS_TYPE_ADTS;

void sigint_handler_sample(int signal)
{
	printf("signal = %d\n", signal);
	running = 0;
}

void register_inthandler(void)
{
	signal(SIGINT, sigint_handler_sample);
	signal(SIGHUP, sigint_handler_sample);
	signal(SIGTERM, sigint_handler_sample);
}

static int AUTO_CHECK_NULL_PTR(void *ptr)
{
	if ((CVI_U8 *)ptr == NULL) {
		printf("[cvi_auto_error] ptr is NULL,fuc:%s,line:%d\n", __func__, __LINE__);
		return -1;
	}
	return 0;
}

static CVI_BOOL _update_agc_anr_setting(AI_TALKVQE_CONFIG_S *pstAiVqeTalkAttr)
{
	if (pstAiVqeTalkAttr == NULL)
		return CVI_FALSE;

	pstAiVqeTalkAttr->u32OpenMask |= (NR_ENABLE | AGC_ENABLE | DCREMOVER_ENABLE);

	AUDIO_AGC_CONFIG_S st_AGC_Setting;
	AUDIO_ANR_CONFIG_S st_ANR_Setting;

	st_AGC_Setting.para_agc_max_gain = 0;
	st_AGC_Setting.para_agc_target_high = 2;
	st_AGC_Setting.para_agc_target_low = 72;
	st_AGC_Setting.para_agc_vad_ena = CVI_TRUE;
	st_ANR_Setting.para_nr_snr_coeff = 15;
	st_ANR_Setting.para_nr_init_sile_time = 0;



	pstAiVqeTalkAttr->stAgcCfg = st_AGC_Setting;
	pstAiVqeTalkAttr->stAnrCfg = st_ANR_Setting;

	pstAiVqeTalkAttr->para_notch_freq = 0;
	printf("pstAiVqeTalkAttr:u32OpenMask[0x%x]\n", pstAiVqeTalkAttr->u32OpenMask);
	return CVI_TRUE;
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
	pstAiVqeTalkAttr->u32OpenMask = LP_AEC_ENABLE | NLP_AES_ENABLE |
					NR_ENABLE | AGC_ENABLE;
	printf("pstAiVqeTalkAttr:u32OpenMask[0x%x]\n", pstAiVqeTalkAttr->u32OpenMask);
	return CVI_FALSE;
}


static int _updata_Adec_setting(ADEC_CHN_ATTR_S *pAdecAttr)
{
	AUTO_CHECK_NULL_PTR((void *)pAdecAttr);
	pAdecAttr->u32BufSize = 20;
	pAdecAttr->enMode = ADEC_MODE_STREAM;/* propose use pack mode in your app */
	pAdecAttr->bFileDbgMode = CVI_FALSE;

	if (pAdecAttr->enType == PT_ADPCMA) {
		ADEC_ATTR_ADPCM_S *pstAdpcm = malloc(sizeof(ADEC_ATTR_ADPCM_S));

		pAdecAttr->pValue = pstAdpcm;
		pstAdpcm->enADPCMType = AUDIO_ADPCM_TYPE;
	} else if (pAdecAttr->enType == PT_G711A || pAdecAttr->enType == PT_G711U) {
		ADEC_ATTR_G711_S *pstAdecG711 = malloc(sizeof(ADEC_ATTR_G711_S));

		pAdecAttr->pValue = pstAdecG711;
	} else if (pAdecAttr->enType == PT_G726) {
		ADEC_ATTR_G726_S *pstAdecG726 = malloc(sizeof(ADEC_ATTR_G726_S));

		pAdecAttr->pValue = pstAdecG726;
		pstAdecG726->enG726bps = G726_BPS;
	} else if (pAdecAttr->enType == PT_LPCM) {
		ADEC_ATTR_LPCM_S *pstAdecLpcm = malloc(sizeof(ADEC_ATTR_LPCM_S));

		pAdecAttr->pValue = pstAdecLpcm;
		pAdecAttr->enMode = ADEC_MODE_PACK;/* lpcm must use pack mode */
	}

	if (pAdecAttr->enType == PT_AAC) {
		CVI_MPI_ADEC_AacInit();
		ADEC_ATTR_AAC_S *pstAdecAac = malloc(sizeof(ADEC_ATTR_AAC_S));

		pstAdecAac->enTransType = gs_enAacTransType;
		pstAdecAac->enSoundMode = (pAdecAttr->s32ChannelNums == 2 ?
					   AUDIO_SOUND_MODE_STEREO : AUDIO_SOUND_MODE_MONO);
		pstAdecAac->enSmpRate = pAdecAttr->s32Sample_rate;
		pAdecAttr->pValue = pstAdecAac;
		pAdecAttr->enMode = ADEC_MODE_STREAM;   /* aac should be stream mode */
		pAdecAttr->s32frame_size = 1024;
	}

	return 0;
}

static int _destroy_Adec_setting(ADEC_CHN_ATTR_S *pAdecAttr)
{
	AUTO_CHECK_NULL_PTR((void *)pAdecAttr);
	AUTO_CHECK_NULL_PTR((void *)pAdecAttr->pValue);

	if (pAdecAttr->enType == PT_AAC)
		CVI_MPI_ADEC_AacDeInit();

	AUTO_FREE_BUF(pAdecAttr->pValue);
	memset(pAdecAttr, 0, sizeof(ADEC_CHN_ATTR_S));

	return 0;
}


static CVI_S32  _update_aenc_params(AENC_CHN_ATTR_S *pAencAttrs,
				    AIO_ATTR_S *pAioAttrs,
				    PAYLOAD_TYPE_E enType)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	AUTO_CHECK_NULL_PTR((void *)pAencAttrs);
	AUTO_CHECK_NULL_PTR((void *)pAioAttrs);

	memset(pAencAttrs, 0, sizeof(AENC_CHN_ATTR_S));
	pAencAttrs->enType = enType;
	pAencAttrs->u32BufSize = 30;
	pAencAttrs->u32PtNumPerFrm = pAioAttrs->u32PtNumPerFrm;

	if (pAencAttrs->enType == PT_ADPCMA) {
		AENC_ATTR_ADPCM_S *pstAdpcmAenc = (AENC_ATTR_ADPCM_S *)malloc(sizeof(
				AENC_ATTR_ADPCM_S));

		pstAdpcmAenc->enADPCMType = AUDIO_ADPCM_TYPE;
		pAencAttrs->pValue       = (CVI_VOID *)pstAdpcmAenc;
	} else if (pAencAttrs->enType == PT_G711A || pAencAttrs->enType == PT_G711U) {
		AENC_ATTR_G711_S *pstAencG711 = (AENC_ATTR_G711_S *)malloc(sizeof(
							AENC_ATTR_G711_S));

		pAencAttrs->pValue = (CVI_VOID *)pstAencG711;
	} else if (pAencAttrs->enType == PT_G726) {
		AENC_ATTR_G726_S *pstAencG726 = (AENC_ATTR_G726_S *)malloc(sizeof(
							AENC_ATTR_G726_S));
		pstAencG726->enG726bps = G726_BPS;
		pAencAttrs->pValue = (CVI_VOID *)pstAencG726;

	} else if (pAencAttrs->enType == PT_LPCM) {
		AENC_ATTR_LPCM_S *pstAencLpcm = (AENC_ATTR_LPCM_S *)malloc(sizeof(
							AENC_ATTR_LPCM_S));

		pAencAttrs->pValue = (CVI_VOID *)pstAencLpcm;
	} else if (pAencAttrs->enType == PT_AAC) {
		CVI_AUTO_INFO("Need update detail external AAC function params\n");
		//need update AAC if supported
	} else {
		CVI_AUTO_ERROR("Not support codec type[%d]\n", enType);
		s32Ret = CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 _update_Aenc_setting(AIO_ATTR_S *pstAioAttr,
			     AENC_CHN_ATTR_S *pstAencAttr,
			     PAYLOAD_TYPE_E enType,
			     int Sample_rate, bool bVqe)
{

	CVI_S32 s32Ret;

	s32Ret = _update_aenc_params(pstAencAttr, pstAioAttr, enType);
	if (s32Ret != CVI_SUCCESS) {
		CVI_AUTO_ERROR("error params\n");
		return CVI_FAILURE;
	}

	if (enType == PT_AAC) {
		AENC_ATTR_AAC_S  *pstAencAac = (AENC_ATTR_AAC_S *)malloc(sizeof(
							AENC_ATTR_AAC_S));

		pstAencAac->enAACType = gs_enAacType;
		pstAencAac->enBitRate = gs_enAacBps;
		pstAencAac->enBitWidth = AUDIO_BIT_WIDTH_16;

		pstAencAac->enSmpRate = Sample_rate;
		CVI_AUTO_INFO("AAC enc,smp-rate[%d]\n",
			      pstAencAac->enSmpRate);

		pstAencAac->enSoundMode = bVqe ? AUDIO_SOUND_MODE_MONO :
					  pstAioAttr->enSoundmode;
		pstAencAac->enTransType = gs_enAacTransType;
		pstAencAac->s16BandWidth = 0;
		pstAencAttr->pValue = pstAencAac;
		s32Ret = CVI_MPI_AENC_AacInit();
		CVI_AUTO_INFO("s32Ret:%d\n", s32Ret);
	}

	pstAencAttr->bFileDbgMode = CVI_FALSE;

	return CVI_SUCCESS;
}

int _destroy_Aenc_setting(AENC_CHN_ATTR_S *pstAencAttr)
{
	AUTO_CHECK_NULL_PTR((void *)pstAencAttr);
	AUTO_CHECK_NULL_PTR((void *)pstAencAttr->pValue);

	if (pstAencAttr->enType == PT_AAC)
		CVI_MPI_AENC_AacDeInit();

	if (pstAencAttr->pValue) {
		free(pstAencAttr->pValue);
		pstAencAttr->pValue = NULL;
	}
	memset(pstAencAttr, 0, sizeof(AENC_CHN_ATTR_S));

	return 0;
}


int record_test(void)
{
	int AiChn = 0;
	int s32Ret = 0;
	bool bVqe = false;
	int channel = 2;
	int AudMaxChn = 3;
	int sample_rate = 8000;
	unsigned int Outsample_rate = 16000;
	int period_size = 320;
	int period_count = 4;

	AIO_ATTR_S AudinAttr;

	AudinAttr.enSamplerate = (AUDIO_SAMPLE_RATE_E)sample_rate;
	AudinAttr.u32ChnCnt = AudMaxChn;
	AudinAttr.enSoundmode = (channel == 2 ? AUDIO_SOUND_MODE_STEREO :
				 AUDIO_SOUND_MODE_MONO);
	AudinAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	AudinAttr.enWorkmode = AIO_MODE_I2S_MASTER;
	AudinAttr.u32EXFlag = 0;
	AudinAttr.u32FrmNum = period_count; /* only use in bind mode */
	AudinAttr.u32PtNumPerFrm = period_size; /* sample_rate/fps */
	AudinAttr.u32ClkSel = 0;
	AudinAttr.enI2sType = AIO_I2STYPE_INNERCODEC;

	/*if you want to use vqe ,ai chn must 2chn.*/
	/*if you don't need to use vqe, you can skip this step*/


	AI_TALKVQE_CONFIG_S stAiVqeTalkAttr = {0};
	AI_TALKVQE_CONFIG_S *pstAiVqeTalkAttr = (AI_TALKVQE_CONFIG_S *)&stAiVqeTalkAttr;

	if (((AudinAttr.enSamplerate == AUDIO_SAMPLE_RATE_8000) ||
	     (AudinAttr.enSamplerate == AUDIO_SAMPLE_RATE_16000)) &&
	    channel == 2) {

		pstAiVqeTalkAttr->s32WorkSampleRate = AudinAttr.enSamplerate;
		_update_agc_anr_setting(pstAiVqeTalkAttr);
		_update_aec_setting(pstAiVqeTalkAttr);
	} else {
		printf("[error] AEC will need to setup record in to channel Count = 2\n");
		printf("[error] VQE only support on 8k/16k sample rate. current[%d]\n",
		       AudinAttr.enSamplerate);
	}


	while (running && run_count--) {
		/* AI ENABLE*/
		s32Ret = CVI_AUDIO_INIT();
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("s32Ret = %#x\n", s32Ret);
			return -1;
		}

		s32Ret = CVI_AI_SetPubAttr(AiDev, &AudinAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("s32Ret = %#x\n", s32Ret);
			return -1;
		}
		s32Ret = CVI_AI_Enable(AiDev);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("s32Ret = %#x\n", s32Ret);
			return -1;
		}

		s32Ret = CVI_AI_EnableChn(AiDev, AiChn);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("s32Ret = %#x\n", s32Ret);
			return -1;
		}

		if (bVqe == true) {
			s32Ret = CVI_AI_SetTalkVqeAttr(AiDev, AiChn, 0, 0,
						       &stAiVqeTalkAttr);
			if (s32Ret != CVI_SUCCESS) {
				CVI_AUTO_ERROR("s32Ret = %#x\n", s32Ret);
				return -1;
			}

			s32Ret = CVI_AI_EnableVqe(AiDev, AiChn);
			if (s32Ret != CVI_SUCCESS) {
				CVI_AUTO_ERROR("s32Ret = %#x\n", s32Ret);
				return -1;
			}

		}

		if ((Outsample_rate != (unsigned int)AudinAttr.enSamplerate)) {
			s32Ret = CVI_AI_EnableReSmp(AiDev, AiChn, Outsample_rate);
			if (s32Ret != CVI_SUCCESS) {
				CVI_AUTO_ERROR("s32Ret = %#x, Outsample_rate = %d\n", s32Ret, Outsample_rate);
				return -1;
			}

		}


		/* AENC ENABLE */
		int AeChn = 0;
		AENC_CHN_ATTR_S stAencAttr;
		PAYLOAD_TYPE_E enType = PT_G711A;

		_update_Aenc_setting(&AudinAttr, &stAencAttr, enType,
				     Outsample_rate, bVqe);

		s32Ret = CVI_AENC_CreateChn(AeChn, &stAencAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("s32Ret = %#x\n", s32Ret);
			return -1;
		}

		usleep(1000 * 20);

		/* AENC DISABLE */

		s32Ret = CVI_AENC_DestroyChn(AeChn);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("s32Ret = %#x\n", s32Ret);
			return -1;
		}
		_destroy_Aenc_setting(&stAencAttr);

		/* AI DISABLE */
		if (bVqe == true) {
			s32Ret = CVI_AI_DisableVqe(AiDev, AiChn);
			if (s32Ret != CVI_SUCCESS) {
				CVI_AUTO_ERROR("s32Ret = %#x\n", s32Ret);
				return -1;
			}

		}

		if ((Outsample_rate != (unsigned int)AudinAttr.enSamplerate)) {
			s32Ret = CVI_AI_DisableReSmp(AiDev, AiChn);
			if (s32Ret != CVI_SUCCESS) {
				CVI_AUTO_ERROR("s32Ret = %#x\n", s32Ret);
				return -1;
			}

		}

		s32Ret = CVI_AI_DisableChn(AiDev, AiChn);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("s32Ret = %#x\n", s32Ret);
			return -1;
		}


		s32Ret = CVI_AI_Disable(AiDev);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("s32Ret = %#x\n", s32Ret);
			return -1;
		}

		s32Ret = CVI_AUDIO_DEINIT();
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("s32Ret = %#x\n", s32Ret);
			return -1;
		}


	}



	return 0;
}

int play_test(void)
{
	int AoChn = 0;
	int AdChn = 0;
	int s32Ret = 0;

	AIO_ATTR_S AudoutAttr;
	int channels = 2;
	int sample_rate = 16000;
	int chnSampl_rate = 48000;
	int period_size = 320;
	int period_count = 4;
	int AudMaxChn = 3;

	AudoutAttr.u32ChnCnt = AudMaxChn;
	AudoutAttr.enSamplerate   = sample_rate;
	AudoutAttr.enSoundmode	  = (channels == 2 ?
				     AUDIO_SOUND_MODE_STEREO :
				     AUDIO_SOUND_MODE_MONO);
	AudoutAttr.enWorkmode	  = AIO_MODE_I2S_MASTER;
	AudoutAttr.u32EXFlag	  = 0;
	AudoutAttr.u32FrmNum	  = period_count; /* only use in bind mode */
	AudoutAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	AudoutAttr.u32PtNumPerFrm =
		period_size;/* 20*targetsamplerate/1000 */
	AudoutAttr.u32ClkSel	  = 0;
	AudoutAttr.enI2sType = AIO_I2STYPE_INNERCODEC;



	while (running && run_count--) {

		/* AO ENABLE */
		s32Ret = CVI_AUDIO_INIT();
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("s32Ret = %#x\n", s32Ret);
			return -1;
		}

		s32Ret = CVI_AO_SetPubAttr(AoDev, &AudoutAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("AoDev = %d, s32Ret = %#x\n", AoDev, s32Ret);
			return -1;
		}

		s32Ret = CVI_AO_Enable(AoDev);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("AoDev = %d, s32Ret = %#x\n", AoDev, s32Ret);
			return -1;
		}


		s32Ret = CVI_AO_EnableChn(AoDev, AoChn);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("s32Ret = %#x, AoChn = %d\n", s32Ret, AoChn);
			return -1;
		}


		s32Ret = CVI_AO_EnableReSmp(AoDev, AoChn, chnSampl_rate);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("devsr:%d to %d, s32Ret = %#x\n", sample_rate, chnSampl_rate,
				       s32Ret);
			return -1;
		}



		/* ADEC ENABLE */

		ADEC_CHN_ATTR_S stAdecAttr;

		memset(&stAdecAttr, 0, sizeof(ADEC_CHN_ATTR_S));
		stAdecAttr.s32Sample_rate = chnSampl_rate;
		stAdecAttr.s32ChannelNums = channels;
		stAdecAttr.s32BytesPerSample = BYTES_PER_SAMPLE;
		stAdecAttr.s32frame_size = AudoutAttr.u32PtNumPerFrm;
		stAdecAttr.enType = PT_G711A;
		if (stAdecAttr.enType == PT_AAC)
			stAdecAttr.s32frame_size = 1024;


		_updata_Adec_setting(&stAdecAttr);

		s32Ret = CVI_ADEC_CreateChn(AdChn, &stAdecAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("s32Ret = %d, AoChn = %d, AdChn = %d\n", s32Ret, AoChn, AdChn);
			_destroy_Adec_setting(&stAdecAttr);
			return -1;
		}

		usleep(1000 * 20);

		/* ADEC DISABLE */

		_destroy_Adec_setting(&stAdecAttr);
		s32Ret = CVI_ADEC_DestroyChn(AdChn);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("s32Ret = %#x\n", s32Ret);
			return -1;
		}

		/* AO DISABLE */

		s32Ret = CVI_AO_DisableReSmp(AoDev, AoChn);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("devsr:%d to %d, s32Ret = %#x\n", sample_rate, chnSampl_rate,
				       s32Ret);
			return -1;
		}

		s32Ret = CVI_AO_DisableChn(AoDev, AoChn);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("AoDev = %d, s32Ret = %#x\n", AoDev, s32Ret);
			return -1;
		}


		s32Ret = CVI_AO_Disable(AoDev);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("AoDev = %d, s32Ret = %#x\n", AoDev, s32Ret);
			return -1;
		}


		s32Ret = CVI_AUDIO_DEINIT();
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("s32Ret = %#x\n", s32Ret);
			return -1;
		}


	}


	return 0;
}


int main(int argc, char *argv[])
{
	int s32Ret = 0;

	if (argc != 2) {
		printf("[record] use:%s 1\n", argv[0]);
		printf("[play] use:%s 2\n", argv[0]);
		return -1;
	}

	register_inthandler();
	int case_count = atoi(argv[1]);


	switch (case_count) {
	case 1:
		s32Ret = record_test();
		break;
	case 2:
		s32Ret = play_test();
		break;
	default:
		break;
	}

	if (s32Ret == CVI_SUCCESS)
		printf("TEST-PASS\n");

	return 0;
}
