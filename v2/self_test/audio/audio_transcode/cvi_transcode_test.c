/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: sample/cvi_transcode_test.c
 * Description: Transcode sample code based on ffmpeg function
 */
#include <stdio.h>
#ifdef USE_FFMPEG
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavcodec/avcodec.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/frame.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if defined(__CV181X__) || defined(__CV180X__)
#include <cvi_type.h>
#else
#include "cvi_type.h"
#endif
#include "cvi_transcode_interface.h"
#include "cvi_audio_loadcheck.h"
#endif

#define AUDIO_BUFFER_MAX (50 * 160 * 1024)

/* The output bit rate in bit/s */
#define OUTPUT_BIT_RATE 96000
/* The number of output channels */
#define OUTPUT_CHANNELS 2
#define CVI_MODIFIED 1
#if CVI_MODIFIED
int argv3;
#endif


S_CVI_AUDIO_CONFIG cviConfig;
CVI_BOOL bEncode;
CVI_BOOL bDecode;
E_CVI_AUDIOCODEC eACodecType;
char *input_filename;
char output_filename[128] = {0};

static CVI_S32 _cvi_usage_msg(void)
{
	do {
		printf("How to use sample_audio_transcode:\n");
		printf("-----------------------------------------------------------------------------------\n");
		printf("type in: ./sample_audio_transcode  $(var1)\n");
		printf("$(var1):The file name you want to transcode\n");
		printf("Enter(Choose) the following option which print on the terminal\n");
		printf("THe program will print out the result filename after exit\n");
		printf("-------------------------------------------------------------------------------------\n");
	} while (0);
	return 0;
}

static CVI_BOOL _cvi_checkname_iswav(char *infilename)
{
	CVI_S32 s32InputFileLen = 0;

	s32InputFileLen = strlen(infilename);

	if (s32InputFileLen == 0) {
		printf("No Input File Name..force return\n");
		return 0;
	}

	if (infilename[s32InputFileLen-4] == '.' &&
	(infilename[s32InputFileLen-3] == 'W' || infilename[s32InputFileLen-3] == 'w') &&
	(infilename[s32InputFileLen-2] == 'A' || infilename[s32InputFileLen-2] == 'a') &&
	(infilename[s32InputFileLen-1] == 'V' || infilename[s32InputFileLen-1] == 'v')) {
		printf("Enter wav file\n");
		return CVI_TRUE;
	} else
		return CVI_FALSE;

}


static CVI_S32 _cvi_transcode_option_unittest(void)
{

	CVI_S32 s32Ret = CVI_SUCCESS;

	int sample_rate;
	int channel_num;

	int s_option;

	cviConfig.bitrate = Rate16kBits;
	sample_rate = 16000;
	channel_num = 2;
	cviConfig.channel_num = channel_num;
	cviConfig.sample_rate = sample_rate;
	s_option = 2;
	printf("[unit test]sample rate[%d]\n", sample_rate);
	printf("[unit test]channel numbers[%d]\n", channel_num);
	printf("0:g711_a_law\n");
	printf("1:g711_mu_law\n");
	printf("[unit test]2:g726\n");
	printf("3:ADPCM_IMA\n");
	printf("4:ADPCM_DVI4\n");
	printf("5:ADPCM_VDVI\n");
	printf("6:AAC-ADTS\n");

	if (s_option == 0) {
		eACodecType = AUD_CODEC_G711_ALAW;
		snprintf(output_filename, 128, "%s.g711a", input_filename);
	} else if (s_option == 1) {
		eACodecType = AUD_CODEC_G711_MLAW;
		snprintf(output_filename, 128, "%s.g711mu", input_filename);
	} else if (s_option == 2) {
		eACodecType = AUD_CODEC_G726;
		snprintf(output_filename, 128, "%s.g726", input_filename);
	} else if (s_option == 3) {
		eACodecType = AUD_CODEC_ADPCM_IMA;
		snprintf(output_filename, 128, "%s.adpcm_ima", input_filename);
	} else if (s_option == 4) {
		eACodecType = AUD_CODEC_ADPCM_DVI4;
		snprintf(output_filename, 128, "%s.adpcm_dvi4", input_filename);
	} else if (s_option == 5) {
		eACodecType = AUD_CODEC_ADPCM_VDVI;
		snprintf(output_filename, 128, "%s.adpcm_vdvi", input_filename);
	} else if (s_option == 6) {
		//codetype = 6;
#if 0
		eACodecType = AUD_CODEC_AAC;
		snprintf(output_filename, 128, "%s.aac_adts", input_filename);
#endif
	} else {
		eACodecType = AUD_CODEC_NONE;
		snprintf(output_filename, 128, "%s.error", input_filename);
	}

	if (bDecode)
		snprintf(output_filename, 128, "%s.raw", input_filename);

	return s32Ret;
}

static CVI_S32 _cvi_transcode_option(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	int sample_rate;
	int channel_num;
	int codetype = -1;
	char s[100];
	int s_option;

	printf("Enter sample rate :");
	fgets(s, 10, stdin);
	sample_rate = atoi(s);
	printf("\n");
	printf("sample rate[%d]\n", sample_rate);

	printf("Enter channel numbers :");
	fgets(s, 10, stdin);
	channel_num = atoi(s);
	printf("\n");
	printf("channel numbers[%d]\n", channel_num);

	cviConfig.channel_num = channel_num;
	cviConfig.sample_rate = sample_rate;
	cviConfig.bitrate = Rate16kBits;
	do {
		printf("0:g711_a_law\n");
		printf("1:g711_mu_law\n");
		printf("2:g726\n");
		printf("3:ADPCM_IMA\n");
		printf("4:ADPCM_DVI4\n");
		//printf("5:ADPCM_VDVI\n"); //remove VDVI option, not support in SPEC
		//printf("6:AAC-ADTS\n");//AAC transcode not available here
		if (bEncode)
			printf("Enter Encode target type:");
		if (bDecode)
			printf("Enter current type:");

		fgets(s, 10, stdin);
		s_option = atoi(s);
		printf("\n");

		if (s_option == 0) {
			codetype = 0;
			eACodecType = AUD_CODEC_G711_ALAW;
			snprintf(output_filename, 128, "%s.g711a", input_filename);
		} else if (s_option == 1) {
			codetype = 1;
			eACodecType = AUD_CODEC_G711_MLAW;
			snprintf(output_filename, 128, "%s.g711mu", input_filename);
		} else if (s_option == 2) {
			int tmp = 0;

			codetype = 2;
			eACodecType = AUD_CODEC_G726;
			snprintf(output_filename, 128, "%s.g726", input_filename);

			printf("Enter bit rate 0:16k, 1:24k, 2:32k 3:40k\n");
			fgets(s, 10, stdin);
			tmp = atoi(s);
			if (tmp == 0) {
				printf("choose 16k\n");
				cviConfig.bitrate = Rate16kBits;
			} else if (tmp == 1) {
				printf("choose 24k\n");
				cviConfig.bitrate = Rate24kBits;
			} else if (tmp == 2) {
				printf("choose 32k\n");
				cviConfig.bitrate = Rate32kBits;
			} else if (tmp == 3) {
				printf("choose 40k\n");
				cviConfig.bitrate = Rate40kBits;
			} else {
				printf("default 16k\n");
				cviConfig.bitrate = Rate16kBits;
			}
		} else if (s_option == 3) {
			codetype = 3;
			eACodecType = AUD_CODEC_ADPCM_IMA;
			snprintf(output_filename, 128, "%s.adpcm_ima", input_filename);
		} else if (s_option == 4) {
			codetype = 4;
			eACodecType = AUD_CODEC_ADPCM_DVI4;
			snprintf(output_filename, 128, "%s.adpcm_dvi4", input_filename);
		} else if (s_option == 5) {
			codetype = 5;
			eACodecType = AUD_CODEC_ADPCM_VDVI;
			snprintf(output_filename, 128, "%s.adpcm_vdvi", input_filename);
		} else if (s_option == 6) {
			codetype = 6;
#if 0
			eACodecType = AUD_CODEC_AAC;
			snprintf(output_filename, 128, "%s.aac_adts", input_filename);
#endif
		} else {
			eACodecType = AUD_CODEC_NONE;
			codetype = -1;
			snprintf(output_filename, 128, "%s.error", input_filename);
		}

	} while (codetype < 0);

	if (bDecode)
		snprintf(output_filename, 128, "%s.raw", input_filename);

	return s32Ret;
}

int main(int argc, char **argv)
{
#define AUDIO_CPU_LOADING_CHECK_G711 1
#define AUDIO_CPU_LOADING_CHECK_G726 2
#define AUDIO_CPU_LOADING_CHECK_NONE 3
	printf("Not USE FFMPEG\n");
	CVI_CHAR *audio_buffer;
	CVI_CHAR *pOutputBUf;
	CVI_S32 s32InputLen;
	CVI_S32 s32OutLen;
	CVI_CHAR s_option[100];
	CVI_S32 s32Ret = CVI_FAILURE;
	CVI_S32 s32RetFrm = CVI_FAILURE;
	CVI_S32 opt = -1;
	CVI_S32 s32framesize = 160;
	CVI_BOOL bIsWav = CVI_FALSE;
	CVI_BOOL bUnitTestModeDecode = CVI_FALSE;
	CVI_BOOL bUnitTestModeEncode = CVI_FALSE;
	int user_default_option = AUDIO_CPU_LOADING_CHECK_NONE;
	void *ploadcheckhandle = NULL;

	bEncode = CVI_FALSE;
	bDecode = CVI_FALSE;


	FILE *pfd_in = NULL;
	FILE *pfd_out = NULL;

	input_filename = argv[1];


	/* Step 1: open file and check is raw or wav */
	do {

	/* check the input argv valid or not*/
	_cvi_usage_msg();
	printf("=============================================\n");
	} while (0);

	//none unit test mode
	//usage: sample_audio_transcode [filename]
	if (!(argc > 1)) {
		_cvi_usage_msg();
		printf("Error: not enough input arg\n");
		return 0;
	}

	//for unit test only
	if (argc >= 3) {
		//usage: sample_audio_transcode --unittest decode [filename]
		//you need to at least enter 4 input argument to setup unit test mode
		if (!strcmp(argv[1], "--unittest")) {
			printf("transcode enter unit test mode argc[%d]\n", argc);
			if (argc == 5)  {
				user_default_option = atoi(argv[4]);
				printf("user_default_option[%d]\n", user_default_option);
			}
			if (!strcmp(argv[2], "decode")) {
				bUnitTestModeDecode = CVI_TRUE;
				bEncode = CVI_FALSE;
				bDecode = CVI_TRUE;
				printf("transcode enter unit test mode..decode\n");
			} else if (!strcmp(argv[2], "encode")) {
				bUnitTestModeEncode = CVI_TRUE;
				bEncode = CVI_TRUE;
				bDecode = CVI_FALSE;
				printf("transcode enter unit test mode..encode\n");
			} else {
				bUnitTestModeEncode = CVI_FALSE;
				bUnitTestModeDecode = CVI_FALSE;
				printf("Enter Unit test mode failure\n");
			}
			input_filename = argv[3];

			if (access(input_filename, 0) < 0) {
				printf("[Error]Input file not exist\n");
				bUnitTestModeEncode = CVI_FALSE;
				bUnitTestModeDecode = CVI_FALSE;
			}

		} else {
			bUnitTestModeDecode = CVI_FALSE;
			bUnitTestModeEncode = CVI_FALSE;
		}
	}


	bIsWav = _cvi_checkname_iswav(input_filename);

	if (bIsWav == CVI_TRUE)
		printf("[Error] The test unit only supprot encoded file type or raw format\n");

	CVI_S32 s32Cnt = -1;

	if (bUnitTestModeEncode || bUnitTestModeDecode) {
		//enter unit test
		s32Ret = _cvi_transcode_option_unittest();

		if (s32Ret != CVI_SUCCESS) {
			printf("[Error]setting transcode option abnormal...!\n");
			return 0;
		}

	} else {
		do {
			printf("Choose 1:Encode or 2:Decode :");
			fgets(s_option, 10, stdin);
			opt = atoi(s_option);
			s32Cnt++;
			printf("select[%d]\n", opt);
		} while ((opt != 1) && (opt != 2) && (s32Cnt < 3));
		printf("\n");


		if (opt == 1) {
			bEncode = CVI_TRUE;
			bDecode = CVI_FALSE;
		} else if (opt == 2) {
			bDecode = CVI_TRUE;
			bEncode = CVI_FALSE;
		} else {
			printf("[Error]Option failure..force return\n");
			return 0;
		}
		/* Step 2: Do the transcode option */
		s32Ret = _cvi_transcode_option();

		if (s32Ret != CVI_SUCCESS) {
			printf("[Error]setting transcode option abnormal...!\n");
			return 0;
		}
	}
	if (user_default_option != 0) {
		if (user_default_option == AUDIO_CPU_LOADING_CHECK_G711) {
			ploadcheckhandle = CVI_AUDIO_LoadingCheck_Init(8000, 1, 20, "transcode_g711");
			eACodecType = AUD_CODEC_G711_MLAW;
		}
		if (user_default_option == AUDIO_CPU_LOADING_CHECK_G726) {
			ploadcheckhandle = CVI_AUDIO_LoadingCheck_Init(8000, 1, 20, "transcode_g726");
			eACodecType = AUD_CODEC_G726;
		}
	}

	CVI_VOID *cviTrans = CVI_NULL;

	cviTrans = CVI_AUDIO_Transcode_Init(
		NULL,
		eACodecType,
		&cviConfig);

	if (cviTrans == CVI_NULL) {
		printf("[Error]transcode INIT setting error ....\n");
		return 0;
	}

#if 0
	if (eACodecType == AUD_CODEC_AAC) {
		s32framesize = 1024;
		s32framesize = s32framesize * cviConfig.channel_num;
		printf("AAC encoder frame size(samples) is [%d] to cope with spec[%d]\n", s32framesize);
	}
#endif


	audio_buffer = (CVI_CHAR *)malloc(AUDIO_BUFFER_MAX);
	pOutputBUf =  (CVI_CHAR *)malloc(AUDIO_BUFFER_MAX);
	pfd_in =  fopen(input_filename, "rb");
	pfd_out = fopen(output_filename, "wb");
	if (((pfd_in) == NULL) || ((pfd_out) == NULL)) {
		printf("[Error]Cannot open input / output file\n");
		goto Pattern_EOF2;
	}
	/* Step 2-1: Do the encode */
	if (bEncode) {

		for (;;) {
			s32InputLen = sizeof(short)*s32framesize;//input bytes

			if (fread(audio_buffer,
				 sizeof(short),
				 s32framesize, pfd_in) != (size_t)s32framesize) {
			/* get current frame data */
			printf("leaving encode...[%d]\n", __LINE__);
			goto Pattern_EOF;
			} else {

				CVI_AUDIO_LoadingCheck_Begin(ploadcheckhandle);
			s32RetFrm = s32Ret = CVI_AUDIO_Encode(cviTrans,
				eACodecType,
				(CVI_VOID *)audio_buffer,
				(CVI_VOID *)pOutputBUf,
				s32InputLen,
				&s32OutLen);//s32OutLen return bytes
			CVI_AUDIO_LoadingCheck_End(ploadcheckhandle, s32InputLen);
			if (s32Ret != CVI_SUCCESS) {

			printf("[Error]encode error\n");

			goto Pattern_EOF;
			} else {
			/* Write out to file */
			fwrite(pOutputBUf, 1, s32OutLen, pfd_out);
			}

			}
		}
	} /* End of encode */



	/* Step 2-2: Do the decode */
	if (bDecode) {

		s32InputLen = sizeof(short)*s32framesize;//input bytes

		#if 0
		if (eACodecType == AUD_CODEC_AAC) {
			fseek(pfd_in, 0, SEEK_END);

			uint64_t fsize = ftell(pfd_in);

			fseek(pfd_in, 0, SEEK_SET);
			s32framesize = fsize/2;
			printf("AAC decode edition......size sample[%d]\n", s32framesize);
			s32InputLen = sizeof(short)*s32framesize;//input bytes

			if (s32InputLen > AUDIO_BUFFER_MAX) {
				printf("Error .. the AAC input file size too large(over 1M)\n");
				printf("Please choose shorter files\n");
				goto Pattern_EOF;
			}
		}
		#endif

		for (;;) {


			if (0) {//eACodecType == AUD_CODEC_AAC) {
				//for AAC adts decode ---------------start
				long curpos;
				CVI_S32 s32RequireBytes = 0;

				curpos = ftell(pfd_in);
				printf("curpos-------------------------xxxxxx>[%ld]\n ", curpos);

				if (fread(audio_buffer,
				 sizeof(short),
				 s32framesize, pfd_in) != (size_t)s32framesize) {
				/* get current frame data */
				printf("leaving decode...AAC file end[%d]\n", __LINE__);
				goto Pattern_EOF;
				}

				s32Ret =  CVI_AUDIO_DecodeParseAAC(cviTrans,
						eACodecType,
						(CVI_VOID *)audio_buffer,
						s32InputLen,
						&s32RequireBytes);
				if (s32Ret == CVI_FAILURE) {
					printf("leaving decode...No ADTS header[%d]\n", __LINE__);
					goto Pattern_EOF;
				}
				printf("s32RequireBytes---------->[%d]\n", s32RequireBytes);


				fseek(pfd_in, curpos, SEEK_SET);

				if (fread(audio_buffer,
						1,
						s32RequireBytes, pfd_in) != (size_t)s32RequireBytes) {
				printf("leaving decode...AAC file end2[%d]\n", __LINE__);
				goto Pattern_EOF;
				}

				s32Ret = CVI_AUDIO_Decode(cviTrans,
				eACodecType,
				(CVI_VOID *)audio_buffer,
				(CVI_VOID *)pOutputBUf,
				s32RequireBytes,
				&s32OutLen);//return bytes
				if (s32Ret != CVI_SUCCESS) {
				printf("[Error]decode error[%d]\n", s32Ret);
				goto Pattern_EOF;
				} else {
				/* Write out to file */
				fwrite(pOutputBUf, 1, s32OutLen, pfd_out);
				}
				//for AAC adts decode ---------------end
			} else {

			if (fread(audio_buffer,
			 sizeof(short),
			 s32framesize, pfd_in) != (size_t)s32framesize) {
			/* get current frame data */
			printf("leaving decode.......[%d]\n", __LINE__);
			goto Pattern_EOF;
			} else {
			s32RetFrm = s32Ret = CVI_AUDIO_Decode(cviTrans,
				eACodecType,
				(CVI_VOID *)audio_buffer,
				(CVI_VOID *)pOutputBUf,
				s32InputLen,
				&s32OutLen);//return bytes
			if (s32Ret != CVI_SUCCESS) {
			printf("[Error]decode error[%d]\n", s32Ret);
			goto Pattern_EOF;
			} else {
			/* Write out to file */
			fwrite(pOutputBUf, 1, s32OutLen, pfd_out);
			}

			}

			}
#if 0 //backup
			if (fread(audio_buffer,
				 sizeof(short),
				 s32framesize, pfd_in) != s32framesize) {
			/* get current frame data */
			printf("leaving decode...[%d]\n", __LINE__);
			goto Pattern_EOF;
			} else {
			s32RetFrm = s32Ret = CVI_AUDIO_Decode(cviTrans,
				eACodecType,
				(CVI_VOID *)audio_buffer,
				(CVI_VOID *)pOutputBUf,
				s32InputLen,
				&s32OutLen);//return bytes
			if (s32Ret != CVI_SUCCESS) {
			printf("[Error]decode error[%d]\n", s32Ret);
			goto Pattern_EOF;
			} else {
			/* Write out to file */
			fwrite(pOutputBUf, 1, s32OutLen, pfd_out);
			}

			}
#endif
		}

	}
	CVI_AUDIO_LoadingCheck_DeInit(ploadcheckhandle);
	/* Step 3: Finish the trancode */
Pattern_EOF:
	if (bEncode)
		printf("Encode finished\n");
	if (bDecode)
		printf("Decode finished\n");
	fclose(pfd_in);
	fclose(pfd_out);
Pattern_EOF2:
	free(audio_buffer);
	free(pOutputBUf);
	CVI_AUDIO_Transcode_DeInit(cviTrans,
				eACodecType);
	if (bUnitTestModeEncode || bUnitTestModeDecode) {
		if (s32RetFrm == CVI_SUCCESS)
			printf("cvi_transcode unit TEST-PASS\n");
		else
			printf("cvi_transcode unit TEST-NG\n");
	}

	return 0;
}





