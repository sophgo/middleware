#ifndef __CVI_AUTO_TEST_H_
#define __CVI_AUTO_TEST_H_

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <inttypes.h>
#include <getopt.h>


int cvi_auto_dbg = 2;

#define CVI_AUTO_ERROR(fmt, args...) \
			do {\
				if (cvi_auto_dbg > 0) \
					printf("[cvi_auto_error][%s][%d] "fmt, __func__, __LINE__, ##args); \
			} while (0)

#define CVI_AUTO_WARN(fmt, args...) \
			do {\
				if (cvi_auto_dbg > 0) \
					printf("[cvi_auto_warn][%s][%d] "fmt, __func__, __LINE__, ##args); \
			} while (0)

#define CVI_AUTO_INFO(fmt, args...) \
			do {\
				if (cvi_auto_dbg > 1) \
					printf("[cvi_auto_info][%s][%d] "fmt, __func__, __LINE__, ##args); \
			} while (0)

#define CVI_AUTO_DBG_INFO(fmt, args...) \
			do {\
				if (cvi_auto_dbg > 2) \
					printf("[cvi_auto_info][%s][%d] "fmt, __func__, __LINE__, ##args); \
			} while (0)



#define AUTO_FREE_BUF(OBJ) {if (NULL != OBJ) {free(OBJ); OBJ = NULL; } }
#define AUTO_AUD_UNUSED_REF(X)  ((X) = (X))

#define AUD_MAX_CHN_NUM 3
#define AUD_AI_MAX_CHN_NUM 3

#define MAX_AUD_OPTIONS 128
#define MAX_AUD_STRING_LEN 128
#define STATUS_HELP -2
#define BYTE_ONE_SAMPLE 2

#define AUDIO_ADPCM_TYPE ADPCM_TYPE_DVI4/* ADPCM_TYPE_IMA, ADPCM_TYPE_DVI4*/
#define G726_BPS MEDIA_G726_32K         /* MEDIA_G726_16K, MEDIA_G726_24K ... */

#define BYTES_PER_SAMPLE 2
#define MS_DATA 20


/* WAV */
#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT	0x20746d66
#define ID_DATA 0x61746164

struct riff_wave_header {
	unsigned int riff_id;
	unsigned int riff_sz;
	unsigned int wave_id;
};

struct chunk_header {
	unsigned int id;
	unsigned int sz;
};

struct chunk_fmt {
	unsigned short audio_format;
	unsigned short num_channels;
	unsigned int sample_rate;
	unsigned int byte_rate;
	unsigned short block_align;
	unsigned short bits_per_sample;
};

typedef struct _optionExt_ {
	struct option opt;
	int64_t min;
	int64_t max;
	const char *help;
} optionExt;

#endif /* End of #ifndef __CVI_AUTO_TEST_H_*/


