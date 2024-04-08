#include "cvi_audio_uac.h"

static void _show_usage(void)
{
	printf("====================================\n");
	printf("sample_audio_uac usage :\n");
	printf("Type in:\n");
	printf("[sample_audio_uac 1]: mic in record(48k/1chn) to /tmp/filesave.pcm file\n");
	printf("[sample_audio_uac 2]: read&decode /tmp/mic_record.pcm file and play out in speaker\n");
	printf("[sample_audio_uac 3]: read&decode /tmp/test.mp3 file and play out in speaker\n");
	printf("[sample_audio_uac 4]: read&decode /tmp/testaac.aac file and play out in speaker\n");
	printf("[sample_audio_uac 5]: read&decode /tmp/test.mp3 file and save to /tmp/filesave.pcm file\n");
	printf("[smaple_audio_uac 6]: read&decode /tmp/test.mp3 file and play out in USB headset(usb speaker)\n");
	printf("[sample_audio_uac 7]: read&decode /tmp/testaac.aac file and play out in USB headset(usb_speaker)\n");
	printf("[sampel_audio_uac 8]: mic in record(48/2chn) and play out in USB headset(usb speaker)\n");
	printf("[sampel_audio_uac 9]: simulate network stream in  /tmp/test.mp3 and play out in speaker\n");
	printf("[sampel_audio_uac 10]: usb in mic  and play out in speaker\n");
	printf("====================================\n");
}

int main(int argc, const char *const argv[])
{
//------parse input param for differ sample case[start]

	if (!(argc > 1)) {
		_show_usage();
		return 0;
	}
	printf("Enter sample_uac_audio api test\n");
//------parse input param for differ sample case[end]
	int sample_case = atoi(argv[1]);

	ST_UAC_SRC_PARAM stUacSrcParam;

	memset(&stUacSrcParam, 0, sizeof(ST_UAC_SRC_PARAM));
	cviaudio_uac_init();
	cviaudio_set_spk_volume(6);//range 0~32
	cviaudio_set_mic_volume(12);//range 0~24

	switch (sample_case) {

	case 1:
	{
		printf("mic in record(48k/2chn) to /tmp/filesave.pcm file\n");
		stUacSrcParam.stMicInParam.channels = 1;
		stUacSrcParam.stMicInParam.period_size = 480;
		stUacSrcParam.stMicInParam.rate = 48000;
		stUacSrcParam.stMicInParam.pfd_MicInSave = fopen("/tmp/filesave48k1chn.pcm", "wb");
		cviaudio_uac_createSrc(CVIAUDIO_SRC_MIC_IN,
					CVIAUDIO_UAC_TYPE_PCM,
					&stUacSrcParam);

		cviaudio_uac_createDst(CVIAUDIO_DST_FILE_SAVE);
		cviaudio_uac_bind(CVIAUDIO_SRC_MIC_IN, CVIAUDIO_DST_FILE_SAVE);
		printf("Keep press Enter key to leave the test\n");
		getchar();
		getchar();
		cviaudio_uac_unbind(CVIAUDIO_SRC_MIC_IN, CVIAUDIO_DST_FILE_SAVE);
		cviaudio_uac_destroy(CVIAUDIO_SRC_MIC_IN, CVIAUDIO_DST_FILE_SAVE);
	}
	break;

	case 2:
	{
		printf("read&decode /tmp/mic_record.pcm file[44.1k/2ch] and play out in speaker\n");

		stUacSrcParam.stFileInParam.pcmInfo.channels = 2;
		stUacSrcParam.stFileInParam.pcmInfo.period_size = 480;
		stUacSrcParam.stFileInParam.pcmInfo.rate = 44100;
		stUacSrcParam.stFileInParam.pfd =  fopen("/tmp/mic_record.pcm", "rb");
		stUacSrcParam.stFileInParam.pfd_save =  NULL;//no need to save file
		cviaudio_uac_createSrc(CVIAUDIO_SRC_FILE_IN,
					CVIAUDIO_UAC_TYPE_PCM,
					&stUacSrcParam);
		cviaudio_uac_createDst(CVIAUDIO_DST_SPK_OUT);
		cviaudio_uac_bind(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_SPK_OUT);
		printf("Keep press Enter key to leave the test\n");
		getchar();
		getchar();
		cviaudio_uac_unbind(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_SPK_OUT);
		cviaudio_uac_destroy(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_SPK_OUT);

	}
	break;

	case 3:
	{
		printf("read&decode /tmp/test.mp3 file and play out in speaker\n");
		stUacSrcParam.stFileInParam.decodeFileType = CVIAUDIO_UAC_TYPE_MP3;
		stUacSrcParam.stFileInParam.pfd = fopen("/tmp/test.mp3", "rb");
		stUacSrcParam.stFileInParam.pfd_save = NULL;
		cviaudio_uac_createSrc(CVIAUDIO_SRC_FILE_IN,
					CVIAUDIO_UAC_TYPE_MP3,
					&stUacSrcParam);
		cviaudio_uac_createDst(CVIAUDIO_DST_SPK_OUT);//48k  1ch / 48 2ch  USB IN/OUT pcm_open(2,0,pcm_in)
		cviaudio_uac_bind(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_SPK_OUT);
		printf("Keep press Enter key to leave the test\n");
		getchar();
		getchar();
		cviaudio_uac_unbind(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_SPK_OUT);
		cviaudio_uac_destroy(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_SPK_OUT);
	}
	break;

	case 4:
	{
		printf("read&decode /tmp/testaac.aac file and play out in speaker\n");
		stUacSrcParam.stFileInParam.decodeFileType = CVIAUDIO_UAC_TYPE_AAC;
		stUacSrcParam.stFileInParam.pfd = fopen("/tmp/testaac.aac", "rb");
		stUacSrcParam.stFileInParam.pfd_save = NULL;
		cviaudio_uac_createSrc(CVIAUDIO_SRC_FILE_IN,
					CVIAUDIO_UAC_TYPE_AAC,
					&stUacSrcParam);

		cviaudio_uac_createDst(CVIAUDIO_DST_SPK_OUT);//48k  1ch / 48 2ch  USB IN/OUT pcm_open(2,0,pcm_in)
		cviaudio_uac_bind(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_SPK_OUT);
		printf("Keep press Enter key to leave the test\n");
		getchar();
		getchar();
		cviaudio_uac_unbind(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_SPK_OUT);
		cviaudio_uac_destroy(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_SPK_OUT);

	}
	break;

	case 5:
	{
		printf("read&decode /tmp/test.mp3 file and save to /tmp/filesave.pcm file\n");
		stUacSrcParam.stFileInParam.decodeFileType = CVIAUDIO_UAC_TYPE_AAC;
		stUacSrcParam.stFileInParam.pfd = fopen("/tmp/test.mp3", "rb");
		stUacSrcParam.stFileInParam.pfd_save = fopen("/tmp/test_mp3dec.raw", "wb");
		cviaudio_uac_createSrc(CVIAUDIO_SRC_FILE_IN,
					CVIAUDIO_UAC_TYPE_MP3,
					&stUacSrcParam);

		cviaudio_uac_createDst(CVIAUDIO_DST_FILE_SAVE);
		cviaudio_uac_createDst(CVIAUDIO_DST_SPK_OUT);
		cviaudio_uac_bind(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_FILE_SAVE);
		cviaudio_uac_bind(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_SPK_OUT);
		printf("Keep press Enter key to leave the test\n");
		getchar();
		getchar();
		cviaudio_uac_unbind(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_SPK_OUT);
		cviaudio_uac_unbind(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_FILE_SAVE);
		cviaudio_uac_destroy(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_SPK_OUT);
		cviaudio_uac_destroy(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_FILE_SAVE);
	}
	break;

	case 6:
	{
		printf("read&decode /tmp/test.mp3 file and play out in USB headset(usb speaker)\n");
		stUacSrcParam.stFileInParam.decodeFileType = CVIAUDIO_UAC_TYPE_MP3;
		stUacSrcParam.stFileInParam.pfd = fopen("/tmp/test.mp3", "rb");
		stUacSrcParam.stFileInParam.pfd_save = NULL;
		cviaudio_uac_createSrc(CVIAUDIO_SRC_FILE_IN,
					CVIAUDIO_UAC_TYPE_MP3,
					&stUacSrcParam);
		cviaudio_uac_createDst(CVIAUDIO_DST_USB_OUT);//48k  1ch / 48 2ch  USB IN/OUT pcm_open(2,0,pcm_in)
		cviaudio_uac_bind(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_USB_OUT);
		printf("Keep press Enter key to leave the test\n");
		getchar();
		getchar();
		cviaudio_uac_unbind(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_USB_OUT);
		cviaudio_uac_destroy(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_USB_OUT);
	}
	break;

	case 7:
	{
		printf("read&decode /tmp/testaac.aac file and play out in USB headset(usb_speaker)\n");
		stUacSrcParam.stFileInParam.decodeFileType = CVIAUDIO_UAC_TYPE_AAC;
		stUacSrcParam.stFileInParam.pfd = fopen("/tmp/testaac.aac", "rb");
		stUacSrcParam.stFileInParam.pfd_save = NULL;
		cviaudio_uac_createSrc(CVIAUDIO_SRC_FILE_IN,
					CVIAUDIO_UAC_TYPE_AAC,
					&stUacSrcParam);

		cviaudio_uac_createDst(CVIAUDIO_DST_USB_OUT);//48k  1ch / 48 2ch  USB IN/OUT pcm_open(2,0,pcm_in)
		cviaudio_uac_bind(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_USB_OUT);
		printf("Keep press Enter key to leave the test\n");
		getchar();
		getchar();
		cviaudio_uac_unbind(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_USB_OUT);
		cviaudio_uac_destroy(CVIAUDIO_SRC_FILE_IN, CVIAUDIO_DST_USB_OUT);
	}
	break;

	case 8:
	{
		printf("mic in record(48/2chn) and play out in USB headset(usb speaker)\n");
		stUacSrcParam.stMicInParam.channels = 2;
		stUacSrcParam.stMicInParam.period_size = 480;
		stUacSrcParam.stMicInParam.rate = 48000;
		stUacSrcParam.stMicInParam.pfd_MicInSave = NULL;
		cviaudio_uac_createSrc(CVIAUDIO_SRC_MIC_IN,
					CVIAUDIO_UAC_TYPE_PCM,
					&stUacSrcParam);

		cviaudio_uac_createDst(CVIAUDIO_DST_USB_OUT);//48k  1ch / 48 2ch  USB IN/OUT pcm_open(2,0,pcm_in)
		cviaudio_uac_bind(CVIAUDIO_SRC_MIC_IN, CVIAUDIO_DST_USB_OUT);
		printf("Keep press Enter key to leave the test\n");
		getchar();
		getchar();
		cviaudio_uac_unbind(CVIAUDIO_SRC_MIC_IN, CVIAUDIO_DST_USB_OUT);
		cviaudio_uac_destroy(CVIAUDIO_SRC_MIC_IN, CVIAUDIO_DST_USB_OUT);
	}
	break;

	case 9:
	{
		char playfilename[128] = {0};

		printf("Enter play out path/filename(mp3)\n");
		scanf("%s", playfilename);

		FILE *fp = fopen(playfilename, "rb");

		int buffer_size = 512; //input size range[512~2048]bytes
		char *input_buffer = (char *)malloc(buffer_size);
		int read_size = 0;
		int ret = CVI_FAILURE;

		printf("simulate network stream in  /tmp/test.mp3 and play out in speaker\n");

		stUacSrcParam.stFileInParam.decodeFileType = CVIAUDIO_UAC_TYPE_MP3;
		cviaudio_uac_createSrc(CVIAUDIO_SRC_STREAM_IN,
					CVIAUDIO_UAC_TYPE_MP3,
					&stUacSrcParam);

		cviaudio_uac_createDst(CVIAUDIO_DST_SPK_OUT);//48k  1ch / 48 2ch  USB IN/OUT pcm_open(2,0,pcm_in)
		cviaudio_uac_bind(CVIAUDIO_SRC_STREAM_IN, CVIAUDIO_DST_SPK_OUT);



		while (1) {
			read_size = fread(input_buffer, 1,
						buffer_size,
						fp);
			if (read_size == 0) {
				printf("read mp3 file end ...prepare to leave\n");
				break;

			} else {

				ret = cviaudio_uac_stream_mode(input_buffer, read_size);
				if (ret != CVI_SUCCESS)
					printf("[Error][%s][%d]\n", __func__, __LINE__);

			}
		}
		free(input_buffer);
		fclose(fp);
	}
	break;

	case 10:
	{
		printf("usb in mic  and play out in speaker\n");

		cviaudio_uac_createSrc(CVIAUDIO_SRC_USB_IN,
					CVIAUDIO_UAC_TYPE_PCM,
					&stUacSrcParam);

		cviaudio_uac_createDst(CVIAUDIO_DST_SPK_OUT);//48k  1ch / 48 2ch  USB IN/OUT pcm_open(2,0,pcm_in)
		cviaudio_uac_bind(CVIAUDIO_SRC_USB_IN, CVIAUDIO_DST_SPK_OUT);
		printf("Keep press Enter key to leave the test\n");
		getchar();
		getchar();
		cviaudio_uac_unbind(CVIAUDIO_SRC_USB_IN, CVIAUDIO_DST_SPK_OUT);
		cviaudio_uac_destroy(CVIAUDIO_SRC_USB_IN, CVIAUDIO_DST_SPK_OUT);
	}
	break;

	default:
		printf("[Error]not support this test case[%d]\n", sample_case);
		break;
	}
	printf("leave the uacaudio sample program\n");
	cviaudio_uac_deinit();
	return 0;
}

