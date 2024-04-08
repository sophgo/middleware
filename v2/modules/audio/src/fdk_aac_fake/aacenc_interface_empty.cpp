/*
 * Copyright ., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: aacenc_interface.c
 * Description: audio transcode function interface as example
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include "cvi_type.h"
#include "cvi_aacenc.h"


#define MAX_CHANNELS 2
#define AACENC_BLOCKSIZE 1024
#define _VERSION_TAG_  "audio_aac_enc_empty_release_add_len"


#define AACLC_ENC_AOT 2
#define HEAAC_ENC_AOT 5
#define HEAAC_PLUS_ENC_AOT 29
#define AACLD_ENC_AOT 23
#define AACELD_ENC_AOT 39



/**
 *brief Get version information.
 *attention \n
 *N/A
 *param[in] pVersion       version describe struct
 *retval ::CVI_SUCCESS   : Success
 *retval ::CVI_FAILURE          : FAILURE
 *see \n
 *N/A
 */
CVI_S32  CVI_AACENC_GetVersion(AACENC_VERSION_S *pVersion)
{
	printf("[%s][%s]\n", __func__, _VERSION_TAG_);
	strcpy((char *)pVersion->aVersion, (const char *)_VERSION_TAG_);
	return 0;
}

/**
 *brief get reasonable default configuration.
 *attention \n
 *N/A
 *param[in] pstConfig    pointer to an configuration information structure
 *retval ::CVI_SUCCESS   : Success
 *retval ::CVI_FAILURE          : FAILURE
 *see \n
 *N/A
 */
int  AACInitDefaultConfig(AACENC_CONFIG *pstConfig)
{
	//Empty function for release
	return 0;
}

/**
 *brief allocate and initialize a new encoder instance.
 *attention \n
 *N/A
 *param[in] phAacPlusEnc    pointer to an configuration information structure
 *param[in] pstConfig    pointer to an configuration information structure
 *retval ::CVI_SUCCESS   : Success
 *retval ::CVI_FAILURE   : FAILURE
 *see \n
 *N/A
 */
int  AACEncoderOpen(AAC_ENCODER_S **phAacPlusEnc,
							AACENC_CONFIG *pstConfig)
{
	//Empty function for release
	printf("[%s][%s]\n", __func__, _VERSION_TAG_);
	return 0;
}

/**
 *brief allocate and initialize a new encoder instance
 *attention \n
 *N/A
 *param[in] hAacPlusEnc    pointer to an configuration information structure
 *param[in] ps16PcmBuf    BLOCKSIZE*nChannels audio samples,interleaved
 *param[in] pu8Outbuf    pointer to output buffer,(must be 6144/8*MAX_CHANNELS bytes large)
 *param[in] ps32NumOutBytes    number of bytes in output buffer after processing
 *retval ::CVI_SUCCESS   : Success
 *retval ::CVI_FAILURE   : FAILURE
 *see \n
 *N/A
 */
CVI_S32  AACEncoderFrame(AAC_ENCODER_S * hAacPlusEnc,
						CVI_S16 * ps16PcmBuf,
						CVI_U8 *pu8Outbuf,
						CVI_S32 s32InputBytes,
						CVI_S32 * ps32NumOutBytes)
{
	//Empty function for release
	*ps32NumOutBytes = 640;
	return 0;
}

/**
 *brief close encoder device.
 *attention \n
 *N/A
 *param[in] hAacPlusEnc    pointer to an configuration information structure
 *retval N/A
 *see \n
 *N/A
 */
CVI_VOID AACEncoderClose(AAC_ENCODER_S *hAacPlusEnc)
{
	//Empty function for release
	//return 0;
}




