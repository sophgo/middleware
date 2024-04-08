/*
 * Copyright ., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: aacdec_interface.c
 * Description: audio transcode function interface as example
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include "cvi_type.h"
#include "cvi_aacdec.h"


#define _VERSION_TAG_  "audio_aac_EMPTY_release_add_len"

#ifndef AAC_MAX_NCHANS
#define AAC_MAX_NCHANS 2
#endif
#define AAC_MAX_NSAMPS 1024
#define AAC_MAINBUF_SIZE (768 * AAC_MAX_NCHANS)
/**<according to spec (13818-7 section 8.2.2, 14496-3 section 4.5.3),6144 bits =  */
 /*  768 bytes per SCE or CCE-I,12288 bits = 1536 bytes per CPE*/

#define AAC_NUM_PROFILES 3
#define AAC_PROFILE_MP 0
#define AAC_PROFILE_LC 1
#define AAC_PROFILE_SSR 2


/** @} */ /** <!-- ==== Macro Definition end ==== */
/*************************** Structure Definition ****************************/
/** \addtogroup      AACDEC */
/** @{ */ /** <!-- [AACDEC] */
/**Defines AACDEC error code*/

/**
 *brief Get version information.
 *attention \n
 *N/A
 *param[in] pVersion    :   version describe struct
 *retval ::CVI_SUCCESS   :   Success
 *retval ::CVI_FAILURE   :   pVersion is NULL, return CVI_FAILURE
 *see \n
 *N/A
 */
CVI_S32 CVI_AACDEC_GetVersion(AACDEC_VERSION_S *pVersion)
{
	strcpy((char *)pVersion->aVersion, (const char *)_VERSION_TAG_);
	printf("[%s][%s]\n", __func__, _VERSION_TAG_);
	return 0;
}



/**
 *brief create and initial decoder device.
 *attention
 *N/A
 *param[in] enTranType   : transport type
 *retval ::CVIAACDecoder   : init success, return non-NULL handle.
 *retval ::NULL          : init failure, return NULL
 *see
 *N/A
 */
CVIAACDecoder AACInitDecoder(AACDECTransportType enTranType)
{
	//Empty function for release
	printf("[%s][%s]\n", __func__, _VERSION_TAG_);
	AACFrameInfo *decoder = (AACFrameInfo *)malloc(sizeof(AACFrameInfo));
	return (void *)decoder;
}


/**
 *brief destroy AAC-Decoder, free the memory.
 *attention \n
 *N/A
 *param[in] CVIAACDecoder  : AAC-Decoder handle
 *see \n
 *N/A
 */
CVI_VOID AACFreeDecoder(CVIAACDecoder CVIAACDecoder)
{
	//Empty function for release
	if (CVIAACDecoder != NULL) {
		free((AACFrameInfo *)CVIAACDecoder);
	}

}



/**
 *brief set RawMode before decode Raw Format aac bitstream(Reserved API, unused now.)
 *attention \n
 *N/A
 *param[in] CVIAACDecoder  : AAC-Decoder handle
 *param[in] Chans       : inout channels
 *param[in] sampRate     : input sample rate
 *retval ::CVI_FAILURE    : RESERVED API, always return CVI_FAILURE.
 *see \n
 *N/A
 */
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

/**
 *brief look for valid AAC sync header
 *attention \n
 *N/A
 *param[in] CVIAACDecoder      : AAC-Decoder handle
 *param[in/out] ppInbufPtr   : address of the pointer of start-point of the bitstream
 *param[in/out] pBytesLeft   : pointer to BytesLeft that indicates bitstream numbers at input buffer
 *retval ::<0                : err, always return ERR_AAC_INDATA_UNDERFLOW
 *retval ::other             : Success, return number bytes of current frame
 *see \n
 *N/A
 */
CVI_S32 AACDecodeFindSyncHeader(CVIAACDecoder CVIAACDecoder,
										CVI_U8 **ppInbufPtr,
										CVI_S32 *pBytesLeft)
{
	//Empty function for release
	return 0;
}



/**
 *brief decoding AAC frame and output 1024(LC) OR
 *2048(HEAAC/eAAC/eAAC+) 16bit PCM samples per channel.
 *attention \n
 *param[in] CVIAACDecoder       : AAC-Decoder handle
 *param[in] ppInbufPtr        : address of the pointer of start-point of the bitstream
 *param[in/out] pBytesLeft    : pointer to BytesLeft that indicates
 *bitstream numbers at input buffer,indicates the left bytes
 *param[in] pOutPcm           : the address of the out pcm buffer,
 *pcm data in noninterlaced fotmat: L/L/L/... R/R/R/...
 *retval :: SUCCESS           : Success
 *retval :: ERROR_CODE        : FAILURE, return error_code.
 *see \n
 */
CVI_S32 AACDecodeFrame(CVIAACDecoder CVIAACDecoder,
							CVI_U8 **ppInbufPtr,
							CVI_S32 *pBytesLeft,
							CVI_S16 *pOutPcm)
{
	//Empty function for release
	*pBytesLeft = 2048;
	return 0;
}


/**
 *brief get the frame information.
 *attention \n
 *param[in] CVIAACDecoder       : AAC-Decoder handle
 *param[out] aacFrameInfo     : frame information
 *retval :: CVI_SUCCESS        : Success
 *retval :: ERROR_CODE        : FAILURE, return error_code.
 *see \n
 *N/A
 */
CVI_S32 AACGetLastFrameInfo(CVIAACDecoder CVIAACDecoder,
								AACFrameInfo *aacFrameInfo)
{
	//Empty function for release
	aacFrameInfo->outputSamps = 1024;
	aacFrameInfo->nChans = 1;
	return 0;
}


/**
 *brief set eosflag.
 *attention \n
 *param[in] CVIAACDecoder       : AAC-Decoder handle
 *param[in] s32Eosflag        : end flag
 *retval :: CVI_SUCCESS        : Success
 *retval :: ERROR_CODE        : FAILURE, return error_code.
 *see \n
 *N/A
 */

CVI_S32 AACDecoderSetEosFlag(CVIAACDecoder CVIAACDecoder,
								CVI_S32 s32Eosflag)
{
	//Empty function for release
	return 0;
}



/**
 *brief flush internal codec state (after seeking, for example)
 *attention \n
 *param[in] CVIAACDecoder       : AAC-Decoder handle
 *retval :: CVI_SUCCESS        : Success
 *retval :: ERROR_CODE        : FAILURE, return error_code.
 *see \n
 *N/A
 */
CVI_S32 AACFlushCodec(CVIAACDecoder CVIAACDecoder)
{
	//Empty function for release
	return 0;
}