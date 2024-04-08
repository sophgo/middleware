#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include "../fdkaac/libAACdec/include/aacdecoder_lib.h"
#include <string.h>
#include "cvi_aac_dec.h"
//#include "cvi_audio.h"
#include "alog.h"

#define CVI_FREE_BUF(OBJ) {if (NULL != OBJ) {free(OBJ); OBJ = NULL; } }
#define CVI_DECODE_AAC_TYPE AACDEC_ADTS
#define _VERSION_TAG_ "cvi_aac_decode_2021_6_22"

#define CVI_AAC_BUF_SIZE (1024 * 10)

#define CVI_AAC_CB_STATUS_START 0x2
#define CVI_AAC_CB_STATUS_SEND_DATA 0x3
#define CVI_AAC_CB_CHANGE_INFO 0x4

//int aac_decplay_level = 1;


#define ERR_PRINTF(fmt, args...) \
	do { \
		if (cviaud_dbg > 0) \
			fprintf(stderr, "[cvi_AacPlay][err][%s][%d] "fmt, __func__, __LINE__, ##args);\
	} while (0)

#define DBG_PRINTF(fmt, args...) \
	do { \
		if (cviaud_dbg > 1) \
			fprintf(stderr, "[cvi_AacPlay][info][%s][%d] "fmt, __func__, __LINE__, ##args);\
	} while (0)

#define DUM_PRINTF(fmt, args...) \
	do { \
		if (cviaud_dbg > 2) \
			fprintf(stderr, "[cvi_AacPlay] "fmt, ##args);\
	} while (0)


typedef struct _st_aac_dec_handler {
	HANDLE_AACDECODER dec_handle;
	int sample_rate;
	int channel;
	int frame_byte;//一个ADTS帧的长度包括ADTS头和AAC原始流.

	CStreamInfo *theInfo;
	pDecode_Cb pCbDecode;
	ST_AAC_DEC_INFO aac_info;

	unsigned char *aac_inputbuffer;
	unsigned  long long aac_remainbytes;
	unsigned  long long aac_bufferbyte;

	INT_PCM *aac_dec_after_buf;
} st_aac_dec_handler;

static void dump_audiodata(char *filename, char *buf, unsigned int len)
{
	FILE *fp;

	if (filename == NULL) {
		return;
	}

	fp = fopen(filename, "ab+");
	fwrite(buf, 1, len, fp);
	fclose(fp);

}

static void _check_and_dump(const char *filename, char *buf,
			    unsigned int sizebytes)
{
	if (access(filename, F_OK) == 0) {
		char newfilename[128] = {0};

		snprintf(newfilename, 128, "%s.pcm", filename);

		dump_audiodata((char *)newfilename,
			       (char *)buf,
			       (unsigned int)sizebytes);
	}
}

int get_aac_sample_rate(int sampling_frequency_index)
{
	int sample_rate;

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
		ERR_PRINTF("sampling_frequency_index unknown\n");
		break;
	}
	return sample_rate;
}

int update_Cb_info(void *paac_handle, INT_PCM *pOutBuf)
{
	st_aac_dec_handler *paac_dec_handle = (st_aac_dec_handler *)paac_handle;
	int frame_size;

	if (paac_handle == NULL) {
		ERR_PRINTF("paac_handle is null\n");
		return -1;
	}

	paac_dec_handle->aac_info.channel_num = paac_dec_handle->channel;
	paac_dec_handle->aac_info.sample_rate = paac_dec_handle->sample_rate;
	paac_dec_handle->aac_info.frame_byte = paac_dec_handle->frame_byte;

	paac_dec_handle->theInfo = aacDecoder_GetStreamInfo(
					   paac_dec_handle->dec_handle);
	if (paac_dec_handle->theInfo == NULL) {
		ERR_PRINTF("Get stream info null info\n");
		return -1;
	}

	frame_size = paac_dec_handle->theInfo->frameSize *
		     paac_dec_handle->theInfo->numChannels;
	DBG_PRINTF("paac_dec_handle->theInfo.frameSize = %d,paac_dec_handle->theInfo.numChannels = %d\n",
		   paac_dec_handle->theInfo->frameSize, paac_dec_handle->theInfo->numChannels);

	paac_dec_handle->pCbDecode((void *)paac_dec_handle, &paac_dec_handle->aac_info,
				   (char *)pOutBuf, frame_size * 2);

	return frame_size;
}


void *CVI_AAC_Decode_Init(AACDECTransportType enTranType, FILE *intput_file_fd)
{
	st_aac_dec_handler *paac_dec_handle;

	paac_dec_handle = (st_aac_dec_handler *)malloc(sizeof(st_aac_dec_handler));
	memset(paac_dec_handle, 0, sizeof(st_aac_dec_handler));
	cviAudioGetDbgMask(&cviaud_dbg);

	if (enTranType == AACDEC_ADTS) {
		printf("[%s][%s]\n", __func__, _VERSION_TAG_);
		paac_dec_handle->dec_handle = aacDecoder_Open(TT_MP4_ADTS, 1);
		DBG_PRINTF("ADTS header decoder set\n");
	} else if (enTranType == AACDEC_LOAS)
		paac_dec_handle->dec_handle = aacDecoder_Open(TT_MP4_LOAS, 1);
	else if (enTranType == AACDEC_LATM_MCP1)
		paac_dec_handle->dec_handle = aacDecoder_Open(TT_MP4_LATM_MCP1, 1);
	else {
		DBG_PRINTF("[Warning]set TransportType[%d] to RawMode\n", (int)enTranType);
		paac_dec_handle->dec_handle = aacDecoder_Open(TT_MP4_RAW, 1);
		DBG_PRINTF("set to TT_MP4_RAW\n");

	}
	if (paac_dec_handle->dec_handle == NULL) {
		ERR_PRINTF("dec handle is NULL\n");
		return NULL;
	}

	paac_dec_handle->sample_rate = 0;
	paac_dec_handle->channel = 0;
	paac_dec_handle->frame_byte = 0;
	paac_dec_handle->theInfo = (CStreamInfo *)malloc(sizeof(CStreamInfo));
	paac_dec_handle->aac_inputbuffer = (unsigned char *)malloc(CVI_AAC_BUF_SIZE);
	paac_dec_handle->aac_bufferbyte = CVI_AAC_BUF_SIZE;

	paac_dec_handle->aac_dec_after_buf = (INT_PCM *)malloc(1024 * 8);
	memset(paac_dec_handle->aac_dec_after_buf, 0, 1024 * 8);
	paac_dec_handle->aac_remainbytes = 0;



	return (void *)paac_dec_handle;
}

int _aac_decode_one_frame(void *paac_handle, unsigned char **ppInputBuf,
			  INT_PCM *pOutBuf, int *pbytesLeft, int *frame_size)
{
	st_aac_dec_handler *paac_dec_handle = (st_aac_dec_handler *)paac_handle;

	unsigned char *inBufferArray[1];
	unsigned int inBuffReaded[1];
	unsigned int byte_left;
	AAC_DECODER_ERROR err = AAC_DEC_OK;

	inBufferArray[0] = (unsigned char *)(*ppInputBuf);
	inBuffReaded[0] = *pbytesLeft;
	byte_left =  *pbytesLeft;

	if (paac_handle == NULL) {
		ERR_PRINTF("paac_handle is null\n");
		return -1;
	}

	err = aacDecoder_Fill(paac_dec_handle->dec_handle, inBufferArray, inBuffReaded,
			      &byte_left);
	if (err != AAC_DEC_OK) {
		ERR_PRINTF("decode fill faile\n");
		ERR_PRINTF("err = [0x%x]\n", err);
	} else {
		if (byte_left != 0)
			ERR_PRINTF("[xxxx]aacDecoder_Fill after byte left[%d]\n", byte_left);
	}

	err = aacDecoder_DecodeFrame(paac_dec_handle->dec_handle, pOutBuf, 8 * 2048,
				     0); //output_size = 8*sizeof(INT_PCM)*2048;8*2048frame
	if (err != AAC_DEC_OK) {
		ERR_PRINTF("aac decode frame faile\n");
		ERR_PRINTF("err = [0x%x]\n", err);
		if (err == AAC_DEC_NOT_ENOUGH_BITS)
			ERR_PRINTF("AAC_DEC_NOT_ENOUGH_BITS\n");
		if (err == AAC_DEC_TRANSPORT_ERROR)
			ERR_PRINTF("AAC_DEC_TRANSPORT_ERROR\n");

		return err;
	}

	*frame_size = update_Cb_info(paac_dec_handle, pOutBuf);
	if (*frame_size == -1) {
		ERR_PRINTF("update_Cb_info faile\n");
		return -1;
	}

	*pbytesLeft = byte_left;
	return 0;
}


int CVI_AAC_Decode(void *paac_handle, unsigned char *pInputBuf, INT_PCM *pOutBuf, int byte_left, int *frame_size)
{
	st_aac_dec_handler *paac_dec_handle = (st_aac_dec_handler *)paac_handle;

	int sampling_frequency_index;
	AAC_DECODER_ERROR err = AAC_DEC_OK;
	int s32TotalSizeBytes = byte_left;
	int poutbuf_sample = 0;

	if (paac_handle == NULL) {
		ERR_PRINTF("paac_handle is null\n");
		return -1;
	}

	if (byte_left > (int) paac_dec_handle->aac_bufferbyte) {
		ERR_PRINTF("byte_left[%d] > aac_bufferbyte[%d]\n", byte_left, (int) paac_dec_handle->aac_bufferbyte);
		return -1;
	}
	if (paac_dec_handle->aac_remainbytes != 0) {
		if ((paac_dec_handle->aac_remainbytes + byte_left) >
			paac_dec_handle->aac_bufferbyte) {

			ERR_PRINTF("remain[%d] + input[%d] > bufsize[%d]",
				(int)paac_dec_handle->aac_remainbytes,
				byte_left,
				(int)paac_dec_handle->aac_bufferbyte);
			ERR_PRINTF("force flush the buffer\n");
			memset(paac_dec_handle->aac_inputbuffer, 0, paac_dec_handle->aac_bufferbyte);
			memcpy(paac_dec_handle->aac_inputbuffer, pInputBuf, byte_left);
			s32TotalSizeBytes = byte_left;
			return -1;
		}

		memcpy(paac_dec_handle->aac_inputbuffer + paac_dec_handle->aac_remainbytes, pInputBuf, byte_left);
		s32TotalSizeBytes = byte_left + paac_dec_handle->aac_remainbytes;
		DBG_PRINTF("has data s32TotalSizeBytes = %d\n", s32TotalSizeBytes);

	} else {
		memcpy(paac_dec_handle->aac_inputbuffer, pInputBuf, byte_left);
		s32TotalSizeBytes = byte_left;
		DBG_PRINTF("[%s][%d] inputlen[%d]\n", __func__, __LINE__, byte_left);
	}

	paac_dec_handle->aac_remainbytes = s32TotalSizeBytes;

while (1) {

	if (paac_dec_handle->aac_remainbytes > 7) {

		if ((paac_dec_handle->aac_inputbuffer[0] == 0xff) &&
				((paac_dec_handle->aac_inputbuffer[1] & 0xf0) == 0xf0)) {//adts

			sampling_frequency_index = (paac_dec_handle->aac_inputbuffer[2] & 0x3C) >> 2;
			paac_dec_handle->sample_rate = get_aac_sample_rate(sampling_frequency_index);
			paac_dec_handle->channel = ((paac_dec_handle->aac_inputbuffer[2] & 0x01) << 2) |
				 ((paac_dec_handle->aac_inputbuffer[3] & 0xC0) >> 6);

			paac_dec_handle->frame_byte = ((paac_dec_handle->aac_inputbuffer[3] & 0x03) << 11) |
				(paac_dec_handle->aac_inputbuffer[4] << 3) | (paac_dec_handle->aac_inputbuffer[5] >> 5);

			DBG_PRINTF("packer_byte = %d,channel = %d,sample_rate = %d\n",
				paac_dec_handle->frame_byte,
				paac_dec_handle->channel,
				paac_dec_handle->sample_rate);

			if (paac_dec_handle->aac_remainbytes < paac_dec_handle->frame_byte) {
				DBG_PRINTF(" ADTS_byte(7) <remainbytes < aac frame_byte\n");
				break;
			}

			int out_size = 0;
			int vail = paac_dec_handle->frame_byte;

			_check_and_dump("/tmp/aacplay_decode_before",
				(char *)paac_dec_handle->aac_inputbuffer,
				(unsigned int)vail);

			err = _aac_decode_one_frame(paac_dec_handle,
				&paac_dec_handle->aac_inputbuffer,
				paac_dec_handle->aac_dec_after_buf,
				&vail, &out_size);

			if (err != 0) {
				ERR_PRINTF("aac one frame decode faile\n");
				return -1;
			}
			_check_and_dump("/tmp/aacplay_decode_after",
				(char *)paac_dec_handle->aac_dec_after_buf,
				(unsigned int)out_size*2);

			paac_dec_handle->aac_remainbytes = paac_dec_handle->aac_remainbytes -
				paac_dec_handle->frame_byte;

			memcpy(paac_dec_handle->aac_inputbuffer,
				paac_dec_handle->aac_inputbuffer + paac_dec_handle->frame_byte,
				paac_dec_handle->aac_remainbytes);

			memcpy(pOutBuf + poutbuf_sample, paac_dec_handle->aac_dec_after_buf, out_size * 2);
			poutbuf_sample += out_size;
			*frame_size = poutbuf_sample;

		} else {
			ERR_PRINTF("aac_inputbuffer is not ADTS\n");
			break;
		}

	} else {
		DBG_PRINTF("remainbytes = < ADTS_byte(7)\n");
		break;
	}

}

	return 0;
}

int CVI_AAC_Decode_Dinit(void *paac_handle)
{
	st_aac_dec_handler *paac_dec_handle = (st_aac_dec_handler *)paac_handle;

	if (paac_dec_handle) {
		aacDecoder_Close(paac_dec_handle->dec_handle);
	}
	return 0;
}

static int _aac_dec_callback(void *paac_handle, ST_AAC_DEC_INFO *paac_info,
			     char *pBuff, int byte) //byte
{
	FILE *fp_local;

	if (paac_handle == NULL) {
		ERR_PRINTF("paac_handle is null\n");
		return -1;
	}

	fp_local = fopen("aac_dec_cbDump.pcm", "ab+");
	fwrite(pBuff, 1, byte, fp_local);
	fclose(fp_local);
	return 0;

}

int CVI_AAC_Decode_InstallCb(void *paac_handle, pDecode_Cb pCbFunc)
{
	st_aac_dec_handler *paac_dec_handle = (st_aac_dec_handler *)paac_handle;

	if (paac_handle == NULL) {
		ERR_PRINTF("paac_handle is null\n");
		return -1;
	}
	paac_dec_handle->pCbDecode = pCbFunc;
	paac_dec_handle->aac_info.cbState = CVI_AAC_CB_STATUS_START;
	return 0;
}



