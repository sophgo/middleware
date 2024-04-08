
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include "cvi_audio.h"
#include "cvi_mp3_encode.h"


static int _console_request(char *printout, int defalut_val);
#define _CONSOLE_REQ(X, Y) _console_request((char *)X, (int)Y)

//global variable
FILE *pfd_out;
FILE *pfd_in;
char *input_filename;
char output_filename[128] = {0};
unsigned char *input_buffer;
unsigned char *output_buffer;
CVI_S32 s32OutLen;
#define CVI_AUDIO_GET_FRAME_BLOCK_MODE -1

static int _console_request(char *printout, int default_val)
{
	char s_option[128];
	int s32Ret = -1;

	printf("\e[0;32m=====================================\n");
	fflush(stdout);
	fflush(stdin);
	if (printout != NULL) {
		printf(printout);
		fgets(s_option, 10, stdin);
		if (s_option[0] == '\n') {
			s32Ret = default_val;
			printf("input default val[%d]\n", s32Ret);
		} else {
			s32Ret = atoi(s_option);
			printf("input [%d]\n", s32Ret);
		}
	} else
		printf("Error input type[%s][%d]\n", __func__, __LINE__);

	printf("\e[0;32m=====================================\n");
	fflush(stdin);
	return s32Ret;
}

static int cvimp3enc_encode_usage(void)
{
	printf("=================================================\n");
	printf("cvi_mp3recorder usage\n");
	printf("-------------------------------------------------\n");
	printf("[encode file to mp3 format]:\n");
	printf("Command on console:\n");
	printf("cvi_mp3recorder $(var1).raw\n");
	printf("create $(input).raw.mp3 file after encode complete\n");
	printf("-------------------------------------------------\n");
	printf("[encode microphone in audio to mp3 format]:\n");
	printf("Command on console:\n");
	printf("cvi_mp3recorder live\n");
	printf("create audioin.mp3 file after encode complete\n");
	printf("=================================================\n");
	return 0;
}

static int _mp3_encode_live_from_mic(void)
{
	//step1:open file
	CVI_S32 s32Ret;
	AIO_ATTR_S	AudinAttr;
	AUDIO_FRAME_S stFrame;
	AEC_FRAME_S   stAecFrm;
	CVI_S32 s32RecSeconds = 0;
	CVI_S32 s32RecLoops = 0;
	CVI_S32 s32DevId = 0;

	snprintf(output_filename, 128, "audioin.mp3");
	pfd_out = fopen(output_filename, "wb");

	//step 1: setup audio in parameters
	AudinAttr.enSamplerate =
		(AUDIO_SAMPLE_RATE_E)_CONSOLE_REQ("Enter sample rate(default:16000)\n", 16000);
	AudinAttr.u32ChnCnt = _CONSOLE_REQ("Enter channel numbers(1 or 2)(default:2)\n", 2);
	if (AudinAttr.u32ChnCnt == 1)
		AudinAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
	else if (AudinAttr.u32ChnCnt == 2)
		AudinAttr.enSoundmode = AUDIO_SOUND_MODE_STEREO;
	else {
		printf("do not support other channel count[%d]\n", AudinAttr.u32ChnCnt);
		return CVI_FAILURE;
	}

	AudinAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	AudinAttr.enWorkmode = AIO_MODE_I2S_MASTER;
	AudinAttr.u32EXFlag = 0;
	AudinAttr.u32FrmNum = 10; /* only use in bind mode */
	AudinAttr.u32PtNumPerFrm =
		_CONSOLE_REQ("Enter period size(samples per frame)(default:480)\n", 480);
	AudinAttr.u32ClkSel = 0;
	AudinAttr.enI2sType = AIO_I2STYPE_INNERCODEC;
	s32Ret = CVI_AI_SetPubAttr(s32DevId, &AudinAttr);
	s32Ret = CVI_AI_EnableChn(s32DevId, 0);
	s32Ret = CVI_AI_Enable(s32DevId);
	if (s32Ret == CVI_FAILURE)
		printf("CVI_AI_Enable failure\n");

	s32RecSeconds = _CONSOLE_REQ("How many seconds you want to record(default:10s)\n", 10);
	s32RecLoops = (s32RecSeconds * AudinAttr.enSamplerate) / AudinAttr.u32PtNumPerFrm;
	printf("--------------------------------->start recording...\n");
	CVI_S32 s32ChnCnt = AudinAttr.u32ChnCnt;
	//step2:start init the mp3 encoder
	void *pMp3EncHandler;
	ST_CVI_MP3_ENC_INIT stMp3EncConfig;

	//TODO: this should be optional
	stMp3EncConfig.bitrate = 128000;
	stMp3EncConfig.sample_rate = (int)AudinAttr.enSamplerate;
	stMp3EncConfig.channel_num = s32ChnCnt;
	stMp3EncConfig.quality = 9;
	pMp3EncHandler = CVI_MP3_Encode_Init(NULL, stMp3EncConfig);
	if (pMp3EncHandler == NULL) {
		printf("[error][%s][%d]\n", __func__, __LINE__);
		return -1;
	}

	input_buffer = (unsigned char *)malloc(2048);
	output_buffer = (unsigned char *)malloc(2048);

	while (s32RecLoops--) {
		s32Ret = CVI_AI_GetFrame(s32DevId, 0,
						&stFrame,
						&stAecFrm,
						CVI_AUDIO_GET_FRAME_BLOCK_MODE);
		if (s32Ret != CVI_SUCCESS) {
			printf("[Error]CVI_AI_GetFrame none!!\n");
			break;
		}

		memcpy(input_buffer, stFrame.u64VirAddr[0], (stFrame.u32Len * s32ChnCnt * 2));
		s32Ret = CVI_MP3_Encode((CVI_VOID *)pMp3EncHandler,
					(CVI_VOID *)input_buffer,
					(CVI_VOID *)output_buffer,
					(CVI_S32)(stFrame.u32Len * s32ChnCnt * 2),
					(CVI_S32 *)&s32OutLen);

		if (s32Ret != CVI_SUCCESS) {
			if (s32Ret == CVI_MP3_ENC_CB_REQUIRE_MORE_INPUT)
				printf("Require more bytes\n");
		} else
			printf("[success][%s][%d]\n", __func__, __LINE__);

		printf("output length s32OutLen[%d]\n", s32OutLen);
		if (s32OutLen != 0)
			fwrite(output_buffer, 1, (s32OutLen), pfd_out);
	}

	CVI_MP3_Encode_DeInit(pMp3EncHandler);

	if (input_buffer != NULL)
		free(input_buffer);
	if (output_buffer != NULL)
		free(output_buffer);

	fclose(pfd_out);
	return CVI_SUCCESS;
}

static int _mp3_encode_file_mode(void)
{
	CVI_S32 s32Ret;

	//step1:open file
	snprintf(output_filename, 128, "%s.mp3", input_filename);
	pfd_in = fopen(input_filename, "rb");
	pfd_out = fopen(output_filename, "wb");
	fseek(pfd_in, 0, SEEK_END);
	int fsize = ftell(pfd_in);

	fseek(pfd_in, 0, SEEK_SET);
	printf("input file total size bytes[%d]\n", fsize);
	input_buffer = (unsigned char *)malloc(fsize);
	output_buffer = (unsigned char *)malloc(fsize * 6);
	printf("[v][v][%s][%d] total output buffer size[%d]\n", __func__, __LINE__, fsize);

	//step2:start init the mp3 encoder
	void *pMp3EncHandler;
	ST_CVI_MP3_ENC_INIT stMp3EncConfig;

	//TODO: this should be optional
	stMp3EncConfig.bitrate = 128000;
	stMp3EncConfig.sample_rate = _CONSOLE_REQ("Enter sample rate:(default:32000)\n", 32000);
	stMp3EncConfig.channel_num = _CONSOLE_REQ("Enter channel num:(default:2)\n", 2);
	stMp3EncConfig.quality = 9;
	pMp3EncHandler = CVI_MP3_Encode_Init(NULL, stMp3EncConfig);
	if (pMp3EncHandler == NULL) {
		printf("[error][%s][%d]\n", __func__, __LINE__);
		return -1;
	}

	//optional:user call also get the data from call back function
	//s32Ret = CVI_MP3_Decode_InstallCb(pMp3DecHandler, _mp3_dec_callback_forApp);
	//_mp3_enc_callback_forApp(NULL, NULL, 0);
	int read_size = 0;

	while (1) {
		read_size = fread(input_buffer, 1, 1024, pfd_in);
		if (read_size != 1024) {
			printf("read file end ...break\n");
			break;
		}
		printf("test 222--->give it 1024\n");
		s32Ret = CVI_MP3_Encode((CVI_VOID *)pMp3EncHandler,
					(CVI_VOID *)input_buffer,
					(CVI_VOID *)output_buffer,
					(CVI_S32)1024,
					(CVI_S32 *)&s32OutLen);

		if (s32Ret != CVI_SUCCESS) {
			if (s32Ret == CVI_MP3_ENC_CB_REQUIRE_MORE_INPUT)
				printf("Require more bytes\n");
		} else
			printf("[error][%s][%d]\n", __func__, __LINE__);

		printf("output length s32OutLen[%d]\n", s32OutLen);
		if (s32OutLen != 0)
			fwrite(output_buffer, 1, (s32OutLen), pfd_out);
	}

	CVI_MP3_Encode_DeInit(pMp3EncHandler);

	if (input_buffer != NULL)
		free(input_buffer);
	if (output_buffer != NULL)
		free(output_buffer);

	fclose(pfd_in);
	fclose(pfd_out);
	return CVI_SUCCESS;
}

int main(int argc, char *argv[])
{
	printf("enter cvi_mp3recorder exe\n");
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_BOOL bLiveRecord = CVI_FALSE;


	if (argc < 2) {
		printf("Not enough input\n");
		cvimp3enc_encode_usage();
		return 0;
	}

	s32Ret = strcmp(argv[1], "live");

	if (s32Ret == 0)
		bLiveRecord = CVI_TRUE;
	else
		bLiveRecord = CVI_FALSE;//encode the file input

	if (bLiveRecord == CVI_TRUE) {
		printf("Record from mic on board and encode to mp3\n");
		_mp3_encode_live_from_mic();
	} else {
		printf("file encode to mp3\n");
		input_filename = argv[1];
		if (access(input_filename, 0) <  0) {
			printf("[Error]check file[%s] not exist!....error\n", input_filename);
			return (-1);
		}

		_mp3_encode_file_mode();
	}

	printf("Exit program!!!\n");
	return 0;
}

