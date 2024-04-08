#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "cvi_resampler_api.h"

typedef struct RESAMPLE_INFO_T {
	int inSampleRate;
	int outSampleRate;
	double stepDist;
	uint64_t fixedFraction;
	double normFixed;
	uint64_t step;
	int16_t *last_input;
	int16_t *output_buf;
	uint64_t curOffset;
	uint32_t inputsamples;
	uint32_t prev_inputsamples; //user may not always send the same size
	uint32_t channels;
	uint32_t output_index;
	int last_delta;
} RESAMPLE_INFO;


#define _RESAMPLE_VERSION_TAG_ "cvi_resample_ver3_fromInfi_20210629"

CVI_VOID *CVI_Resampler_Create(CVI_S32 s32Inrate, CVI_S32 s32Outrate,
			       CVI_S32 s32Chans)
{
	printf("_RESAMPLE_VERSION_TAG[%s]\n", _RESAMPLE_VERSION_TAG_);
	RESAMPLE_INFO *gstResInfo_ptr = (RESAMPLE_INFO *)malloc(sizeof(RESAMPLE_INFO));

	if (!gstResInfo_ptr) {
		printf("<%s,%d> failed,no mem\n", __func__, __LINE__);
		return NULL;
	}

	memset(gstResInfo_ptr, 0, sizeof(RESAMPLE_INFO));
	gstResInfo_ptr->channels = s32Chans;
	gstResInfo_ptr->inSampleRate = s32Inrate;
	gstResInfo_ptr->outSampleRate = s32Outrate;
	gstResInfo_ptr->stepDist = ((double) s32Inrate / (double) s32Outrate);
	gstResInfo_ptr->fixedFraction = (1LL << 32);
	gstResInfo_ptr->normFixed = (1.0 / (1LL << 32));
	gstResInfo_ptr->step = ((uint64_t)(gstResInfo_ptr->stepDist *
					   gstResInfo_ptr->fixedFraction + 0.5));
	gstResInfo_ptr->curOffset = 0;
	gstResInfo_ptr->inputsamples = 0;
	gstResInfo_ptr->prev_inputsamples = 0; //user may not always send the same size

	return (void *)gstResInfo_ptr;
}


/**************************************************************************************
 * Function:    CVI_Resampler_Process
 *
 * Description: Resample pcm data to specific samplerate, only for interlaced format
 *
 * Inputs:      inst: valid Resampler instance pointer (HResampler)
 *              inbuf:   pointer to inputbuf
 *              insamps: input number of sample pointers
 * Outputs:     outbuf:  pointer to outputbuf
 *
 * Return:      output sample number per-channel
 * Notes:       sure insamps < MAXFRAMESIZE


 **************************************************************************************/

CVI_S32 CVI_Resampler_Process(CVI_VOID *inst, CVI_S16 *s16Inbuf,
			      CVI_S32 s32Insamps, CVI_S16 *s16Outbuf)
{
	int i;
	uint32_t c;
	int16_t *input_tmp;
	int outputSize;
	int16_t *output_buf_ptr;
	char *output_move;
	int j_cnt = 0;
	int loop_max_cnt = 0;
	//int output_framesize = 0;
	char *ptr_input;
	RESAMPLE_INFO *gstResInfo_ptr = (RESAMPLE_INFO *)inst;

	if (!inst) {
		printf("<%s,%d> params is NULL.\n", __func__, __LINE__);
		return 0;
	}

	if (s16Inbuf == NULL)
		return -1;
	if (s16Outbuf == NULL)
		return 0;

	outputSize = s32Insamps * gstResInfo_ptr->outSampleRate /
		     gstResInfo_ptr->inSampleRate;

	if (gstResInfo_ptr == NULL) {
		printf("CvResampleProcess resampler not init ok.\n");
		return -2;
	}


	if (!gstResInfo_ptr->last_input) {
		gstResInfo_ptr->last_input = (int16_t *)malloc((1024 * 4 * 3) *
					     gstResInfo_ptr->channels * sizeof(short));

		if (!gstResInfo_ptr->last_input) {
			printf("ERROR, Resampler Process malloc  last_input failed.\n");
			return -3;
		}

		CVI_S32 s32WorstOutSize = (1024 * 4 * 3) * gstResInfo_ptr->outSampleRate /
			gstResInfo_ptr->inSampleRate;

		gstResInfo_ptr->output_buf = (int16_t *)malloc(s32WorstOutSize *
					     gstResInfo_ptr->channels * sizeof(short) * 2);

		if (!gstResInfo_ptr->output_buf) {
			printf("ERROR, Resampler Process malloc output_buf failed.\n");
			return -4;
		}

		gstResInfo_ptr->prev_inputsamples = s32Insamps;
		//gstResInfo_ptr->output_index = 0;
		memcpy(gstResInfo_ptr->last_input, s16Inbuf,
		       s32Insamps * gstResInfo_ptr->channels * sizeof(short));
		gstResInfo_ptr->prev_inputsamples = s32Insamps;
		return 0;
	}

	loop_max_cnt = outputSize > s32Insamps ? s32Insamps : outputSize;
	//output_framesize = gstResInfo_ptr->output_index;
	//ptr_input = (char *) gstResInfo_ptr->last_input;
	//output_buf_ptr = &gstResInfo_ptr->output_buf[gstResInfo_ptr->output_index * gstResInfo_ptr->channels];
	output_buf_ptr = gstResInfo_ptr->output_buf;
	memcpy(&gstResInfo_ptr->last_input[gstResInfo_ptr->prev_inputsamples * gstResInfo_ptr->channels],
		s16Inbuf, gstResInfo_ptr->channels * sizeof(short)); //copy the first new frame

	input_tmp = gstResInfo_ptr->last_input;

	double dCoefValue = (double)(gstResInfo_ptr->curOffset >> 32) +
			    ((gstResInfo_ptr->curOffset & (gstResInfo_ptr->fixedFraction - 1)) *
			     gstResInfo_ptr->normFixed);

	while (j_cnt < loop_max_cnt) {
		for (c = 0; c < gstResInfo_ptr->channels; c += 1) {
			*output_buf_ptr++ = (int16_t)
					    (input_tmp[c] + (input_tmp[c + gstResInfo_ptr->channels] - input_tmp[c]) *
					     dCoefValue);
		}
		//output_framesize++;
		gstResInfo_ptr->curOffset += gstResInfo_ptr->step;
		gstResInfo_ptr->last_delta = (gstResInfo_ptr->curOffset >> 32) *
					     gstResInfo_ptr->channels;
		input_tmp += gstResInfo_ptr->last_delta;
		if (gstResInfo_ptr->last_delta)
			j_cnt++;

		gstResInfo_ptr->curOffset &= (gstResInfo_ptr->fixedFraction - 1);
		dCoefValue = (double)(gstResInfo_ptr->curOffset >> 32) +
			     ((gstResInfo_ptr->curOffset & (gstResInfo_ptr->fixedFraction - 1)) *
			      gstResInfo_ptr->normFixed);
	}
	//printf("output_framesize :%d\n",output_framesize);

	memcpy(s16Outbuf, gstResInfo_ptr->output_buf,
		       outputSize * gstResInfo_ptr->channels * sizeof(short));

	memcpy(gstResInfo_ptr->last_input, s16Inbuf,
	       s32Insamps * gstResInfo_ptr->channels * sizeof(short));
	gstResInfo_ptr->prev_inputsamples = s32Insamps;
	return outputSize;
}


/**************************************************************************************
 * Function:    CVI_Resampler_Destroy
 *
 * Description: free platform-specific data allocated by ResamplerCreate
 *
 * Inputs:      valid Resampler instance pointer (HResampler)
 * Outputs:     none
 *
 * Return:      none
 **************************************************************************************/
CVI_VOID CVI_Resampler_Destroy(CVI_VOID *inst)
{
	RESAMPLE_INFO *gstResInfo_ptr = (RESAMPLE_INFO *)inst;

	if (!inst) {
		printf("<%s,%d> params is NULL.\n", __func__, __LINE__);
		return;
	}

	if (gstResInfo_ptr && gstResInfo_ptr->last_input) {
		free(gstResInfo_ptr->last_input);
		gstResInfo_ptr->last_input = 0;
	}


	if (gstResInfo_ptr && gstResInfo_ptr->output_buf) {
		free(gstResInfo_ptr->output_buf);
	}


	if (gstResInfo_ptr)
		free(gstResInfo_ptr);
	gstResInfo_ptr = NULL;

}


/*******************************************************************************
 * Function:	CVI_Resampler_GetMaxOutputNum
 *
 * Description: Calculate max output number at specific input number
 *
 * Inputs:		inst:	  valid Resampler instance pointer (CVI_HANDLE)
 *				insamps:  input data number per-channel, insamps must be even
 * Outputs: none
 * Return:		>=0:	  Success, return the max output number per-channel
 *				other:	  Fail, return error code
 * Notes:
 * 1  if stereo(chans==2), sure insamps%2 == 0
 ******************************************************************************/
CVI_S32 CVI_Resampler_GetMaxOutputNum(CVI_VOID *inst, CVI_S32 s32Insamps)
{
	RESAMPLE_INFO *gstResInfo_ptr = (RESAMPLE_INFO *)inst;

	if (!inst) {
		printf("<%s,%d> params is NULL.\n", __func__, __LINE__);
		return 0;
	}

	return s32Insamps * gstResInfo_ptr->outSampleRate /
	       gstResInfo_ptr->inSampleRate;
}

