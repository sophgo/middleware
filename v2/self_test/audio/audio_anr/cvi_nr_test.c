/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: sample/cvi_nr_test.c
 * Description:
 * This example C main file shows how to call entry-point functions and read input signals.
 * You must customize this file for your development environment/platform
 * Modify it and integrate it into your own development environment/platform.
 */

/* Include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "cvi_type.h"
#include "cvi_defines.h"
#include "cvi_comm_aio.h"
#include "cvi_audio.h"
#include "cvi_audio_vqe.h"
#ifdef ENABLE_FUNCTION_SYSTRACE
#include "function_tracer.h"
#else
#define BITMAIN_FUNCTION_TRACE(x)
#endif

CVI_S32 dbg_level = 2;
#define ERR_PRINTF(fmt, args...) \
	do { \
		if (dbg_level > 0) \
			fprintf(stderr, "[cviaudio][error][%s][%d]" fmt, __func__, __LINE__, ##args);\
	} while (0)

#define DBG_PRINTF(fmt, args...) \
	do { \
		if (dbg_level > 1) \
			fprintf(stderr, "[cviaudio][info]" fmt, ##args);\
	} while (0)


FILE *fp_test_input;
FILE *fp_test_output;


unsigned long long get_current_time(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
unsigned long long checktime = 1;


/* TODO: This static function should be moving as a audio utility lib */
static CVI_BOOL _cvi_checkname_iswav(char *infilename)
{
	CVI_S32 s32InputFileLen = 0;
	CVI_S32 s32Read = 0;

	ST_CVI_WAV_HEADER wavHead;
	FILE *ptr;
	CVI_U8 buffer4[4];

	s32InputFileLen = strlen(infilename);

	if (s32InputFileLen == 0) {
		printf("No Input File Name..force return\n");
		return 0;
	}


	/* Step1: Identify the file by the file extension */
	if (infilename[s32InputFileLen - 4] == '.' &&
	    (infilename[s32InputFileLen - 3] == 'W' || infilename[s32InputFileLen - 3] == 'w') &&
	    (infilename[s32InputFileLen - 2] == 'A' || infilename[s32InputFileLen - 2] == 'a') &&
	    (infilename[s32InputFileLen - 1] == 'V' || infilename[s32InputFileLen - 1] == 'v')) {
		printf("Enter wav file\n");
		return CVI_TRUE;
		/* Only judge by the extension */
	} else {
		/* Do Not return directly, do the header parsing */
		/* return CVI_FALSE; */
		DBG_PRINTF("Do not see the file extension name\n");
	}
	/* Step2: Check the file by the file header */
	ptr = fopen(infilename, "rb");
	if (ptr == NULL) {
		ERR_PRINTF("\n");
		return CVI_FALSE;
	}

	s32Read = fread(wavHead.riff, sizeof(wavHead.riff), 1, ptr);
	/*s32Read is only for return value, we do not actually use it */
	s32Read = fread(buffer4, sizeof(buffer4), 1, ptr);
	/*s32Read is only for return value, we do not actually use it */
	s32Read = fread(wavHead.wave, sizeof(wavHead.wave), 1, ptr);
	printf("(9-12) Wave marker: %.*s\n", 4, wavHead.wave);
	s32Read = strncmp((const char *)wavHead.wave, "WAVE", 4);
	if (s32Read == 0)
		return CVI_TRUE;
	else
		return CVI_FALSE;


}

static CVI_S32 _vqe_user_option(AI_TALKVQE_CONFIG_S *pstAiVqeAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_value;

	if ((pstAiVqeAttr->u32OpenMask & AGC_ENABLE) == AGC_ENABLE) {
		printf("Enter agc_max_gain [0, 3]\n");
		scanf("%d", &input_value);
		pstAiVqeAttr->stAgcCfg.para_agc_max_gain = input_value;
		printf("Enter agc_target_high [0, 36]\n");
		scanf("%d", &input_value);
		pstAiVqeAttr->stAgcCfg.para_agc_target_high = input_value;
		printf("Enter agc_target_low [0, 36]\n");
		scanf("%d", &input_value);
		pstAiVqeAttr->stAgcCfg.para_agc_target_low = input_value;
		printf("Enter agc_vad_enable [0, 1]\n");
		scanf("%d", &input_value);

		pstAiVqeAttr->stAgcCfg.para_agc_vad_ena = input_value;
		printf("\n");
		printf("AGC param: [%d, %d, %d, %d]\n",
		       pstAiVqeAttr->stAgcCfg.para_agc_max_gain,
		       pstAiVqeAttr->stAgcCfg.para_agc_target_high,
		       pstAiVqeAttr->stAgcCfg.para_agc_target_low,
		       pstAiVqeAttr->stAgcCfg.para_agc_vad_ena);



	}

	if ((pstAiVqeAttr->u32OpenMask & NR_ENABLE) == NR_ENABLE) {
		printf("Enter nr_snr_coeff [0, 20]\n");
		scanf("%d", &input_value);
		pstAiVqeAttr->stAnrCfg.para_nr_snr_coeff = input_value;

		printf("Enter para_nr_init_sile_time [0, 1]\n");
		scanf("%d", &input_value);
		pstAiVqeAttr->stAnrCfg.para_nr_init_sile_time = input_value;
		printf("\n");
		printf("ANR param: [%d] sile_time[%d]\n",
		       pstAiVqeAttr->stAnrCfg.para_nr_snr_coeff,
		       pstAiVqeAttr->stAnrCfg.para_nr_init_sile_time);

	}

	return s32Ret;
}


static void _using_default_anr_parameters(AI_TALKVQE_CONFIG_S *pAiVqeAttr, int sample_rate)
{
	printf("update default agc/anr parameters!!!\n");
	pAiVqeAttr->stAgcCfg.para_agc_max_gain = 0;
	pAiVqeAttr->stAgcCfg.para_agc_target_high = 2;
	pAiVqeAttr->stAgcCfg.para_agc_target_low = 72;
	pAiVqeAttr->stAgcCfg.para_agc_vad_ena = 1;
	pAiVqeAttr->para_notch_freq = 0;
	pAiVqeAttr->stAnrCfg.para_nr_snr_coeff = 15;
	pAiVqeAttr->stAnrCfg.para_nr_init_sile_time = 0;
	pAiVqeAttr->u32OpenMask = NR_ENABLE|AGC_ENABLE|DCREMOVER_ENABLE;
	pAiVqeAttr->s32WorkSampleRate = sample_rate;
	strcpy(pAiVqeAttr->customize, "none");

}
CVI_S32 main(CVI_S32 argc, const char *const argv[])

{
	/* All the function flow are wrapped to speech_algo_xxx function api */
	/* The reference flow is based on main_speech_algorithm1126 */
	char s[100];
	CVI_S32 s32Ret = CVI_FAILURE;
	CVI_BOOL bIsWav = CVI_FALSE;
	CVI_S32 s32BytesOut = 0;
	CVI_U8 *GetFrameBuff = (CVI_U8 *) malloc(6400);
	CVI_S32 iAgcOn = 0;
	CVI_S32 iNrOn = 0;
	char *input_filename = (char *)argv[1];
	char output_filename[128] = {0};
	CVI_CHAR *pWrite = NULL;
	CVI_CHAR *audio_buffer = NULL;
	CVI_S32 u32UsrFrmDepth = 1;//10;
	short wav_header[44];
	CVI_S32 count = 0;
	CVI_S32 hopsize = 160;
	AI_TALKVQE_CONFIG_S pstAiVqeAttr;
	CVI_BOOL  bWithNotchFilter = CVI_FALSE;
	int use_default_parameters = 0;
	int sample_rate = 0;
	CVI_S32 s32FramePerSample = 160;

	(void)argc;
	(void)argv;
	BITMAIN_FUNCTION_TRACE(__PRETTY_FUNCTION__);

	memset(&pstAiVqeAttr, 0, sizeof(AI_TALKVQE_CONFIG_S));
	/* Step 1 do the file handle */
	if (argc  < 2) {
		printf("================================================\n");
		printf("cvi_nr_test usage :\n");
		printf("./sample_audio_nr  [agr1]\n");
		printf("arg1: $(filename).wav or $(filename).raw\n");
		printf("Test Result : create two files in different format: after_$(filename).pcm ");
		printf("after_$(filename).wav\n");
		printf("=================================================\n");
		free(GetFrameBuff);
		return CVI_FAILURE;
	}

	if ((argc == 4) && (strcmp(argv[3], "-default") == 0)) {
		printf("sample_audio_nr use default parameters!!!\n");
		use_default_parameters = 1;
		sample_rate = atoi(argv[2]);
		iNrOn = 1;
		iAgcOn = 1;
		goto DEFAULT_ANR_PATH;
	}

	bIsWav = _cvi_checkname_iswav(input_filename);

	if (bIsWav == CVI_TRUE)
		printf("----->wav format\n");
	else
		printf("----->not wav format\n");

	do {
		printf("-----------------------------\n");
		printf("Enter NR off:0 , On:1 : ");
		fgets(s, 10, stdin);
		iNrOn = atoi(s);

	} while (0);
	printf("\n");

	do {
		printf("-----------------------------\n");
		printf("Enter AGC off:0 , On:1 : ");
		fgets(s, 10, stdin);
		iAgcOn = atoi(s);
	} while (0);
	printf("\n");
	/* Step 1-1 rename the output */
DEFAULT_ANR_PATH:
	if (iNrOn == 1) {
		pstAiVqeAttr.u32OpenMask |= (NR_ENABLE);
		if (iAgcOn == 1) {
			pstAiVqeAttr.u32OpenMask |= (AGC_ENABLE|DCREMOVER_ENABLE);
			snprintf(output_filename, 128, "NR_AGC_%s.pcm", input_filename);
		} else {
			pstAiVqeAttr.u32OpenMask &= (~(AGC_ENABLE|DCREMOVER_ENABLE));
			snprintf(output_filename, 128, "NR_%s.pcm", input_filename);
		}
	} else {
		pstAiVqeAttr.u32OpenMask &= (~(NR_ENABLE));
		if (iAgcOn == 1) {
			pstAiVqeAttr.u32OpenMask |= (AGC_ENABLE|DCREMOVER_ENABLE);
			snprintf(output_filename, 128, "AGC_%s.pcm", input_filename);
		} else {
			pstAiVqeAttr.u32OpenMask &= (~(AGC_ENABLE|DCREMOVER_ENABLE));
			snprintf(output_filename, 128, "ByPass_%s.pcm", input_filename);
		}
	}
#if 0
	do {
		printf("-----------------------------\n");
		printf("Customize Notch Filter Off:0 , On:1 : ");
		fgets(s, 10, stdin);
		bWithNotchFilter = (CVI_BOOL)atoi(s);
	} while (0);
	printf("\n");
#endif

	fp_test_input = fopen(input_filename, "rb");
	fp_test_output = fopen(output_filename, "wb");
	if (((fp_test_output) == NULL) || ((fp_test_input) == NULL)) {
		printf("Cannot open input / output file\n");
		goto ERROR_HANDEL1;
	}


	//step 1 presetting
	/* Step 2 do the  buffer condition */
	audio_buffer = (CVI_CHAR *)malloc(50 * 160 * 1024);
	if (audio_buffer != NULL)
		pWrite = audio_buffer;
	else
		printf("fatal error while allocate buffer[%s][%d]\n", __func__, __LINE__);


	/* Step 2 assign config for vqe attribute */
	AI_AEC_CONFIG_S default_AEC_Setting;
	AUDIO_AGC_CONFIG_S default_AGC_Setting;
	AUDIO_ANR_CONFIG_S  default_ANR_Setting;
	AUDIO_DELAY_CONFIG_S default_AecDelayCfg;
	memset(&default_AGC_Setting, 0, sizeof(AUDIO_AGC_CONFIG_S));
	memset(&default_ANR_Setting, 0, sizeof(AUDIO_ANR_CONFIG_S));
	memset(&default_AecDelayCfg, 0, sizeof(AUDIO_DELAY_CONFIG_S));


	default_AGC_Setting.para_agc_max_gain = 0;
	default_AGC_Setting.para_agc_target_high = 2;
	default_AGC_Setting.para_agc_target_low = 72;
	default_AGC_Setting.para_agc_vad_ena = 1;
	default_ANR_Setting.para_nr_snr_coeff = 15;
	pstAiVqeAttr.stAnrCfg.para_nr_init_sile_time = 0;
	pstAiVqeAttr.stAgcCfg = default_AGC_Setting;
	pstAiVqeAttr.stAnrCfg = default_ANR_Setting;
	pstAiVqeAttr.stAecCfg = default_AEC_Setting;
	pstAiVqeAttr.stAecDelayCfg = default_AecDelayCfg;
	if (use_default_parameters == 1) {
		_using_default_anr_parameters(&pstAiVqeAttr, sample_rate);
		hopsize = s32FramePerSample;

	} else {
	/* Step3: User option for NR AGC */
	s32Ret = _vqe_user_option(&pstAiVqeAttr);
	printf("pstAiVqeAttr.u32OpenMask[0x%x]\n", pstAiVqeAttr.u32OpenMask);


	printf("Enter frame size (samples)160 multiply\n");
	scanf("%d", &s32FramePerSample);
	printf("frame size[%d]\n", s32FramePerSample);
	pstAiVqeAttr.s32WorkSampleRate = 16000;

	if (bWithNotchFilter == CVI_TRUE) {
		printf("With customize Notch Filter\n");
		pstAiVqeAttr.stAgcCfg.para_agc_max_gain = 0;
		pstAiVqeAttr.stAgcCfg.para_agc_target_high = 2;
		pstAiVqeAttr.stAgcCfg.para_agc_target_low = 6;
		pstAiVqeAttr.stAgcCfg.para_agc_vad_ena = 1;
		pstAiVqeAttr.stAnrCfg.para_nr_snr_coeff = 10;
		pstAiVqeAttr.para_notch_freq = 1;
		pstAiVqeAttr.stAnrCfg.para_nr_init_sile_time = 0;

	}
	hopsize = s32FramePerSample;
	if (s32Ret == CVI_FAILURE) {
		ERR_PRINTF("Err\n");
		return CVI_FAILURE;
	}

	if (bIsWav == CVI_TRUE) {
		fread(&wav_header[0], 1, 44, fp_test_input);/* wav header  */
		pstAiVqeAttr.s32WorkSampleRate = (CVI_S32)wav_header[12];
	} else {
		int rate;

		printf("Enter sample rate (8000/16000):\n");
		scanf("%d", &rate);
		printf("\n");
		pstAiVqeAttr.s32WorkSampleRate  = rate;
	}
	printf("sample rate[%d]\n", pstAiVqeAttr.s32WorkSampleRate);
	}
	/* step4 init algo function */
	s32Ret = CVI_AI_SetTalkVqeAttr(
			 0,
			 0,
			 0,
			 0,
			 &pstAiVqeAttr);

	if (s32Ret != CVI_SUCCESS) {
		ERR_PRINTF("\n");
		free(audio_buffer);
		return CVI_FAILURE;
	}

	CVI_AI_EnableVqe(0, 0);

	/* step5 do the vqe process */

	//step4 do the NR algo
	for (;;) {	/* Main Frame Loop */
		/* This section of codes have to be replaced by reading system layer */
		/* bitstream in real platform */
		/* copy unit in memcpy is byte */
		if (fread(pWrite, sizeof(short), hopsize * u32UsrFrmDepth,
			  fp_test_input) != (size_t)(hopsize * u32UsrFrmDepth)) {
			/* get current frame data */
			goto Pattern_EOF;
		} else {

			checktime = get_current_time();

			CVI_AudIn_AlgoProcess_AnrAgc(pWrite,
					      (CVI_CHAR *)GetFrameBuff,
					      sizeof(short) * hopsize * u32UsrFrmDepth,
					      &s32BytesOut);
			//printf("s32BytesOut[%d][%d]\n", (int)(sizeof(short) * hopsize * u32UsrFrmDepth), s32BytesOut);
			if ((s32BytesOut != 0) &&
			    (s32BytesOut == (CVI_S32)(sizeof(short) * hopsize * u32UsrFrmDepth))) {
				/* Write to output file */
				fwrite(GetFrameBuff, 1, s32BytesOut, fp_test_output);
			} else {
				//ERR_PRINTF("fatal while copy pWrite_VQE (%d = %d * %d *%d)\n",
				//	s32BytesOut, sizeof(short), hopsize, u32UsrFrmDepth);
				free(audio_buffer);
				free(GetFrameBuff);
				return CVI_FAILURE;
			}

			printf("proc_sample per frame:[%d] time_spent:[%lld]ms\n",
			       (hopsize * u32UsrFrmDepth), get_current_time() - checktime);

			count++;

		}

	}
	CVI_AI_DisableVqe(0, 0);
	//CVI_AudIn_AlgoFreeBuffer();
	CVI_AudIn_AlgoDeInit();
	free(audio_buffer);
	return 0;
Pattern_EOF:
	fclose(fp_test_output);
	printf("Pattern_EOF[%d]\n", count);
	CVI_AI_DisableVqe(0, 0);
	//CVI_AudIn_AlgoFreeBuffer();
	CVI_AudIn_AlgoDeInit();
	free(audio_buffer);
	return 0;


ERROR_HANDEL1:
	fclose(fp_test_input);
	fclose(fp_test_output);

}




