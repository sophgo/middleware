#include "cvi_audio_internal_test.h"


#define FLUSH_ONE_SECOND_RECORD_FRM 1
#define SAFE_FREE_BUF(OBJ) {if (NULL != OBJ) {free(OBJ); OBJ = NULL; } }


/* global variable */
typedef unsigned long snd_pcm_uframes_t;
typedef long snd_pcm_sframes_t;

#define RES_LIB_NAME "libcvi_RES1.so"

#define UNUSED_REF(X)  ((X) = (X))
#define DEFAULT_FORMAT	SND_PCM_FORMAT_S16
#define DEFAULT_MAX_FRM_SIZE (2048)
#define DEFAULT_CHANNEL_NUMBER 2
#define DEFAULT_BYTES_PER_SAMPLE 2//16bit = 2 bytes
#define DEFAULT_AEC_OUT_CHN_COUNT 1//AEC with 2 chn input, and 1 chn out
#define MAX_BYTES_PER_SAMPLE  4 //32bit = 4 bytes
#define BASIC_BUFFER_DEPTH 4
#define MAX_BUFFERING_DEPTH  300  /* #define CVI_MAX_AUDIO_FRAME_NUM		300 */
#define MAX_BUFFER_SETTING	(1 * 1024 * 1024)
//#define ENCODED_RING_BUFFER_SIZE  (144 * 1024)
#define MAX_BUFFER_FRAME_COUNT 300
//for ain_getframe / aout_send frame time mode usage
#define CVI_AUD_MASK_ERR	(0x00)
#define CVI_AUD_MASK_INFO	(0x01)
#define CVI_AUD_MASK_DBG	(0x02)

static int cviaud_dbg = 3;

#ifdef AUDIO_PRINT_WITH_GLOBAL_COMM_LOG  /* use global module id as keyword */
#define PRINTF(fmt, args...) \
	do { \
		if (cviaud_dbg > 0) \
			fprintf(stderr, "[cviaudio] "fmt, ##args);\
	} while (0)

#define ERR_PRINTF(fmt, ...) \
	do { \
		if (cviaud_dbg > 0) \
			CVI_TRACE(CVI_DBG_ERR, CVI_ID_AUD, "%s:%d:%s(): " fmt,\
			 __FILENAME__,\
			  __LINE__,\
			   __func__, \
			   ##__VA_ARGS__);\
	} while (0)

#define DBG_PRINTF(fmt, ...) \
	do { \
		if (cviaud_dbg > 1) \
		CVI_TRACE(CVI_DBG_DEBUG, CVI_ID_AUD, "%s:%d:%s(): " fmt,\
		 __FILENAME__, \
		 __LINE__, __func__, ##__VA_ARGS__);\
	} while (0)

#define TRA_PRINTF(fmt, ...) \
	do { \
		if (cviaud_dbg > 2) \
			CVI_TRACE(CVI_DBG_INFO, CVI_ID_AUD, "%s:%d:%s(): " fmt, \
			__FILENAME__, \
			__LINE__, __func__, ##__VA_ARGS__);\
	} while (0)

#define DUM_PRINTF(fmt, ...) \
	do { \
		if (cviaud_dbg > 3) \
			CVI_TRACE(CVI_DBG_NOTICE, CVI_ID_AUD, "%s:%d:%s(): " fmt,\
			 __FILENAME__,\
			  __LINE__, __func__, ##__VA_ARGS__);\
	} while (0)



#else  /*  use [cviaudio] key word  as printf macro*/




#define PRINTF(fmt, args...) \
	do { \
		if (cviaud_dbg >= CVI_AUD_MASK_ERR) \
			fprintf(stderr, "[cviaudio] "fmt, ##args);\
	} while (0)

#define ERR_PRINTF(fmt, args...) \
	do { \
		if (cviaud_dbg >= CVI_AUD_MASK_ERR) \
			fprintf(stderr, "[cviaudio][error][%s][%d] "fmt, __func__, __LINE__, ##args);\
	} while (0)

#define DBG_PRINTF(fmt, args...) \
	do { \
		if (cviaud_dbg >= CVI_AUD_MASK_DBG) \
			fprintf(stderr, "[cviaudio][dbg] "fmt, ##args);\
	} while (0)

#define AUD_INFO_PRINTF(fmt, args...) \
	do { \
		if (cviaud_dbg >= CVI_AUD_MASK_INFO) \
			fprintf(stderr, "[cviaudio][info] "fmt, ##args);\
	} while (0)

#define AUD_TRA_PRINTF(fmt, args...) \
	do { \
		if (cviaud_dbg >= CVI_AUD_MASK_DBG) \
			fprintf(stderr, "[cviaudio][tra][%s][%d] "fmt, __func__, __LINE__, ##args);\
	} while (0)

#define DUM_PRINTF(fmt, args...) \
	do { \
		if (cviaud_dbg >= CVI_AUD_MASK_DBG) \
			fprintf(stderr, "[cviaudio][dump] "fmt, ##args);\
	} while (0)
#endif


CVI_S32 cvi_audio_set_dbg_level(CVI_S32 dbglevel)
{
	PRINTF("current level : %d\n", cviaud_dbg);

	if (dbglevel < 0 || dbglevel > 4) {
		PRINTF("invalid debug level[%d]\n", dbglevel);
		return 0;
	}

	cviaud_dbg = dbglevel;
	PRINTF("set debug level : %d\n", cviaud_dbg);
	return CVI_SUCCESS;
}

int check_param(struct pcm_params *params, unsigned int param, unsigned int value,
		       char *param_name, char *param_unit)
{
	unsigned int min;
	unsigned int max;
	int is_within_bounds = 1;

	min = pcm_params_get_min(params, param);

	if (value < min) {
		fprintf(stderr, "%s is %u%s, device only supports >= %u%s\n", param_name, value,
			param_unit, min, param_unit);
		is_within_bounds = 0;
	}

	max = pcm_params_get_max(params, param);

	if (value > max) {
		fprintf(stderr, "%s is %u%s, device only supports <= %u%s\n", param_name, value,
			param_unit, max, param_unit);
		is_within_bounds = 0;
	}

	return is_within_bounds;
}


int _transform_pcm_to_wave(const char *pcmpath, int channels,
				  int sample_rate,
				  int bits_per_sample,
				  const char *wavepath)
{
	PRINTF("pcm to wav ch:%d smp_rate:%d bits_in_sample:%d filename%s\n",
	       channels,
	       sample_rate,
	       bits_per_sample,
	       wavepath);

	typedef struct WAVE_HEADER {
		char	fccID[4];//內容為"RIFF"
		unsigned int dwSize;//最後填寫，WAVE格式音訊的大小
		char	fccType[4];//內容為"WAVE"
	} WAVE_HEADER;

	typedef struct WAVE_FMT {
		char	fccID[4];		   //內容為"fmt "
		unsigned int  dwSize;	  //內容為WAVE_FMT佔的位元組數，為16
		short int wFormatTag; //如果為PCM，改值為 1
		short int wChannels;  //通道數，單通道=1，雙通道=2
		unsigned int  dwSamplesPerSec;//取樣頻率
		unsigned int  dwAvgBytesPerSec;/* ==dwSamplesPerSec*wChannels*uiBitsPerSample/8 */
		short int wBlockAlign;//==wChannels*uiBitsPerSample/8
		short int uiBitsPerSample;//每個取樣點的bit數，8bits=8, 16bits=16
	} WAVE_FMT;

	typedef struct WAVE_DATA {
		char	fccID[4];		//內容為"data"
		unsigned int dwSize;   //==NumSamples*wChannels*uiBitsPerSample/8
	} WAVE_DATA;

	if (channels == 0 || sample_rate == 0) {
		channels = 1;
		sample_rate = 16000;
	}

	WAVE_HEADER pcmHEADER;
	WAVE_FMT	pcmFMT;
	WAVE_DATA	pcmDATA;
	short int m_pcmData;
	FILE *fp, *fpout;

	fp = fopen(pcmpath, "rb");

	if (fp == NULL) {
		PRINTF("Open pcm file error.\n");
		return -1;
	}

	fpout = fopen(wavepath, "wb");

	if (fpout == NULL) {
		PRINTF("Create wav file error.\n");

		if (fp != NULL)
			fclose(fp);

		return -1;
	}

	/* WAVE_HEADER */
	memcpy(pcmHEADER.fccID, "RIFF", 4);
	memcpy(pcmHEADER.fccType, "WAVE", 4);
	fseek(fpout, sizeof(WAVE_HEADER), 1);	//1=SEEK_CUR
	/* WAVE_FMT */
	memcpy(pcmFMT.fccID, "fmt ", 4);
	pcmFMT.dwSize = 16;
	pcmFMT.wFormatTag = 0x0001;
	pcmFMT.wChannels = channels;//1;
	pcmFMT.dwSamplesPerSec = sample_rate;//16000;
	pcmFMT.uiBitsPerSample = bits_per_sample;
	/* ==dwSamplesPerSec*wChannels*uiBitsPerSample/8 */
	pcmFMT.dwAvgBytesPerSec = pcmFMT.dwSamplesPerSec * pcmFMT.wChannels * pcmFMT.uiBitsPerSample / 8;
	/* ==wChannels*uiBitsPerSample/8 */
	pcmFMT.wBlockAlign = pcmFMT.wChannels * pcmFMT.uiBitsPerSample / 8;
	fwrite(&pcmFMT, sizeof(WAVE_FMT), 1, fpout);
	/* WAVE_DATA */
	memcpy(pcmDATA.fccID, "data", 4);

	pcmDATA.dwSize = 0;
	fseek(fpout, sizeof(WAVE_DATA), 1);
	fread(&m_pcmData, sizeof(short int), 1, fp);

	while (!feof(fp)) {
		pcmDATA.dwSize += sizeof(short int);
		fwrite(&m_pcmData, sizeof(short int), 1, fpout);
		fread(&m_pcmData, sizeof(short int), 1, fp);
	}

	pcmHEADER.dwSize = 36 + pcmDATA.dwSize;
	rewind(fpout);
	fwrite(&pcmHEADER, sizeof(WAVE_HEADER), 1, fpout);
	fseek(fpout, sizeof(WAVE_FMT), SEEK_CUR);
	fwrite(&pcmDATA, sizeof(WAVE_DATA), 1, fpout);
	fclose(fp);
	fclose(fpout);
	return 0;
}


CVI_S32 _preflush_audin_frm_record(struct pcm *capture_handle,
		CVI_S32 period_size,
		CVI_S32 preiod_size_inBytes,
		CVI_S32 sample_rate)
{
	CVI_S32 s32Cnt = 0;

	s32Cnt = sample_rate / period_size;//one second
	s32Cnt = s32Cnt * 2; //flush 2 second

	if (s32Cnt == 0)
		return CVI_FAILURE;

	CVI_CHAR *buffer;

	buffer = malloc(preiod_size_inBytes * 2);

	while (s32Cnt > 0) {
		/* read but not save */
		pcm_read(capture_handle, buffer, preiod_size_inBytes);
		s32Cnt--;
	}

	PRINTF("record pre - flush set[%d][%d][%d]\n", sample_rate, period_size, s32Cnt);
	SAFE_FREE_BUF(buffer);
	return CVI_SUCCESS;
}


CVI_S32 _cvi_audio_recordfile_sample(CVI_S32 DevId, CVI_S32 rate, CVI_S32 channel, ST_AudioUnitTestCfg *testCfg)
{
	CVI_S32 err;
	CVI_CHAR *buffer;
	CVI_S32 period_size = 960;
	CVI_S32 s32sample_rate = 0;
	CVI_S32 record_second = 12;
	CVI_S32 period_count = 2;
	struct pcm_config capture_config;
	struct pcm *capture_handle;
	enum pcm_format format = PCM_FORMAT_S16_LE;
	CVI_S32 card = 0;
	CVI_S32 device = 0;


	if (rate == 11025) {
		PRINTF("Force set period_size to 1378 for rate:11025\n");
		period_size = 1378;
	}

	if (rate == 8000) {
		PRINTF("Force set period_size to 250 for rate:8000\n");
		period_size = 250;
	}
#ifdef ARCH_CV183X
	printf("ARCH_CV183X\n");
	//period_size = 960;
	if (rate == 8000) {
		PRINTF("Force set buffer size 4000 for 8k\n");
		period_count = 4000 / period_size;

		if (period_count <= 0)
			period_count = 10;
	}
#else
	printf("ARCH_CV182X\n");
	period_size = 320;
	period_count = 2;
#endif
	if (testCfg->bOptCfg == CVI_TRUE) {
		record_second = (testCfg->Time_in_second == 0) ? 12 : testCfg->Time_in_second;
		period_size = (testCfg->period_size == 0) ? period_size : testCfg->period_size;

	} else {
		PRINTF("Enter record seconds (1~12) : ");
		err = scanf("%d", &record_second);

		if (err == EOF)
			ERR_PRINTF("\n");

		printf("\n");
	}


	s32sample_rate = rate;

	if (DevId == 0)
		card  = 0;
	else if (DevId == 1)
		card = 1;
	else {
		printf("card id = 2\n");
		card = 2;
	}

	memset(&capture_config, 0, sizeof(capture_config));
	capture_config.channels = channel;
	capture_config.rate = rate;
	printf("check period_size period cnt[%d][%d]\n", period_size, period_count);
	capture_config.period_size = period_size;
	capture_config.period_count = period_count;
	capture_config.format = format;
	capture_config.start_threshold = 0;
	capture_config.stop_threshold = 2147483647;
	capture_config.silence_threshold = 0;

	capture_handle = pcm_open(card, device, PCM_IN, &capture_config);

	if (!capture_handle || !pcm_is_ready(capture_handle)) {
		ERR_PRINTF("Unable to open PCM device (%s)\n",
			   pcm_get_error(capture_handle));
		return CVI_FAILURE;
	}

#if FLUSH_ONE_SECOND_RECORD_FRM
	_preflush_audin_frm_record(capture_handle, period_size, (period_size * channel * 2), s32sample_rate);
#endif
	DBG_PRINTF("audio interface prepared\n");
	CVI_S32	size = pcm_frames_to_bytes(capture_handle, pcm_get_buffer_size(capture_handle));

	printf("[pcm_get_frame_size][%d]\n", size);
	buffer = malloc(size);

	DBG_PRINTF("[DEBUG]buffer allocated size[%d] period_size[%d] channel[%d]\n", size, period_size, channel);


	CVI_S32 loops = (record_second * rate) / period_size; // record for 12 second
	//loops = loops / period_count;

	//size = period_size * channel * 2;
	size =	pcm_frames_to_bytes(capture_handle, period_size);


	printf("tinyalsa loops[%d] second[%d] rate[%d] divider[]%d]size[%d]p_count[%d]\n",
	       loops, record_second, rate, period_size, size, period_count);


	FILE *fp_ain = NULL;

	fp_ain =  fopen("record.raw", "wb");

	if ((fp_ain) == NULL) {
		PRINTF("Cannot open input file\n");
		exit(2);
	}

	//PRINTF("start to record %d seconds[%d]\n", record_second);
	PRINTF("***********************************************************************\n");

	//PRINTF("**************Always record to 38400x2 bytes to file********************\n");
	while (loops > 0) {
		//while (sizeleft > 0) {
		loops--;
		err = pcm_read(capture_handle, buffer, size);

		if (err > 0) {
			/* EPIPE means overrun */
			ERR_PRINTF("[%s]\n", pcm_get_error(capture_handle));
		}
		fwrite(buffer, 1, size, fp_ain);
	}

	if (fp_ain != NULL)
		fclose(fp_ain);

	SAFE_FREE_BUF(buffer);
	fprintf(stdout, "[DEBUG]buffer freed\n");
	pcm_close(capture_handle);
	PRINTF("start to record %d finish\n", record_second);
	return s32sample_rate;
}

CVI_BOOL _cvi_checkname_iswav(char *infilename)
{
	CVI_S32 s32InputFileLen = 0;
	CVI_S32 s32Read = 0;

	ST_CVI_WAV_HEADER wavHead;
	FILE *ptr;
	CVI_U8 buffer4[4];

	s32InputFileLen = strlen(infilename);

	if (s32InputFileLen == 0) {
		ERR_PRINTF("No Input File Name..force return\n");
		return 0;
	}


	/* Step1: Identify the file by the file extension */
	if (infilename[s32InputFileLen - 4] == '.' &&
	    (infilename[s32InputFileLen - 3] == 'W' || infilename[s32InputFileLen - 3] == 'w') &&
	    (infilename[s32InputFileLen - 2] == 'A' || infilename[s32InputFileLen - 2] == 'a') &&
	    (infilename[s32InputFileLen - 1] == 'V' || infilename[s32InputFileLen - 1] == 'v')) {
		AUD_TRA_PRINTF("Enter wav file\n");
		return CVI_TRUE;
		/* Only judge by the extension */
	} else {
		/* Do Not return directly, do the header parsing */
		/* return CVI_FALSE; */
		//DBG_PRINTF("Do not see the file extension name\n");
	}

	/* Step2: Check the file by the file header */
	ptr = fopen(infilename, "rb");

	if (ptr == NULL) {
		ERR_PRINTF("\n");
		return CVI_FALSE;
	}

	s32Read = fread(wavHead.riff, sizeof(wavHead.riff), 1, ptr);
	s32Read = fread(buffer4, sizeof(buffer4), 1, ptr);
	s32Read = fread(wavHead.wave, sizeof(wavHead.wave), 1, ptr);

#if 0 //only for debug
	strncmp(WavHead.chunkID, "RIFF", 4) ||
	strncmp(WavHead.format, "WAVE", 4) ||
	strncmp(WavHead.subchunkID, "fmt", 3) ||
	strncmp(WavHead.dataID, "data", 4) ||
	WavHead.audioFormat != 1 ||
	WavHead.bitDepth != 16
#endif

	s32Read = strncmp((const char *)wavHead.wave, "WAVE", 4);

	if (s32Read == 0) {
		/* if not a wav file, wavHEAD.wave will be random character */
		AUD_TRA_PRINTF("(9-12) Wave marker_: %.*s\n", 4, wavHead.wave);
		return CVI_TRUE;
	} else
		return CVI_FALSE;
}

int sample_is_playable(unsigned int card, unsigned int device, unsigned int channels,
			      unsigned int rate, unsigned int bits, unsigned int period_size,
			      unsigned int period_count)
{
	struct pcm_params *params;
	int can_play;

	params = pcm_params_get(card, device, PCM_OUT);

	UNUSED_REF(channels);

	if (params == NULL) {
		fprintf(stderr, "Unable to open PCM device %u.\n", card);
		return 0;
	}

	can_play = check_param(params, PCM_PARAM_RATE, rate, "Sample rate", "Hz");
	//can_play &= check_param(params, PCM_PARAM_CHANNELS, channels, "Sample", " channels");
	can_play &= check_param(params, PCM_PARAM_SAMPLE_BITS, bits, "Bitrate", " bits");
	can_play &= check_param(params, PCM_PARAM_PERIOD_SIZE, period_size, "Period size", " frames");
	can_play &= check_param(params, PCM_PARAM_PERIODS, period_count, "Period count", " periods");
	pcm_params_free(params);

	return can_play;
}


CVI_S32 _cvi_audio_playfile_sample3(CVI_CHAR *filename, CVI_S32 DevId, ST_AudioUnitTestCfg *testCfg)
{
//sample code
	CVI_U32 pcm;
	CVI_S32 rate = 8000;
	CVI_S32 u32BitDepthSelect = 16;
	CVI_S32 channels = 1;
	CVI_S32 seconds = 12;
	CVI_S32 s32BytesPerSample = 2;
	CVI_S32 err;

	short wav_header[44];

	struct pcm_config pcm_config;
	struct pcm *pcm_handle;
	snd_pcm_uframes_t frames = 960;
	snd_pcm_uframes_t savefrm = frames;
	CVI_CHAR *buff;
	CVI_CHAR *zero_buff;
	CVI_S32 buff_size, loops;
	FILE *fp = NULL;
	CVI_S32 card = 1;
	CVI_S32 device = 0;

#ifdef ARCH_CV183X
	frames = 960;
#else
	printf("ARCH_CV182X\n");
	frames = 320;
#endif
	/* Open the file first */
	if (access(filename, 0) >=	0)
		printf("check file[%s] exist!\n", filename);
	else {
		printf("[Error]check file[%s] not exist!....error\n", filename);
		return CVI_FAILURE;
	}


	fp = fopen(filename, "rb");

	if (!fp) {
		PRINTF("cannot open file [%s] , force return\n", filename);
		return CVI_FAILURE;
	}

	/* Check the wav format ----start */
	CVI_BOOL isWav = _cvi_checkname_iswav(filename);

	if (isWav == CVI_TRUE) {
		PRINTF("Check format wav file header inside...\n");
		fread(&wav_header[0], 1, 44, fp);/* wav header	*/

		ST_CVI_WAV_HEADER  *stWaveCheck = (ST_CVI_WAV_HEADER *)&wav_header[0];

		channels = stWaveCheck->channels;
		rate = stWaveCheck->sample_rate;
		u32BitDepthSelect =  stWaveCheck->bits_per_sample;
		PRINTF("Parsing chn[%d] rate[%d] block_align[0x%x] bitspersample[0x%x]\n",
		       channels, rate, stWaveCheck->block_align, u32BitDepthSelect);
		PRINTF("Detect bit depth [%d]\n", u32BitDepthSelect);
	} else {
		/* Not a wav file, need to fill up the format by user -------------start */

		if (testCfg->bOptCfg == CVI_TRUE) {
			PRINTF("\n");
			seconds = (testCfg->Time_in_second == 0) ? 12 : testCfg->Time_in_second;
			rate =	(testCfg->sample_rate == 0) ? 8000 : testCfg->sample_rate;
			channels =	(testCfg->channels == 0) ? 1 : testCfg->channels;
			u32BitDepthSelect = (testCfg->bitdepth == 0) ? 16 : testCfg->bitdepth;
			frames = (testCfg->period_size == 0) ? frames : (snd_pcm_uframes_t)testCfg->period_size;
			savefrm = frames;
		} else {
			PRINTF("\n");
			PRINTF("Enter playback seconds (1~12) : ");
			err = scanf("%d", &seconds);
			PRINTF("\n");
			PRINTF("Enter sample rate ");
			err = scanf("%d", &rate);
			PRINTF("\n");
			PRINTF("Enter channel numbers: 1/2:");
			err = scanf("%d", &channels);
			PRINTF("\n");
			PRINTF("[%s] chn[%d] rate[%d]\n", __func__, channels, rate);
			PRINTF("Enter enter bitdepth : 16/ 24 / 32:");
			err = scanf("%d", &u32BitDepthSelect);

			if (err == EOF)
				ERR_PRINTF("\n");
		}
		/* Not a wav file, need to fill up the format by user -------------end */
	}


	if (DevId == 0)
		card = 0;
	else if (DevId == 1)
		card = 1;
	else if (DevId == 2)
		card = 2;
	else
		card = 1;

	if (!sample_is_playable(card, device, channels, rate, u32BitDepthSelect, frames, 4)) {
		ERR_PRINTF("Not open playout device/card success!!!\n");

		return CVI_FAILURE;
	}


	memset(&pcm_config, 0, sizeof(pcm_config));
	pcm_config.channels = channels;
	pcm_config.rate = rate;
	pcm_config.period_size = frames;
	pcm_config.period_count = 4;

	if (u32BitDepthSelect == 32)
		pcm_config.format = PCM_FORMAT_S32_LE;
	else if (u32BitDepthSelect == 24)
		pcm_config.format = PCM_FORMAT_S24_3LE;
	else if (u32BitDepthSelect == 16)
		pcm_config.format = PCM_FORMAT_S16_LE;

	pcm_config.start_threshold = 0;
	pcm_config.stop_threshold = 0;
	pcm_config.silence_threshold = 0;


	pcm_handle = pcm_open(card, device, PCM_OUT, &pcm_config);

	if (!pcm_handle || !pcm_is_ready(pcm_handle)) {
		ERR_PRINTF("Unable to open PCM card %d device %u (%s)\n",
			card, device, pcm_get_error(pcm_handle));

		return CVI_FAILURE;
	}

	buff_size = frames * channels * s32BytesPerSample /* 2 bytes per sample */;
	buff = (CVI_CHAR *) malloc(buff_size);
	zero_buff = (CVI_CHAR *) malloc(buff_size);
	memset(zero_buff, 0, buff_size);

	//time = rate / frames; //how many frame get in one second
	//time = 1000 / time; //(ms/ get-frame)
	//loops = (seconds * 1000) / time;
	loops = seconds * rate / frames;

	while (1) {
		if (isWav == CVI_FALSE) {
			loops--;

			if (loops <= 0) {
				PRINTF("Up to time -->play out over\n");
				break;
			}
		}

		pcm = fread(buff, 1, buff_size, fp);

		if (isWav == CVI_TRUE &&
		    pcm < (size_t)buff_size &&
		    pcm > 0) {
			memcpy(zero_buff, buff, pcm);
			memcpy(buff, zero_buff, buff_size);
		}

		if (pcm <= 0) {
			if (isWav == CVI_TRUE) {
				PRINTF("nothing to read out from file , force exit [%d]\n", pcm);
				break;

			} else {
				PRINTF("padding 0 to target seconds\n");
				memset(buff, 0, buff_size);
			}
		}


		if ((pcm < (size_t)buff_size) && (channels != 0)) {
			/*Debug usage*/
			frames = (pcm / channels);
			frames = (frames / s32BytesPerSample);

			if (frames == 0 && isWav == CVI_FALSE) {
				/* Padding 0 in buffer */
				frames = savefrm;
			}

			PRINTF("size mismatch  got to EOF-->play out over [%d] [%d]frm[%d]\n",\
			pcm, buff_size, (int)frames);

		} else {
			/* Debug usage */
			/* PRINTF("keep play -->play out  [%d] [%d] frm[%d]\n", pcm, buff_size, frames); */
		}

		//if (pcm = read(fp, buff, buff_size) == 0) {
		//	PRINTF("Early end of file.\n");
		//	return CVI_SUCCESS;
		//}

		pcm = pcm_write(pcm_handle, buff, buff_size);

		if (pcm > 0)
			ERR_PRINTF("[%s]\n", pcm_get_error(pcm_handle));
	}

	fclose(fp);
	PRINTF("drain and close\n");
	pcm_close(pcm_handle);
	SAFE_FREE_BUF(buff);
	SAFE_FREE_BUF(zero_buff);
	return CVI_SUCCESS;
}

int _aud_scanf(char *printout, int *getvalue, int default_value)
{
	char charbuf[10];

	printf(printout);
	if (getvalue == NULL) {
		printf("_aud_scanf input null buffer\n");
		return CVI_FAILURE;
	}
	fflush(stdin);
	fgets(charbuf, 10, stdin);
	if (charbuf[0] == '\n') {
		//using default value
		*getvalue = default_value;
	} else {
		//using fgets value
		*getvalue = atoi(charbuf);
	}
	printf("\n");
	fflush(stdin);
	return CVI_SUCCESS;
}


int _aud_scanfchar(char *printout, char *getvalue, char *default_char)
{
#define AUD_SCAN_MAX_CHAR_CNT 128
	char charbuf[AUD_SCAN_MAX_CHAR_CNT] = "";
	int index = 0;
	char single_char;

	printf(printout);
	if (getvalue == NULL) {
		printf("_aud_scanfchar input null buffer\n");
		return CVI_FAILURE;
	}

	fflush(stdin);
	while (1) {
		single_char = getchar();
		//if ((single_char == '\n') || (single_char == EOF)) {
		if (single_char == '\n') {
			if (index == 0)
				strcpy(getvalue, default_char);
			else {
				charbuf[index++] = '\0';
				strcpy(getvalue, charbuf);
			}

			break;

		} else {
			//filling char one by one
			charbuf[index++] = single_char;
		}
		if (index == (AUD_SCAN_MAX_CHAR_CNT - 1)) {
			printf("input string oversize\n");
			break;
		}
	}

	printf("\n");
	fflush(stdin);
	return CVI_SUCCESS;
}


CVI_S32 cvi_audio_set_dbg_option(ST_AudioUnitTestCfg *testCfg)
{
	printf("bOptCfg[%d]\n", (int)testCfg->bOptCfg);
	if (testCfg->bOptCfg == CVI_TRUE) {
		//set debug level
		printf("setup the debug level\n");
		//int getValue = 0x00;
		int setValue = 0x00;

		//getValue = _cviAudGetEnv("cviaudio_level", "%d", NULL);
		//printf("[CVIAUDIO]cviaudio_level current_level = [%d]\n", getValue);
		printf("Choose level: 0:error, 1:dbg, 2:info, 3:trace\n");
		_aud_scanf("Default:1\t", &setValue, 1);
		cvi_audio_set_dbg_level(setValue);
		if (setValue == 1) {
			printf("export cviaudio_level=1\n");
			system("export cviaudio_level=1");
		} else if (setValue == 2) {
			printf("export cviaudio_level=2\n");
			system("export cvuaudio_level=2");
		} else if (setValue == 3) {
			printf("export cviaudio_level=3\n");
			system("export cviaudio_level=3");
		} else if (setValue == 0) {
			printf("export cviaudio_level=0\n");
			system("export cviaudio_level=0");
		} else {
			printf("none set any debug level\n");
			printf("set in value[%d]\n", setValue);
		}

	} else {
		//do the pcm test
		printf("Do the pcm test\n");
		CVI_S32 s32GetVal = -1;
		CVI_S32 err = 0;
		CVI_BOOL bRecord = CVI_TRUE;
		CVI_S32 s32CardId = 0;
		CVI_S32 s32DevId = 0;
		CVI_S32 s32Rate = 8000;
		CVI_S32 s32Channels = -1;
		CVI_S32 s32PeriodSize = 480;
		CVI_S32 s32PeriodCnt = 2;
		CVI_S32 s32Seconds = 1;
		char filename[128];
		char *buffer = NULL;
		int buffersize = 0;
		FILE *fp_tmp = NULL;
		struct pcm_config pcm_config;
		struct pcm *pcm_handle = NULL;
		//1.prepare requiring input parameters
		_aud_scanf("Enter Record:0 Play:1 [default:0]\t", &s32GetVal, 0);
		if (s32GetVal == 0)
			bRecord = CVI_TRUE;
		else
			bRecord = CVI_FALSE;

		_aud_scanf("Enter CardId[default:0]\t", &s32CardId, 0);
		_aud_scanf("Enter Device Id[default:0]\t", &s32DevId, 0);
		_aud_scanf("Enter sample rate[default:8000]\t", &s32Rate, 8000);
		_aud_scanf("Enter channel numbers(1, 2)[default:1]\t", &s32Channels, 1);
		_aud_scanf("Enter period size[default:960]\t", &s32PeriodSize, 960);
		_aud_scanf("Enter period count[default:4]\t", &s32PeriodCnt, 4);
		if (bRecord == CVI_TRUE) {
			printf("test 0306\n");
			_aud_scanf("Enter record seconds[default:10]\t", &s32Seconds, 10);
			_aud_scanfchar("Enter record filename[default:record_pcm.raw]\t", filename, "record.raw");
			printf("test 0306\n");
		} else {
			printf("test 0306\n");
			_aud_scanf("Enter play out  seconds[default:10]\t", &s32Seconds, 10);
			_aud_scanfchar("Enter play filename[default:record.raw]\t", filename, "record.raw");
		}
		//2.start do the pcm test
		memset(&pcm_config, 0, sizeof(struct pcm_config));
		pcm_config.channels = s32Channels;
		pcm_config.rate = s32Rate;
		pcm_config.period_size = s32PeriodSize;
		pcm_config.period_count = s32PeriodCnt;
		pcm_config.format = PCM_FORMAT_S16_LE;
		pcm_config.start_threshold = 0;
		pcm_config.stop_threshold = 2147483647;
		pcm_config.silence_threshold = 0;
		if (bRecord == CVI_TRUE) {
			pcm_handle = pcm_open(s32CardId, s32DevId, PCM_IN, &pcm_config);
			printf("\n\nbRecord[%d], sample rate[%d] channels[%d]",
				(int)bRecord, s32Rate, s32Channels);
			printf("periodsize[%d]periodcount[%d] filename[%s] sec[%d]\n\n",
				s32PeriodSize, s32PeriodCnt, filename, s32Seconds);
			if (!pcm_handle || !pcm_is_ready(pcm_handle)) {
				printf("[Error]Unable to open PCM error(%s)\n",
					pcm_get_error(pcm_handle));
				return CVI_FAILURE;

			} else
				printf("pcm open success\n");

			buffersize = pcm_get_buffer_size(pcm_handle);
			printf("[buffersize][%d]\n", buffersize);
			buffer = malloc(buffersize);

			int loops = (s32Seconds * s32Rate) / s32PeriodSize;

			fp_tmp = fopen(filename, "wb");
			if (fp_tmp == NULL) {
				printf("open file error[%s]\n", filename);
				return CVI_FAILURE;
			}

			while (loops--) {
				err = pcm_read(pcm_handle, buffer, buffersize);
				if (err > 0) {
					//pcm_read error
					printf("pcm_read error:[%s]\n", pcm_get_error(pcm_handle));
				}
				fwrite(buffer, 1, buffersize, fp_tmp);
			}
			fclose(fp_tmp);
			SAFE_FREE_BUF(buffer);
			pcm_close(pcm_handle);
			printf("pcm_read finished !!!\n");
			return CVI_SUCCESS;

		} else {
			if (access(filename, 0) >= 0) {
				printf("[%s]exist\n", filename);
				fp_tmp = fopen(filename, "rb");
			} else {
				printf("[%s]not exist to play\n", filename);
				return CVI_FAILURE;
			}

			pcm_handle = pcm_open(s32CardId, s32DevId, PCM_OUT, &pcm_config);
			printf("bRecord[%d], sample rate[%d] channels[%d]",
				(int)bRecord, s32Rate, s32Channels);
			printf("periodsize[%d]periodcount[%d] filename[%s] sec[%d]\n",
				s32PeriodSize, s32PeriodCnt, filename, s32Seconds);
			if (!pcm_handle || !pcm_is_ready(pcm_handle)) {
				printf("[Error]Unable to open PCM error(%s)\n",
					pcm_get_error(pcm_handle));
				return CVI_FAILURE;

			} else
				printf("pcm open success\n");

			buffersize = s32Channels * DEFAULT_BYTES_PER_SAMPLE * s32PeriodSize;
			printf("[buffersize][%d]\n", buffersize);
			buffer = malloc(buffersize);
			char *zero_buff = (char *) malloc(buffersize);

			memset(zero_buff, 0, buffersize);
			int loops = s32Seconds * s32Rate / s32PeriodSize;

			while (loops--) {
				err = fread(buffer, 1, buffersize, fp_tmp);

				if (err < buffersize && err > 0) {
					memcpy(zero_buff, buffer, err);
					memcpy(buffer, zero_buff, buffersize);
				}

				if (err <= 0) {
					printf("padding 0 to target seconds\n");
					memset(buffer, 0, buffersize);
				}

				err = pcm_write(pcm_handle, buffer, buffersize);
				if (err > 0) {
					//pcm_read error
					printf("pcm_write error:[%s]\n", pcm_get_error(pcm_handle));
				}
			}
			fclose(fp_tmp);
			PRINTF("pcm out finished!!!\n");
			pcm_close(pcm_handle);
			SAFE_FREE_BUF(buffer);
			SAFE_FREE_BUF(zero_buff);

		}

	}

	return CVI_SUCCESS;
}


CVI_S32 cvi_audio_set_dbg_option2(ST_AudioUnitTestCfg *testCfg)
{
	PRINTF("-----------dump audio unit test user option[start]\n");
	printf("bOptCfg[%d]\n", (int)testCfg->bOptCfg);
	printf("channels[%d]\n", (int)testCfg->channels);
	printf("Time_in_second[%d]\n", (int)testCfg->Time_in_second);
	printf("sample_rate[%d]\n", (int)testCfg->sample_rate);
	printf("format[%s]\n", testCfg->format);
	printf("period_size[%d]\n", (int)testCfg->period_size);
	printf("bitdepth[%d]\n", (int)testCfg->bitdepth);
	printf("filename[%s]\n", testCfg->filename);
	PRINTF("-----------dump audio unit test user option[end]\n");
	return CVI_SUCCESS;
}


CVI_S32 cvi_audio_set_dbg_record(ST_AudioUnitTestCfg *testCfg)
{

	PRINTF("Enter [%s] [%d]\n", __func__, __LINE__);
#define SINGLE_VQE_FRAME_SAMPLE 160
	CVI_S32 idselect = 0;
	CVI_S32 rate = 8000;//16000;
	CVI_S32 s32Ret;
	CVI_S32 channel = 1;
	CVI_S32 err;

	char output_filenamewav1[512];
	char output_filenamewav2[512];

	snprintf(output_filenamewav1, 512, "%s.wav", "record");
	snprintf(output_filenamewav2, 512, "%s.wav", "vqeplay");

	if (testCfg->bOptCfg == CVI_TRUE) {
		PRINTF("user option mode\n");
		cvi_audio_set_dbg_option2(testCfg);
		idselect = 0;
		rate =	(testCfg->sample_rate == 0) ? rate : testCfg->sample_rate;
		channel = (testCfg->channels == 0) ? channel : testCfg->channels;
	} else {

		PRINTF("----------------------cvi check------------------------\n");
		PRINTF("Enter available card id: 0, 1, 2 ??\n");
		PRINTF("Current available card ....\n");
		PRINTF("\n enter: \t");

		err = scanf("%d", &idselect);
		PRINTF("\n");
		PRINTF("Enter record sample rate  : ");
		err = scanf("%d", &rate);

		if (rate > 64000) {
			PRINTF("sample rate not support above 64k\n");
			PRINTF("exit......\n");
		}

		PRINTF("\n");
		PRINTF("Enter record channel 1 ?  2? : ");
		err = scanf("%d", &channel);

		if ((channel != 1) && (channel != 2)) {
			PRINTF("using defalut value 1\n");
			channel = 1;
		}

		if (err == EOF)
			ERR_PRINTF("\n");
	}

	PRINTF("\n chn[%d] rate[%d]\n", channel, rate);

	if (idselect == 0)
		s32Ret = _cvi_audio_recordfile_sample(0, rate, channel, testCfg);
	else if (idselect == 1)
		s32Ret = _cvi_audio_recordfile_sample(1, rate, channel, testCfg);
	else
		s32Ret = _cvi_audio_recordfile_sample(2, rate, channel, testCfg);


	if (s32Ret != CVI_FAILURE)
		rate = s32Ret;
	else {
		PRINTF("Something wrong while open pcm ...please check\n");
		/* Error Handle  */
	}

	PRINTF("[%s] record confirm rate[%d]\n", __func__, rate);
	//start to transfer the record.raw to  file after NR algo effect
	CVI_S32 u32UsrFrmDepth = 1;
	short wav_header[44];
	CVI_S32 count = 0;
	CVI_S32 hopsize = 960;
	/* step 1 presetting*/
	FILE *fp_test_input = NULL;
	FILE *fp_test_output = NULL;

	fp_test_input = fopen("record.raw", "rb");

	if ((fp_test_input) == NULL) {
		PRINTF("Cannot open input file\n");
		exit(2);
	}

	fp_test_output = fopen("vqeplay.raw", "wb");

	if ((fp_test_output) == NULL) {
		PRINTF("Cannot open output file\n");
		exit(2);
	}

	/* step 2 simulate buffer condition*/
	CVI_CHAR *pWrite = (CVI_CHAR *)(CVI_NULL);
	CVI_CHAR *audio_buffer = NULL;
	CVI_S32 s32Size = MAX_BYTES_PER_SAMPLE *
			  DEFAULT_MAX_FRM_SIZE *
			  DEFAULT_CHANNEL_NUMBER *
			  BASIC_BUFFER_DEPTH;
	audio_buffer = (CVI_CHAR *)malloc(s32Size);

	if (audio_buffer != NULL)
		pWrite = audio_buffer;
	else
		PRINTF("fatal error while allocate buffer[%s][%d]\n", __func__, __LINE__);


	/* step3 init*/
	fread(&wav_header[0], 1, 44, fp_test_input);		/* wav header  */

	if ((rate == 8000) || (rate == 16000)) {
		/* record file already success */
		/* Do not need to update sample rate again */
	} else {
		PRINTF("rate not equal to 8000 or 16000Hz, skip algo process\n");
		goto SKIP_ALGO;
	}

	PRINTF("\n");
	PRINTF("Enter vqe (Enter 1 or 0)?? (1:yes / 0:no) ");

	s32Ret = 1;
	AI_TALKVQE_CONFIG_S pstAiVqeAttr;

	hopsize = 160;
	memset(&pstAiVqeAttr, 0, sizeof(AI_TALKVQE_CONFIG_S));
	/* Default vqe setting ..................start */

#ifdef NEXT_SSP_ALGO
	AI_AEC_CONFIG_S default_AEC_Setting;
	AUDIO_AGC_CONFIG_S default_AGC_Setting;

	default_AGC_Setting.para_agc_max_gain = 0;
	default_AGC_Setting.para_agc_target_high = 2;
	default_AGC_Setting.para_agc_target_low = 72;
	default_AGC_Setting.para_agc_vad_ena = 1;

	AUDIO_ANR_CONFIG_S	default_ANR_Setting;

	default_ANR_Setting.para_nr_snr_coeff = 15;
	default_ANR_Setting.para_nr_init_sile_time = 0;
	/* Default vqe setting ..................end */
	pstAiVqeAttr.s32WorkSampleRate = rate;
#else
	AI_AEC_CONFIG_S default_AEC_Setting;
	AUDIO_AGC_CONFIG_S default_AGC_Setting;

	default_AGC_Setting.para_agc_max_gain = 1;
	default_AGC_Setting.para_agc_target_high = 2;
	default_AGC_Setting.para_agc_target_low = 6;
	default_AGC_Setting.para_agc_vad_enable = CVI_TRUE;
	default_AGC_Setting.para_agc_vad_cnt = 13;
	default_AGC_Setting.para_agc_cut6_enable = CVI_TRUE;

	AUDIO_ANR_CONFIG_S	default_ANR_Setting;

	default_ANR_Setting.para_nr_snr_coeff = 15;
	default_ANR_Setting.para_nr_noise_coeff = 2;
	/* Default vqe setting ..................end */
	pstAiVqeAttr.s32WorkSampleRate = rate;
	pstAiVqeAttr.s32FrameSample = 160;
	pstAiVqeAttr.enWorkstate = VQE_WORKSTATE_COMMON;
	pstAiVqeAttr.s32BytesPerSample = 2;
#endif




	pstAiVqeAttr.stAgcCfg = default_AGC_Setting;
	pstAiVqeAttr.stAnrCfg = default_ANR_Setting;
	pstAiVqeAttr.stAecCfg = default_AEC_Setting;
	pstAiVqeAttr.u32OpenMask = (AI_TALKVQE_MASK_AGC | AI_TALKVQE_MASK_ANR);

	pstAiVqeAttr.para_notch_freq = 0;

	s32Ret = CVI_AI_SetTalkVqeAttr(
			 0,
			 0,
			 0,
			 0,
			 &pstAiVqeAttr);

	//gstAiVQE.s32BytesPerSample  = 2;
	if (s32Ret == CVI_FAILURE) {
		printf("Cannot init audio VQE module\n");
		goto SKIP_ALGO;
	}

	CVI_AI_EnableVqe(0, 0);

	if (s32Ret != CVI_SUCCESS) {
		PRINTF("fail to set talk attr\n");
		PRINTF("skip NR_AGC..\n");
		goto Pattern_EOF_ERR;
	}

	CVI_S32 s32SizeInput = 0;

	CVI_S32 s32SizeGetOut = 0;
	CVI_S32 s32BuffSize = DEFAULT_MAX_FRM_SIZE *
			      MAX_BYTES_PER_SAMPLE *
			      BASIC_BUFFER_DEPTH *
			      DEFAULT_CHANNEL_NUMBER;
	CVI_U8 *GetFrameBuff = (CVI_U8 *) malloc(s32BuffSize);

	for (;;) {	/* Main Frame Loop */
		/* This section of codes have to be replaced by reading system layer bitstream in real platform */
		/* copy unit in memcpy is byte */
		if (fread(pWrite, sizeof(short), hopsize * u32UsrFrmDepth,
			  fp_test_input) != (size_t)(hopsize * u32UsrFrmDepth)) {
			/* get current frame data */
			free(GetFrameBuff);
			goto Pattern_EOF;
		} else {
			s32SizeInput = sizeof(short) * hopsize * u32UsrFrmDepth;

			s32Ret = CVI_AudIn_AlgoProcess_AnrAgc(
					pWrite,
					(CVI_CHAR *)GetFrameBuff,
					s32SizeInput,
					&s32SizeGetOut);

			if (count == 0) { //first single frame from VQE out
				CVI_S32 s32ShiftFirstVQEfrm = SINGLE_VQE_FRAME_SAMPLE * 2;

				fwrite((GetFrameBuff + s32ShiftFirstVQEfrm), 1,
				       (s32SizeGetOut - s32ShiftFirstVQEfrm), fp_test_output);
			} else
				fwrite(GetFrameBuff, 1, s32SizeGetOut, fp_test_output);

			count++;

		}
	}

	SAFE_FREE_BUF(GetFrameBuff);
	SAFE_FREE_BUF(audio_buffer);
Pattern_EOF:
	fclose(fp_test_output);
	fclose(fp_test_input);

	_transform_pcm_to_wave("record.raw", channel, rate, 16, output_filenamewav1);
	_transform_pcm_to_wave("vqeplay.raw", channel, rate, 16, output_filenamewav2);
	/* Start transferring to wav format----------end */

	PRINTF("Pattern_EOF[%d]\n", count);

	if (testCfg->bOptCfg == CVI_TRUE) {
		if (access("vqeplay.raw", 0) >=  0)
			printf("sampe_audio 9 unit TEST-PASS\n");
		else
			printf("sampe_audio 9 unit TEST-NG\n");
	}

	return CVI_SUCCESS;
Pattern_EOF_ERR:
	fclose(fp_test_output);
	fclose(fp_test_input);
	PRINTF("Pattern_EOF_ERR[%d]\n", count);
	return CVI_FAILURE;
SKIP_ALGO:
	fclose(fp_test_output);
	fclose(fp_test_input);
	_transform_pcm_to_wave("record.raw", channel, rate, 16, output_filenamewav1);

	if (testCfg->bOptCfg == CVI_TRUE) {
		if (access("record.raw", 0) >=	0)
			printf("sampe_audio 9 unit TEST-PASS\n");
		else
			printf("sampe_audio 9 unit TEST-NG\n");
	}

	return CVI_SUCCESS;

}


CVI_S32 cvi_audio_set_dbg_play(ST_AudioUnitTestCfg *testCfg)
{
	CVI_S32 idevid = 0;
	CVI_CHAR *localfilename = "record.raw";
	CVI_S32 err;

	PRINTF("Enter [%s] [%d]\n", __func__, __LINE__);

	if (strcmp(testCfg->filename,  "NULL")) {
		//if (testCfg->filename != CVI_NULL) {
		//localfilename = cvifilename;//
		localfilename = testCfg->filename;
		PRINTF("Play user option  filename--->[%s]\n", localfilename);
		/* print out the file name to replay the record.raw */
	} else
		PRINTF("Play default filename[%s]\n", localfilename);

	if (testCfg->bOptCfg == CVI_TRUE) {
		PRINTF("user option mode\n");
		cvi_audio_set_dbg_option2(testCfg);
		idevid = 1;
		_cvi_audio_playfile_sample3(localfilename, 1, testCfg);

	} else {
		/* PRINTF("Enter play out device id:  0 for earphone / 1 for speaker out \t"); */
		PRINTF("Please enter available card id for playback: 0, 1, 2 ??\n");
		PRINTF("Current available card ....\n");

		PRINTF("----------------------cvi check------------------------\n");
		PRINTF("\n enter: \t");
		err = scanf("%d", &idevid);

		if (err == EOF)
			ERR_PRINTF("\n");

		if (idevid == 0) {
			PRINTF("device id 0\n");
			_cvi_audio_playfile_sample3(localfilename, 0, testCfg);
		} else if (idevid == 1) {
			PRINTF("device id 1\n");
			_cvi_audio_playfile_sample3(localfilename, 1, testCfg);
		} else if (idevid == 2) {
			PRINTF("device id 2\n");
			_cvi_audio_playfile_sample3(localfilename, 2, testCfg);
		} else {
			PRINTF("Wrong selection\n");
			//_cvi_audio_playfile_sample(localfilename);
		}
	}

	if (testCfg->bOptCfg == CVI_TRUE) {
		if (access(localfilename, 0) >=  0)
			printf("sampe_audio 10 unit TEST-PASS\n");
		else
			printf("sampe_audio 10 unit TEST-NG\n");
	}

	return CVI_SUCCESS;
}
CVI_S32 cvi_audio_set_dbg_vqe_play(ST_AudioUnitTestCfg *testCfg)
{
	PRINTF("Enter [%s]\n", __func__);
	CVI_S32 idevid = 0;
	CVI_S32 err;

	if (testCfg->bOptCfg == CVI_TRUE) {
		PRINTF("user option mode\n");
		idevid = 1;
	} else {
		PRINTF("Enter available card id for playback: 0, 1, 2 ??\n");
		PRINTF("Current available card ....\n");
		PRINTF("----------------------cvi check------------------------\n");
		PRINTF("\n enter: \t");
		err = scanf("%d", &idevid);

		if (err == EOF)
			ERR_PRINTF("\n");

	}

	if (idevid == 0) {
		PRINTF("device id 0\n");
		_cvi_audio_playfile_sample3("vqeplay.raw", 0, testCfg);
	} else if (idevid == 1) {
		PRINTF("device id 1\n");
		_cvi_audio_playfile_sample3("vqeplay.raw", 1, testCfg);
	} else if (idevid == 2) {
		PRINTF("device id 2\n");
		_cvi_audio_playfile_sample3("vqeplay.raw", 2, testCfg);
	} else {
		//_cvi_audio_playfile_sample("vqeplay.raw");
		PRINTF("Wrong selection\n");
	}

	if (testCfg->bOptCfg == CVI_TRUE) {
		if (access("vqeplay.raw", 0) >=  0)
			printf("sampe_audio 11 unit TEST-PASS\n");
		else
			printf("sampe_audio11 unit TEST-NG\n");
	}

	return CVI_SUCCESS;
}

CVI_S32 cvi_audio_set_dbg_set_volume(ST_AudioUnitTestCfg *testCfg)
{
	PRINTF("Enter [%s]\n", __func__);
	CVI_S32 idevid = 0;
	CVI_S32 volumedb = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 err;
	CVI_S32 s32SetInputVol = 0;

	if (testCfg->unit_test == 1) {
		idevid = 1;
		volumedb = 8;//rang 15 ~0
		s32Ret = CVI_AO_SetVolume(idevid, volumedb);

		if (s32Ret != CVI_SUCCESS) {
			ERR_PRINTF("\n");
			printf("sample_audio 14 set volume unit TEST-NG\n");
		} else {
			//print for auto test / unit test
			printf("sample_audio 14 set volume unit TEST-PASS\n");
		}
	} else {

		PRINTF("----------------------cvi check------------------------\n");
		PRINTF("Current available card ....\n");

		PRINTF("\n Enter output card id: \t");
		err = scanf("%d", &idevid);
		PRINTF("\n Enter volume \t");
		err = scanf("%d", &volumedb);
		PRINTF("\n enter card[%d] vol[%d]\n", idevid, volumedb);

		if (err == EOF)
			ERR_PRINTF("\n");


		PRINTF("\n Set Ain Volume:1  Set Aout Volume:0 ? [0 or 1]\n");

		err = scanf("%d", &s32SetInputVol);
		PRINTF("select [%d]\n", s32SetInputVol);

		if (s32SetInputVol == 0) {
		s32Ret = CVI_AO_SetVolume(idevid, volumedb);
		if (s32Ret != CVI_SUCCESS) {
			ERR_PRINTF("\n");
			return CVI_FAILURE;
		}

		} else {
		s32Ret = CVI_AI_SetVolume(idevid, volumedb);
		if (s32Ret != CVI_SUCCESS) {
			ERR_PRINTF("\n");
			return CVI_FAILURE;
		}
		}

	}

	return CVI_SUCCESS;
}


CVI_S32 cvi_audio_set_dbg_get_volume(ST_AudioUnitTestCfg *testCfg)
{
	PRINTF("Enter [%s]\n", __func__);
	CVI_S32 idevid = 0;
	CVI_S32 volume = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 err;

	if (testCfg->unit_test == 1) {
		int volumedb = 0;

		idevid = 1;
		volumedb = 9;
		s32Ret = CVI_AO_SetVolume(idevid, volumedb);

		if (s32Ret != CVI_SUCCESS) {
			ERR_PRINTF("\n");
			printf("sample_audio 15 get volume fail\n");
		} else {
			//print for auto test / unit test
			s32Ret = CVI_AO_GetVolume(idevid, &volumedb);
			PRINTF("Get Volume [%d]\n", volumedb);

			if (s32Ret != CVI_SUCCESS) {
				ERR_PRINTF("\n");
				printf("sample_audio 15 get volume unit TEST-NG\n");
			} else {
				//print for auto test / unit test
				printf("sample_audio 15 get volume unit TEST-PASS\n");
			}
		}
	} else {
		PRINTF("----------------------cvi check------------------------\n");
		PRINTF("Current available card ....\n");

		PRINTF("Enter output card id: \t");
		err = scanf("%d", &idevid);
		if (err == EOF)
			ERR_PRINTF("\n");

		PRINTF("\n enter card[%d]\n", idevid);
		s32Ret = CVI_AO_GetVolume(idevid, &volume);
		PRINTF("Get Volume Aout[%d]\n", volume);
		if (s32Ret != CVI_SUCCESS) {
			ERR_PRINTF("\n");
			return CVI_FAILURE;
		}

		s32Ret = CVI_AI_GetVolume(idevid, &volume);
		PRINTF("Get Volume Ain[%d]\n", volume);

		if (s32Ret != CVI_SUCCESS) {
			ERR_PRINTF("\n");
			return CVI_FAILURE;
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 _cvi_audio_playfile_sample32(CVI_CHAR *filename, CVI_S32 DevId)
{

	CVI_S32 s32Ret = 0;
	CVI_S32 pcm;
	CVI_S32 seconds = 6;
	struct pcm *pcm_handle;
	struct pcm_config pcm_config;
	//snd_pcm_uframes_t frames = 960;
	CVI_CHAR *buff;
	CVI_S32 buff_size, loops;
	FILE *fp = NULL;

	CVI_S32 device = 0;
	CVI_S32 card = 1;
	CVI_S32  period_size = 320;
	CVI_S32 period_count = 4;
	CVI_S32 channels = 1;
	CVI_S32 rate = 16000;
	CVI_S32 u32BitDepthSelect = 16;

	printf("320---->xxxxx\n");
	PRINTF("\n");
	UNUSED_REF(DevId);
	PRINTF("Enter playback seconds (6):device(%d)card(%d)\n", device, card);
	rate = 16000;
	channels = 2;

	if (!sample_is_playable(card, device, channels, rate, u32BitDepthSelect, period_size, period_count)) {
		printf("device [%d] card [%d] cannot opened success\n", device, card);
		return CVI_FAILURE;
	}

	memset(&pcm_config, 0, sizeof(struct pcm_config));
	pcm_config.channels = channels;
	pcm_config.rate = rate;
	pcm_config.period_size = period_size;
	pcm_config.period_count = period_count;
	pcm_config.format = PCM_FORMAT_S16_LE;
	pcm_config.start_threshold = 0;
	pcm_config.stop_threshold = 0;
	pcm_config.silence_threshold = 0;


	fp = fopen(filename, "rb");

	if (!fp)
		PRINTF("cannot open file [%s] , force return\n", filename);

	pcm_handle = pcm_open(card, device, PCM_OUT, &pcm_config);

	if (!pcm_is_ready(pcm_handle)) {
		ERR_PRINTF("Unable to open PCM device %u (%s)\n",
			   device, pcm_get_error(pcm_handle));
	}

	/* int time; */
	loops = (seconds * rate) / period_size;
	buff_size = period_size * DEFAULT_BYTES_PER_SAMPLE * channels;
	buff =  calloc(1, buff_size);
	while (loops) {
		loops--;

		pcm = fread(buff, 1, buff_size, fp);

		if (pcm <= 0) {
			PRINTF("nothing to read out from file , force exit\n");
			break;
		}

		s32Ret = pcm_write(pcm_handle, buff, buff_size);

		if (s32Ret > 0)
			ERR_PRINTF("\n");

	}

	fclose(fp);
	pcm_close(pcm_handle);
	if (buff)
		free(buff);
	return CVI_SUCCESS;
}


CVI_S32 _cvi_audio_recordfile_sample2(CVI_S32 DevId, CVI_S32 rate, CVI_S32 channel)
{
	CVI_S32 err;
	CVI_CHAR *buffer;
	CVI_S32 period_size = 320;
	int fd_ain = CVI_NULL;
	CVI_S32 s32sample_rate = 0;
	CVI_S32 record_second = 6;
	struct pcm_config capture_config;
	struct pcm *capture_handle;
	enum pcm_format format = PCM_FORMAT_S16_LE;
	CVI_S32  card = 0;
	CVI_S32 device = 0;
	CVI_S32 period_count = 4;

	printf("320 / 4 xxxxxx\n");
#ifdef ARCH_CV183X
	period_size = 960;
#else
	printf("ARCH_CV182X\n");
	period_size = 320;
#endif
	PRINTF("record seconds (6) seconds quick test\n");
	s32sample_rate = rate;

	if (DevId == 0)
		card = 0;
	else if (DevId == 1)
		card = 1;
	else
		card = 0;

	memset(&capture_config, 0, sizeof(capture_config));
	capture_config.channels = channel;
	capture_config.rate = rate;
	capture_config.period_size = period_size;
	capture_config.period_count = period_count;
	capture_config.format = format;
	capture_config.start_threshold = 0;
	capture_config.stop_threshold = 2147483647;
	capture_config.silence_threshold = 0;

	capture_handle = pcm_open(card, device, PCM_IN, &capture_config);

	if (!capture_handle || !pcm_is_ready(capture_handle)) {
		ERR_PRINTF("Unable to open PCM device (%s)\n",
			   pcm_get_error(capture_handle));
		return CVI_FAILURE;
	}


	//CVI_S32 size = (period_size * snd_pcm_format_width(format) / 8) * channel;
	CVI_S32 size = pcm_frames_to_bytes(capture_handle, pcm_get_buffer_size(capture_handle));


	buffer = malloc(size);
	size =	pcm_frames_to_bytes(capture_handle, period_size);

	fprintf(stdout, "[DEBUG]buffer allocated size[%d] period_size[%d] rate[%d]\n",
		size, period_size, rate);

	CVI_S32 loops = record_second * rate / period_size;

	fd_ain	=  0;
	fd_ain = open("record.raw", O_WRONLY + O_CREAT, 0644);

	if (fd_ain != 0)
		PRINTF("[CVIAUDIO]open file success\n");
	else {
		ERR_PRINTF("open file failure!!\n");
		exit(1);
	}

	PRINTF("start to record %d seconds\n", record_second);

	while (loops > 0) {
		loops--;
		err = pcm_read(capture_handle, buffer, size);

		if (err == 0)
			AUD_TRA_PRINTF("\n");
		else
			ERR_PRINTF("[%s]\n", pcm_get_error(capture_handle));

		err = write(fd_ain, buffer, size);

		if (err != size)
			fprintf(stderr, "short write: wrote %d bytes\n", err);

		//PRINTF("loop[%d]\n",loops);
	}

	close(fd_ain);
	fd_ain = -1;
	SAFE_FREE_BUF(buffer);
	fprintf(stdout, "[DEBUG]buffer freed\n");
	pcm_close(capture_handle);
	PRINTF("start to record %d finish\n", record_second);
	return s32sample_rate;
}


CVI_S32 cvi_audio_dbg_quick_test(CVI_VOID)
{
	DBG_PRINTF("\n");
	//unsigned long long checktime = 0;
	//start to record to file record.raw for 8 seconds
	CVI_S32 idselect = 0;
	CVI_S32 rate = 16000;
	CVI_S32 s32Ret;
	CVI_S32 channel = 1;


	/*	PRINTF("Enter record Dev ID: 0 for linein/ 1 for mic in\t"); */
	PRINTF("----------------------cvi check------------------------\n");
	PRINTF("Current available card ....\n");

	idselect = 0;
	rate = 16000;
	channel = 2;


	PRINTF("\n idselect:Card_[%d] chn[%d] rate[%d]\n", channel, rate, idselect);

	if (idselect == 0)
		s32Ret = _cvi_audio_recordfile_sample2(0, rate, channel);
	else if (idselect == 1)
		s32Ret = _cvi_audio_recordfile_sample2(1, rate, channel);
	else
		s32Ret = _cvi_audio_recordfile_sample2(3, rate, channel);


	if (s32Ret != CVI_FAILURE) {
		if (s32Ret == 8000)
			rate = 8000;

		if (s32Ret == 16000)
			rate = 16000;
	} else {
		/* PRINTF("Something wrong while open pcm ...please check\n"); */
		rate = s32Ret;
	}

	PRINTF("[%s] record confirm rate[%d]\n", __func__, rate);
	CVI_S32 count = 0;
	CVI_CHAR *audio_buffer = NULL;
	CVI_S32 s32Size = MAX_BYTES_PER_SAMPLE *
			  DEFAULT_MAX_FRM_SIZE *
			  DEFAULT_CHANNEL_NUMBER *
			  BASIC_BUFFER_DEPTH;
	audio_buffer = (CVI_CHAR *)malloc(s32Size);

	if (audio_buffer == NULL)
		PRINTF("fatal error while allocate buffer[%s][%d]\n", __func__, __LINE__);
	/* step3 init*/
	if ((rate == 8000) || (rate == 16000)) {
		/* record file already success */
		/* Do not need to update sample rate again */
	} else
		ERR_PRINTF("Not Support sample rate aside 16k / 8k[%d]\n", rate);


	DBG_PRINTF("record 6 seconds in record.raw .....Ready to play out sound\n");
	usleep(1500);

	CVI_S32 idevid = 1;

	if (1) {
		PRINTF("Current available card ....\n");
		PRINTF("----------------------cvi play out check------------------------\n");

		PRINTF("\n PLAY OUT idselect:Card_[%d]\n", idevid);

		if (idevid == 0) {

			PRINTF("device id 0\n");
			_cvi_audio_playfile_sample32("record.raw", 0);
		} else if (idevid == 1) {

			PRINTF("device id 1\n");
			_cvi_audio_playfile_sample32("record.raw", 1);
		} else if (idevid == 2) {

			PRINTF("device id 2\n");
			_cvi_audio_playfile_sample32("record.raw", 2);
		} else
			PRINTF("Wrong selection\n");

	}
	SAFE_FREE_BUF(audio_buffer);
	PRINTF("Pattern_EOF[%d]\n", count);
	return CVI_SUCCESS;
}
