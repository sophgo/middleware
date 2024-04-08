#ifndef CVI_INTERNAL_AUDIO_TEST_INCLUDE_H_
#define CVI_INTERNAL_AUDIO_TEST_INCLUDE_H_
#include "cvi_audio.h"
#include "asoundlib.h"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>


CVI_S32 cvi_audio_set_dbg_level(CVI_S32 dbglevel);
int check_param(struct pcm_params *params, unsigned int param, unsigned int value,
		       char *param_name, char *param_unit);

int _transform_pcm_to_wave(const char *pcmpath, int channels,
				  int sample_rate,
				  int bits_per_sample,
				  const char *wavepath);

CVI_S32 _preflush_audin_frm_record(struct pcm *capture_handle,
		CVI_S32 period_size,
		CVI_S32 preiod_size_inBytes,
		CVI_S32 sample_rate);

CVI_S32 _cvi_audio_recordfile_sample(CVI_S32 DevId, CVI_S32 rate, CVI_S32 channel, ST_AudioUnitTestCfg *testCfg);


CVI_BOOL _cvi_checkname_iswav(char *infilename);


int sample_is_playable(unsigned int card, unsigned int device, unsigned int channels,
			      unsigned int rate, unsigned int bits, unsigned int period_size,
			      unsigned int period_count);

CVI_S32 _cvi_audio_playfile_sample3(CVI_CHAR *filename, CVI_S32 DevId, ST_AudioUnitTestCfg *testCfg);
int _aud_scanf(char *printout, int *getvalue, int default_value);
int _aud_scanfchar(char *printout, char *getvalue, char *default_char);
CVI_S32 cvi_audio_set_dbg_option(ST_AudioUnitTestCfg *testCfg);
CVI_S32 cvi_audio_set_dbg_option2(ST_AudioUnitTestCfg *testCfg);
CVI_S32 cvi_audio_set_dbg_record(ST_AudioUnitTestCfg *testCfg);
CVI_S32 cvi_audio_set_dbg_play(ST_AudioUnitTestCfg *testCfg);
CVI_S32 cvi_audio_set_dbg_vqe_play(ST_AudioUnitTestCfg *testCfg);
//CVI_S32 cvi_audio_set_dbg_set_volume(ST_AudioUnitTestCfg *testCfg);
//CVI_S32 cvi_audio_set_dbg_get_volume(ST_AudioUnitTestCfg *testCfg);

#endif
