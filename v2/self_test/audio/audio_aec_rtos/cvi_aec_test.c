/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: sample/cvi_aec_test.c
 * Description:
 * This example C main file shows how to call entry-point functions and read input signals.
 * You must customize this file for your development environment/platform
 * Modify it and integrate it into your own development environment/platform.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#ifdef __CV181X__
#include <cvi_type.h>
#else
#include "cvi_type.h"
#endif
#include "cvi_comm_aio.h"
#include "cvi_audio.h"
#include "cvi_audio_vqe.h"
#include "cvi_audio_loadcheck.h"

#define AEC_INPUT_CHANNEL 2
#define AEC_INPUT_PERIOD_SIZE 160
#ifndef DEFAULT_BYTES_PER_SAMPLE
#define DEFAULT_BYTES_PER_SAMPLE 2
#endif



int main(int argc, char const *argv[])
{
#define SAMPLE_AUDIO_AEC_USER_PARAM  -1
#define SAMPLE_AUDIO_MIPS_AEC_MIN 1
#define SAMPLE_AUDIO_MIPS_AEC_MAX 2
#define SAMPLE_AUDIO_MIPS_AEC_AES_MIN 3
#define SAMPLE_AUDIO_MIPS_AEC_AES_MAX 4
#define SAMPLE_AUDIO_MIPS_NR 5
#define SAMPLE_AUDIO_MIPS_AGC 6
#define SAMPLE_AUDIO_MIPS_AEC_NR_AGC_MIN 7
#define SAMPLE_AUDIO_MIPS_AEC_NR_AGC_MAX 8
#define SAMPLE_AUDIO_MIPS_AEC_AES_NR_AGC_MIN 9
#define SAMPLE_AUDIO_MIPS_AEC_AES_NR_AGC_MAX 10

	FILE *fp_in_wav = NULL;
	FILE *fp_out_wav = NULL;
	char *pout_wav_name = NULL;
	CVI_S16 *pcm_frame = NULL;
	CVI_S16 *ptmp_buf = NULL;
	CVI_S32 read_size = 0;
	CVI_S32 s32MacSize = 0;
	CVI_S32 s32InputBytes = 0;
	struct timeval tv1;
	struct timeval tv2;
	struct timezone tz;
	CVI_FLOAT duration = 0.0f;
	CVI_S32 input_arg = 0;
	int retLen;
	AI_TALKVQE_CONFIG_S pstAiVqeAttr;
	AI_AEC_CONFIG_S default_AEC_Setting;
	AUDIO_AGC_CONFIG_S default_AGC_Setting;
	AUDIO_ANR_CONFIG_S  default_ANR_Setting;
	AUDIO_DELAY_CONFIG_S default_AecDelayCfg;
	int use_default_parameters = 0;
	int sr  = 16000;
	void *ploadhandle = NULL;
	int load_check_sec = 3;
	//int aec_filter_len = 1;
	printf("***********************************************\n"
	       "***Audio AEC TEST[internal test] ***\n"
	       "usage	: <input .raw> <output.raw> <sample rate>\n"
	       "***********************************************\n");
	if (argc < 4) {
		printf("[Error]Please check the usage\n");
		printf("[Error]Input argument is not enough!!!");
		return -1;
	}

	CVI_S32 ret;

	sr = atoi(argv[3]);
	if (sr != 8000 && sr != 16000) {
		printf("sample_rate error,not support:%d.\n", sr);
		return -2;
	}

	if ((argc >= 5) && (strcmp(argv[4], "-default") == 0)) {
		printf("using unit test mode for SQA argc[%d]\n", argc);
		if (argc == 6) {
			use_default_parameters = atoi(argv[5]);
		}
		printf("sample_audio_aec using default AEC parameters[%d]!!!\n",
							use_default_parameters);

	} else {
		printf("User enter parameters in console\n");
		use_default_parameters = SAMPLE_AUDIO_AEC_USER_PARAM;
	}


	memset(&pstAiVqeAttr, 0, sizeof(AI_TALKVQE_CONFIG_S));
	memset(&default_AGC_Setting, 0, sizeof(AUDIO_AGC_CONFIG_S));
	memset(&default_ANR_Setting, 0, sizeof(AUDIO_ANR_CONFIG_S));
	memset(&default_AEC_Setting, 0, sizeof(AI_AEC_CONFIG_S));
	if (use_default_parameters > SAMPLE_AUDIO_AEC_USER_PARAM) {
		printf("using default AEC parameters[%d]!!!\n",
							use_default_parameters);
		default_AEC_Setting.para_aec_filter_len = 13;
		default_AEC_Setting.para_aes_std_thrd = 37;
		default_AEC_Setting.para_aes_supp_coeff = 60;
		default_AGC_Setting.para_agc_max_gain = 0;
		default_AGC_Setting.para_agc_target_high = 2;
		default_AGC_Setting.para_agc_target_low = 72;
		default_AGC_Setting.para_agc_vad_ena = 0;
		memset(&default_ANR_Setting, 0, sizeof(AUDIO_ANR_CONFIG_S));
		default_ANR_Setting.para_nr_snr_coeff = 15;
		default_ANR_Setting.para_nr_init_sile_time = 0;
		memset(&default_AecDelayCfg, 0, sizeof(AUDIO_DELAY_CONFIG_S));
		default_AecDelayCfg.para_aec_init_filter_len = 2;
		default_AecDelayCfg.para_dg_target = 1;
		default_AecDelayCfg.para_delay_sample = 1;
#ifdef __CV181X__
		int ssp_path = 0;

		printf("enter ssp path option:\t");
		scanf("%d", &ssp_path);
		printf("the ssp_path is [%d]\n", ssp_path);
		if (CVI_VQE_PathSelect(ssp_path) != CVI_SUCCESS) {
			printf("CVI_VQE_PathSelect error\n");
		} else
			printf("CVI_VQE_PathSelect ok...going next!!\n");


#endif
		if (use_default_parameters == SAMPLE_AUDIO_MIPS_AEC_MIN) {
			pstAiVqeAttr.u32OpenMask = LP_AEC_ENABLE;
			default_AEC_Setting.para_aec_filter_len = 1;
			default_AecDelayCfg.para_aec_init_filter_len = 1;
			default_AGC_Setting.para_agc_vad_ena = 0;
			ploadhandle = CVI_AUDIO_LoadingCheck_Init(sr, 2, load_check_sec, "AEC_min");
		} else if (use_default_parameters == SAMPLE_AUDIO_MIPS_AEC_MAX) {
			pstAiVqeAttr.u32OpenMask = LP_AEC_ENABLE;
			default_AEC_Setting.para_aec_filter_len = 13;
			default_AecDelayCfg.para_aec_init_filter_len = 13;
			default_AGC_Setting.para_agc_vad_ena = 0;
			ploadhandle = CVI_AUDIO_LoadingCheck_Init(sr, 2, load_check_sec, "AEC_max");
		} else if (use_default_parameters == SAMPLE_AUDIO_MIPS_AEC_AES_MIN) {
			pstAiVqeAttr.u32OpenMask = LP_AEC_ENABLE | NLP_AES_ENABLE;
			default_AEC_Setting.para_aec_filter_len = 1;
			default_AecDelayCfg.para_aec_init_filter_len = 1;
			default_AGC_Setting.para_agc_vad_ena = 0;
			ploadhandle = CVI_AUDIO_LoadingCheck_Init(sr, 2, load_check_sec, "AEC_AES_min");
		} else if (use_default_parameters == SAMPLE_AUDIO_MIPS_AEC_AES_MAX) {
			pstAiVqeAttr.u32OpenMask = LP_AEC_ENABLE | NLP_AES_ENABLE;
			default_AEC_Setting.para_aec_filter_len = 13;
			default_AecDelayCfg.para_aec_init_filter_len = 13;
			default_AGC_Setting.para_agc_vad_ena = 0;
			ploadhandle = CVI_AUDIO_LoadingCheck_Init(sr, 2, load_check_sec, "AEC_AES_max");
		} else if (use_default_parameters == SAMPLE_AUDIO_MIPS_NR) {
			pstAiVqeAttr.u32OpenMask = NR_ENABLE;
			default_AGC_Setting.para_agc_vad_ena = 0;
			ploadhandle = CVI_AUDIO_LoadingCheck_Init(sr, 1, load_check_sec, "NR");
		} else if (use_default_parameters == SAMPLE_AUDIO_MIPS_AGC) {
			pstAiVqeAttr.u32OpenMask = AGC_ENABLE;
			default_AGC_Setting.para_agc_vad_ena = 0;
			ploadhandle = CVI_AUDIO_LoadingCheck_Init(sr, 1, load_check_sec, "AGC");
		} else if (use_default_parameters == SAMPLE_AUDIO_MIPS_AEC_NR_AGC_MIN) {
			pstAiVqeAttr.u32OpenMask = AGC_ENABLE | NR_ENABLE | LP_AEC_ENABLE;
			default_AGC_Setting.para_agc_vad_ena = 1;
			default_AEC_Setting.para_aec_filter_len = 1;
			default_AecDelayCfg.para_aec_init_filter_len = 1;
			ploadhandle = CVI_AUDIO_LoadingCheck_Init(sr, 2, load_check_sec, "AEC_NR_AGC_min");
		} else if (use_default_parameters == SAMPLE_AUDIO_MIPS_AEC_NR_AGC_MAX) {
			pstAiVqeAttr.u32OpenMask = AGC_ENABLE | NR_ENABLE | LP_AEC_ENABLE;
			default_AGC_Setting.para_agc_vad_ena = 1;
			default_AEC_Setting.para_aec_filter_len = 13;
			default_AecDelayCfg.para_aec_init_filter_len = 13;
			ploadhandle = CVI_AUDIO_LoadingCheck_Init(sr, 2, load_check_sec, "AEC_NR_AGC_max");
		} else if (use_default_parameters == SAMPLE_AUDIO_MIPS_AEC_AES_NR_AGC_MIN) {
			pstAiVqeAttr.u32OpenMask = AGC_ENABLE | NR_ENABLE | LP_AEC_ENABLE | NLP_AES_ENABLE;
			default_AGC_Setting.para_agc_vad_ena = 1;
			default_AEC_Setting.para_aec_filter_len = 1;
			default_AecDelayCfg.para_aec_init_filter_len = 1;
			ploadhandle = CVI_AUDIO_LoadingCheck_Init(sr, 2, load_check_sec, "AEC_AES_NR_AGC_min");
		} else if (use_default_parameters == SAMPLE_AUDIO_MIPS_AEC_AES_NR_AGC_MAX) {
			pstAiVqeAttr.u32OpenMask = AGC_ENABLE | NR_ENABLE | LP_AEC_ENABLE | NLP_AES_ENABLE;
			default_AGC_Setting.para_agc_vad_ena = 1;
			default_AEC_Setting.para_aec_filter_len = 13;
			default_AecDelayCfg.para_aec_init_filter_len = 13;
			ploadhandle = CVI_AUDIO_LoadingCheck_Init(sr, 2, load_check_sec, "AEC_AES_NR_AGC_max");
		} else {
			printf("option for default setting in[%s][%d]\n", __func__, use_default_parameters);
			printf("Not doing any cpu loading check!!\n");
		}

	} else {
		ploadhandle = CVI_AUDIO_LoadingCheck_Init(sr, 2, load_check_sec, "sample_audio_aec");
		default_AGC_Setting.para_agc_max_gain = 0;
		default_AGC_Setting.para_agc_target_high = 2;
		default_AGC_Setting.para_agc_target_low = 72;
		default_AGC_Setting.para_agc_vad_ena = 1;
		memset(&default_ANR_Setting, 0, sizeof(AUDIO_ANR_CONFIG_S));
		default_ANR_Setting.para_nr_snr_coeff = 15;
		default_ANR_Setting.para_nr_init_sile_time = 0;
		memset(&default_AecDelayCfg, 0, sizeof(AUDIO_DELAY_CONFIG_S));
		default_AecDelayCfg.para_aec_init_filter_len = 2;
		default_AecDelayCfg.para_dg_target = 1;
		default_AecDelayCfg.para_delay_sample = 1;

		printf("\n");
		printf("Enter para_aec_filter_len[1~13]:\t");
		scanf("%d", &input_arg);
		printf("\n");
		default_AEC_Setting.para_aec_filter_len = input_arg;

		printf("Enter para_aes_std_thrd[0~39]:\t");
		scanf("%d", &input_arg);
		printf("\n");
		default_AEC_Setting.para_aes_std_thrd = input_arg;

		printf("Enter para_aes_supp_coeff[0~100]:\t");
		scanf("%d", &input_arg);
		printf("\n");
		default_AEC_Setting.para_aes_supp_coeff = input_arg;
	}

	pstAiVqeAttr.stAgcCfg = default_AGC_Setting;
	pstAiVqeAttr.stAnrCfg = default_ANR_Setting;
	pstAiVqeAttr.stAecCfg = default_AEC_Setting;
	pstAiVqeAttr.stAecDelayCfg = default_AecDelayCfg;
	if (use_default_parameters == 0) //else trigger by QA setting
		pstAiVqeAttr.u32OpenMask = LP_AEC_ENABLE|NLP_AES_ENABLE|NR_ENABLE|AGC_ENABLE|DCREMOVER_ENABLE;



	pstAiVqeAttr.s32WorkSampleRate = sr;
	pstAiVqeAttr.para_notch_freq = 0;
	pstAiVqeAttr.s32RevMask = 0;
	strcpy(pstAiVqeAttr.customize, "none");
	if (use_default_parameters > SAMPLE_AUDIO_AEC_USER_PARAM) {
		printf("Dump the parameters:-------------------------------\n");
		printf("para_fun_config:%d\n", pstAiVqeAttr.u32OpenMask);
		printf("para_delay_sample:%d\n", default_AecDelayCfg.para_delay_sample);
		printf("para_dg_target:%d\n", default_AecDelayCfg.para_dg_target);
		printf("para_aec_init_filter_len:%d\n", default_AecDelayCfg.para_aec_init_filter_len);
		printf("para_aec_filter_len:%d\n", default_AEC_Setting.para_aec_filter_len);
		printf("para_aes_std_thrd:%d\n", default_AEC_Setting.para_aes_std_thrd);
		printf("para_aes_supp_coeff:%d\n", default_AEC_Setting.para_aes_supp_coeff);
		printf("para_nr_init_sile_time:%d\n", default_ANR_Setting.para_nr_init_sile_time);
		printf("para_nr_snr_coeff:%d\n", default_ANR_Setting.para_nr_snr_coeff);
		printf("para_agc_max_gain:%d\n", default_AGC_Setting.para_agc_max_gain);
		printf("para_agc_target_high:%d\n", default_AGC_Setting.para_agc_target_high);
		printf("para_agc_target_low:%d\n", default_AGC_Setting.para_agc_target_low);
		printf("para_agc_vad_ena:%d\n", default_AGC_Setting.para_agc_vad_ena);
		printf("para_notch_freq:%d\n", pstAiVqeAttr.para_notch_freq);
		printf("para_spk_fun_config:0\n");
		printf("Dump the parameters:-------------------------------\n");
	}
printf("[v][%s][%d]\n", __func__, __LINE__);
	ret = CVI_AI_SetTalkVqeAttr(
			 0,
			 0,
			 0,
			 0,
			 &pstAiVqeAttr);
printf("[v][%s][%d]\n", __func__, __LINE__);
	if (ret != CVI_SUCCESS)
		printf("CVI_AI_SetTalkVqeAttr ret no success\n");
printf("[v][%s][%d]\n", __func__, __LINE__);
	CVI_AI_EnableVqe(0, 0);

printf("[v][%s][%d]\n", __func__, __LINE__);
	fp_in_wav  = fopen(argv[1], "rb+");
	if (!fp_in_wav)
		goto EXIT;
	ret = fseek(fp_in_wav, 44, SEEK_SET); //pcm data no need to this
	pout_wav_name = (char *)argv[2];
	s32MacSize = AEC_INPUT_CHANNEL * DEFAULT_BYTES_PER_SAMPLE * AEC_INPUT_PERIOD_SIZE;

	pcm_frame = (CVI_S16 *)malloc(s32MacSize);
	if (pcm_frame ==  NULL) {
		printf("%s() %d: malloc failed\n", __func__, __LINE__);
		goto EXIT;
	}

	s32MacSize = AEC_INPUT_PERIOD_SIZE * sizeof(CVI_S16) * 2; //one channel output
	ptmp_buf  = (CVI_S16 *)malloc(s32MacSize);
	if (ptmp_buf ==  NULL) {
		printf("%s() %d: malloc failed\n", __func__, __LINE__);
		goto EXIT;
	}
	fp_out_wav = fopen(pout_wav_name, "wb+");
	if (!fp_out_wav) {
		printf("Could not open %s for writing!\n", pout_wav_name);
		goto EXIT;
	}

	/*  time calc start*/
	gettimeofday(&tv1, &tz);
	s32InputBytes = DEFAULT_BYTES_PER_SAMPLE * AEC_INPUT_CHANNEL * AEC_INPUT_PERIOD_SIZE;

	while (1) {
		read_size = fread(pcm_frame,
				DEFAULT_BYTES_PER_SAMPLE * AEC_INPUT_CHANNEL,
				AEC_INPUT_PERIOD_SIZE, fp_in_wav);
		if (read_size != AEC_INPUT_PERIOD_SIZE) {
			printf("leaving read file!!!\n");
			break;
		}

		CVI_AUDIO_LoadingCheck_Begin(ploadhandle);

		if ((use_default_parameters == SAMPLE_AUDIO_MIPS_NR) ||
			(use_default_parameters == SAMPLE_AUDIO_MIPS_AGC)) {

			CVI_AudIn_AlgoProcess_AnrAgc((CVI_CHAR *)pcm_frame,
					      (CVI_CHAR *)ptmp_buf,
					      s32InputBytes,
					      &retLen);
		} else {

			ret = CVI_AudIn_AlgoProcess_AEC(
				(CVI_CHAR *)pcm_frame,
				(CVI_CHAR *)ptmp_buf,
				s32InputBytes,
				&retLen);
		}

		CVI_AUDIO_LoadingCheck_End(ploadhandle, s32InputBytes);
		if (retLen == 0)
			printf("read_size=%d, ret=%d, retLen=%d, samples=%d\n", read_size, ret, retLen, read_size);

		if (retLen)
			fwrite(ptmp_buf, 1, retLen, fp_out_wav);

	}

	gettimeofday(&tv2, &tz);

	/* elapse times */
	duration = (CVI_FLOAT)(tv2.tv_sec - tv1.tv_sec) +  (CVI_FLOAT)(tv2.tv_usec - tv1.tv_usec)/1000000;
	printf("Duration: %f s\n", duration);

EXIT:

	if (pcm_frame)
		free(pcm_frame);

	if (ptmp_buf)
		free(ptmp_buf);

	if (fp_in_wav)
		fclose(fp_in_wav);

	if (fp_out_wav)
		fclose(fp_out_wav);
	CVI_AUDIO_LoadingCheck_DeInit(ploadhandle);
	CVI_AI_DisableVqe(0, 0);
	//CVI_AudIn_AlgoFreeBuffer();
	CVI_AudIn_AlgoDeInit();
	return 0;
}

