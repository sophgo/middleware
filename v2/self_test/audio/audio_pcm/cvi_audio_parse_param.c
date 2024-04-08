#include "cvi_audio_parse_param.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
//#include "sample_comm.h"
#include "acodec.h"


stAudioPara stAudioparam;
int parse_command_valid;

int printf_usage(void)
{
	printf("use: sample_audio 1 --list -r [sample_rate]");
	printf(" -c [channel] -p [preiod_size] -C [codec 0:g726 1:g711A 2:g711Mu 3: adpcm 4.AAC]");
	printf(" -V [bVqeOn] -A [bAecOn] -R [bResmp] -b [bind_mode] -T [record time]\n");
	printf("eg:./sample_audio 1 --list -r 8000 -c 2 -p 320 -C 0 -V 1 -A 1 -R 0 -b 0 -T 20\n");
	return 0;
}

#define CVIAUDIO_PARSE_LONG_OPT_NUMS 3
#ifndef NO_ARGUMENT
#define NO_ARGUMENT		0
#endif

#ifndef REQUIRED_ARGUMENT
#define REQUIRED_ARGUMENT	1
#endif

#ifndef OPTIONAL_ARGUMENT
#define OPTIONAL_ARGUMENT	2
#endif

typedef enum _E_CVIAUDIO_LONG_OPT_CMD {
	E_CVIAUDIO_LONG_OPT_CMD_NAME = 0,
	E_CVIAUDIO_LONG_OPT_CMD_AIN_VOL = 1,
	E_CVIAUDIO_LONG_OPT_CMD_AOUT_VOL = 2,
	E_CVIAUDIO_LONG_OPT_CMD_MAX_SIZE = 3,
} E_CVIAUDIO_LONG_OPT_CMD;


static void _init_audioparam(stAudioPara *pAudioparam)
{
	if (pAudioparam == NULL)
		return;

	memset(pAudioparam, 0, sizeof(stAudioPara));
	strcpy(pAudioparam->filename, "None");
	pAudioparam->ain_volume = CVIAUDIO_PARSE_NONE_SET;
	pAudioparam->aout_volume = CVIAUDIO_PARSE_NONE_SET;
}

int audio_parse(int argc, char **argv)
{
	int i, ret;
	int index = E_CVIAUDIO_LONG_OPT_CMD_MAX_SIZE;

	//memset(&stAudioparam, 0, sizeof(stAudioPara));
	_init_audioparam(&stAudioparam);

	for (i = 1; i < argc - 2; i++) {
		argv[i] = argv[i + 2];
	}
	argv[argc - 2] = "\n";
	argv[argc - 1] = "\n";

	struct option longOptions[] = {
		{ "name", REQUIRED_ARGUMENT, NULL, 0 },
		{ "ain_volume", REQUIRED_ARGUMENT, NULL, 0 },
		{ "aout_volume", REQUIRED_ARGUMENT, NULL, 0 },
		{ 0, 0, 0, 0 },
	};

	//while ((ret = getopt(argc, argv, "r:c:p:C:V:A:R:b:T:")) != -1) {
	while ((ret = getopt_long(argc, argv, "r:c:p:C:V:A:R:b:T:", longOptions, &index)) != -1) {
		switch (ret) {
		case 'r':
			stAudioparam.sample_rate = atoi(optarg);
			break;
		case 'c':
			stAudioparam.channel = atoi(optarg);
			break;
		case 'p':
			stAudioparam.preiod_size = atoi(optarg);
			break;
		case 'C':
			stAudioparam.codec = atoi(optarg);
			break;
		case 'V':
			stAudioparam.bVqeOn = (bool)atoi(optarg);
			break;
		case 'A':
			stAudioparam.bAecOn = (bool)atoi(optarg);
			break;
		case 'R':         //ptind=13    1
			stAudioparam.bResmp = (bool)atoi(optarg);
			break;
		case 'b':
			stAudioparam.bind_mode = atoi(optarg);
			break;
		case 'T':
			stAudioparam.record_time = atoi(optarg);
			break;
		case 0:
			if (!strcmp(longOptions[index].name, "name")) {
				memcpy(stAudioparam.filename, optarg, strlen(optarg));
				printf("parsing nmae index[%d][%s]\n", index, stAudioparam.filename);
			} else if (!strcmp(longOptions[index].name, "ain_volume")) {
				stAudioparam.ain_volume = atoi(optarg);
				printf("parsing ain_volume[%d]\n", stAudioparam.ain_volume);
			} else if (!strcmp(longOptions[index].name, "aout_volume")) {
				stAudioparam.aout_volume = atoi(optarg);
				printf("parsing aout_volume[%d]\n", stAudioparam.ain_volume);
			}
			break;
		default:
			printf_usage();
			return -1;
		}


	}
	// stAudioparam.eType = get_audio_codec(codec);
	parse_command_valid = 1;
	return 0;
}


int _get_parseval(stAudioPara stAudparam)
{
	memcpy(&stAudparam, &stAudioparam, sizeof(stAudioPara));
	return 0;
}


int _parsing_audio_status(void)
{
	return parse_command_valid;
}

char *Search_string(char *str1, char *str2)
{
	char *p;

	if (str1 == NULL || str2 == NULL) {
		printf("str1 or str2 is null\n");
		return NULL;
	}
	p = strstr(str1, str2);
	//if(p != NULL)
	//printf("%s",p);
	return p;
}

void *_parsing_request_name(char *printout, int default_val)
{
	if (printout != NULL) {
		if (Search_string(printout, "name") != NULL) {
			printf("filename = %s\n", stAudioparam.filename);
			return (void *)stAudioparam.filename;
		}

	}
	printf("Error input type[%s][%d]\n", __func__, __LINE__);
	return NULL;
}

int _parsing_request(char *printout, int default_val)
{

	if (printout != NULL) {
		if (Search_string(printout, "sample rate") != NULL) {
			printf("sample_rate = %d\n", stAudioparam.sample_rate);
			return stAudioparam.sample_rate;
		}

		if (Search_string(printout, "channel") != NULL) {
			printf("channel = %d\n", stAudioparam.channel);
			return stAudioparam.channel;
		}

		if (Search_string(printout, "period size") != NULL) {
			printf("preiod_size = %d\n", stAudioparam.preiod_size);
			return stAudioparam.preiod_size;
		}

		if (Search_string(printout, "codec") != NULL) {
			printf("codec = %d\n", stAudioparam.codec);
			return stAudioparam.codec;
		}

		if (Search_string(printout, "bVqeOn") != NULL) {
			printf("bVqeOn = %d\n", stAudioparam.bVqeOn);
			return stAudioparam.bVqeOn;
		}
		if (Search_string(printout, "bAecOn") != NULL) {
			printf("bAecOn = %d\n", stAudioparam.bAecOn);
			return stAudioparam.bAecOn;
		}

		if (Search_string(printout, "resample") != NULL) {
			printf("bResmp = %d\n", stAudioparam.bResmp);
			return stAudioparam.bResmp;
		}

		if (Search_string(printout, "bind mode") != NULL) {
			printf("bind_mode = %d\n", stAudioparam.bind_mode);
			return stAudioparam.bind_mode;
		}
		if (Search_string(printout, "record time") != NULL) {
			printf("record_time = %d\n", stAudioparam.record_time);
			return stAudioparam.record_time;
		}
#if 0
		if (Search_string(printout, "name") != NULL) {
			printf("filename = %s\n", stAudioparam.filename);
			int addr_tmp = (int)stAudioparam.filename;
			return addr_tmp;
		}
#endif
		if (Search_string(printout, "ain_volume") != NULL) {
			printf("ain_volume = %d\n", stAudioparam.ain_volume);
			return stAudioparam.ain_volume;
		}

		if (Search_string(printout, "aout_volume") != NULL) {
			printf("aout_volume = %d\n", stAudioparam.aout_volume);
			return stAudioparam.aout_volume;
		}
		printf("use default_val = %d\n", default_val);
		return default_val;
	}
	printf("Error input type[%s][%d]\n", __func__, __LINE__);
	return -1;
}


