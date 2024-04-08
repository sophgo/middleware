#include "cvi_auto_test.h"
#include "sample_comm.h"
#include "cvi_audio_aac_adp.h"


static AAC_TYPE_E     gs_enAacType = AAC_TYPE_AACLC;
static AAC_BPS_E     gs_enAacBps  = AAC_BPS_32K;
static AAC_TRANS_TYPE_E gs_enAacTransType = AAC_TRANS_TYPE_ADTS;

typedef struct _aplayChnParam {
	char output_patch[MAX_AUD_STRING_LEN];
	int currChn;
	int bindMond;
	bool bStopAcap;
	bool bVqe;
	int chnSampleRate;
	char Aencformate[MAX_AUD_STRING_LEN];
} ST_ACAP_CHN_PARAM;

typedef struct _aplayParam {
	AIO_ATTR_S AiInstance;
	int sampleRate;
	int	channels;
	int period_size;
	int period_count;
	int all_useChn;
	int record_time;
	ST_ACAP_CHN_PARAM stChnParam[AUD_AI_MAX_CHN_NUM];
} ST_ACAP_PATAM;

ST_ACAP_PATAM gstAcapParam;
char ThreadRet[50] = "AudAcap Success";
AI_TALKVQE_CONFIG_S stAiVqeTalkAttr;
int AiDev;
int AeDev;



static optionExt long_option_ext[] = {
	{	{"numChn",    optional_argument, NULL, 0},  1,   AUD_AI_MAX_CHN_NUM,
		"number of channels to play"
	},
	{	{"chn",       optional_argument, NULL, 0},  0,   AUD_AI_MAX_CHN_NUM - 1,
		"set channel-id to configure the following parameters"
	},

	{	{"output",		optional_argument, NULL, 'o'},  0,   0,
		"source bitstream"
	},
	{	{"bindmode",		optional_argument, NULL, 'b'},  0,   1,
		"bind mode"
	},
	{	{"outSamRate", optional_argument, NULL, 'R'},  8000,   48000,
		"the sample_rate of playback"
	},

	{	{"formate", optional_argument, NULL, 'f'}, 0, 0,
		"g726,g711a,g711u,adpcm,aac"
	},
	{	{"bVqe", optional_argument, NULL, 'v'}, 0, 1,
		"open agc anr aec"
	},

	{	{"help",		no_argument, NULL, 'h'}, 0,   0,
		"help"
	},
	{{NULL, 0, NULL, 0}, 0, 0, ""}
};


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

static PAYLOAD_TYPE_E get_codec_type(char *formate)
{
	AUTO_CHECK_NULL_PTR((void *)formate);
	PAYLOAD_TYPE_E eType = PT_G726;


	if (!strcmp(formate, "g726")) {
		//CVI_AUTO_INFO("Codec G726\n");
		eType = PT_G726;
	} else if (!strcmp(formate, "g711a")) {
		//CVI_AUTO_INFO("Codec G711A\n");
		eType = PT_G711A;
	} else if (!strcmp(formate, "g711u")) {
		//CVI_AUTO_INFO("Codec G711Mu\n");
		eType = PT_G711U;
	} else if (!strcmp(formate, "adpcm")) {
		//CVI_AUTO_INFO("Codec PT_ADPCMA\n");
		eType = PT_ADPCMA;
	} else if (!strcmp(formate, "aac")) {
		//CVI_AUTO_INFO("Codec AAC_LC\n");
		eType = PT_AAC;
	} else {
		//CVI_AUTO_INFO("Enter invalid num ....select g726\n");
		eType = PT_G726;
	}

	return eType;
}

void printAudHelp(char **argv)
{
	int idx;

	printf("[------------------------------------------------]\n");
	printf(" %s -h\n", argv[0]);
	printf(" %s -r 8000 -c 2 -p 320 -n 4 -T 10 -R 8000 -f g711a -b 0 -o out.g711a -v 0\n",
	       argv[0]);
	printf(" %s -r 8000 -c 2 -p 320 -n 4 -T 10 --numChn=2", argv[0]);
	printf(" --chn=0 -f g711a -b 0 -R 8000 -o out.g711a -v 0 --chn=1 -f g711u -b 1 -R 8000 -o out.g711u\n");
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

static int parseAcapArgv(int argc, char **argv)
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

	for (int i = 0; i < AUD_AI_MAX_CHN_NUM; i++) {
		gstAcapParam.stChnParam[i].currChn = -1;
		gstAcapParam.stChnParam[i].bStopAcap = false;
		gstAcapParam.stChnParam[i].bVqe = false;
	}


	ST_ACAP_CHN_PARAM * pstChnParam = &gstAcapParam.stChnParam[0];

	gstAcapParam.all_useChn = 1;
	gstAcapParam.stChnParam[0].currChn = 0;

	optind = 0;
	while ((ch = getopt_long(argc, argv, "r:c:p:n:v:T:o:b:R:f:", long_options,
				 &idx)) != -1) {
		CVI_AUTO_DBG_INFO("ch = %c,optarg = %s, optind = %d, argv[optind]= %s idx = %d\n",
				  ch, optarg, optind, argv[optind], idx);

		switch (ch) {
		case 'r':
			gstAcapParam.sampleRate = atoi(optarg);
			CVI_AUTO_DBG_INFO("sampleRate = %d\n", gstAcapParam.sampleRate);
			break;
		case 'c':
			gstAcapParam.channels = atoi(optarg);
			CVI_AUTO_DBG_INFO("channels = %d\n", gstAcapParam.channels);
			break;
		case 'p':
			gstAcapParam.period_size = atoi(optarg);
			CVI_AUTO_DBG_INFO("period_size = %d\n", gstAcapParam.period_size);
			break;
		case 'n':
			gstAcapParam.period_count = atoi(optarg);
			CVI_AUTO_DBG_INFO("period_count = %d\n", gstAcapParam.period_count);
			break;
		case 'T':
			gstAcapParam.record_time = atoi(optarg);
			CVI_AUTO_DBG_INFO("recordtime = %d\n", gstAcapParam.record_time);
			break;
		case 'o':
			strcpy(pstChnParam->output_patch, optarg);
			CVI_AUTO_DBG_INFO("chnParam[%d].output_patch = %s\n", pstChnParam->currChn,
					  pstChnParam->output_patch);
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
		case 'f':
			strcpy(pstChnParam->Aencformate, optarg);
			CVI_AUTO_DBG_INFO("chnParam[%d].Aencformate = %s\n", pstChnParam->currChn,
					  pstChnParam->Aencformate);
			break;
		case 'v':
			pstChnParam->bVqe = atoi(optarg);
			CVI_AUTO_DBG_INFO("bVqe = %d\n", pstChnParam->bVqe);
			break;
		case 'h':
			printAudHelp(argv);
			return STATUS_HELP;

		case 0:
			if (!strcmp(long_options[idx].name, "numChn")) {

				gstAcapParam.all_useChn = atoi(optarg);
				CVI_AUTO_DBG_INFO("numChn == %d\n", gstAcapParam.all_useChn);
			} else if (!strcmp(long_options[idx].name, "chn")) {

				pstChnParam = &gstAcapParam.stChnParam[atoi(optarg)];
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

static CVI_BOOL _update_agc_anr_setting(AI_TALKVQE_CONFIG_S *pstAiVqeTalkAttr)
{
	if (pstAiVqeTalkAttr == NULL)
		return CVI_FALSE;

	pstAiVqeTalkAttr->u32OpenMask |= (NR_ENABLE | AGC_ENABLE | DCREMOVER_ENABLE);

	AUDIO_AGC_CONFIG_S st_AGC_Setting;
	AUDIO_ANR_CONFIG_S st_ANR_Setting;

	st_AGC_Setting.para_agc_max_gain = 0;
	st_AGC_Setting.para_agc_target_high = 2;
	st_AGC_Setting.para_agc_target_low = 72;
	st_AGC_Setting.para_agc_vad_ena = CVI_TRUE;
	st_ANR_Setting.para_nr_snr_coeff = 15;
	st_ANR_Setting.para_nr_init_sile_time = 0;



	pstAiVqeTalkAttr->stAgcCfg = st_AGC_Setting;
	pstAiVqeTalkAttr->stAnrCfg = st_ANR_Setting;

	pstAiVqeTalkAttr->para_notch_freq = 0;
	CVI_AUTO_INFO("pstAiVqeTalkAttr:u32OpenMask[0x%x]\n",
		      pstAiVqeTalkAttr->u32OpenMask);
	return CVI_TRUE;
}

static CVI_BOOL _update_aec_setting(AI_TALKVQE_CONFIG_S *pstAiVqeTalkAttr)
{
	if (pstAiVqeTalkAttr == NULL)
		return CVI_FALSE;

	AI_AEC_CONFIG_S default_AEC_Setting;

	memset(&default_AEC_Setting, 0, sizeof(AI_AEC_CONFIG_S));
	default_AEC_Setting.para_aec_filter_len = 13;
	default_AEC_Setting.para_aes_std_thrd = 37;
	default_AEC_Setting.para_aes_supp_coeff = 60;
	pstAiVqeTalkAttr->stAecCfg = default_AEC_Setting;
	pstAiVqeTalkAttr->u32OpenMask = LP_AEC_ENABLE | NLP_AES_ENABLE |
					NR_ENABLE | AGC_ENABLE;
	CVI_AUTO_INFO("pstAiVqeTalkAttr:u32OpenMask[0x%x]\n",
		      pstAiVqeTalkAttr->u32OpenMask);
	return CVI_FALSE;
}


static int check_is_rawData(char *infilename)
{
	AUTO_CHECK_NULL_PTR((void *)infilename);
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
		return 0;

	}
	CVI_AUTO_DBG_INFO("input data is not pcm/raw,maybe you should enable aenc\n");
	return -1;
}

static CVI_S32  _update_aenc_params(AENC_CHN_ATTR_S *pAencAttrs,
				    AIO_ATTR_S *pAioAttrs,
				    PAYLOAD_TYPE_E enType)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	AUTO_CHECK_NULL_PTR((void *)pAencAttrs);
	AUTO_CHECK_NULL_PTR((void *)pAioAttrs);

	memset(pAencAttrs, 0, sizeof(AENC_CHN_ATTR_S));
	pAencAttrs->enType = enType;
	pAencAttrs->u32BufSize = 30;
	pAencAttrs->u32PtNumPerFrm = pAioAttrs->u32PtNumPerFrm;

	if (pAencAttrs->enType == PT_ADPCMA) {
		AENC_ATTR_ADPCM_S *pstAdpcmAenc = (AENC_ATTR_ADPCM_S *)malloc(sizeof(
				AENC_ATTR_ADPCM_S));

		pstAdpcmAenc->enADPCMType = AUDIO_ADPCM_TYPE;
		pAencAttrs->pValue       = (CVI_VOID *)pstAdpcmAenc;
	} else if (pAencAttrs->enType == PT_G711A || pAencAttrs->enType == PT_G711U) {
		AENC_ATTR_G711_S *pstAencG711 = (AENC_ATTR_G711_S *)malloc(sizeof(
							AENC_ATTR_G711_S));

		pAencAttrs->pValue = (CVI_VOID *)pstAencG711;
	} else if (pAencAttrs->enType == PT_G726) {
		AENC_ATTR_G726_S *pstAencG726 = (AENC_ATTR_G726_S *)malloc(sizeof(
							AENC_ATTR_G726_S));
		pstAencG726->enG726bps = G726_BPS;
		pAencAttrs->pValue = (CVI_VOID *)pstAencG726;

	} else if (pAencAttrs->enType == PT_LPCM) {
		AENC_ATTR_LPCM_S *pstAencLpcm = (AENC_ATTR_LPCM_S *)malloc(sizeof(
							AENC_ATTR_LPCM_S));

		pAencAttrs->pValue = (CVI_VOID *)pstAencLpcm;
	} else if (pAencAttrs->enType == PT_AAC) {
		CVI_AUTO_INFO("Need update detail external AAC function params\n");
		//need update AAC if supported
	} else {
		CVI_AUTO_ERROR("Not support codec type[%d]\n", enType);
		s32Ret = CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 _update_Aenc_setting(AIO_ATTR_S *pstAioAttr,
			     AENC_CHN_ATTR_S *pstAencAttr,
			     PAYLOAD_TYPE_E enType,
			     int Sample_rate, bool bVqe)
{

	CVI_S32 s32Ret;

	s32Ret = _update_aenc_params(pstAencAttr, pstAioAttr, enType);
	if (s32Ret != CVI_SUCCESS) {
		CVI_AUTO_ERROR("error params\n");
		return CVI_FAILURE;
	}

	if (enType == PT_AAC) {
		AENC_ATTR_AAC_S  *pstAencAac = (AENC_ATTR_AAC_S *)malloc(sizeof(
							AENC_ATTR_AAC_S));
		pstAencAac->enAACType = gs_enAacType;
		pstAencAac->enBitRate = gs_enAacBps;
		pstAencAac->enBitWidth = AUDIO_BIT_WIDTH_16;

		pstAencAac->enSmpRate = Sample_rate;
		CVI_AUTO_INFO("AAC enc,smp-rate[%d]\n",
			      pstAencAac->enSmpRate);

		pstAencAac->enSoundMode = bVqe ? AUDIO_SOUND_MODE_MONO :
						pstAioAttr->enSoundmode;
		//pstAencAac->enSoundMode = pstAioAttr->enSoundmode;
		pstAencAac->enTransType = gs_enAacTransType;
		pstAencAac->s16BandWidth = 0;
		pstAencAttr->pValue = pstAencAac;
		s32Ret = CVI_MPI_AENC_AacInit();
		CVI_AUTO_INFO("s32Ret:%d\n", s32Ret);
	}

	pstAencAttr->bFileDbgMode = CVI_FALSE;

	return CVI_SUCCESS;
}

int _destroy_Aenc_setting(AENC_CHN_ATTR_S *pstAencAttr)
{
	AUTO_CHECK_NULL_PTR((void *)pstAencAttr);
	AUTO_CHECK_NULL_PTR((void *)pstAencAttr->pValue);

	if (pstAencAttr->enType == PT_AAC)
		CVI_MPI_AENC_AacDeInit();
	if (pstAencAttr->pValue) {
		free(pstAencAttr->pValue);
		pstAencAttr->pValue = NULL;
	}
	memset(pstAencAttr, 0, sizeof(AENC_CHN_ATTR_S));

	return 0;
}

int acap_audio_aenc_unbind(AENC_CHN_ATTR_S *pstAencAttr, int AeChn, int AiChn)
{
	int s32Ret = 0;
	AUDIO_STREAM_S stStream;
	AEC_FRAME_S   stAecFrm;
	AUDIO_FRAME_S stFrame;
	struct timespec end;
	struct timespec now;

	AUTO_CHECK_NULL_PTR((void *)pstAencAttr);

	clock_gettime(CLOCK_MONOTONIC, &now);
	end.tv_sec = now.tv_sec + gstAcapParam.record_time;
	end.tv_nsec = now.tv_nsec;

	FILE *fpAenc = fopen(gstAcapParam.stChnParam[AiChn].output_patch, "wb");

	AUTO_CHECK_NULL_PTR((void *)fpAenc);

	while (running) {

		s32Ret = CVI_AI_GetFrame(AiDev, AiChn, &stFrame, &stAecFrm, -1);
		if (s32Ret) {
			CVI_AUTO_ERROR("AiChn = %d, s32Ret = %#x\n", AiChn, s32Ret);
			fclose(fpAenc);
			return -1;
		}

		s32Ret = CVI_AENC_SendFrame(AeChn, &stFrame, &stAecFrm);
		if (s32Ret) {
			CVI_AUTO_ERROR("AiChn = %d, s32Ret = %#x\n", AiChn, s32Ret);
			fclose(fpAenc);
			return -1;
		}

		s32Ret = CVI_AENC_GetStream(AeChn, &stStream, -1);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("AiChn = %d, s32Ret = %#x\n", AiChn, s32Ret);
			fclose(fpAenc);
			return -1;
		}
		if (!stStream.u32Len) {
			continue;
		}

		fwrite(stStream.pStream, 1, stStream.u32Len, fpAenc);

		if (gstAcapParam.record_time) {
			clock_gettime(CLOCK_MONOTONIC, &now);
			if (now.tv_sec > end.tv_sec ||
			    (now.tv_sec == end.tv_sec && now.tv_nsec >= end.tv_nsec))
				break;
		}

	}

	fclose(fpAenc);

	return 0;
}
int acap_audio_aenc_bind(AENC_CHN_ATTR_S *pstAencAttr, int AeChn, int AiChn)
{
	AUTO_CHECK_NULL_PTR((void *)pstAencAttr);
	AUDIO_STREAM_S stStream;
	MMF_CHN_S stSrcChn, stDestChn;
	struct timespec end;
	struct timespec now;
	int s32Ret = 0;

	stSrcChn.enModId = CVI_ID_AI;
	stSrcChn.s32DevId = AiDev;
	stSrcChn.s32ChnId = AiChn;
	stDestChn.enModId = CVI_ID_AENC;
	stDestChn.s32DevId = AeDev;
	stDestChn.s32ChnId = AeChn;

	CVI_AUD_SYS_Bind(&stSrcChn, &stDestChn);

	clock_gettime(CLOCK_MONOTONIC, &now);
	end.tv_sec = now.tv_sec + gstAcapParam.record_time;
	end.tv_nsec = now.tv_nsec;

	FILE *fpAenc = fopen(gstAcapParam.stChnParam[AiChn].output_patch, "wb");

	AUTO_CHECK_NULL_PTR((void *)fpAenc);

	while (running) {

		s32Ret = CVI_AENC_GetStream(AeChn, &stStream, -1);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("AeChn[%d], AiChn[%d], s32Ret[%#x]\n", AeChn, AiChn, s32Ret);
			fclose(fpAenc);
			return -1;
		}
		if (!stStream.u32Len) {
			//CVI_AUTO_WARN("aenc len is 0\n");
			usleep(10 * 1000);
			continue;
		}


		fwrite(stStream.pStream, 1, stStream.u32Len, fpAenc);

		if (gstAcapParam.record_time) {
			clock_gettime(CLOCK_MONOTONIC, &now);
			if (now.tv_sec > end.tv_sec ||
			    (now.tv_sec == end.tv_sec && now.tv_nsec >= end.tv_nsec))
				break;
		}
	}

	fclose(fpAenc);

	return 0;
}


int acap_audio_file(AIO_ATTR_S *pstAioAttr, FILE *fp, int AiChn)
{
	AUTO_CHECK_NULL_PTR((void *)pstAioAttr);
	AUTO_CHECK_NULL_PTR((void *)fp);
	int s32Ret = CVI_SUCCESS;
	AUDIO_FRAME_S stFrame;
	AEC_FRAME_S   stAecFrm;
	struct timespec end;
	struct timespec now;
	int s32OutputChnCnt = gstAcapParam.channels;

	clock_gettime(CLOCK_MONOTONIC, &now);
	end.tv_sec = now.tv_sec + gstAcapParam.record_time;
	end.tv_nsec = now.tv_nsec;


	while (running) {

		s32Ret = CVI_AI_GetFrame(AiDev, AiChn, &stFrame, &stAecFrm, -1);
		if (s32Ret) {
			CVI_AUTO_ERROR("AiDev[%d], AiChn[%d], s32Ret[%#x]\n", AiDev, AiChn, s32Ret);
			return -1;
		}

		if (gstAcapParam.stChnParam[AiChn].bVqe == true)
			s32OutputChnCnt = 1;
		fwrite(stFrame.u64VirAddr[0], 1, stFrame.u32Len * s32OutputChnCnt * BYTE_ONE_SAMPLE, fp);

		if (gstAcapParam.record_time) {
			clock_gettime(CLOCK_MONOTONIC, &now);
			if (now.tv_sec > end.tv_sec ||
			    (now.tv_sec == end.tv_sec && now.tv_nsec >= end.tv_nsec))
				break;
		}

	}

	return 0;
}

void *AudAcapThread(void *pargv)
{
	AUTO_CHECK_NULL_PTR((void *)pargv);
	int s32Ret = CVI_SUCCESS;
	int AiChn = *(int *)pargv;
	FILE *Fp = NULL;
	//int *ps32Ret = (int *)malloc(sizeof(int));

	snprintf(ThreadRet, 50, "AcapThread[%d] Success", AiChn);

CVI_AUTO_INFO("open AiChn[%d][%d]:output_patch = %s, bindMond = %d, ChnSr = %d, bVqe = %d, AecnFormate = %s[%d]\n",
		      gstAcapParam.stChnParam[AiChn].currChn, AiChn,
		      gstAcapParam.stChnParam[AiChn].output_patch,
		      gstAcapParam.stChnParam[AiChn].bindMond,
		      gstAcapParam.stChnParam[AiChn].chnSampleRate,
			  gstAcapParam.stChnParam[AiChn].bVqe,
		      gstAcapParam.stChnParam[AiChn].Aencformate,
		      get_codec_type(gstAcapParam.stChnParam[AiChn].Aencformate));


	s32Ret = CVI_AI_EnableChn(AiDev, AiChn);
	if (s32Ret != CVI_SUCCESS) {
		CVI_AUTO_ERROR("AiDev[%d], AiChn[%d], s32Ret[%#x]\n", AiDev, AiChn, s32Ret);
		return NULL;
	}


	if (gstAcapParam.stChnParam[AiChn].bVqe == true) {

		s32Ret = CVI_AI_SetTalkVqeAttr(AiDev, AiChn, 0, 0,
					       &stAiVqeTalkAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("AiDev[%d], AiChn[%d], s32Ret[%#x]\n", AiDev, AiChn,
				       s32Ret);
			goto ERROR1;
		}

		s32Ret = CVI_AI_EnableVqe(AiDev, AiChn);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("AiDev[%d], AiChn[%d], s32Ret[%#x]\n", AiDev, AiChn,
				       s32Ret);
			goto ERROR1;
		}

	}


	if (gstAcapParam.stChnParam[AiChn].chnSampleRate != gstAcapParam.sampleRate) {
		s32Ret = CVI_AI_EnableReSmp(AiDev, AiChn,
					    gstAcapParam.stChnParam[AiChn].chnSampleRate);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("AiDev[%d], AiChn[%d], s32Ret[%#x] chnSr[%d] AiSr[%d]\n",
				       AiDev, AiChn, s32Ret, gstAcapParam.stChnParam[AiChn].chnSampleRate,
				       gstAcapParam.sampleRate);
			goto ERROR1;
		}
	}


	if (check_is_rawData(gstAcapParam.stChnParam[AiChn].output_patch) == 0) {

		Fp = fopen(gstAcapParam.stChnParam[AiChn].output_patch, "wb");
		if (Fp == NULL) {
			CVI_AUTO_ERROR("%s open fail\n", gstAcapParam.stChnParam[AiChn].output_patch);
			goto ERROR2;
		}
		s32Ret = acap_audio_file(&gstAcapParam.AiInstance, Fp, AiChn);
		if (s32Ret)
			goto ERROR2;

	} else {
		PAYLOAD_TYPE_E enType = get_codec_type(
						gstAcapParam.stChnParam[AiChn].Aencformate);
		//int AeChn = check_free_AeChn();
		int AeChn = AiChn;
		AENC_CHN_ATTR_S stAencAttr;

		CVI_AUTO_INFO("AeChn = %d, enType = %d\n", AeChn, enType);

		_update_Aenc_setting(&gstAcapParam.AiInstance, &stAencAttr, enType,
				     gstAcapParam.stChnParam[AiChn].chnSampleRate, gstAcapParam.stChnParam[AiChn].bVqe);

		s32Ret = CVI_AENC_CreateChn(AeChn, &stAencAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_AUTO_ERROR("AeChn = %d, s32Ret=%#x,\n", AeChn, s32Ret);
			goto ERROR2;
		}

		if (gstAcapParam.stChnParam[AiChn].bindMond == 1) {/*bindmode */
			s32Ret = acap_audio_aenc_bind(&stAencAttr, AeChn, AiChn);
			if (s32Ret)
				goto ERROR2;
		} else {/*unbindmode */
			s32Ret = acap_audio_aenc_unbind(&stAencAttr, AeChn, AiChn);
			if (s32Ret)
				goto ERROR2;
		}

		CVI_AENC_DestroyChn(AeChn);
		_destroy_Aenc_setting(&stAencAttr);
	}


	if (gstAcapParam.stChnParam[AiChn].bVqe == true)
		CVI_AI_DisableVqe(AiDev, AiChn);
	if (gstAcapParam.stChnParam[AiChn].chnSampleRate != gstAcapParam.sampleRate)
		CVI_AI_DisableReSmp(AiDev, AiChn);
	CVI_AI_DisableChn(AiDev, AiChn);
	gstAcapParam.stChnParam[AiChn].bStopAcap = true;
	return (void *)ThreadRet;

ERROR2:
	if (gstAcapParam.stChnParam[AiChn].bVqe == true)
		CVI_AI_DisableVqe(AiDev, AiChn);

	if (gstAcapParam.stChnParam[AiChn].chnSampleRate != gstAcapParam.sampleRate)
		CVI_AI_DisableReSmp(AiDev, AiChn);
ERROR1:
	CVI_AI_DisableChn(AiDev, AiChn);
	gstAcapParam.stChnParam[AiChn].bStopAcap = true;

	return NULL;
}



int main(int argc, char *argv[])
{
	int s32Ret = CVI_SUCCESS;
	int Chncount = 0;
	//int AiSupVqeChn = 0;
	int MaxAudioChn = 3;
	bool Thread_error = false;
	pthread_t AiDevThread[AUD_AI_MAX_CHN_NUM];
	void *ThreadRet[AUD_AI_MAX_CHN_NUM];
	AIO_ATTR_S AudinAttr;



	register_inthandler();

	s32Ret = parseAcapArgv(argc, argv);
	if (s32Ret < 0) {
		if (s32Ret == STATUS_HELP)
			return CVI_SUCCESS;
		return CVI_FAILURE;
	}

	CVI_AUTO_INFO("sr:%d, chn:%d, period_size:%d, period_count:%d, record_time:%d, all_useChn:%d\n",
		      gstAcapParam.sampleRate, gstAcapParam.channels, gstAcapParam.period_size,
		      gstAcapParam.period_count,
		      gstAcapParam.record_time, gstAcapParam.all_useChn);


	for (int i = 0; i < AUD_AI_MAX_CHN_NUM; i++) {
		if (gstAcapParam.stChnParam[i].currChn != -1) {
			CVI_AUTO_INFO("chn[%d]:output_patch:%s, bindMond:%d, ChnSr:%d, bVqe:%d, AecnFormate:%s[%d]\n",
				      gstAcapParam.stChnParam[i].currChn,
				      gstAcapParam.stChnParam[i].output_patch, gstAcapParam.stChnParam[i].bindMond,
				      gstAcapParam.stChnParam[i].chnSampleRate,
				      gstAcapParam.stChnParam[i].bVqe, gstAcapParam.stChnParam[i].Aencformate,
				      get_codec_type(gstAcapParam.stChnParam[i].Aencformate));
		}

	}


//STEP 1: set ai and enable ai

	AudinAttr.enSamplerate = (AUDIO_SAMPLE_RATE_E)gstAcapParam.sampleRate;
	AudinAttr.u32ChnCnt = MaxAudioChn;
	AudinAttr.enSoundmode = (gstAcapParam.channels == 2 ? AUDIO_SOUND_MODE_STEREO :
				 AUDIO_SOUND_MODE_MONO);
	AudinAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	AudinAttr.enWorkmode = AIO_MODE_I2S_MASTER;
	AudinAttr.u32EXFlag = 0;
	AudinAttr.u32FrmNum = 10; /* only use in bind mode */
	AudinAttr.u32PtNumPerFrm = gstAcapParam.period_size; /* sample_rate/fps */
	AudinAttr.u32ClkSel = 0;
	AudinAttr.enI2sType = AIO_I2STYPE_INNERCODEC;

	memcpy(&gstAcapParam.AiInstance, &AudinAttr, sizeof(AIO_ATTR_S));
	/*if you want to use vqe ,ai chn must 2chn.*/
	/*if you don't need to use vqe, you can skip this step*/


	AI_TALKVQE_CONFIG_S *pstAiVqeTalkAttr = (AI_TALKVQE_CONFIG_S *)&stAiVqeTalkAttr;

	if (((AudinAttr.enSamplerate == AUDIO_SAMPLE_RATE_8000) ||
	     (AudinAttr.enSamplerate == AUDIO_SAMPLE_RATE_16000)) &&
	   gstAcapParam.channels == 2) {

		pstAiVqeTalkAttr->s32WorkSampleRate = AudinAttr.enSamplerate;
		_update_agc_anr_setting(pstAiVqeTalkAttr);
		_update_aec_setting(pstAiVqeTalkAttr);

	} else {
		CVI_AUTO_WARN("AEC will need to setup record in to channel Count = 2\n");
		CVI_AUTO_WARN("VQE only support on 8k/16k sample rate. current[%d]\n",
			      AudinAttr.enSamplerate);
	}

	s32Ret = CVI_AUDIO_INIT();
	if (s32Ret != CVI_SUCCESS) {
		CVI_AUTO_ERROR("audio init error. s32Ret[%#x]\n", s32Ret);
		return -1;
	}


	s32Ret = CVI_AI_SetPubAttr(AiDev, &AudinAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_AUTO_ERROR("ai set attr error. AiDev[%d],s32Ret[%#x]\n", AiDev, s32Ret);
		return -1;
	}
	s32Ret = CVI_AI_Enable(AiDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_AUTO_ERROR("ai enable error. AiDev[%d],s32Ret[%#x]\n", AiDev, s32Ret);
		return -1;
	}

//STEP 2: Create threads and send data to devices
	for (int i = 0; i < AUD_AI_MAX_CHN_NUM; i++) {

		if (gstAcapParam.stChnParam[i].currChn != -1) {
			CVI_AUTO_INFO("creat Thread chn[%d][%d]\n", i,
				      gstAcapParam.stChnParam[i].currChn);
			pthread_create(&AiDevThread[i], NULL,
				       (CVI_VOID * (*)(CVI_VOID *))AudAcapThread, &gstAcapParam.stChnParam[i].currChn);
			Chncount++;
			CVI_AUTO_INFO("chn_count = %d\n", Chncount);
		}
	}

	if (Chncount != gstAcapParam.all_useChn)
		CVI_AUTO_ERROR("Thread_num[%d] != all_useChn[%d]\n", Chncount,
			       gstAcapParam.all_useChn);

	for (int i = 0; i < AUD_MAX_CHN_NUM; i++) {
		if (gstAcapParam.stChnParam[i].currChn != -1) {
			while (!gstAcapParam.stChnParam[i].bStopAcap) {
				usleep(1000 * 20);
			}

			pthread_join(AiDevThread[i], &ThreadRet[i]);
			CVI_AUTO_INFO("chn[%d][%d][%s] the end of playing\n", i,
				      gstAcapParam.stChnParam[i].currChn, (char *)ThreadRet[i]);

			if (ThreadRet[i] == NULL)
				Thread_error = true;
		}
	}

	CVI_AI_Disable(AiDev);
	CVI_AUDIO_DEINIT();
	if (Thread_error == false)
		printf("TEST-PASS\n");

	return 0;

}


