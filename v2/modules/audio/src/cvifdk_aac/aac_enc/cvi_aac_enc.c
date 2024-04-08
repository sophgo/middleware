#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "../fdkaac/libAACenc/include/aacenc_lib.h"
#include "cvi_aac_enc.h"
#include "cvi_audio.h"

#define CVI_FREE_BUF(OBJ) {if (NULL != OBJ) {free(OBJ); OBJ = NULL; } }
#define CVI_DECODE_AAC_TYPE AACDEC_ADTS
#define DEFAULT_BYTES_PER_SAMPLE 2

typedef struct _st_aac_dec_handler {
	HANDLE_AACENCODER enc_handle;
	int sample_rate;
	int channel;
	int frame_size;

	pEncode_Cb pCbEncode;
	ST_AAC_ENC_INFO aac_info;
	AAC_ENC_INFO aac_enc;
	int suggestframe_len;

	short *aac_inputbuffer;
	unsigned  long long aac_remainbytes;
	unsigned  long long aac_bufferbyte;

} st_aac_enc_handler;

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


void *CVI_AAC_Encode_Init(AAC_ENC_INFO aac_enc)
{

	int afterburner = 1;//1:high quality 0:medium, low quality
	int eld_sbr = 0;
	int variable_bitrate = 0;
	CHANNEL_MODE mode;
	AACENC_InfoStruct info = { 0 };//->frameLength

	st_aac_enc_handler *paac_enc_handle;

	paac_enc_handle = (st_aac_enc_handler *)malloc(sizeof(st_aac_enc_handler));
	memset(paac_enc_handle, 0, sizeof(paac_enc_handle));
	memcpy(&paac_enc_handle->aac_enc, &aac_enc, sizeof(AAC_ENC_INFO));

	switch (aac_enc.channel_num) {
	case 1:
		mode = MODE_1;
		break;
	case 2:
		mode = MODE_2;
		break;
	case 3:
		mode = MODE_1_2;
		break;
	case 4:
		mode = MODE_1_2_1;
		break;
	case 5:
		mode = MODE_1_2_2;
		break;
	case 6:
		mode = MODE_1_2_2_1;
		break;
	default:
		ERR_PRINTF("[Error]Unsupported WAV channels %d\n", aac_enc.channel_num);
		return (-1);
	}

	printf("[%s][%s]\n", __func__, _VERSION_TAG_);
	if (aacEncOpen(&paac_enc_handle->enc_handle, 0,
		       aac_enc.channel_num) != AACENC_OK) {
		ERR_PRINTF("Unable to open encoder\n");
		return NULL;
	}
	if (aacEncoder_SetParam(paac_enc_handle->enc_handle, AACENC_AOT,
				CodecArray[aac_enc.aacformat]) != AACENC_OK) {
		ERR_PRINTF("Unable to set the AOT\n");
		return NULL;
	}

	if (CodecArray[aac_enc.aacformat] == AACELD_ENC_AOT && eld_sbr) {
		if (aacEncoder_SetParam(paac_enc_handle->enc_handle, AACENC_SBR_MODE,
					1) != AACENC_OK) {
			ERR_PRINTF("Unable to set SBR mode for ELD\n");
			return NULL;
		}
	}

	if (aacEncoder_SetParam(paac_enc_handle->enc_handle, AACENC_SAMPLERATE,
				aac_enc.sample_rate) != AACENC_OK) {
		ERR_PRINTF("Unable to set the AOT\n");
		return NULL;
	}
	if (aacEncoder_SetParam(paac_enc_handle->enc_handle, AACENC_CHANNELMODE,
				mode) != AACENC_OK) {//channel mode
		ERR_PRINTF("Unable to set the channel mode\n");
		return NULL;
	}

	if (aacEncoder_SetParam(paac_enc_handle->enc_handle, AACENC_CHANNELORDER,
				1) != AACENC_OK) {
		ERR_PRINTF("Unable to set the wav channel order\n");
		return NULL;
	}
	if (variable_bitrate) {
		if (aacEncoder_SetParam(paac_enc_handle->enc_handle, AACENC_BITRATEMODE,
					variable_bitrate) != AACENC_OK) {
			ERR_PRINTF("Unable to set the VBR bitrate mode\n");
			return NULL;
		}
	} else {
		if (aacEncoder_SetParam(paac_enc_handle->enc_handle, AACENC_BITRATE,
					aac_enc.bitrate) != AACENC_OK) {
			ERR_PRINTF("Unable to set the bitrate\n");
			return NULL;
		}
	}


	if (aacEncoder_SetParam(paac_enc_handle->enc_handle, AACENC_TRANSMUX,
				TT_MP4_ADTS) != AACENC_OK) {//ADTS or ADIF
		ERR_PRINTF("Unable to set the ADTS transmux\n");
		return NULL;
	}
	if (aacEncoder_SetParam(paac_enc_handle->enc_handle, AACENC_AFTERBURNER,
				afterburner) != AACENC_OK) {
		ERR_PRINTF("Unable to set the afterburner mode\n");
		return NULL;
	}
	if (aacEncEncode(paac_enc_handle->enc_handle, NULL, NULL, NULL,
			 NULL) != AACENC_OK) {
		ERR_PRINTF("Unable to initialize the encoder\n");
		return NULL;
	}
	if (aacEncInfo(paac_enc_handle->enc_handle, &info) != AACENC_OK) {
		ERR_PRINTF("Unable to get the encoder info\n");
		return NULL;
	}




	DBG_PRINTF("[%s]print out config size[%d] buffer[0x%x] [0x%x] [0x%x] [0x%x] [0x%x]\n",
		   __func__, info.confSize,
		   info.confBuf[0], info.confBuf[1], info.confBuf[2], info.confBuf[3],
		   info.confBuf[4]);
	DBG_PRINTF("AAC encoder handle open success framelength[%d]\n",
		   info.frameLength);
	paac_enc_handle->suggestframe_len =
		info.frameLength;//1chn 1024 sample ->2048byte ,2chn 2048sample->4096byte

	paac_enc_handle->aac_inputbuffer = (short *)malloc(info.frameLength *
					   aac_enc.channel_num * DEFAULT_BYTES_PER_SAMPLE * 2);
	paac_enc_handle->aac_bufferbyte = info.frameLength * aac_enc.channel_num *
					  DEFAULT_BYTES_PER_SAMPLE * 2;

	return paac_enc_handle;
}

int CVI_AAC_Encode(void *paac_handle, short *pInputBuf,
		   unsigned char *pu8Outbuf, int s32InputBytes, int *ps32NumOutBytes)
{
	st_aac_enc_handler *paac_enc_handle = (st_aac_enc_handler *)paac_handle;
	int suggestframe_byte;
	int s32TotalSizeBytes = s32InputBytes;
	int s32Ret;

	if (paac_handle == NULL) {
		printf("paac_handle is null\n");
		return -1;
	}

	*ps32NumOutBytes = 0;
	suggestframe_byte = paac_enc_handle->suggestframe_len *
			    paac_enc_handle->aac_enc.channel_num * DEFAULT_BYTES_PER_SAMPLE;
	DBG_PRINTF("s32InputBytes = %d,suggestframe_byte = %d\n", s32InputBytes,
		   suggestframe_byte);


	if (s32InputBytes > paac_enc_handle->aac_bufferbyte) {
		ERR_PRINTF("s32InputBytes[%d] > aac_bufferbyte[%d]\n", s32InputBytes,
			   (int) paac_enc_handle->aac_bufferbyte);
		return -1;
	}

	if (paac_enc_handle->aac_remainbytes != 0) {

		if ((paac_enc_handle->aac_remainbytes + s32InputBytes) >
		    paac_enc_handle->aac_bufferbyte) {

			ERR_PRINTF("remain[%d] + input[%d] > bufsize[%d]",
				   (int)paac_enc_handle->aac_remainbytes,
				   s32InputBytes,
				   (int)paac_enc_handle->aac_bufferbyte);
			ERR_PRINTF("force flush the buffer\n");
			memset(paac_enc_handle->aac_inputbuffer, 0, paac_enc_handle->aac_bufferbyte);
			memcpy(paac_enc_handle->aac_inputbuffer, pInputBuf, s32InputBytes);
			s32TotalSizeBytes = s32InputBytes;
			return -1;
		}
		memcpy(paac_enc_handle->aac_inputbuffer + (paac_enc_handle->aac_remainbytes /
				sizeof(short)), pInputBuf, s32InputBytes);
		s32TotalSizeBytes = s32InputBytes + paac_enc_handle->aac_remainbytes;
		DBG_PRINTF("has data s32TotalSizeBytes = %d\n", s32TotalSizeBytes);

	} else {
		memcpy(paac_enc_handle->aac_inputbuffer, pInputBuf, s32InputBytes); //1280
		s32TotalSizeBytes = s32InputBytes;
		DBG_PRINTF("[%s][%d] inputlen[%d]\n", __func__, __LINE__, s32InputBytes);

	}

	paac_enc_handle->aac_remainbytes = s32TotalSizeBytes;


	if (paac_enc_handle->aac_remainbytes < suggestframe_byte) {
		//printf("remainbyte[%d]< suggestframe_byte[%d]\n",paac_enc_handle->aac_remainbytes,suggestframe_byte);
		return 0;
	}
		_check_and_dump("/tmp/before_aac_enc", (char *)paac_enc_handle->aac_inputbuffer,
				(unsigned int)suggestframe_byte);
		s32Ret = _aac_encode(paac_handle, paac_enc_handle->aac_inputbuffer, pu8Outbuf,
				     suggestframe_byte, ps32NumOutBytes);
		if (s32Ret) {
			printf("aac encode suggestframe_byte faile\n");
			return -1;
		}
		_check_and_dump("/tmp/after_aac_enc", (char *)pu8Outbuf,
				(unsigned int)ps32NumOutBytes);

		paac_enc_handle->aac_remainbytes = paac_enc_handle->aac_remainbytes -
						   suggestframe_byte;
		memcpy(paac_enc_handle->aac_inputbuffer,
		       paac_enc_handle->aac_inputbuffer + (suggestframe_byte / sizeof(short)),
		       paac_enc_handle->aac_remainbytes);
		//break;


	return 0;
}


int _aac_encode(void *paac_handle, short int *pInputBuf,
		unsigned char *pu8Outbuf, int s32InputBytes, int *ps32NumOutBytes)
{

	AACENC_BufDesc in_buf = { 0 }, out_buf = { 0 };
	AACENC_InArgs in_args = { 0 };
	AACENC_OutArgs out_args = { 0 };
	void *in_ptr, *out_ptr;
	int in_size, in_elem_size;
	int in_identifier = IN_AUDIO_DATA; //交错模式
	int out_identifier = OUT_BITSTREAM_DATA;
	int out_size, out_elem_size;
	unsigned char outbuf[20480];
	AACENC_ERROR err;

	st_aac_enc_handler *paac_enc_handle = (st_aac_enc_handler *)paac_handle;

	if (paac_handle == NULL) {
		printf("paac_handle is null\n");
		return -1;
	}

	in_ptr = pInputBuf;
	in_size = s32InputBytes;
	in_elem_size = 2;
	in_buf.numBufs = 1;
	in_buf.bufs = &in_ptr;
	in_buf.bufferIdentifiers = &in_identifier;
	in_buf.bufSizes = &in_size;
	in_buf.bufElSizes = &in_elem_size;

	in_args.numInSamples = s32InputBytes / 2;

	out_ptr = outbuf;
	out_size = sizeof(outbuf);
	out_elem_size = 1;
	out_buf.numBufs = 1;
	out_buf.bufs = &out_ptr;
	out_buf.bufferIdentifiers = &out_identifier;
	out_buf.bufSizes = &out_size;
	out_buf.bufElSizes = &out_elem_size;


	err = aacEncEncode(paac_enc_handle->enc_handle, &in_buf, &out_buf, &in_args,
			   &out_args);
	if (err != AACENC_OK) {
		if (err == AACENC_ENCODE_EOF) {
			printf("end of coding");
			return 0;
		}
		ERR_PRINTF("Encoding failed\n");
		return -1;
	}

	if (err == AACENC_OK) {
		if (out_args.numOutBytes != 0) {
			*ps32NumOutBytes = out_args.numOutBytes;
			memcpy(pu8Outbuf, outbuf, out_args.numOutBytes);
		} else
			ERR_PRINTF("return 0 size[%s][%d]\n", __func__, __LINE__);
	} else {
		if (err == AACENC_ENCODE_EOF) {
			printf("end of coding");
			return 0;
		}
		ERR_PRINTF("aac encode fail\n");
		return -1;
	}
	return 0;
}


int CVI_AAC_Decode_Dinit(void *paac_handle)
{
	st_aac_enc_handler *paac_enc_handle = (st_aac_enc_handler *)paac_handle;

	if (paac_handle == NULL) {
		ERR_PRINTF("paac_handle is null\n");
		return -1;
	}
	aacEncClose(&paac_enc_handle->enc_handle);
	//free(paac_enc_handle->aac_inputbuffer);
	return 0;
}


int CVI_AAC_Encode_InstallCb(void *paac_handle, pEncode_Cb pCbFunc)
{
	st_aac_enc_handler *paac_enc_handle = (st_aac_enc_handler *)paac_handle;

	if (paac_handle == NULL) {
		ERR_PRINTF("paac_handle is null\n");
		return -1;
	}

	paac_enc_handle->pCbEncode = pCbFunc;
	return 0;
}









