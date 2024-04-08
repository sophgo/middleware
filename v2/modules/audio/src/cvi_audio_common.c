#include<stdio.h>
#include <unistd.h>

#include<cvi_audio.h>
#include<cvi_type.h>
#include "cvi_ao_internal.h"
#include "cvi_audio_transcode.h"
#include "cvi_ai_internal.h"
#include "cvi_audio_vqe.h"

//ST_BIND_CONFIG gstBindConfig;
static CVI_BOOL g_audio_init = CVI_FALSE;//use ioctl cmd to set state in kernel

int CVIAUDIO_CHECK_NULL(void *ptr)
{
	if (ptr == NULL) {
		log_error("ptr is NULL\n");
		return -1;
	}
	return 0;
}

CVI_S32 CVI_AUDIO_INIT(void)
{
	if (g_audio_init) {
		log_error("alrdeay inited.\n");
		return CVI_ERR_AIO_NOT_PERM;
	}

	cviAudioGetDbgMask(&cviaud_dbg);
#ifdef RPC_MULTI_PROCESS_AUDIO
	printf("[rpc] %s\n", __func__);
	rpc_server_audio_init();
#else
	printf("[%s][%d]\n", __func__, __LINE__);
#endif
	CVI_AI_Init();
	CVI_AO_Init();
	CVI_AENC_Init();
	CVI_ADEC_Init();
#ifdef AUD_SUPPORT_KERNEL_MODE
	CVI_VQE_PathSelect(E_VQE_KERNEL_BLOCK_MODE);
#endif
	g_audio_init = CVI_TRUE;
	log_debug("log level:0x%x\n", cviaud_dbg);
	return CVI_SUCCESS;
}


CVI_S32 CVI_AUDIO_DEINIT(CVI_VOID)
{

	cviAudioGetDbgMask(&cviaud_dbg);
	log_debug("\n");
	CVI_AI_Deinit();
	CVI_AO_Deinit();
	CVI_AENC_Deinit();
	CVI_ADEC_Deinit();

#ifdef RPC_MULTI_PROCESS_AUDIO
	rpc_server_audio_deinit();
	printf("[rpc] %s\n", __func__);
#endif
	g_audio_init = CVI_FALSE;
	return CVI_SUCCESS;
}



static CVI_VOID _vqe_check_config(AI_TALKVQE_CONFIG_S *pstTalkVqeCfg)
{
	//dump mask and customer config
	printf("u32OpenMask[0x%x]\n", pstTalkVqeCfg->u32OpenMask);
	printf("s32WorkSampleRate[%d]\n", pstTalkVqeCfg->s32WorkSampleRate);
	printf("para_client_config[%d]\n", pstTalkVqeCfg->para_client_config);
	//dump aec config
	printf("stAecCfg.para_aec_filter_len[%d]\n", pstTalkVqeCfg->stAecCfg.para_aec_filter_len);
	printf("stAecCfg.para_aes_std_thrd[%d]\n", pstTalkVqeCfg->stAecCfg.para_aes_std_thrd);
	printf("stAecCfg.para_aes_supp_coeff[%d]\n", pstTalkVqeCfg->stAecCfg.para_aes_supp_coeff);
	//dump anr config
	printf("stAnrCfg.para_nr_init_sile_time[%d]\n", pstTalkVqeCfg->stAnrCfg.para_nr_init_sile_time);
	printf("stAnrCfg.para_nr_snr_coeff[%d]\n", pstTalkVqeCfg->stAnrCfg.para_nr_snr_coeff);
	//dump agc config
	printf("stAgcCfg.para_agc_max_gain[%d]\n", pstTalkVqeCfg->stAgcCfg.para_agc_max_gain);
	printf("stAgcCfg.para_agc_target_high[%d]\n", pstTalkVqeCfg->stAgcCfg.para_agc_target_high);
	printf("stAgcCfg.para_agc_target_low[%d]\n", pstTalkVqeCfg->stAgcCfg.para_agc_target_low);
	printf("stAgcCfg.para_agc_vad_ena[%d]\n", pstTalkVqeCfg->stAgcCfg.para_agc_vad_ena);
	//dump swaec delsy config
	printf("stAecDelayCfg.para_aec_init_filter_len[%d]\n", pstTalkVqeCfg->stAecDelayCfg.para_aec_init_filter_len);
	printf("stAecDelayCfg.para_dg_target[%d]\n", pstTalkVqeCfg->stAecDelayCfg.para_dg_target);
	printf("stAecDelayCfg.para_delay_sample[%d]\n", pstTalkVqeCfg->stAecDelayCfg.para_delay_sample);
	//dump reversion mask for customize
	printf("s32RevMask[0x%x]\n", pstTalkVqeCfg->s32RevMask);
	printf("para_notch_freq[%d]\n", pstTalkVqeCfg->para_notch_freq);
	printf("customize[%s]\n", pstTalkVqeCfg->customize);
}


CVI_S32 CVI_AUDIO_DEBUG(CVI_VOID)
{
	int _getvalue;

	printf("[ShowVersion]========================================\n");
	printf("version [%s]\n", _VERSION_TAG_);
	printf("[ShowDebugLevel]=====================================\n");
	cviAudioGetDbgMask(&cviaud_dbg);
	_getvalue = _cviAudGetEnv("cviaudio_level", "%d", NULL);
	printf("cviaudio_level get[%d]\n", _getvalue);
	printf("this is PRINT\n");
	log_error("this is ERR log_error\n");
	log_warn("this is log_warn\n");
	log_info("this is log_info\n");
	log_debug("this is log_debug\n");

	printf("[ShowVQE /mnt/data/audvqe.cfg]========================\n");
	if (access("/mnt/data/audvqe.cfg", F_OK) == 0) {
		FILE *fp = CVI_NULL;
		AI_TALKVQE_CONFIG_S stTalkVqeCfg;

		fp = fopen("/mnt/data/audvqe.cfg", "r");
		if (fp != NULL) {
			CVI_AUD_VQE_LoadParamFromCfg(fp, (CVI_U8 *)&stTalkVqeCfg);
			printf("Load from VQE Online mode-----------------[begin]\n");
			_vqe_check_config(&stTalkVqeCfg);
			printf("Load from VQE Online mode-----------------[End]\n");

		} else
			printf("Online mode vqe parameter:open failure\n");

		if (fp != CVI_NULL)
			fclose(fp);
	} else
		printf("Online mode vqe parameter:None\n");

	return CVI_SUCCESS;
}

unsigned long long _get_current_time(CVI_VOID)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000llu + tv.tv_usec / 1000llu;
}

unsigned long long _get_current_pts(CVI_VOID)
{
	struct timespec timestamp;
	/* This API mimic the cvi_vi stVFrame.u64PTS */
	/* pstFrameInfo->stVFrame.u64PTS = vb->buf.tv.tv_sec * 1000000 + vb->buf.tv.tv_usec;*/
	clock_gettime(CLOCK_MONOTONIC, &timestamp);
	return timestamp.tv_sec * 1000000llu + timestamp.tv_nsec / 1000llu;
}


static void dump_audiodata(char *filename, char *buf, unsigned int len)
{
	FILE *fp;

	if (filename == NULL) {
		return;
	}

	fp = fopen(filename, "ab+");
	fwrite(buf, 1, len, fp);
	fclose(fp);

}

void _check_and_dump(const char *filename, char *buf, unsigned int sizebytes)
{
	if (access(filename, F_OK) == 0) {
		char newfilename[128] = {0};

		snprintf(newfilename, 128, "%s.pcm", filename);

		dump_audiodata((char *)newfilename,
			       (char *)buf,
			       (unsigned int)sizebytes);
	}
}

void _check_and_dump_num(const char *filename, int num, char *buf, unsigned int sizebytes)
{
	if (access(filename, F_OK) == 0) {
		char newfilename[128] = {0};

		snprintf(newfilename, 128, "%s_%d.pcm", filename, num);

		dump_audiodata((char *)newfilename,
			       (char *)buf,
			       (unsigned int)sizebytes);
	}
}


/* system bind unbind api */
CVI_S32  CVI_AUD_SYS_Bind(const MMF_CHN_S *pstSrcChn,
			  const MMF_CHN_S *pstDestChn)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_sys_bind(pstSrcChn, pstDestChn);
	}
#endif
	if (pstSrcChn->enModId == CVI_ID_ADEC) { //src AI bind
		if (pstDestChn->enModId == CVI_ID_AO) {
			ST_ADEC_INSTANCE *_adec_instance = &gstAdecInstance[pstSrcChn->s32ChnId];
			ST_AO_INSTANCE *pstAoInstance = &gstAoInstance[pstDestChn->s32DevId];
			CVI_ST_AUD_TRACK_INFO_PTR pstTrackInfo =
				pstAoInstance->pastTrackInfo[pstDestChn->s32ChnId];
			if (CVIAUDIO_CHECK_NULL((void *)_adec_instance))
				return -1;
			if (CVIAUDIO_CHECK_NULL((void *)pstTrackInfo))
				return -1;
			if (_adec_instance->stBindinfo.bBind
			    || pstTrackInfo->stBindinfo.bBind) {
				log_error("already bind?\n");
				return -2;
			}
			_adec_instance->stBindinfo.bBind = CVI_TRUE;
			_adec_instance->stBindinfo.role = ROLE_E_SOURCE;
			_adec_instance->stBindinfo.dstinfo.s32DevId = pstDestChn->s32DevId;
			_adec_instance->stBindinfo.dstinfo.s32ChnId = pstDestChn->s32ChnId;

			pstTrackInfo->stBindinfo.bBind = CVI_TRUE;
			pstTrackInfo->stBindinfo.role = ROLE_E_DEST;
			pstTrackInfo->stBindinfo.srcinfo.s32DevId = pstSrcChn->s32DevId;
			pstTrackInfo->stBindinfo.srcinfo.s32ChnId = pstSrcChn->s32ChnId;
		}
	} else if (pstSrcChn->enModId == CVI_ID_AI) {
		if (pstDestChn->enModId == CVI_ID_AENC) {
			ST_AENC_INSTANCE *_aenc_instance = &gstAencInstance[pstDestChn->s32ChnId];
			ST_AI_INSTANCE *pstAiInstance = &gstAiInstance[pstSrcChn->s32DevId];
			CVI_ST_AUD_TRACK_INFO_PTR pstTrackInfo = pstAiInstance->ppastTrackInfo[pstSrcChn->s32ChnId];

			if (CVIAUDIO_CHECK_NULL((void *)_aenc_instance))
				return -1;
			if (CVIAUDIO_CHECK_NULL((void *)pstTrackInfo))
				return -1;

			_aenc_instance->stBindinfo.bBind = CVI_TRUE;
			_aenc_instance->stBindinfo.role = ROLE_E_DEST;
			_aenc_instance->stBindinfo.srcinfo.s32DevId = pstSrcChn->s32DevId;
			_aenc_instance->stBindinfo.srcinfo.s32ChnId = pstSrcChn->s32ChnId;

			pstTrackInfo->stBindinfo.bBind = CVI_TRUE;
			pstTrackInfo->stBindinfo.role = ROLE_E_SOURCE;
			pstTrackInfo->stBindinfo.dstinfo.s32DevId = pstDestChn->s32DevId;
			pstTrackInfo->stBindinfo.dstinfo.s32ChnId = pstDestChn->s32ChnId;


		}

	}


	return CVI_SUCCESS;
}
CVI_S32  CVI_AUD_SYS_UnBind(const MMF_CHN_S *pstSrcChn,
			    const MMF_CHN_S *pstDestChn)
{
#ifdef RPC_MULTI_PROCESS_AUDIO
	if (!isAudioMaster()) {
		printf("[rpc] %s\n", __func__);
		return rpc_client_aud_sys_unbind(pstSrcChn, pstDestChn);
	}
#endif

	if (pstSrcChn->enModId == CVI_ID_ADEC) { //src AI bind
		if (pstDestChn->enModId == CVI_ID_AO) {
			ST_ADEC_INSTANCE *_adec_instance = &gstAdecInstance[pstSrcChn->s32ChnId];
			ST_AO_INSTANCE *pstAoInstance = &gstAoInstance[pstDestChn->s32DevId];
			CVI_ST_AUD_TRACK_INFO_PTR pstTrackInfo =
				pstAoInstance->pastTrackInfo[pstDestChn->s32ChnId];
			if (CVIAUDIO_CHECK_NULL((void *)_adec_instance))
				return -1;
			if (CVIAUDIO_CHECK_NULL((void *)pstTrackInfo))
				return -1;
			if (!_adec_instance->stBindinfo.bBind
			    || !pstTrackInfo->stBindinfo.bBind) {
				log_error("not bind yet.\n");
				return -2;
			}
			if ((_adec_instance->stBindinfo.dstinfo.s32DevId == pstDestChn->s32DevId)
			    && (_adec_instance->stBindinfo.dstinfo.s32ChnId == pstDestChn->s32ChnId)
			    && (pstTrackInfo->stBindinfo.srcinfo.s32DevId == pstSrcChn->s32DevId)
			    && (pstTrackInfo->stBindinfo.srcinfo.s32ChnId == pstSrcChn->s32ChnId)) {
				memset(&_adec_instance->stBindinfo, 0, sizeof(AUD_BIND_INFO));
				memset(&pstTrackInfo->stBindinfo, 0, sizeof(AUD_BIND_INFO));
			} else {
				log_error("unbind failed,error params.\n");
				return -3;
			}

		}
	} else if (pstSrcChn->enModId == CVI_ID_AI) {
		if (pstDestChn->enModId == CVI_ID_AENC) {
			ST_AENC_INSTANCE *_aenc_instance = &gstAencInstance[pstDestChn->s32ChnId];
			ST_AI_INSTANCE *pstAiInstance = &gstAiInstance[pstSrcChn->s32DevId];
			CVI_ST_AUD_TRACK_INFO_PTR pstTrackInfo = pstAiInstance->ppastTrackInfo[pstSrcChn->s32ChnId];

			if (CVIAUDIO_CHECK_NULL((void *)_aenc_instance))
				return -1;
			if (CVIAUDIO_CHECK_NULL((void *)pstTrackInfo))
				return -1;

			if (!_aenc_instance->stBindinfo.bBind
			    || !pstTrackInfo->stBindinfo.bBind) {
				log_error("not bind yet.\n");
				return -2;
			}

			if ((_aenc_instance->stBindinfo.dstinfo.s32DevId == pstDestChn->s32DevId)
			    && (_aenc_instance->stBindinfo.dstinfo.s32ChnId == pstDestChn->s32ChnId)
			    && (pstTrackInfo->stBindinfo.srcinfo.s32DevId == pstSrcChn->s32DevId)
			    && (pstTrackInfo->stBindinfo.srcinfo.s32ChnId == pstSrcChn->s32ChnId)) {
				memset(&_aenc_instance->stBindinfo, 0, sizeof(AUD_BIND_INFO));
				memset(&pstTrackInfo->stBindinfo, 0, sizeof(AUD_BIND_INFO));
			} else {
				log_error("unbind failed,error params.\n");
				return -3;
			}

		}

	}

	return CVI_SUCCESS;
}



