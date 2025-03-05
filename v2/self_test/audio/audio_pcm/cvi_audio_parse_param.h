#ifndef __CVI_AUDIO_PARSE_PARAM_H__
#define __CVI_AUDIO_PARSE_PARAM_H__

//#include "sample_comm.h"
#if defined(__CV181X__) || defined(__CV180X__) || defined(__CV186X__)
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <cvi_type.h>
#include <cvi_common.h>
#include <string.h>
#else
#include "cvi_common.h"
#endif

#define CVIAUDIO_PARSE_FILE_LENGTH 128
#define CVIAUDIO_PARSE_NONE_SET (-1)
typedef struct {
	int sample_rate;
	int channel;
	int preiod_size;
	int codec;
	//PAYLOAD_TYPE_E eType;
	bool bVqeOn;
	bool bAecOn;
	bool bResmp;
	int bind_mode;
	int record_time;
	//use for long option
	char filename[CVIAUDIO_PARSE_FILE_LENGTH];
	int ain_volume;
	int aout_volume;
} stAudioPara;



int audio_parse(int argc, char **argv);
int _get_parseval(stAudioPara stAudparam);
int _parsing_audio_status(void);
int _parsing_request(char *printout, int default_val);
void *_parsing_request_name(char *printout, int default_val);

#endif


