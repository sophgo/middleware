#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <cvi_type.h>
#include "cvi_aacdec.h"


#define _VERSION_TAG_  "audio_aac_EMPTY_release"

#ifndef AAC_MAX_NCHANS
#define AAC_MAX_NCHANS 2
#endif
#define AAC_MAX_NSAMPS 1024
#define AAC_MAINBUF_SIZE (768 * AAC_MAX_NCHANS)

#define AAC_NUM_PROFILES 3
#define AAC_PROFILE_MP 0
#define AAC_PROFILE_LC 1
#define AAC_PROFILE_SSR 2


CVI_S32 CVI_AACDEC_GetVersion(AACDEC_VERSION_S *pVersion)
{
	strcpy((char *)pVersion->aVersion, (const char *)_VERSION_TAG_);
	printf("[%s][%s]\n", __func__, _VERSION_TAG_);
	return 0;
}


CVIAACDecoder AACInitDecoder(AACDECTransportType enTranType)
{
	//Empty function for release
	printf("[%s][%s]\n", __func__, _VERSION_TAG_);
	return NULL;
}


CVI_VOID AACFreeDecoder(CVIAACDecoder CVIAACDecoder)
{
	//Empty function for release

}

CVI_S32 AACSetRawMode(CVIAACDecoder CVIAACDecoder,
							CVI_S32 nChans,
							CVI_S32 sampRate)
{
	//Empty function for release
	return 0;
}

int get_one_ADTS_frame(unsigned char *buffer, size_t buf_size, size_t *data_size)
{
	//Empty function for release
	return 0;
}

CVI_S32 AACDecodeFindSyncHeader(CVIAACDecoder CVIAACDecoder,
										CVI_U8 **ppInbufPtr,
										CVI_S32 *pBytesLeft)
{
	//Empty function for release
	return 0;
}

CVI_S32 AACDecodeFrame(CVIAACDecoder CVIAACDecoder,
							CVI_U8 **ppInbufPtr,
							CVI_S32 *pBytesLeft,
							CVI_S16 *pOutPcm)
{
	//Empty function for release
	return 0;
}

CVI_S32 AACGetLastFrameInfo(CVIAACDecoder CVIAACDecoder,
								AACFrameInfo *aacFrameInfo)
{
	//Empty function for release
	return 0;
}

CVI_S32 AACDecoderSetEosFlag(CVIAACDecoder CVIAACDecoder,
								CVI_S32 s32Eosflag)
{
	//Empty function for release
	return 0;
}

CVI_S32 AACFlushCodec(CVIAACDecoder CVIAACDecoder)
{
	//Empty function for release
	return 0;
}

