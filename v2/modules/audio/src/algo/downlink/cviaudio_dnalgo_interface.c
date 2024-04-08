/*usage from C std lib */
#include <stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
/* usage from cviaudio header */
#include "cvi_comm_aio.h"
#include "cvi_dn_interface.h"
#define  _AUDIO_DNALGO_INTERFACE_VERSION_TAG_ "Aud_DN_Interface_01"
#define _AUDIO_DNALGO_INTERNAL_VERSION_ "CVITEK_DN_Algo_01"
//turn on all the mask to user config, do not wrap or simply to 3 function HPF/EQ/DRC
#define CVIAUDIO_ALGO_FUNCTION_HPF 0x01
#define CVIAUDIO_ALGO_FUNCTION_EQ 0x02
#define CVIAUDIO_ALGO_FUNCTION_DRC  0x04

//TODO: step 1 add include header for specific algo
/* porting layer for specific algorithm */

#define DN_FRAME_LENGTH (160)
#ifndef CHECK_NULL_PTR
#define CHECK_NULL_PTR(ptr) \
	do { \
		if (!(ptr)) { \
			printf("func:%s,line:%d, NULL pointer\n", __func__, __LINE__); \
		} \
	} while (0)
#endif

static int notch_dbglevel = 2;

#define CVIAUD_NOTCH_ERR_PRINTF(fmt, args...) \
	do { \
		if (notch_dbglevel > 0) \
			fprintf(stderr, "[cviaudio][error][%s][%d] "fmt, __func__, __LINE__, ##args);\
	} while (0)

#define CVIAUD_NOTCH_DBG_PRINTF(fmt, args...) \
	do { \
		if (notch_dbglevel > 1) \
			fprintf(stderr, "[cviaudio][info] "fmt, ##args);\
	} while (0)

#define CVIAUD_NOTCH_TRA_PRINTF(fmt, args...) \
	do { \
		if (notch_dbglevel > 2) \
			fprintf(stderr, "[cvitrace][%s][%d] "fmt, __func__, __LINE__, ##args);\
	} while (0)

#ifndef SSP_UNUSED_REF
#define SSP_UNUSED_REF(X)  ((X) = (X))
#endif

#ifndef DEFAULT_BYTES_PER_SAMPLE
#define DEFAULT_BYTES_PER_SAMPLE 2
#endif

#ifndef CVI_DN_FRAMES_LEN
#define CVI_DN_FRAMES_LEN 160
#endif
//#define IBUFFSIZE 160/* Input buffer size */
#define MAX_IBUFFSIZE 4096
/* sample code from ffmpeg resampling_audio.c -----------end */

void *CviAud_DnAlgo_Init(int s32FunctMask, void *param_info)
{
	int s32Ret;
	void *_handle = NULL;

	const AO_VQE_CONFIG_S *pstVqeConfig = (const AO_VQE_CONFIG_S *)param_info;

    printf("openMask:0x%x\n", s32FunctMask);
	_handle = (void *)cvi_audio_pp_init((void *)pstVqeConfig);
	return (void *)_handle;
}

int CviAud_DnAlgo_Process(void *pHandle,  short *spk_in,
			short *spk_out, int iLength)
{
	SSP_UNUSED_REF(iLength);
	int ret = 0;
	int s32RetTotalSamples = 0;

	if (pHandle == NULL) {
		printf("Null input [%s][%d]\n", __func__, __LINE__);
		return -1;
	}
	if (iLength != DN_FRAME_LENGTH) {
		printf("input length only support 160 samples[%s][%d]\n", __func__, __LINE__);
		return -1;
	}

	ret = cvi_audio_pp_process(pHandle, spk_in, spk_out, DN_FRAME_LENGTH);
	if (ret != 0) {
		printf("cvi_audio_pp_process error ret(%d).\n", ret);
		return -1;
	}
	s32RetTotalSamples = DN_FRAME_LENGTH;
	return s32RetTotalSamples;

}

void CviAud_DnAlgo_DeInit(void *pHandle)
{
	cvi_audio_pp_deinit(pHandle);
}

/**************************************************************************************
 * Function:    CviAud_Algo_GetVersion
 *
 * Description: Get version info: algorithrm source , date  through this api
 *
 * Inputs:      None
 * Outputs:	None
 *
 * Return:     Version info in string type
 **************************************************************************************/
void  CviAud_DnAlgo_GetVersion(char *pstrVersion)
{

	CVIAUD_NOTCH_DBG_PRINTF("CviAud Dnalgo interface[%s]\n", _AUDIO_DNALGO_INTERFACE_VERSION_TAG_);
	CVIAUD_NOTCH_DBG_PRINTF("CviAud  Dnalgo lib[%s]\n", _AUDIO_DNALGO_INTERNAL_VERSION_);
	sprintf(pstrVersion, _AUDIO_DNALGO_INTERNAL_VERSION_);
}
