#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#if defined(__CV181X__) || defined(__CV180X__) || defined(__CV186X__)
#include <linux/cvi_type.h>
#else
#include <cvi_type.h>
#endif
#include "cvi_aacenc.h"


#define MAX_CHANNELS 2
#define AACENC_BLOCKSIZE 1024
#define _VERSION_TAG_  "audio_aac_enc_empty_release"


#define AACLC_ENC_AOT 2
#define HEAAC_ENC_AOT 5
#define HEAAC_PLUS_ENC_AOT 29
#define AACLD_ENC_AOT 23
#define AACELD_ENC_AOT 39



CVI_S32  CVI_AACENC_GetVersion(AACENC_VERSION_S *pVersion)
{
	printf("[%s][%s]\n", __func__, _VERSION_TAG_);
	strcpy((char *)pVersion->aVersion, (const char *)_VERSION_TAG_);
	return 0;
}

int  AACInitDefaultConfig(AACENC_CONFIG *pstConfig)
{
	//Empty function for release
	return 0;
}

int  AACEncoderOpen(AAC_ENCODER_S **phAacPlusEnc,
							AACENC_CONFIG *pstConfig)
{
	//Empty function for release
	printf("[%s][%s]\n", __func__, _VERSION_TAG_);
	return 0;
}

CVI_S32  AACEncoderFrame(AAC_ENCODER_S *hAacPlusEnc,
						CVI_S16 *ps16PcmBuf,
						CVI_U8 *pu8Outbuf,
						CVI_S32 s32InputBytes,
						CVI_S32 *ps32NumOutBytes)
{
	//Empty function for release
	return 0;
}

CVI_VOID AACEncoderClose(AAC_ENCODER_S *hAacPlusEnc)
{
	//Empty function for release
	//return 0;
}




