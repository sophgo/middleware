#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <linux/netlink.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <cvi_audio.h>
#include <cvi_type.h>
#include <sys/prctl.h>
#include <asoundlib.h>
#include "cvi_ao_internal.h"
#include "alog.h"
#include "cvi_audio_common.h"
#include "cvi_aud_threads.h"
#include "cvi_ai_internal.h"
#include "volume_ctrl.h"
#define SPEAKER_CARD_STRING "dac"
#define MIC_CARD_STRING "adc"

AI_CLI_THREAD_STATUS gstAiThreadStatus;
AO_CLI_THREAD_STATUS gstAoThreadStatus;
static const uint64_t u64DefaultStandDelayUs = 3 * 1000 * 1000;

uint64_t get_systemtime(void)
{
	struct timespec timestamp;

	clock_gettime(CLOCK_MONOTONIC, &timestamp);
	return timestamp.tv_sec * 1000000 + timestamp.tv_nsec / 1000;
}

void cvitek_dump_audiodata(char *filename, char *buf, unsigned int len)
{
	FILE *fp;

	if (filename == NULL) {
		return;
	}

	fp = fopen(filename, "ab+");
	fwrite(buf, 1, len, fp);
	fclose(fp);
}


static int getSndCardId(char *str)
{
	int card = -1;
	char buffer[640];
	FILE *fp = fopen("/proc/asound/cards", "r");

	while (fp && fgets(buffer, 640, fp)) {
		if (strstr(buffer, str)) {
			card = atoi(buffer);
			break;
		}
		memset(buffer, 0, 640);
	}
	if (fp)
		fclose(fp);
	return card;
}

static void audio_mono_2_stereo(short *sInput, short *sOutput, int len)
{
	for (int i = 0; i < len; i++) {
		sOutput[2 * i] =  sInput[i];
		sOutput[2 * i + 1] = sInput[i];
	}
}

static void audio_stereo_2_mono(short *sInput, short *sOutput, int len)
{
	for (int i = 0; i < len; i++) {
		sOutput[i] = sInput[2 * i];
	}
}

static void ai_cli_thread_status(int mode, int Aichn, int u32ChnCnt, CLI_INPUT_STATUS_E estaistatus)
{
	if (mode == 0) {
		UNUSED_REF(u32ChnCnt);
		gstAiThreadStatus.audThreadStatus[Aichn] = estaistatus;
	} else {
		UNUSED_REF(Aichn);
		for (int i = 0; i < u32ChnCnt; i++)
			gstAiThreadStatus.audThreadStatus[i] = estaistatus;
	}

}

static void ao_cli_thread_status(int mode, int Aochn, int u32ChnCnt, CLI_OUT_STATUS_E estaistatus)
{
	if (mode == 0) {
		UNUSED_REF(u32ChnCnt);
		gstAoThreadStatus.aoThreadStatus[Aochn] = estaistatus;
	} else {
		UNUSED_REF(Aochn);
		for (int i = 0; i < u32ChnCnt; i++)
			gstAoThreadStatus.aoThreadStatus[i] = estaistatus;
	}
}

static void updateOutputThreadTracks(ST_AO_INSTANCE *pstAoInstance, int OutputChnCnt)
{
	CVI_ST_AUD_TRACK_INFO **TrackVector;

	if (!pstAoInstance) {
		return;
	}
	TrackVector = pstAoInstance->pastTrackInfo;

	for (int i = 0; i < OutputChnCnt; i++) {
		if (TrackVector[i] && TrackVector[i]->state == STATE_CLOSE) {
			TrackVector[i]->refFlag = false;
		}
	}
}

static int get_audio_pcm_config_params(struct pcm_config *pstPcmCfg, AIO_ATTR_S *pstAioAttrs, AUDIO_INOUT_E Inoutflag)
{
	if (CVIAUDIO_CHECK_NULL((void *)pstPcmCfg) ||
		CVIAUDIO_CHECK_NULL((void *)pstAioAttrs))
		return -1;
	if (pstAioAttrs->enBitwidth != AUDIO_BIT_WIDTH_16) {
		log_warn("only support 16 bitdeepth,%d\n", pstAioAttrs->enBitwidth);
	}
	pstPcmCfg->format = PCM_FORMAT_S16_LE;
	pstPcmCfg->channels = (pstAioAttrs->enSoundmode == AUDIO_SOUND_MODE_STEREO) ? 2 : 1;
	pstPcmCfg->rate = pstAioAttrs->enSamplerate;

	pstPcmCfg->period_count = pstAioAttrs->u32FrmNum >= 4 ? 3 : pstAioAttrs->u32FrmNum;
	pstPcmCfg->start_threshold = 0;
	pstPcmCfg->stop_threshold = 2147483647;
	pstPcmCfg->silence_threshold = 0;
	if (Inoutflag == AUDIO_IN) {
		pstPcmCfg->period_size = (pstPcmCfg->rate / 1000) * PERIOD_MS;
		if (pstPcmCfg->rate != AUDIO_SAMPLE_RATE_8000 &&
				pstPcmCfg->rate != AUDIO_SAMPLE_RATE_16000 &&
				pstPcmCfg->rate != AUDIO_SAMPLE_RATE_32000) {
			log_warn("please use resample 8k/16k to dest_sampleRate\n");
			pstPcmCfg->period_size = 320;
		}
	} else
		pstPcmCfg->period_size = pstAioAttrs->u32PtNumPerFrm;

	log_info("period_size:%d period_cnt:%d channels:%d rate:%d\n",
		 pstPcmCfg->period_size, pstPcmCfg->period_count, pstPcmCfg->channels, pstPcmCfg->rate);

	return 0;
}


static int _parsing_ain_dev_vqe_status(AUDIO_DEV AiDevId, ST_THREAD_VQE_INFO *pstThreadVqeInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	pstThreadVqeInfo->bDevEnableVqe = (bool)CVI_AI_DevVQECheckEnable(AiDevId);

	if (pstThreadVqeInfo->bDevEnableVqe == CVI_FALSE) {
		s32Ret = CVI_FAILURE;
	} else {
		CVI_S32 s32VqeMask = 0x0;

		s32VqeMask = CVI_AI_VQECheckFlag(AiDevId, 0);
		if ((s32VqeMask & AI_TALKVQE_MASK_AEC) == AI_TALKVQE_MASK_AEC)
			pstThreadVqeInfo->bDevVqeWithAecOn = CVI_TRUE;
		else if (((s32VqeMask & AI_TALKVQE_MASK_AGC) == AI_TALKVQE_MASK_AGC) ||
			 ((s32VqeMask & AI_TALKVQE_MASK_ANR) == AI_TALKVQE_MASK_ANR))
			pstThreadVqeInfo->bDevVqeWithAecOn = CVI_FALSE;
		else {
			log_error("vqe on but mask unidentified dev[%d] mask[0x%x]\n",
				  AiDevId, s32VqeMask);
			pstThreadVqeInfo->bDevEnableVqe = CVI_FALSE;
			s32Ret = CVI_FAILURE;
		}
	}

	return s32Ret;
}

CVI_VOID *AudioPrimaryOutputThread(CVI_VOID *arg)
{
	int ret = 0;
	char szThreadName[50] = "ao_playback";
	ST_AO_INSTANCE *pstAoInstance = (ST_AO_INSTANCE *)arg;
	CVI_ST_THREAD_INFO *pstThreadInfo;
	struct pcm_config *pstPcmCfg;
	CVI_ST_AUD_TRACK_INFO *pstTrackInfo = NULL;
	int bHasAudioData = CVI_FALSE;
	int i32NumDelayedWrites = 0;
	uint64_t lastWriteFinished = 0;
	uint64_t lastWriteTime = 0;
	uint64_t delta = 0;
	uint64_t u64StandbyTimeUs = get_systemtime();
	int outRespFrame = 0;
	int write_byte = 0;
	int dnvqeOutbyte = 0;

	if (!pstAoInstance) {
		log_error("invalid parmas.\n");
		return NULL;
	}
	CVI_ST_AUD_TRACK_INFO **TrackVector = pstAoInstance->pastTrackInfo;

	pstThreadInfo = &pstAoInstance->stThreadInfo;
	pstPcmCfg = &pstThreadInfo->stPcmCfg;
	snprintf(szThreadName, 50, "ao_playback_%d", pstAoInstance->s32DevId);
	prctl(PR_SET_NAME, szThreadName, 0, 0, 0);

	ret = get_audio_pcm_config_params(pstPcmCfg, &pstAoInstance->ao_attrs, AUDIO_OUT);
	if (ret < 0) {
		log_error("get output pcm config params failed.\n");
		return NULL;
	}

	if (pstAoInstance->ao_attrs.enSamplerate > AUDIO_SAMPLE_RATE_48000) {
		log_error("not support yet.\n");
		return NULL;
	}

	//int period_cnt = pstAoInstance->ao_attrs.u32FrmNum;
	int period_cnt = pstPcmCfg->period_count;
	int period_ms = pstAoInstance->ao_attrs.u32PtNumPerFrm * 1000 /
			pstAoInstance->ao_attrs.enSamplerate;
	int period_frame_len = pstAoInstance->ao_attrs.u32PtNumPerFrm;
	//const int period_bytes = period_frame_len * (pstAoInstance->ao_attrs.enSoundmode + 1) *
	//						DEFAULT_BYTES_PER_SAMPLE;
	int max_size = AUDIO_SAMPLE_RATE_48000 / 1000 * 2 * DEFAULT_BYTES_PER_SAMPLE * period_ms;
	int pDestFrame = period_frame_len;
	short *pTmpBuffer = (short *)malloc(max_size);
	short *pReadBuffer = (short *)malloc(max_size);
	short *pOutBuffer = (short *)malloc(max_size);
	short *pAlsaBuffer = (short *)malloc(max_size);
	short *pDest = NULL;

	//pReadBuffer --> pTmpBuffer --> pOutBuffer --> pAlsaBuffer

	log_info("period_cnt %d, period_ms %d,period_frame_len %d.\n",
		 period_cnt, period_ms, period_frame_len);
	if (!pAlsaBuffer || !pTmpBuffer || !pReadBuffer || !pOutBuffer) {
		log_error("malloc failed.\n");
		return NULL;
	}

	pstThreadInfo->card = getSndCardId(SPEAKER_CARD_STRING);
	if (pstThreadInfo->card < 0) {
		log_warn("card warn\n");
		pstThreadInfo->card = 1;
	}
	pstThreadInfo->pcmHandle = pcm_open(pstThreadInfo->card, 1, PCM_OUT, pstPcmCfg);
	if (!pstThreadInfo->pcmHandle) {
		log_error("open output pcm device failed(%s).\n", pcm_get_error(pstThreadInfo->pcmHandle));
		return NULL;
	}
	pstThreadInfo->bStandby = CVI_TRUE;
	log_info("ao thread start here.\n");

	while (!pstThreadInfo->i32ExitPending) {

		ao_cli_thread_status(1, 0, pstAoInstance->ao_attrs.u32ChnCnt, AUD_READLY);
		memset(pAlsaBuffer, 0, max_size);
		if (pstThreadInfo->bStandby) {
			for (CVI_U32 i = 0; i < pstAoInstance->ao_attrs.u32ChnCnt; i++) {
				if (TrackVector[i]) {
					pstTrackInfo = TrackVector[i];
					if (pstTrackInfo->state == STATE_READY) {
						pstTrackInfo->refFlag = true;
					}
					int iFrameBytes = DEFAULT_BYTES_PER_SAMPLE * pstTrackInfo->iChannels *
								period_ms * pstTrackInfo->iSampleRate/1000;

					if ((share_cyclebuffer_frameready_size(pstTrackInfo->iShmMemIndex) >=
							(iFrameBytes * period_cnt)) ||
							(pstTrackInfo->state == STATE_STOP &&
							pstTrackInfo->lastState == STATE_READY))
						pstThreadInfo->bStandby = false;
				}
			}
		}

		//read data from share buffer
	if (!pstThreadInfo->bStandby) {
		for (unsigned int i = 0; i < pstAoInstance->ao_attrs.u32ChnCnt; i++) {
			if (TrackVector[i] && (TrackVector[i]->state == STATE_READY
							|| TrackVector[i]->state == STATE_RUNNING
							|| (TrackVector[i]->state == STATE_STOP &&
							TrackVector[i]->lastState == STATE_READY))) {

				ao_cli_thread_status(0, i, 0, AUD_SHARE_READLY);
				pstTrackInfo = TrackVector[i];
				int iFrameLen = period_ms * pstTrackInfo->iSampleRate / 1000;
				int iReadLen = share_cyclebuffer_server_read(pstTrackInfo->iShmMemIndex,
						(char *)pReadBuffer, iFrameLen * pstTrackInfo->iChannels *
						DEFAULT_BYTES_PER_SAMPLE);

				if (iReadLen > 0) {
					if (pstTrackInfo->state == STATE_READY) {
						pstTrackInfo->lastState = pstTrackInfo->state;//STATE_READY
						pstTrackInfo->state = STATE_RUNNING;
						pstTrackInfo->refFlag = true;
					}

					ao_cli_thread_status(0, i, 0, AUD_RUNINNG);
					if (pstTrackInfo->b_dump_enable) {
						char dump_name[64] = "play_";
						snprintf(dump_name + strlen(dump_name), 64, "%d.pcm",
									pstTrackInfo->iShmMemIndex);
						cvitek_dump_audiodata(dump_name, (char *)pReadBuffer,
							iFrameLen * pstTrackInfo->iChannels
							* DEFAULT_BYTES_PER_SAMPLE);
					}
					// do resample if need
					if (pstTrackInfo->pResHandle) {
						_check_and_dump("/tmp/dump_ao_Resin", (char *)pReadBuffer,
							iFrameLen * pstTrackInfo->iChannels * DEFAULT_BYTES_PER_SAMPLE);

						outRespFrame = CVI_Resampler_Process(pstTrackInfo->pResHandle,
									pReadBuffer, iFrameLen, pTmpBuffer);
					if (outRespFrame == 0) {
						outRespFrame = period_frame_len;
					}
						pDestFrame = outRespFrame;
						_check_and_dump("/tmp/dump_ao_Reskout", (char *)pTmpBuffer,
							pDestFrame * pstTrackInfo->iChannels
							* DEFAULT_BYTES_PER_SAMPLE);

					} else {
						memcpy(pTmpBuffer, pReadBuffer,
								iFrameLen * pstTrackInfo->iChannels *
								DEFAULT_BYTES_PER_SAMPLE);
						pDestFrame = iFrameLen;
					}


					if (CVI_AO_VQECheckEnable(pstAoInstance->s32DevId, pstTrackInfo->chnid)) {
						_check_and_dump("/tmp/dump_dnvqe_befor", (char *)pTmpBuffer,
							pDestFrame * pstTrackInfo->iChannels
							* DEFAULT_BYTES_PER_SAMPLE);
						ret = CVI_AudOut_AlgoProcess((CVI_CHAR *)pTmpBuffer, (CVI_CHAR *)pTmpBuffer,
							pDestFrame * pstTrackInfo->iChannels * DEFAULT_BYTES_PER_SAMPLE, &dnvqeOutbyte);

						if (ret) {
							log_error("dnvqeProcess error frame:%d, channels:%d, dnvqeOutbyte:%d\n",
								pDestFrame, pstTrackInfo->iChannels, dnvqeOutbyte);
						}
						_check_and_dump("/tmp/dump_dnvqe_after", (char *)pTmpBuffer,
							pDestFrame * pstTrackInfo->iChannels
							* DEFAULT_BYTES_PER_SAMPLE);
					}

					// one channel to stereo channels
					if (pstTrackInfo->iChannels ==
							(int)(pstAoInstance->ao_attrs.enSoundmode + 1)) {

						pDest = pTmpBuffer;
					} else if (pstTrackInfo->iChannels == 1
							&& (int)(pstAoInstance->ao_attrs.enSoundmode + 1) == 2) {
						audio_mono_2_stereo(pTmpBuffer, pOutBuffer,
										pDestFrame);
						pDest = pOutBuffer;
					} else if (pstTrackInfo->iChannels == 2
							&& (int)(pstAoInstance->ao_attrs.enSoundmode + 1) == 1) {
						audio_stereo_2_mono(pTmpBuffer, pOutBuffer,
										pDestFrame);
						pDest = pOutBuffer;
					}

					//mix here
					for (int j = 0; j < (pDestFrame * pstTrackInfo->iChannels); j++) {
						pAlsaBuffer[j] += pDest[j];
					}
					bHasAudioData = true;
					u64StandbyTimeUs = get_systemtime() + u64DefaultStandDelayUs;
				} else if (pstTrackInfo->state == STATE_STOP) {
					pstTrackInfo->lastState = pstTrackInfo->state;
					pstTrackInfo->state = STATE_CLOSE;
				} else if (pstTrackInfo->state == STATE_RUNNING) {
					log_debug("track:%p underrun,framesReady(%d) < framesDesired(%d)\n",
						pstTrackInfo,
						share_cyclebuffer_frameready_size(pstTrackInfo->iShmMemIndex)/
							(pstTrackInfo->iChannels * DEFAULT_BYTES_PER_SAMPLE),
							pDestFrame * pstTrackInfo->iChannels
							* DEFAULT_BYTES_PER_SAMPLE);
				} else if (iReadLen < 0) {
					log_error("track:%p iReadLen:%d,shmIdx:%d,channels:%d,sr:%d\n",
							pstTrackInfo, iReadLen, pstTrackInfo->iShmMemIndex,
							pstTrackInfo->iChannels, pstTrackInfo->iSampleRate);
					goto AOUT_THREAD_EXIT;
				}
			}
		}
	}
		//update tracks in vectors
		updateOutputThreadTracks(pstAoInstance, pstAoInstance->ao_attrs.u32ChnCnt);
		if (!pstThreadInfo->bStandby && !bHasAudioData && get_systemtime() > u64StandbyTimeUs) {
			pstThreadInfo->bStandby = true;
			continue;
		}

		write_byte = pDestFrame * (pstAoInstance->ao_attrs.enSoundmode + 1) *
					DEFAULT_BYTES_PER_SAMPLE;
		if (pstThreadInfo->b_enable_dump  || access("/tmp/dump_ao_output", F_OK) == 0) {
			cvitek_dump_audiodata("dump_ao_output.pcm", (char *)pAlsaBuffer, write_byte);
		}

		ao_cli_thread_status(1, 0, pstAoInstance->ao_attrs.u32ChnCnt, AUD_WRITE_READLY);
		//write to alsa
		lastWriteTime = get_systemtime();
		ret = pcm_write(pstThreadInfo->pcmHandle, (char *)pAlsaBuffer, write_byte);
		lastWriteFinished = get_systemtime();
		if (ret < 0) {
			log_error("%s write failed,ret:%d\n", szThreadName, ret);
			usleep(period_ms * 1000 / 2);
		}
		if (!pstThreadInfo->bStandby) {
			delta = lastWriteFinished - lastWriteTime;
			if (delta < 50 * 1000 * 1000 && delta > 3000 * 1000) {
				i32NumDelayedWrites++;
				log_warn("write blocked for %llu msecs, %d delayed writes\n",
					 delta, i32NumDelayedWrites);
				system("cat /proc/sysDMA/ch_status");
			}
		}
		pstThreadInfo->u32Count++;
	}
AOUT_THREAD_EXIT:
	//update tracks in vectors
	updateOutputThreadTracks(pstAoInstance, pstAoInstance->ao_attrs.u32ChnCnt);
	if (pTmpBuffer)
		free(pTmpBuffer);
	if (pAlsaBuffer)
		free(pAlsaBuffer);
	if (pOutBuffer)
		free(pOutBuffer);
	if (pReadBuffer)
		free(pReadBuffer);
	pcm_close(pstThreadInfo->pcmHandle);

	ao_cli_thread_status(1, 0, pstAoInstance->ao_attrs.u32ChnCnt, AUD_AO_THREAD_EXIT);
	printf("AudioOutputThread out.\n");

	return NULL;
}


CVI_VOID *AudioPrimaryInputThread(CVI_VOID *arg)
{
	int ret = 0;
	int AeChn = 0;
	int s32VqeBytesOut = 0;
	int s32OutResFrameLen = 0;
	int AiWriteLen = 0;
	unsigned long long  ts = 0;
	unsigned long long  lastTs = 0;
	bool bEnableChnVqe = false;
	bool bDevVqeStatus = false;
	CVI_ST_AUD_TRACK_INFO *pstTrackInfo = NULL;
	ST_AI_INSTANCE *pstAiInstance = (ST_AI_INSTANCE *)arg;
	CVI_ST_THREAD_INFO *pstThreadInfo = &pstAiInstance->stThreadInfo;
	struct pcm_config *pstPcmCfg = &pstThreadInfo->stPcmCfg;
	CVI_ST_AUD_TRACK_INFO **TrackVector = pstAiInstance->ppastTrackInfo;
	ST_THREAD_VQE_INFO stThreadVqeInfo;
	char TaskName[64];
	short *pDest = NULL;
	short *pAencDest = NULL;

	sprintf(TaskName, "arecord_%d", pstAiInstance->s32DevId);
	prctl(PR_SET_NAME, TaskName);

	ret = get_audio_pcm_config_params(pstPcmCfg, &pstAiInstance->aio_attrs, AUDIO_IN);
	if (ret < 0) {
		log_error("get output pcm config params failed.\n");
		return NULL;
	}

	pstThreadInfo->card = getSndCardId(MIC_CARD_STRING);
	if (pstThreadInfo->card < 0) {
		log_warn("card warn\n");
		pstThreadInfo->card = 0;
	}

	pstThreadInfo->pcmHandle = pcm_open(pstThreadInfo->card, 0, PCM_IN, pstPcmCfg);
	if (!pstThreadInfo->pcmHandle) {
		log_error("open input pcm device failed(%s).\n", pcm_get_error(pstThreadInfo->pcmHandle));
		return NULL;
	}

	log_info("TaskName:%s,card:%d,pcmHandle:%p\n",
		 TaskName, pstThreadInfo->card, pstThreadInfo->pcmHandle);
	int period_frame_len = PERIOD_MS * pstPcmCfg->rate / 1000;

	if (pstPcmCfg->rate != AUDIO_SAMPLE_RATE_8000 &&
			pstPcmCfg->rate != AUDIO_SAMPLE_RATE_16000 &&
			pstPcmCfg->rate != AUDIO_SAMPLE_RATE_32000) {
		log_warn("please use resample 8k/16k to dest_sampleRate[%d]\n", pstPcmCfg->rate);
		period_frame_len = 320;
	}

	const int period_bytes = period_frame_len * pstPcmCfg->channels * DEFAULT_BYTES_PER_SAMPLE;
	int max_period_bytes = AUDIO_SAMPLE_RATE_48000 / 1000 * period_bytes;
	short *pAlsaBuffer = (short *)malloc(period_bytes);
	short *pOutBuffer = (short *)malloc(period_bytes);
	short *pVolBuffer = (short *)malloc(period_bytes);
	short *pAiResBuffer = (short *)malloc(max_period_bytes);
	short *pAiAencBuffer = (short *)malloc(max_period_bytes);

	/* pAlsaBuffer --> pOutBuffer --> --> pAiResBuffer -->pAiAencBuffer */
	log_info("period_frame_len:%d, period_bytes:%d\n", period_frame_len, period_bytes);
	if (!pAlsaBuffer || !pOutBuffer || !pAiAencBuffer || !pAiResBuffer || !pVolBuffer) {
		log_error("malloc failed.\n");
		goto ERROR;
	}

	memset(&stThreadVqeInfo, 0, sizeof(ST_THREAD_VQE_INFO));
	memset(pAiResBuffer, 0, period_bytes);
	memset(pAiAencBuffer, 0, period_bytes);

	while (!pstThreadInfo->i32ExitPending) {

		memset(pAlsaBuffer, 0, period_bytes);
		_parsing_ain_dev_vqe_status(pstThreadInfo->card, &stThreadVqeInfo);

		ai_cli_thread_status(1, 0, pstAiInstance->aio_attrs.u32ChnCnt, AUD_PCM_READLY);
		ret = pcm_read(pstThreadInfo->pcmHandle, (char *)pAlsaBuffer, period_bytes);
		if (ret < 0) {
			log_error("read error,ret=%d,(%s)\n", ret, strerror(errno));
			goto ERROR;
		}

		ai_cli_thread_status(1, 0, pstAiInstance->aio_attrs.u32ChnCnt, AUD_VQE_READLY);
		if (pstThreadInfo->b_enable_dump || access("/tmp/ain_record", F_OK) == 0)
			cvitek_dump_audiodata("/tmp/ain_record.pcm", (char *)pAlsaBuffer, period_bytes);

		if (stThreadVqeInfo.bDevEnableVqe) {

			if (pstAiInstance->aio_attrs.enSamplerate != AUDIO_SAMPLE_RATE_8000
			    && pstAiInstance->aio_attrs.enSamplerate != AUDIO_SAMPLE_RATE_16000) {
				log_error("vqe only support 16000 or 8000.\n");
				goto ERROR;
			}

			_check_and_dump("/tmp/dump_before_aec", (char *)pAlsaBuffer, period_bytes);
			if (stThreadVqeInfo.bDevVqeWithAecOn) {/* aec agc anr */
				if (pstPcmCfg->channels != 2) {
					log_error("aec must two channels, curr_chn:%d\n", pstPcmCfg->channels);
					goto ERROR;
				}
				CVI_AudIn_AlgoProcess_AEC(
					(CVI_CHAR *)pAlsaBuffer,
					(CVI_CHAR *)pOutBuffer,
					period_bytes,
					&s32VqeBytesOut);

			} else {/* anr agc */
				CVI_AudIn_AlgoProcess_AnrAgc(
					(CVI_CHAR *)pAlsaBuffer,
					(CVI_CHAR *)pOutBuffer,
					period_bytes,
					&s32VqeBytesOut);
			}
			bDevVqeStatus = true;
			_check_and_dump("/tmp/dump_after_aec", (char *)pOutBuffer, s32VqeBytesOut);

		} else {
			if (bDevVqeStatus) {
				CVI_AudIn_AlgoDeInit();
				bDevVqeStatus = false;
			}
		}

		ai_cli_thread_status(1, 0, pstAiInstance->aio_attrs.u32ChnCnt, AUD_TRACK_READLY);
		for (unsigned int i = 0; i < pstAiInstance->aio_attrs.u32ChnCnt; i++) {
			pstTrackInfo = TrackVector[i];

			if (pstTrackInfo && (pstTrackInfo->state == STATE_RUNNING
							|| (pstTrackInfo->stBindinfo.bBind))) {
				pDest = pAlsaBuffer;
				AiWriteLen = period_frame_len;
				bEnableChnVqe = pstTrackInfo->stAiChnCfg.bEnableVqe;
				if (pstTrackInfo->stBindinfo.bBind) {
					AeChn = pstTrackInfo->stBindinfo.dstinfo.s32ChnId;
					int AiChn = gstAencInstance[AeChn].stBindinfo.srcinfo.s32ChnId;
					int AiDev = gstAencInstance[AeChn].stBindinfo.srcinfo.s32DevId;

					_parsing_aenc_channel_vqe_status(AiDev, AiChn, &gstAencInstance[AeChn]);
					bEnableChnVqe = gstAencInstance[AeChn].bVqeOn;
				}

				if (bEnableChnVqe) {
					pstTrackInfo->iChannels = 1;
					pDest = pOutBuffer;
				}

				if (pstTrackInfo->b_dump_enable) {
					char dump_name[64] = "arecord_";

					snprintf(dump_name + strlen(dump_name), 64, "%d.pcm",
								pstTrackInfo->iShmMemIndex);
					cvitek_dump_audiodata(dump_name, (char *)pDest,
						AiWriteLen * pstTrackInfo->iChannels * DEFAULT_BYTES_PER_SAMPLE);
				}

				if (pstAiInstance->ppVolinstance[i]) {//sw set vol
					_check_and_dump_num("/tmp/vol_before", i,
						(char *)pDest,
						AiWriteLen * pstTrackInfo->iChannels *
						DEFAULT_BYTES_PER_SAMPLE);

					vol_ctrl_process(pstAiInstance->ppVolinstance[i], pDest, pVolBuffer,
							AiWriteLen * pstTrackInfo->iChannels);
					pDest = pVolBuffer;
					_check_and_dump_num("/tmp/vol_after", i,
						(char *)pDest,
						AiWriteLen * pstTrackInfo->iChannels *
						DEFAULT_BYTES_PER_SAMPLE);
				}

			if (pstTrackInfo->stBindinfo.bBind) {/* ai bind aenc */

				if (gstAencInstance[AeChn].iAencShmMemIndex > 0) {
					s32OutResFrameLen = AiWriteLen;
					pAencDest = pDest;

					ai_cli_thread_status(0, i, 0, AUD_TRACK_RUNINNG);
					gstAencInstance[AeChn].s32SendBytePeriod =
							pstAiInstance->aio_attrs.u32PtNumPerFrm *
							pstTrackInfo->iChannels * DEFAULT_BYTES_PER_SAMPLE;
					if (pstTrackInfo->b_need_resample && pstTrackInfo->pResHandle) {
						_check_and_dump_num("/tmp/dump_ai_bind_aenc_Resin", i,
							(char *)pDest,
							s32OutResFrameLen * pstTrackInfo->iChannels *
							DEFAULT_BYTES_PER_SAMPLE);
						s32OutResFrameLen = CVI_Resampler_Process(
									pstTrackInfo->pResHandle,
									pDest,
									period_frame_len,
									pAiAencBuffer);

					if (s32OutResFrameLen > 0) {
						pAencDest = pAiAencBuffer;
						_check_and_dump_num("/tmp/dump_ai_bind_aenc_Resout", i,
							(char *)pAiAencBuffer,
							s32OutResFrameLen * pstTrackInfo->iChannels *
							DEFAULT_BYTES_PER_SAMPLE);
					}

						gstAencInstance[AeChn].s32SendBytePeriod =
								gstAencInstance[AeChn].s32SendBytePeriod
								* pstTrackInfo->iSampleRate
								/ pstAiInstance->aio_attrs.enSamplerate;

					}

					ai_cli_thread_status(0, i, 0, AUD_SHATEMEM_READLY);
					if (s32OutResFrameLen > 0) {
						ret = share_cyclebuffer_server_write(
									gstAencInstance[AeChn].iAencShmMemIndex,
									(char *)pAencDest, s32OutResFrameLen *
									pstTrackInfo->iChannels *
									DEFAULT_BYTES_PER_SAMPLE);

					if (ret < 0) {
						log_error("[write error], record[%p] FrameLen:%d ",
								pstTrackInfo, s32OutResFrameLen);
						printf("index:%d ret:%d\n",
								gstAencInstance[AeChn].iAencShmMemIndex,
								ret);

					} else if (ret == 0) {
						ts = get_systemtime();
						log_debug("[aiaenc overrun] record:%p index:%d ",
							pstTrackInfo,
							gstAencInstance[AeChn].iAencShmMemIndex);
						log_debug("iFrameLen:%d time:%2.2f[ms]\n",
							period_frame_len,
							(ts - lastTs)/1000.0);
						lastTs = ts;
					}

					}


				}


			} else { /* ai */

				ai_cli_thread_status(0, i, 0, AUD_TRACK_RUNINNG);
				if (pstTrackInfo->b_need_resample && pstTrackInfo->pResHandle) {

					_check_and_dump_num("/tmp/dump_ai_Resin", i, (char *)pDest,
								AiWriteLen * pstTrackInfo->iChannels *
								DEFAULT_BYTES_PER_SAMPLE);
					s32OutResFrameLen = CVI_Resampler_Process(pstTrackInfo->pResHandle,
								pDest,
								period_frame_len,
								pAiResBuffer);

					_check_and_dump_num("/tmp/dump_ai_Reskout", i, (char *)pAiResBuffer,
							s32OutResFrameLen * pstTrackInfo->iChannels *
							DEFAULT_BYTES_PER_SAMPLE);

					pDest = pAiResBuffer;
					AiWriteLen = s32OutResFrameLen;

				}

				ai_cli_thread_status(0, i, 0, AUD_SHATEMEM_READLY);
				if (pstTrackInfo->iShmMemIndex > 0 && AiWriteLen > 0) {
					ret = share_cyclebuffer_server_write(pstTrackInfo->iShmMemIndex,
									(char *)pDest,
									AiWriteLen * pstTrackInfo->iChannels *
										DEFAULT_BYTES_PER_SAMPLE);
					if (ret < 0) {
						log_error("[write error], record[%p] buffer:%d code:%d\n",
								pstTrackInfo, pstTrackInfo->iShmMemIndex, ret);
					} else if (ret == 0) {
						ts = get_systemtime();
						log_debug("[ai overrun], index:%d iFrameLen:%d time:%2.2f\n",
								pstTrackInfo->iShmMemIndex,
								AiWriteLen, (ts - lastTs)/1000.0);
						lastTs = ts;
					}
				}


			}

			}




		}

		pstThreadInfo->u32Count++;
	}

ERROR:
	SAFE_FREE_BUF(pAlsaBuffer);
	SAFE_FREE_BUF(pOutBuffer);
	SAFE_FREE_BUF(pVolBuffer);
	SAFE_FREE_BUF(pAiResBuffer);
	SAFE_FREE_BUF(pAiAencBuffer);
	CVI_AudIn_AlgoDeInit();
	pcm_close(pstThreadInfo->pcmHandle);

	ai_cli_thread_status(1, 0, pstAiInstance->aio_attrs.u32ChnCnt, AUD_AI_THREAD_EXIT);
	log_info("record out.\n");
	printf("AudioInputThread exit\n");
	return NULL;
}



