/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cviaudio_kernel_mode.c
 * Description:
 *	 audio kernel_mode interface  functions.
 */
#include<stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <inttypes.h>
#include <pthread.h>
#include "cvi_audio_arch.h"
#include "cviaudio_ioctl_cmd.h"
#include "cviaudio_kernel_mode.h"
#include "cviaudio_rtos_align.h"
#include "cvi_sys.h" //for ion phy buffer
#include "rtos_cmdqu.h"

//macro definition
#ifndef DEFAULT_BYTES_PER_SAMPLE
#define DEFAULT_BYTES_PER_SAMPLE 2//16bit = 2 bytes
#endif
#ifndef ERR_PRINTF
#define ERR_PRINTF(fmt, args...) fprintf(stderr, "[aud_kerMode][err][%s][%d] "fmt, __func__, __LINE__, ##args)
#endif
#ifdef UNUSED_PARAM
#define UNUSED_PARAM(X)	 ((X) = (X))
#endif
#ifndef CVIAUDIO_CALLOC
#define CVIAUDIO_CALLOC(TYPE, COUNT) ((TYPE *)calloc(COUNT, sizeof(TYPE)))
#endif

#ifndef SAFE_FREE_BUF
#define SAFE_FREE_BUF(OBJ) {if (NULL != OBJ) {free(OBJ); OBJ = NULL; } }
#endif


//static function declaration
static CVI_S32 _cviaudio_open_device(void);
static CVI_S32 _cviaudio_close_device(void);
static CVI_S32 _check_aud_kernel_mode_support(void);
static CVI_VOID _attach_vqe_param_blk_mode(AI_TALKVQE_CONFIG_S_RTOS *pstVqeConfigRtos,
					const AI_TALKVQE_CONFIG_S *pstVqeConfig);
static CVI_S32 _init_cviaudio_blk_mode_memory(void);
//global variable
CVI_S32 s32AudioCore = -1;
CVI_S32 s32SavePeriodSize = -1;
AUDIO_FRAME_S gstAudioFrm;
//global variable for block mode memory
CVI_S32 s32RtosCoreKo;
static int devm_rtos_fd = -1;
CVI_S32 NoEchoCancel = -1;
void *gstBlkModeVir;
uint64_t gstBlkModePhy;
#ifndef AEC_FRAMES_LEN
#define AEC_FRAMES_LEN 160
#endif
typedef struct _CVIAUDIO_BLK_MODE_PHY {
	uint64_t gstBlkFrmPhy;
	uint64_t gstBlkMsgInitPhy;
	uint64_t pBlkMicInPhy;
	uint64_t pBlkRefInPhy;
	uint64_t pBlkOutPhy;
	uint64_t gstAinVqeCfgRtosPhy;
} ST_CVIAUDIO_BLK_MODE_PHY;

ST_CVIAUDIO_BLK_MODE_PHY gstBlkPhyAll;
ST_CVIAUDIO_BLOCK_FRAME *gstBlkFrm;
ST_CVIAUDIO_MAILBOX_BLOCK *gstBlkMsgInit;
AI_TALKVQE_CONFIG_S_RTOS *gstAinVqeCfgRtos;
char *pBlkMicIn;
char *pBlkRefIn;
char *pBlkOut;


CVI_S32 _CHECK_NULL_PTR(void *ptr, const char *p_func_name, int line)
{
	if (!ptr) {
		printf("[Error]Null pt[%s][%d]\n", p_func_name, line);
		return 1;

	} else
		return 0;
}

CVI_BOOL _cviaudio_check_audio_kernel_device(void)
{
	if (s32AudioCore <= 0)
		return CVI_FALSE;
	else
		return CVI_TRUE;
}


CVI_S32 _cviaudio_open_cviaudio_device(void)
{
	_cviaudio_open_device();
	return CVI_SUCCESS;
}

static CVI_S32 _check_aud_kernel_mode_support(void)
{
#ifdef AUD_SUPPORT_KERNEL_MODE
	return 1;
#else
	printf("Not support audio kernel mode!!!\n");
	return 0;
#endif
}

static CVI_S32 _cviaudio_open_device(void)
{
	CVI_CHAR devName[255];

	sprintf(devName, "/dev/%s", CVIAUDIO_CEO_DEV_NAME);
	s32AudioCore = open(devName, O_RDWR | O_DSYNC);
	printf("open device (%s) fd: %d\n", devName, s32AudioCore);
	if (s32AudioCore <= 0) {
		printf("open device (%s) FAILURE: %d\n", devName, s32AudioCore);
		return CVI_FAILURE;

	} else
		printf("open device (%s) SUCCESS: %d\n", devName, s32AudioCore);

	return CVI_SUCCESS;

}

static CVI_S32 _cviaudio_close_device(void)
{
	printf("[%s][%d]---->close device in user space\n", __func__, __LINE__);
	if (s32AudioCore == -1)
		return -1;

	if (-1 == close(s32AudioCore)) {
		fprintf(stderr, "%s: fd(%d) failure\n", __func__, s32AudioCore);
		return -1;
	}

	s32AudioCore = -1;

	return CVI_SUCCESS;
}

static CVI_S32 _init_cviaudio_blk_mode_memory(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int total_size = 0;

	if (gstBlkModeVir) {
		printf("gstBlkModeVir exist!!\n");
		return 0;
	}
	s32Ret = CVI_SYS_IonAlloc((CVI_U64 *)&gstBlkModePhy,
				(CVI_VOID **)&gstBlkModeVir,
				"cviaudio_kernel_blk_msg",
				8 * 1024);

	if (s32Ret != CVI_SUCCESS) {
		ERR_PRINTF("CVI_SYS_IonAlloc_Cached failure\n");
		return -1;

	} else {
		gstBlkPhyAll.gstBlkFrmPhy = gstBlkModePhy;
		gstBlkFrm = (ST_CVIAUDIO_BLOCK_FRAME *)gstBlkModeVir;
		total_size += sizeof(ST_CVIAUDIO_BLOCK_FRAME);
		total_size = CVIAUDIO_ALIGN(total_size, CVIAUDIO_BYTES_ALIGNMENT);

		gstBlkPhyAll.gstBlkMsgInitPhy = gstBlkModePhy + total_size;
		gstBlkMsgInit = (ST_CVIAUDIO_MAILBOX_BLOCK *)((size_t)gstBlkModeVir + total_size);
		total_size += sizeof(ST_CVIAUDIO_MAILBOX_BLOCK);
		total_size = CVIAUDIO_ALIGN(total_size, CVIAUDIO_BYTES_ALIGNMENT);

		gstBlkPhyAll.pBlkMicInPhy = gstBlkModePhy + total_size;
		pBlkMicIn = (char *)((size_t)gstBlkModeVir + total_size);
		total_size += 1280; //setup 1280 bytes(2 frame 2 x 160) for frame buffer
		total_size = CVIAUDIO_ALIGN(total_size, CVIAUDIO_BYTES_ALIGNMENT);

		gstBlkPhyAll.pBlkRefInPhy = gstBlkModePhy + total_size;
		pBlkRefIn = (char *)((size_t)gstBlkModeVir + total_size);
		total_size += 1280; //setup 1280 bytes(2 frame 2 x 160) for frame buffer
		total_size = CVIAUDIO_ALIGN(total_size, CVIAUDIO_BYTES_ALIGNMENT);

		gstBlkPhyAll.pBlkOutPhy = gstBlkModePhy + total_size;
		pBlkOut = (char *)((size_t)gstBlkModeVir + total_size);
		total_size += 1280; //setup 1280 bytes(2 frame 2 x 160) for frame buffer
		total_size = CVIAUDIO_ALIGN(total_size, CVIAUDIO_BYTES_ALIGNMENT);

		gstBlkPhyAll.gstAinVqeCfgRtosPhy = gstBlkModePhy + total_size;
		gstAinVqeCfgRtos = (AI_TALKVQE_CONFIG_S_RTOS *)((size_t)gstBlkModeVir + total_size);
		total_size += sizeof(AI_TALKVQE_CONFIG_S_RTOS);
		total_size = CVIAUDIO_ALIGN(total_size, CVIAUDIO_BYTES_ALIGNMENT);
		if (total_size > 4 * 1024) {
			ERR_PRINTF("Fatal error in memory size !!!\n");
			return -1;
		}
	}

	return 1;
}

static CVI_VOID _attach_vqe_param_blk_mode(AI_TALKVQE_CONFIG_S_RTOS *pAinVqeCfgRtos,
					const AI_TALKVQE_CONFIG_S *pAinVqeCfg)
{
	if ((!pAinVqeCfgRtos) || (!pAinVqeCfg)) {
		ERR_PRINTF("Null pt detect\n");
		return;
	}
	pAinVqeCfgRtos->u32OpenMask = pAinVqeCfg->u32OpenMask;
	pAinVqeCfgRtos->s32WorkSampleRate = pAinVqeCfg->s32WorkSampleRate;
	pAinVqeCfgRtos->stAecCfg.para_aec_filter_len = pAinVqeCfg->stAecCfg.para_aec_filter_len;
	pAinVqeCfgRtos->stAecCfg.para_aes_std_thrd = pAinVqeCfg->stAecCfg.para_aes_std_thrd;
	pAinVqeCfgRtos->stAecCfg.para_aes_supp_coeff = pAinVqeCfg->stAecCfg.para_aes_supp_coeff;
	pAinVqeCfgRtos->stAnrCfg.para_nr_snr_coeff = pAinVqeCfg->stAnrCfg.para_nr_snr_coeff;
	pAinVqeCfgRtos->stAnrCfg.para_nr_init_sile_time = pAinVqeCfg->stAnrCfg.para_nr_init_sile_time;
	pAinVqeCfgRtos->stAgcCfg.para_agc_max_gain = pAinVqeCfg->stAgcCfg.para_agc_max_gain;
	pAinVqeCfgRtos->stAgcCfg.para_agc_target_high = pAinVqeCfg->stAgcCfg.para_agc_target_high;
	pAinVqeCfgRtos->stAgcCfg.para_agc_target_low = pAinVqeCfg->stAgcCfg.para_agc_target_low;
	pAinVqeCfgRtos->stAgcCfg.para_agc_vad_ena = pAinVqeCfg->stAgcCfg.para_agc_vad_ena;
	pAinVqeCfgRtos->stAecDelayCfg.para_aec_init_filter_len =
		pAinVqeCfg->stAecDelayCfg.para_aec_init_filter_len;
	pAinVqeCfgRtos->stAecDelayCfg.para_dg_target = pAinVqeCfg->stAecDelayCfg.para_dg_target;
	pAinVqeCfgRtos->stAecDelayCfg.para_delay_sample = pAinVqeCfg->stAecDelayCfg.para_delay_sample;
	pAinVqeCfgRtos->para_notch_freq = pAinVqeCfg->para_notch_freq;
}

CVI_S32 CVI_AI_EnableExtSSp(CVI_S32 s32SampleRate, CVI_S32 s32PeriodSize,
				const AI_TALKVQE_CONFIG_S *pstVqeConfig)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ST_SSP_PCM_MSG stSspPcmMsg;
	ST_SSP_MSG stSspMsg;
	int ret = 0;

	if (!_check_aud_kernel_mode_support())
		return CVI_FAILURE;
	memset(&stSspPcmMsg, 0, sizeof(ST_SSP_PCM_MSG));
	memset(&stSspMsg, 0, sizeof(ST_SSP_MSG));

	if (_CHECK_NULL_PTR((void *)pstVqeConfig, __func__, __LINE__))
		printf("SSP turned off...only original single chn record\n!");
	else
		memcpy(&stSspMsg.stVqeConfig, pstVqeConfig, sizeof(AI_TALKVQE_CONFIG_S));


	if (s32AudioCore <= 0) {
		s32Ret = _cviaudio_open_device();
		if (s32Ret != CVI_SUCCESS) {
			printf("[Error][%s][%d]...cannot open cviaudio core ko dev\n",
				__func__, __LINE__);
			return CVI_FAILURE;
		}
	}


	if ((stSspMsg.stVqeConfig.u32OpenMask | LP_AEC_ENABLE) | NLP_AES_ENABLE) {
		//do the AEC , record in required 2 chn
		stSspPcmMsg.channels = 2;
	} else
		stSspPcmMsg.channels = 1;
	stSspPcmMsg.rate = s32SampleRate;
	stSspPcmMsg.period_size = s32PeriodSize;
	s32SavePeriodSize = s32PeriodSize;

	ret = ioctl(s32AudioCore, CVIAUDIO_IOCTL_SSP_UPDATE_PCM_RECORD_CFG, &stSspPcmMsg);
	if (ret != 0) {
		ERR_PRINTF("ioctl[%d] CVIAUDIO_IOCTL_SSP_UPDATE_PCM_RECORD_CFG fail with %d\n",
			CVIAUDIO_IOCTL_SSP_UPDATE_PCM_RECORD_CFG, ret);
		return CVI_FAILURE;
	}

	stSspMsg.bytes_per_period = s32PeriodSize * DEFAULT_BYTES_PER_SAMPLE;
	stSspMsg.channel_cnt = stSspPcmMsg.channels;
	ret = ioctl(s32AudioCore, CVIAUDIO_IOCTL_SSP_INIT, &stSspMsg);
	if (ret != 0) {
		ERR_PRINTF("ioctl[%d] CVIAUDIO_IOCTL_SSP_INIT fail with %d\n",
			CVIAUDIO_IOCTL_SSP_INIT, ret);
		return  CVI_FAILURE;
	}
	//allocate the audio frame
	gstAudioFrm.u64VirAddr[0] = CVIAUDIO_CALLOC(CVI_U8,
					stSspMsg.bytes_per_period * stSspMsg.channel_cnt);
	gstAudioFrm.u64PhyAddr[0] = 0;

	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_GetFrameExtSsp(AUDIO_FRAME_S *pstFrm)
{
	ST_SSP_DATA_MSG stSspDataMsg;
	int ret = 0;
	int check_cnt = 0;
#define RETRY_CNT_FOR_CVIAUDIO_CORE 100
	if (!_check_aud_kernel_mode_support())
		return CVI_FAILURE;

	if (_CHECK_NULL_PTR(pstFrm, __func__, __LINE__))
		return CVI_FAILURE;

	if (_CHECK_NULL_PTR(gstAudioFrm.u64VirAddr[0], __func__, __LINE__))
		return CVI_FAILURE;

	pstFrm->u32Len = 0;
	do {
		pstFrm->u64PhyAddr[0] = gstAudioFrm.u64PhyAddr[0];
		pstFrm->u64VirAddr[0] = gstAudioFrm.u64VirAddr[0];
		stSspDataMsg.data_phyaddr = pstFrm->u64PhyAddr[0];
		stSspDataMsg.data_addr = (char *)pstFrm->u64VirAddr[0];
		stSspDataMsg.required_size_bytes = s32SavePeriodSize * DEFAULT_BYTES_PER_SAMPLE;
		stSspDataMsg.data_valid = 0;
		ret = ioctl(s32AudioCore, CVIAUDIO_IOCTL_SSP_PROC, &stSspDataMsg);

		if (ret != 0) {
			ERR_PRINTF("ioctl[%d] CVIAUDIO_IOCTL_SSP_PROC fail with %d\n",
				CVIAUDIO_IOCTL_SSP_PROC, ret);
			return CVI_FAILURE;
		}
		check_cnt += 1;
		if (stSspDataMsg.data_valid <= 0 && check_cnt < RETRY_CNT_FOR_CVIAUDIO_CORE) {
			usleep(5000);
			if (check_cnt > 90) {
				printf("[Error][%s][%d]Retry count up to limit, check the cviaudio_core condition\n",
					__func__, __LINE__);
			}
			continue;
		} else
			break;
	} while (1);
	if (stSspDataMsg.data_valid <= 0) {
		ERR_PRINTF("RTOS SSP data invalid xxxxxxxxxxxxxxx\n");
		return CVI_FAILURE;
	}
	pstFrm->u32Len = s32SavePeriodSize;

	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_DisableExtSsp(void)
{
	int ret = 0;

	if (!_check_aud_kernel_mode_support())
		return CVI_FAILURE;

	ret = ioctl(s32AudioCore, CVIAUDIO_IOCTL_SSP_DEINIT, NULL);
	if (ret != 0) {
		ERR_PRINTF("ioctl[%d] CVIAUDIO_IOCTL_SSP_DEINIT fail with %d\n",
			CVIAUDIO_IOCTL_SSP_DEINIT, ret);
		return CVI_FAILURE;
	}
	if (gstAudioFrm.u64VirAddr[0])
		SAFE_FREE_BUF(gstAudioFrm.u64VirAddr[0]);

	_cviaudio_close_device();
	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_EnableExtSSp_BlkMode(CVI_S32 s32SampleRate, CVI_S32 s32PeriodSize,
				const AI_TALKVQE_CONFIG_S *pstVqeConfig)
{
	CVI_S32 s32Ret = 0;
	CVI_CHAR devName[255];
	cmdqu_t cmdq;
	int ret = 0;

	if ((s32SampleRate != 8000
		&& s32SampleRate != 16000)) {
		ERR_PRINTF("invalid parameter:%d, %d\n",
			s32SampleRate, s32PeriodSize);
		return CVI_FAILURE;
	}
	if ((s32PeriodSize % 160) != 0) {
		ERR_PRINTF("invalid parameter:%d, %d\n",
			s32SampleRate, s32PeriodSize);
		return CVI_FAILURE;
	}

	if (!pstVqeConfig) {
		ERR_PRINTF("Null pt detect\n");
		return CVI_FAILURE;
	}
	//init the ion and attach phy memory
	s32Ret = _init_cviaudio_blk_mode_memory();
	if (s32Ret < 0) {
		ERR_PRINTF("Error in required phy memory\n");
		return CVI_FAILURE;

	} else if (s32Ret == 0)
		ERR_PRINTF("already init cviaudio block mode memory\n");
	else
		printf("init cviaudio_kernel_mode memory success!!\n");

	//attach the rtos structure vqe parameters
	_attach_vqe_param_blk_mode(gstAinVqeCfgRtos, pstVqeConfig);

	if (((pstVqeConfig->u32OpenMask & LP_AEC_ENABLE) == 0) &&
	    ((pstVqeConfig->u32OpenMask & NLP_AES_ENABLE) == 0)) {
		//only exist AGC/ANR or else

		NoEchoCancel = 1;
	} else {

		NoEchoCancel = 0;
	}
	//send out the rtos command to cmdqu
	sprintf(devName, "/dev/%s", RTOS_CMDQU_DEV_NAME);
	s32RtosCoreKo = open(devName, O_RDWR | O_DSYNC);
	printf("open device (%s) fd: %d\n", devName, s32RtosCoreKo);
	if (s32RtosCoreKo <= 0) {
		printf("open device (%s) FAILURE: %d\n", devName, __LINE__);
		return CVI_FAILURE;

	} else
		printf("open device (%s) SUCCESS: %d\n", devName, __LINE__);

	if (devm_rtos_fd == -1) {
		printf("open rtos devm for fd\n");
		devm_rtos_fd = open("/dev/mem", O_RDWR | O_SYNC);
		if (devm_rtos_fd == -1) {
			printf("[cviaudio][kernel_mode]cannot open '/dev/mem'\n");
		}
	}

	memset(&cmdq, 0, sizeof(struct cmdqu_t));
	gstBlkMsgInit->u64RevMask = 0xbb;
	gstBlkMsgInit->AinVqeCfgPhy = gstBlkPhyAll.gstAinVqeCfgRtosPhy;
	cmdq.ip_id = IP_AUDIO;
	cmdq.cmd_id = CVIAUDIO_RTOS_CMD_SSP_INIT_BLOCK;
	cmdq.block = 1;
	cmdq.resv.mstime = -1;
	cmdq.param_ptr = gstBlkPhyAll.gstBlkMsgInitPhy;
	printf("[%s][%d]...send init\n", __func__, __LINE__);
	ret = ioctl(s32RtosCoreKo, RTOS_CMDQU_SEND_WAIT, &cmdq);
	printf("[%s][%d]...send init out\n", __func__, __LINE__);
	if (ret) {
		printf("[%s][%d]...ioctl error init\n",
			__func__, __LINE__);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_GetFrameExtSsp_BlkMode(CVI_CHAR *datain,
					CVI_CHAR *dataout,
					CVI_S32 s32SizeInBytes,
					CVI_S32 *s32SizeOutBytes)
{
	cmdqu_t cmdq;
	CVI_S16 *mic_in;
	CVI_S16 *ref_in;
	CVI_S16 *ps16DataIn;
	int i;
	int ret = 0;
	int chn_cnt = (NoEchoCancel == 1) ? 1 : 2;
	CVI_S32 s32RemainSizeSample = (s32SizeInBytes / 2) / chn_cnt;

	if ((!datain) || (!dataout) || (!pBlkMicIn) || (!pBlkRefIn) || (!pBlkOut)) {
		ERR_PRINTF("Null pt detect!!\n");
		return CVI_FAILURE;
	}

	*s32SizeOutBytes = 0;
	//TODO: need check currently only AGC/ANR or not
	//default go to AEC case, not exist AGC/ANR only case
	mic_in = (CVI_S16 *)pBlkMicIn;
	ref_in = (CVI_S16 *)pBlkRefIn;

	//call the disable cmd to rtos_cmdqu
	memset(&cmdq, 0, sizeof(struct cmdqu_t));
	gstBlkFrm->mic_in_addr = gstBlkPhyAll.pBlkMicInPhy;
	gstBlkFrm->ref_in_addr = (NoEchoCancel == 1) ? 0 : gstBlkPhyAll.pBlkRefInPhy;
	gstBlkFrm->output_addr = gstBlkPhyAll.pBlkOutPhy;
	gstBlkFrm->u64RevMask = 0xbb;

	while (s32RemainSizeSample >= AEC_FRAMES_LEN) {
		ps16DataIn = (CVI_S16 *)datain;
		for (i = 0; i < AEC_FRAMES_LEN; i++) {
			if (NoEchoCancel == 1) {
				mic_in[i] = ps16DataIn[i];
			} else {
				mic_in[i] = ps16DataIn[i * 2];
				ref_in[i] = ps16DataIn[i * 2 + 1];
			}
		}
		cmdq.ip_id = IP_AUDIO;
		cmdq.cmd_id = CVIAUDIO_RTOS_CMD_SSP_PROC_BLOCK;
		cmdq.block = 1;
		cmdq.resv.mstime = -1;
		cmdq.param_ptr = gstBlkPhyAll.gstBlkFrmPhy;

		ret = ioctl(s32RtosCoreKo, RTOS_CMDQU_SEND_WAIT, &cmdq);
		if (ret) {
			printf("[%s][%d]...ioctl error init\n",
				__func__, __LINE__);
			return CVI_FAILURE;
		}

		CVI_SYS_IonInvalidateCache(gstBlkModePhy, gstBlkModeVir,  4*1024);
		s32RemainSizeSample -= (AEC_FRAMES_LEN);
		memcpy(dataout, pBlkOut, AEC_FRAMES_LEN * sizeof(CVI_S16));

		*s32SizeOutBytes += (sizeof(CVI_S16) * AEC_FRAMES_LEN);
		if (s32RemainSizeSample) {
			//if still reamin samples in buffer, then moving buffer pt
			//2 channel audio input in datain
			datain += AEC_FRAMES_LEN * 2 * chn_cnt;
			dataout += (sizeof(CVI_S16) * AEC_FRAMES_LEN);
		}

	}

	if (s32RemainSizeSample != 0) {
		ERR_PRINTF("remaining data ...not process AEC\n");
		if (s32RemainSizeSample != 0) {
			/* Not return */
			ERR_PRINTF("Please set frame sample 160x..remain[%d]smpls\n", s32RemainSizeSample);
		}
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_AI_DisableExtSsp_BlkMode(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	cmdqu_t cmdq;
	int ret = 0;

	//call the disable cmd to rtos_cmdqu
	memset(&cmdq, 0, sizeof(struct cmdqu_t));
	cmdq.ip_id = IP_AUDIO;
	cmdq.cmd_id = CVIAUDIO_RTOS_CMD_SSP_DEINIT_BLOCK;
	cmdq.block = 1;
	cmdq.resv.mstime = -1;
	cmdq.param_ptr = 0;
	printf("[%s][%d]...send deinit\n", __func__, __LINE__);
	ret = ioctl(s32RtosCoreKo, RTOS_CMDQU_SEND_WAIT, &cmdq);
	printf("[%s][%d]...send deinit out\n", __func__, __LINE__);
	if (ret) {
		printf("[%s][%d]...ioctl error init\n",
			__func__, __LINE__);
		return CVI_FAILURE;
	}

	//free the global memory from ion
	//gstBlkModeVir
	if (gstBlkModeVir) {
		s32Ret = CVI_SYS_IonFree((CVI_U64)gstBlkModePhy,
					(CVI_VOID *)gstBlkModeVir);
		if (s32Ret != CVI_SUCCESS)
			ERR_PRINTF("Error in CVI_SYS_IonFree\n");
		gstBlkModeVir = NULL;
	} else
		ERR_PRINTF("No available gstBlkModeVir address!!\n");

	return CVI_SUCCESS;
}


