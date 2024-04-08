#include<stdio.h>
#include <sys/prctl.h>
#include "cvi_audio_interface_tinyalsa.h"
#include "cvi_ao_internal.h"
#ifdef RPC_MULTI_PROCESS_AUDIO
#include "cvi_audio_rpc.h"
#endif
#include "cvi_audio_transcode.h"


#define CVITEK_AUD_DEC_OUT_BUFFER   (10240)
#define CVI_AAC_BUF_SIZE (1024 * 6)
CVI_S32 adec_end_flag; //for aac


ST_ADEC_INSTANCE gstAdecInstance[ADEC_MAX_CHN_NUM];
ADEC_DECODER_S gAACdec;
ST_AAC_DEC_INST  *gstpAacDecInstance;

//extern ST_AO_INSTANCE gstAoInstance[CVI_MAX_AO_DEVICE_ID_NUM];
/* ADEC(audio input <ffmpeg>) function api. */
/* ADEC function api. */

//----------------------
int get_aac_sample_rate(int sampling_frequency_index)
{
	int sample_rate = 0;

	switch (sampling_frequency_index) {
	case 0:
		sample_rate = 96000;
		break;
	case 1:
		sample_rate = 88200;
		break;
	case 2:
		sample_rate = 64000;
		break;
	case 3:
		sample_rate = 48000;
		break;
	case 4:
		sample_rate = 44100;
		break;
	case 5:
		sample_rate = 32000;
		break;
	case 6:
		sample_rate = 24000;
		break;
	case 7:
		sample_rate = 22050;
		break;
	case 8:
		sample_rate = 16000;
		break;
	case 9:
		sample_rate = 12000;
		break;
	case 10:
		sample_rate = 11025;
		break;
	case 11:
		sample_rate = 8000;
		break;
	default:
		log_error("sampling_frequency_index unknown\n");
		break;
	}
	return sample_rate;
}


int _cvi_aac_decode(int AdChn, ST_AAC_DEC *pst_aac_dec, unsigned char *pInputBuf, short *pOutbuf, int byte_left,
		    int *frame_byte)
{
	ST_ADEC_INSTANCE *_adec_instance = &gstAdecInstance[AdChn];
	int sampling_frequency_index;
	int s32Ret;
	int s32TotalSizeBytes = byte_left;
	int poutbuf_byte = 0;

	AAC_FRAME_INFO_S  AACInfo;
	int totalbytepass;

	if (byte_left > (int) pst_aac_dec->aac_bufferbyte) {
		log_error("byte_left[%d] > aac_bufferbyte[%d]\n", byte_left, (int) pst_aac_dec->aac_bufferbyte);
		return -1;
	}
	if (pst_aac_dec->aac_remainbytes != 0) {
		if ((pst_aac_dec->aac_remainbytes + byte_left) >
		    pst_aac_dec->aac_bufferbyte) {

			log_error("remain[%d] + input[%d] > bufsize[%d]",
				  (int)pst_aac_dec->aac_remainbytes,
				  byte_left,
				  (int)pst_aac_dec->aac_bufferbyte);
			log_error("force flush the buffer\n");
			memset(pst_aac_dec->aac_inputbuffer, 0, pst_aac_dec->aac_bufferbyte);
			memmove(pst_aac_dec->aac_inputbuffer, pInputBuf, byte_left);
			s32TotalSizeBytes = byte_left;
			pst_aac_dec->aac_remainbytes = s32TotalSizeBytes;
		} else {
			memmove(pst_aac_dec->aac_inputbuffer + pst_aac_dec->aac_remainbytes, pInputBuf, byte_left);
			s32TotalSizeBytes = byte_left + pst_aac_dec->aac_remainbytes;
			log_debug("has data s32TotalSizeBytes = %d\n", s32TotalSizeBytes);
		}

	} else {
		memmove(pst_aac_dec->aac_inputbuffer, pInputBuf, byte_left);
		s32TotalSizeBytes = byte_left;
		log_debug("[%s][%d] inputlen[%d]\n", __func__, __LINE__, byte_left);
	}

	pst_aac_dec->aac_remainbytes = s32TotalSizeBytes;

	while (1) {
		if (pst_aac_dec->aac_remainbytes > 7) {

			if ((pst_aac_dec->aac_inputbuffer[0] == 0xff) &&
			    ((pst_aac_dec->aac_inputbuffer[1] & 0xf0) == 0xf0)) {//adts

				sampling_frequency_index = (pst_aac_dec->aac_inputbuffer[2] & 0x3C) >> 2;
				pst_aac_dec->sample_rate = get_aac_sample_rate(sampling_frequency_index);
				pst_aac_dec->channel = ((pst_aac_dec->aac_inputbuffer[2] & 0x01) << 2) |
						       ((pst_aac_dec->aac_inputbuffer[3] & 0xC0) >> 6);

				pst_aac_dec->frame_byte = ((pst_aac_dec->aac_inputbuffer[3] & 0x03) << 11) |
					(pst_aac_dec->aac_inputbuffer[4] << 3) | (pst_aac_dec->aac_inputbuffer[5] >> 5);

				log_debug("packer_byte = %d,channel = %d,sample_rate = %d\n",
					  pst_aac_dec->frame_byte,
					  pst_aac_dec->channel,
					  pst_aac_dec->sample_rate);

				if (pst_aac_dec->aac_remainbytes < (unsigned int)pst_aac_dec->frame_byte) {
					log_debug(" ADTS_byte(7) <remainbytes < aac frame_byte\n");
					break;
				}

				int output_bytes = 0;
				int vail = pst_aac_dec->frame_byte;

				_check_and_dump("/tmp/aacplay_decode_before",
						(char *)pst_aac_dec->aac_inputbuffer,
						(unsigned int)vail);

				s32Ret = gAACdec.pfnDecodeFrm(_adec_instance->AdecHandle,
							      (CVI_U8 **)(&pst_aac_dec->aac_inputbuffer),
							      (CVI_S32 *)&vail,
							      (CVI_U16 *)pst_aac_dec->aac_dec_after_buf,
							      (CVI_U32 *)&output_bytes,
							      (CVI_U32 *)&pst_aac_dec->channel);
				if (s32Ret != 0) {
					log_error("aac one frame decode faile\n");
					return -1;
				}
				_check_and_dump("/tmp/aacplay_decode_after",
						(char *)pst_aac_dec->aac_dec_after_buf,
						(unsigned int)output_bytes);

				pst_aac_dec->aac_remainbytes = pst_aac_dec->aac_remainbytes -
							       pst_aac_dec->frame_byte;

				memmove(pst_aac_dec->aac_inputbuffer,
				       pst_aac_dec->aac_inputbuffer + pst_aac_dec->frame_byte,
				       pst_aac_dec->aac_remainbytes);

				memmove(pOutbuf + poutbuf_byte/sizeof(short),
					pst_aac_dec->aac_dec_after_buf, output_bytes);
				poutbuf_byte += output_bytes;
				*frame_byte = poutbuf_byte;

				totalbytepass = gAACdec.pfnGetFrmInfo(_adec_instance->AdecHandle,
								      (CVI_VOID *)(&AACInfo));
				log_debug("output_bytes = %d, poutbuf_byte = %d, totalbytepass = %d, bitrate = %d\n",
					  output_bytes, poutbuf_byte, totalbytepass, AACInfo.s32BitRate);

			} else {
				memmove(pst_aac_dec->aac_inputbuffer, pst_aac_dec->aac_inputbuffer + 1,
						pst_aac_dec->aac_remainbytes - 1);
				pst_aac_dec->aac_remainbytes -= 1;
				//log_error("aac_inputbuffer is not ADTS\n");
			}

		} else {
			log_debug("remainbytes = < ADTS_byte(7)\n");
			break;
		}

	}

	return 0;
}


CVI_S32 CVI_ADEC_Init(void)
{
	for (int i = 0 ; i < ADEC_MAX_CHN_NUM ; i++) {
		memset(&gstAdecInstance[i], 0, sizeof(ST_ADEC_INSTANCE));
	}
	return CVI_SUCCESS;
}

CVI_VOID CVI_ADEC_Deinit(void)
{
	//return;
}

CVI_S32 CVI_ADEC_CreateChn(ADEC_CHN AdChn, const ADEC_CHN_ATTR_S *pstAttr)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_adec_create_chn(AdChn, pstAttr);
	}
#endif
	if (CHECK_ADEC_DEVID_VALID(AdChn)) {
		log_error("\n");
		return CVI_ERR_ADEC_INVALID_DEVID;
	}

	ST_ADEC_INSTANCE *_adec_instance = &gstAdecInstance[AdChn];

	if (CVIAUDIO_CHECK_NULL((void *)pstAttr))
		return -1;
	if (_adec_instance->bEnableAdec) {
		printf("AdChn:%d areadly init\n", AdChn);
		return CVI_ERR_ADEC_EXIST;
	}

	if (pstAttr->s32ChannelNums != 1 && pstAttr->s32ChannelNums != 2) {
		log_error("invalid channels %d.\n", pstAttr->s32ChannelNums);
		return CVI_ERR_ADEC_ILLEGAL_PARAM;
	}
	_adec_instance->AdecHandle = CVI_NULL;
	memcpy(&gstAdecInstance[AdChn].adec_attrs,
	       (ADEC_CHN_ATTR_S *)pstAttr,
	       sizeof(ADEC_CHN_ATTR_S));
	_adec_instance->DecBuff =
		(CVI_U8 *) malloc(_adec_instance->adec_attrs.u32BufSize * DEFAULT_MAX_FRM_SIZE);
	if (!_adec_instance->DecBuff) {
		log_error("malloc failed.\n");
		return CVI_ERR_AO_NOMEM;
	}

	_adec_instance->pDecReadBuff =
		(CVI_CHAR *) malloc(_adec_instance->adec_attrs.u32BufSize * DEFAULT_MAX_FRM_SIZE);
	if (!_adec_instance->pDecReadBuff) {
		log_error("_adec_instance->pDecReadBuff FAILURE\n");
		free(_adec_instance->DecBuff);
		return CVI_ERR_AO_NOMEM;
	}

	log_debug("_adec_instance->pDecReadBuff SUCCESS\n");
	_adec_instance->iShmMemIndex =
		share_cyclebuffer_init(_adec_instance->adec_attrs.u32BufSize * DEFAULT_MAX_FRM_SIZE, 0);

	if (_adec_instance->iShmMemIndex < 0) {
		log_error("query share buffer failed.\n");
		free(_adec_instance->DecBuff);
		free(_adec_instance->pDecReadBuff);
		return CVI_ERR_AO_NOMEM;
	}

	PAYLOAD_TYPE_E eInputType = gstAdecInstance[AdChn].adec_attrs.enType;

	if (eInputType == PT_G711A) {
		/* g-series speech codec*/
		_adec_instance->eACodecType = AUD_CODEC_G711_ALAW;
	} else if (eInputType == PT_G711U) {
		/* g-series speech codec*/
		_adec_instance->eACodecType = AUD_CODEC_G711_MLAW;
	} else if (eInputType == PT_G726) {
		/* g-series speech codec*/
		_adec_instance->eACodecType = AUD_CODEC_G726;
	} else if (eInputType == PT_ADPCMA) {
		/* ADPCM speech codec*/
		_adec_instance->eACodecType = AUD_CODEC_ADPCM_IMA;
	} else if ((eInputType == PT_DVI4_8K) || (eInputType == PT_DVI4_16K) ||
		   (eInputType == PT_DVI4_3) || (eInputType == PT_DVI4_4)) {
		/* ADPCM speech codec*/
		_adec_instance->eACodecType = AUD_CODEC_ADPCM_DVI4;
	} else if (eInputType == PT_AAC) {
		/* g-series speech codec*/
		log_debug("[%s]PT_AAC\n", __func__);
		_adec_instance->eACodecType = AUD_CODEC_AAC;
		adec_end_flag = CVI_FALSE;
		//totaldecbytes = 0;
	} else {
		log_error("Not support transcode codec[%d][%d]\n",
			  (int)eInputType, (int)pstAttr->enType);
		free(_adec_instance->DecBuff);
		free(_adec_instance->pDecReadBuff);
		share_cyclebuffer_destroy(_adec_instance->iShmMemIndex);
		return CVI_ERR_ADEC_NOT_SUPPORT;
	}

	/* decoder setting to fit cvi_transcode_interface--start */
	/* setup default bitrate to 16bit */
	_adec_instance->cviAdecConfig.bitrate = Rate16kBits;
	_adec_instance->cviAdecConfig.channel_num = pstAttr->s32ChannelNums;
	_adec_instance->cviAdecConfig.sample_rate = _adec_instance->adec_attrs.s32Sample_rate;

	switch (pstAttr->enType) {
	case PT_G711A:
	case PT_G711U:
		break;

	case PT_G726: {
		AENC_ATTR_G726_S *pValue = (AENC_ATTR_G726_S *)pstAttr->pValue;

		if ((pValue->enG726bps == MEDIA_G726_16K) || (pValue->enG726bps == G726_16K)) {
			log_debug("G726 bit 16k\n");
			_adec_instance->cviAdecConfig.bitrate = Rate16kBits;
		}

		if ((pValue->enG726bps == MEDIA_G726_24K) || (pValue->enG726bps == G726_24K)) {
			log_debug("G726 bit 24k\n");
			_adec_instance->cviAdecConfig.bitrate = Rate24kBits;
		}

		if ((pValue->enG726bps == G726_32K) || (pValue->enG726bps == MEDIA_G726_32K)) {
			log_debug("G726 bit 32k\n");
			_adec_instance->cviAdecConfig.bitrate = Rate32kBits;
		}

		if ((pValue->enG726bps == G726_40K) || (pValue->enG726bps == MEDIA_G726_40K)) {
			log_debug("G726 bit 40k\n");
			_adec_instance->cviAdecConfig.bitrate = Rate40kBits;
		}

		break;
	}

	case PT_AAC: {
		//check if AAC encoder / decoder handle exist
		if (gstpAacDecInstance != NULL &&  gAACdec.pfnOpenDecoder != NULL)
			log_debug("create AAC decoder channel\n");
		else {
			log_debug("create AAC decoder channel failure\n");
			return CVI_FAILURE;
		}

		break;
	}

	case PT_ADPCMA:
	case PT_D_VDVI:
	case PT_DVI4_8K:
	case PT_DVI4_16K:
		break;

	default:
		log_error("Not support encode type\n");
		break;
	}

	/* init the decoder handle */
	if (pstAttr->enType != PT_AAC) {
		log_debug("aud dec handle config chn_num[%d] type[%d] b_rate[%d] sr[%d]\n",
			  _adec_instance->cviAdecConfig.channel_num,
			  _adec_instance->eACodecType,
			  _adec_instance->cviAdecConfig.bitrate,
			  _adec_instance->cviAdecConfig.sample_rate);

		_adec_instance->AdecHandle = CVI_AUDIO_Transcode_Init(NULL,
					     _adec_instance->eACodecType,
					     &_adec_instance->cviAdecConfig);

		if (_adec_instance->AdecHandle == CVI_NULL) {
			log_error("transcode INIT setting error ....\n");
			free(_adec_instance->DecBuff);
			free(_adec_instance->pDecReadBuff);
			share_cyclebuffer_destroy(_adec_instance->iShmMemIndex);
			return CVI_FAILURE;
		}
	} else { //aac

		CVI_S32 s32Ret;
		ST_ADEC_ATTR_AAC_S	*stAdecAac = (ST_ADEC_ATTR_AAC_S *)pstAttr->pValue;
		ST_ADEC_ATTR_AAC_S	aacdec_config;

		if (_adec_instance->adec_attrs.s32Sample_rate != 0)
			aacdec_config.enSmpRate = _adec_instance->adec_attrs.s32Sample_rate;
		else
			log_info("[TODO]Get sample rate from AO, adec not set attr\n");

		if (_adec_instance->adec_attrs.s32ChannelNums != 0)
			aacdec_config.enSoundMode = _adec_instance->adec_attrs.s32ChannelNums - 1;
		else
			log_info("[TODO]Get sample rate from AO, adec not set attr\n");

		aacdec_config.enTransType = stAdecAac->enTransType;

		log_info("[%s][%d]rate[%d] soundmode[%d]transtype[%d]\n", __func__, __LINE__,
			 aacdec_config.enSmpRate,
			 aacdec_config.enSoundMode,
			 aacdec_config.enTransType);

		//open openaacdecoder
		s32Ret = gAACdec.pfnOpenDecoder((CVI_VOID *)(&aacdec_config),
						(CVI_VOID **)(&_adec_instance->AdecHandle));
		if (s32Ret != CVI_SUCCESS) {
			log_error("AAC decoder open failure\n");
			return CVI_FAILURE;
		}
			log_info("AAC decoder open success\n");
			log_info("decoder handle[0x%x]\n", _adec_instance->AdecHandle);


		_adec_instance->pst_aac_dec = (ST_AAC_DEC *)malloc(sizeof(ST_AAC_DEC));
		if (!_adec_instance->pst_aac_dec) {
			log_error("malloc error\n");
			return CVI_FAILURE;
		}

		memset(_adec_instance->pst_aac_dec, 0, sizeof(ST_AAC_DEC));

		_adec_instance->pst_aac_dec->sample_rate = 0;
		_adec_instance->pst_aac_dec->channel = 0;
		_adec_instance->pst_aac_dec->frame_byte = 0;
		_adec_instance->pst_aac_dec->aac_remainbytes = 0;
		_adec_instance->pst_aac_dec->aac_bufferbyte = CVI_AAC_BUF_SIZE;
		_adec_instance->pst_aac_dec->aac_inputbuffer = (unsigned char *)malloc(CVI_AAC_BUF_SIZE);
		_adec_instance->pst_aac_dec->aac_dec_after_buf = (short *)malloc(CVI_AAC_BUF_SIZE);

		if (!_adec_instance->pst_aac_dec->aac_inputbuffer
			|| !_adec_instance->pst_aac_dec->aac_dec_after_buf) {
			log_error("malloc error, aac_inputbuffer[%p], aac_dec_after_buf[%p]\n",
					_adec_instance->pst_aac_dec->aac_inputbuffer,
					_adec_instance->pst_aac_dec->aac_dec_after_buf);
			return CVI_FAILURE;
		}
		memset(_adec_instance->pst_aac_dec->aac_dec_after_buf, 0, CVI_AAC_BUF_SIZE);
	}
	_adec_instance->bEnableAdec = CVI_TRUE;

	return CVI_SUCCESS;
}


CVI_S32 CVI_ADEC_DestroyChn(ADEC_CHN AdChn)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_adec_destroy_chn(AdChn);
	}
#endif
	if (CHECK_ADEC_DEVID_VALID(AdChn)) {
		log_error("\n");
		return CVI_ERR_ADEC_INVALID_DEVID;
	}

	ST_ADEC_INSTANCE *_adec_instance = &gstAdecInstance[AdChn];
	CVI_S32 s32Ret = CVI_FAILURE;

	if (!_adec_instance->bEnableAdec) {
		return CVI_SUCCESS;
	}
	_adec_instance->bEnableAdec = CVI_FALSE;
	SAFE_FREE_BUF(_adec_instance->DecBuff);
	SAFE_FREE_BUF(_adec_instance->pDecReadBuff);

	if (_adec_instance->eACodecType == AUD_CODEC_AAC) {
		if ((gstpAacDecInstance != NULL) && (gstpAacDecInstance->pstAACDecoder != NULL)) {
			if (_adec_instance->AdecHandle)
				s32Ret = gAACdec.pfnCloseDecoder(_adec_instance->AdecHandle);
		}
		SAFE_FREE_BUF(_adec_instance->pst_aac_dec->aac_inputbuffer);
		SAFE_FREE_BUF(_adec_instance->pst_aac_dec->aac_dec_after_buf);
		SAFE_FREE_BUF(_adec_instance->pst_aac_dec);

	} else {
		s32Ret = CVI_AUDIO_Transcode_DeInit(_adec_instance->AdecHandle,
						    _adec_instance->eACodecType);
	}

	share_cyclebuffer_destroy(_adec_instance->iShmMemIndex);
	memset(&gstAdecInstance[AdChn], 0, sizeof(ST_ADEC_INSTANCE));
	return s32Ret;
}

CVI_S32 CVI_ADEC_SendStream(ADEC_CHN AdChn, const AUDIO_STREAM_S *pstStream,
			    CVI_BOOL bBlock)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		return rpc_client_aud_adec_send_stream(AdChn, pstStream, bBlock);
	}
#endif
	CVI_S32 s32Ret;
	CVI_S32 s32OutLenBytes = 0;
	CVI_S32 s32ShareWriteLen = 0;

	if (CHECK_ADEC_DEVID_VALID(AdChn)) {
		log_error("\n");
		return CVI_ERR_ADEC_INVALID_DEVID;
	}
	ST_ADEC_INSTANCE *_adec_instance = &gstAdecInstance[AdChn];

	if (!_adec_instance->bEnableAdec) {
		log_error("chn:%d is not valid.\n", AdChn);
		return CVI_ERR_ADEC_NOT_CONFIG;
	}

	if (pstStream->u32Len == 0) {
		log_error("chn:%d send in length is 0\n", AdChn);
		return CVI_FAILURE;
	}

	_check_and_dump("/tmp/dump_adec_in", (char *)pstStream->pStream, pstStream->u32Len);

	_adec_instance->decInbyte = pstStream->u32Len;

	if (_adec_instance->eACodecType == AUD_CODEC_AAC) {
		if (gAACdec.pfnDecodeFrm == NULL) {
			log_error("please call CVI_MPI_ADEC_AacInit first\n");
			return CVI_FAILURE;
		}
		s32Ret = _cvi_aac_decode(AdChn, _adec_instance->pst_aac_dec,
					 (unsigned char *)pstStream->pStream, (short *)_adec_instance->DecBuff,
					 pstStream->u32Len, &s32OutLenBytes);
		if (s32Ret != CVI_SUCCESS) {
			log_error("decode error,ret %d\n", s32Ret);
			return s32Ret;
		}

	} else {
		s32Ret = CVI_AUDIO_Decode(_adec_instance->AdecHandle,
					  _adec_instance->eACodecType,
					  (CVI_VOID *)pstStream->pStream,
					  (CVI_VOID *)_adec_instance->DecBuff,
					  pstStream->u32Len,
					  &s32OutLenBytes);//return in bytes

		if (s32Ret != CVI_SUCCESS) {
			log_error("decode error,ret %#x\n", s32Ret);
			return s32Ret;
		}
	}

	if (s32OutLenBytes > 0) {
		_check_and_dump("/tmp/dump_adec_out", (char *)_adec_instance->DecBuff, s32OutLenBytes);
		_adec_instance->u64TimeStamp = pstStream->u64TimeStamp;

		if (!_adec_instance->stBindinfo.bBind) {
			CVI_S32 s32Size = share_cyclebuffer_total_buf_size(_adec_instance->iShmMemIndex);

			if (s32Size < s32OutLenBytes) {
				log_error("dec buf size %d is too small,output size:%d\n", s32Size, s32OutLenBytes);
				return -1;
			}
			s32ShareWriteLen = share_cyclebuffer_client_write(_adec_instance->iShmMemIndex,
					_adec_instance->DecBuff,
					s32OutLenBytes,
					bBlock ? -1 : 0);
			if (s32ShareWriteLen > 0)
				s32Ret = CVI_SUCCESS;
		} else {
			AUDIO_FRAME_S stOutFrame;

			stOutFrame.u64VirAddr[0] = (CVI_U8 *)_adec_instance->DecBuff;
			stOutFrame.u32Len = s32OutLenBytes/
					(_adec_instance->adec_attrs.s32ChannelNums * DEFAULT_BYTES_PER_SAMPLE);
			stOutFrame.u64TimeStamp = _adec_instance->u64TimeStamp;
			stOutFrame.enSoundmode =  _adec_instance->cviAdecConfig.channel_num - 1;
			stOutFrame.enBitwidth = AUDIO_BIT_WIDTH_16;
			AUD_DEV_CHN_INFO *pdstinfo = &_adec_instance->stBindinfo.dstinfo;
			ST_AO_INSTANCE *pstAoInstance = &gstAoInstance[pdstinfo->s32DevId];
			CVI_ST_AUD_TRACK_INFO_PTR pstTrackInfo =
				pstAoInstance->pastTrackInfo[pdstinfo->s32ChnId];
			if (pstTrackInfo && !pstTrackInfo->stBindinfo.bBind) {
				log_error("track %p not bind?\n", pstTrackInfo);
				return CVI_ERR_ADEC_NOT_CONFIG;
			}
			s32Ret = _audio_sendframe(pstTrackInfo, &stOutFrame, bBlock ? -1 : 1000);
		}
		_adec_instance->decOutbyte = s32OutLenBytes;
		_adec_instance->s32DecodedDataBytes += s32OutLenBytes;
	}

	return s32Ret;
}


CVI_S32 CVI_ADEC_GetFrame(ADEC_CHN AdChn, AUDIO_FRAME_INFO_S *pstFrmInfo,
			  CVI_BOOL bBlock)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		return rpc_client_aud_adec_get_frame(AdChn, pstFrmInfo, bBlock);
	}
#endif

	CVI_S32 s32FrameSize = 0;
	int timeout = 0;
	bBlock = false;
	if (CHECK_ADEC_DEVID_VALID(AdChn)) {
		log_error("\n");
		return CVI_ERR_ADEC_INVALID_DEVID;
	}
	ST_ADEC_INSTANCE *_adec_instance = &gstAdecInstance[AdChn];

	if (!_adec_instance->bEnableAdec) {
		log_error("chn:%d is not valid.\n", AdChn);
		return CVI_ERR_ADEC_NOT_CONFIG;
	}
	if (_adec_instance->stBindinfo.bBind) {
		log_info("current %d ch already bind.\n", AdChn);
		return CVI_ERR_ADEC_NOT_SUPPORT;
	}

	do {
		s32FrameSize = share_cyclebuffer_frameready_size(_adec_instance->iShmMemIndex);
		if (s32FrameSize) {
			if (s32FrameSize > (int)(_adec_instance->adec_attrs.u32BufSize * DEFAULT_MAX_FRM_SIZE)) {
				log_error("databyte[%d] > bufferbyte[%d]\n", s32FrameSize,
					_adec_instance->adec_attrs.u32BufSize * DEFAULT_MAX_FRM_SIZE);
				return CVI_ERR_ADEC_NOMEM;
			}
			share_cyclebuffer_server_read(_adec_instance->iShmMemIndex,
						      _adec_instance->pDecReadBuff, s32FrameSize);
			break;
		}

		timeout++;
		usleep(1000);
		if (timeout > 5000) {
			log_error("no data,timeout exit.\n");
			break;
		}

	} while (bBlock);

	//step 3:update pstFrmInfo/pstFrame
	pstFrmInfo->pstFrame->u64VirAddr[0] = (CVI_U8 *)_adec_instance->pDecReadBuff;
	pstFrmInfo->pstFrame->u32Len =
		(s32FrameSize / _adec_instance->adec_attrs.s32ChannelNums) / DEFAULT_BYTES_PER_SAMPLE;

	pstFrmInfo->pstFrame->u64TimeStamp = _adec_instance->u64TimeStamp;
	pstFrmInfo->pstFrame->enSoundmode =  _adec_instance->cviAdecConfig.channel_num - 1;
	pstFrmInfo->pstFrame->enBitwidth = AUDIO_BIT_WIDTH_16;
	return CVI_SUCCESS;
}

CVI_S32 CVI_ADEC_SendEndOfStream(ADEC_CHN AdChn, CVI_BOOL bInstant)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_adec_send_end_of_stream(AdChn, bInstant);
	}
#endif
	UNUSED_REF(AdChn);
	UNUSED_REF(bInstant);
	return CVI_SUCCESS;
}

CVI_S32 CVI_ADEC_ReleaseFrame(ADEC_CHN AdChn,
			      const AUDIO_FRAME_INFO_S *pstFrmInfo)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_adec_release_frame(AdChn, pstFrmInfo);
	}
#endif
	UNUSED_REF(AdChn);
	UNUSED_REF(pstFrmInfo);
	return CVI_SUCCESS;
}

CVI_S32 CVI_ADEC_ClearChnBuf(ADEC_CHN AdChn)
{
	if (CHECK_ADEC_DEVID_VALID(AdChn)) {
		log_error("\n");
		return CVI_ERR_ADEC_INVALID_DEVID;
	}
	ST_ADEC_INSTANCE *_adec_instance = &gstAdecInstance[AdChn];

	if (!_adec_instance->bEnableAdec) {
		log_error("chn:%d is not valid.\n", AdChn);
		return CVI_ERR_ADEC_NOT_CONFIG;
	}
	if (!_adec_instance->stBindinfo.bBind) {
		AUD_DEV_CHN_INFO *pdstinfo = &_adec_instance->stBindinfo.dstinfo;
		ST_AO_INSTANCE *pstAoInstance = &gstAoInstance[pdstinfo->s32DevId];
		CVI_ST_AUD_TRACK_INFO_PTR pstTrackInfo =
			pstAoInstance->pastTrackInfo[pdstinfo->s32ChnId];
		if (pstTrackInfo && !pstTrackInfo->stBindinfo.bBind) {
			log_error("track %p not bind?\n", pstTrackInfo);
			return CVI_ERR_ADEC_NOT_CONFIG;
		}
		return share_cyclebuffer_clear(pstTrackInfo->iShmMemIndex);
	}

	return share_cyclebuffer_clear(_adec_instance->iShmMemIndex);
}

CVI_S32 CVI_ADEC_RegisterDecoder(CVI_S32 *ps32Handle,
				 const ADEC_DECODER_S *pstDecoder)
{
	/* AAC decode needed, do not need this now */
	UNUSED_REF(ps32Handle);
	UNUSED_REF(pstDecoder);
	log_error("Pleae use CVI_ADEC_RegisterExternalDecoder instead\n");
	return CVI_ERR_ADEC_NOT_SUPPORT;
}
CVI_S32 CVI_ADEC_UnRegisterDecoder(CVI_S32 s32Handle)
{
	/* AAC decode needed, do not need this now */
	UNUSED_REF(s32Handle);
	log_error("Please use CVI_ADEC_UnRegisterExternalDecoder instead\n");
	return CVI_ERR_ADEC_NOT_SUPPORT;
}


#if 1 /* sample function start */



//-----support AAC inside cvi_audio----start


CVI_S32 CVI_ADEC_UnRegisterExternalDecoder(CVI_S32 s32Handle)
{
	//CVI_S32 *p32handle = s32Handle;
	UNUSED_REF(s32Handle);
	if (gstpAacDecInstance != NULL) {
		free(gstpAacDecInstance);
		gstpAacDecInstance = NULL;
	}
	return CVI_SUCCESS;
}


CVI_S32 CVI_ADEC_RegisterExternalDecoder(CVI_S32 *ps32Handle,
		const ADEC_DECODER_S *pstDecoder)
{
	if (gstpAacDecInstance == NULL)
		gstpAacDecInstance = calloc(1, sizeof(ST_AAC_DEC_INST));

	UNUSED_REF(ps32Handle);
	ADEC_DECODER_S *plocal = (ADEC_DECODER_S *)pstDecoder;

	gstpAacDecInstance->pstAACDecoder = pstDecoder;
	memcpy(&gAACdec.aszName, &plocal->aszName, sizeof(CVI_CHAR) * 17);
	gAACdec.enType = plocal->enType;
	gAACdec.pfnCloseDecoder = plocal->pfnCloseDecoder;
	gAACdec.pfnDecodeFrm = plocal->pfnDecodeFrm;
	gAACdec.pfnGetFrmInfo = plocal->pfnGetFrmInfo;
	gAACdec.pfnOpenDecoder = plocal->pfnOpenDecoder;
	gAACdec.pfnResetDecoder = plocal->pfnResetDecoder;
	//printf("[%s][%d]\n", __func__, __LINE__);
	//sleep(1);

	return CVI_SUCCESS;
}


//-----support AAC inside cvi_audio----end
#endif

