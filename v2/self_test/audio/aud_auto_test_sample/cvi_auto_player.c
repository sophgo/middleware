#include "cvi_audio_aac_adp.h"
#include "cvi_auto_test.h"

static AAC_TRANS_TYPE_E gs_enAacTransType = AAC_TRANS_TYPE_ADTS;

typedef enum {
	PCM = 0,
	WAV = 1,
	G726 = 2,
	G711A = 3,
	G711U = 4,
	ADPCM = 5,
	AAC = 7,
	BUTT
} APLAY_TYPE_E;

typedef struct _aplayChnParam {
	char input_patch[MAX_AUD_STRING_LEN];
	int currChn;
	int bindMond;
	bool bStopPlay;
	int chnSampleRate;
	int chnSoundmode;
} ST_APLAY_CHN_PARAM;

typedef struct _aplayParam {
	AIO_ATTR_S AoInstance;
	int sampleRate;
	int	channels;
	int period_size;
	int period_count;
	int all_useChn;
	ST_APLAY_CHN_PARAM stChnParam[AUD_MAX_CHN_NUM];
} ST_APLAY_PATAM;

ST_APLAY_PATAM gstAplayParam;
char ThreadRet[50] = "AudAplay Success";
int AoDev;
int AdDev;


int running = 1;
void sigint_handler_sample(int signal)
{
	printf("signal = %d\n", signal);
	running = 0;
}

void register_inthandler(void)
{
	signal(SIGINT, sigint_handler_sample);
	signal(SIGHUP, sigint_handler_sample);
	signal(SIGTERM, sigint_handler_sample);
}

static int AUTO_CHECK_NULL_PTR(void *ptr)
{
	if ((CVI_U8 *)ptr == NULL) {
		printf("[cvi_auto_error] ptr is NULL,fuc:%s,line:%d\n", __func__, __LINE__);
		return -1;
	}
	return 0;
}

static optionExt long_option_ext[] = {
	{	{"numChn",    optional_argument, NULL, 0},  1,   AUD_MAX_CHN_NUM,
		"number of channels to play"
	},
	{	{"chn",       optional_argument, NULL, 0},  0,   AUD_MAX_CHN_NUM - 1,
		"set channel-id to configure the following parameters"
	},

	{	{"input",		optional_argument, NULL, 'i'},  0,   0,
		"source bitstream"
	},
	{	{"bindmode",	optional_argument, NULL, 'b'},  0,   1,
		"bindmode"
	},
	{	{"outSamRate", optional_argument, NULL, 'R'},  8000,   48000,
		"the sample_rate of playback"
	},
	{	{"ChnSoundmode",       optional_argument, NULL, 'C'},  1,   2,
		"number of channels each chn"
	},
	{	{"help",		no_argument, NULL, 'h'}, 0,   0,
		"help"
	},
	{{NULL, 0, NULL, 0}, 0, 0, ""}
};

void printAudHelp(char **argv)
{
	int idx;

	printf("[------------------------------------------------]\n");
	printf(" %s -h\n", argv[0]);
	printf(" %s -r 48000 -c 2 -p 320 -n 4  -i input.aac -b 0 -R 8000 -C 2\n", argv[0]);
	printf(" %s -r 48000 -c 2 -p 320 -n 4 --numChn=2", argv[0]);
	printf(" --chn=0 -b 0 -C 2 -R 8000 -i input0.wav --chn=1 -b 0 -C 2 -R 8000 -i input1.wav\n");

	printf("-r\n");
	printf("	inputSamRate\n");
	printf("-c\n");
	printf("	mono/stereo\n");
	printf("-p\n");
	printf("	period_size\n");
	printf("-n\n");
	printf("	period_count\n");

	for (idx = 0; idx < (int)(sizeof(long_option_ext) / sizeof(optionExt)); idx++) {
		if (long_option_ext[idx].opt.name == NULL)
			break;
		if (long_option_ext[idx].opt.val != 0) {
			printf("--%s/-%c\n", long_option_ext[idx].opt.name,
			       long_option_ext[idx].opt.val);
			printf("	%s\n", long_option_ext[idx].help);
		} else {
			printf("--%s\n", long_option_ext[idx].opt.name);
			printf("    %s\n", long_option_ext[idx].help);
		}

	}

	printf("[------------------------------------------------]\n");
}

int parsePlayArgv(int argc, char **argv)
{
	int ch, idx;
	struct option long_options[MAX_AUD_OPTIONS + 1];

	memset((void *)long_options, 0, sizeof(long_options));

	for (idx = 0; idx < MAX_AUD_OPTIONS; idx++) {
		if (long_option_ext[idx].opt.name == NULL)
			break;

		if (idx >= MAX_AUD_OPTIONS) {
			CVI_AUTO_ERROR("too many options\n");
			return -1;
		}

		memcpy(&long_options[idx], &long_option_ext[idx].opt, sizeof(struct option));
	}

	for (int i = 0; i < AUD_MAX_CHN_NUM; i++) {
		gstAplayParam.stChnParam[i].currChn = -1;
		gstAplayParam.stChnParam[i].bStopPlay = false;
	}


	ST_APLAY_CHN_PARAM * pstChnParam = &gstAplayParam.stChnParam[0];

	gstAplayParam.all_useChn = 1;
	gstAplayParam.stChnParam[0].currChn = 0;

	optind = 0;
	while ((ch = getopt_long(argc, argv, "r:c:p:n:i:b:R:C:", long_options,
				 &idx)) != -1) {
	//CVI_AUTO_DBG_INFO("ch = %c,optarg = %s, optind = %d, argv[optind]= %s idx = %d\n",
	//	ch, optarg, optind, argv[optind], idx);

		switch (ch) {
		case 'r':
			gstAplayParam.sampleRate = atoi(optarg);
			CVI_AUTO_DBG_INFO("sampleRate = %d\n", gstAplayParam.sampleRate);
			break;
		case 'c':
			gstAplayParam.channels = atoi(optarg);
			CVI_AUTO_DBG_INFO("channels = %d\n", gstAplayParam.channels);
			break;
		case 'p':
			gstAplayParam.period_size = atoi(optarg);
			CVI_AUTO_DBG_INFO("period_size = %d\n", gstAplayParam.period_size);
			break;
		case 'n':
			gstAplayParam.period_count = atoi(optarg);
			CVI_AUTO_DBG_INFO("period_count = %d\n", gstAplayParam.period_count);
			break;

		case 'i':
			strcpy(pstChnParam->input_patch, optarg);
			CVI_AUTO_DBG_INFO("chnParam[%d].input_patch = %s\n", pstChnParam->currChn,
					  pstChnParam->input_patch);
			break;
		case 'b':
			pstChnParam->bindMond = atoi(optarg);
			CVI_AUTO_DBG_INFO("chnParam[%d].bindmode = %d\n", pstChnParam->currChn,
					  pstChnParam->bindMond);
			break;
		case 'R':
			pstChnParam->chnSampleRate = atoi(optarg);
			CVI_AUTO_DBG_INFO("chnParam[%d].chnSampleRate = %d\n", pstChnParam->currChn,
					  pstChnParam->chnSampleRate);
			break;
		case 'C':
			pstChnParam->chnSoundmode = atoi(optarg);
			CVI_AUTO_DBG_INFO("chnParam[%d].chnSoundmode = %d\n", pstChnParam->currChn,
					  pstChnParam->chnSoundmode);
			break;
		case 'h':
			printAudHelp(argv);
			return STATUS_HELP;

		case 0:
			if (!strcmp(long_options[idx].name, "numChn")) {

				gstAplayParam.all_useChn = atoi(optarg);
				CVI_AUTO_DBG_INFO("numChn == %d\n", gstAplayParam.all_useChn);
			} else if (!strcmp(long_options[idx].name, "chn")) {

				pstChnParam = &gstAplayParam.stChnParam[atoi(optarg)];
				pstChnParam->currChn = atoi(optarg);
				CVI_AUTO_DBG_INFO("chn == %d\n", pstChnParam->currChn);
			} else {
				CVI_AUTO_ERROR("not exist name = %s\n", long_options[idx].name);
				printAudHelp(argv);
				return -1;
			}
			break;
		default:
			CVI_AUTO_DBG_INFO("ch = %c\n", ch);
			printAudHelp(argv);
			break;
		}

	}
	if (optind < argc)
		printAudHelp(argv);
	return 0;

}



static FILE *audio_open_wavfile(const char *filename, int *channels,
				int *sample_rate)
{
	FILE *file;
	struct riff_wave_header riff_wave_header;
	struct chunk_header chunk_header;
	struct chunk_fmt chunk_fmt;
	int more_chunks = 1;

	file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "Unable to open file '%s'\n", filename);
		return NULL;
	}

	fread(&riff_wave_header, sizeof(riff_wave_header), 1, file);
	if ((riff_wave_header.riff_id != ID_RIFF) ||
	    (riff_wave_header.wave_id != ID_WAVE)) {
		fprintf(stderr, "Error: '%s' is not a riff/wave file\n", filename);
		fclose(file);
		return NULL;
	}

	do {
		fread(&chunk_header, sizeof(chunk_header), 1, file);

		switch (chunk_header.id) {
		case ID_FMT:
			fread(&chunk_fmt, sizeof(chunk_fmt), 1, file);
			*sample_rate = chunk_fmt.sample_rate;
			*channels = chunk_fmt.num_channels;
			/* If the format header is larger, skip the rest */
			if (chunk_header.sz > sizeof(chunk_fmt))
				fseek(file, chunk_header.sz - sizeof(chunk_fmt), SEEK_CUR);
			break;
		case ID_DATA:
			/* Stop looking for chunks */
			more_chunks = 0;
			chunk_header.sz = le32toh(chunk_header.sz);
			break;
		default:
			/* Unknown chunk, skip bytes */
			fseek(file, chunk_header.sz, SEEK_CUR);
		}
	} while (more_chunks);
	return file;
}



APLAY_TYPE_E check_file_formate(char *infilename)
{

	int s32InputFileLen = 0;

	s32InputFileLen = strlen(infilename);
	if (s32InputFileLen == 0) {
		CVI_AUTO_INFO("No Input File Name..force return\n");
		return -1;
	}

	if (infilename[s32InputFileLen - 4] == '.' &&
	    (infilename[s32InputFileLen - 3] == 'p' ||
	     infilename[s32InputFileLen - 3] == 'r') &&
	    (infilename[s32InputFileLen - 2] == 'c' ||
	     infilename[s32InputFileLen - 2] == 'a') &&
	    (infilename[s32InputFileLen - 1] == 'm' ||
	     infilename[s32InputFileLen - 1] == 'w')) {

		CVI_AUTO_INFO("file formate [pcm/raw]\n");
		return PCM;

	} else if (infilename[s32InputFileLen - 4] == '.' &&
		   infilename[s32InputFileLen - 3] == 'w' &&
		   infilename[s32InputFileLen - 2] == 'a' &&
		   infilename[s32InputFileLen - 1] == 'v') {

		CVI_AUTO_INFO("file formate [wav]\n");
		return WAV;
	} else if (infilename[s32InputFileLen - 5] == '.' &&
		   infilename[s32InputFileLen - 4] == 'g' &&
		   infilename[s32InputFileLen - 3] == '7' &&
		   infilename[s32InputFileLen - 2] == '2' &&
		   infilename[s32InputFileLen - 1] == '6') {

		CVI_AUTO_INFO("file formate [g726]\n");
		return G726;
	} else if (infilename[s32InputFileLen - 6] == '.' &&
		   infilename[s32InputFileLen - 5] == 'g' &&
		   infilename[s32InputFileLen - 4] == '7' &&
		   infilename[s32InputFileLen - 3] == '1' &&
		   infilename[s32InputFileLen - 2] == '1' &&
		   (infilename[s32InputFileLen - 1] == 'a' ||
		    infilename[s32InputFileLen - 1] == 'u')) {

		CVI_AUTO_INFO("file formate [g711%c]\n", infilename[s32InputFileLen - 1]);
		if (infilename[s32InputFileLen - 1] == 'a')
			return G711A;
		else
			return G711U;

	} else if (infilename[s32InputFileLen - 6] == '.' &&
		   infilename[s32InputFileLen - 5] == 'a' &&
		   infilename[s32InputFileLen - 4] == 'd' &&
		   infilename[s32InputFileLen - 3] == 'p' &&
		   infilename[s32InputFileLen - 2] == 'c' &&
		   infilename[s32InputFileLen - 1] == 'm') {

		CVI_AUTO_INFO("file formate [adpcm]\n");
		return ADPCM;
	} else if (infilename[s32InputFileLen - 4] == '.' &&
		   infilename[s32InputFileLen - 3] == 'a' &&
		   infilename[s32InputFileLen - 2] == 'a' &&
		   infilename[s32InputFileLen - 1] == 'c') {

		CVI_AUTO_INFO("file formate [aac]\n");
		return AAC;
	}

	CVI_AUTO_ERROR("file[%s] is error\n", infilename);
	return -1;
}


PAYLOAD_TYPE_E check_adec_formate(APLAY_TYPE_E etype)
{
	if (etype == G726)
		return PT_G726;
	else if (etype == G711A)
		return PT_G711A;
	else if (etype == G711U)
		return PT_G711U;
	else if (etype == ADPCM)
		return PT_ADPCMA;
	else if (etype == AAC)
		return PT_AAC;
	return PT_BUTT;
}


int _updata_Adec_setting(ADEC_CHN_ATTR_S *pAdecAttr)
{
	if (AUTO_CHECK_NULL_PTR((void *)pAdecAttr))
		return -1;
	pAdecAttr->u32BufSize = 20;
	pAdecAttr->enMode = ADEC_MODE_STREAM;/* propose use pack mode in your app */
	pAdecAttr->bFileDbgMode = CVI_FALSE;

	if (pAdecAttr->enType == PT_ADPCMA) {
		ADEC_ATTR_ADPCM_S *pstAdpcm = malloc(sizeof(ADEC_ATTR_ADPCM_S));

		pAdecAttr->pValue = pstAdpcm;
		pstAdpcm->enADPCMType = AUDIO_ADPCM_TYPE;
	} else if (pAdecAttr->enType == PT_G711A || pAdecAttr->enType == PT_G711U) {
		ADEC_ATTR_G711_S *pstAdecG711 = malloc(sizeof(ADEC_ATTR_G711_S));

		pAdecAttr->pValue = pstAdecG711;
	} else if (pAdecAttr->enType == PT_G726) {
		ADEC_ATTR_G726_S *pstAdecG726 = malloc(sizeof(ADEC_ATTR_G726_S));

		pAdecAttr->pValue = pstAdecG726;
		pstAdecG726->enG726bps = G726_BPS;
	} else if (pAdecAttr->enType == PT_LPCM) {
		ADEC_ATTR_LPCM_S *pstAdecLpcm = malloc(sizeof(ADEC_ATTR_LPCM_S));

		pAdecAttr->pValue = pstAdecLpcm;
		pAdecAttr->enMode = ADEC_MODE_PACK;/* lpcm must use pack mode */
	}

	AUTO_AUD_UNUSED_REF(gs_enAacTransType);
	if (pAdecAttr->enType == PT_AAC) {
		CVI_MPI_ADEC_AacInit();
		ADEC_ATTR_AAC_S *pstAdecAac = malloc(sizeof(ADEC_ATTR_AAC_S));

		pstAdecAac->enTransType = gs_enAacTransType;
		pstAdecAac->enSoundMode = (pAdecAttr->s32ChannelNums == 2 ?
					   AUDIO_SOUND_MODE_STEREO : AUDIO_SOUND_MODE_MONO);
		pstAdecAac->enSmpRate = pAdecAttr->s32Sample_rate;
		pAdecAttr->pValue = pstAdecAac;
		pAdecAttr->enMode = ADEC_MODE_STREAM;   /* aac should be stream mode */
		pAdecAttr->s32frame_size = 1024;
	}

	return 0;
}

int _destroy_Adec_setting(ADEC_CHN_ATTR_S *pAdecAttr)
{
	if (AUTO_CHECK_NULL_PTR((void *)pAdecAttr))
		return -1;
	if (AUTO_CHECK_NULL_PTR((void *)pAdecAttr->pValue))
		return -1;

	if (pAdecAttr->enType == PT_AAC)
		CVI_MPI_ADEC_AacDeInit();

	AUTO_FREE_BUF(pAdecAttr->pValue);
	memset(pAdecAttr, 0, sizeof(ADEC_CHN_ATTR_S));

	return 0;
}

int play_audio_adec_unbind(ADEC_CHN_ATTR_S *pstAdecAttr, int AdChn, int AoChn)
{
	if (AUTO_CHECK_NULL_PTR((void *)pstAdecAttr))
		return -1;
	int s32Ret = CVI_SUCCESS;
	int num_readbytes = 0;
	AUDIO_STREAM_S stAudioStream;
	int length = 640;
	AUDIO_FRAME_S stFrame;
	AUDIO_FRAME_S *pstFrame = &stFrame;
	AUDIO_FRAME_INFO_S sDecOutFrm;
	int chnframe_size = (pstAdecAttr->s32Sample_rate * MS_DATA)/1000;//20ms

	length = chnframe_size;
	char *pBuffer = malloc(length);

	memset(pBuffer, 0, length);
	FILE *fpAdec = fopen(gstAplayParam.stChnParam[AoChn].input_patch, "rb+");

	if (AUTO_CHECK_NULL_PTR((void *)fpAdec))
		return -1;
	sDecOutFrm.pstFrame = (AUDIO_FRAME_S *)&stFrame;

	CVI_AUTO_INFO("chn_framesize:%d, dev_framesize:%d\n", chnframe_size, pstAdecAttr->s32frame_size);
	while (running) {
		stAudioStream.pStream = (CVI_U8 *)pBuffer;
		num_readbytes = fread(stAudioStream.pStream, 1, length, fpAdec);
		if (num_readbytes <= 0) {
			s32Ret = CVI_ADEC_SendEndOfStream(AdChn, CVI_FALSE);
			if (s32Ret != CVI_SUCCESS) {
				CVI_AUTO_ERROR("AdChn[%d], s32Ret[%#x], num_readbytes[%d]\n",
					       AdChn, s32Ret, num_readbytes);
				goto ERROR;
			}
			break;
		}
		stAudioStream.u32Len = num_readbytes; //640 bytes
		s32Ret = CVI_ADEC_SendStream(AdChn, &stAudioStream, CVI_TRUE);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("AdChn[%d]AoChn[%d] unbind send stream failed Len[%d]ret[%#x]!\n",
				       AdChn, AoChn, stAudioStream.u32Len, s32Ret);
				goto ERROR;
		}

		s32Ret = CVI_ADEC_GetFrame(AdChn, &sDecOutFrm, 0);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("AdChn[%d]AoChn[%d] get stream failed Len[%d]ret[%#x]!\n",
				       AdChn, AoChn, stAudioStream.u32Len, s32Ret);
				goto ERROR;
		}

		if (pstFrame->u32Len != 0) {
			s32Ret = CVI_AO_SendFrame(AoDev, AoChn, pstFrame, 1000);
			if (s32Ret != CVI_SUCCESS) {
				CVI_AUTO_ERROR("AoDev(%d), failed with %#x!\n", AoDev, s32Ret);
				goto ERROR;
			}
		} else
			CVI_AUTO_WARN("dec out frame size 0\n");


	}

	AUTO_FREE_BUF(pBuffer);
	fclose(fpAdec);
	return 0;
ERROR:
	AUTO_FREE_BUF(pBuffer);
	fclose(fpAdec);
	return -1;
}

int play_audio_adec_bind(ADEC_CHN_ATTR_S *pstAdecAttr, int AdChn, int AoChn)
{

	if (AUTO_CHECK_NULL_PTR((void *)pstAdecAttr))
		return -1;
	int s32Ret = CVI_SUCCESS;
	int num_readbytes = 0;
	AUDIO_STREAM_S stAudioStream;
	int length = 640;
	MMF_CHN_S stSrcChn, stDestChn;
	int chnframe_size = (pstAdecAttr->s32Sample_rate * MS_DATA)/1000;//20ms

	stSrcChn.enModId = CVI_ID_ADEC;
	stSrcChn.s32DevId = AdDev;
	stSrcChn.s32ChnId = AdChn;
	stDestChn.enModId = CVI_ID_AO;
	stDestChn.s32DevId = AoDev;
	stDestChn.s32ChnId = AoChn;
	CVI_AUD_SYS_Bind(&stSrcChn, &stDestChn);


	length = chnframe_size;
	char *pBuffer = malloc(length);

	memset(pBuffer, 0, length);
	FILE *fpAdec = fopen(gstAplayParam.stChnParam[AoChn].input_patch, "rb+");

	if (AUTO_CHECK_NULL_PTR((void *)fpAdec))
		return -1;

	CVI_AUTO_INFO("chn_framesize:%d, dev_framesize:%d\n", chnframe_size, pstAdecAttr->s32frame_size);
	while (running) {
		stAudioStream.pStream = (CVI_U8 *)pBuffer;
		num_readbytes = fread(stAudioStream.pStream, 1, length, fpAdec);
		if (num_readbytes <= 0) {
			s32Ret = CVI_ADEC_SendEndOfStream(AdChn, CVI_FALSE);
			if (s32Ret != CVI_SUCCESS) {
				CVI_AUTO_ERROR("AdChn[%d], s32Ret[%#x], num_readbytes[%d]\n",
					       AdChn, s32Ret, num_readbytes);
				goto ERROR;
			}
			break;
		}
		stAudioStream.u32Len = num_readbytes;
		s32Ret = CVI_ADEC_SendStream(AdChn, &stAudioStream, CVI_TRUE);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("AdChn[%d]AoChn[%d] bind mode send stream failed Len[%d]ret[%#x]!\n",
				       AdChn, AoChn, stAudioStream.u32Len, s32Ret);

			goto ERROR;
		}

	}


	CVI_AUD_SYS_UnBind(&stSrcChn, &stDestChn);
	AUTO_FREE_BUF(pBuffer);
	fclose(fpAdec);
	return 0;

ERROR:
	CVI_AUD_SYS_UnBind(&stSrcChn, &stDestChn);
	AUTO_FREE_BUF(pBuffer);
	fclose(fpAdec);
	return -1;
}


int play_audio_file(AIO_ATTR_S *pstAioAttr, FILE *fp, int AoChn)
{
	int s32Ret = CVI_SUCCESS;
	AUDIO_FRAME_S stFrame;
	char szThreadName[20] = "downlink";
	int num_readbytes;

	if (AUTO_CHECK_NULL_PTR((void *)pstAioAttr))
		return -1;
	int channel_count = gstAplayParam.stChnParam[AoChn].chnSoundmode;
	int chn_sr = gstAplayParam.stChnParam[AoChn].chnSampleRate;
	int s32FrameBytes = channel_count * ((chn_sr * MS_DATA)/1000) * BYTES_PER_SAMPLE;

	char *pBuffer = malloc(s32FrameBytes);


	prctl(PR_SET_NAME, szThreadName, 0, 0, 0);
	memset(pBuffer, 0, s32FrameBytes);


	CVI_AUTO_INFO("devsr:%d, chn_sr:%d, devchn:%d, chnsm:%d, period:%d, s32FrameBytes:%d.\n",
		      pstAioAttr->enSamplerate, chn_sr,
		      pstAioAttr->enSoundmode + 1, channel_count, pstAioAttr->u32PtNumPerFrm, s32FrameBytes);
	while (running) {
		memset(pBuffer, 0, s32FrameBytes);
		num_readbytes = fread(pBuffer, 1, s32FrameBytes, fp);
		if (num_readbytes > 0) {
			stFrame.u64VirAddr[0] = (CVI_U8 *)pBuffer;
			stFrame.u32Len = ((chn_sr * MS_DATA)/1000);//20ms according to chn_sr
			stFrame.u64TimeStamp = 0;
			stFrame.enSoundmode = channel_count - 1;
			stFrame.enBitwidth = AUDIO_BIT_WIDTH_16;

			s32Ret = CVI_AO_SendFrame(AoDev, AoChn, (const AUDIO_FRAME_S *)&stFrame, 1000);
			if (s32Ret != CVI_SUCCESS) {
				CVI_AUTO_ERROR("AoChn[%d] ret[%#x]!\n", AoChn, s32Ret);
				AUTO_FREE_BUF(pBuffer);
				fclose(fp);
				return -1;
			}
		} else {
			CVI_AUTO_INFO("num_framebytes %d.\n", num_readbytes);
			break;
		}
	}

	AUTO_FREE_BUF(pBuffer);

	if (!fp)
		fclose(fp);

	return 0;
}

void *AudPlayThread(void *pargv)
{

	int s32Ret = CVI_SUCCESS;
	int AoChn = *(int *)pargv;
	FILE *Fp = NULL;

	snprintf(ThreadRet, 50, "AplayThread[%d] Success", AoChn);

	CVI_AUTO_INFO("open AoChn [%d], input_file = %s, bindmode = %d, chnsr = %d, chns = %d\n",
		      AoChn, gstAplayParam.stChnParam[AoChn].input_patch,
		      gstAplayParam.stChnParam[AoChn].bindMond,
			  gstAplayParam.stChnParam[AoChn].chnSampleRate,
			  gstAplayParam.stChnParam[AoChn].chnSoundmode);

	s32Ret = CVI_AO_EnableChn(AoDev, AoChn);
	if (s32Ret != CVI_SUCCESS) {
		CVI_AUTO_ERROR("s32Ret = %d, AoChn = %d\n", s32Ret, AoChn);
		return NULL;
	}

	APLAY_TYPE_E etype = check_file_formate(
				     gstAplayParam.stChnParam[AoChn].input_patch);

	if (etype == PCM) {
		Fp = fopen(gstAplayParam.stChnParam[AoChn].input_patch, "rb");
		if (Fp == NULL) {
			CVI_AUTO_ERROR("%s open fail\n", gstAplayParam.stChnParam[AoChn].input_patch);
			goto ERROR;
		}
	} else if (etype == WAV) {
		int wav_chn = 0;
		int wav_sample_rate = 0;

		Fp = audio_open_wavfile(gstAplayParam.stChnParam[AoChn].input_patch,
					(int *)&wav_chn,
					(int *)&wav_sample_rate);
		if (Fp == NULL) {
			CVI_AUTO_ERROR("%s open fail\n", gstAplayParam.stChnParam[AoChn].input_patch);
			goto ERROR;
		}

		if ((wav_chn != gstAplayParam.stChnParam[AoChn].chnSoundmode) ||
		    (wav_sample_rate != gstAplayParam.stChnParam[AoChn].chnSampleRate)) {
			CVI_AUTO_WARN("wav_chn[%d] != input_chn [%d] or wav_sr[%d] != input_sr[%d]\n",
				       wav_chn, gstAplayParam.stChnParam[AoChn].chnSoundmode, wav_sample_rate,
				       gstAplayParam.stChnParam[AoChn].chnSampleRate);

			gstAplayParam.stChnParam[AoChn].chnSoundmode = wav_chn;
			gstAplayParam.stChnParam[AoChn].chnSampleRate = wav_sample_rate;

		}

	}


	if (gstAplayParam.sampleRate != gstAplayParam.stChnParam[AoChn].chnSampleRate) {
		s32Ret = CVI_AO_EnableReSmp(AoDev, AoChn,
					    gstAplayParam.stChnParam[AoChn].chnSampleRate);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("decSr[%d], chn[%d].chnSr[%d], s32ret[%#x]\n",
				       gstAplayParam.sampleRate, AoChn, gstAplayParam.stChnParam[AoChn].chnSampleRate,
				       s32Ret);
			goto ERROR;
		}
	}


	if (etype == PCM || etype == WAV) {

		s32Ret = play_audio_file(&gstAplayParam.AoInstance, Fp, AoChn);
		if (s32Ret)
			goto ERROR;
	} else {

		ADEC_CHN_ATTR_S stAdecAttr;
		int AdChn = AoChn;

		CVI_AUTO_INFO("AdChn:%d\n", AdChn);

		memset(&stAdecAttr, 0, sizeof(ADEC_CHN_ATTR_S));
		stAdecAttr.s32Sample_rate = gstAplayParam.stChnParam[AoChn].chnSampleRate;
		stAdecAttr.s32ChannelNums = gstAplayParam.stChnParam[AoChn].chnSoundmode;
		stAdecAttr.s32BytesPerSample = BYTES_PER_SAMPLE;
		stAdecAttr.s32frame_size = gstAplayParam.period_size;
		stAdecAttr.enType = check_adec_formate(etype);
		if (stAdecAttr.enType == PT_AAC)
			stAdecAttr.s32frame_size = 1024;

		_updata_Adec_setting(&stAdecAttr);

		s32Ret = CVI_ADEC_CreateChn(AdChn, &stAdecAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("s32Ret = %d, AoChn = %d, AdChn = %d\n", s32Ret, AoChn, AdChn);
			_destroy_Adec_setting(&stAdecAttr);
			goto ERROR;
		}

		if (gstAplayParam.stChnParam[AoChn].bindMond) {//bind mode
			CVI_AUTO_INFO("bind mode\n");
			s32Ret = play_audio_adec_bind(&stAdecAttr, AdChn, AoChn);
			if (s32Ret)
				goto ERROR;

		} else {//no bind mode
			CVI_AUTO_INFO("unbind mode\n");
			s32Ret = play_audio_adec_unbind(&stAdecAttr, AdChn, AoChn);
			if (s32Ret)
				goto ERROR;
		}

		CVI_ADEC_DestroyChn(AdChn);
		_destroy_Adec_setting(&stAdecAttr);
	}

	CVI_AO_DisableChn(AoDev, AoChn);
	gstAplayParam.stChnParam[AoChn].bStopPlay = true;
	return (void *)ThreadRet;
ERROR:
	CVI_AO_DisableChn(AoDev, AoChn);
	gstAplayParam.stChnParam[AoChn].bStopPlay = true;

	return NULL;
}


int main(int argc, char *argv[])
{
	int s32Ret = CVI_SUCCESS;
	int Chncount = 0;
	pthread_t AoutDevThread[AUD_MAX_CHN_NUM];
	AIO_ATTR_S AudoutAttr;
	int MaxAudioChn = 3;
	void *ThreadRet[AUD_MAX_CHN_NUM];
	bool Thread_error = false;

	register_inthandler();
	s32Ret = parsePlayArgv(argc, argv);
	if (s32Ret < 0) {
		if (s32Ret == STATUS_HELP)
			return CVI_SUCCESS;
		return CVI_FAILURE;
	}

	printf("sample_rate = %d, channels = %d, period_size = %d, period_count = %d, all_useChn = %d\n",
	       gstAplayParam.sampleRate, gstAplayParam.channels, gstAplayParam.period_size,
	       gstAplayParam.period_count, gstAplayParam.all_useChn);


	for (int i = 0; i < AUD_MAX_CHN_NUM; i++) {
		if (gstAplayParam.stChnParam[i].currChn != -1) {
			printf("chn[%d]:input_patch = %s, bindMond = %d, ChnSr = %d, chnsm = %d\n",
			       gstAplayParam.stChnParam[i].currChn,
			       gstAplayParam.stChnParam[i].input_patch, gstAplayParam.stChnParam[i].bindMond,
			       gstAplayParam.stChnParam[i].chnSampleRate, gstAplayParam.stChnParam[i].chnSoundmode);
		}

	}

//STEP 1: set ao and enable ao
	AudoutAttr.u32ChnCnt = MaxAudioChn;
	AudoutAttr.enSamplerate   = gstAplayParam.sampleRate;
	AudoutAttr.enSoundmode	  = (gstAplayParam.channels == 2 ?
				     AUDIO_SOUND_MODE_STEREO :
				     AUDIO_SOUND_MODE_MONO);
	AudoutAttr.enWorkmode	  = AIO_MODE_I2S_MASTER;
	AudoutAttr.u32EXFlag	  = 0;
	AudoutAttr.u32FrmNum	  = gstAplayParam.period_count; /* only use in bind mode */
	AudoutAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	AudoutAttr.u32PtNumPerFrm =
		gstAplayParam.period_size;/* 20*targetsamplerate/1000 */
	AudoutAttr.u32ClkSel	  = 0;
	AudoutAttr.enI2sType = AIO_I2STYPE_INNERCODEC;
	memcpy(&gstAplayParam.AoInstance, &AudoutAttr, sizeof(AIO_ATTR_S));

	s32Ret = CVI_AUDIO_INIT();
	if (s32Ret != CVI_SUCCESS) {
		CVI_AUTO_ERROR("s32Ret = %d\n", s32Ret);
		return -1;
	}

	s32Ret = CVI_AO_SetPubAttr(AoDev, &AudoutAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_AUTO_ERROR("AoDev = %d, s32Ret = %d\n", AoDev, s32Ret);
		return -1;
	}

	s32Ret = CVI_AO_Enable(AoDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_AUTO_ERROR("AoDev = %d, s32Ret = %d\n", AoDev, s32Ret);
		return -1;
	}

//STEP 2: Create threads and send data to devices
	for (int i = 0; i < AUD_MAX_CHN_NUM; i++) {

		if (gstAplayParam.stChnParam[i].currChn != -1) {
			CVI_AUTO_INFO("creat Thread chn[%d][%d]\n", i,
				      gstAplayParam.stChnParam[i].currChn);
			pthread_create(&AoutDevThread[i], NULL,
				       (CVI_VOID * (*)(CVI_VOID *))AudPlayThread,
				       &gstAplayParam.stChnParam[i].currChn);
			Chncount++;
			CVI_AUTO_INFO("chn_count = %d\n", Chncount);
		}
	}

	if (Chncount != gstAplayParam.all_useChn)
		CVI_AUTO_ERROR("Thread_num[%d] != all_useChn[%d]\n", Chncount,
			       gstAplayParam.all_useChn);

	for (int i = 0; i < AUD_MAX_CHN_NUM; i++) {
		if (gstAplayParam.stChnParam[i].currChn != -1) {
			while (!gstAplayParam.stChnParam[i].bStopPlay) {
				usleep(1000 * 20);
			}
			pthread_join(AoutDevThread[i], &ThreadRet[i]);
			CVI_AUTO_INFO("chn[%d][%d][%s] the end of playing\n", i,
				      gstAplayParam.stChnParam[i].currChn, (char *)ThreadRet[i]);
			if (ThreadRet[i] == NULL)
				Thread_error = true;
		}
	}

	CVI_AO_Disable(AoDev);
	CVI_AUDIO_DEINIT();
	if (Thread_error == false)
		printf("TEST-PASS\n");

	return 0;
}


