#include <stdio.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <stdint.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "cvi_audio_speed.h"

#define CVITEK_AUD_OUT_PATH_BUFFER   (10240)
#define DEFAULT_BYTES_PER_SAMPLE 2//16bit = 2 bytes

//---------------------------------------------------------------------speed up API[start]
//declarate two global variables
static ST_CVIAUDIO_SPEED_CTRL_FUNC	gstAoSpeedCtrlFunct = {0};
ST_CVIAUDIO_SPEED_PLAY_INSTANCE gstAoSpeedInstance[CVI_MAX_AO_DEVICE_ID_NUM];
void *gpstCircleBuffer = CVI_NULL;


static CVI_BOOL _cviaudio_ao_check_speedplay_function_exist(void)
{
	if (!gstAoSpeedCtrlFunct.psonicCreateStream || !gstAoSpeedCtrlFunct.psonicDestroyStream) {
		//log_error("gstAoSpeedCtrlFunct FUNCTION PT NULL\n");
		return CVI_FALSE;

	} else {
		log_debug("gstAoSpeedCtrlFunct FUNCTION PT Already exist\n");
		return CVI_TRUE;
	}
}


static CVI_S32 _cviaudio_dlopen_ao_speedplay(void)
{
	memset(&gstAoSpeedCtrlFunct, 0, sizeof(ST_CVIAUDIO_SPEED_CTRL_FUNC));
	gstAoSpeedCtrlFunct.pLibHandle = dlopen(CVIAUDIO_SPEED_PLAY_LIB, RTLD_LAZY | RTLD_LOCAL);
	if (gstAoSpeedCtrlFunct.pLibHandle  == CVI_NULL) {
		printf("dlopen %s failed!\n", CVIAUDIO_SPEED_PLAY_LIB);
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicCreateStream = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicCreateStream");
	if (gstAoSpeedCtrlFunct.psonicCreateStream == CVI_NULL) {
		log_error("dlsym failed! sonicCreateStream\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicDestroyStream = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicDestroyStream");
	if (gstAoSpeedCtrlFunct.psonicDestroyStream == CVI_NULL) {
		log_error("dlsym failed! sonicDestroyStream\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicWriteFloatToStream = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicWriteFloatToStream");
	if (gstAoSpeedCtrlFunct.psonicWriteFloatToStream == CVI_NULL) {
		log_error("dlsym failed! sonicWriteFloatToStream\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicWriteShortToStream = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicWriteShortToStream");
	if (gstAoSpeedCtrlFunct.psonicWriteShortToStream == CVI_NULL) {
		log_error("dlsym failed! sonicWriteShortToStream\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicWriteUnsignedCharToStream = dlsym(gstAoSpeedCtrlFunct.pLibHandle,
			"sonicWriteUnsignedCharToStream");
	if (gstAoSpeedCtrlFunct.psonicWriteUnsignedCharToStream == CVI_NULL) {
		log_error("dlsym failed! sonicWriteUnsignedCharToStream\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicReadFloatFromStream = dlsym(gstAoSpeedCtrlFunct.pLibHandle,
			"sonicReadFloatFromStream");
	if (gstAoSpeedCtrlFunct.psonicReadFloatFromStream == CVI_NULL) {
		log_error("dlsym failed! sonicReadFloatFromStream\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicReadShortFromStream = dlsym(gstAoSpeedCtrlFunct.pLibHandle,
			"sonicReadShortFromStream");
	if (gstAoSpeedCtrlFunct.psonicReadShortFromStream == CVI_NULL) {
		log_error("dlsym failed! sonicReadShortFromStream\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicReadUnsignedCharFromStream = dlsym(gstAoSpeedCtrlFunct.pLibHandle,
			"sonicReadUnsignedCharFromStream");
	if (gstAoSpeedCtrlFunct.psonicReadUnsignedCharFromStream == CVI_NULL) {
		log_error("dlsym failed! psonicReadUnsignedCharFromStream\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicFlushStream = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicFlushStream");
	if (gstAoSpeedCtrlFunct.psonicFlushStream == CVI_NULL) {
		log_error("dlsym failed! sonicFlushStream\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicSamplesAvailable = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicSamplesAvailable");
	if (gstAoSpeedCtrlFunct.psonicSamplesAvailable == CVI_NULL) {
		log_error("dlsym failed! sonicSamplesAvailable\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicGetSpeed = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicGetSpeed");
	if (gstAoSpeedCtrlFunct.psonicGetSpeed == CVI_NULL) {
		log_error("dlsym failed! sonicGetSpeed\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicSetSpeed = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicSetSpeed");
	if (gstAoSpeedCtrlFunct.psonicSetSpeed == CVI_NULL) {
		log_error("dlsym failed! sonicSetSpeed\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicGetPitch = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicGetPitch");
	if (gstAoSpeedCtrlFunct.psonicGetPitch == CVI_NULL) {
		log_error("dlsym failed! sonicGetPitch\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicSetPitch = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicSetPitch");
	if (gstAoSpeedCtrlFunct.psonicSetPitch == CVI_NULL) {
		log_error("dlsym failed! sonicSetPitch\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicGetRate = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicGetRate");
	if (gstAoSpeedCtrlFunct.psonicGetRate == CVI_NULL) {
		log_error("dlsym failed! sonicGetRate\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicSetRate = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicSetRate");
	if (gstAoSpeedCtrlFunct.psonicSetRate == CVI_NULL) {
		log_error("dlsym failed! sonicSetRate\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicGetVolume = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicGetVolume");
	if (gstAoSpeedCtrlFunct.psonicGetVolume == CVI_NULL) {
		log_error("dlsym failed! sonicGetVolume\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicSetVolume = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicSetVolume");
	if (gstAoSpeedCtrlFunct.psonicSetVolume == CVI_NULL) {
		log_error("dlsym failed! sonicSetVolume\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicGetChordPitch = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicGetChordPitch");
	if (gstAoSpeedCtrlFunct.psonicGetChordPitch == CVI_NULL) {
		log_error("dlsym failed! sonicGetChordPitch\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicSetChordPitch = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicSetChordPitch");
	if (gstAoSpeedCtrlFunct.psonicSetChordPitch == CVI_NULL) {
		log_error("dlsym failed! sonicSetChordPitch\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicGetQuality = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicGetQuality");
	if (gstAoSpeedCtrlFunct.psonicGetQuality == CVI_NULL) {
		log_error("dlsym failed! sonicGetQuality\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicSetQuality = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicSetQuality");
	if (gstAoSpeedCtrlFunct.psonicSetQuality == CVI_NULL) {
		log_error("dlsym failed! sonicSetQuality\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicGetSampleRate = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicGetSampleRate");
	if (gstAoSpeedCtrlFunct.psonicGetSampleRate == CVI_NULL) {
		log_error("dlsym failed! sonicGetSampleRate\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicSetSampleRate = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicSetSampleRate");
	if (gstAoSpeedCtrlFunct.psonicSetSampleRate == CVI_NULL) {
		log_error("dlsym failed! sonicSetSampleRate\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicGetNumChannels = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicGetNumChannels");
	if (gstAoSpeedCtrlFunct.psonicGetNumChannels == CVI_NULL) {
		log_error("dlsym failed! sonicGetNumChannels\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicSetNumChannels = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicSetNumChannels");
	if (gstAoSpeedCtrlFunct.psonicSetNumChannels == CVI_NULL) {
		log_error("dlsym failed! sonicSetNumChannels\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicSetNumChannels = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicSetNumChannels");
	if (gstAoSpeedCtrlFunct.psonicSetNumChannels == CVI_NULL) {
		log_error("dlsym failed! sonicSetNumChannels\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicChangeFloatSpeed = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicChangeFloatSpeed");
	if (gstAoSpeedCtrlFunct.psonicChangeFloatSpeed == CVI_NULL) {
		log_error("dlsym failed! sonicChangeFloatSpeed\n");
		return CVI_FAILURE;
	}

	gstAoSpeedCtrlFunct.psonicChangeShortSpeed = dlsym(gstAoSpeedCtrlFunct.pLibHandle, "sonicChangeShortSpeed");
	if (gstAoSpeedCtrlFunct.psonicChangeShortSpeed == CVI_NULL) {
		log_error("dlsym failed! sonicChangeShortSpeed\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_AO_EnableSpeedPlay(AUDIO_DEV AoDevId, ST_CVIAO_SPEEDPLAY_CONFIG  AoSpeedCfg)
{
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("\n");
		return CVI_ERR_AO_INVALID_DEVID;
	}
	//step 1 check dlopen exist in special handle
	CVI_BOOL bCheckFunctPt = CVI_FALSE;
	CVI_S32 s32Ret = CVI_FAILURE;

	CycleBufferInit(&gpstCircleBuffer, CVITEK_AUD_OUT_PATH_BUFFER);//4*8*200*3*2  1200ms data
	bCheckFunctPt = _cviaudio_ao_check_speedplay_function_exist();
	if (bCheckFunctPt != CVI_TRUE) {
		printf("dlopen and connect function pt from lib:%s\n", CVIAUDIO_SPEED_PLAY_LIB);
		s32Ret = _cviaudio_dlopen_ao_speedplay();
		if (s32Ret != CVI_SUCCESS) {
			log_error("\n");
			return CVI_FAILURE;

		} else
			printf("[cviaudio]dlopen speed play success!\n");
	}
	//step 2 create handle for AoDevId and attach to instance
	if ((gstAoSpeedInstance[AoDevId].stream != CVI_NULL) && (gstAoSpeedInstance[AoDevId].bEnable)) {
		printf("cviaudio Ao Speed play handle already exist dev[%d][%x]..update param\n",
		       AoDevId, gstAoSpeedInstance[AoDevId].stream);
	} else {
		CycleBufferInit((void **)&gstAoSpeedInstance[AoDevId].ao_speedfifo,
				CVITEK_AUD_OUT_PATH_BUFFER + CVITEK_AUD_OUT_PATH_BUFFER / 2);
		gstAoSpeedInstance[AoDevId].speedout_buffer = (short *)calloc(CVIAUDIO_AO_SPEEDOUT_BUF_SIZE,
				sizeof(short));
		gstAoSpeedInstance[AoDevId].before_buffer = (short *)calloc(DEFAULT_AO_SPEED_PROCESS_BUFFER,
				sizeof(short));
		gstAoSpeedInstance[AoDevId].speedout_buffersize = CVIAUDIO_AO_SPEEDOUT_BUF_SIZE;
		gstAoSpeedInstance[AoDevId].stream = gstAoSpeedCtrlFunct.psonicCreateStream(AoSpeedCfg.sampleRate,
						     AoSpeedCfg.channels);
	}

	printf("AOSpeedPlay sampleRate[%d] rate[%f] channels[%d] pitch[%f] speed[%f] vol[%f]\n",
	       AoSpeedCfg.sampleRate,
	       (float)AoSpeedCfg.rate,
	       (int)AoSpeedCfg.channels,
	       (float)AoSpeedCfg.pitch,
	       (float)AoSpeedCfg.speed,
	       (float)AoSpeedCfg.volume);


	gstAoSpeedCtrlFunct.psonicSetSpeed(gstAoSpeedInstance[AoDevId].stream, AoSpeedCfg.speed);
	gstAoSpeedCtrlFunct.psonicSetPitch(gstAoSpeedInstance[AoDevId].stream, AoSpeedCfg.pitch);
	gstAoSpeedCtrlFunct.psonicSetRate(gstAoSpeedInstance[AoDevId].stream, AoSpeedCfg.rate);
	gstAoSpeedCtrlFunct.psonicSetVolume(gstAoSpeedInstance[AoDevId].stream, AoSpeedCfg.volume);
	gstAoSpeedCtrlFunct.psonicSetChordPitch(gstAoSpeedInstance[AoDevId].stream, 0);
	gstAoSpeedCtrlFunct.psonicSetQuality(gstAoSpeedInstance[AoDevId].stream, 0);
	gstAoSpeedInstance[AoDevId].bEnable = CVI_TRUE;
	//always flush, if user change parameter by this API
	gstAoSpeedCtrlFunct.psonicFlushStream(gstAoSpeedInstance[AoDevId].stream);
	memcpy(&gstAoSpeedInstance[AoDevId].cfg, &AoSpeedCfg, sizeof(ST_CVIAO_SPEEDPLAY_CONFIG));
	return CVI_SUCCESS;
}

CVI_S32 CVI_AO_DisableSpeedPlay(AUDIO_DEV AoDevId)
{
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("\n");
		return CVI_ERR_AO_INVALID_DEVID;
	}
	//step 1 always check the AoDevId valid and speed play function pointer exist or not
	if (gstAoSpeedInstance[AoDevId].bEnable == CVI_TRUE)
		gstAoSpeedInstance[AoDevId].bEnable = CVI_FALSE;
	return CVI_SUCCESS;
}

CVI_S32 CVI_AO_DestroySpeedPlay(AUDIO_DEV AoDevId)
{
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("\n");
		return CVI_ERR_AO_INVALID_DEVID;
	}

	gstAoSpeedInstance[AoDevId].bEnable = CVI_FALSE;

	if (gstAoSpeedInstance[AoDevId].stream != CVI_NULL) {
		gstAoSpeedCtrlFunct.psonicDestroyStream(gstAoSpeedInstance[AoDevId].stream);
		gstAoSpeedInstance[AoDevId].stream = CVI_NULL;
	}
	if (gstAoSpeedInstance[AoDevId].ao_speedfifo) {
		CycleBufferDestroy(gstAoSpeedInstance[AoDevId].ao_speedfifo);
		gstAoSpeedInstance[AoDevId].ao_speedfifo = CVI_NULL;
	}
	SAFE_FREE_BUF(gstAoSpeedInstance[AoDevId].speedout_buffer);
	SAFE_FREE_BUF(gstAoSpeedInstance[AoDevId].before_buffer);
	memset(&gstAoSpeedInstance[AoDevId], 0, sizeof(ST_CVIAUDIO_SPEED_PLAY_INSTANCE));
	if (gstAoSpeedCtrlFunct.pLibHandle != CVI_NULL)
		dlclose(gstAoSpeedCtrlFunct.pLibHandle);
	if (gstAoSpeedCtrlFunct.psonicCreateStream != CVI_NULL)
		memset(&gstAoSpeedCtrlFunct, 0, sizeof(ST_CVIAUDIO_SPEED_CTRL_FUNC));

	CycleBufferDestroy(gpstCircleBuffer);
	gpstCircleBuffer = CVI_NULL;

	return CVI_SUCCESS;
}

CVI_S32 CVI_AO_SpeedPlayProc(AUDIO_DEV AoDevId, short *input_buffer,
			     CVI_S32 s32InputBytes, short **output_buffer, CVI_S32 s32OutputBufferSizeBytes)
{
	CVI_S32 s32ReturnBytes = 0;
	CVI_S32 s32BufferFreeLevel = 0;
	CVI_S32 s32BufferDataLevel = 0;
	CVI_BOOL bCheck = CVI_FALSE;
	CVI_S32 s32Ret = 0;
	CVI_S32 s32SpeedOutSamples = 0;
	CVI_S32 s32ExpectOutPerChn = 0;
	CVI_S32 s32Channels = gstAoSpeedInstance[AoDevId].cfg.channels;
	CVI_S32 s32BytesInAllChannels = gstAoSpeedInstance[AoDevId].cfg.channels * DEFAULT_BYTES_PER_SAMPLE;
	short *pout = (short *)(*output_buffer);

	//step 1 basic error handle
	if (CHECK_AO_DEVID_VALID(AoDevId)) {
		log_error("\n");
		return CVI_ERR_AO_INVALID_DEVID;
	}

	if (input_buffer == NULL || output_buffer == NULL) {
		log_error("Null buffer detect\n");
		return CVI_FAILURE;//return size < 0
	}

	if (!gstAoSpeedInstance[AoDevId].bEnable) {
		log_error("\n");
		return CVI_FAILURE;
	}

	if (s32BytesInAllChannels <= 0) {
		log_error("Channel info failure!!\n");
		return CVI_FAILURE;
	}

	if (gstAoSpeedInstance[AoDevId].stream == CVI_NULL ||
	    gstAoSpeedInstance[AoDevId].ao_speedfifo == CVI_NULL ||
	    gstAoSpeedInstance[AoDevId].speedout_buffer == CVI_NULL) {
		log_error(" NULL pt detect\n");
		return CVI_FAILURE;
	}

	bCheck = _cviaudio_ao_check_speedplay_function_exist();
	if (bCheck == CVI_FALSE) {
		log_error("\n");
		return CVI_FAILURE;
	}
	//step2  check the buffer level, stuffing data first , then read out and convert every
	//1024 samples
	s32BufferFreeLevel = CycleBufferSize(gstAoSpeedInstance[AoDevId].ao_speedfifo) -
			     CycleBufferDataLen(gstAoSpeedInstance[AoDevId].ao_speedfifo);
	s32ExpectOutPerChn = AO_SPEED_PROC_THRESHOLD_BYTES / s32Channels;
	s32ExpectOutPerChn = s32ExpectOutPerChn / DEFAULT_BYTES_PER_SAMPLE;
	if (s32BufferFreeLevel > s32InputBytes) {
		//step 3-1 stuffing data first then do the speed proc
		s32Ret = CycleBufferWriteWait(
				 gstAoSpeedInstance[AoDevId].ao_speedfifo,
				 (char *)input_buffer,
				 s32InputBytes, 5000);
		if (s32Ret == 0)
			log_error("cviaudio_ao_speedbuffer abnormal!\n");
		//step 4: do the speed proc while buffer data > 1024
		s32BufferDataLevel = CycleBufferDataLen(gstAoSpeedInstance[AoDevId].ao_speedfifo);

		while (s32BufferDataLevel >= AO_SPEED_PROC_THRESHOLD_BYTES) {
			s32Ret = CycleBufferRead(gstAoSpeedInstance[AoDevId].ao_speedfifo,
						 (char *)gstAoSpeedInstance[AoDevId].before_buffer,
						 AO_SPEED_PROC_THRESHOLD_BYTES);

			if (access("/tmp/dump_speedin", F_OK) == 0) {
				ao_dump_audiodata((char *)"/tmp/dump_speedin.pcm",
						  (char *)gstAoSpeedInstance[AoDevId].before_buffer,
						  s32Ret);
			}
			//write in the samples of single channels, but actual have two channels bytes
			gstAoSpeedCtrlFunct.psonicWriteShortToStream(gstAoSpeedInstance[AoDevId].stream,
					gstAoSpeedInstance[AoDevId].before_buffer,
					(s32Ret / DEFAULT_BYTES_PER_SAMPLE) / s32Channels);

			do {
				s32SpeedOutSamples = gstAoSpeedCtrlFunct.psonicReadShortFromStream(
							     gstAoSpeedInstance[AoDevId].stream,
							     (short *)pout,
							     s32ExpectOutPerChn);

				//printf("[%s][%d]<-READ samples[%d]\n", __func__, __LINE__, s32SpeedOutSamples);
				if (s32SpeedOutSamples == 0)
					break;
				if (access("/tmp/dump_speedout", F_OK) == 0) {
					ao_dump_audiodata((char *)"/tmp/dump_speedout.pcm",
							  (char *)pout,
							  s32SpeedOutSamples * DEFAULT_BYTES_PER_SAMPLE * s32Channels);
				}
				if (s32OutputBufferSizeBytes > s32SpeedOutSamples * s32BytesInAllChannels) {
					s32OutputBufferSizeBytes -= s32SpeedOutSamples * s32BytesInAllChannels;
					s32ReturnBytes += s32SpeedOutSamples * s32BytesInAllChannels;
					pout +=  s32SpeedOutSamples * s32Channels;
				} else {
					printf("->Not enough output buffer...!!OutLeft[%d]Cur[%d]\n",
					       s32OutputBufferSizeBytes, s32SpeedOutSamples * s32BytesInAllChannels);
					//call to psonicReadShortFromStream next time
					break;
				}
			} while (s32SpeedOutSamples > 0);
			s32BufferDataLevel = CycleBufferDataLen(gstAoSpeedInstance[AoDevId].ao_speedfifo);
		}

	} else {
		printf("[cviaudio]xxxxxxxxxxxxxxxxxx->\n");
		//step 3-2 do the speed proc first then stuff
		s32BufferDataLevel = CycleBufferDataLen(gstAoSpeedInstance[AoDevId].ao_speedfifo);
		while (s32BufferDataLevel >= AO_SPEED_PROC_THRESHOLD_BYTES) {
			s32Ret = CycleBufferRead(gstAoSpeedInstance[AoDevId].ao_speedfifo,
						 (char *)gstAoSpeedInstance[AoDevId].before_buffer,
						 AO_SPEED_PROC_THRESHOLD_BYTES);

			gstAoSpeedCtrlFunct.psonicWriteShortToStream(gstAoSpeedInstance[AoDevId].stream,
					gstAoSpeedInstance[AoDevId].before_buffer,
					(s32Ret / DEFAULT_BYTES_PER_SAMPLE) / s32Channels);
			s32SpeedOutSamples = gstAoSpeedCtrlFunct.psonicReadShortFromStream(
						     gstAoSpeedInstance[AoDevId].stream,
						     pout,
						     s32ExpectOutPerChn);

			if (s32OutputBufferSizeBytes > s32SpeedOutSamples * s32BytesInAllChannels) {
				s32OutputBufferSizeBytes -= (s32SpeedOutSamples * s32BytesInAllChannels);
				s32ReturnBytes += s32SpeedOutSamples * s32BytesInAllChannels;
				pout +=  s32SpeedOutSamples;
			} else
				break;

			s32BufferDataLevel = CycleBufferDataLen(gstAoSpeedInstance[AoDevId].ao_speedfifo);
		}
		s32Ret = CycleBufferWriteWait(
				 gstAoSpeedInstance[AoDevId].ao_speedfifo,
				 (char *)input_buffer,
				 s32InputBytes, 5000);
		if (s32Ret == 0)
			log_error("cviaudio_ao_speedbuffer abnormal!\n");
	}

	return s32ReturnBytes;
}


