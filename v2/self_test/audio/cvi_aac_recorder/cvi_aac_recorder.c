#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include "cvi_aac_enc.h"
#include "cvi_audio.h"


const char *infile, *outfile;
FILE *fp_input;
FILE *fp_out;
int cap_time = 20;
int capturing = 1;

void sigint_handler(int sig)
{
	capturing = 0;
}

void register_inthandler(void)
{
	signal(SIGINT, sigint_handler);
	signal(SIGHUP, sigint_handler);
	signal(SIGTERM, sigint_handler);
}

void usage(const char *name)
{
	printf("=================================================\n");
	printf("if you want to record from mic on board and encode to aac\n");
	fprintf(stderr,
		"%s [-c channel] [-s sample_rate] [-p frame_size] [-b bitrate:64000] [-m 1] out.aac\n",
		name);
	printf("if you want to use file to file mode\n");
	fprintf(stderr,
		"%s [-c channel] [-s sample_rate] [-p frame_size] [-b bitrate:64000] [-m 0] in.wav out.aac\n",
		name);
	printf("=================================================\n");

}

int aac_encode_bind_ai(AAC_ENC_INFO aac_enc)
{
	AIO_ATTR_S	AudinAttr;
	AUDIO_FRAME_S stFrame;
	AEC_FRAME_S   stAecFrm;
	void *paac_enc_handle;
	int input_byte;
	int s32NumOutBytes;
	struct timespec end;
	struct timespec now;

	int s32Ret;
	int AiDev = 0;
	int AiChn = 0;

	AudinAttr.enSamplerate = aac_enc.sample_rate;
	AudinAttr.u32ChnCnt = aac_enc.channel_num;
	if (AudinAttr.u32ChnCnt == 1)
		AudinAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
	else if (AudinAttr.u32ChnCnt == 2)
		AudinAttr.enSoundmode = AUDIO_SOUND_MODE_STEREO;
	else {
		printf("do not support other channel count[%d]\n", AudinAttr.u32ChnCnt);
		return CVI_FAILURE;
	}

	register_inthandler();

	AudinAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	AudinAttr.enWorkmode = AIO_MODE_I2S_MASTER;
	AudinAttr.u32EXFlag = 0;
	AudinAttr.u32FrmNum = 10; /* only use in bind mode */
	AudinAttr.u32PtNumPerFrm = aac_enc.frame_size;
	AudinAttr.u32ClkSel = 0;
	AudinAttr.enI2sType = AIO_I2STYPE_INNERCODEC;

	s32Ret = CVI_AI_SetPubAttr(AiDev, &AudinAttr);
	s32Ret = CVI_AI_EnableChn(AiDev, AiChn);
	s32Ret = CVI_AI_Enable(AiDev);
	if (s32Ret == CVI_FAILURE)
		printf("CVI_AI_Enable failure\n");


	paac_enc_handle = CVI_AAC_Encode_Init(aac_enc);
	if (!paac_enc_handle) {
		printf("aac encode init error\n");
		return -1;
	}
	//CVI_AAC_Encode_InstallCb(paac_enc_handle,aac_enc_callback);

	input_byte = aac_enc.frame_size * aac_enc.channel_num * 2;
	short *inputbuf = (short *)malloc(input_byte);
	char *outbuf = malloc(input_byte);

	clock_gettime(CLOCK_MONOTONIC, &now);
	end.tv_sec = now.tv_sec + cap_time;
	end.tv_nsec = now.tv_nsec;

	while (capturing) {


		memset(inputbuf, 0, input_byte);
		memset(outbuf, 0, input_byte);
		s32Ret = CVI_AI_GetFrame(AiDev, AiChn,
					 &stFrame,
					 &stAecFrm,
					 -1);
		if (s32Ret != CVI_SUCCESS) {
			printf("[Error]CVI_AI_GetFrame none!!\n");
			break;
		}

		input_byte = stFrame.u32Len * AudinAttr.u32ChnCnt * 2;//16bit
		memcpy(inputbuf, stFrame.u64VirAddr[0], input_byte);



		s32Ret = CVI_AAC_Encode(paac_enc_handle, inputbuf,
					(unsigned char *)outbuf, input_byte,
					&s32NumOutBytes);
		if (s32Ret) {
			printf("aac encode error\n");
			return -1;
		}
		//printf("s32NumOutBytes = %d\n",s32NumOutBytes);
		fwrite(outbuf, 1, s32NumOutBytes, fp_out);

		if (cap_time) {
			clock_gettime(CLOCK_MONOTONIC, &now);
			if (now.tv_sec > end.tv_sec ||
			    (now.tv_sec == end.tv_sec && now.tv_nsec >= end.tv_nsec))
				break;
		}

	}

	free(inputbuf);
	inputbuf = NULL;
	free(outbuf);
	outbuf = NULL;

	CVI_AAC_Decode_Dinit(paac_enc_handle);

	CVI_AI_DisableChn(AiDev, AiChn);
	CVI_AI_Disable(AiDev);
	return 0;
}


int aac_encode_file_mode(AAC_ENC_INFO aac_enc)
{

	int input_byte;
	void *paac_enc_handle;
	int len;
	int s32NumOutBytes;
	int s32Ret;

	paac_enc_handle = CVI_AAC_Encode_Init(aac_enc);
	if (!paac_enc_handle) {
		printf("aac encode init error\n");
		return -1;
	}
	//CVI_AAC_Encode_InstallCb(paac_enc_handle,aac_enc_callback);
	input_byte = aac_enc.frame_size * aac_enc.channel_num * 2; //1280
	short *inputbuf = (short *)malloc(input_byte);
	char *outbuf = malloc(input_byte);


	while (1) {
		memset(inputbuf, 0, input_byte);
		memset(outbuf, 0, input_byte);
		len = fread(inputbuf, 1, input_byte, fp_input);
		if (len < input_byte) {
			printf("end of aac encode\n");
			break;
		}

		s32Ret = CVI_AAC_Encode(paac_enc_handle, inputbuf,
					(unsigned char *)outbuf, len,
					&s32NumOutBytes);
		if (s32Ret) {
			printf("aac encode error\n");
			return -1;
		}
		if (s32NumOutBytes ==
		    0) //if you set frame_size not 1024,you must continue to send data.
			continue;
		fwrite(outbuf, 1, s32NumOutBytes, fp_out);

	}

	free(outbuf);
	inputbuf = NULL;
	free(inputbuf);
	outbuf = NULL;

	CVI_AAC_Decode_Dinit(paac_enc_handle);
	return 0;
}


int main(int argc, char *argv[])
{
	int ch;

	int channel_num = 2;
	int sample_rate = 16000;
	int frame_size = 1024;
	int bitrate = 64000;
	EncFormat aacformat = 0;//AACLC

	AAC_ENC_INFO aac_enc;
	int s32Ret;
	int bind_ai = 1;

	while ((ch = getopt(argc, argv, "c:s:p:b:m:")) != -1) {
		switch (ch) {
		case 'c':
			channel_num = atoi(optarg);
			break;
		case 's':
			sample_rate = atoi(optarg);
			break;
		case 'p':
			frame_size = atoi(optarg);
			break;
		case 'b':
			bitrate = atoi(optarg);
			break;
		case 'm':
			bind_ai = atoi(optarg);
			break;
		case '?':
		default:
			printf("Please input the correct\n");
			usage(argv[0]);
			return 1;
		}
	}

	if (bind_ai == 1) {
		outfile = argv[optind];
		fp_out = fopen(outfile, "wb");
		if (!fp_out) {
			usage(argv[0]);
			perror("fp_out");
			return -1;
		}

	} else {
		infile = argv[optind];
		outfile = argv[optind + 1];

		fp_input = fopen(infile, "rb");
		if (!fp_input) {
			usage(argv[0]);
			perror("fp_input");
			return -1;
		}

		fp_out = fopen(outfile, "wb");
		if (!fp_out) {
			usage(argv[0]);
			perror("fp_out");
			return -1;
		}
	}

	aac_enc.channel_num = channel_num;
	aac_enc.sample_rate = sample_rate;
	aac_enc.frame_size = frame_size;
	aac_enc.bitrate = bitrate;
	aac_enc.aacformat = aacformat;


	if (bind_ai == 1) {
		printf("Record from mic on board and encode to aac\n");
		s32Ret = aac_encode_bind_ai(aac_enc);
		if (s32Ret) {
			printf("aac_encode_bind_ai fail\n");
			return -1;
		}

	} else {
		printf("file encode to aac\n");
		s32Ret = aac_encode_file_mode(aac_enc);
		if (s32Ret) {
			printf("aac_encode_file_mode fail\n");
			return -1;
		}
	}

	if (bind_ai == 1)
		fclose(fp_input);
	fclose(fp_out);
	return 0;

}
