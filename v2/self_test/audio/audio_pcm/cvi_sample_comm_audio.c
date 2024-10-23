#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>
#include <semaphore.h>
#if !(defined(__CV181X__) || defined(__CV180X__) || defined(__CV186X__))
#include "sample_comm.h"
#endif
#include "acodec.h"
//#include "cvi_aud_internal.h"
#ifdef SUPPORT_EXTERNAL_AAC
#include "cvi_audio_aac_adp.h"
#endif


#ifdef __cplusplus
#if __cplusplus
/* EXTERN_CVI "C" {*/
#endif
#endif /* End of #ifdef __cplusplus */

#if defined(ARCH_CV183X)
#define ACODEC_ADC	"/dev/cv1835adc"
#define ACODEC_DAC	"/dev/cv1835dac"
#elif defined(__CV181X__) || defined(__CV180X__)
#define ACODEC_ADC	"/dev/cvitekaadc"
#define ACODEC_DAC	"/dev/cvitekadac"
#elif defined(__CV186X__)
#define ACODEC_ADC	"/dev/misccvitekadc_0"
#define ACODEC_DAC	"/dev/misccvitekdac_1"
#else
#define ACODEC_ADC	"/dev/cv182xadc"
#define ACODEC_DAC	"/dev/cv182xdac"
#endif

#define AUDIO_ADPCM_TYPE ADPCM_TYPE_DVI4/* ADPCM_TYPE_IMA, ADPCM_TYPE_DVI4*/
#define G726_BPS MEDIA_G726_32K         /* MEDIA_G726_16K, MEDIA_G726_24K ... */

#ifdef SUPPORT_EXTERNAL_AAC
static AAC_TYPE_E     gs_enAacType = AAC_TYPE_AACLC;
static AAC_BPS_E     gs_enAacBps  = AAC_BPS_32K;
static AAC_TRANS_TYPE_E gs_enAacTransType = AAC_TRANS_TYPE_ADTS;
#endif

#ifndef _UNUSED_VAR
#define _UNUSED_VAR(X) ((X) = (X))
#endif

static sem_t s_ExitSem;

typedef struct tagSAMPLE_AENC_S {
	CVI_BOOL bStart;
	pthread_t stAencPid;
	CVI_S32  AeChn;
	CVI_S32  AdChn;
	FILE    *pfd;
	CVI_BOOL bSendAdChn;
} SAMPLE_AENC_S;


typedef struct tagSAMPLE_ADEC_AO_S {
	CVI_BOOL bStart;
	pthread_t stPid;
	CVI_S32  AdChn;
	AUDIO_DEV AoDev;
	FILE    *pfd;
} SAMPLE_ADEC_AO_S;

typedef struct tagSAMPLE_AI_S {
	CVI_BOOL bStart;
	CVI_S32  AiDev;
	CVI_S32  AiChn;
	CVI_S32  AencChn;
	CVI_S32  AoDev;
	CVI_S32  AoChn;
	CVI_BOOL bSendAenc;
	CVI_BOOL bSendAo;
	pthread_t stAiPid;
} SAMPLE_AI_S;

typedef struct tagSAMPLE_ADEC_S {
	CVI_BOOL bStart;
	CVI_S32 AdChn;
	FILE *pfd;
	pthread_t stAdPid;
} SAMPLE_ADEC_S;

typedef struct tagSAMPLE_AO_S {
	AUDIO_DEV AoDev;
	CVI_BOOL bStart;
	pthread_t stAoPid;
} SAMPLE_AO_S;

CVI_BOOL bAinResample;
AUDIO_SAMPLE_RATE_E enAinTargetSampleRate = AUDIO_SAMPLE_RATE_BUTT;
PAYLOAD_TYPE_E	genAdecType;
static SAMPLE_AI_S   gs_stSampleAi[AI_DEV_MAX_NUM * AI_MAX_CHN_NUM];
static SAMPLE_AENC_S gs_stSampleAenc[AENC_MAX_CHN_NUM];
static SAMPLE_ADEC_S gs_stSampleAdec[ADEC_MAX_CHN_NUM];
static SAMPLE_AO_S   gs_stSampleAo[AO_DEV_MAX_NUM];
static SAMPLE_ADEC_AO_S gs_stSampleAdecAo[ADEC_MAX_CHN_NUM];
static CVI_S32  _update_aenc_params(AENC_CHN_ATTR_S *pAencAttrs,
				    AIO_ATTR_S *pAioAttrs,
				    PAYLOAD_TYPE_E enType);

#ifdef CVI_ACODEC_TYPE_TLV320AIC31
CVI_S32 SAMPLE_Tlv320_CfgAudio(AIO_MODE_E enWorkmode,
			       AUDIO_SAMPLE_RATE_E enSample)
{
	CVI_S32 sample;
	CVI_S32 vol = 0x100;
	Audio_Ctrl audio_ctrl;
	int s_fdTlv = -1;
	CVI_BOOL bPCMmode = CVI_FALSE;
	CVI_BOOL bMaster = CVI_TRUE;
	CVI_BOOL bPCMStd = CVI_FALSE;


	CVI_BOOL b44100HzSeries = CVI_FALSE;

	if (enSample == AUDIO_SAMPLE_RATE_8000) {
		sample = AC31_SET_8K_SAMPLERATE;
	} else if (enSample == AUDIO_SAMPLE_RATE_11025) {
		b44100HzSeries = CVI_TRUE;
		sample = AC31_SET_11_025K_SAMPLERATE;
	} else if (enSample == AUDIO_SAMPLE_RATE_16000) {
		sample = AC31_SET_16K_SAMPLERATE;
	} else if (enSampl == eAUDIO_SAMPLE_RATE_22050) {
		b44100HzSeries = CVI_TRUE;
		sample = AC31_SET_22_05K_SAMPLERATE;
	} else if (enSample == AUDIO_SAMPLE_RATE_24000) {
		sample = AC31_SET_24K_SAMPLERATE;
	} else if (enSample == enSamplAUDIO_SAMPLE_RATE_32000e) {
		sample = AC31_SET_32K_SAMPLERATE;
	} else if (enSample == AUDIO_SAMPLE_RATE_44100) {
		b44100HzSeries = CVI_TRUE;
		sample = AC31_SET_44_1K_SAMPLERATE;
	} else if (enSample == AUDIO_SAMPLE_RATE_48000) {
		sample = AC31_SET_48K_SAMPLERATE;
	} else {
		printf("SAMPLE_Tlv320_CfgAudio(), not support enSample:%d\n", enSample);
		return -1;
	}

	if (enWorkmode == AIO_MODE_I2S_MASTER) {
		bPCMmode = CVI_FALSE;
		bMaster = CVI_FALSE;
	} else if (enWorkmode == AIO_MODE_I2S_SLAVE) {
		bPCMmode = CVI_FALSE;
		bMaster = CVI_TRUE;
	} else if ((enWorkmode == AIO_MODE_PCM_MASTER_NSTD)
		   || (enWorkmode == AIO_MODE_PCM_MASTER_STD)) {
		bPCMmode = CVI_TRUE;
		bMaster = CVI_FALSE;
	} else if ((enWorkmode == AIO_MODE_PCM_SLAVE_NSTD)
		   || (enWorkmode == AIO_MODE_PCM_SLAVE_STD)) {
		bPCMmode = CVI_TRUE;
		bMaster = CVI_TRUE;
	} else {
		printf("SAMPLE_Tlv320_CfgAudio(), not support workmode:%d\n\n", enWorkmode);
	}

	s_fdTlv = open(TLV320_FILE, O_RDWR);
	if (s_fdTlv < 0) {
		printf("can't open tlv320,%s\n", TLV320_FILE);
		return -1;
	}

	audio_ctrl.chip_num = 0;
	if (ioctl(s_fdTlv, SOFT_RESET, &audio_ctrl)) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n", __func__, __LINE__,
		       "tlv320aic31 reset failed");
	}


	audio_ctrl.ctrl_mode = bMaster;
	audio_ctrl.if_44100hz_series = b44100HzSeries;
	audio_ctrl.sample = sample;
	audio_ctrl.sampleRate = (CVI_U32)enSample;
	ioctl(s_fdTlv, SET_CTRL_MODE, &audio_ctrl);

	/* set transfer mode 0:I2S 1:PCM */
	audio_ctrl.trans_mode = bPCMmode;
	if (ioctl(s_fdTlv, SET_TRANSFER_MODE, &audio_ctrl)) {
		printf("set tlv320aic31 trans_mode err\n");
		close(s_fdTlv);
		return -1;
	}

	/*set sample of DAC and ADC */
	if (ioctl(s_fdTlv, SET_DAC_SAMPLE, &audio_ctrl)) {
		printf("ioctl err1\n");
		close(s_fdTlv);
		return -1;
	}

	if (ioctl(s_fdTlv, SET_ADC_SAMPLE, &audio_ctrl)) {
		printf("ioctl err2\n");
		close(s_fdTlv);
		return -1;
	}

	/*set volume control of left and right DAC */
	audio_ctrl.if_mute_route = 0;
	audio_ctrl.input_level = 0;
	ioctl(s_fdTlv, LEFT_DAC_VOL_CTRL, &audio_ctrl);
	ioctl(s_fdTlv, RIGHT_DAC_VOL_CTRL, &audio_ctrl);

	/*Right/Left DAC Datapath Control */
	audio_ctrl.if_powerup =
		1;/*Left/Right DAC datapath plays left/right channel input data*/
	ioctl(s_fdTlv, LEFT_DAC_POWER_SETUP, &audio_ctrl);
	if ((enWorkmode != AIO_MODE_I2S_MASTER) && (enWorkmode != AIO_MODE_I2S_SLAVE))
		audio_ctrl.if_powerup = 0;

	ioctl(s_fdTlv, RIGHT_DAC_POWER_SETUP, &audio_ctrl);

	/* config PCM standard mode and nonstandard mode */
	if ((enWorkmode == AIO_MODE_PCM_MASTER_STD)
	    || (enWorkmode == AIO_MODE_PCM_SLAVE_STD)) {
		bPCMStd = CVI_TRUE;
		audio_ctrl.data_offset = 2;
		ioctl(s_fdTlv, SET_SERIAL_DATA_OFFSET, &audio_ctrl);
	} else if ((enWorkmode == AIO_MODE_PCM_MASTER_NSTD)
		   || (enWorkmode == AIO_MODE_PCM_SLAVE_NSTD)) {
		bPCMStd = CVI_FALSE;
		audio_ctrl.data_offset = bPCMStd;
		ioctl(s_fdTlv, SET_SERIAL_DATA_OFFSET, &audio_ctrl);
	} else {
		;
	}

	/* (0:16bit 1:20bit 2:24bit 3:32bit) */
	audio_ctrl.data_length = 0;
	ioctl(s_fdTlv, SET_DATA_LENGTH, &audio_ctrl);

	/*DACL1 TO LEFT_LOP/RIGHT_LOP VOLUME CONTROL 82 92*/
	audio_ctrl.if_mute_route = 1;/* route*/
	audio_ctrl.input_level = vol; /*level control*/
	ioctl(s_fdTlv, DACL1_2_LEFT_LOP_VOL_CTRL, &audio_ctrl);
	ioctl(s_fdTlv, DACR1_2_RIGHT_LOP_VOL_CTRL, &audio_ctrl);

	/* LEFT_LOP/RIGHT_LOP OUTPUT LEVEL CONTROL 86 93*/
	audio_ctrl.if_mute_route = 1;
	audio_ctrl.if_powerup = 1;
	audio_ctrl.input_level = 0;
	ioctl(s_fdTlv, LEFT_LOP_OUTPUT_LEVEL_CTRL, &audio_ctrl);
	ioctl(s_fdTlv, RIGHT_LOP_OUTPUT_LEVEL_CTRL, &audio_ctrl);

	/* LEFT/RIGHT ADC PGA GAIN CONTROL 15 16*/
	audio_ctrl.if_mute_route = 0;
	audio_ctrl.input_level = 0;
	ioctl(s_fdTlv, LEFT_ADC_PGA_CTRL, &audio_ctrl);
	ioctl(s_fdTlv, RIGHT_ADC_PGA_CTRL, &audio_ctrl);

	/*INT2L TO LEFT/RIGTH ADCCONTROL 17 18*/
	audio_ctrl.input_level = 0;
	ioctl(s_fdTlv, IN2LR_2_LEFT_ADC_CTRL, &audio_ctrl);
	ioctl(s_fdTlv, IN2LR_2_RIGTH_ADC_CTRL, &audio_ctrl);

	/*IN1L_2_LEFT/RIGTH_ADC_CTRL 19 22*/
	/*audio_ctrl.input_level = 0xf;						*/
	/*audio_ctrl.if_powerup = 1;*/
	/*printf("audio_ctrl.input_level=0x%x,audio_ctrl.if_powerup=0x%x\n",*/
	/*audio_ctrl.input_level,audio_ctrl.if_powerup);*/
	/*if (ioctl(s_fdTlv,IN1L_2_LEFT_ADC_CTRL,&audio_ctrl)==0)*/
	/*    perror("ioctl err\n");*/
	/*getchar();*/
	/*printf("audio_ctrl.input_level=0x%x,audio_ctrl.if_powerup=0x%x\n",*/
	/*audio_ctrl.input_level,audio_ctrl.if_powerup);*/
	/*ioctl(s_fdTlv,IN1R_2_RIGHT_ADC_CTRL,&audio_ctrl);*/
	/*getchar();*/
	/*printf("set 19 22\n");*/

	audio_ctrl.if_mute_route = 1;
	audio_ctrl.input_level = 9;
	audio_ctrl.if_powerup = 1;
	ioctl(s_fdTlv, HPLOUT_OUTPUT_LEVEL_CTRL, &audio_ctrl);
	ioctl(s_fdTlv, HPROUT_OUTPUT_LEVEL_CTRL, &audio_ctrl);

	close(s_fdTlv);
	printf("Set aic31 ok: bMaster = %d, enWorkmode = %d, enSamplerate = %d\n",
	       bMaster, enWorkmode, enSample);
	return 0;
}


CVI_S32 SAMPLE_Tlv320_Disable(CVI_VOID)
{
	Audio_Ctrl audio_ctrl;
	int s_fdTlv = -1;
	CVI_S32 s32Ret;

	s_fdTlv = open(TLV320_FILE, O_RDWR);
	if (s_fdTlv < 0) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n", __func__, __LINE__,
		       "can't open /dev/tlv320aic31");
		return CVI_FAILURE;
	}

	/* reset transfer mode 0:I2S 1:PCM */
	audio_ctrl.chip_num = 0;
	s32Ret = ioctl(s_fdTlv, SOFT_RESET, &audio_ctrl);
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n", __func__, __LINE__,
		       "tlv320aic31 reset failed");
	}
	close(s_fdTlv);

	return s32Ret;
}
#endif // end  CVI_ACODEC_TYPE_TLV320AIC31

unsigned long long get_current_time(CVI_VOID)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

CVI_S32 SAMPLE_AUDIO_DEBUG(CVI_VOID)
{
	CVI_AUDIO_DEBUG();
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_AUDIO_DEBUG_LEVEL(ST_AudioUnitTestCfg *testCfg)
{
	//cvi_audio_set_dbg_level(level);
	cvi_audio_set_dbg_option(testCfg);
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_AUDIO_DEBUG_RECORD(ST_AudioUnitTestCfg *testCfg)
{
	printf("Enter SAMPLE_AUDIO_DEBUG_RECORD\n");
	cvi_audio_set_dbg_record(testCfg);
	return CVI_SUCCESS;
}

//CVI_S32 SAMPLE_AUDIO_DEBUG_PLAY(CVI_CHAR  *cvifilename)
CVI_S32 SAMPLE_AUDIO_DEBUG_PLAY(ST_AudioUnitTestCfg *testCfg)
{
	printf("Enter SAMPLE_AUDIO_DEBUG_PLAY\n");
	//cvi_audio_set_dbg_play(cvifilename);
	cvi_audio_set_dbg_play(testCfg);
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_AUDIO_DEBUG_VQE_PLAY(ST_AudioUnitTestCfg *testCfg)
{
	printf("Enter SAMPLE_AUDIO_DEBUG_VQE_PLAY\n");
	cvi_audio_set_dbg_vqe_play(testCfg);
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_AUDIO_DEBUG_SET_VOLUME(ST_AudioUnitTestCfg *testCfg)
{
	printf("Enter SAMPLE_AUDIO_DEBUG_SET_VOLUME\n");
	_UNUSED_VAR(testCfg);
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 err;
	CVI_S32 idevid = 0;
	CVI_S32 volumedb = 0;
	CVI_S32 s32SetInputOrOutput = 0;

	printf("----------------------cvi check------------------------\n");
	printf("\n Enter output card id: \t");
	err = scanf("%d", &idevid);
	printf("\n Enter volume \t");
	err = scanf("%d", &volumedb);
	printf("\n enter card[%d] vol[%d]\n", idevid, volumedb);
	if (err == EOF)
		printf("[Error][%s][%d]\n", __func__, __LINE__);
	printf("\n Set [Ain]Volume:1  Set [Aout]Volume:0 ? [0 or 1]\n");
	err = scanf("%d", &s32SetInputOrOutput);
	printf("select [%d]\n", s32SetInputOrOutput);
	if (s32SetInputOrOutput == 0) {
		s32Ret = CVI_AO_SetVolume(idevid, volumedb);
		if (s32Ret != CVI_SUCCESS) {
			printf("[Error][%s][%d]\n", __func__, __LINE__);
			return CVI_FAILURE;
		}
	} else {
		s32Ret = CVI_AI_SetVolume(idevid, volumedb);
		if (s32Ret != CVI_SUCCESS) {
			printf("[Error][%s][%d]\n", __func__, __LINE__);
			return CVI_FAILURE;
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_AUDIO_DEBUG_GET_VOLUME(ST_AudioUnitTestCfg *testCfg)
{
	printf("Enter SAMPLE_AUDIO_DEBUG_GET_VOLUME\n");
	CVI_S32 idevid = 0;
	CVI_S32 volume = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 err;

	_UNUSED_VAR(testCfg);

	printf("Enter output card id: \t");
	err = scanf("%d", &idevid);
	if (err == EOF)
		printf("[Error][%s][%d]\n", __func__, __LINE__);
	printf("\n enter card[%d]\n", idevid);
	s32Ret = CVI_AO_GetVolume(idevid, &volume);
	printf("Get Volume Aout[%d]\n", volume);
	if (s32Ret != CVI_SUCCESS) {
		printf("[Error][%s][%d]\n", __func__, __LINE__);
		return CVI_FAILURE;
	}
	s32Ret = CVI_AI_GetVolume(idevid, &volume);
	printf("Get Volume Ain[%d]\n", volume);
	if (s32Ret != CVI_SUCCESS) {
		printf("[Error][%s][%d]\n", __func__, __LINE__);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_AUDIO_Quick_Test(void)
{
	//cvi_audio_set_dbg_get_volume();
	cvi_audio_dbg_quick_test();
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_AUDIO_DEBUG_QUICK_TEST(CVI_VOID)
{
	printf("Enter SAMPLE_AUDIO_DEBUG_QUICK_TEST..\n");
	SAMPLE_COMM_AUDIO_Quick_Test();
	return CVI_SUCCESS;
}


CVI_S32 SAMPLE_INNER_CODEC_CfgAudio(AUDIO_SAMPLE_RATE_E enSample)
{
	enSample = enSample;
	printf("This function is not needed in CV183X series\n");
	return CVI_SUCCESS;
}

/* config codec */
CVI_S32 SAMPLE_COMM_AUDIO_CfgAcodec(AIO_ATTR_S *pstAioAttr)
{
	pstAioAttr = pstAioAttr;
	if (CVI_AUDIO_INIT() == CVI_SUCCESS) {
		printf("CVI_AUDIO_INIT success!!\n");
		return CVI_SUCCESS;

	} else {
		printf("CVI_AUDIO_INIT failure!!\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

/******************************************************************************/
/* function : get frame from Ai, send it  to Aenc or Ao*/
/******************************************************************************/
#define TOGGLE_TIME_MEASUREMENT 1
#if TOGGLE_TIME_MEASUREMENT
unsigned long long gettimenow;
unsigned long long checktime_getframe;
unsigned long long checktime_getframe_period;
unsigned long long checktime_getframe_periodpre;
unsigned long long checktime_sendframe;
unsigned long long checktime_sendframe_period;
unsigned long long checktime_sendframe_periodpre;
unsigned long long checktime_releaseframe;
#endif
CVI_VOID *SAMPLE_COMM_AUDIO_AiProc(CVI_VOID *parg)
{
	CVI_S32 s32Ret;
	SAMPLE_AI_S *pstAiCtl = (SAMPLE_AI_S *)parg;
	AUDIO_FRAME_S stFrame;
	AEC_FRAME_S   stAecFrm;
	AI_CHN_PARAM_S stAiChnPara;

	s32Ret = CVI_AI_GetChnParam(pstAiCtl->AiDev, pstAiCtl->AiChn, &stAiChnPara);
	if (s32Ret != CVI_SUCCESS) {
		printf("%s: Get ai chn param failed\n", __func__);
		return NULL;
	}

	stAiChnPara.u32UsrFrmDepth = 10; //30->10

	s32Ret = CVI_AI_SetChnParam(pstAiCtl->AiDev, pstAiCtl->AiChn, &stAiChnPara);
	if (s32Ret != CVI_SUCCESS) {
		printf("%s: set ai chn param failed\n", __func__);
		return NULL;
	}

	while (pstAiCtl->bStart) {
		/* get frame from ai chn */
		memset(&stAecFrm, 0, sizeof(AEC_FRAME_S));
		gettimenow = get_current_time();
		//checktime_getframe_period = get_current_time() - checktime_getframe_periodpre;

		s32Ret = CVI_AI_GetFrame(pstAiCtl->AiDev, pstAiCtl->AiChn, &stFrame,
					 &stAecFrm, -1);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_AI_GetFrame none!!\n");
			continue;
		}
		//printf("CVI_AI_GetFrame time[%lld]\n", get_current_time() - gettimenow);
		checktime_getframe = get_current_time() - gettimenow;
		if (stFrame.u32Len == 0) {
			printf("none block mode return size 0...retry\n");
			continue;
		}
		/* send frame to encoder */
		if (pstAiCtl->bSendAenc == CVI_TRUE) {
			s32Ret = CVI_AENC_SendFrame(pstAiCtl->AencChn, &stFrame, &stAecFrm);
			if (s32Ret != CVI_SUCCESS) {
				printf("%s: CVI_AENC_SendFrame(%d), failed with %#x!\n",
				       __func__, pstAiCtl->AencChn, s32Ret);
				pstAiCtl->bStart = CVI_FALSE;
				return NULL;
			}
		}
		/* send frame to ao */
		/* If owner toggle bSendAenc, do not toggle bSendAo */
		/* You cannot send encode frame to CVI_AO_SendFrame */
		/* It cannot play out encode frame by only AO_SendFrame*/
		if (pstAiCtl->bSendAo == CVI_TRUE) {
			gettimenow = get_current_time();
			//   checktime_sendframe_period = get_current_time()-checktime_sendframe_periodpre;
			s32Ret = CVI_AO_SendFrame(pstAiCtl->AoDev, pstAiCtl->AoChn, &stFrame, 1000);

			if (s32Ret != CVI_SUCCESS) {
				printf("%s: CVI_AO_SendFrame(%d, %d), failed with %#x!\n",
				       __func__, pstAiCtl->AoDev, pstAiCtl->AoChn, s32Ret);
				pstAiCtl->bStart = CVI_FALSE;
				return NULL;
			}
			printf("time ain_get frm[%d]timecost[%lld] ao_send timecost[%lld]\n",
			       stFrame.u32Len,
			       checktime_getframe,
			       get_current_time() - gettimenow);
			// checktime_sendframe = get_current_time()-gettimenow;
			//checktime_sendframe_periodpre = get_current_time();
		}
		/* finally you must release the stream */
		//    gettimenow = get_current_time();
		s32Ret = CVI_AI_ReleaseFrame(pstAiCtl->AiDev, pstAiCtl->AiChn, &stFrame,
					     &stAecFrm);

		if (s32Ret != CVI_SUCCESS) {
			printf("%s: CVI_AI_ReleaseFrame(%d, %d), failed with %#x!\n",
			       __func__, pstAiCtl->AiDev, pstAiCtl->AiChn, s32Ret);
			pstAiCtl->bStart = CVI_FALSE;
			return NULL;
		}
	}

	pstAiCtl->bStart = CVI_FALSE;
	return NULL;
}

CVI_VOID *SAMPLE_COMM_AUDIO_AencGetProc(CVI_VOID *parg)
{
	CVI_S32 s32Ret;
	SAMPLE_AENC_S *pstAencCtl = (SAMPLE_AENC_S *)parg;
	AUDIO_STREAM_S stStream;
	struct timespec end;
	struct timespec now;
	int cap_time = _get_record_time();

	printf("cap_time = %d\n", cap_time);
	clock_gettime(CLOCK_MONOTONIC, &now);
	end.tv_sec = now.tv_sec + cap_time;
	end.tv_nsec = now.tv_nsec;

	while (pstAencCtl->bStart) {
		s32Ret = CVI_AENC_GetStream(pstAencCtl->AeChn, &stStream, CVI_FALSE);
		if (s32Ret != CVI_SUCCESS) {
			printf("%s: CVI_AENC_GetStream(%d), failed with %#x!\n",
			       __func__, pstAencCtl->AeChn, s32Ret);
			pstAencCtl->bStart = CVI_FALSE;
			return NULL;
		}

		if (stStream.u32Len != 0) {
			/* save audio stream to file */
			(CVI_VOID)fwrite(stStream.pStream, 1, stStream.u32Len, pstAencCtl->pfd);
		}
		//fflush(pstAencCtl->pfd);
		/* finally you must release the stream */
		s32Ret = CVI_AENC_ReleaseStream(pstAencCtl->AeChn, &stStream);
		if (s32Ret != CVI_SUCCESS) {
			printf("%s: CVI_AENC_ReleaseStream(%d), failed with %#x!\n",
			       __func__, pstAencCtl->AeChn, s32Ret);
			pstAencCtl->bStart = CVI_FALSE;
			return NULL;
		}

		if (cap_time) {
			clock_gettime(CLOCK_MONOTONIC, &now);
			if (now.tv_sec > end.tv_sec ||
			    (now.tv_sec == end.tv_sec && now.tv_nsec >= end.tv_nsec)) {
				sem_post(&s_ExitSem);
				break;
			}
		}

	}
	fclose(pstAencCtl->pfd);
	pstAencCtl->bStart = CVI_FALSE;
	return NULL;
}

CVI_VOID SAMPLE_COMM_AUDIO_GET_AencGet_ExitFlag(void)
{
	sem_init(&s_ExitSem, 0, 0);

	while ((sem_wait(&s_ExitSem) != 0)) {
		//use sem as timer
		//
	}
}

CVI_VOID SAMPLE_COMM_AUDIO_GET_AdecSend_ExitFlag(void)
{
	sem_init(&s_ExitSem, 0, 0);

	while ((sem_wait(&s_ExitSem) != 0)) {
		//use sem as timer
		//
	}
}


CVI_VOID *SAMPLE_COMM_AUDIO_AdecAoProc(CVI_VOID *parg)
{
	CVI_S32 s32Ret;
	SAMPLE_ADEC_AO_S *pstAdecAo = (SAMPLE_ADEC_AO_S *)parg;
	AUDIO_FRAME_S stFrame;
	AUDIO_FRAME_S *pstFrame = &stFrame;
	AUDIO_FRAME_INFO_S sDecOutFrm;

	sDecOutFrm.pstFrame = (AUDIO_FRAME_S *)&stFrame;
	printf("Enter SAMPLE_COMM_AUDIO_AdecAoProc\n");

	while (pstAdecAo->bStart) {
		s32Ret = CVI_ADEC_GetFrame(pstAdecAo->AdChn, &sDecOutFrm, CVI_TRUE);
		if (s32Ret != CVI_SUCCESS) {
			printf("%s: CVI_ADEC_GetFrame(%d), failed with %#x!\n",
			       __func__, pstAdecAo->AdChn, s32Ret);
			pstAdecAo->bStart = CVI_FALSE;
			return NULL;
		}

		if (pstFrame->u32Len != 0) {
			s32Ret = CVI_AO_SendFrame(pstAdecAo->AoDev, pstAdecAo->AdChn, pstFrame, 5000);
			if (s32Ret != CVI_SUCCESS) {
				printf("%s: CVI_AO_SendFrame(%d), failed with %#x!\n",
				       __func__, pstAdecAo->AoDev, s32Ret);
				pstAdecAo->bStart = CVI_FALSE;
				return NULL;
			}
		} else
			printf("dec out frame size 0\n");


	}
	printf("leaving SAMPLE_COMM_AUDIO_AdecAoProc\n");
	pstAdecAo->bStart = CVI_FALSE;
	return NULL;

}

CVI_S32 SAMPLE_COMM_AUDIO_CreatTrdAdecAo(ADEC_CHN AdChn, AUDIO_DEV AoDev,
		FILE *pFd)
{
	SAMPLE_ADEC_AO_S *pstAdecAo = NULL;

	pstAdecAo = &gs_stSampleAdecAo[AdChn];
	pstAdecAo->AdChn = AdChn;
	pstAdecAo->AoDev = AoDev;
	pstAdecAo->pfd = pFd;
	pstAdecAo->bStart = CVI_TRUE;

	pthread_create(&pstAdecAo->stPid, 0, SAMPLE_COMM_AUDIO_AdecAoProc, pstAdecAo);

	return CVI_SUCCESS;
}
/******************************************************************************/
/* function : get stream from Aenc, send it  to Adec & save it to file*/
/******************************************************************************/
CVI_VOID *SAMPLE_COMM_AUDIO_AencProc(CVI_VOID *parg)
{
	CVI_S32 s32Ret;

	SAMPLE_AENC_S *pstAencCtl = (SAMPLE_AENC_S *)parg;
	AUDIO_STREAM_S stStream;

	while (pstAencCtl->bStart) {

		/* get stream from aenc chn */
		s32Ret = CVI_AENC_GetStream(pstAencCtl->AeChn, &stStream, CVI_FALSE);
		if (s32Ret != CVI_SUCCESS) {
			printf("%s: CVI_AENC_GetStream(%d), failed with %#x!\n",
			       __func__, pstAencCtl->AeChn, s32Ret);
			pstAencCtl->bStart = CVI_FALSE;
			return NULL;
		}
		/* send stream to decoder and play for testing */
		if (pstAencCtl->bSendAdChn == CVI_TRUE) {
			s32Ret = CVI_ADEC_SendStream(pstAencCtl->AdChn, &stStream, CVI_TRUE);
			if (s32Ret != CVI_SUCCESS) {
				printf("%s: CVI_ADEC_SendStream(%d), failed with %#x!\n",
				       __func__, pstAencCtl->AdChn, s32Ret);
				pstAencCtl->bStart = CVI_FALSE;
				return NULL;
			}
		}
		/* save audio stream to file */
		(CVI_VOID)fwrite(stStream.pStream, 1, stStream.u32Len, pstAencCtl->pfd);
		fflush(pstAencCtl->pfd);
		/* finally you must release the stream */
		s32Ret = CVI_AENC_ReleaseStream(pstAencCtl->AeChn, &stStream);
		if (s32Ret != CVI_SUCCESS) {
			printf("%s: CVI_AENC_ReleaseStream(%d), failed with %#x!\n",
			       __func__, pstAencCtl->AeChn, s32Ret);
			pstAencCtl->bStart = CVI_FALSE;
			return NULL;
		}

	}

	fclose(pstAencCtl->pfd);
	pstAencCtl->bStart = CVI_FALSE;
	return NULL;
}

/******************************************************************************/
/* function : get stream from file, and send it  to Adec*/
/******************************************************************************/
CVI_VOID *SAMPLE_COMM_AUDIO_AdecProc(CVI_VOID *parg)
{
	CVI_S32 s32Ret;
	AUDIO_STREAM_S stAudioStream;

	CVI_U32 u32Len = 320; //testing for g726 //hi set the adec default length= 640
	//CVI_U32 u32Len = 1152; //for mp2
	CVI_U32 u32ReadLen;
	CVI_S32 s32AdecChn;
	CVI_U8 *pu8AudioStream = NULL;
	SAMPLE_ADEC_S *pstAdecCtl = (SAMPLE_ADEC_S *)parg;
	FILE *pfd = pstAdecCtl->pfd;
	struct timespec end;
	struct timespec now;
	int play_time = _get_record_time();

	printf("play_time = %d\n", play_time);
	clock_gettime(CLOCK_MONOTONIC, &now);
	end.tv_sec = now.tv_sec + play_time;
	end.tv_nsec = now.tv_nsec;

	s32AdecChn = pstAdecCtl->AdChn;
	pu8AudioStream = (CVI_U8 *)malloc(sizeof(CVI_U8) * CVI_MAX_AUDIO_STREAM_LEN);

	if (pu8AudioStream == NULL) {
		printf("%s: malloc failed!\n", __func__);
		return NULL;
	}
	if (genAdecType == PT_AAC)
		u32Len = 1024;

	while (pstAdecCtl->bStart == CVI_TRUE) {
		/* read from file */
		stAudioStream.pStream = pu8AudioStream;
		u32ReadLen = fread(stAudioStream.pStream, 1, u32Len, pfd);
		if (u32ReadLen <= 0) {
			s32Ret = CVI_ADEC_SendEndOfStream(s32AdecChn, CVI_FALSE);
			if (s32Ret != CVI_SUCCESS)
				printf("%s: CVI_ADEC_SendEndOfStream failed!\n", __func__);


			if (genAdecType == PT_AAC) {
				sleep(3);
				printf("End Of file play\n");
			} else {
				(CVI_VOID)fseek(pfd, 0, SEEK_SET);/*read file again*/
				printf("audio replay.....loopback file\n");
				continue;
			}
		}

		/* here only demo adec streaming sending mode, but pack sending mode is commended */
		stAudioStream.u32Len = u32ReadLen; //640 bytes
		s32Ret = CVI_ADEC_SendStream(s32AdecChn, &stAudioStream, CVI_TRUE);
		if (s32Ret != CVI_SUCCESS) {
			printf("%s: CVI_ADEC_SendStream(%d) failed with %#x!\n",
			       __func__, s32AdecChn, s32Ret);
			break;
		}
		if (play_time) {
			clock_gettime(CLOCK_MONOTONIC, &now);
			if (now.tv_sec > end.tv_sec ||
			    (now.tv_sec == end.tv_sec && now.tv_nsec >= end.tv_nsec)) {
				sem_post(&s_ExitSem);
				break;
			}
		}
	}

	free(pu8AudioStream);
	pu8AudioStream = NULL;
	fclose(pfd);
	pstAdecCtl->bStart = CVI_FALSE;
	return NULL;
}

/******************************************************************************/
/* function : set ao volume */
/******************************************************************************/
CVI_VOID *SAMPLE_COMM_AUDIO_AoVolProc(CVI_VOID *parg)
{
	CVI_S32 s32Ret;
	CVI_S32 s32Volume;
	AUDIO_DEV AoDev;
	AUDIO_FADE_S stFade;
	SAMPLE_AO_S *pstAoCtl = (SAMPLE_AO_S *)parg;

	AoDev = pstAoCtl->AoDev;

	while (pstAoCtl->bStart) {
		/* range 0~15 (each step 1.5db)[15-0, 0:mute] */
		for (s32Volume = 0; s32Volume <= 14; s32Volume++) {

			s32Ret = CVI_AO_SetVolume(AoDev, s32Volume);
			if (s32Ret != CVI_SUCCESS) {
				printf("%s: [error]CVI_AO_SetVolume(%d), failed with %#x!\n",
				       __func__, AoDev, s32Ret);
			}
			printf("\rset volume %d          ", s32Volume);
			fflush(stdout);
			sleep(2);
		}

		for (s32Volume = 15; s32Volume >= 1; s32Volume--) {

			s32Ret = CVI_AO_SetVolume(AoDev, s32Volume);
			if (s32Ret != CVI_SUCCESS) {
				printf("%s: [error]CVI_AO_SetVolume(%d), failed with %#x!\n",
				       __func__, AoDev, s32Ret);
			}
			printf("\rset volume %d          ", s32Volume);
			fflush(stdout);
			sleep(2);
		}

		for (s32Volume = 0; s32Volume <= 6; s32Volume++) {
			s32Ret = CVI_AO_SetVolume(AoDev, s32Volume);
			if (s32Ret != CVI_SUCCESS) {
				printf("%s: CVI_AO_SetVolume(%d), failed with %#x!\n",
				       __func__, AoDev, s32Ret);
			}
			printf("\rset volume %d          ", s32Volume);
			fflush(stdout);
			sleep(2);
		}

		stFade.bFade         = CVI_TRUE;
		stFade.enFadeInRate  = AUDIO_FADE_RATE_200;
		stFade.enFadeOutRate = AUDIO_FADE_RATE_200;

		s32Ret = CVI_AO_SetMute(AoDev, CVI_TRUE, &stFade);
		if (s32Ret != CVI_SUCCESS) {
			printf("%s: CVI_AO_SetVolume(%d), failed with %#x!\n",
			       __func__, AoDev, s32Ret);
		}
		printf("\rset Ao mute            ");
		fflush(stdout);
		sleep(2);

		s32Ret = CVI_AO_SetMute(AoDev, CVI_FALSE, NULL);

		if (s32Ret != CVI_SUCCESS) {
			printf("%s: CVI_AO_SetVolume(%d), failed with %#x!\n",
			       __func__, AoDev, s32Ret);
		}
		printf("\rset Ao unmute          ");
		fflush(stdout);
		sleep(2);
	}
	return NULL;
}

/******************************************************************************/
/* function : Create the thread to get frame from ai and send to ao */
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_CreatTrdAiAo(AUDIO_DEV AiDev, AI_CHN AiChn,
				       AUDIO_DEV AoDev, AO_CHN AoChn)
{
	SAMPLE_AI_S *pstAi = NULL;

	pstAi = &gs_stSampleAi[AiDev * AI_MAX_CHN_NUM + AiChn];
	pstAi->bSendAenc = CVI_FALSE;
	pstAi->bSendAo = CVI_TRUE;
	pstAi->bStart = CVI_TRUE;
	pstAi->AiDev = AiDev;
	pstAi->AiChn = AiChn;
	pstAi->AoDev = AoDev;
	pstAi->AoChn = AoChn;

	pthread_create(&pstAi->stAiPid, 0, SAMPLE_COMM_AUDIO_AiProc, pstAi);

	return CVI_SUCCESS;
}

/******************************************************************************/
/* function : Create the thread to get frame from ai and send to aenc*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_CreatTrdAiAenc(AUDIO_DEV AiDev, AI_CHN AiChn,
		AENC_CHN AeChn)
{
	SAMPLE_AI_S *pstAi = NULL;

	pstAi = &gs_stSampleAi[AiDev * AI_MAX_CHN_NUM + AiChn];
	pstAi->bSendAenc = CVI_TRUE;
	pstAi->bSendAo = CVI_FALSE;
	pstAi->bStart = CVI_TRUE;
	pstAi->AiDev = AiDev;
	pstAi->AiChn = AiChn;
	pstAi->AencChn = AeChn;
	pthread_create(&pstAi->stAiPid, 0, SAMPLE_COMM_AUDIO_AiProc, pstAi);

	return CVI_SUCCESS;
}

/******************************************************************************/
/* function : Create the thread to get stream from aenc and send to adec*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_CreatTrdAencAdec(AENC_CHN AeChn, ADEC_CHN AdChn,
		FILE *pAecFd)
{
	SAMPLE_AENC_S *pstAenc = NULL;

	if (pAecFd == NULL)
		return CVI_FAILURE;

	pstAenc = &gs_stSampleAenc[AeChn];
	pstAenc->AeChn = AeChn;
	pstAenc->AdChn = AdChn;
	pstAenc->bSendAdChn = CVI_TRUE;
	pstAenc->pfd = pAecFd;
	pstAenc->bStart = CVI_TRUE;
	pthread_create(&pstAenc->stAencPid, 0, SAMPLE_COMM_AUDIO_AencProc, pstAenc);

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_AUDIO_GetAenc(AENC_CHN AeChn, FILE *pAencFd)
{
	SAMPLE_AENC_S *pstAenc = NULL;

	if (pAencFd == NULL) {
		printf("[Error][%s][%d]\n", __func__, __LINE__);
		return CVI_FAILURE;
	}

	pstAenc = &gs_stSampleAenc[AeChn];
	pstAenc->AeChn = AeChn;
	pstAenc->bSendAdChn = CVI_TRUE;
	pstAenc->pfd = pAencFd;
	pstAenc->bStart = CVI_TRUE;
	pthread_create(&pstAenc->stAencPid, 0, SAMPLE_COMM_AUDIO_AencGetProc, pstAenc);
	//pthread_detach(pstAenc->stAencPid);

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_AUDIO_DestoryGetAenc(AENC_CHN AeChn)
{
	SAMPLE_AENC_S *pstAenc = NULL;

	pstAenc = &gs_stSampleAenc[AeChn];
	if (pstAenc->bStart) {
		pstAenc->bStart = CVI_FALSE;
		//pthread_cancel(pstAenc->stAencPid);
		pthread_join(pstAenc->stAencPid, 0);
	}

	return CVI_SUCCESS;
}

/******************************************************************************/
/* function : Create the thread to get stream from file and send to adec*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_CreatTrdFileAdec(ADEC_CHN AdChn, FILE *pAdcFd)
{
	SAMPLE_ADEC_S *pstAdec = NULL;

	if (pAdcFd == NULL)
		return CVI_FAILURE;

	pstAdec = &gs_stSampleAdec[AdChn];
	pstAdec->AdChn = AdChn;
	pstAdec->pfd = pAdcFd;
	pstAdec->bStart = CVI_TRUE;
	pthread_create(&pstAdec->stAdPid, 0, SAMPLE_COMM_AUDIO_AdecProc, pstAdec);

	return CVI_SUCCESS;
}

/******************************************************************************/


/******************************************************************************/
/* function : Create the thread to set Ao volume*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_CreatTrdAoVolCtrl(AUDIO_DEV AoDev)
{
	SAMPLE_AO_S *pstAoCtl = NULL;

	pstAoCtl =  &gs_stSampleAo[AoDev];
	pstAoCtl->AoDev =  AoDev;
	pstAoCtl->bStart = CVI_TRUE;
	pthread_create(&pstAoCtl->stAoPid, 0, SAMPLE_COMM_AUDIO_AoVolProc, pstAoCtl);

	return CVI_SUCCESS;
}


/******************************************************************************/
/* function : Destroy the thread to get frame from ai and send to ao or aenc*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_DestoryTrdAi(AUDIO_DEV AiDev, AI_CHN AiChn)
{
	SAMPLE_AI_S *pstAi = NULL;

	pstAi = &gs_stSampleAi[AiDev * AI_MAX_CHN_NUM + AiChn];
	if (pstAi->bStart) {
		pstAi->bStart = CVI_FALSE;
		//pthread_cancel(pstAi->stAiPid);
		pthread_join(pstAi->stAiPid, 0);
	}


	return CVI_SUCCESS;
}


CVI_S32 SAMPLE_COMM_AUDIO_DestoryTrdAdecAo(ADEC_CHN AdChn)
{
	SAMPLE_ADEC_AO_S *pstAdecAo = NULL;

	pstAdecAo = &gs_stSampleAdecAo[AdChn];
	if (pstAdecAo->bStart) {
		pstAdecAo->bStart = CVI_FALSE;
		//pthread_cancel(pstAenc->stAencPid);
		pthread_join(pstAdecAo->stPid, 0);
	}

	return CVI_SUCCESS;
}
/******************************************************************************/
/* function : Destroy the thread to get stream from aenc and send to adec*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_DestoryTrdAencAdec(AENC_CHN AeChn)
{
	SAMPLE_AENC_S *pstAenc = NULL;

	pstAenc = &gs_stSampleAenc[AeChn];
	if (pstAenc->bStart) {
		pstAenc->bStart = CVI_FALSE;
		//pthread_cancel(pstAenc->stAencPid);
		pthread_join(pstAenc->stAencPid, 0);
	}


	return CVI_SUCCESS;
}

/******************************************************************************/
/* function : Destroy the thread to get stream from file and send to adec*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_DestoryTrdFileAdec(ADEC_CHN AdChn)
{
	SAMPLE_ADEC_S *pstAdec = NULL;

	pstAdec = &gs_stSampleAdec[AdChn];
	if (pstAdec->bStart) {
		pstAdec->bStart = CVI_FALSE;
		//pthread_cancel(pstAdec->stAdPid);
		pthread_join(pstAdec->stAdPid, 0);
	}


	return CVI_SUCCESS;
}

/******************************************************************************/
/* function : Destroy the thread to set Ao volume*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_DestoryTrdAoVolCtrl(AUDIO_DEV AoDev)
{
	SAMPLE_AO_S *pstAoCtl = NULL;

	pstAoCtl =  &gs_stSampleAo[AoDev];
	if (pstAoCtl->bStart) {
		pstAoCtl->bStart = CVI_FALSE;
		pthread_cancel(pstAoCtl->stAoPid);
		pthread_join(pstAoCtl->stAoPid, 0);
	}


	return CVI_SUCCESS;
}

/******************************************************************************/
/* function : Ao bind Adec*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_AoBindAdec(AUDIO_DEV AoDev, AO_CHN AoChn,
				     ADEC_CHN AdChn)
{
	MMF_CHN_S stSrcChn, stDestChn;

	stSrcChn.enModId = CVI_ID_ADEC;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = AdChn;
	stDestChn.enModId = CVI_ID_AO;
	stDestChn.s32DevId = AoDev;
	stDestChn.s32ChnId = AoChn;

	return CVI_AUD_SYS_Bind(&stSrcChn, &stDestChn);
}

/******************************************************************************/
/* function : Ao unbind Adec*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_AoUnbindAdec(AUDIO_DEV AoDev, AO_CHN AoChn,
				       ADEC_CHN AdChn)
{
	MMF_CHN_S stSrcChn, stDestChn;

	stSrcChn.enModId = CVI_ID_ADEC;
	stSrcChn.s32ChnId = AdChn;
	stSrcChn.s32DevId = 0;
	stDestChn.enModId = CVI_ID_AO;
	stDestChn.s32DevId = AoDev;
	stDestChn.s32ChnId = AoChn;
	return CVI_AUD_SYS_UnBind(&stSrcChn, &stDestChn);
}

/******************************************************************************/
/* function : Ao bind Ai*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_AoBindAi(AUDIO_DEV AiDev, AI_CHN AiChn,
				   AUDIO_DEV AoDev, AO_CHN AoChn)
{
	MMF_CHN_S stSrcChn, stDestChn;

	stSrcChn.enModId = CVI_ID_AI;
	stSrcChn.s32ChnId = AiChn;
	stSrcChn.s32DevId = AiDev;
	stDestChn.enModId = CVI_ID_AO;
	stDestChn.s32DevId = AoDev;
	stDestChn.s32ChnId = AoChn;

	return CVI_AUD_SYS_Bind(&stSrcChn, &stDestChn);
}

/******************************************************************************/
/* function : Ao unbind Ai*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_AoUnbindAi(AUDIO_DEV AiDev, AI_CHN AiChn,
				     AUDIO_DEV AoDev, AO_CHN AoChn)
{
	MMF_CHN_S stSrcChn, stDestChn;

	stSrcChn.enModId = CVI_ID_AI;
	stSrcChn.s32ChnId = AiChn;
	stSrcChn.s32DevId = AiDev;
	stDestChn.enModId = CVI_ID_AO;
	stDestChn.s32DevId = AoDev;
	stDestChn.s32ChnId = AoChn;


	return CVI_AUD_SYS_UnBind(&stSrcChn, &stDestChn);
}

/******************************************************************************/
/* function : Aenc bind Ai*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_AencBindAi(AUDIO_DEV AiDev, AI_CHN AiChn,
				     AENC_CHN AeChn)
{
	MMF_CHN_S stSrcChn, stDestChn;

	stSrcChn.enModId = CVI_ID_AI;
	stSrcChn.s32DevId = AiDev;
	stSrcChn.s32ChnId = AiChn;
	stDestChn.enModId = CVI_ID_AENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = AeChn;

	return CVI_AUD_SYS_Bind(&stSrcChn, &stDestChn);
}

/******************************************************************************/
/* function : Aenc unbind Ai*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_AencUnbindAi(AUDIO_DEV AiDev, AI_CHN AiChn,
				       AENC_CHN AeChn)
{
	MMF_CHN_S stSrcChn, stDestChn;

	stSrcChn.enModId = CVI_ID_AI;
	stSrcChn.s32DevId = AiDev;
	stSrcChn.s32ChnId = AiChn;
	stDestChn.enModId = CVI_ID_AENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = AeChn;

	return CVI_AUD_SYS_UnBind(&stSrcChn, &stDestChn);
}

/******************************************************************************/
/* function : Start Ai*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_StartAi(AUDIO_DEV AiDevId, CVI_S32 s32AiChn,
				  AIO_ATTR_S *pstAioAttr, AUDIO_SAMPLE_RATE_E enOutSampleRate,
				  CVI_BOOL bResampleEn, CVI_VOID *pstAiVqeAttr, CVI_U32 u32AiVqeType)
{
	CVI_S32 i = s32AiChn;
	CVI_S32 s32Ret;

	s32Ret = CVI_AI_SetPubAttr(AiDevId, pstAioAttr);
	if (s32Ret) {
		printf("%s: CVI_AI_SetPubAttr(%d) failed with %#x\n", __func__, AiDevId,
		       s32Ret);
		return s32Ret;
	}

	printf("SAMPLE_COMM_AUDIO_StartAi pstAioAttr->enSoundmode(%d)\n",
	       pstAioAttr->enSoundmode);

	printf("s32AiChnCnt[%d]pstAioAttr->enSoundmode[%d]\n", s32AiChn,
	       pstAioAttr->enSoundmode);

	s32Ret = CVI_AI_EnableChn(AiDevId, s32AiChn);
	if (s32Ret) {
		printf("%s: CVI_AI_EnableChn(%d,%d) failed with %#x\n", __func__,
		       AiDevId, i, s32Ret);
		return s32Ret;
	}

	if (pstAiVqeAttr != NULL) {
		printf("enter vqe setting\n");
		CVI_BOOL bAiVqe = CVI_TRUE;

		switch (u32AiVqeType) {
		case 0:
			s32Ret = CVI_SUCCESS;
			bAiVqe = CVI_FALSE;
			break;
		case 1: {
			printf("[Error]CVI AUDIO do not support Record VQE\n");
			printf("[Error]Please setup talk VQE not record VQE ...\n");
			printf("CVI_AUDIO only support Talk VQE(with ANR/AGC/AEC)\n");
			AI_RECORDVQE_CONFIG_S *pstAiVQE = (AI_RECORDVQE_CONFIG_S *)pstAiVqeAttr;

			pstAiVQE->s32BytesPerSample = 2;
			s32Ret = CVI_AI_SetRecordVqeAttr(AiDevId, i,
							 (AI_RECORDVQE_CONFIG_S *)pstAiVqeAttr);
			break;
		}
		case 2: {
#ifndef NEXT_SSP_ALGO
			AI_TALKVQE_CONFIG_S *pstAiVQE_Talk = (AI_TALKVQE_CONFIG_S *)pstAiVqeAttr;

			pstAiVQE_Talk->s32BytesPerSample = 2;
#endif
			s32Ret = CVI_AI_SetTalkVqeAttr(
					 AiDevId,
					 i,
					 0,
					 0,
					 (AI_TALKVQE_CONFIG_S *)pstAiVqeAttr);
			break;
		}
		default:
			s32Ret = CVI_FAILURE;
			break;
		}
		if (s32Ret) {
			printf("%s: SetAiVqe%d(%d,%d) failed with %#x\n", __func__, u32AiVqeType,
			       AiDevId, i, s32Ret);
			return s32Ret;
		}
		if (bAiVqe) {
			s32Ret = CVI_AI_EnableVqe(AiDevId, s32AiChn);
			if (s32Ret) {
				printf("%s: CVI_AI_EnableVqe(%d,%d) failed with %#x\n", __func__,
				       AiDevId, i, s32Ret);
				return s32Ret;
			}
		}
	} else
		printf("not setting vqe\n");


	if (bResampleEn == CVI_TRUE) {
		s32Ret = CVI_AI_EnableReSmp(AiDevId, s32AiChn, enOutSampleRate);
		if (s32Ret) {
			printf("%s: CVI_AI_EnableReSmp(%d,%d) failed with %#x\n", __func__,
			       AiDevId, i, s32Ret);
			return s32Ret;
		}
		bAinResample = CVI_TRUE;
		enAinTargetSampleRate = enOutSampleRate;
	} else {
		bAinResample = CVI_FALSE;
		enAinTargetSampleRate = AUDIO_SAMPLE_RATE_BUTT;

	}
	s32Ret = CVI_AI_Enable(AiDevId);
	if (s32Ret) {
		printf("%s: CVI_AI_Enable(%d) failed with %#x\n", __func__, AiDevId,
		       s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

/******************************************************************************/
/* function : Stop Ai*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_StopAi(AUDIO_DEV AiDevId, CVI_S32 s32AiChn,
				 CVI_BOOL bResampleEn, CVI_BOOL bVqeEn)
{
	CVI_S32 i = s32AiChn;
	CVI_S32 s32Ret;


	if (bResampleEn == CVI_TRUE) {
		s32Ret = CVI_AI_DisableReSmp(AiDevId, i);
		if (s32Ret != CVI_SUCCESS) {
			printf("[Func]:%s [Line]:%d [Info]:%s\n", __func__, __LINE__, "failed");
			return s32Ret;
		}
	}

	if (bVqeEn == CVI_TRUE) {
		s32Ret = CVI_AI_DisableVqe(AiDevId, i);
		if (s32Ret != CVI_SUCCESS) {
			printf("[Func]:%s [Line]:%d [Info]:%s\n", __func__, __LINE__, "failed");
			return s32Ret;
		}
	}

	s32Ret = CVI_AI_DisableChn(AiDevId, i);

	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n", __func__, __LINE__, "failed");
		return s32Ret;
	}

	s32Ret = CVI_AI_Disable(AiDevId);
	if (s32Ret != CVI_SUCCESS) {
		printf("[Func]:%s [Line]:%d [Info]:%s\n", __func__, __LINE__, "failed");
		return s32Ret;
	}

	CVI_AUDIO_DEINIT();


	return CVI_SUCCESS;
}
/******************************************************************************/
/* function : Start Ao*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_StartAo(AUDIO_DEV AoDevId, CVI_S32 s32AoChn,
				  AIO_ATTR_S *pstAioAttr, AUDIO_SAMPLE_RATE_E enInSampleRate,
				  CVI_BOOL bResampleEn)
{
	CVI_S32 s32Ret;
#if 0
	printf("modification SAMPLE_COMM_AUDIO_StartAo\n");
	pstAioAttr->enSamplerate = AUDIO_SAMPLE_RATE_48000;
	pstAioAttr->u32ChnCnt = 1;
	pstAioAttr->u32PtNumPerFrm = 320 * 6;

#endif
	s32Ret = CVI_AO_SetPubAttr(AoDevId, pstAioAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("%s: CVI_AO_SetPubAttr(%d) failed with %#x!\n", __func__,
		       AoDevId, s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = CVI_AO_Enable(AoDevId);
	if (s32Ret != CVI_SUCCESS) {
		printf("%s: CVI_AO_Enable(%d) failed with %#x!\n", __func__, AoDevId,
		       s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = CVI_AO_EnableChn(AoDevId, s32AoChn);
	if (s32Ret != CVI_SUCCESS) {
		printf("%s: CVI_AO_EnableChn(%d) failed with %#x!\n", __func__, 0,
		       s32Ret);
		return CVI_FAILURE;
	}

	if (bResampleEn == CVI_TRUE) {
		s32Ret = CVI_AO_EnableReSmp(AoDevId, s32AoChn, enInSampleRate);
		if (s32Ret != CVI_SUCCESS) {
			printf("%s: [error]CVI_AO_EnableReSmp(%d,%d) failed with %#x!\n", __func__,
			       AoDevId, 0, s32Ret);
			return CVI_FAILURE;
		}
	}

	return CVI_SUCCESS;
}

/******************************************************************************/
/* function : Stop Ao*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_StopAo(AUDIO_DEV AoDevId, CVI_S32 s32AoChn,
				 CVI_BOOL bResampleEn)
{
	CVI_S32 i = s32AoChn;
	CVI_S32 s32Ret;

	if (bResampleEn == CVI_TRUE) {
		s32Ret = CVI_AO_DisableReSmp(AoDevId, i);
		if (s32Ret != CVI_SUCCESS) {
			printf("%s: CVI_AO_DisableReSmp failed with %#x!\n", __func__, s32Ret);
			return s32Ret;
		}
	}

	s32Ret = CVI_AO_DisableChn(AoDevId, i);
	if (s32Ret != CVI_SUCCESS) {
		printf("%s: CVI_AO_DisableChn failed with %#x!\n", __func__, s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_AO_Disable(AoDevId);
	if (s32Ret != CVI_SUCCESS) {
		printf("%s: CVI_AO_Disable failed with %#x!\n", __func__, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

/******************************************************************************/
/* function : Start Aenc*/
/******************************************************************************/
static CVI_S32  _update_aenc_params(AENC_CHN_ATTR_S *pAencAttrs,
				    AIO_ATTR_S *pAioAttrs,
				    PAYLOAD_TYPE_E enType)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (pAencAttrs == NULL || pAioAttrs == NULL) {
		printf("[Error]Attribute NULL !!! Cannot proceed to create AENC channel\n");
		s32Ret = CVI_FAILURE;
	}

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
		printf("Need update detail external AAC function params\n");
		//need update AAC if supported
	} else {
		printf("[Error]Not support codec type[%d]\n", enType);
		s32Ret = CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_AUDIO_StartAenc(CVI_S32 s32AencChn,
				    AIO_ATTR_S *pstAioAttr, PAYLOAD_TYPE_E enType)
{
	AENC_CHN AeChn;
	CVI_S32 s32Ret;
	AENC_CHN_ATTR_S stAencAttr;
	AENC_ATTR_AAC_S  stAencAac;

	s32Ret = _update_aenc_params(&stAencAttr, pstAioAttr, enType);
	if (s32Ret != CVI_SUCCESS) {
		printf("[Error][%s]failure in params\n", __func__);
		return CVI_FAILURE;
	}

//#ifdef SUPPORT_EXTERNAL_AAC
	if (enType == PT_AAC) {

		stAencAac.enAACType = gs_enAacType;
		stAencAac.enBitRate = gs_enAacBps;
		stAencAac.enBitWidth = AUDIO_BIT_WIDTH_16;
		if ((bAinResample == CVI_TRUE) &&
		    (enAinTargetSampleRate != AUDIO_SAMPLE_RATE_BUTT)) {
			stAencAac.enSmpRate = enAinTargetSampleRate;
		} else {
			stAencAac.enSmpRate = pstAioAttr->enSamplerate;
		}
		printf("AAC enc[%s][%d]smp-rate[%d]\n",
		       __func__,
		       __LINE__,
		       stAencAac.enSmpRate);

		stAencAac.enSoundMode = pstAioAttr->enSoundmode;
		stAencAac.enTransType = gs_enAacTransType;
		stAencAac.s16BandWidth = 0;
		stAencAttr.pValue = &stAencAac;
	}
//#endif

	/* If toggle the bFileDbgMode, audio in frame will save to file after encode */
	stAencAttr.bFileDbgMode = CVI_TRUE;
	AeChn = s32AencChn;
	/* create aenc chn*/
	s32Ret = CVI_AENC_CreateChn(AeChn, &stAencAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("%s: CVI_AENC_CreateChn(%d) failed with %#x!\n", __func__,
		       AeChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

/******************************************************************************/
/* function : Stop Aenc*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_StopAenc(CVI_S32 s32AencChn)
{
	CVI_S32 s32Ret;

	s32Ret = CVI_AENC_DestroyChn(s32AencChn);
	if (s32Ret != CVI_SUCCESS) {
		printf("%s: CVI_AENC_DestroyChn(%d) failed with %#x!\n", __func__,
		       s32AencChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

/*******************************************************************************/
/* function : Destroy the all thread*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_DestoryAllTrd(CVI_VOID)
{
	CVI_U32 u32DevId, u32ChnId;

	for (u32DevId = 0; u32DevId < AI_DEV_MAX_NUM; u32DevId++) {
		for (u32ChnId = 0; u32ChnId < AI_MAX_CHN_NUM; u32ChnId++) {
			if (SAMPLE_COMM_AUDIO_DestoryTrdAi(u32DevId, u32ChnId) != CVI_SUCCESS) {
				printf("%s: SAMPLE_COMM_AUDIO_DestoryTrdAi(%d,%d) failed!\n", __func__,
				       u32DevId, u32ChnId);
				return CVI_FAILURE;
			}
		}
	}

	for (u32ChnId = 0; u32ChnId < AENC_MAX_CHN_NUM; u32ChnId++) {
		if (SAMPLE_COMM_AUDIO_DestoryTrdAencAdec(u32ChnId) != CVI_SUCCESS) {
			printf("%s: SAMPLE_COMM_AUDIO_DestoryTrdAencAdec(%d) failed!\n", __func__,
			       u32ChnId);
			return CVI_FAILURE;
		}
	}

	for (u32ChnId = 0; u32ChnId < ADEC_MAX_CHN_NUM; u32ChnId++) {
		if (SAMPLE_COMM_AUDIO_DestoryTrdFileAdec(u32ChnId) != CVI_SUCCESS) {
			printf("%s: SAMPLE_COMM_AUDIO_DestoryTrdFileAdec(%d) failed!\n", __func__,
			       u32ChnId);
			return CVI_FAILURE;
		}
	}

	for (u32ChnId = 0; u32ChnId < AO_DEV_MAX_NUM; u32ChnId++) {
		if (SAMPLE_COMM_AUDIO_DestoryTrdAoVolCtrl(u32ChnId) != CVI_SUCCESS) {
			printf("%s: SAMPLE_COMM_AUDIO_DestoryTrdAoVolCtrl(%d) failed!\n", __func__,
			       u32ChnId);
			return CVI_FAILURE;
		}
	}


	return CVI_SUCCESS;
}


/*******************************************************************************/
/* function : Start Adec*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_StartAdec(ADEC_CHN AdChn, PAYLOAD_TYPE_E enType,
				    ADEC_CHN_ATTR_S *pAdecAttr)
{
	CVI_S32 s32Ret;

	ADEC_CHN_ATTR_S stAdecAttr;
	ADEC_ATTR_ADPCM_S stAdpcm;
	ADEC_ATTR_G711_S stAdecG711;
	ADEC_ATTR_G726_S stAdecG726;
	ADEC_ATTR_LPCM_S stAdecLpcm;
	ADEC_ATTR_AAC_S stAdecAac;

	memset(&stAdecAttr, 0, sizeof(ADEC_CHN_ATTR_S));
	stAdecAttr.enType = enType;
	stAdecAttr.u32BufSize = 20;
	stAdecAttr.enMode = ADEC_MODE_STREAM;/* propose use pack mode in your app */
	stAdecAttr.bFileDbgMode = CVI_TRUE;

	if (stAdecAttr.enType == PT_ADPCMA) {
		stAdecAttr.pValue = &stAdpcm;
		stAdpcm.enADPCMType = AUDIO_ADPCM_TYPE;
	} else if (stAdecAttr.enType == PT_G711A || stAdecAttr.enType == PT_G711U) {
		stAdecAttr.pValue = &stAdecG711;
	} else if (stAdecAttr.enType == PT_G726) {
		stAdecAttr.pValue = &stAdecG726;
		stAdecG726.enG726bps = G726_BPS;
	} else if (stAdecAttr.enType == PT_LPCM) {
		stAdecAttr.pValue = &stAdecLpcm;
		stAdecAttr.enMode = ADEC_MODE_PACK;/* lpcm must use pack mode */
	}

	/* create adec chn*/
	stAdecAttr.s32Sample_rate = pAdecAttr->s32Sample_rate;
	stAdecAttr.s32ChannelNums = pAdecAttr->s32ChannelNums;
	stAdecAttr.s32frame_size = pAdecAttr->s32frame_size;
	stAdecAttr.s32BytesPerSample =
		pAdecAttr->s32BytesPerSample; //16 bits , 2 bytes per samples

	if (stAdecAttr.enType == PT_AAC) {
		stAdecAac.enTransType = gs_enAacTransType;
		if (pAdecAttr->s32ChannelNums == 1)
			stAdecAac.enSoundMode = AUDIO_SOUND_MODE_MONO;
		else
			stAdecAac.enSoundMode = AUDIO_SOUND_MODE_STEREO;
		stAdecAac.enSmpRate = pAdecAttr->s32Sample_rate;
		stAdecAttr.pValue = &stAdecAac;
		stAdecAttr.enMode = ADEC_MODE_STREAM;   /* aac should be stream mode */
		stAdecAttr.s32frame_size = AACLC_SAMPLES_PER_FRAME;
		stAdecAttr.s32BytesPerSample = 2;
	}

	genAdecType = stAdecAttr.enType;
	s32Ret = CVI_ADEC_CreateChn(AdChn, &stAdecAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("%s: CVI_ADEC_CreateChn(%d) failed with %#x!\n", __func__,
		       AdChn, s32Ret);
		return s32Ret;
	}
	return 0;
}

/*******************************************************************************/
/* function : Stop Adec*/
/******************************************************************************/
CVI_S32 SAMPLE_COMM_AUDIO_StopAdec(ADEC_CHN AdChn)
{
	CVI_S32 s32Ret;

	s32Ret = CVI_ADEC_DestroyChn(AdChn);
	if (s32Ret != CVI_SUCCESS) {
		printf("%s: CVI_ADEC_DestroyChn(%d) failed with %#x!\n", __func__,
		       AdChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
/* }*/
#endif
#endif /* End of #ifdef __cplusplus */

