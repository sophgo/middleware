#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>
#include "asoundlib.h"
#include "cvi_audio_uac.h"
#include "cyclebuffer.h"


#define __VERSION_TAG__   "CVI_UAC_20210412_dual_RefineCode"
#define CV18XX_AS_HOST 0//1 ic as usb host, 0 ic as usb device(slave)

#ifdef THIS_IS_32
#define AUDIO_PERIOD_SIZE 320
#define CAP_PERIOD_COUNT 4
#else
#define AUDIO_PERIOD_SIZE 960
#define CAP_PERIOD_COUNT 10
#endif


#define PLAY_PERIOD_COUNT 4
#define CHANNEL_COUNT 2
#define SAVE_FILE_AUDIO_FROM_MIC 0
#define SAVE_FILE_AUDIO_TO_SPK 0
#define BYTES_PER_SAMPLE 2
#define DOWNLINK_USE_CYCLE_BUFFER 1
#define UPLINK_USE_CYCLE_BUFFER 1
int bUplinkEnable;
int  bDownlinkEnable;

//define print level -------------------------------------[start]
#define CVI_UAC_MASK_ERR	(0x01)
#define CVI_UAC_MASK_INFO	(0x01)
#define CVI_UAC_MASK_WARN	(0x02)
#define CVI_UAC_MASK_DBG	(0x04)
#define CVI_UAC_MASK_TRACE	(0x08)

int  gUAC_level;
#define ERR_UAC_PRINT(msg, ...)	\
	do { \
		if (gUAC_level >= CVI_UAC_MASK_ERR) \
		printf("[UAC][ERR] %s = %d, "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define INFO_UAC_PRINT(msg, ...)	\
	do { \
		if (gUAC_level >= CVI_UAC_MASK_INFO) \
		printf("[UAC][INFO] %s = %d, "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define WARN_UAC_PRINT(msg, ...)	\
	do { \
		if (gUAC_level >= CVI_UAC_MASK_WARN) \
		printf("[UAC][WARN] %s = %d, "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define DBG_UAC_PRINT(msg, ...)	\
	do { \
		if (gUAC_level >= CVI_UAC_MASK_DBG) \
		printf("[UAC][DBG] %s = %d, "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define TRACE_UAC_PRINT(msg, ...)	\
	do { \
		if (gUAC_level == CVI_UAC_MASK_TRACE) \
		printf("[UAC][DBG] %s = %d, "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

static int  cviUacGetEnv(char *env, char *fmt, void *param);
static void cvi_audio_uac_getDbgMask(void);
//define print level .................................................[end]

#if DOWNLINK_USE_CYCLE_BUFFER
#define DOWNLINK_CYCLE_BUFFER_SIZE (1024 * 30 * 4)
#define DOWNLINK_SRC_OPEN_STATE 0x41
#define DOWNLINK_SRC_DETECT_STATE 0x42
#define DOWNLINK_SRC_SEND_DATA_STATE 0x43
#define DOWNLINK_SRC_CLOSE_AND_RETRY_STATE 0x44
#define DOWNLINK_DST_OPEN_STATE 0x51
#define DOWNLINK_DST_DETECT_STATE 0x52
#define DOWNLINK_DST_SEND_DATA_STATE 0x53
#define DOWNLINK_DST_SEND_DATA_MUTE_STATE 0x54
#define DOWNLINK_DST_CLOSE_AND_RETRY_STATE 0x55
pthread_t *pcm_downlink_src_usb_thread;
pthread_t *pcm_downlink_dst_spk_thread;
static void *thread_downlink_src_audio(void *arg);
static void *thread_downlink_dst_audio(void *arg);
void *gpstDownlinkCB;
char *pDownlinkCBuffer;
char *buffer_dl_src;
char *buffer_dl_dst;
#else
pthread_t *pcm_pc_to_mic_thread;
char *buffer; //downlink
#endif

#if UPLINK_USE_CYCLE_BUFFER
#define UPLINK_CYCLE_BUFFER_SIZE (960 * 16*4)
#define UPLINK_SRC_OPEN_STATE 0x61
#define UPLINK_SRC_DETECT_STATE 0x62
#define UPLINK_SRC_SEND_DATA_STATE 0x63
#define UPLINK_SRC_FORCE_FLUSH_STATE 0x64
#define UPLINK_SRC_CLOSE_AND_RETRY_STATE 0x65
#define UPLINK_DST_OPEN_STATE 0x71
#define UPLINK_DST_DETECT_STATE 0x72
#define UPLINK_DST_SEND_DATA_STATE 0x73
#define UPLINK_DST_SEND_DATA_MUTE_STATE 0x74
#define UPLINK_DST_CLOSE_AND_RETRY_STATE 0x75
pthread_t *pcm_uplink_src_mic_thread;
pthread_t *pcm_uplink_dst_usb_thread;
static void *thread_uplink_src_audio(void *arg);
static void *thread_uplink_dst_audio(void *arg);
char *buffer_up_src;
char *buffer_up_dst;
void *gpstUplinkCB;
char *pUplinkCBuffer;
char  *pflush_up_buffer;
#else
pthread_t *pcm_mic_to_pc_thread;
char *bufferup; //uplink
char *bufferup2;
#endif

struct pcm_config stPcmCapConfigUp = {
	.channels = 2,
	.rate = 48000,
	.period_size = AUDIO_PERIOD_SIZE,
	.period_count = CAP_PERIOD_COUNT,
	.format = PCM_FORMAT_S16_LE,
	.start_threshold = 0,
	.stop_threshold = INT_MAX,
};


struct pcm_config stPcmPlayConfigUp = {
	.channels = 2,
	.rate = 48000,
	.period_size = AUDIO_PERIOD_SIZE,
	.period_count = PLAY_PERIOD_COUNT,
	.format = PCM_FORMAT_S16_LE,
	.start_threshold = 0,
	.stop_threshold = INT_MAX,
};

#if UPLINK_USE_CYCLE_BUFFER
static void *thread_uplink_src_audio(void *arg)
{
#if 0
#define UPLINK_SRC_OPEN_STATE 0x61
#define UPLINK_SRC_DETECT_STATE 0x62
#define UPLINK_SRC_SEND_DATA_STATE 0x63
#define UPLINK_SRC_FORCE_FLUSH_STATE 0x64
#define UPLINK_SRC_CLOSE_AND_RETRY_STATE 0x65
#endif
	int	err;
	int err_read = -1;
	int sizebytes = 0;
	int ret_b;
	int ret_b_count = 0;
	int detect_mic_count = 0;
	int flush_count = 0;
	struct pcm *pcm_recordhandleup = NULL;
	char zero_bufferup[AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT*2];
	int uplink_srcstate = UPLINK_SRC_OPEN_STATE;
#define EDGE_AS_SRC_STABLE_THRESHOLD 2

	sizebytes = AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT;
	memset(zero_bufferup, 0, (AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT*2));


	while (bUplinkEnable) {
		switch (uplink_srcstate) {
		case UPLINK_SRC_OPEN_STATE:
		{
			if (err_read != 0) {
				pcm_recordhandleup = pcm_open(0, 0, PCM_IN, &stPcmCapConfigUp);
				if (!pcm_recordhandleup || !pcm_is_ready(pcm_recordhandleup)) {
					ERR_UAC_PRINT("Unable to openpcm_recordhandle (%s)\n",
							pcm_get_error(pcm_recordhandleup));
					ERR_UAC_PRINT("\n");
					usleep(500*1000);
					sleep(1);
					//force return for the open failure;
				} else {
					err_read = 0;
					DBG_UAC_PRINT("pcm_recordhandleup success\n");
				}
			}
			uplink_srcstate = UPLINK_SRC_DETECT_STATE;
		}
		break;

		case UPLINK_SRC_DETECT_STATE:
		{
			sizebytes = AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT;
			err_read = pcm_read(pcm_recordhandleup, buffer_up_src, sizebytes);

			if (err_read == 0) {
				detect_mic_count++;
				if (detect_mic_count > EDGE_AS_SRC_STABLE_THRESHOLD) {
					DBG_UAC_PRINT("Edge  as uplink  source(mic in) stable\n");
					uplink_srcstate = UPLINK_SRC_SEND_DATA_STATE;
					detect_mic_count = 0;
				} else {
					uplink_srcstate = UPLINK_SRC_DETECT_STATE;
				}
			} else {
				detect_mic_count = 0;
				uplink_srcstate = UPLINK_SRC_CLOSE_AND_RETRY_STATE;
				 sleep(1);
				 usleep(500*1000);
				 DBG_UAC_PRINT("UPLINK_SRC_DETECT_STATE(mic in)  failure ...go to retry\n");
			}
		}
		break;

		case UPLINK_SRC_SEND_DATA_STATE:
		{
			sizebytes = AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT;
			err_read = pcm_read(pcm_recordhandleup, buffer_up_src, AUDIO_PERIOD_SIZE*2*2);
			if (err_read != 0) {
				ERR_UAC_PRINT("card 0[%d][%s]\n", __LINE__, pcm_get_error(pcm_recordhandleup));
				ERR_UAC_PRINT("[uplinksrc_audio src] pcm read NG, go close and retry....\n");
				sleep(1);
				usleep(500*1000);
				uplink_srcstate = UPLINK_SRC_CLOSE_AND_RETRY_STATE;

			} else {
				ret_b =  CycleBufferWrite(gpstUplinkCB, buffer_up_src, sizebytes);
				if (!ret_b) {
					//buffer abnormal
					ret_b_count++;
					usleep(1000*sizebytes/(64));
					if (ret_b_count >= 8) {
					ret_b = CycleBufferDataLen(gpstUplinkCB);
					DBG_UAC_PRINT("[Error]uplink cycle buffer abnormal bufferlevel[%d] of [%d]\n",
							ret_b, UPLINK_CYCLE_BUFFER_SIZE);
					//cannot write into buffer/ cycle buffer  overflow
					//cycle buffer not consuming or usb did not connect in uplink dst
					//go to force flush
					uplink_srcstate = UPLINK_SRC_FORCE_FLUSH_STATE;
					ret_b_count = 0;
					}
				} else {
					//buffer ok
					uplink_srcstate = UPLINK_SRC_SEND_DATA_STATE;
					ret_b_count = 0;
					flush_count = 0;
				}
			}
		}
		break;

		case UPLINK_SRC_FORCE_FLUSH_STATE:
		{
			int retsize;

			ret_b = CycleBufferDataLen(gpstUplinkCB);
			if (ret_b != 0)
				retsize = CycleBufferRead(gpstUplinkCB, pflush_up_buffer, ret_b);
			if (retsize != ret_b)
				ERR_UAC_PRINT("[Error]size mismatch[%d]\n", __LINE__);


			flush_count++;
			if (flush_count > 10) {
				uplink_srcstate = UPLINK_SRC_CLOSE_AND_RETRY_STATE;
				flush_count = 0;
				//usb as dst has close for a while
			} else {
				//retry to send data into cycle buffer
				uplink_srcstate = UPLINK_SRC_SEND_DATA_STATE;
			}

		}
		break;

		case UPLINK_SRC_CLOSE_AND_RETRY_STATE:
		{
			free(buffer_up_src);
			if (err_read != 0) {
				DBG_UAC_PRINT("pcm_close pcm_recordhandleup\n");
				pcm_close(pcm_recordhandleup);
				pcm_recordhandleup = NULL;
			}
			buffer_up_src = (char *)malloc(AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT);
			DBG_UAC_PRINT("UPLINK_SRC_CLOSE_AND_RETRY_STATE\n");
			sleep(1);
			uplink_srcstate = UPLINK_SRC_OPEN_STATE;
		}
		break;

		default:
			ERR_UAC_PRINT("[Error]uplink edge(mic in) as src abnormal...[%d]\n", uplink_srcstate);
			sleep(2);
			break;
		}
	}
	return (void *)0;
}


static void *thread_uplink_dst_audio(void *arg)
{
#if 0
#define UPLINK_DST_OPEN_STATE 0x71
#define UPLINK_DST_DETECT_STATE 0x72
#define UPLINK_DST_SEND_DATA_STATE 0x73
#define UPLINK_DST_SEND_DATA_MUTE_STATE 0x74
#define UPLINK_DST_CLOSE_AND_RETRY_STATE 0x75
#endif
	int err;
	int err_write = -1;
	int ret_b = 0;
	struct pcm *pcm_playhandleup = NULL;
	int detect_usb_count = 0;
	char zero_bufferup[AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT*2];
	int sizebytes = AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT;
#define  USB_AS_SPK_STABLE_THRESHOLD 2
	memset(zero_bufferup, 0, (AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT*2));

	int uplink_dststate = UPLINK_DST_OPEN_STATE;

	while (bUplinkEnable) {
		switch (uplink_dststate) {
		case UPLINK_DST_OPEN_STATE:
		{
			if (err_write != 0) {
				pcm_playhandleup = pcm_open(2, 0, PCM_OUT, &stPcmPlayConfigUp);
				if (!pcm_playhandleup || !pcm_is_ready(pcm_playhandleup)) {
					INFO_UAC_PRINT("Unable to open pcm_playhandleup (%s)\n",
							pcm_get_error(pcm_playhandleup));
					INFO_UAC_PRINT("\n");
					usleep(500*1000);
					//return 0;
					//force return for the open failure
				} else {
					err_write = 0;
					DBG_UAC_PRINT("pcm_playhandleup success\n");
				}
			}
			uplink_dststate = UPLINK_DST_DETECT_STATE;
		}
		break;

		case UPLINK_DST_DETECT_STATE:
		{
			err_write = pcm_write(pcm_playhandleup, zero_bufferup, sizebytes);
			if (err_write != 0) {
				INFO_UAC_PRINT("[usb as spk]uplink to usb dst failure\n");
				sleep(2);
				detect_usb_count = 0;
				memset(zero_bufferup, 0, sizebytes);
				uplink_dststate = UPLINK_DST_CLOSE_AND_RETRY_STATE;
			} else {
				detect_usb_count++;
				if (detect_usb_count > USB_AS_SPK_STABLE_THRESHOLD) {
					DBG_UAC_PRINT("uplink usb_dst in detect .....ok[%d]\n", detect_usb_count);
					uplink_dststate = UPLINK_DST_SEND_DATA_STATE;
					detect_usb_count = 0;
				} else {
					uplink_dststate = UPLINK_DST_DETECT_STATE;
				}
			}
		}
		break;

		case UPLINK_DST_SEND_DATA_STATE:
		{
			ret_b = CycleBufferDataLen(gpstUplinkCB);
			if (ret_b >= sizebytes * 2) {
				//sufficient with buffering 2 frame in cycble buffer
				//send the true data
				ret_b = CycleBufferRead(gpstUplinkCB, buffer_up_dst, sizebytes);
				if (ret_b != sizebytes) {
					//...cycle buffer abnormal
					DBG_UAC_PRINT("Error not enough in cycle buffer ...abnormal[%d]\n", __LINE__);
				}
				err_write = pcm_write(pcm_playhandleup, buffer_up_dst, sizebytes);
				if (err_write != 0) {
					INFO_UAC_PRINT("card 2[%d][%s]\n", __LINE__, pcm_get_error(pcm_playhandleup));
					INFO_UAC_PRINT("Error pcm_write failure in uplink dst...go retry\n");
					sleep(2);
					uplink_dststate = UPLINK_DST_CLOSE_AND_RETRY_STATE;
				}  else {
					//send data success ...keep sending from cycle buffer
					uplink_dststate = UPLINK_DST_SEND_DATA_STATE;
				}
			} else {
				//buffer not enough go to mute state
				uplink_dststate = UPLINK_DST_SEND_DATA_MUTE_STATE;
			}
		}
		break;

		case UPLINK_DST_SEND_DATA_MUTE_STATE:
		{
			err_write = pcm_write(pcm_playhandleup, zero_bufferup, sizebytes);
			if (err_write != 0) {
				INFO_UAC_PRINT("Error pcm_write Mute failure in uplink dst...go retry\n");
				sleep(2);
				uplink_dststate = UPLINK_DST_CLOSE_AND_RETRY_STATE;
			}  else {
				ret_b = CycleBufferDataLen(gpstUplinkCB);
				if (ret_b < sizebytes * 2) {
					//cycle buffer underrun
					uplink_dststate = UPLINK_DST_SEND_DATA_MUTE_STATE;
					usleep(10*1000);
				} else {
					//cycle buffer enough data
					//go to send data
					uplink_dststate = UPLINK_DST_SEND_DATA_STATE;
				}
			}
		}
		break;

		case UPLINK_DST_CLOSE_AND_RETRY_STATE:
		{
			#ifdef THIS_IS_32
			#else
			free(buffer_up_dst);
			#endif
			if (err_write != 0) {
				DBG_UAC_PRINT("pcm_close pcm_playhandleup\n");
				pcm_close(pcm_playhandleup);
				pcm_playhandleup = NULL;
			}
			#ifdef THIS_IS_32
			#else
			buffer_up_dst = (char *)malloc(AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT);
			#endif
			usleep(500*1000);
			sleep(1);
			DBG_UAC_PRINT("UPLINK_DST_CLOSE_AND_RETRY_STATE\n");
			uplink_dststate = UPLINK_DST_OPEN_STATE;
		}
		break;
		default:
			ERR_UAC_PRINT("[Error]Not a valid state in uplink dst(to usb)[%d]\n", uplink_dststate);
			sleep(2);
			break;
		}
	}

	return (void *)0;
}
#else

#endif
struct pcm_config stPcmCapConfig = {
#if CV18XX_AS_HOST
	.channels = 1,//CHANNEL_COUNT,
#else
	.channels = CHANNEL_COUNT,
#endif
	.rate = 48000,
	.period_size = AUDIO_PERIOD_SIZE,
	.period_count = CAP_PERIOD_COUNT,
	.format = PCM_FORMAT_S16_LE,
	.start_threshold = 0,
	.stop_threshold = INT_MAX,
};


struct pcm_config stPcmPlayConfig = {
#if CV18XX_AS_HOST
	.channels = 1,//CHANNEL_COUNT,
#else
	.channels = CHANNEL_COUNT,
#endif
	.rate = 48000,
	.period_size = AUDIO_PERIOD_SIZE,
	.period_count = PLAY_PERIOD_COUNT,
	.format = PCM_FORMAT_S16_LE,
	.start_threshold = 0,
	.stop_threshold = INT_MAX,
};

#if DOWNLINK_USE_CYCLE_BUFFER
static void *thread_downlink_src_audio(void *arg)
{
	//pcm stttus
	int err;
	int err_read = -1; // -1 : not  success, 0 open & read success
	int sizebytes = 0;
	//cycle buffer status
	int ret_b;
	int ret_b_count = 0;
	//handle
	struct pcm *pcm_recordhandle = NULL;
	char zero_downbuffer[AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT*2];
	int downlink_src_StableCnt = 0;
#define  USB_AS_SRC_STABLE_THRESHOLD 5

	memset(zero_downbuffer, 0, (AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT));


	int downlink_srcstate = DOWNLINK_SRC_OPEN_STATE;

	while (bDownlinkEnable) {
		switch (downlink_srcstate) {
		case DOWNLINK_SRC_OPEN_STATE:
		{
			if (err_read != 0) {
				pcm_recordhandle = pcm_open(2, 0, PCM_IN, &stPcmCapConfig);
				if (!pcm_recordhandle || !pcm_is_ready(pcm_recordhandle)) {
					INFO_UAC_PRINT("Unable to open pcm_recordhandle (%s)\n",
						pcm_get_error(pcm_recordhandle));
					ERR_UAC_PRINT("\n");
					usleep(500*1000);

				} else {
					err_read = 0;
					DBG_UAC_PRINT("pcm_down_src recordhandle success\n");
				}
			}

			downlink_srcstate = DOWNLINK_SRC_DETECT_STATE;
		}
		break;

		case DOWNLINK_SRC_DETECT_STATE:
		{
			sizebytes = AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT;
			err_read = pcm_read(pcm_recordhandle, buffer_dl_src, sizebytes);
			if (err_read == 0) {
				downlink_src_StableCnt++;
				if (downlink_src_StableCnt > USB_AS_SRC_STABLE_THRESHOLD) {
					DBG_UAC_PRINT("USB as downlink source stable\n");
					downlink_srcstate = DOWNLINK_SRC_SEND_DATA_STATE;
					downlink_src_StableCnt = 0;
				} else {
					downlink_srcstate = DOWNLINK_SRC_DETECT_STATE;
				}
			} else {
				downlink_src_StableCnt = 0;
				downlink_srcstate = DOWNLINK_SRC_CLOSE_AND_RETRY_STATE;
				 sleep(1);
				 usleep(500*1000);
				 DBG_UAC_PRINT("DOWNLINK_SRC_DETECT_STATE failure ...go to retry\n");
			}

		}
		break;

		case DOWNLINK_SRC_SEND_DATA_STATE:
		{
			sizebytes = AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT;
			err_read = pcm_read(pcm_recordhandle, buffer_dl_src, sizebytes);
			if (err_read != 0) {
				ERR_UAC_PRINT("card 2 not get data from PC\n");
				ERR_UAC_PRINT("card 2[%s]\n", pcm_get_error(pcm_recordhandle));
				INFO_UAC_PRINT("[downlink_audio src]go close and retry....\n");
				sleep(1);
				usleep(500*1000);
				downlink_srcstate = DOWNLINK_SRC_CLOSE_AND_RETRY_STATE;
			} else {
				downlink_srcstate = DOWNLINK_SRC_SEND_DATA_STATE;
				ret_b =  CycleBufferWrite(gpstDownlinkCB, buffer_dl_src, sizebytes);
				if (!ret_b) {
					//buffer abnormal
					ret_b_count++;
					usleep(1000*sizebytes/(64));
					if (ret_b_count >= 10) {
					ret_b = CycleBufferDataLen(gpstDownlinkCB);
					DBG_UAC_PRINT("[Error]downlink cycle buffer abnormal bufferlevel[%d] of [%d]\n",
							ret_b, DOWNLINK_CYCLE_BUFFER_SIZE);
					}
				} else {
					//buffer ok
					ret_b_count = 0;
				}
			}
		}
		break;

		case DOWNLINK_SRC_CLOSE_AND_RETRY_STATE:
		{
			free(buffer_dl_src);
			if (err_read != 0) {
				DBG_UAC_PRINT("pcm_close pcm_recordhandle\n");
				pcm_close(pcm_recordhandle);
				pcm_recordhandle = NULL;
			}
			buffer_dl_src = (char *)malloc(AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT);
			DBG_UAC_PRINT("DOWNLINK_SRC_CLOSE_AND_RETRY_STATE\n");
			sleep(1);
			downlink_srcstate = DOWNLINK_SRC_OPEN_STATE;
		}
		break;

		default:
			ERR_UAC_PRINT("[Error]downlink usb as src abnormal...[%d]\n", downlink_srcstate);
			sleep(2);
			break;

		}
	}
	return (void *)0;
}
static void *thread_downlink_dst_audio(void *arg)
{

	int err_write = -1;
	int ret_b = 0;
	struct pcm *pcm_playhandle = NULL;
	char zero_downbuffer[AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT*2];
	int sizebytes = AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT;
#define  EDGE_AS_SPK_STABLE_THRESHOLD 1

	memset(zero_downbuffer, 0, (AUDIO_PERIOD_SIZE*2*CHANNEL_COUNT*2));
	int detect_spk_count = 0;
	//downlink dst is output microphone
	int downlink_dststate = DOWNLINK_DST_OPEN_STATE;

	while (bDownlinkEnable) {
		switch (downlink_dststate) {
		case DOWNLINK_DST_OPEN_STATE:
		{
			if (err_write != 0) {
				pcm_playhandle = pcm_open(1, 0, PCM_OUT, &stPcmPlayConfig);
				if (!pcm_playhandle || !pcm_is_ready(pcm_playhandle)) {
					ERR_UAC_PRINT("Unable to open pcm_playhandle (%s)\n",
						pcm_get_error(pcm_playhandle));
					ERR_UAC_PRINT("\n");
					usleep(500*1000);
				} else {
					err_write = 0;
					DBG_UAC_PRINT("pcm_down_playhandle success\n");
				}
			} else {
				err_write = 0;
				DBG_UAC_PRINT("pcm_playhandle success\n");
				usleep(10*1000);
			}
			downlink_dststate = DOWNLINK_DST_DETECT_STATE;
		}
		break;

		case DOWNLINK_DST_DETECT_STATE:
		{
			err_write = pcm_write(pcm_playhandle, zero_downbuffer, sizebytes);
			if (err_write != 0) {
				ERR_UAC_PRINT("card 1[%d][%s]\n", __LINE__, pcm_get_error(pcm_playhandle));
				detect_spk_count = 0;
				sleep(1);
				downlink_dststate = DOWNLINK_DST_CLOSE_AND_RETRY_STATE;
			} else {
				detect_spk_count++;
				if (detect_spk_count > EDGE_AS_SPK_STABLE_THRESHOLD) {
					DBG_UAC_PRINT("Edge as downlink dst stable\n");
					downlink_dststate = DOWNLINK_DST_SEND_DATA_STATE;
					detect_spk_count = 0;
				} else {
					downlink_dststate = DOWNLINK_DST_DETECT_STATE;
				}
			}
		}
		break;

		case DOWNLINK_DST_SEND_DATA_STATE:
		{
			ret_b = CycleBufferDataLen(gpstDownlinkCB);
			if (ret_b >= sizebytes * 2) {
				//sufficient with buffering 2 frame in cycble buffer
				//send the true data
				ret_b = CycleBufferRead(gpstDownlinkCB, buffer_dl_dst, sizebytes);
				if (ret_b != sizebytes) {
					//...cycle buffer abnormal
					DBG_UAC_PRINT("Error not enough in cycle buffer ...abnormal[%d]\n", __LINE__);
				}
				err_write = pcm_write(pcm_playhandle, buffer_dl_dst, sizebytes);
				if (err_write != 0) {
					ERR_UAC_PRINT("card 1[%d][%s]\n", __LINE__, pcm_get_error(pcm_playhandle));
					ERR_UAC_PRINT("Error pcm_write failure in downlink dst(spk)...go retry\n");
					sleep(2);
					downlink_dststate = DOWNLINK_DST_CLOSE_AND_RETRY_STATE;
				}  else {
					//send data success ...keep sending from cycle buffer
					downlink_dststate = DOWNLINK_DST_SEND_DATA_STATE;
				}
			} else {
				//buffer not enough go to mute state
				downlink_dststate = DOWNLINK_DST_SEND_DATA_MUTE_STATE;
			}
		}
		break;

		case DOWNLINK_DST_SEND_DATA_MUTE_STATE:
		{
			err_write = pcm_write(pcm_playhandle, zero_downbuffer, sizebytes);
			if (err_write != 0) {
				ERR_UAC_PRINT("Error pcm_write Mute failure in downlink dst(spk)...go retry\n");
				sleep(2);
				downlink_dststate = DOWNLINK_DST_CLOSE_AND_RETRY_STATE;
			}  else {
				ret_b = CycleBufferDataLen(gpstDownlinkCB);
				if (ret_b < sizebytes * 2) {
					//cycle buffer underrun
					downlink_dststate = DOWNLINK_DST_SEND_DATA_MUTE_STATE;
				} else {
					//cycle buffer enough data
					//go to send data
					downlink_dststate = DOWNLINK_DST_SEND_DATA_STATE;
				}
			}
		}
		break;

		case DOWNLINK_DST_CLOSE_AND_RETRY_STATE:
		{
			free(buffer_dl_dst);
			if (err_write != 0) {
				DBG_UAC_PRINT("pcm_close pcm_playhandle\n");
				pcm_close(pcm_playhandle);
				pcm_playhandle = NULL;
			}
			buffer_dl_dst = (char *)malloc(AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT);
			usleep(500*1000);
			DBG_UAC_PRINT("DOWNLINK_DST_CLOSE_AND_RETRY_STATE\n");
			downlink_dststate = DOWNLINK_DST_OPEN_STATE;
		}
		break;

		default:
		ERR_UAC_PRINT("[Error]Not a valid state in downlink dst(speaker)[%d]\n", downlink_dststate);
		sleep(2);
		break;
		}
	}

}
#else

#endif

int cvi_audio_uac_uplink(void)
{
	bUplinkEnable = 1;
#if UPLINK_USE_CYCLE_BUFFER
	struct sched_param param;
	pthread_attr_t attr;

	param.sched_priority = 80;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(pcm_uplink_src_mic_thread, &attr, thread_uplink_src_audio, NULL);


	struct sched_param param2;
	pthread_attr_t attr2;

	param2.sched_priority = 80;
	pthread_attr_init(&attr2);
	pthread_attr_setschedpolicy(&attr2, SCHED_RR);
	pthread_attr_setschedparam(&attr2, &param);
	pthread_attr_setinheritsched(&attr2, PTHREAD_EXPLICIT_SCHED);
	pthread_create(pcm_uplink_dst_usb_thread, &attr2, thread_uplink_dst_audio, NULL);
#else
	struct sched_param param;
	pthread_attr_t attr;

	param.sched_priority = 80;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(pcm_mic_to_pc_thread, &attr, thread_uplink_audio, NULL);
	//pthread_detach(pcm_mic_to_pc_thread);
#endif
	return 1;
}

int cvi_audio_uac_uplink_stop(void)
{
	bUplinkEnable = 0;
	return 1;
}

int cvi_audio_uac_downlink(void)
{
	bDownlinkEnable = 1;
#if DOWNLINK_USE_CYCLE_BUFFER
	struct sched_param param;
	pthread_attr_t attr;

	param.sched_priority = 80;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(pcm_downlink_src_usb_thread, NULL, thread_downlink_src_audio, NULL);


	struct sched_param param2;
	pthread_attr_t attr2;

	param2.sched_priority = 80;
	pthread_attr_init(&attr2);
	pthread_attr_setschedpolicy(&attr2, SCHED_RR);
	pthread_attr_setschedparam(&attr2, &param2);
	pthread_attr_setinheritsched(&attr2, PTHREAD_EXPLICIT_SCHED);
	pthread_create(pcm_downlink_dst_spk_thread, NULL, thread_downlink_dst_audio, NULL);

#else
	struct sched_param param;
	pthread_attr_t attr;

	param.sched_priority = 80;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(pcm_pc_to_mic_thread, NULL, thread_downlink_audio, NULL);
	//pthread_detach(pcm_pc_to_mic_thread);
#endif
	return 1;
}

int cvi_audio_uac_downlink_stop(void)
{
	bDownlinkEnable = 0;
	return 1;
}

int  cvi_audio_init_uac(void)
{
	printf("init...easy uac audio api[%s]\n", __VERSION_TAG__);
	cvi_audio_uac_getDbgMask();
	bUplinkEnable = 0;
	bDownlinkEnable = 0;

#if DOWNLINK_USE_CYCLE_BUFFER

	CycleBufferInit(&gpstDownlinkCB, DOWNLINK_CYCLE_BUFFER_SIZE);
	pDownlinkCBuffer = (char *)malloc(DOWNLINK_CYCLE_BUFFER_SIZE);
	buffer_dl_src =  (char *)malloc(AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT);
	buffer_dl_dst =  (char *)malloc(AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT);
	pcm_downlink_src_usb_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pcm_downlink_dst_spk_thread = (pthread_t *)malloc(sizeof(pthread_t));

#else
	buffer = (char *)malloc(AUDIO_PERIOD_SIZE*2*CHANNEL_COUNT*2);
	pcm_pc_to_mic_thread = (pthread_t *)malloc(sizeof(pthread_t));
#endif


#if UPLINK_USE_CYCLE_BUFFER
	pcm_uplink_src_mic_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pcm_uplink_dst_usb_thread = (pthread_t *)malloc(sizeof(pthread_t));
	CycleBufferInit(&gpstUplinkCB, UPLINK_CYCLE_BUFFER_SIZE);
	buffer_up_src =  (char *)malloc(AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT);
	buffer_up_dst =  (char *)malloc(AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT);
	pflush_up_buffer = (char *)malloc(UPLINK_CYCLE_BUFFER_SIZE);
#else
	pcm_mic_to_pc_thread = (pthread_t *)malloc(sizeof(pthread_t));
	bufferup = (char *)malloc(960 * 2 * 4);
	bufferup2 = (char *)malloc(960 * 2 * 4);
#endif

	return 1;
}

int cvi_audio_deinit_uac(void)
{
	int s32Ret;

	printf("deinit audio ...easy audio api\n");
	bUplinkEnable = 0;
	bDownlinkEnable = 0;
#if UPLINK_USE_CYCLE_BUFFER
	if (*pcm_uplink_src_mic_thread != 0) {
		s32Ret = pthread_join(*pcm_uplink_src_mic_thread, NULL);
		if (s32Ret == 0)
			*pcm_uplink_src_mic_thread = 0;

	}

	if (*pcm_uplink_dst_usb_thread != 0) {
		s32Ret = pthread_join(*pcm_uplink_dst_usb_thread, NULL);
		if (s32Ret == 0)
			*pcm_uplink_dst_usb_thread = 0;
	}
	//destroy cycle  buffer
	CycleBufferDestory(gpstUplinkCB);
	gpstUplinkCB = NULL;
	if (pUplinkCBuffer != NULL)
		free(pUplinkCBuffer);
	//free buffer
	if (buffer_up_src != 0)
		free(buffer_up_src);

	if (buffer_up_dst != 0)
		free(buffer_up_dst);
#else
	if (*pcm_mic_to_pc_thread != 0) {
		s32Ret = pthread_join(*pcm_mic_to_pc_thread, NULL);
		if (s32Ret == 0)
			*pcm_mic_to_pc_thread = 0;

	}

	if (bufferup != 0)
		free(bufferup);
	if (bufferup2 != 0)
		free(bufferup2);
#endif

#if DOWNLINK_USE_CYCLE_BUFFER
	//clear downlink src / dst thread
	if (*pcm_downlink_src_usb_thread != 0) {
		s32Ret = pthread_join(*pcm_downlink_src_usb_thread, NULL);
		if (s32Ret == 0)
			*pcm_downlink_src_usb_thread = 0;

	}
	if (*pcm_downlink_dst_spk_thread != 0) {
		s32Ret = pthread_join(*pcm_downlink_dst_spk_thread, NULL);
		if (s32Ret == 0)
			*pcm_downlink_dst_spk_thread = 0;

	}
	//destroy cycle  buffer
	CycleBufferDestory(gpstDownlinkCB);
	gpstDownlinkCB = NULL;
	if (pDownlinkCBuffer != NULL)
		free(pDownlinkCBuffer);
	//free buffer
	if (buffer_dl_src != 0)
		free(buffer_dl_src);

	if (buffer_dl_dst != 0)
		free(buffer_dl_dst);
#else
	if (*pcm_pc_to_mic_thread != 0) {
		s32Ret = pthread_join(*pcm_pc_to_mic_thread, NULL);
		if (s32Ret == 0)
			*pcm_pc_to_mic_thread = 0;

	}
	if (buffer != 0)
		free(buffer);
#endif

}


static int  cviUacGetEnv(char *env, char *fmt, void *param)
{
#define NOT_SET -2
#define INPUT_ERR -1
	char *pEnv;
	int val = NOT_SET;

	pEnv = getenv(env);
	if (pEnv) {
		if (strcmp(fmt, "%s") == 0) {
			strcpy(param, pEnv);
		} else {
			if (sscanf(pEnv, fmt, &val) != 1)
				return INPUT_ERR;
			printf("[UAC]%s = %d\n", env, val);
		}
	} else
		printf("[Err][%s][%d]getenv failure\n", __func__, __LINE__);

	return val;
}
static void cvi_audio_uac_getDbgMask(void)
{
#define NOT_SET -2
#define INPUT_ERR -1
	int  *pUAC_level = &gUAC_level;
	int getValue;

	*pUAC_level = 0;

	getValue = cviUacGetEnv("uac_level", "%d", NULL);
	if (getValue == NOT_SET || getValue == INPUT_ERR) {
		printf("uac level not set or input err..force level=1\n");
		*pUAC_level  = CVI_UAC_MASK_ERR;
	} else
		*pUAC_level = getValue;

	printf("[UAC]uac_level = [%d]\n", (*pUAC_level));
}

#ifndef ENABLE_UVC_AUDIO
int main(void)
{
	cvi_audio_init_uac();

	printf("start uplink after....\n");
	cvi_audio_uac_uplink();

	printf("start downlink....\n");
	cvi_audio_uac_downlink();

	while (1) {
		//this is trying to keep thread alive
		sleep(10);
	}

}
#endif


