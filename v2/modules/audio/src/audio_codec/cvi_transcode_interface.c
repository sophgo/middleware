/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_transcode_interface.c
 * Description: audio transcode function interface
 */

#include <sys/types.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "condef.h"
#include "g711.h"
#include "g726.h"
#include "ima_adpcm.h"
#ifdef FAAD_FAAC_TEST
#include "aac_encode.h"
#include "aac_decode.h"
#endif

#include "cvi_transcode_interface.h"

/* 20200629 : adding aac encoder */
#define MAX_SUPPORT_CODEC 4
#define DEFAULT_BYTES_PER_SAMPLE 2
#define G711_ONE_LEN 160
#define CVI_TRANS_UNUSED_REF(X)  ((X) = (X))
#define NOT_SUPPORT_CODEC_TYPE(x)  (x >= AUD_CODEC_END ? 1 : 0)
#define CHK_NULL_PT(ptr)\
/*
 *	do {\
 *		if (NULL == (CVI_U8 *)ptr)\
 *			printf("[fatal error]ptr is NULL,fuc:%s,line:%d\n", __func__, __LINE__);\
 *			usleep(1000);\
 *			return CVI_FAILURE;\
 *	} while (0)
 */

/* Handle pointer for each codec */
g726_state_t *m_state726;
ima_adpcm_state_t *ima_enc_state;
ima_adpcm_state_t *ima_dec_state;
#ifdef FAAD_FAAC_TEST
stAAC_DecInfo aacdec_info;
#endif
CVI_S32 m_pcmSize;
CVI_S32 m_g7FrameSize;
CVI_S32 s32BytesPerSample = 2;

#define VERIFIED_VDVI 0
#if VERIFIED_VDVI
char *ptmpbuffer;
#endif

typedef struct _cviTransInstance {
	CVI_VOID *pEncState;
	CVI_VOID *pDecState;
	S_CVI_AUDIO_CONFIG Config;
	CVI_S32 s32BytesPerSample;
} stCviTranscodeInst;

static void aenc_dump_audiodata(char *filename, char *buf, unsigned int len)
{
	FILE *fp;

	if (filename == NULL) {
		return;
	}

	fp = fopen(filename, "ab+");
	fwrite(buf, 1, len, fp);
	fclose(fp);

}


stCviTranscodeInst *pstCviTranscodeInst;

CVI_VOID *CVI_AUDIO_Transcode_Init(CVI_VOID *inst,
				E_CVI_AUDIOCODEC eAudCodecType,
				S_CVI_AUDIO_CONFIG *pConfig)
{

	printf("CVI_AUDIO_Transcode VER[%s]\n", _AUDIO_TRANSCODE_VERSION_TAG_);
	pstCviTranscodeInst = NULL;

	pstCviTranscodeInst = (stCviTranscodeInst *)malloc(sizeof(stCviTranscodeInst));

	if (pstCviTranscodeInst == NULL) {
		printf("[FATAL ERROR]System allocate transcode instance failure\n");
		return CVI_NULL;

	} else {
		memset(pstCviTranscodeInst, 0, sizeof(stCviTranscodeInst));
		pstCviTranscodeInst->pDecState = NULL;
		pstCviTranscodeInst->pEncState = NULL;
		memcpy(&pstCviTranscodeInst->Config, pConfig, sizeof(S_CVI_AUDIO_CONFIG));
	}


	CVI_S32 s32Rate = 0;

	m_state726 = NULL;

	m_g7FrameSize = G711_ONE_LEN;


	if (NOT_SUPPORT_CODEC_TYPE(eAudCodecType) == 1) {
		printf("[Error] Not support codec type[%d]\n", (int)eAudCodecType);
		return CVI_NULL;
	}

#if VERIFIED_VDVI
	ptmpbuffer =  (CVI_CHAR *)malloc(50 * 160 * 1024);
#endif
	switch (pConfig->bitrate) {
	case Rate16kBits: {
		pstCviTranscodeInst->s32BytesPerSample = 2;
		break;
	}
	case Rate24kBits: {
		pstCviTranscodeInst->s32BytesPerSample = 3;
		break;
	}
	case Rate32kBits: {
		pstCviTranscodeInst->s32BytesPerSample = 4;
		break;
	}
	case Rate40kBits: {
		pstCviTranscodeInst->s32BytesPerSample = 5;
		break;
	}
	default: {
		pstCviTranscodeInst->s32BytesPerSample = 2;
		printf("[Error]Cannot recognize bit rate\n");
		break;
	}
	}



	if (eAudCodecType == AUD_CODEC_G726) {
		printf("AUD_CODEC_G726 Init\n");
		m_state726 = (g726_state_t *)malloc(sizeof(g726_state_t));

		switch (pConfig->bitrate) {

		case Rate16kBits: {
			s32Rate = 8000 * 2;
			m_pcmSize = (2 * m_g7FrameSize * 120 + 30) / 30;
			break;
		}
		case Rate24kBits: {
			s32Rate = 8000 * 3;
			m_pcmSize = (2 * m_g7FrameSize * 80 + 30) / 30;
			break;
		}
		case Rate32kBits: {
			s32Rate = 8000 * 4;
			m_pcmSize = (2 * m_g7FrameSize * 60 + 30) / 30;
			break;
		}
		case Rate40kBits: {
			s32Rate = 8000 * 5;
			m_pcmSize = (2 * m_g7FrameSize * 48 + 30) / 30;
			break;
		}
		default: {
			printf("[Error]Cannot recognize bit rate\n");
			break;
		}
		}

		m_state726 = g726_init(m_state726, s32Rate);
		pstCviTranscodeInst->pDecState = m_state726;
		pstCviTranscodeInst->pEncState = m_state726;

	}

	if (eAudCodecType == AUD_CODEC_ADPCM_IMA) {
		int enc_chunk_size = 505;

		ima_enc_state = ima_adpcm_init(NULL, IMA_ADPCM_IMA4, enc_chunk_size);

		if (ima_enc_state == NULL) {
		printf("[Error]Cannot create encoder AUD_CODEC_ADPCM_IMA\n");
		return CVI_NULL;
		}

		ima_dec_state = ima_adpcm_init(NULL, IMA_ADPCM_IMA4, enc_chunk_size);

		if (ima_dec_state == NULL) {
		printf("[Error]Cannot create decoder AUD_CODEC_ADPCM_IMA\n");
		return CVI_NULL;
		}
		pstCviTranscodeInst->pDecState = ima_enc_state;
		pstCviTranscodeInst->pEncState = ima_dec_state;

	}

	if (eAudCodecType == AUD_CODEC_ADPCM_DVI4) {
		int enc_chunk_size = 505;

		ima_enc_state = ima_adpcm_init(NULL, IMA_ADPCM_DVI4, enc_chunk_size);

		if (ima_enc_state == NULL) {
		printf("[Error]Cannot create encoder AUD_CODEC_ADPCM_DVI4\n");
		return CVI_NULL;
		}

		ima_dec_state = ima_adpcm_init(NULL, IMA_ADPCM_DVI4, enc_chunk_size);

		if (ima_dec_state == NULL) {
		printf("[Error]Cannot create decoder AUD_CODEC_ADPCM_DVI4\n");
		return CVI_NULL;
		}
		pstCviTranscodeInst->pDecState = ima_enc_state;
		pstCviTranscodeInst->pEncState = ima_dec_state;
	}

	if (eAudCodecType == AUD_CODEC_ADPCM_VDVI) {
		int enc_chunk_size = 505;

		ima_enc_state = ima_adpcm_init(NULL, IMA_ADPCM_VDVI, enc_chunk_size);

		if (ima_enc_state == NULL) {
		printf("[Error]Cannot create encoder AUD_CODEC_ADPCM_VDVI\n");
		return CVI_NULL;
		}

		printf("[Important Notice]:--------------------------------------------------\n");
		printf("[Warning]ADPCM_VDVI: Output of encoder ADPCM_VDVI is variant\n");
		printf("[Warning]ADPCM_VDVI: User must keep track of each encoded output size\n");
		printf("[Warning]ADPCM_VDVI: Otherwise you may decode failure\n");
		printf("---------------------------------------------------------------------\n");

		ima_dec_state = ima_adpcm_init(NULL, IMA_ADPCM_VDVI, enc_chunk_size);

		if (ima_dec_state == NULL) {
		printf("[Error]Cannot create decoder AUD_CODEC_ADPCM_VDVI\n");
		return CVI_NULL;
		}
		pstCviTranscodeInst->pDecState = ima_enc_state;
		pstCviTranscodeInst->pEncState = ima_dec_state;
	}
#ifdef FAAD_FAAC_TEST
	if (eAudCodecType == AUD_CODEC_AAC) {
		//init for aac encoder ----------start
		stAAC_InAudioInfo aac_info;

		switch (pConfig->bitrate) {

		case Rate16kBits: {
			aac_info.u32PCMBitSize = 16;
			aacdec_info.u32PCMBitSize = 16;
			break;
		}
		case Rate24kBits: {
			aac_info.u32PCMBitSize = 24;
			aacdec_info.u32PCMBitSize = 24;
			break;
		}
		case Rate32kBits: {
			aac_info.u32PCMBitSize = 32;
			aacdec_info.u32PCMBitSize = 32;
			break;
		}
		case Rate40kBits: {
			aac_info.u32PCMBitSize = 40;
			aacdec_info.u32PCMBitSize = 40;
			break;
		}
		default: {
			printf("[Error]Cannot recognize bit rate\n");
			break;
		}
		}

		aac_info.ucAudioChannel = pConfig->channel_num;
		aac_info.u32AudioSamplerate = pConfig->sample_rate;

		if (aac_enc_init(aac_info) != CVI_TRUE) {
			/*error in aac init*/
			printf("[Error]AAC encode init failure\n");
		} else {
			printf("AAC encode init success\n");
		}
		//init for aac encoder ----------end
		//init aac decode ----start
		aacdec_info.ucAudioChannel = pConfig->channel_num;
		aacdec_info.u32AudioSamplerate =  pConfig->sample_rate;

		aacdec_info.handle = aac_dec_init(aacdec_info);
		if (aacdec_info.handle == CVI_NULL)
			printf("[Error]aac decoder init failure....\n");
		else
			pstCviTranscodeInst->pDecState = aacdec_info.handle;
		//init aac decode ----end

	}
#endif
	inst == NULL ? (inst = NULL) : (inst = (CVI_VOID *)pstCviTranscodeInst);

	return pstCviTranscodeInst;
}

CVI_S32 CVI_AUDIO_Transcode_DeInit(CVI_VOID *inst, E_CVI_AUDIOCODEC eAudCodecType)
{
	CHK_NULL_PT(inst);
	stCviTranscodeInst *pTransInst = (stCviTranscodeInst *)inst;

	if (eAudCodecType == AUD_CODEC_G726) {
		printf("AUD_CODEC_G726 DeInit\n");
		SAFE_FREE_BUF(pTransInst->pDecState);
		/* Do not need to double free */
		/* SAFE_FREE_BUF(pTransInst->pEncState); */
	}

	if ((eAudCodecType == AUD_CODEC_ADPCM_VDVI) ||
		(eAudCodecType == AUD_CODEC_ADPCM_DVI4) ||
		(eAudCodecType == AUD_CODEC_ADPCM_IMA)) {

		ima_adpcm_free(pTransInst->pDecState);
		ima_adpcm_free(pTransInst->pEncState);
	}

#ifdef FAAD_FAAC_TEST
	if (eAudCodecType == AUD_CODEC_AAC) {
		//deinit the aac encoder -----------------------start
		printf("AUD_CODEC_AAC DeInit\n");
		if (aac_enc_deinit() != CVI_TRUE)
			printf("[Error]AUD_CODEC_AAC Deinit Failure\n");
		else
			printf("AUD_CODEC_AAC DeInit success!!\n");
		//deinit the aac encoder -----------------------end

		//deinit the aac decoder -----------------------start
		CVI_BOOL bCloseAACDec = CVI_FALSE;

		if (pTransInst->pDecState != CVI_NULL) {
			bCloseAACDec = aac_dec_deinit(pTransInst->pDecState);

			if (bCloseAACDec)
				printf("AUD_CODEC_AAC decode Deinit success!!\n");
			else
				printf("AUD_CODEC_AAC decode Deinit failure!!\n");
		}
		//deinit the aac decoder -----------------------end

	}
#endif

#if VERIFIED_VDVI
free(ptmpbuffer);
#endif
	free(pTransInst);
	return CVI_SUCCESS;
}

CVI_S32 CVI_AUDIO_Encode(CVI_VOID *inst,
			E_CVI_AUDIOCODEC eACodecType, CVI_VOID *pInputBuf,
			CVI_VOID *pOutputBUf, CVI_S32 s32InputLen, CVI_S32 *ps32OutLen)
{

	CHK_NULL_PT(inst);

	stCviTranscodeInst *pTransInst = (stCviTranscodeInst *)inst;
	CVI_S32 s32Ret = -1;
	/* user notice: when using this api */
	/* both input/output length unit are in bytes not sample */
	/* Inside this api will auto matically transfer to sample */
	/* s32InputLen: in bytes */
	/* *ps32OutLen in bytes */
	if (access("/tmp/dump_aenc_in", F_OK) == 0) {
		aenc_dump_audiodata((char *)"/tmp/dump_aenc_in.pcm",
			(char *)pInputBuf,
			(unsigned int)s32InputLen);
	}

	switch (eACodecType) {
	case AUD_CODEC_NONE: {
		printf("Aud Codec Set To None[0]\n");
		break;
	}
	case AUD_CODEC_G711_ALAW: {
		//transfer input bytes to input samples
		s32InputLen = s32InputLen / s32BytesPerSample;
		/* input in samples return numbers of bytes */
		s32Ret = g711_encode(pOutputBUf, ps32OutLen,
				pInputBuf, s32InputLen, TP_ALAW);

		break;
	}
	case AUD_CODEC_G711_MLAW: {
		//transfer input bytes to input samples
		s32InputLen = s32InputLen / s32BytesPerSample;
		/* input in samples return numbers of bytes */
		s32Ret = g711_encode(pOutputBUf, ps32OutLen,
				     pInputBuf, s32InputLen, TP_ULAW);
				     //input samples return numbers of bytes
		break;
	}
	case AUD_CODEC_G726: {
		/* int g726_encode(g726_state_t *s, */
		/* unsigned char g726_data[] */
		/* const short amp[] */
		/* int len) */
		//transfer input bytes to input samples
		s32InputLen = s32InputLen / s32BytesPerSample;
		/* input in samples return numbers of bytes */
		s32Ret = g726_encode(pTransInst->pEncState, (unsigned char *)pOutputBUf,
				     (const short *)pInputBuf, s32InputLen);
		if (s32Ret >= 0)
			*ps32OutLen = s32Ret;
		break;
	}
	case AUD_CODEC_ADPCM_IMA: {
		printf("s32InputLen[%d] s32BytesPerSample[%d]\n", s32InputLen, s32BytesPerSample);
		s32InputLen = s32InputLen / s32BytesPerSample;
		printf("frm size[%d]\n", s32InputLen);
		s32Ret = ima_adpcm_encode(pTransInst->pEncState, (unsigned char *)pOutputBUf,
					(const short *)pInputBuf, s32InputLen);
		if (s32Ret >= 0)
			*ps32OutLen = s32Ret;
		break;
	}
	case AUD_CODEC_ADPCM_DVI4: {
		printf("s32InputLen[%d] s32BytesPerSample[%d]\n", s32InputLen, s32BytesPerSample);
		s32InputLen = s32InputLen / s32BytesPerSample;
		s32Ret = ima_adpcm_encode(pTransInst->pEncState, (unsigned char *)pOutputBUf,
					(const short *)pInputBuf, s32InputLen);
		if (s32Ret >= 0)
			*ps32OutLen = s32Ret;
		break;
	}
	case AUD_CODEC_ADPCM_VDVI: {
		s32InputLen = s32InputLen / s32BytesPerSample;
		s32Ret = ima_adpcm_encode(pTransInst->pEncState, (unsigned char *)pOutputBUf,
					(const short *)pInputBuf, s32InputLen);
		printf("VDVI encode input sample[%d] return bytes[%d]\n", s32InputLen, s32Ret);
#if VERIFIED_VDVI
		memcpy(ptmpbuffer, pOutputBUf, s32Ret);
		int intmp = s32Ret;

		s32Ret = ima_adpcm_decode(pTransInst->pDecState, (short *)pOutputBUf,
				(const unsigned char *)ptmpbuffer, s32Ret);

		printf("#VDVI decode input bytes[%d] return sample[%d]\n", intmp, s32Ret);
		if (s32Ret >= 0) /* return dec_frames */
			*ps32OutLen = s32Ret * s32BytesPerSample;

#else
		if (s32Ret >= 0)
			*ps32OutLen = s32Ret;
#endif
		break;
	}
#ifdef FAAD_FAAC_TEST
	case AUD_CODEC_AAC: {

		s32InputLen = s32InputLen / s32BytesPerSample;
		s32Ret = aac_encode((int32_t *)pInputBuf, (unsigned int)s32InputLen,
							(unsigned char *)pOutputBUf, (unsigned int *) ps32OutLen);
		if (s32Ret >= 0) {
			//printf("s32Ret[%d] ps32OutLen[%d]\n", )
			s32Ret = *ps32OutLen;
		}

		break;
	}
#endif
	case AUD_CODEC_END:
	default: {
		printf("Aud Codec Set Out of range\n");
		break;
	}
	}

	if (access("/tmp/dump_aenc_out", F_OK) == 0) {
		aenc_dump_audiodata((char *)"/tmp/dump_aenc_out.raw",
			(char *)pOutputBUf,
			(unsigned int)*ps32OutLen);
	}

	//printf("ENCODE inlen[%d] outlen[%d]\n", s32InputLen, *ps32OutLen);

#ifdef FAAD_FAAC_TEST
	if ((eACodecType != AUD_CODEC_AAC) && (s32Ret <= 0))
		return CVI_FAILURE;
#endif

	return CVI_SUCCESS;
}

CVI_S32 CVI_AUDIO_DecodeParseAAC(CVI_VOID *inst,
			E_CVI_AUDIOCODEC eACodecType,
			CVI_VOID *pInputBuf,
			CVI_S32 s32InputLen,
			CVI_S32 *s32RequireBytes)
{
	CVI_TRANS_UNUSED_REF(inst);
	CVI_TRANS_UNUSED_REF(eACodecType);
	CVI_TRANS_UNUSED_REF(pInputBuf);
	CVI_TRANS_UNUSED_REF(s32InputLen);
	CVI_TRANS_UNUSED_REF(s32RequireBytes);
#ifdef FAAD_FAAC_TEST
	CHK_NULL_PT(inst);
	stCviTranscodeInst *pTransInst = (stCviTranscodeInst *)inst;
	CVI_S32 s32Ret = -1;

	if (eACodecType == AUD_CODEC_AAC) {
		s32Ret = aac_decode_ParseInputBytes(pTransInst->pDecState,
										(unsigned char *)pInputBuf,
										(unsigned int) s32InputLen,
										(unsigned int *)s32RequireBytes);

		if (s32Ret == CVI_FAILURE) {
			return CVI_FAILURE;
			//return failure if parsing failure
		}

	} else {
		printf("[Error]This API only support AAC decode\n");
		return CVI_FAILURE;
	}
#endif
	return CVI_SUCCESS;
}

CVI_S32 CVI_AUDIO_Decode(CVI_VOID *inst,
			E_CVI_AUDIOCODEC eACodecType, CVI_VOID *pInputBuf,
			CVI_VOID *pOutputBUf, CVI_S32 s32InputLen, CVI_S32 *s32OutLen)
{
	CHK_NULL_PT(inst);
	stCviTranscodeInst *pTransInst = (stCviTranscodeInst *)inst;
	CVI_S32 s32Ret = -1;
	/* user notice: when using this api */
	/* both input/output length unit are in bytes not sample */
	/* Inside this api will auto matically transfer to sample */
	/* s32InputLen: in bytes */
	/* *ps32OutLen in bytes */
	switch (eACodecType) {
	case AUD_CODEC_NONE: {
		printf("Aud Codec Set To None[0]\n");
		break;
	}
	break;
	case AUD_CODEC_G711_ALAW: {
		*s32OutLen = 2*s32InputLen;
		/* input in bytes return in samples*/
		s32Ret = g711_decode(pOutputBUf, s32OutLen,
					pInputBuf, s32InputLen, TP_ALAW);
		if (s32Ret >= 0)
			*s32OutLen = s32Ret * s32BytesPerSample;
		break;
	}
	case AUD_CODEC_G711_MLAW: {
		*s32OutLen = 2*s32InputLen;
		/* input in bytes return in samples*/
		s32Ret = g711_decode(pOutputBUf, s32OutLen,
					pInputBuf, s32InputLen, TP_ULAW);
		if (s32Ret >= 0)
			*s32OutLen = s32Ret * s32BytesPerSample;
		break;
	}
	case AUD_CODEC_G726: {
		/*int iRet = g726_decode(m_state726, (short*)outbuf, inbuf, inlen); */
		/* input in bytes return in samples*/
		s32Ret = g726_decode(pTransInst->pDecState, (short *)pOutputBUf,
					(unsigned char *)pInputBuf, (unsigned int)s32InputLen);

		if (s32Ret >= 0)
			*s32OutLen = s32Ret * s32BytesPerSample;
		break;
	}
	case AUD_CODEC_ADPCM_IMA: {

		s32Ret = ima_adpcm_decode(pTransInst->pDecState, (short *)pOutputBUf,
				(const unsigned char *)pInputBuf, s32InputLen);

		if (s32Ret >= 0) /* return dec_frames */
			*s32OutLen = s32Ret * s32BytesPerSample;
		break;
	}
	case AUD_CODEC_ADPCM_DVI4: {

		s32Ret = ima_adpcm_decode(pTransInst->pDecState, (short *)pOutputBUf,
				(const unsigned char *)pInputBuf, s32InputLen);

		if (s32Ret >= 0) /* return dec_frames */
			*s32OutLen = s32Ret * s32BytesPerSample;
		break;
	}
	case AUD_CODEC_ADPCM_VDVI: {

		s32Ret = ima_adpcm_decode(pTransInst->pDecState, (short *)pOutputBUf,
				(const unsigned char *)pInputBuf, s32InputLen);
		printf("VDVI decode input bytes[%d] return sample[%d]\n", s32InputLen, s32Ret);
		if (s32Ret >= 0) /* return dec_frames */
			*s32OutLen = s32Ret * s32BytesPerSample;
		break;
	}
#ifdef FAAD_FAAC_TEST
	case AUD_CODEC_AAC: {
		if (pTransInst->pDecState == CVI_NULL) {
			printf("[Error]aac decoder handle null.....\n");
			return CVI_FAILURE;

		} else {
		s32Ret = aac_decode(pTransInst->pDecState,
						(unsigned char *)pInputBuf,
						(unsigned int) s32InputLen,
						(unsigned char *)pOutputBUf,
						(unsigned int *)s32OutLen);

		if (s32Ret < 0)
			printf("[Error]func:aac_decode failure...\n");
		}
		break;
	}
#endif
	default:
		break;
	}

	if (s32Ret < 0)
		return CVI_FAILURE;

	return CVI_SUCCESS;
}

