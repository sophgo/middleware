#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/queue.h>
#include <pthread.h>
#include "nn.h"
#include "reqrep.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include "cvi_audio_rpc.h"
#include "alog.h"
#ifdef SHARE_MEM_IN_AUDIO_RPC
#include "cvi_audio_shm.h"
#endif
//macro definition------start
#ifndef AUD_RPC_UNUSED
#define AUD_RPC_UNUSED(x) ((x) = (x))
#endif

#ifndef BYTES_PER_AUD_SAMPLE
#define BYTES_PER_AUD_SAMPLE 2
#endif
//compiler flag for debug --- start
#define CHECK_AUD_SERVER_CMD 0
//compiler flag for debug --- stop
#define DEFAULT_AUD_DEV_ID 0
#define DEFAULT_AUD_CHN_ID 0

#define RPC_FD_AUD_AIO 0
#define RPC_FD_AUD_AENC 1
#define RPC_FD_AUD_ADEC 2
#define RPC_FD_AUD_AIO_CH1 3
#define RPC_FD_AUD_AIO_CH2 4
int g_audio_rpc_fd[8] = {[0 ... 7] = -1};//AIO 6 channel, AENC 1chn, ADEC 1chn
//CVI_S32 s32Rpc_Aud_level = 1;
static pthread_mutex_t g_rpc_init_lock = PTHREAD_MUTEX_INITIALIZER;
#if DUMP_SERVER_CLIENT
FILE *fd_server;
#endif

#define MODE_AUDIO_AIO 0x02
#define MODE_AUDIO_AENC 0x04
#define MODE_AUDIO_ADEC 0x06
#define AIN_GET_FRAME_FORCE_BLOCK (-1)
#define RPC_BUFFER_BYTE 2048

#define RPC_AUD_ERR(CMD, FUNC, LINE)\
	do { \
			printf("cmd[%d], func[%s] line[%d]\n", CMD, FUNC, LINE);\
	} while (0)

//macro definition------end
//global var/function-------start
#define TIME_CHECK 0
#if TIME_CHECK
unsigned long long _get_current_time_for_rpc(CVI_VOID)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
#endif

CVI_U8	RpcEncBuff[RPC_BUFFER_BYTE];
CVI_U8	RpcDecBuff[RPC_BUFFER_BYTE];
CVI_U8	RpcAinGetBuff[RPC_BUFFER_BYTE];

CVI_U8 RpcServAdecSendBuf[3096];
static pthread_t s_pthRpc[AUD_RPC_SERVER_CNT];
static bool s_master;
static bool s_rpc_server_run;
static CVI_S32 aud_client_common(CVI_S32 mode,
			CVI_S32 devId,
			CVI_S32 chnId,
			CVI_S32 cmd,
			CVI_S32 add_size,
			char *add_data,
			void *pGetVal);
#ifdef SHARE_MEM_IN_AUDIO_RPC
static pthread_t s_pthAinStream[CVI_AUD_MAX_CHANNEL_NUM];
AI_SHARE_MEM_INFO stAinStreamShmInfo[CVI_AUD_MAX_CHANNEL_NUM] = {0};
//for aenc

static pthread_t s_pthAudStream[AENC_MAX_CHN_NUM];
AUD_STREAM_SHARE_MEM_INFO stAencStreamShmInfo[AENC_MAX_CHN_NUM] = {0};

//for ao
static pthread_t s_pthAoStream[CVI_AUD_MAX_CHANNEL_NUM];
AO_SHARE_MEM_INFO stAoStreamShmInfo[CVI_AUD_MAX_CHANNEL_NUM] = {0};

//for dec
static pthread_t s_pthAdecStream[CVI_AUD_MAX_CHANNEL_NUM];
ADEC_SHARE_MEM_INFO stAdecStreamShmInfo[CVI_AUD_MAX_CHANNEL_NUM] = {0};

#endif
//global var/function-------end

int rpc_aud_client_response_proc(char *response, int len, void *args)
{
	AUD_RPC_UNUSED(len);
	AUD_RPC_UNUSED(args);
	int ret = 0;
	int cmd;
	//uint64_t  *pu64CurPTS = NULL;
	AIO_ATTR_S *pstAttr;
	AUDIO_FRAME_S *pstFrm;
	AUDIO_STREAM_S *pstStream;
	AUDIO_FRAME_INFO_S *pstFrmInfo;
	AI_CHN_PARAM_S *pstChnParam;
	AI_TALKVQE_CONFIG_S *pstVqeConfig;
	char *pChar;
	int sizebytes = 0;
	//AEC_FRAME_S *pstAecFrm;

	struct rpc_aud_msg *pRpcMsg = (struct rpc_aud_msg *)response;

	cmd = GET_CMD(pRpcMsg->msg);
	if (pRpcMsg->magic != AUD_RPC_MAGIC) {
		printf("magic error\n");
		return -1;
	}
	ret = pRpcMsg->result;
	if (ret) {
		printf("cmd:%d failed, ret:%d\n", cmd, ret);
		return ret;
	}

	switch (cmd) {
	case RPC_CMD_AUD_SYS_BIND:
		break;
	case RPC_CMD_AUD_SYS_UNBIND:
		break;
	case RPC_CMD_AI_SET_PUB_ATTR:
		break;
	case RPC_CMD_AI_GET_PUB_ATTR:
		if (args != NULL) {
			pstAttr = (AIO_ATTR_S *)args;
			memcpy(pstAttr, pRpcMsg->body, sizeof(AIO_ATTR_S));
		} else
			RPC_AUD_ERR(RPC_CMD_AI_GET_PUB_ATTR, __func__, __LINE__);
		break;
	case RPC_CMD_AI_ENABLE:
		break;
	case RPC_CMD_AI_DISABLE:
		break;
	case RPC_CMD_AI_ENABLE_CHN:
		break;
	case RPC_CMD_AI_DISABLE_CHN:
		break;
	case RPC_CMD_AI_GET_FRAME:

#ifndef SHARE_MEM_IN_AUDIO_RPC
		pstFrm = (AUDIO_FRAME_S *)args;
		memcpy(pstFrm, pRpcMsg->body, sizeof(AUDIO_FRAME_S));
		printf("[%s][%d]len[%d]xxx[0x%x]\n",
			__func__,
			__LINE__,
			pRpcMsg->body_len,
			pstFrm->u64VirAddr[0]);
		sizebytes = pstFrm->u32Len * BYTES_PER_AUD_SAMPLE * (pstFrm->enSoundmode + 1);
		memcpy(RpcAinGetBuff, (pRpcMsg->body + sizeof(AUDIO_FRAME_S)), sizebytes);
		pstFrm->u64VirAddr[0] = RpcAinGetBuff;
#if DUMP_SERVER_CLIENT
		FILE *fd_client;

		fd_client = fopen("client.raw", "ab+");
		fwrite(pstFrm->u64VirAddr[0], 1, sizebytes, fd_client);
		fclose(fd_client);
#endif
#endif
		break;
	case RPC_CMD_AI_RELEASE_FRAME:
		if (args != NULL) {
		pstFrm = (AUDIO_FRAME_S *)args;
		memcpy(pstFrm,  pRpcMsg->body, sizeof(AUDIO_FRAME_S));
		}
		break;
	case RPC_CMD_AI_SET_CHN_PARAM:
		break;
	case RPC_CMD_AI_GET_CHN_PARAM:
		if (args != NULL) {
			pstChnParam = (AI_CHN_PARAM_S *)args;
			memcpy(pstChnParam, pRpcMsg->body, sizeof(AI_CHN_PARAM_S));
		} else
			RPC_AUD_ERR(RPC_CMD_AI_GET_CHN_PARAM, __func__, __LINE__);

		break;
	case RPC_CMD_AI_SET_VOLUME:
		break;
	case RPC_CMD_AI_GET_VOLUME:
		if (args != NULL) {
		CVI_S32 *ps32VolumeStep = (CVI_S32 *)args;

		memcpy(ps32VolumeStep,  pRpcMsg->body, sizeof(CVI_S32));
		} else
			RPC_AUD_ERR(RPC_CMD_AI_GET_VOLUME, __func__, __LINE__);
		break;
	case RPC_CMD_AI_ENABLE_VQE:
		break;
	case RPC_CMD_AI_DISABLE_VQE:
		break;
	case RPC_CMD_AI_ENABLE_RESMP:
		break;
	case RPC_CMD_AI_DISABLE_RESMP:
		break;
	case RPC_CMD_AI_CLR_PUB_ATTR:
		break;
	case RPC_CMD_AI_SET_TALK_VQE_ATTR:
		break;
	case RPC_CMD_AI_GET_TALK_VQE_ATTR:
		if (args != NULL) {
			pstVqeConfig = (AI_TALKVQE_CONFIG_S *)args;
			memcpy(pstVqeConfig, pRpcMsg->body, sizeof(AI_TALKVQE_CONFIG_S));
		} else
			RPC_AUD_ERR(RPC_CMD_AI_GET_TALK_VQE_ATTR, __func__, __LINE__);
		break;
	case RPC_CMD_AO_SET_PUB_ATTR:
		break;
	case RPC_CMD_AO_GET_PUB_ATTR:
		if (args != NULL) {
			pstAttr = (AIO_ATTR_S *)args;
			memcpy(pstAttr, pRpcMsg->body, sizeof(AIO_ATTR_S));
		} else
			RPC_AUD_ERR(RPC_CMD_AO_GET_PUB_ATTR, __func__, __LINE__);
		break;
	case RPC_CMD_AO_ENABLE:
		break;
	case RPC_CMD_AO_DISABLE:
		break;
	case RPC_CMD_AO_ENABLE_CHN:
		break;
	case RPC_CMD_AO_DISABLE_CHN:
		break;
	case RPC_CMD_AO_SEND_FRAME:
		break;
	case RPC_CMD_AO_ENABLE_RESMP:
		break;
	case RPC_CMD_AO_DISABLE_RESMP:
		break;
	case RPC_CMD_AO_SET_VOLUME:
		break;
	case RPC_CMD_AO_GET_VOLUME:
		if (args != NULL) {
		CVI_S32 *ps32VolumeDb = (CVI_S32 *)args;

		memcpy(ps32VolumeDb,  pRpcMsg->body, sizeof(CVI_S32));
		} else
			RPC_AUD_ERR(RPC_CMD_AO_GET_VOLUME, __func__, __LINE__);
		break;
	case RPC_CMD_AO_SET_MUTE:
		break;
	case RPC_CMD_AO_GET_MUTE:
		if (args != NULL) {
			pChar = (char *)args;
			CVI_S32 s32Size = sizeof(CVI_S32) + sizeof(AUDIO_FADE_S);

			memcpy(pChar,  pRpcMsg->body, s32Size);
		} else {
			RPC_AUD_ERR(cmd, __func__, __LINE__);
		}
		break;
	case RPC_CMD_AENC_CREATE_CHN:
		break;
	case RPC_CMD_AENC_DESTROY_CHN:
		break;
	case PRC_CMD_AENC_SEND_FRAME:
		break;
	case RPC_CMD_AENC_GET_STREAM:
#ifndef SHARE_MEM_IN_AUDIO_RPC
		if (args != NULL) {
			pstStream = (AUDIO_STREAM_S *)args;
			memcpy(pstStream,  pRpcMsg->body, sizeof(AUDIO_STREAM_S));
			//memcpy(pstStream->pStream, (pRpcMsg->body + sizeof(AUDIO_STREAM_S)), pstStream->u32Len);
			memcpy(RpcEncBuff, (pRpcMsg->body + sizeof(AUDIO_STREAM_S)), pstStream->u32Len);
			pstStream->pStream = RpcEncBuff;

		} else {
			RPC_AUD_ERR(cmd, __func__, __LINE__);
		}
#endif
		break;
	case RPC_CMD_AENC_RELEASE_STREAM:
		if (args != NULL) {
			pstStream = (AUDIO_STREAM_S *)args;
			memcpy(pstStream,  pRpcMsg->body, sizeof(AUDIO_STREAM_S));
		} else {
			//RPC_AUD_ERR(cmd, __func__, __LINE__);
		}
		break;
	case RPC_CMD_AENC_REGISTER_EXTERNAL_ENCODER:
		break;
	case RPC_CMD_AENC_UNREGISTER_EXTERNAL_ENCODER:
		break;
	case RPC_CMD_ADEC_CREATE_CHN:
		break;
	case RPC_CMD_ADEC_DESTROY_CHN:
		break;
	case RPC_CMD_ADEC_SEND_STREAM:
		break;
	case RPC_CMD_ADEC_REGISTER_EXTERNAL_DECODER:
		break;
	case RPC_CMD_ADEC_UNREGISTER_EXTERNAL_DECODER:
		break;
	case RPC_CMD_ADEC_GET_FRAME:
		if (args != NULL) {
			AUDIO_FRAME_INFO_S stlocalFrameInfo;

			pstFrmInfo = (AUDIO_FRAME_INFO_S *)args;
			memcpy(&stlocalFrameInfo, pRpcMsg->body, sizeof(AUDIO_FRAME_INFO_S));
			pstFrmInfo->u32Id = stlocalFrameInfo.u32Id;
			if (pstFrmInfo->pstFrame != NULL) {
				memcpy(pstFrmInfo->pstFrame,
					   pRpcMsg->body + sizeof(AUDIO_FRAME_INFO_S),
					   sizeof(AUDIO_FRAME_S));
			} else
				printf("Warning ADEC_GET_FRAME pstFrame == NULL\n");

			sizebytes = pstFrmInfo->pstFrame->u32Len * BYTES_PER_AUD_SAMPLE *
					(pstFrmInfo->pstFrame->enSoundmode + 1);
			memcpy(RpcDecBuff,
				   (pRpcMsg->body + sizeof(AUDIO_FRAME_INFO_S) + sizeof(AUDIO_FRAME_S))
				   , sizebytes);
			pstFrmInfo->pstFrame->u64VirAddr[0] = RpcDecBuff;
		} else {
			printf("[error][%s][%d]args == NULL\n", __func__, __LINE__);
		}
		break;
	case RPC_CMD_ADEC_RELEASE_FRAME:
		if (args != NULL) {
			pstFrmInfo = (AUDIO_FRAME_INFO_S *)args;
			memcpy(pstFrmInfo,  pRpcMsg->body, sizeof(AUDIO_FRAME_INFO_S));
		}
		break;
	case RPC_CMD_ADEC_SEND_END_OF_STREAM:
		break;

	default:
		printf("[ERROR]cmd none[%d]\n", cmd);
		break;
	}

	return ret;
}

int rpc_aud_client_send(unsigned char *buf, int size, int s32MilliSec,
			char **response, int *responselen)
{
	int fd = 1;
	int rc;
	int mode;
	int chn;
	int index;

	pthread_mutex_lock(&g_rpc_init_lock);
	struct rpc_aud_msg *pstRpcMsg = (struct rpc_aud_msg *)buf;
	char const *rpc_url_all[] = {RPC_URL_AUDCH0, RPC_URL_AUDCH1, RPC_URL_AUDCH2,
				RPC_URL_AUDAIO_CH1, RPC_URL_AUDAIO_CH2};
	char const *url = rpc_url_all[0];
	AUD_RPC_UNUSED(s32MilliSec);

	mode = GET_MODE(pstRpcMsg->msg);
	chn = GET_CHN(pstRpcMsg->msg);
	chn = chn;

	if (mode == MODE_AUDIO_AIO) {

		//setup the rpc client server fd by channel number in AIO
		//available one device for 3 channels, if want more, please add yourself
		if (chn == 0) {
			url = rpc_url_all[AUD_RPC_URL_AIO];
			fd = g_audio_rpc_fd[RPC_FD_AUD_AIO];
			index = 0;
		} else if (chn == 1) {
			url = rpc_url_all[AUD_RPC_URL_AIO_CH1];
			fd = g_audio_rpc_fd[RPC_FD_AUD_AIO_CH1];
			index = 3;
		} else if (chn == 2) {
			url = rpc_url_all[AUD_RPC_URL_AIO_CH2];
			fd = g_audio_rpc_fd[RPC_FD_AUD_AIO_CH2];
			index = 4;
		}

	} else if (mode == MODE_AUDIO_AENC) {
		url = rpc_url_all[AUD_RPC_URL_AENC];
		fd = g_audio_rpc_fd[RPC_FD_AUD_AENC];
		index = 1;
	} else if (mode == MODE_AUDIO_ADEC) {
		url = rpc_url_all[AUD_RPC_URL_ADEC];
		fd = g_audio_rpc_fd[RPC_FD_AUD_ADEC];
		index = 2;
	} else {
		url = rpc_url_all[AUD_RPC_URL_AIO];
		fd = g_audio_rpc_fd[RPC_FD_AUD_AIO];
		index = 0;
		printf("[Error]audio Mode error in [%s][%d]_[%d]\n",
			__func__, __LINE__, mode);
	}

	if (fd < 0) {
		fd = nn_socket(AF_SP, NN_REQ);
		if (fd < 0) {
			fprintf(stderr, "nn_socket: %s\n", nn_strerror(nn_errno()));
			pthread_mutex_unlock(&g_rpc_init_lock);
			return (-1);
		}

		if (nn_connect(fd, url) < 0) {
			fprintf(stderr, "nn_socket: %s\n", nn_strerror(nn_errno()));
			nn_close(fd);
			pthread_mutex_unlock(&g_rpc_init_lock);
			return (-1);
		}
		g_audio_rpc_fd[index] = fd;
		usleep(1000);
	}

	if (nn_send(fd, buf, size, 0) < 0) {
		fprintf(stderr, "chn[%d] nn_send: %s\n", chn, nn_strerror(nn_errno()));
		nn_close(fd);
		pthread_mutex_unlock(&g_rpc_init_lock);
		return (-1);
	}
	rc = nn_recv(fd, response, NN_MSG, 0);
	if (rc < 0) {
		fprintf(stderr, "chn[%d]nn_recv: %s\n", chn, nn_strerror(nn_errno()));
		fprintf(stderr, "fd[%d] url:[%s]response: %s\n", fd, url, *response);
		nn_close(fd);
		pthread_mutex_unlock(&g_rpc_init_lock);
		return (-1);
	}

	*responselen = rc;

	pthread_mutex_unlock(&g_rpc_init_lock);
	return 0;
}

static CVI_S32 aud_client_common(CVI_S32 mode,
				 CVI_S32 devId,
				 CVI_S32 chnId,
				 CVI_S32 cmd,
				 CVI_S32 add_size,
				 char *add_data,
				 void *pGetVal)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	CVI_S32 bufsize = 0;
	unsigned int msg = SET_MSG(mode,
				   devId,
				   chnId,
				   cmd);

	bufsize = sizeof(struct rpc_aud_msg) + add_size;

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("[Error]malloc failed [%d]\n", bufsize);
		return CVI_FAILURE;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	if (add_size != 0 && (add_data != NULL)) {
		memcpy(buf + sizeof(struct rpc_aud_msg), add_data, add_size);
	}

	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("[Error]rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, pGetVal);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			__func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
	return s32Ret;
}
//implement server/client response function------start
CVI_S32 rpc_client_aud_ao_get_mute(AUDIO_DEV AoDevId, CVI_BOOL *pbEnable,
				   AUDIO_FADE_S *pstFade)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	char *pChar = malloc(sizeof(CVI_S32) + sizeof(AUDIO_FADE_S));
	CVI_S32 s32Enable;
	AUDIO_FADE_S stFade;

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AoDevId,
				   DEFAULT_AUD_CHN_ID,
				   RPC_CMD_AO_GET_MUTE,
				   0,
				   NULL,
				   (void *)pChar);
	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AO_GET_MUTE, __func__, __LINE__);
		free(pChar);
		return CVI_FAILURE;
	}

	memcpy(&s32Enable, pChar, sizeof(CVI_S32));
	if (s32Enable == 1)
		*pbEnable = CVI_TRUE;
	else
		*pbEnable = CVI_FALSE;


	memcpy(&stFade, (pChar + sizeof(CVI_S32)), sizeof(AUDIO_FADE_S));
	*pstFade = stFade;

	free(pChar);
	return s32Ret;
}

CVI_S32 rpc_client_aud_ao_get_volume(AUDIO_DEV AoDevId, CVI_S32 *ps32VolumeDb)
{
	// x_info("enter");
	CVI_S32 s32Ret;

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AoDevId,
				   DEFAULT_AUD_CHN_ID,
				   RPC_CMD_AO_GET_VOLUME,
				   0,
				   NULL,
				   (void *)ps32VolumeDb);

	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AO_GET_VOLUME, __func__, __LINE__);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 rpc_client_aud_ai_get_volume(AUDIO_DEV AiDevId, CVI_S32 *ps32VolumeStep)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_AIO,
				   AiDevId,
				   DEFAULT_AUD_CHN_ID,
				   RPC_CMD_AI_GET_VOLUME);
	CVI_S32 bufsize = sizeof(struct rpc_aud_msg) + sizeof(CVI_S32);

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(buf + sizeof(struct rpc_aud_msg), ps32VolumeStep, sizeof(CVI_S32));

	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, ps32VolumeStep);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
	return s32Ret;
}

CVI_S32 rpc_client_aud_ao_set_volume(AUDIO_DEV AoDevId, CVI_S32 s32VolumeDb)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32AoSetVolume = s32VolumeDb;

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AoDevId,
				   DEFAULT_AUD_CHN_ID,
				   RPC_CMD_AO_SET_VOLUME,
				   sizeof(CVI_S32),
				   (char *)&s32AoSetVolume,
				   NULL);
	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AO_SET_VOLUME, __func__, __LINE__);
		return CVI_FAILURE;
	}

	return s32Ret;
}


CVI_S32 rpc_client_aud_ao_set_mute(AUDIO_DEV AoDevId, CVI_BOOL bEnable,
				   const AUDIO_FADE_S *pstFade)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32InputSize = sizeof(CVI_S32) + sizeof(AUDIO_FADE_S);
	char *InputBuf = malloc(s32InputSize);
	CVI_S32 s32Enable = 0;

	if (bEnable == CVI_TRUE)
		s32Enable = 1;
	else
		s32Enable = 0;

	memcpy(InputBuf, &s32Enable, sizeof(CVI_S32));
	if (pstFade != NULL)
		memcpy(InputBuf + sizeof(CVI_S32), pstFade, sizeof(AUDIO_FADE_S));
	else
		memset(InputBuf + sizeof(CVI_S32), 0, sizeof(AUDIO_FADE_S));

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AoDevId,
				   DEFAULT_AUD_CHN_ID,
				   RPC_CMD_AO_SET_MUTE,
				   s32InputSize,
				   (char *)InputBuf,
				   NULL);
	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AO_SET_MUTE, __func__, __LINE__);
		free(InputBuf);
		return CVI_FAILURE;
	}

	free(InputBuf);
	return s32Ret;
}


CVI_S32 rpc_client_aud_ai_set_volume(AUDIO_DEV AiDevId, CVI_S32 s32VolumeStep)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_AIO,
				   AiDevId,
				   DEFAULT_AUD_CHN_ID,
				   RPC_CMD_AI_SET_VOLUME);
	CVI_S32 bufsize = sizeof(struct rpc_aud_msg) + sizeof(CVI_S32);

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(buf + sizeof(struct rpc_aud_msg), &s32VolumeStep, sizeof(CVI_S32));

	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, NULL);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
	return s32Ret;
}

CVI_S32 rpc_client_aud_adec_destroy_chn(ADEC_CHN AdChn)
{
	// x_info("enter");
	CVI_S32 s32Ret;

#ifdef SHARE_MEM_IN_AUDIO_RPC
	deinit_adec_share_memory(AdChn);
#endif
	s32Ret = aud_client_common(MODE_AUDIO_ADEC,
				   DEFAULT_AUD_DEV_ID,
				   AdChn,
				   RPC_CMD_ADEC_DESTROY_CHN,
				   0,
				   NULL,
				   NULL);
	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_ADEC_DESTROY_CHN, __func__, __LINE__);
		return CVI_FAILURE;
	}
	return s32Ret;
}


CVI_S32 rpc_client_aud_adec_send_end_of_stream(ADEC_CHN AdChn,
		CVI_BOOL bInstant)
{
	CVI_S32 s32Ret;
	CVI_S32 s32Instant = 0;

	if (bInstant == CVI_TRUE)
		s32Instant = 1;
	else
		s32Instant = 0;

	s32Ret = aud_client_common(MODE_AUDIO_ADEC,
				   DEFAULT_AUD_DEV_ID,
				   AdChn,
				   RPC_CMD_ADEC_SEND_END_OF_STREAM,
				   sizeof(CVI_S32),
				   (char *)&s32Instant,
				   NULL);
	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_ADEC_SEND_END_OF_STREAM, __func__, __LINE__);
		return CVI_FAILURE;
	}
	return s32Ret;
}

CVI_S32 rpc_client_aud_adec_create_chn(ADEC_CHN AdChn,
					   const ADEC_CHN_ATTR_S *pstAttr)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	ADEC_CHN_ATTR_S stAdecChnAttr;

	memcpy(&stAdecChnAttr, pstAttr, sizeof(ADEC_CHN_ATTR_S));
	s32Ret = aud_client_common(MODE_AUDIO_ADEC,
				   DEFAULT_AUD_DEV_ID,
				   AdChn,
				   RPC_CMD_ADEC_CREATE_CHN,
				   sizeof(ADEC_CHN_ATTR_S),
				   (char *)&stAdecChnAttr,
				   NULL);
	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_ADEC_CREATE_CHN, __func__, __LINE__);
		return CVI_FAILURE;
	}
	return s32Ret;
}

CVI_S32 rpc_client_aud_adec_release_frame(ADEC_CHN AdChn,
		const AUDIO_FRAME_INFO_S *pstFrmInfo)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_ADEC,
				   DEFAULT_AUD_DEV_ID,
				   AdChn,
				   RPC_CMD_ADEC_RELEASE_FRAME);
	CVI_S32 bufsize = sizeof(struct rpc_aud_msg) + sizeof(AUDIO_FRAME_INFO_S);

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(buf + sizeof(struct rpc_aud_msg), pstFrmInfo,
		   sizeof(AUDIO_FRAME_INFO_S));

	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, NULL);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
	return s32Ret;
}

CVI_S32 rpc_client_aud_adec_get_frame(ADEC_CHN AdChn,
					  AUDIO_FRAME_INFO_S *pstFrmInfo,
					  CVI_BOOL bBlock)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	CVI_S32 s32Block;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_ADEC,
				   DEFAULT_AUD_DEV_ID,
				   AdChn,
				   RPC_CMD_ADEC_GET_FRAME);
	CVI_S32 bufsize = sizeof(struct rpc_aud_msg) + sizeof(CVI_S32);

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	if (bBlock == CVI_TRUE)
		s32Block = 1;
	else
		s32Block = 0;

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(buf + sizeof(struct rpc_aud_msg), &s32Block, sizeof(CVI_S32));

	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, pstFrmInfo);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
	return s32Ret;
}



CVI_S32 rpc_client_aud_aenc_send_frame(AENC_CHN AeChn,
					   const AUDIO_FRAME_S *pstFrm,
					   const AEC_FRAME_S *pstAecFrm)
{

	// x_info("enter");
	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_AENC,
				   DEFAULT_AUD_DEV_ID,
				   AeChn,
				   PRC_CMD_AENC_SEND_FRAME);
	CVI_S32 bufsize = 0;
	CVI_S32 s32AudDataSizeBytes = pstFrm->u32Len * BYTES_PER_AUD_SAMPLE *
					  (pstFrm->enSoundmode + 1);
	CVI_S32 s32AudFrmSize = sizeof(AUDIO_FRAME_S) + s32AudDataSizeBytes;
	CVI_S32 s32AecFrmSize = 0;
	const AUDIO_FRAME_S *pstRefFrame;

	if (pstAecFrm != NULL) {
		pstRefFrame = &pstAecFrm->stRefFrame;
		if (pstRefFrame != NULL)
			s32AecFrmSize = sizeof(AEC_FRAME_S) + pstRefFrame->u32Len *
					BYTES_PER_AUD_SAMPLE;
		else
			s32AecFrmSize = 0;
	} else
		s32AecFrmSize = 0;

	bufsize = sizeof(struct rpc_aud_msg) + s32AudFrmSize + s32AecFrmSize;

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = bufsize;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(buf + sizeof(struct rpc_aud_msg), pstFrm, sizeof(AUDIO_FRAME_S));
	memcpy(buf + sizeof(struct rpc_aud_msg) + sizeof(AUDIO_FRAME_S),
		   pstFrm->u64VirAddr[0],
		   s32AudDataSizeBytes);
	if (s32AecFrmSize != 0) {
		memcpy(buf + sizeof(struct rpc_aud_msg) + sizeof(AUDIO_FRAME_S) +
			   s32AudDataSizeBytes,
			   pstRefFrame,
			   sizeof(AEC_FRAME_S));
		if (pstRefFrame != NULL) {
			unsigned char *pt = buf + s32AudFrmSize;

			pt = pt + sizeof(AEC_FRAME_S);
			memcpy(buf, pstRefFrame->u64VirAddr[0],
				   pstRefFrame->u32Len * BYTES_PER_AUD_SAMPLE);
		}
	}


	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, NULL);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
	return s32Ret;
}
#ifdef SHARE_MEM_IN_AUDIO_RPC
CVI_S32 rpc_client_aud_aenc_release_stream(AENC_CHN AeChn,
				const AUDIO_STREAM_S *pstStream)
{

	CVI_S32 s32Ret = 0;
	//do not release stream in client while using shared memory
	AUD_RPC_UNUSED(AeChn);
	AUD_RPC_UNUSED(pstStream);
	return s32Ret;
}

CVI_S32 rpc_client_aud_aenc_get_stream(AENC_CHN AeChn,
			AUDIO_STREAM_S *pstStream,
			   CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	char *p_shm = NULL;
	int read_suc = 0;
	AUD_AENC_STREAM_CBLK stAudStreamHead;

	//static CVI_U64 u64LastTime[3] = {0};
	//static CVI_U64 u64NowTime[3] = {0};
	//CVI_U64 delayTime = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_AENC,
				DEFAULT_AUD_DEV_ID,
				AeChn,
				RPC_CMD_AENC_GET_STREAM);
	CVI_S32 bufsize = sizeof(struct rpc_aud_msg) + sizeof(CVI_S32);

	if (!stAencStreamShmInfo[AeChn].bInit) {

		buf = malloc(bufsize);
		if (buf == NULL) {
			printf("malloc failed\n");
			return -1;
		}

		stRpcMsg.msg = msg;
		stRpcMsg.magic = AUD_RPC_MAGIC;
		stRpcMsg.result = 0;
		stRpcMsg.body_len = 0;
		memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
		memcpy(buf + sizeof(struct rpc_aud_msg), &s32MilliSec, sizeof(CVI_S32));

		s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
		if (s32Ret != 0) {
			printf("rpc_send failed\n");
			free(buf);
			RPC_AUD_ERR(RPC_CMD_AENC_GET_STREAM, __func__, __LINE__);
			return s32Ret;
		}

		s32Ret = rpc_aud_client_response_proc(response, responselen, NULL);//
		if (s32Ret != 0) {
			printf("[%s][%d]rpc_response_proc failed with %#x\n",
				__func__, __LINE__, s32Ret);
			RPC_AUD_ERR(RPC_CMD_AENC_GET_STREAM, __func__, __LINE__);
		}
		free(buf);
		nn_freemsg(response);
	}
	//create shm and sem

	if (!stAencStreamShmInfo[AeChn].bInit) {
		printf("init_aenc_share_memory\n");
		init_aenc_share_memory(AeChn);
		stAencStreamShmInfo[AeChn].bRuning = 1;
	}
	if (!stAencStreamShmInfo[AeChn].bInit) {
		RPC_AUD_ERR(RPC_CMD_AENC_GET_STREAM, __func__, __LINE__);
		return -1;
	}

	while (stAencStreamShmInfo[AeChn].bRuning) {

		p_shm =	stAencStreamShmInfo[AeChn].g_shm_ptr +
			(stAencStreamShmInfo[AeChn].m_queue_index % stAencStreamShmInfo[AeChn].m_queue_cap) *
			RPC_AUD_STREAM_ONEFRMAE_MAX_SIZE;
		memset(&stAudStreamHead, 0, sizeof(AUD_AENC_STREAM_CBLK));
		sem_wait(stAencStreamShmInfo[AeChn].m_pSemRead);
		memcpy(&stAudStreamHead, p_shm, sizeof(AUD_AENC_STREAM_CBLK)); //read HEAD

		if (checkIsAencStreamHead(&stAudStreamHead)) {
			stAencStreamShmInfo[AeChn].m_queue_index++;
			//u64NowTime[AeChn] = rpc_aud_get_boot_time();
			//delayTime = u64NowTime[AeChn] - stAudStreamHead.u64TimeStamp;
			//printf("[aenc]client chn:%d delay:%llu,getstream delta:%llu,size:%u, m_queue_index:%d\n",
			//	AeChn, delayTime, u64NowTime[AeChn] - u64LastTime[AeChn],
			//	stAudStreamHead.u32Len, stAencStreamShmInfo[AeChn].m_queue_index);

			//u64LastTime[AeChn] = u64NowTime[AeChn];
			memcpy(stAencStreamShmInfo[AeChn].streamPtr,
				p_shm + sizeof(AUD_AENC_STREAM_CBLK),
				stAudStreamHead.u32Len);
			pstStream->u32Len = stAudStreamHead.u32Len;
			pstStream->u32Seq = stAudStreamHead.u32Seq;
			pstStream->u64TimeStamp = stAudStreamHead.u64TimeStamp;
			pstStream->pStream = stAencStreamShmInfo[AeChn].streamPtr;
			read_suc = 1;
			memset(p_shm, 0, sizeof(AUD_AENC_STREAM_CBLK));
			sem_post(stAencStreamShmInfo[AeChn].m_pSemWrite);
			break;
		}

	}

	if (read_suc)
		return 0;
	else
		return -1;

}


void *Aud_stream_process_thread(void *args)
{
	AUD_STREAM_SHARE_MEM_INFO *pVencShmInfo = (AUD_STREAM_SHARE_MEM_INFO *)args;
	CVI_CHAR TaskName[64];
	AENC_CHN AeChn = pVencShmInfo->channel;
	CVI_S32 s32Ret;
	ssize_t writelen = 0;

	sprintf(TaskName, "chn%dAudStream", AeChn);
	prctl(PR_SET_NAME, TaskName, 0, 0, 0);
	printf("server venc task%d start\n", AeChn);
	AUD_AENC_STREAM_CBLK stAudStreamHead;
	char *shm_ptr = stAencStreamShmInfo[AeChn].g_shm_ptr;

	if (!shm_ptr) {
		printf("shm is NULL\n");
		return NULL;
	}
	//(char*) shm_ptr = 0;
	memset(shm_ptr, 0, sizeof(AUD_AENC_STREAM_CBLK));
	stAudStreamHead.s32SyncCode = 0xa5a5;

	for (int j = 0; j < stAencStreamShmInfo[AeChn].m_queue_cap; j++)
		sem_post(stAencStreamShmInfo[AeChn].m_pSemWrite);

	while (stAencStreamShmInfo[AeChn].bRuning) {
		AUDIO_STREAM_S stStream;

		memset(&stStream, 0, sizeof(AUDIO_STREAM_S));
		s32Ret = CVI_AENC_GetStream(AeChn, &stStream, 0);
		if (s32Ret != CVI_SUCCESS) {
			printf("[error],[%s],[line:%d],\n", __func__, __LINE__);
			break;
		}
		if (!stStream.u32Len) {
			usleep(10*1000);
			continue;
		}
		sem_wait(stAencStreamShmInfo[AeChn].m_pSemWrite);
		shm_ptr = stAencStreamShmInfo[AeChn].g_shm_ptr +
			(stAencStreamShmInfo[AeChn].m_queue_index % stAencStreamShmInfo[AeChn].m_queue_cap) *
			RPC_AUD_STREAM_ONEFRMAE_MAX_SIZE;
		stAencStreamShmInfo[AeChn].m_queue_index++;
		memset(shm_ptr, 0, sizeof(AUD_AENC_STREAM_CBLK));
		writelen = sizeof(AUD_AENC_STREAM_CBLK);
		memcpy(shm_ptr + writelen, stStream.pStream, stStream.u32Len);
		stAudStreamHead.u32Len = stStream.u32Len;
		stAudStreamHead.u32Seq = stStream.u32Seq;
		stAudStreamHead.u64TimeStamp = stStream.u64TimeStamp;
		stAudStreamHead.u64PhyAddr = stStream.u64PhyAddr;

		memcpy(shm_ptr, &stAudStreamHead, sizeof(AUD_AENC_STREAM_CBLK));
		sem_post(stAencStreamShmInfo[AeChn].m_pSemRead);

	}

	printf("aenc_stream_process_thread exit\n");
	return NULL;
}



CVI_S32 rpc_server_aud_aenc_get_stream(struct rpc_aud_msg *pstRpcMsg,
		unsigned char *response,
		int *responselen)
{
	AENC_CHN AeChn = GET_CHN(pstRpcMsg->msg);

	AUD_RPC_UNUSED(response);
	*responselen = sizeof(struct rpc_aud_msg);
	pstRpcMsg->body_len = (*responselen);
	pstRpcMsg->result = CVI_SUCCESS;
	*responselen = *responselen;
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));

	if (stAencStreamShmInfo[AeChn].bRuning)
		return 0;

	printf("rpc_server_aud_aenc_get_stream\n");
	init_aenc_share_memory(AeChn);

	stAencStreamShmInfo[AeChn].bRuning = 1;
	struct sched_param param;
	pthread_attr_t attr;

	param.sched_priority = 75;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(&s_pthAudStream[AeChn], &attr, Aud_stream_process_thread, (void *)&stAencStreamShmInfo[AeChn]);
	return 0;
}

void *Ai_stream_process_thread(void *args)
{

	AI_SHARE_MEM_INFO *pAiShmInfo = (AI_SHARE_MEM_INFO *)args;
	char Task_name[64];
	AI_CHN AiChn = pAiShmInfo->Aichn;
	AUDIO_DEV AiDev = pAiShmInfo->AiDev;
	int s32Ret;

	sprintf(Task_name, "chn%dAiStream", AiChn);
	prctl(PR_SET_NAME, Task_name, 0, 0, 0);
	printf("server ai task%d start\n", AiChn);

	AUD_AIN_FRAME_CBLK stAiStreamHead;
	char *shm_ptr = stAinStreamShmInfo[AiChn].g_shm_ptr;

	if (!shm_ptr) {
		printf("shm is NULL\n");
		return NULL;
	}
	memset(shm_ptr, 0, sizeof(AUD_AIN_FRAME_CBLK));
	stAiStreamHead.s32SyncCode = 0xa1a1;

	for (int j = 0; j < stAinStreamShmInfo[AiChn].m_queue_cap; j++)
		sem_post(stAinStreamShmInfo[AiChn].m_pSemWrite);

	while (stAinStreamShmInfo[AiChn].bRuning) {
		AUDIO_FRAME_S stFrm;
		AEC_FRAME_S stAecFrm;
		int writelen = 0;

		memset(&stAecFrm, 0, sizeof(AEC_FRAME_S));
		memset(&stFrm, 0, sizeof(AUDIO_FRAME_S));

		s32Ret = CVI_AI_GetFrame(AiDev, AiChn,
					 &stFrm,
					 &stAecFrm,
					 AIN_GET_FRAME_FORCE_BLOCK);

		if (s32Ret != CVI_SUCCESS) {
			printf("[error],[%s],[line:%d],\n", __func__, __LINE__);
			break;
		}
		if (!stFrm.u32Len) {
			usleep(10 * 1000);
			continue;
		}
		sem_wait(stAinStreamShmInfo[AiChn].m_pSemWrite);

		shm_ptr = stAinStreamShmInfo[AiChn].g_shm_ptr +
			  (stAinStreamShmInfo[AiChn].m_queue_index % stAinStreamShmInfo[AiChn].m_queue_cap)
			  * RPC_AUD_STREAM_ONEFRMAE_MAX_SIZE;
		stAinStreamShmInfo[AiChn].m_queue_index++;
		memset(shm_ptr, 0, sizeof(AUD_AIN_FRAME_CBLK));
		writelen = sizeof(AUD_AIN_FRAME_CBLK);

		memcpy(shm_ptr + writelen, stFrm.u64VirAddr[0],
			   stFrm.u32Len * BYTES_PER_AUD_SAMPLE * (stFrm.enSoundmode + 1));
		stAiStreamHead.enSoundmode = stFrm.enSoundmode;
		stAiStreamHead.u32Len = stFrm.u32Len;
		stAiStreamHead.u32Seq = stFrm.u32Seq;
		stAiStreamHead.u64TimeStamp = stFrm.u64TimeStamp;
		stAiStreamHead.u64PhyAddr = stFrm.u64PhyAddr[0];
		memcpy(shm_ptr, &stAiStreamHead, sizeof(AUD_AIN_FRAME_CBLK));
		sem_post(stAinStreamShmInfo[AiChn].m_pSemRead);
	}

	printf("ai_stream_process_thread exit\n");


	return NULL;
}

CVI_S32 rpc_client_aud_ai_get_frame(AUDIO_DEV AiDevId, AI_CHN AiChn,
					AUDIO_FRAME_S *pstFrm, AEC_FRAME_S *pstAecFrm,
					CVI_S32 s32MilliSec)
{


	int s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	char *p_shm = NULL;
	int read_suc = 0;
	AUD_AIN_FRAME_CBLK stAiStreamHead;
	//static CVI_U64 u64LastTime[3] = {0};
	//static CVI_U64 u64NowTime[3] = {0};
	//CVI_U64 delayTime = 0;
	int read_byte = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_AIO,
				   AiDevId,
				   AiChn,
				   RPC_CMD_AI_GET_FRAME);
	CVI_S32 bufsize = sizeof(struct rpc_aud_msg) + sizeof(CVI_S32);

	AUD_RPC_UNUSED(pstAecFrm);
	if (!stAinStreamShmInfo[AiChn].bInit) {
		buf = malloc(bufsize);
		if (buf == NULL) {
			printf("malloc failed\n");
			return -1;
		}

		stRpcMsg.msg = msg;
		stRpcMsg.magic = AUD_RPC_MAGIC;
		stRpcMsg.result = 0;
		stRpcMsg.body_len = 0;
		memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
		memcpy(buf + sizeof(struct rpc_aud_msg), &s32MilliSec, sizeof(CVI_S32));

		s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
		if (s32Ret != 0) {
			printf("rpc_send failed\n");
			free(buf);
			return s32Ret;
		}

		s32Ret = rpc_aud_client_response_proc(response, responselen, pstFrm);
		if (s32Ret != 0) {
			printf("[%s][%d]rpc_response_proc failed with %#x\n",
				   __func__, __LINE__, s32Ret);
		}

		free(buf);
		nn_freemsg(response);
	}

	if (!stAinStreamShmInfo[AiChn].bInit) {
		printf("[client] init_ain_share_memory AiChn:%d\n", AiChn);
		init_ain_share_memory(AiChn);
		stAinStreamShmInfo[AiChn].bRuning = 1;

	}

	if (!stAinStreamShmInfo[AiChn].bInit) {
		return -1;
	}

	while (stAinStreamShmInfo[AiChn].bRuning) {
		p_shm = stAinStreamShmInfo[AiChn].g_shm_ptr +
			(stAinStreamShmInfo[AiChn].m_queue_index % stAinStreamShmInfo[AiChn].m_queue_cap)
			* RPC_AUD_STREAM_ONEFRMAE_MAX_SIZE;

		memset(&stAiStreamHead, 0, sizeof(AUD_AIN_FRAME_CBLK));
		sem_wait(stAinStreamShmInfo[AiChn].m_pSemRead);
		memcpy(&stAiStreamHead, p_shm, sizeof(AUD_AIN_FRAME_CBLK));

		if (checkIsAinFrameHead(&stAiStreamHead)) {
			stAinStreamShmInfo[AiChn].m_queue_index++;
			//u64NowTime[AiChn] = rpc_aud_get_boot_time();
			//delayTime = u64NowTime[AiChn] - stAiStreamHead.u64TimeStamp;

			//printf("[ai]client getstream time aud chn:%d delay:%llu,getstream delta:%llu,size:%u\n",
			//       AiChn, delayTime, u64NowTime[AiChn] - u64LastTime[AiChn],
			//       stAiStreamHead.u32Len);

			//u64LastTime[AiChn] = u64NowTime[AiChn];
			memcpy(stAinStreamShmInfo[AiChn].streamPtr, p_shm + sizeof(AUD_AIN_FRAME_CBLK),
				   stAiStreamHead.u32Len * BYTES_PER_AUD_SAMPLE * (stAiStreamHead.enSoundmode +
						   1));
			read_suc = 1;
			memset(p_shm, 0, sizeof(AUD_AIN_FRAME_CBLK));
			sem_post(stAinStreamShmInfo[AiChn].m_pSemWrite);
			break;
		}

	}

	if (!read_suc) {
		printf("read failure[%s][%d]\n",  __func__, __LINE__);
		return -1;
	}

	pstFrm->u32Len = stAiStreamHead.u32Len;
	pstFrm->u64TimeStamp = stAiStreamHead.u64TimeStamp;
	pstFrm->enSoundmode = stAiStreamHead.enSoundmode;
	read_byte = pstFrm->u32Len * BYTES_PER_AUD_SAMPLE * (pstFrm->enSoundmode + 1);
	if (read_byte > RPC_BUFFER_BYTE) {
		printf("[%s][%d] read_byte:%d > bufferlen:%d\n",
			__func__, __LINE__, read_byte, RPC_BUFFER_BYTE);
		return -1;
	}
	memcpy(RpcAinGetBuff, stAinStreamShmInfo[AiChn].streamPtr, read_byte);
	pstFrm->u64VirAddr[0] = RpcAinGetBuff;

	return 0;
}


CVI_S32 rpc_server_aud_ai_get_frame(struct rpc_aud_msg *pstRpcMsg,
					unsigned char *response,
					int *responselen)
{
	AI_CHN AiChn = GET_CHN(pstRpcMsg->msg);
	AUDIO_DEV Aidev = GET_DEV(pstRpcMsg->msg);

	AUD_RPC_UNUSED(response);
	*responselen = sizeof(struct rpc_aud_msg);
	pstRpcMsg->body_len = (*responselen);
	pstRpcMsg->result = CVI_SUCCESS;
	*responselen = *responselen;
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));

	if (stAinStreamShmInfo[AiChn].bRuning)
		return 0;

	memset(stAinStreamShmInfo[AiChn].cAinStreamShmName, 0, MAX_RPC_AUD_STREAM_NAME_LEN);
	init_ain_share_memory(AiChn);

	stAinStreamShmInfo[AiChn].Aichn = AiChn;
	stAinStreamShmInfo[AiChn].AiDev = Aidev;
	stAinStreamShmInfo[AiChn].bRuning = 1;

	struct sched_param param;
	pthread_attr_t attr;

	param.sched_priority = 75;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	pthread_create(&s_pthAinStream[AiChn], &attr, Ai_stream_process_thread,
			   (void *)&stAinStreamShmInfo[AiChn]);

	return 0;
}



CVI_S32 rpc_client_aud_ao_send_frame(AUDIO_DEV AoDevId,
					 AO_CHN AoChn,
					 const AUDIO_FRAME_S *pstData,
					 CVI_S32 s32MilliSec)
{
	AUD_AO_FRAME_CBLK stAoStreamHead;
	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	int writelen;
	unsigned int msg = SET_MSG(MODE_AUDIO_AIO,
				   AoDevId,
				   AoChn,
				   RPC_CMD_AO_SEND_FRAME);
	CVI_S32 s32FrameBytes = pstData->u32Len * (pstData->enSoundmode + 1);

	s32FrameBytes = s32FrameBytes * BYTES_PER_AUD_SAMPLE;

	CVI_S32 bufsize = sizeof(struct rpc_aud_msg) +
			  sizeof(AUDIO_FRAME_S) + s32FrameBytes + sizeof(CVI_S32);

if (!stAoStreamShmInfo[AoChn].bInit) {


	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(buf + sizeof(struct rpc_aud_msg), pstData, sizeof(AUDIO_FRAME_S));
	memcpy(buf + sizeof(struct rpc_aud_msg) + sizeof(AUDIO_FRAME_S),
		   pstData->u64VirAddr[0],
		   s32FrameBytes);
	memcpy(buf + sizeof(struct rpc_aud_msg) + sizeof(AUDIO_FRAME_S) + s32FrameBytes,
		   &s32MilliSec,
		   sizeof(CVI_S32));

	printf("[chn:%d][%p, %d, %d]\n", AoChn, buf, bufsize, responselen);
	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("[%s]rpc_send failed\n", __func__);
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, NULL);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
}

	memset(&stAoStreamHead, 0, sizeof(AUD_AO_FRAME_CBLK));
	stAoStreamHead.s32SyncCode = 0xa2a2;
	if (stAoStreamShmInfo[AoChn].bInit != 1) {
		int icheck = 0;

		icheck = init_ao_share_memory(AoChn);

		if (icheck < 0)
			printf("[Error][%s][%d] ..init shm failure\n", __func__, __LINE__);

		stAoStreamShmInfo[AoChn].Aochn = AoChn;
		stAoStreamShmInfo[AoChn].AoDev = AoDevId;
		stAoStreamShmInfo[AoChn].bRuning = 1;

	}


	char *shm_ptr = stAoStreamShmInfo[AoChn].g_shm_ptr;

	if (!shm_ptr) {
		printf("shm is NULL\n");
		return -1;
	}

	if (stAoStreamShmInfo[AoChn].bInit) {

		sem_wait(stAoStreamShmInfo[AoChn].m_pSemWrite);

		shm_ptr = stAoStreamShmInfo[AoChn].g_shm_ptr +
			(stAoStreamShmInfo[AoChn].m_queue_index % stAoStreamShmInfo[AoChn].m_queue_cap)
			* RPC_AUD_STREAM_ONEFRMAE_MAX_SIZE;
		stAoStreamShmInfo[AoChn].m_queue_index++;
		memset(shm_ptr, 0, sizeof(AUD_AO_FRAME_CBLK));
		writelen = sizeof(AUD_AO_FRAME_CBLK);

		memcpy(shm_ptr + writelen, pstData->u64VirAddr[0],
			pstData->u32Len * BYTES_PER_AUD_SAMPLE * (pstData->enSoundmode + 1));
		stAoStreamHead.enSoundmode = pstData->enSoundmode;
		stAoStreamHead.u32Len = pstData->u32Len;
		stAoStreamHead.u32Seq = pstData->u32Seq;
		stAoStreamHead.u64TimeStamp = pstData->u64TimeStamp;
		stAoStreamHead.u64PhyAddr = pstData->u64PhyAddr[0];
		stAoStreamHead.enBitwidth = pstData->enBitwidth;
		memcpy(shm_ptr, &stAoStreamHead, sizeof(AUD_AO_FRAME_CBLK));
		sem_post(stAoStreamShmInfo[AoChn].m_pSemRead);

	}

	return 0;

}

void *Ao_stream_process_thread(void *argc)
{

	AO_SHARE_MEM_INFO *pAOShmInfo = (AO_SHARE_MEM_INFO *)argc;
	char Task_name[64];
	AI_CHN AoChn = pAOShmInfo->Aochn;
	AUDIO_DEV AoDev = pAOShmInfo->AoDev;
	int s32Ret;
	char *p_shm = NULL;
	CVI_U64 delayTime = 0;
	int read_suc = 0;
	AUDIO_FRAME_S stAudFrm;
	static CVI_U64 u64LastTime[3] = {0};
	static CVI_U64 u64NowTime[3] = {0};
	int s32FrameBytes;
	AUD_AO_FRAME_CBLK stAoStreamHead;

	sprintf(Task_name, "chn%dAoStream", AoChn);
	prctl(PR_SET_NAME, Task_name, 0, 0, 0);
	printf("server ao task chn[%d] start\n", AoChn);


	for (int j = 0; j < stAoStreamShmInfo[AoChn].m_queue_cap; j++)
		sem_post(stAoStreamShmInfo[AoChn].m_pSemWrite);


	while (stAoStreamShmInfo[AoChn].bRuning) {

		p_shm = stAoStreamShmInfo[AoChn].g_shm_ptr +
			(stAoStreamShmInfo[AoChn].m_queue_index % stAoStreamShmInfo[AoChn].m_queue_cap)
			* RPC_AUD_STREAM_ONEFRMAE_MAX_SIZE;

		memset(&stAoStreamHead, 0, sizeof(AUD_AO_FRAME_CBLK));
		sem_wait(stAoStreamShmInfo[AoChn].m_pSemRead);
		memcpy(&stAoStreamHead, p_shm, sizeof(AUD_AO_FRAME_CBLK));

		if (checkIsAoFrameHead(&stAoStreamHead)) {
			stAoStreamShmInfo[AoChn].m_queue_index++;
			u64NowTime[AoChn] = rpc_aud_get_boot_time();
			delayTime = u64NowTime[AoChn] - stAoStreamHead.u64TimeStamp;
			#if 0
			printf("client getstream time aud chn:%d delay:%llu,getstream delta:%llu,size:%u\n",
				   AoChn, delayTime, u64NowTime[AoChn] - u64LastTime[AoChn],
				   stAoStreamHead.u32Len);
			#endif
			u64LastTime[AoChn] = u64NowTime[AoChn];
			memcpy(stAoStreamShmInfo[AoChn].streamPtr, p_shm + sizeof(AUD_AO_FRAME_CBLK),
				   stAoStreamHead.u32Len * BYTES_PER_AUD_SAMPLE * (stAoStreamHead.enSoundmode +
						   1));
			read_suc = 1;
			memset(p_shm, 0, sizeof(AUD_AO_FRAME_CBLK));
			sem_post(stAoStreamShmInfo[AoChn].m_pSemWrite);

			stAudFrm.u32Len = stAoStreamHead.u32Len;
			stAudFrm.u64TimeStamp = stAoStreamHead.u64TimeStamp;
			stAudFrm.enSoundmode = stAoStreamHead.enSoundmode;
			stAudFrm.enBitwidth = stAoStreamHead.enBitwidth;
			s32FrameBytes = stAudFrm.u32Len * BYTES_PER_AUD_SAMPLE * (stAudFrm.enSoundmode + 1);
			stAudFrm.u64VirAddr[0] = (CVI_U8 *)malloc(s32FrameBytes);
			memcpy(stAudFrm.u64VirAddr[0], stAoStreamShmInfo[AoChn].streamPtr, s32FrameBytes);

			s32Ret = CVI_AO_SendFrame(AoDev, AoChn, (const AUDIO_FRAME_S *)&stAudFrm, 1000);
			if (s32Ret != CVI_SUCCESS) {
				printf("CVI_AO_SendFrame[%d][%d] failed with %#x!\n", AoDev, AoChn, s32Ret);
				printf("Break the loop here[%s][%d]\n", __func__, __LINE__);
				break;
			}

		}

	}
	if (!read_suc) {
		printf("[ao]client getstream time aud chn:%d delay:%llu,getstream delta:%llu,size:%u\n",
			   AoChn, delayTime, u64NowTime[AoChn] - u64LastTime[AoChn],
			   stAoStreamHead.u32Len);

		printf("read failure[%s][%d]\n",  __func__, __LINE__);
		return NULL;
	}

	printf("ao_stream_process_thread exit\n");
	return NULL;
}

CVI_S32 rpc_server_aud_ao_send_frame(struct rpc_aud_msg *pstRpcMsg,
					 unsigned char *response,
					 int *responselen)
{

	AO_CHN AoChn = GET_CHN(pstRpcMsg->msg);
	AUDIO_DEV AoDevId = GET_DEV(pstRpcMsg->msg);

	AUD_RPC_UNUSED(pstRpcMsg);
	*responselen = sizeof(struct rpc_aud_msg);
	*responselen = *responselen;
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));

	if (!stAoStreamShmInfo[AoChn].bInit) {
		printf("init ao share memory for chan[%d] dev[%d]\n", AoChn, AoDevId);
		init_ao_share_memory(AoChn);
		stAoStreamShmInfo[AoChn].bRuning = 1;
		stAoStreamShmInfo[AoChn].Aochn = AoChn;
		stAoStreamShmInfo[AoChn].AoDev = AoDevId;

	}
	printf("rpc_server_aud_ao_send_frame chn:[%d]\n", AoChn);
	if (!stAoStreamShmInfo[AoChn].bInit) {
		printf("[Error][%s][%d]\n", __func__, __LINE__);
		return -1;
	}


	struct sched_param param;
	pthread_attr_t attr;

	param.sched_priority = 75;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	pthread_create(&s_pthAoStream[AoChn], &attr, Ao_stream_process_thread,
			   (void *)&stAoStreamShmInfo[AoChn]);

	return 0;
}


CVI_S32 rpc_client_aud_adec_send_stream(ADEC_CHN AdChn,
					const AUDIO_STREAM_S *pstStream,
					CVI_BOOL bBlock)
{

	// x_info("enter");
	AUD_ADEC_FRAME_CBLK stAdecStreamHead;
	CVI_S32 s32Ret;
	CVI_S32 s32Block;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	CVI_S32 bufsize = 0;
	int writelen;

	unsigned int msg = SET_MSG(MODE_AUDIO_ADEC,
				   DEFAULT_AUD_DEV_ID,
				   AdChn,
				   RPC_CMD_ADEC_SEND_STREAM);

	bufsize = sizeof(struct rpc_aud_msg) + sizeof(AUDIO_STREAM_S) +
		  pstStream->u32Len +
		  sizeof(CVI_S32);

	if (bBlock == CVI_TRUE)
		s32Block = 1;
	else
		s32Block = 0;

	if (!stAdecStreamShmInfo[AdChn].bInit) {
		buf = malloc(bufsize);
		if (buf == NULL) {
			printf("malloc failed\n");
			return -1;
		}

		stRpcMsg.msg = msg;
		stRpcMsg.magic = AUD_RPC_MAGIC;
		stRpcMsg.result = 0;
		stRpcMsg.body_len = 0;
		memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
		memcpy(buf + sizeof(struct rpc_aud_msg), pstStream, sizeof(AUDIO_STREAM_S));
		memcpy(buf + sizeof(struct rpc_aud_msg) + sizeof(AUDIO_STREAM_S),
			pstStream->pStream, pstStream->u32Len);
		memcpy(buf + sizeof(struct rpc_aud_msg) + sizeof(AUDIO_STREAM_S) +
			pstStream->u32Len,
			&s32Block, sizeof(CVI_S32));

		s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
		if (s32Ret != 0) {
			printf("rpc_send failed\n");
			free(buf);
			return s32Ret;
		}

		s32Ret = rpc_aud_client_response_proc(response, responselen, NULL);
		if (s32Ret != 0) {
			printf("[%s][%d]rpc_response_proc failed with %#x\n",
				__func__, __LINE__, s32Ret);
		}

		free(buf);
		nn_freemsg(response);

	}

	memset(&stAdecStreamHead, 0, sizeof(AUD_ADEC_FRAME_CBLK));
	stAdecStreamHead.s32SyncCode = 0xa3a3;
	if (stAdecStreamShmInfo[AdChn].bInit != 1) {
		init_adec_share_memory(AdChn);
		stAdecStreamShmInfo[AdChn].AdChn = AdChn;
		stAdecStreamShmInfo[AdChn].AdDev = 0;
		stAdecStreamShmInfo[AdChn].bRuning = 1;

	}

	char *shm_ptr = stAdecStreamShmInfo[AdChn].g_shm_ptr;

	if (!shm_ptr) {
		printf("shm is NULL\n");
		return -1;
	}

	if (stAdecStreamShmInfo[AdChn].bInit) {

		sem_wait(stAdecStreamShmInfo[AdChn].m_pSemWrite);

		shm_ptr = stAdecStreamShmInfo[AdChn].g_shm_ptr +
			(stAdecStreamShmInfo[AdChn].m_queue_index % stAdecStreamShmInfo[AdChn].m_queue_cap)
			* RPC_AUD_STREAM_ONEFRMAE_MAX_SIZE;
		stAdecStreamShmInfo[AdChn].m_queue_index++;
		memset(shm_ptr, 0, sizeof(AUD_ADEC_FRAME_CBLK));
		writelen = sizeof(AUD_ADEC_FRAME_CBLK);
		memcpy(shm_ptr + writelen, pstStream->pStream, pstStream->u32Len);
		//stAdecStreamHead.enSoundmode = pstStream->enSoundmode;
		stAdecStreamHead.u32Len = pstStream->u32Len;//byte
		stAdecStreamHead.u32Seq = pstStream->u32Seq;
		stAdecStreamHead.u64TimeStamp = pstStream->u64TimeStamp;
		//stAdecStreamHead.u64PhyAddr = pstStream->u64PhyAddr[0];
		memcpy(shm_ptr, &stAdecStreamHead, sizeof(AUD_ADEC_FRAME_CBLK));
		sem_post(stAdecStreamShmInfo[AdChn].m_pSemRead);

	}


	return 0;
}

void *Adec_stream_process_thread(void *argc)
{
	ADEC_SHARE_MEM_INFO *pAdecShmInfo = (ADEC_SHARE_MEM_INFO *)argc;
	char Task_name[64];
	ADEC_CHN AdChn = pAdecShmInfo->AdChn;
	int s32Ret;
	char *p_shm = NULL;
	int read_suc = 0;
	unsigned int buflen = 4096;
	//CVI_U64 delayTime = 0;
	//static CVI_U64 u64LastTime[3] = {0};
	//static CVI_U64 u64NowTime[3] = {0};
	AUD_ADEC_FRAME_CBLK stAdecStreamHead;
	AUDIO_STREAM_S stStream;

	sprintf(Task_name, "chn%dAdecStream", AdChn);
	prctl(PR_SET_NAME, Task_name, 0, 0, 0);
	printf("server adec task%d start\n", AdChn);

	stStream.pStream = (CVI_U8 *)malloc(buflen);

	for (int j = 0; j < stAdecStreamShmInfo[AdChn].m_queue_cap; j++)
		sem_post(stAdecStreamShmInfo[AdChn].m_pSemWrite);

	while (stAdecStreamShmInfo[AdChn].bRuning) {

		p_shm = stAdecStreamShmInfo[AdChn].g_shm_ptr +
			(stAdecStreamShmInfo[AdChn].m_queue_index % stAdecStreamShmInfo[AdChn].m_queue_cap)
			* RPC_AUD_STREAM_ONEFRMAE_MAX_SIZE;

		memset(&stAdecStreamHead, 0, sizeof(AUD_ADEC_FRAME_CBLK));
		sem_wait(stAdecStreamShmInfo[AdChn].m_pSemRead);
		memcpy(&stAdecStreamHead, p_shm, sizeof(AUD_ADEC_FRAME_CBLK));

		if (checkIsAdecFrameHead(&stAdecStreamHead)) {
			stAdecStreamShmInfo[AdChn].m_queue_index++;
			//u64NowTime[AdChn] = rpc_aud_get_boot_time();
			//delayTime = u64NowTime[AdChn] - stAdecStreamHead.u64TimeStamp;
			//printf("[adec]client time chn:%d delay:%llu,getstream delta:%llu,size:%u m_queue_index:%d\n",
			//      AdChn, delayTime, u64NowTime[AdChn] - u64LastTime[AdChn],
			//       stAdecStreamHead.u32Len, stAdecStreamShmInfo[AdChn].m_queue_index);
			//u64LastTime[AdChn] = u64NowTime[AdChn];
			memcpy(stAdecStreamShmInfo[AdChn].streamPtr, p_shm + sizeof(AUD_ADEC_FRAME_CBLK),
				   stAdecStreamHead.u32Len);
			read_suc = 1;
			memset(p_shm, 0, sizeof(AUD_ADEC_FRAME_CBLK));
			sem_post(stAdecStreamShmInfo[AdChn].m_pSemWrite);
			stStream.u32Len = stAdecStreamHead.u32Len;
			stStream.u64TimeStamp = stAdecStreamHead.u64TimeStamp;
			//stStream.enSoundmode = stAdecStreamHead.enSoundmode;

			if (stStream.u32Len > buflen) {
				printf("[error][%s][%d]u32Len:%d > buflen:%d\n",
				__func__, __LINE__, stStream.u32Len, buflen);
				break;
			}

			memset(stStream.pStream, 0, buflen);
			memcpy(stStream.pStream, stAdecStreamShmInfo[AdChn].streamPtr, stStream.u32Len);

			s32Ret = CVI_ADEC_SendStream(AdChn, &stStream, CVI_TRUE);
			if (s32Ret != CVI_SUCCESS) {
				printf("CVI_ADEC_SendStream failed with %#x!\n", s32Ret);
				break;
			}

		}

	}
	free(stStream.pStream);
	if (!read_suc) {
		printf("read failure[%s][%d]\n",  __func__, __LINE__);
		return NULL;
	}

	printf("adec_stream_process_thread exit\n");
	return NULL;

}

CVI_S32 rpc_server_aud_adec_send_stream(struct rpc_aud_msg *pstRpcMsg,
					unsigned char *response,
					int *responselen)
{

	ADEC_CHN AdChn = GET_CHN(pstRpcMsg->msg);

	AUD_RPC_UNUSED(pstRpcMsg);
	*responselen = sizeof(struct rpc_aud_msg);
	*responselen = *responselen;
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));

	if (!stAdecStreamShmInfo[AdChn].bInit) {
		init_adec_share_memory(AdChn);
		stAdecStreamShmInfo[AdChn].bRuning = 1;

	}
	printf("rpc_server_aud_dec_send_frame\n");
	if (!stAdecStreamShmInfo[AdChn].bInit) {
		return -1;
	}

	struct sched_param param;
	pthread_attr_t attr;

	param.sched_priority = 75;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	pthread_create(&s_pthAdecStream[AdChn], &attr, Adec_stream_process_thread,
			   (void *)&stAdecStreamShmInfo[AdChn]);

	return 0;

}

#else
CVI_S32 rpc_client_aud_aenc_release_stream(AENC_CHN AeChn,
		const AUDIO_STREAM_S *pstStream)
{

	// x_info("enter");
	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_AENC,
				   DEFAULT_AUD_DEV_ID,
				   AeChn,
				   RPC_CMD_AENC_RELEASE_STREAM);
	CVI_S32 bufsize = sizeof(struct rpc_aud_msg) + sizeof(AUDIO_STREAM_S);

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(buf + sizeof(struct rpc_aud_msg), pstStream, sizeof(AUDIO_STREAM_S));

	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, NULL);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
	return s32Ret;
}

CVI_S32 rpc_client_aud_aenc_get_stream(AENC_CHN AeChn,
					   AUDIO_STREAM_S *pstStream,
					   CVI_S32 s32MilliSec)
{

	// x_info("enter");
	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_AENC,
				   DEFAULT_AUD_DEV_ID,
				   AeChn,
				   RPC_CMD_AENC_GET_STREAM);
	CVI_S32 bufsize = sizeof(struct rpc_aud_msg) + sizeof(CVI_S32);

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(buf + sizeof(struct rpc_aud_msg), &s32MilliSec, sizeof(CVI_S32));

	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, pstStream);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
	return s32Ret;
}


CVI_S32 rpc_server_aud_aenc_get_stream(struct rpc_aud_msg *pstRpcMsg,
		unsigned char *response,
		int *responselen)
{
	CVI_S32 s32Ret;
	AUDIO_STREAM_S stStream;
	AENC_CHN AeChn = GET_CHN(pstRpcMsg->msg);
	int *ps32MilliSec = (int *)pstRpcMsg->body;
	int s32MilliSec = *ps32MilliSec;

	s32Ret =  CVI_AENC_GetStream(AeChn,
				&stStream,
				s32MilliSec);
	//printf("[%s][%d]len[%d]<---\n", __func__, __LINE__, stStream.u32Len);
	if (s32Ret) {
		printf("CVI_AENC_GetStream failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	*responselen = sizeof(struct rpc_aud_msg) + sizeof(AUDIO_STREAM_S) + stStream.u32Len;
	pstRpcMsg->body_len = (*responselen);
	memcpy(pstRpcMsg->body, &stStream, sizeof(AUDIO_STREAM_S));
	memcpy(pstRpcMsg->body + sizeof(AUDIO_STREAM_S), stStream.pStream, stStream.u32Len);
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg) + sizeof(AUDIO_STREAM_S) + stStream.u32Len);

	return 0;
}

CVI_S32 rpc_client_aud_ai_get_frame(AUDIO_DEV AiDevId, AI_CHN AiChn,
					AUDIO_FRAME_S *pstFrm, AEC_FRAME_S *pstAecFrm,
					CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_AIO,
				AiDevId,
				AiChn,
				RPC_CMD_AI_GET_FRAME);
	CVI_S32 bufsize = sizeof(struct rpc_aud_msg) + sizeof(CVI_S32);

	AUD_RPC_UNUSED(pstAecFrm);

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(buf + sizeof(struct rpc_aud_msg), &s32MilliSec, sizeof(CVI_S32));

	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, pstFrm);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);

	return s32Ret;


}



CVI_S32 rpc_server_aud_ai_get_frame(struct rpc_aud_msg *pstRpcMsg,
		unsigned char *response,
		int *responselen)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	AUDIO_FRAME_S stFrm;
	AEC_FRAME_S stAecFrm;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);
	AI_CHN chn = GET_CHN(pstRpcMsg->msg);
	//int *ps32MilliSec = (int *)pstRpcMsg->body;
	//int s32MilliSec = *ps32MilliSec;
	int s32GetFrameBytes;

	s32Ret = CVI_AI_GetFrame(dev, chn,
				&stFrm,
				&stAecFrm,
				AIN_GET_FRAME_FORCE_BLOCK);

	if (s32Ret) {
		printf("CVI_AI_GetFrame failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;




	s32GetFrameBytes = stFrm.u32Len * BYTES_PER_AUD_SAMPLE * (stFrm.enSoundmode + 1);//test single channel
#if DUMP_SERVER_CLIENT
	fwrite(stFrm.u64VirAddr[0], 1, s32GetFrameBytes, fd_server);
#endif
	*responselen = sizeof(struct rpc_aud_msg) + sizeof(AUDIO_FRAME_S) + s32GetFrameBytes;
	//printf("[%s][%d]responsexxx[%d][0x%x]\n", __func__, __LINE__, *responselen, stFrm.u64VirAddr[0]);
	pstRpcMsg->body_len = (*responselen);
	memcpy(pstRpcMsg->body, &stFrm, sizeof(AUDIO_FRAME_S));
	memcpy(pstRpcMsg->body + sizeof(AUDIO_FRAME_S), stFrm.u64VirAddr[0], s32GetFrameBytes);
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg) + sizeof(AUDIO_FRAME_S) + s32GetFrameBytes);
	return 0;
}



CVI_S32 rpc_client_aud_ao_send_frame(AUDIO_DEV AoDevId,
					 AO_CHN AoChn,
					 const AUDIO_FRAME_S *pstData,
					 CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_AIO,
				   AoDevId,
				   AoChn,
				   RPC_CMD_AO_SEND_FRAME);
	CVI_S32 s32FrameBytes = pstData->u32Len * (pstData->enSoundmode + 1);

	s32FrameBytes = s32FrameBytes * BYTES_PER_AUD_SAMPLE;

	CVI_S32 bufsize = sizeof(struct rpc_aud_msg) +
			  sizeof(AUDIO_FRAME_S) + s32FrameBytes + sizeof(CVI_S32);

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(buf + sizeof(struct rpc_aud_msg), pstData, sizeof(AUDIO_FRAME_S));
	memcpy(buf + sizeof(struct rpc_aud_msg) + sizeof(AUDIO_FRAME_S),
		   pstData->u64VirAddr[0],
		   s32FrameBytes);
	memcpy(buf + sizeof(struct rpc_aud_msg) + sizeof(AUDIO_FRAME_S) + s32FrameBytes,
		   &s32MilliSec,
		   sizeof(CVI_S32));

	//printf("client ao_send frame millisecond[%d]\n", s32MilliSec);
	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, NULL);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
	return s32Ret;

}

CVI_S32 rpc_server_aud_ao_send_frame(struct rpc_aud_msg *pstRpcMsg,
					 unsigned char *response,
					 int *responselen)
{
	CVI_S32 s32Ret;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);
	AO_CHN chn = GET_CHN(pstRpcMsg->msg);
	AUDIO_FRAME_S stAudFrm, *pstFrm = &stAudFrm;
	CVI_S32 s32FrameBytes = 0;
	char *pBuffer = NULL;//2ch 16bit 160 samples
	CVI_S32 s32MilliSec;

	memcpy(pstFrm, pstRpcMsg->body, sizeof(AUDIO_FRAME_S));
	s32FrameBytes = (pstFrm->enSoundmode + 1) * pstFrm->u32Len *
			BYTES_PER_AUD_SAMPLE;
	pBuffer = malloc(s32FrameBytes);
	pstFrm->u64VirAddr[0] = (CVI_U8 *)pBuffer;
	memcpy(pBuffer,
		   (pstRpcMsg->body + sizeof(AUDIO_FRAME_S)),
		   s32FrameBytes);

	memcpy(&s32MilliSec,
		   (pstRpcMsg->body + sizeof(AUDIO_FRAME_S) + s32FrameBytes),
		   sizeof(CVI_S32));
	//printf("[%s]s32MilliSec[%d] s32FrameBytes[%d]\n", __func__, s32MilliSec, s32FrameBytes);

	s32Ret = CVI_AO_SendFrame(dev, chn, pstFrm, s32MilliSec);
	free(pBuffer);
	if (s32Ret) {
		printf("CVI_AO_SendFrame client failed[%d][%d] with %#x\n", dev, chn, s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}


CVI_S32 rpc_client_aud_adec_send_stream(ADEC_CHN AdChn,
					const AUDIO_STREAM_S *pstStream,
					CVI_BOOL bBlock)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	CVI_S32 s32Block;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	CVI_S32 bufsize = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_ADEC,
				   DEFAULT_AUD_DEV_ID,
				   AdChn,
				   RPC_CMD_ADEC_SEND_STREAM);

	bufsize = sizeof(struct rpc_aud_msg) + sizeof(AUDIO_STREAM_S) +
		  pstStream->u32Len +
		  sizeof(CVI_S32);

	if (bBlock == CVI_TRUE)
		s32Block = 1;
	else
		s32Block = 0;

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(buf + sizeof(struct rpc_aud_msg), pstStream, sizeof(AUDIO_STREAM_S));
	memcpy(buf + sizeof(struct rpc_aud_msg) + sizeof(AUDIO_STREAM_S),
		   pstStream->pStream, pstStream->u32Len);
	memcpy(buf + sizeof(struct rpc_aud_msg) + sizeof(AUDIO_STREAM_S) +
		   pstStream->u32Len,
		   &s32Block, sizeof(CVI_S32));

	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, NULL);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
	return s32Ret;

}

CVI_S32 rpc_server_aud_adec_send_stream(struct rpc_aud_msg *pstRpcMsg,
					unsigned char *response,
					int *responselen)
{

	CVI_S32 s32Ret;
	ADEC_CHN AdChn = GET_CHN(pstRpcMsg->msg);
	AUDIO_STREAM_S  stStream, *pstStream = &stStream;
	//unsigned char *streambuf;

	memcpy(pstStream, (AUDIO_STREAM_S *)pstRpcMsg->body, sizeof(AUDIO_STREAM_S));
	//streambuf = malloc(pstStream->u32Len);
	pstStream->pStream = RpcServAdecSendBuf;
	memcpy(pstStream->pStream,
		   (pstRpcMsg->body + sizeof(AUDIO_STREAM_S)),
		   pstStream->u32Len);

	CVI_S32 *pBlock = (CVI_S32 *)(pstRpcMsg->body + sizeof(AUDIO_STREAM_S) +
					  pstStream->u32Len);
	CVI_BOOL bBlock = CVI_TRUE;

	if (*pBlock == 1)
		bBlock = CVI_TRUE;
	else if (pBlock == 0)
		bBlock = CVI_FALSE;
	else {
		printf("[Error][%s][%d]val[%d]\n", __func__, __LINE__, *pBlock);
	}

	s32Ret =  CVI_ADEC_SendStream(AdChn,
					  pstStream,
					  bBlock);
	if (s32Ret) {
		printf("CVI_ADEC_SendStream failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	//free(streambuf);
	return 0;
}
#endif
//----------------------------------------------------------------------------
CVI_S32 rpc_client_aud_ai_release_frame(AUDIO_DEV AiDevId, AI_CHN AiChn,
					const AUDIO_FRAME_S *pstFrm, const AEC_FRAME_S *pstAecFrm)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_AIO,
				   AiDevId,
				   AiChn,
				   RPC_CMD_AI_RELEASE_FRAME);
	CVI_S32 bufsize = sizeof(struct rpc_aud_msg) + sizeof(AUDIO_FRAME_S) + sizeof(
				  AEC_FRAME_S);


	AUD_RPC_UNUSED(pstAecFrm);
	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(buf + sizeof(struct rpc_aud_msg), pstFrm, sizeof(AUDIO_FRAME_S));
	memcpy(buf + sizeof(struct rpc_aud_msg) + sizeof(AUDIO_FRAME_S), pstAecFrm,
		   sizeof(AEC_FRAME_S));

	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, NULL);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
	return s32Ret;
}

CVI_S32 rpc_client_aud_ai_disable(AUDIO_DEV AiDevId)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_AIO,
				   AiDevId,
				   DEFAULT_AUD_CHN_ID,
				   RPC_CMD_AI_DISABLE);
	CVI_S32 bufsize = sizeof(struct rpc_aud_msg);

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, NULL);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
	return s32Ret;
}


CVI_S32 rpc_client_aud_ai_disable_chn(AUDIO_DEV AiDevId, AI_CHN AiChn)
{

#ifdef SHARE_MEM_IN_AUDIO_RPC

	AUD_RPC_UNUSED(AiDevId);
	deinit_ain_share_memory(AiChn);
#endif

	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_AIO,
				   AiDevId,
				   AiChn,
				   RPC_CMD_AI_DISABLE_CHN);
	CVI_S32 bufsize = sizeof(struct rpc_aud_msg);

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, NULL);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);

	return s32Ret;

}

CVI_S32 rpc_client_aud_ao_disable(AUDIO_DEV AoDevId)
{
	CVI_S32 s32Ret;

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AoDevId,
				   DEFAULT_AUD_CHN_ID,
				   RPC_CMD_AO_DISABLE,
				   0,
				   NULL,
				   NULL);

	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AO_DISABLE, __func__, __LINE__);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 rpc_client_aud_ao_enable(AUDIO_DEV AoDevId)
{
	// x_info("enter");
	CVI_S32 s32Ret;

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AoDevId,
				   DEFAULT_AUD_CHN_ID,
				   RPC_CMD_AO_ENABLE,
				   0,
				   NULL,
				   NULL);

	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AO_ENABLE, __func__, __LINE__);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 rpc_client_aud_ao_disable_chn(AUDIO_DEV AoDevId, AO_CHN AoChn)
{
	CVI_S32 s32Ret;

#ifdef SHARE_MEM_IN_AUDIO_RPC

	deinit_ao_share_memory(AoChn);
#endif

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AoDevId,
				   AoChn,
				   RPC_CMD_AO_DISABLE_CHN,
				   0,
				   NULL,
				   NULL);

	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AO_DISABLE_CHN, __func__, __LINE__);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 rpc_client_aud_ao_disable_resmp(AUDIO_DEV AoDevId, AO_CHN AoChn)
{
	CVI_S32 s32Ret;

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AoDevId,
				   AoChn,
				   RPC_CMD_AO_DISABLE_RESMP,
				   0,
				   NULL,
				   NULL);

	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AO_DISABLE_RESMP, __func__, __LINE__);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 rpc_client_aud_ai_disable_resmp(AUDIO_DEV AiDevId, AI_CHN AiChn)
{
	CVI_S32 s32Ret;

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AiDevId,
				   AiChn,
				   RPC_CMD_AI_DISABLE_RESMP,
				   0,
				   NULL,
				   NULL);

	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AI_DISABLE_RESMP, __func__, __LINE__);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 rpc_client_aud_ao_enable_resmp(AUDIO_DEV AoDevId, AO_CHN AoChn,
					   AUDIO_SAMPLE_RATE_E enInSampleRate)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	CVI_S32 s32SampleRate, *ps32SampleRate = &s32SampleRate;

	s32SampleRate = (AUDIO_SAMPLE_RATE_E)enInSampleRate;

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AoDevId,
				   AoChn,
				   RPC_CMD_AO_ENABLE_RESMP,
				   sizeof(CVI_S32),
				   (char *)ps32SampleRate,
				   NULL);

	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AO_ENABLE_RESMP, __func__, __LINE__);
		return CVI_FAILURE;
	}

	return s32Ret;
}


CVI_S32 rpc_client_aud_ai_enable_resmp(AUDIO_DEV AiDevId, AI_CHN AiChn,
					   AUDIO_SAMPLE_RATE_E enOutSampleRate)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	CVI_S32 s32SampleRate, *ps32SampleRate = &s32SampleRate;

	s32SampleRate = (AUDIO_SAMPLE_RATE_E)enOutSampleRate;

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AiDevId,
				   AiChn,
				   RPC_CMD_AI_ENABLE_RESMP,
				   sizeof(CVI_S32),
				   (char *)ps32SampleRate,
				   NULL);

	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AI_ENABLE_RESMP, __func__, __LINE__);
		return CVI_FAILURE;
	}

	return s32Ret;
}


CVI_S32 rpc_client_aud_aenc_destroy_chn(AENC_CHN AeChn)
{
	CVI_S32 s32Ret;
	printf("[%s][\n", __func__);
	s32Ret = aud_client_common(MODE_AUDIO_AENC,
				   DEFAULT_AUD_DEV_ID,
				   AeChn,
				   RPC_CMD_AENC_DESTROY_CHN,
				   0,
				   NULL,
				   NULL);

	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AENC_DESTROY_CHN, __func__, __LINE__);
		return CVI_FAILURE;
	}
#ifdef SHARE_MEM_IN_AUDIO_RPC
	deinit_aenc_share_memory(AeChn);
#endif
	return s32Ret;
}

CVI_S32 rpc_client_aud_aenc_create_chn(AENC_CHN AeChn, const AENC_CHN_ATTR_S *pstAttr)
{
	// x_info("enter");
	CVI_S32 s32Ret;

	s32Ret = aud_client_common(MODE_AUDIO_AENC,
				   DEFAULT_AUD_DEV_ID,
				   AeChn,
				   RPC_CMD_AENC_CREATE_CHN,
				   sizeof(AENC_CHN_ATTR_S),
				   (char *)pstAttr,
				   NULL);

	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AENC_CREATE_CHN, __func__, __LINE__);
		return CVI_FAILURE;
	}

	return s32Ret;
}


CVI_S32 rpc_client_aud_ao_enable_chn(AUDIO_DEV AoDevId, AO_CHN AoChn)
{
	// x_info("enter");
	CVI_S32 s32Ret;

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AoDevId,
				   AoChn,
				   RPC_CMD_AO_ENABLE_CHN,
				   0,
				   NULL,
				   NULL);

	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AO_ENABLE_CHN, __func__, __LINE__);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 rpc_client_aud_ai_enable_chn(AUDIO_DEV AiDevId, AI_CHN AiChn)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_AIO,
				   AiDevId,
				   AiChn,
				   RPC_CMD_AI_ENABLE_CHN);
	CVI_S32 bufsize = sizeof(struct rpc_aud_msg);

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, NULL);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
	return s32Ret;
}


CVI_S32 rpc_client_aud_ai_clr_pub_attr(AUDIO_DEV AiDevId)
{
	// x_info("enter");
	CVI_S32 s32Ret;

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AiDevId,
				   DEFAULT_AUD_CHN_ID,
				   RPC_CMD_AI_CLR_PUB_ATTR,
				   0,
				   NULL,
				   NULL);

	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AI_CLR_PUB_ATTR, __func__, __LINE__);
		return CVI_FAILURE;
	}
	return s32Ret;
}



CVI_S32 rpc_client_aud_ai_enable(AUDIO_DEV AiDevId)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_AIO,
				   AiDevId,
				   DEFAULT_AUD_CHN_ID,
				   RPC_CMD_AI_ENABLE);
	CVI_S32 bufsize = sizeof(struct rpc_aud_msg);

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, NULL);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
	return s32Ret;
}

CVI_S32 rpc_client_aud_ai_enable_vqe(AUDIO_DEV AiDevId, AI_CHN AiChn)
{
	CVI_S32 s32Ret;

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AiDevId,
				   AiChn,
				   RPC_CMD_AI_ENABLE_VQE,
				   0,
				   NULL,
				   NULL);

	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AI_ENABLE_VQE, __func__, __LINE__);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 rpc_client_aud_ai_disable_vqe(AUDIO_DEV AiDevId, AI_CHN AiChn)
{
	CVI_S32 s32Ret;

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AiDevId,
				   AiChn,
				   RPC_CMD_AI_DISABLE_VQE,
				   0,
				   NULL,
				   NULL);

	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AI_DISABLE_VQE, __func__, __LINE__);
		return CVI_FAILURE;
	}

	return s32Ret;
}



CVI_S32 rpc_client_aud_ao_get_pub_attr(AUDIO_DEV AoDevId, AIO_ATTR_S *pstAttr)
{
	// x_info("enter");
	CVI_S32 s32Ret;

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AoDevId,
				   DEFAULT_AUD_CHN_ID,
				   RPC_CMD_AO_GET_PUB_ATTR,
				   sizeof(AIO_ATTR_S),
				   (char *)pstAttr,
				   (void *)pstAttr);

	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AO_GET_PUB_ATTR, __func__, __LINE__);
		return CVI_FAILURE;
	}

	return s32Ret;
}


CVI_S32 rpc_client_aud_ai_get_chn_param(AUDIO_DEV AiDevId, AI_CHN AiChn,
					AI_CHN_PARAM_S *pstChnParam)
{
	CVI_S32 s32Ret;

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AiDevId,
				   AiChn,
				   RPC_CMD_AI_GET_CHN_PARAM,
				   0,
				   NULL,
				   (void *)pstChnParam);

	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AI_GET_CHN_PARAM, __func__, __LINE__);
		return CVI_FAILURE;
	}

	return s32Ret;
}


CVI_S32 rpc_client_aud_ai_get_talk_vqe_attr(AUDIO_DEV AiDevId, AI_CHN AiChn,
		AI_TALKVQE_CONFIG_S *pstVqeConfig)
{
	// x_info("enter");
	CVI_S32 s32Ret;

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AiDevId,
				   AiChn,
				   RPC_CMD_AI_GET_TALK_VQE_ATTR,
				   0,
				   NULL,
				   (void *)pstVqeConfig);

	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AI_GET_TALK_VQE_ATTR, __func__, __LINE__);
		return CVI_FAILURE;
	}
	return s32Ret;
}


CVI_S32 rpc_client_aud_ai_get_pub_attr(AUDIO_DEV AiDevId,
					   AIO_ATTR_S *pstAttr)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_AIO,
				   AiDevId,
				   DEFAULT_AUD_CHN_ID,
				   RPC_CMD_AI_GET_PUB_ATTR);
	CVI_S32 bufsize = sizeof(struct rpc_aud_msg) + sizeof(AIO_ATTR_S);

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(buf + sizeof(struct rpc_aud_msg), pstAttr, sizeof(AIO_ATTR_S));

	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, pstAttr);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
	return s32Ret;
}




CVI_S32 rpc_client_aud_ao_set_pub_attr(AUDIO_DEV AoDevId,
					   const AIO_ATTR_S *pstAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 s32AddSize = sizeof(AIO_ATTR_S);

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AoDevId,
				   DEFAULT_AUD_CHN_ID,
				   RPC_CMD_AO_SET_PUB_ATTR,
				   s32AddSize,
				   (char *)pstAttr,
				   NULL);
	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AO_SET_PUB_ATTR, __func__, __LINE__);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 rpc_client_aud_ai_set_chn_param(AUDIO_DEV AiDevId, AI_CHN AiChn,
					const AI_CHN_PARAM_S *pstChnParam)
{
	CVI_S32 s32Ret;
	AI_CHN_PARAM_S stChnParam;

	memcpy(&stChnParam, pstChnParam, sizeof(AI_CHN_PARAM_S));
	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AiDevId,
				   AiChn,
				   RPC_CMD_AI_SET_CHN_PARAM,
				   sizeof(AI_CHN_PARAM_S),
				   (char *)&stChnParam,
				   NULL);
	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AI_SET_CHN_PARAM, __func__, __LINE__);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 rpc_client_aud_ai_set_talk_vqe_attr(AUDIO_DEV AiDevId, AI_CHN AiChn,
		AUDIO_DEV AoDevId, AO_CHN AoChn,
		const AI_TALKVQE_CONFIG_S *pstVqeConfig)
{
	// x_info("enter");
	CVI_S32 s32Ret;

	AUD_RPC_UNUSED(AoDevId);//not support aout vqe
	AUD_RPC_UNUSED(AoChn);//not support aout vqe
	AI_TALKVQE_CONFIG_S stVqeConfig;

	memcpy(&stVqeConfig, pstVqeConfig, sizeof(AI_TALKVQE_CONFIG_S));

	s32Ret = aud_client_common(MODE_AUDIO_AIO,
				   AiDevId,
				   AiChn,
				   RPC_CMD_AI_SET_TALK_VQE_ATTR,
				   sizeof(AI_TALKVQE_CONFIG_S),
				   (char *)&stVqeConfig,
				   NULL);
	if (s32Ret != CVI_SUCCESS) {
		RPC_AUD_ERR(RPC_CMD_AI_SET_TALK_VQE_ATTR, __func__, __LINE__);
		return CVI_FAILURE;
	}

	return s32Ret;
}



CVI_S32 rpc_client_aud_ai_set_pub_attr(AUDIO_DEV AiDevId,
					   const AIO_ATTR_S *pstAttr)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_AIO,
				   AiDevId,
				   DEFAULT_AUD_CHN_ID,
				   RPC_CMD_AI_SET_PUB_ATTR);
	CVI_S32 bufsize = sizeof(struct rpc_aud_msg) + sizeof(AIO_ATTR_S);

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(buf + sizeof(struct rpc_aud_msg), pstAttr, sizeof(AIO_ATTR_S));

	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, NULL);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
	return s32Ret;
}


CVI_S32  rpc_client_aud_sys_unbind(const MMF_CHN_S *pstSrcChn,
				   const MMF_CHN_S *pstDestChn)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_AIO,
				   DEFAULT_AUD_DEV_ID,
				   DEFAULT_AUD_CHN_ID,
				   RPC_CMD_AUD_SYS_UNBIND);
	CVI_S32 bufsize = sizeof(struct rpc_aud_msg) + sizeof(MMF_CHN_S) + sizeof(
				  MMF_CHN_S);

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(buf + sizeof(struct rpc_aud_msg), pstSrcChn, sizeof(MMF_CHN_S));
	memcpy(buf + sizeof(struct rpc_aud_msg) + sizeof(MMF_CHN_S),
		   pstDestChn, sizeof(MMF_CHN_S));

	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, NULL);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
	return s32Ret;
}

CVI_S32  rpc_client_aud_sys_bind(const MMF_CHN_S *pstSrcChn,
				 const MMF_CHN_S *pstDestChn)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	unsigned char *buf;
	struct rpc_aud_msg stRpcMsg;
	char *response;
	int responselen = 0;
	unsigned int msg = SET_MSG(MODE_AUDIO_AIO,
				   DEFAULT_AUD_DEV_ID,
				   DEFAULT_AUD_CHN_ID,
				   RPC_CMD_AUD_SYS_BIND);
	CVI_S32 bufsize = sizeof(struct rpc_aud_msg) + sizeof(MMF_CHN_S) + sizeof(
				  MMF_CHN_S);

	buf = malloc(bufsize);
	if (buf == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	stRpcMsg.msg = msg;
	stRpcMsg.magic = AUD_RPC_MAGIC;
	stRpcMsg.result = 0;
	stRpcMsg.body_len = 0;
	memcpy(buf, &stRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(buf + sizeof(struct rpc_aud_msg), pstSrcChn, sizeof(MMF_CHN_S));
	memcpy(buf + sizeof(struct rpc_aud_msg) + sizeof(MMF_CHN_S),
		   pstDestChn, sizeof(MMF_CHN_S));

	s32Ret = rpc_aud_client_send(buf, bufsize, 5000, &response, &responselen);
	if (s32Ret != 0) {
		printf("rpc_send failed\n");
		free(buf);
		return s32Ret;
	}

	s32Ret = rpc_aud_client_response_proc(response, responselen, NULL);
	if (s32Ret != 0) {
		printf("[%s][%d]rpc_response_proc failed with %#x\n",
			   __func__, __LINE__, s32Ret);
	}

	free(buf);
	nn_freemsg(response);
	return s32Ret;
}

CVI_S32 rpc_server_aud_sys_bind(struct rpc_aud_msg *pstRpcMsg,
				unsigned char *response, int *responselen)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	MMF_CHN_S *pstSrcChn = (MMF_CHN_S *)pstRpcMsg->body;
	MMF_CHN_S *pstDestChn = (MMF_CHN_S *)(pstRpcMsg->body + sizeof(MMF_CHN_S));

	s32Ret = CVI_AUD_SYS_Bind(pstSrcChn, pstDestChn);
	if (s32Ret) {
		printf("CVI_AUD_SYS_Bind failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}



CVI_S32 rpc_server_aud_sys_unbind(struct rpc_aud_msg *pstRpcMsg,
				  unsigned char *response, int *responselen)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	MMF_CHN_S *pstSrcChn = (MMF_CHN_S *)pstRpcMsg->body;
	MMF_CHN_S *pstDestChn = (MMF_CHN_S *)(pstRpcMsg->body + sizeof(MMF_CHN_S));

	s32Ret = CVI_AUD_SYS_UnBind(pstSrcChn, pstDestChn);
	if (s32Ret) {
		printf("CVI_AUD_SYS_UnBind failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}



CVI_S32 rpc_server_aud_ao_set_pub_attr(struct rpc_aud_msg *pstRpcMsg,
					   unsigned char *response,
					   int *responselen)
{
	CVI_S32 s32Ret;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);
	AIO_ATTR_S stAttr;
	AIO_ATTR_S *pstAttr = &stAttr;

	memcpy(pstAttr, (AIO_ATTR_S *)pstRpcMsg->body, sizeof(AIO_ATTR_S));

	s32Ret = CVI_AO_SetPubAttr(dev, pstAttr);
	if (s32Ret) {
		printf("CVI_AO_SetPubAttr failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}

CVI_S32 rpc_server_aud_ai_set_pub_attr(struct rpc_aud_msg *pstRpcMsg,
					   unsigned char *response,
					   int *responselen)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);
	const AIO_ATTR_S *pstAttr = (AIO_ATTR_S *)pstRpcMsg->body;

	s32Ret = CVI_AI_SetPubAttr(dev, pstAttr);
	if (s32Ret) {
		printf("CVI_AI_SetPubAttr failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}


CVI_S32 rpc_server_aud_ai_set_talk_vqe_attr(struct rpc_aud_msg *pstRpcMsg,
		unsigned char *response,
		int *responselen)
{
	CVI_S32 s32Ret;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);
	AI_CHN AiChn = GET_CHN(pstRpcMsg->msg);

	const AI_TALKVQE_CONFIG_S *pstVqeConfig =
		(AI_TALKVQE_CONFIG_S *)pstRpcMsg->body;

	s32Ret = CVI_AI_SetTalkVqeAttr(dev, AiChn, 0, 0, pstVqeConfig);
	if (s32Ret) {
		printf("CVI_AI_SetTalkVqeAttr failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}

CVI_S32 rpc_server_aud_ai_set_chn_param(struct rpc_aud_msg *pstRpcMsg,
					unsigned char *response,
					int *responselen)
{
	CVI_S32 s32Ret;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);
	AI_CHN chn = GET_CHN(pstRpcMsg->msg);

	const AI_CHN_PARAM_S *pstChnParam = (AI_CHN_PARAM_S *)pstRpcMsg->body;

	s32Ret = CVI_AI_SetChnParam(dev, chn, pstChnParam);
	if (s32Ret) {
		printf("CVI_AI_SetChnParam failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}

CVI_S32 rpc_server_aud_ai_release_frame(struct rpc_aud_msg *pstRpcMsg,
					unsigned char *response,
					int *responselen)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	const AUDIO_FRAME_S *pstFrm = (AUDIO_FRAME_S *)pstRpcMsg->body;
	const AEC_FRAME_S *pstAecFrm = (AEC_FRAME_S *)(pstRpcMsg->body + sizeof(
						   AUDIO_FRAME_S));
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);
	AI_CHN chn = GET_CHN(pstRpcMsg->msg);


	s32Ret = CVI_AI_ReleaseFrame(dev, chn,
					 pstFrm,
					 pstAecFrm);
	if (s32Ret) {
		printf("CVI_AI_ReleaseFrame failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(response +  sizeof(struct rpc_aud_msg), pstFrm, sizeof(AUDIO_FRAME_S));
	memcpy(response +  sizeof(struct rpc_aud_msg) + sizeof(AUDIO_FRAME_S),
		   pstAecFrm, sizeof(AEC_FRAME_S));
	*responselen = sizeof(struct rpc_aud_msg) + sizeof(AUDIO_FRAME_S) + sizeof(
				   AEC_FRAME_S);
	return 0;
}

CVI_S32 rpc_server_aud_ai_get_volume(struct rpc_aud_msg *pstRpcMsg,
					 unsigned char *response,
					 int *responselen)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	CVI_S32 s32VolumeStep, *ps32VolumeStep = &s32VolumeStep;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);


	s32Ret = CVI_AI_GetVolume(dev, ps32VolumeStep);
	if (s32Ret) {
		printf("CVI_AI_GetFrame failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	*responselen = sizeof(struct rpc_aud_msg) + sizeof(CVI_S32);
	pstRpcMsg->body_len = (*responselen);
	memcpy(pstRpcMsg->body, ps32VolumeStep, sizeof(CVI_S32));
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg) + sizeof(CVI_S32));

	return 0;
}


CVI_S32 rpc_server_aud_ao_get_volume(struct rpc_aud_msg *pstRpcMsg,
					 unsigned char *response,
					 int *responselen)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	CVI_S32 s32VolumeDb, *ps32VolumeDb = &s32VolumeDb;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);

	s32Ret = CVI_AO_GetVolume(dev, ps32VolumeDb);
	if (s32Ret) {
		printf("CVI_AI_GetFrame failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	*responselen = sizeof(struct rpc_aud_msg) + sizeof(CVI_S32);
	pstRpcMsg->body_len = (*responselen);
	memcpy(pstRpcMsg->body, ps32VolumeDb, sizeof(CVI_S32));
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg) + sizeof(CVI_S32));

	return 0;
}

CVI_S32 rpc_server_aud_ai_set_volume(struct rpc_aud_msg *pstRpcMsg,
					 unsigned char *response,
					 int *responselen)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	CVI_S32 s32VolumeStep = 0;

	memcpy(&s32VolumeStep, pstRpcMsg->body, sizeof(CVI_S32));
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);

	s32Ret = CVI_AI_SetVolume(dev,
				  s32VolumeStep);
	if (s32Ret) {
		printf("CVI_AI_SetVolume vol[%d] failed with %#x\n",
			   s32VolumeStep, s32Ret);
		pstRpcMsg->result = s32Ret;
	} else {
		pstRpcMsg->result = CVI_SUCCESS;
	}

	*responselen = sizeof(struct rpc_aud_msg);
	pstRpcMsg->body_len = *responselen;
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));

	return 0;
}


CVI_S32 rpc_server_aud_ao_set_volume(struct rpc_aud_msg *pstRpcMsg,
					 unsigned char *response,
					 int *responselen)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	CVI_S32 s32VolumeStep = 0;

	memcpy(&s32VolumeStep, pstRpcMsg->body, sizeof(CVI_S32));
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);

	s32Ret = CVI_AO_SetVolume(dev,
				  s32VolumeStep);
	if (s32Ret) {
		printf("CVI_AO_SetVolume vol[%d] failed with %#x\n",
			   s32VolumeStep, s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	*responselen = sizeof(struct rpc_aud_msg);
	pstRpcMsg->body_len = *responselen;
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	return 0;
}

CVI_S32 rpc_server_aud_adec_release_frame(struct rpc_aud_msg *pstRpcMsg,
		unsigned char *response,
		int *responselen)
{
	CVI_S32 s32Ret;
	ADEC_CHN AdChn = GET_CHN(pstRpcMsg->msg);
	AUDIO_FRAME_INFO_S *pstAdecFrmInfo = (AUDIO_FRAME_INFO_S *)(pstRpcMsg->body);


	s32Ret =  CVI_ADEC_ReleaseFrame(AdChn,
					pstAdecFrmInfo);
	if (s32Ret) {
		printf("CVI_ADEC_ReleaseFrame failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	*responselen = sizeof(struct rpc_aud_msg) + sizeof(AUDIO_FRAME_INFO_S);
	pstRpcMsg->body_len = (*responselen);
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(response + sizeof(struct rpc_aud_msg), pstAdecFrmInfo,
		   sizeof(AUDIO_FRAME_INFO_S));
	return 0;
}



CVI_S32 rpc_server_aud_adec_send_end_of_stream(struct rpc_aud_msg *pstRpcMsg,
		unsigned char *response,
		int *responselen)
{
	CVI_S32 s32Ret;
	ADEC_CHN AdChn = GET_CHN(pstRpcMsg->msg);
	CVI_S32 s32Instant;
	CVI_BOOL bInstant;

	memcpy(&s32Instant, (pstRpcMsg->body), sizeof(CVI_S32));
	if (s32Instant == 1)
		bInstant = CVI_TRUE;
	else
		bInstant = CVI_FALSE;

	s32Ret =  CVI_ADEC_SendEndOfStream(AdChn, bInstant);
	if (s32Ret) {
		printf("CVI_ADEC_SendEndOfStream failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;

}

CVI_S32 rpc_server_aud_adec_get_frame(struct rpc_aud_msg *pstRpcMsg,
					  unsigned char *response,
					  int *responselen)
{
	CVI_S32 s32Ret;
	AUDIO_FRAME_INFO_S stAdecFrmInfo;
	ADEC_CHN AdChn = GET_CHN(pstRpcMsg->msg);
	CVI_S32 *pBlock = (int *)(pstRpcMsg->body);
	CVI_BOOL bBlock = CVI_TRUE;
	CVI_S32 s32GetFrameBytes = 0;
	AUDIO_FRAME_S stFrame, *_pstFrame = &stFrame;

	stAdecFrmInfo.pstFrame = _pstFrame;
	if (*pBlock == 1)
		bBlock = CVI_TRUE;
	else
		bBlock = CVI_FALSE;

	s32Ret =  CVI_ADEC_GetFrame(AdChn,
					&stAdecFrmInfo,
					bBlock);
	if (s32Ret) {
		printf("CVI_ADEC_GetFrame failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	stAdecFrmInfo.u32Id = 100;
	s32GetFrameBytes = _pstFrame->u32Len * BYTES_PER_AUD_SAMPLE *
			   (_pstFrame->enSoundmode + 1);
	*responselen = sizeof(struct rpc_aud_msg) + sizeof(AUDIO_FRAME_INFO_S)
			   + sizeof(AUDIO_FRAME_S) + s32GetFrameBytes;
	pstRpcMsg->body_len = (*responselen);
	memcpy(pstRpcMsg->body, &stAdecFrmInfo, sizeof(AUDIO_FRAME_INFO_S));
	memcpy(pstRpcMsg->body + sizeof(AUDIO_FRAME_INFO_S), stAdecFrmInfo.pstFrame,
		   sizeof(AUDIO_FRAME_S));
	memcpy(pstRpcMsg->body + sizeof(AUDIO_FRAME_INFO_S) + sizeof(AUDIO_FRAME_S),
		   _pstFrame->u64VirAddr[0], s32GetFrameBytes);
	memcpy(response, pstRpcMsg, (*responselen));
	return 0;
}



CVI_S32 rpc_server_aud_adec_destroy_chn(struct rpc_aud_msg *pstRpcMsg,
					unsigned char *response,
					int *responselen)
{

	CVI_S32 s32Ret;
	ADEC_CHN AdChn = GET_CHN(pstRpcMsg->msg);

#ifdef SHARE_MEM_IN_AUDIO_RPC
	deinit_adec_share_memory(AdChn);
#endif

	s32Ret =  CVI_ADEC_DestroyChn(AdChn);
	if (s32Ret) {
		printf("CVI_ADEC_DestroyChn failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}

CVI_S32 rpc_server_aud_adec_create_chn(struct rpc_aud_msg *pstRpcMsg,
					   unsigned char *response,
					   int *responselen)
{
	CVI_S32 s32Ret;
	ADEC_CHN AdChn = GET_CHN(pstRpcMsg->msg);
	ADEC_CHN_ATTR_S  stAdecAttr, *pstAdecAttr = &stAdecAttr;

	memcpy(pstAdecAttr, (ADEC_CHN_ATTR_S *)pstRpcMsg->body,
		   sizeof(ADEC_CHN_ATTR_S));
	s32Ret =  CVI_ADEC_CreateChn(AdChn,
					 pstAdecAttr);
	if (s32Ret) {
		printf("CVI_ADEC_CreateChn failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}

CVI_S32 rpc_server_aud_aenc_send_frame(struct rpc_aud_msg *pstRpcMsg,
					   unsigned char *response,
					   int *responselen)
{
	CVI_S32 s32Ret;
	AENC_CHN AeChn = GET_CHN(pstRpcMsg->msg);
	AUDIO_FRAME_S *pstFrm = (AUDIO_FRAME_S *)pstRpcMsg->body;
	const AEC_FRAME_S *pstAecFrm =
		NULL;//(AEC_FRAME_S *)(pstRpcMsg->body + sizeof(AUDIO_FRAME_S));

	pstFrm->u64VirAddr[0] = (CVI_U8 *)pstRpcMsg->body + sizeof(AUDIO_FRAME_S);
	s32Ret =  CVI_AENC_SendFrame(AeChn,
					 pstFrm,
					 pstAecFrm);
	if (s32Ret) {
		printf("CVI_AENC_SendFrame failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}


CVI_S32 rpc_server_aud_aenc_release_stream(struct rpc_aud_msg *pstRpcMsg,
		unsigned char *response,
		int *responselen)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	const AUDIO_STREAM_S *pstStream = (AUDIO_STREAM_S *)pstRpcMsg->body;
	AENC_CHN AeChn = GET_CHN(pstRpcMsg->msg);

	s32Ret =  CVI_AENC_ReleaseStream(AeChn, pstStream);
	if (s32Ret) {
		printf("CVI_AENC_ReleaseStream failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(response +  sizeof(struct rpc_aud_msg), pstStream,
		   sizeof(AUDIO_STREAM_S));
	*responselen = sizeof(struct rpc_aud_msg) + sizeof(AUDIO_STREAM_S);
	return 0;
}

CVI_S32 rpc_server_aud_ao_get_pub_attr(struct rpc_aud_msg *pstRpcMsg,
					   unsigned char *response,
					   int *responselen)
{
	CVI_S32 s32Ret;
	AIO_ATTR_S stAttr;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);

	s32Ret = CVI_AO_GetPubAttr(dev, &stAttr);
	if (s32Ret) {
		printf("CVI_AO_GetPubAttr failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	*responselen = sizeof(struct rpc_aud_msg) + sizeof(AIO_ATTR_S);
	pstRpcMsg->body_len = (*responselen);
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(response +  sizeof(struct rpc_aud_msg), &stAttr, sizeof(AIO_ATTR_S));

	return 0;
}

CVI_S32 rpc_server_aud_ai_get_pub_attr(struct rpc_aud_msg *pstRpcMsg,
					   unsigned char *response,
					   int *responselen)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	AIO_ATTR_S stAttr;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);

	s32Ret = CVI_AI_GetPubAttr(dev, &stAttr);
	if (s32Ret) {
		printf("CVI_AI_GetPubAttr failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	*responselen = sizeof(struct rpc_aud_msg) + sizeof(AIO_ATTR_S);
	pstRpcMsg->body_len = (*responselen);
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(response +  sizeof(struct rpc_aud_msg), &stAttr, sizeof(AIO_ATTR_S));

	return 0;
}

CVI_S32 rpc_server_aud_ai_get_chn_param(struct rpc_aud_msg *pstRpcMsg,
					unsigned char *response,
					int *responselen)
{
	CVI_S32 s32Ret;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);
	AI_CHN chn = GET_CHN(pstRpcMsg->msg);
	AI_CHN_PARAM_S stAiChnParam;

	s32Ret = CVI_AI_GetChnParam(dev, chn, &stAiChnParam);
	if (s32Ret) {
		printf("CVI_AI_GetChnParam failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	*responselen = sizeof(struct rpc_aud_msg) + sizeof(AI_CHN_PARAM_S);
	pstRpcMsg->body_len = (*responselen);
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(response + sizeof(struct rpc_aud_msg), &stAiChnParam,
		   sizeof(AI_CHN_PARAM_S));

	return 0;
}


CVI_S32 rpc_server_aud_ai_get_talk_vqe_attr(struct rpc_aud_msg *pstRpcMsg,
		unsigned char *response,
		int *responselen)
{
	CVI_S32 s32Ret;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);
	AI_CHN chn = GET_CHN(pstRpcMsg->msg);
	AI_TALKVQE_CONFIG_S stVqeConfig, *pstVqeConfig = &stVqeConfig;

	s32Ret = CVI_AI_GetTalkVqeAttr(dev, chn, pstVqeConfig);
	if (s32Ret) {
		printf("CVI_AI_GetTalkVqeAttr failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	*responselen = sizeof(struct rpc_aud_msg) + sizeof(AI_TALKVQE_CONFIG_S);
	pstRpcMsg->body_len = (*responselen);
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(response + sizeof(struct rpc_aud_msg), pstVqeConfig,
		   sizeof(AI_TALKVQE_CONFIG_S));

	return 0;
}

CVI_S32 rpc_server_aud_ai_enable(struct rpc_aud_msg *pstRpcMsg,
				 unsigned char *response,
				 int *responselen)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);

	s32Ret = CVI_AI_Enable(dev);
	if (s32Ret) {
		printf("CVI_AI_Enable failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}


CVI_S32 rpc_server_aud_ai_enable_resmp(struct rpc_aud_msg *pstRpcMsg,
					   unsigned char *response,
					   int *responselen)
{
	CVI_S32 s32Ret;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);
	AI_CHN chn = GET_CHN(pstRpcMsg->msg);
	CVI_S32 s32OutSampleRate = 0;

	memcpy(&s32OutSampleRate, (CVI_S32 *)pstRpcMsg->body, sizeof(CVI_S32));

	s32Ret = CVI_AI_EnableReSmp(dev, chn, (AUDIO_SAMPLE_RATE_E)s32OutSampleRate);
	if (s32Ret) {
		printf("CVI_AI_EnableReSmp failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}


CVI_S32 rpc_server_aud_ao_disable_resmp(struct rpc_aud_msg *pstRpcMsg,
					unsigned char *response,
					int *responselen)
{
	CVI_S32 s32Ret;
	AUDIO_DEV AoDevId = GET_DEV(pstRpcMsg->msg);
	AI_CHN AoChn = GET_CHN(pstRpcMsg->msg);

	s32Ret = CVI_AO_DisableReSmp(AoDevId, AoChn);
	if (s32Ret) {
		printf("CVI_AO_DisableReSmp failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}

CVI_S32 rpc_server_aud_ao_enable_resmp(struct rpc_aud_msg *pstRpcMsg,
					   unsigned char *response,
					   int *responselen)
{
	CVI_S32 s32Ret;
	AUDIO_DEV AoDevId = GET_DEV(pstRpcMsg->msg);
	AI_CHN AoChn = GET_CHN(pstRpcMsg->msg);
	CVI_S32 s32InSampleRate = 0;

	memcpy(&s32InSampleRate, (CVI_S32 *)pstRpcMsg->body, sizeof(CVI_S32));

	s32Ret = CVI_AO_EnableReSmp(AoDevId, AoChn,
					(AUDIO_SAMPLE_RATE_E)s32InSampleRate);
	if (s32Ret) {
		printf("CVI_AO_EnableReSmp failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}

CVI_S32 rpc_server_aud_ai_disable_resmp(struct rpc_aud_msg *pstRpcMsg,
					unsigned char *response,
					int *responselen)
{
	CVI_S32 s32Ret;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);
	AI_CHN chn = GET_CHN(pstRpcMsg->msg);

	s32Ret = CVI_AI_DisableReSmp(dev, chn);
	if (s32Ret) {
		printf("CVI_AI_DisableReSmp failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}

CVI_S32 rpc_server_aud_ai_enable_vqe(struct rpc_aud_msg *pstRpcMsg,
					 unsigned char *response,
					 int *responselen)
{

	CVI_S32 s32Ret;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);
	AI_CHN chn = GET_CHN(pstRpcMsg->msg);

	s32Ret = CVI_AI_EnableVqe(dev, chn);
	if (s32Ret) {
		printf("CVI_AI_EnableVqe failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}


CVI_S32 rpc_server_aud_ai_clr_pub_attr(struct rpc_aud_msg *pstRpcMsg,
					   unsigned char *response,
					   int *responselen)
{

	CVI_S32 s32Ret;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);

	s32Ret = CVI_AI_ClrPubAttr(dev);
	if (s32Ret) {
		printf("CVI_AI_ClrPubAttr failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}



CVI_S32 rpc_server_aud_ai_disable_vqe(struct rpc_aud_msg *pstRpcMsg,
					  unsigned char *response,
					  int *responselen)
{

	CVI_S32 s32Ret;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);
	AI_CHN chn = GET_CHN(pstRpcMsg->msg);

	s32Ret = CVI_AI_DisableVqe(dev, chn);
	if (s32Ret) {
		printf("CVI_AI_DisableVqe failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}


CVI_S32 rpc_server_aud_ai_disable(struct rpc_aud_msg *pstRpcMsg,
				  unsigned char *response,
				  int *responselen)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	AUDIO_DEV dev = GET_DEV(pstRpcMsg->msg);

	s32Ret = CVI_AI_Disable(dev);
	if (s32Ret) {
		printf("CVI_AI_Disable failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}


CVI_S32 rpc_server_aud_aenc_destroy_chn(struct rpc_aud_msg *pstRpcMsg,
					unsigned char *response,
					int *responselen)
{
	CVI_S32 s32Ret;
	AENC_CHN AeChn = GET_CHN(pstRpcMsg->msg);

	s32Ret = CVI_AENC_DestroyChn(AeChn);
	if (s32Ret) {
		printf("CVI_AENC_DestroyChn failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	*responselen = sizeof(struct rpc_aud_msg);
	pstRpcMsg->body_len = (*responselen);
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
#ifdef SHARE_MEM_IN_AUDIO_RPC
	pthread_join(s_pthAudStream[AeChn], NULL);
	stAencStreamShmInfo[AeChn].bRuning = 0;
	deinit_aenc_share_memory(AeChn);
#endif
	return 0;
}

CVI_S32 rpc_server_aud_aenc_create_chn(struct rpc_aud_msg *pstRpcMsg,
					   unsigned char *response,
					   int *responselen)
{
	CVI_S32 s32Ret;
	AENC_CHN AeChn = GET_CHN(pstRpcMsg->msg);
	AENC_CHN_ATTR_S stAencAttr, *pstAencAttr = &stAencAttr;

	memcpy(pstAencAttr, (AENC_CHN_ATTR_S *)pstRpcMsg->body,
		   sizeof(AENC_CHN_ATTR_S));
	printf("#########rpc_server_aud_aenc_create_chn\n");
	s32Ret = CVI_AENC_CreateChn(AeChn, pstAencAttr);
	if (s32Ret) {
		printf("CVI_AENC_CreateChn failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;
	printf("##5555555555rpc_server_aud_aenc_create_chn\n");
	*responselen = sizeof(struct rpc_aud_msg);
	pstRpcMsg->body_len = (*responselen);
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	return 0;
}

CVI_S32 rpc_server_aud_ao_get_mute(struct rpc_aud_msg *pstRpcMsg,
				   unsigned char *response,
				   int *responselen)
{
	CVI_S32 s32Ret;
	AUDIO_DEV AoDevId = GET_DEV(pstRpcMsg->msg);

	CVI_S32 s32Bool = 0;
	CVI_BOOL bEnable;
	AUDIO_FADE_S stFade, *pstFade = &stFade;

	s32Ret = CVI_AO_GetMute(AoDevId, &bEnable, pstFade);
	if (s32Ret) {
		printf("CVI_AO_SetMute failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	if (bEnable == CVI_TRUE)
		s32Bool = 1;
	else
		s32Bool = 0;

	*responselen = sizeof(struct rpc_aud_msg) + sizeof(CVI_S32) + sizeof(
				   AUDIO_FADE_S);
	pstRpcMsg->body_len = (*responselen);
	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	memcpy(response + sizeof(struct rpc_aud_msg), &s32Bool, sizeof(CVI_S32));
	memcpy(response + sizeof(struct rpc_aud_msg) + sizeof(CVI_S32), pstFade,
		   sizeof(AUDIO_FADE_S));
	return 0;

}

CVI_S32 rpc_server_aud_ao_set_mute(struct rpc_aud_msg *pstRpcMsg,
				   unsigned char *response,
				   int *responselen)
{
	CVI_S32 s32Ret;
	AUDIO_DEV AoDevId = GET_DEV(pstRpcMsg->msg);
	CVI_S32 s32Enable, *ps32Enable = &s32Enable;
	AUDIO_FADE_S stFade;
	AUDIO_FADE_S *pstFade = &stFade;
	CVI_BOOL bEnable = CVI_FALSE;

	memcpy(ps32Enable, pstRpcMsg->body, sizeof(CVI_S32));
	memcpy(pstFade, (pstRpcMsg->body + sizeof(CVI_S32)), sizeof(AUDIO_FADE_S));

	if (s32Enable == 1) {
		bEnable = CVI_TRUE;
	} else if (s32Enable == 0) {
		bEnable = CVI_FALSE;
	} else
		printf("Warning enable value[%d] in [%s]\n", s32Enable, __func__);


	s32Ret = CVI_AO_SetMute(AoDevId, bEnable, pstFade);
	if (s32Ret) {
		printf("CVI_AO_SetMute failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;

}

CVI_S32 rpc_server_aud_ao_enable(struct rpc_aud_msg *pstRpcMsg,
				 unsigned char *response,
				 int *responselen)
{
	CVI_S32 s32Ret;
	AUDIO_DEV AoDevId = GET_DEV(pstRpcMsg->msg);

	s32Ret = CVI_AO_Enable(AoDevId);
	if (s32Ret) {
		printf("CVI_AO_Enable failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;

}

CVI_S32 rpc_server_aud_ao_disable(struct rpc_aud_msg *pstRpcMsg,
				  unsigned char *response,
				  int *responselen)
{
	CVI_S32 s32Ret;
	AUDIO_DEV AoDevId = GET_DEV(pstRpcMsg->msg);

	s32Ret = CVI_AO_Disable(AoDevId);
	if (s32Ret) {
		printf("CVI_AO_Disable failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;

}


CVI_S32 rpc_server_aud_ai_enable_chn(struct rpc_aud_msg *pstRpcMsg,
					 unsigned char *response,
					 int *responselen)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	AUDIO_DEV AiDevId = GET_DEV(pstRpcMsg->msg);
	AI_CHN AiChn = GET_CHN(pstRpcMsg->msg);

	s32Ret = CVI_AI_EnableChn(AiDevId, AiChn);
	if (s32Ret) {
		printf("CVI_AI_EnableChn failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}



CVI_S32 rpc_server_aud_ao_enable_chn(struct rpc_aud_msg *pstRpcMsg,
					 unsigned char *response,
					 int *responselen)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	AUDIO_DEV AoDevId = GET_DEV(pstRpcMsg->msg);
	AI_CHN AoChn = GET_CHN(pstRpcMsg->msg);

	s32Ret = CVI_AO_EnableChn(AoDevId, AoChn);
	if (s32Ret) {
		printf("CVI_AO_EnableChn failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}

CVI_S32 rpc_server_aud_ai_disable_chn(struct rpc_aud_msg *pstRpcMsg,
					  unsigned char *response,
					  int *responselen)
{
	// x_info("enter");
	CVI_S32 s32Ret;
	AUDIO_DEV AiDevId = GET_DEV(pstRpcMsg->msg);
	AI_CHN AiChn = GET_CHN(pstRpcMsg->msg);

#ifdef SHARE_MEM_IN_AUDIO_RPC

	AUD_RPC_UNUSED(AiDevId);
	deinit_ain_share_memory(AiChn);
#endif

	printf("######rpc_server_aud_ai_disable_chn");
	s32Ret = CVI_AI_DisableChn(AiDevId, AiChn);
	if (s32Ret) {
		printf("CVI_AI_DisableChn failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}


CVI_S32 rpc_server_aud_ao_disable_chn(struct rpc_aud_msg *pstRpcMsg,
					  unsigned char *response,
					  int *responselen)
{
	CVI_S32 s32Ret;
	AUDIO_DEV AoDevId = GET_DEV(pstRpcMsg->msg);
	AI_CHN AoChn = GET_CHN(pstRpcMsg->msg);

#ifdef SHARE_MEM_IN_AUDIO_RPC

	deinit_ao_share_memory(AoChn);
#endif

	s32Ret = CVI_AO_DisableChn(AoDevId, AoChn);
	if (s32Ret) {
		printf("CVI_AO_DisableChn failed with %#x\n", s32Ret);
		pstRpcMsg->result = s32Ret;
	} else
		pstRpcMsg->result = CVI_SUCCESS;

	memcpy(response, pstRpcMsg, sizeof(struct rpc_aud_msg));
	*responselen = sizeof(struct rpc_aud_msg);
	return 0;
}

//implement server/client response function-------end
int rpc_server_aud_request_proc(unsigned char *request, int requestlen,
				unsigned char *response, int *responselen)
{
	int cmd;
	struct rpc_aud_msg *pstRpcMsg = (struct rpc_aud_msg *)request;

	AUD_RPC_UNUSED(requestlen);
	cmd = GET_CMD(pstRpcMsg->msg);

	if (pstRpcMsg->magic != AUD_RPC_MAGIC) {
		printf("magic error\n");
		return -1;
	}
#if CHECK_AUD_SERVER_CMD
	printf("[%s]cmd0:[%d]\n", __func__, cmd);
#endif

	switch (cmd) {
	case RPC_CMD_AUD_SYS_BIND:
		rpc_server_aud_sys_bind(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AUD_SYS_UNBIND:
		rpc_server_aud_sys_unbind(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_SET_PUB_ATTR:
		rpc_server_aud_ai_set_pub_attr(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_GET_PUB_ATTR:
		rpc_server_aud_ai_get_pub_attr(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_ENABLE:
		rpc_server_aud_ai_enable(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_DISABLE:
		rpc_server_aud_ai_disable(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_ENABLE_CHN:
		rpc_server_aud_ai_enable_chn(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_DISABLE_CHN:
		rpc_server_aud_ai_disable_chn(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_GET_FRAME:
		rpc_server_aud_ai_get_frame(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_RELEASE_FRAME:
		rpc_server_aud_ai_release_frame(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_SET_CHN_PARAM:
		rpc_server_aud_ai_set_chn_param(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_GET_CHN_PARAM:
		rpc_server_aud_ai_get_chn_param(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_SET_VOLUME:
		rpc_server_aud_ai_set_volume(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_GET_VOLUME:
		rpc_server_aud_ai_get_volume(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_ENABLE_VQE:
		rpc_server_aud_ai_enable_vqe(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_DISABLE_VQE:
		rpc_server_aud_ai_disable_vqe(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_ENABLE_RESMP:
		rpc_server_aud_ai_enable_resmp(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_DISABLE_RESMP:
		rpc_server_aud_ai_disable_resmp(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_CLR_PUB_ATTR:
		rpc_server_aud_ai_clr_pub_attr(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_SET_TALK_VQE_ATTR:
		rpc_server_aud_ai_set_talk_vqe_attr(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AI_GET_TALK_VQE_ATTR:
		rpc_server_aud_ai_get_talk_vqe_attr(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AO_SET_PUB_ATTR:
		rpc_server_aud_ao_set_pub_attr(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AO_GET_PUB_ATTR:
		rpc_server_aud_ao_get_pub_attr(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AO_ENABLE:
		rpc_server_aud_ao_enable(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AO_DISABLE:
		rpc_server_aud_ao_disable(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AO_ENABLE_CHN:
		rpc_server_aud_ao_enable_chn(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AO_DISABLE_CHN:
		rpc_server_aud_ao_disable_chn(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AO_SEND_FRAME:
		rpc_server_aud_ao_send_frame(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AO_ENABLE_RESMP:
		rpc_server_aud_ao_enable_resmp(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AO_DISABLE_RESMP:
		rpc_server_aud_ao_disable_resmp(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AO_SET_VOLUME:
		rpc_server_aud_ao_set_volume(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AO_GET_VOLUME:
		rpc_server_aud_ao_get_volume(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AO_SET_MUTE:
		rpc_server_aud_ao_set_mute(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AO_GET_MUTE:
		rpc_server_aud_ao_get_mute(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AENC_CREATE_CHN:
		rpc_server_aud_aenc_create_chn(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AENC_DESTROY_CHN:
		rpc_server_aud_aenc_destroy_chn(pstRpcMsg, response, responselen);
		break;
	case PRC_CMD_AENC_SEND_FRAME:
		rpc_server_aud_aenc_send_frame(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AENC_GET_STREAM:
		rpc_server_aud_aenc_get_stream(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AENC_RELEASE_STREAM:
		rpc_server_aud_aenc_release_stream(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AENC_REGISTER_EXTERNAL_ENCODER:
		break;
	case RPC_CMD_AENC_UNREGISTER_EXTERNAL_ENCODER:
		break;
	case RPC_CMD_ADEC_CREATE_CHN:
		rpc_server_aud_adec_create_chn(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_ADEC_DESTROY_CHN:
		rpc_server_aud_adec_destroy_chn(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_ADEC_SEND_STREAM:
		rpc_server_aud_adec_send_stream(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_ADEC_REGISTER_EXTERNAL_DECODER:
		break;
	case RPC_CMD_ADEC_UNREGISTER_EXTERNAL_DECODER:
		break;
	case RPC_CMD_ADEC_GET_FRAME:
		rpc_server_aud_adec_get_frame(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_ADEC_RELEASE_FRAME:
		rpc_server_aud_adec_release_frame(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_ADEC_SEND_END_OF_STREAM:
		rpc_server_aud_adec_send_end_of_stream(pstRpcMsg, response, responselen);
		break;
	default:
		break;
	}

	//s_rpc_process_func(cmd,pstRpcMsg, response, responselen);

	return 0;
}


//implement server/client response function-------end
int rpc_server_aud_request_procAenc(unsigned char *request, int requestlen,
					unsigned char *response, int *responselen)
{
	int cmd;
	struct rpc_aud_msg *pstRpcMsg = (struct rpc_aud_msg *)request;

	AUD_RPC_UNUSED(requestlen);
	cmd = GET_CMD(pstRpcMsg->msg);

	if (pstRpcMsg->magic != AUD_RPC_MAGIC) {
		printf("magic error\n");
		return -1;
	}
#if CHECK_AUD_SERVER_CMD
	printf("[%s]cmd1:[%d]\n", __func__, cmd);
#endif

	switch (cmd) {
	case RPC_CMD_AUD_SYS_BIND:
		rpc_server_aud_sys_bind(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AUD_SYS_UNBIND:
		rpc_server_aud_sys_unbind(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AENC_CREATE_CHN:
		rpc_server_aud_aenc_create_chn(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AENC_DESTROY_CHN:
		rpc_server_aud_aenc_destroy_chn(pstRpcMsg, response, responselen);
		break;
	case PRC_CMD_AENC_SEND_FRAME:
		rpc_server_aud_aenc_send_frame(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AENC_GET_STREAM:
		rpc_server_aud_aenc_get_stream(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AENC_RELEASE_STREAM:
		rpc_server_aud_aenc_release_stream(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_AENC_REGISTER_EXTERNAL_ENCODER:
		break;
	case RPC_CMD_AENC_UNREGISTER_EXTERNAL_ENCODER:
		break;
	default:
		printf("[%s][%d]...wrong rpc server for cmd[%d]\n",
			   __func__, __LINE__, cmd);
		break;
	}

	//s_rpc_process_func(cmd,pstRpcMsg, response, responselen);

	return 0;
}



//implement server/client response function-------end
int rpc_server_aud_request_procAdec(unsigned char *request, int requestlen,
					unsigned char *response, int *responselen)
{
	int cmd;
	struct rpc_aud_msg *pstRpcMsg = (struct rpc_aud_msg *)request;

	AUD_RPC_UNUSED(requestlen);
	cmd = GET_CMD(pstRpcMsg->msg);

	if (pstRpcMsg->magic != AUD_RPC_MAGIC) {
		printf("magic error\n");
		return -1;
	}
#if CHECK_AUD_SERVER_CMD
	printf("[%s]cmd2:[%d]\n", __func__, cmd);
#endif
	switch (cmd) {
	case RPC_CMD_ADEC_CREATE_CHN:
		rpc_server_aud_adec_create_chn(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_ADEC_DESTROY_CHN:
		rpc_server_aud_adec_destroy_chn(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_ADEC_SEND_STREAM:
		rpc_server_aud_adec_send_stream(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_ADEC_REGISTER_EXTERNAL_DECODER:
		break;
	case RPC_CMD_ADEC_UNREGISTER_EXTERNAL_DECODER:
		break;
	case RPC_CMD_ADEC_GET_FRAME:
		rpc_server_aud_adec_get_frame(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_ADEC_RELEASE_FRAME:
		rpc_server_aud_adec_release_frame(pstRpcMsg, response, responselen);
		break;
	case RPC_CMD_ADEC_SEND_END_OF_STREAM:
		rpc_server_aud_adec_send_end_of_stream(pstRpcMsg, response, responselen);
		break;
	default:
		printf("[%s][%d]...wrong rpc server for cmd[%d]\n",
			   __func__, __LINE__, cmd);
		break;
	}

	return 0;
}
void *rpc_server_aud_listen(void *args)
{
	unsigned char request[4096];
	unsigned char response[4096];
	int responselen;
	int rc;
	int fd = *((int *)args);

	free(args);


	while (s_rpc_server_run) {

		memset(request, 0, sizeof(request));
		memset(response, 0, sizeof(response));
		rc = nn_recv(fd, request, sizeof(request), 0);

		if (rc == ETIMEDOUT) {
			//timeout condition match the definition
			continue;
		}

		if (rc <= 0) {
			/*  Any error here is unexpected. */
			if ((rc == (-1)) || (rc == 0)) {
				//timeout condition
				continue;
			} else {
				fprintf(stderr, "nn_recv: %s\n", nn_strerror(nn_errno()));
				break;
			}
		}
		responselen = 0;
		rpc_server_aud_request_proc(request, rc, response, &responselen);

		rc = nn_send(fd, response, responselen, 0);
		if (rc < 0) {
			fprintf(stderr, "nn_send: %s (ignoring)\n",
				nn_strerror(nn_errno()));
		}

	}
	nn_close(fd);
	return NULL;
}


void *rpc_server_aud_listenAenc(void *args)
{
	unsigned char request[4096];
	unsigned char response[4096];
	int responselen;
	int rc;
	int fd = *((int *)args);

	free(args);

	while (s_rpc_server_run) {
		memset(request, 0, sizeof(request));
		memset(response, 0, sizeof(response));
		rc = nn_recv(fd, request, sizeof(request), 0);
		if (rc == ETIMEDOUT) {
			//timeout condition match the definition
			continue;
		}

		if (rc <= 0) {
			/*  Any error here is unexpected. */
			if ((rc == (-1)) || (rc == 0)) {
				//timeout condition
				continue;
			} else {
				fprintf(stderr, "nn_recv: %s\n", nn_strerror(nn_errno()));
				break;
			}
		}
		responselen = 0;
		rpc_server_aud_request_procAenc(request, rc, response, &responselen);

		rc = nn_send(fd, response, responselen, 0);
		if (rc < 0) {
			fprintf(stderr, "nn_send: %s (ignoring)\n",
				nn_strerror(nn_errno()));
		}
	}
	nn_close(fd);
	return NULL;
}



void *rpc_server_aud_listenAdec(void *args)
{
	unsigned char request[4096];
	unsigned char response[4096];
	int responselen;
	int rc;
	int fd = *((int *)args);

	free(args);

	while (s_rpc_server_run) {

		memset(request, 0, sizeof(request));
		memset(response, 0, sizeof(response));
		rc = nn_recv(fd, request, sizeof(request), 0);
		if (rc == ETIMEDOUT) {
			//timeout condition match the definition
			//printf("[%s][%d]continue, rc:%d, fd:%d\n", __func__, __LINE__, rc, fd);
			continue;
		}

		if (rc <= 0) {
			/*  Any error here is unexpected. */
			if ((rc == (-1)) || (rc == 0)) {
				//printf("[%s][%d]continue, rc:%d\n", __func__, __LINE__, rc);
				continue;
			} else {
				fprintf(stderr, "nn_recv: %s\n", nn_strerror(nn_errno()));
				break;
			}
		}
		responselen = 0;
		rpc_server_aud_request_procAdec(request, rc, response, &responselen);
		rc = nn_send(fd, response, responselen, 0);
		if (rc < 0) {
			fprintf(stderr, "nn_send: %s (ignoring)\n",
				nn_strerror(nn_errno()));
		}
	}
	nn_close(fd);
	return NULL;
}
int rpc_server_audio_init(void)
{
	int fd;
	char const *rpc_url_all[] = {RPC_URL_AUDCH0, RPC_URL_AUDCH1,
			RPC_URL_AUDCH2, RPC_URL_AUDAIO_CH1, RPC_URL_AUDAIO_CH2};
	int ServerCnt = AUD_RPC_SERVER_CNT;
	cviAudioGetDbgMask(&cviaud_dbg);

	for (int i = 0; i < ServerCnt; i++) {
		/*  Create the socket. */
		fd = nn_socket(AF_SP, NN_REP);
		if (fd < 0) {
			fprintf(stderr, "nn_socket: %s\n", nn_strerror(nn_errno()));
			return (-1);
		}

		/*  Bind to the URL.  This will bind to the address and listen
		 *  synchronously; new clients will be accepted asynchronously
		 *  without further action from the calling program.
		 */

		if (nn_bind(fd, rpc_url_all[i]) < 0) {
			fprintf(stderr, "nn_bind: %s\n", nn_strerror(nn_errno()));
			nn_close(fd);
			return (-1);
		}

		int timeout = 1000;

		if (nn_setsockopt(fd, NN_SOL_SOCKET, NN_RCVTIMEO, &timeout, sizeof(int)) < 0) {
			fprintf(stderr, "nn_setsockopt setup timeout[%d]ms: %s\n", timeout, nn_strerror(nn_errno()));
			nn_close(fd);
			return (-1);
		}

		void *p = malloc(sizeof(int));
		int *pfd = (int *)p;
		*pfd = fd;

		s_rpc_server_run = true;
		if (i == AUD_RPC_URL_AIO || i == AUD_RPC_URL_AIO_CH1 || i == AUD_RPC_URL_AIO_CH2) {
			if (pthread_create(&s_pthRpc[i], NULL, rpc_server_aud_listen, p) < 0) {
				fprintf(stderr, "pthread_create failed i[%d]\n", i);
				free(p);
				nn_close(fd);
			}
		} else if (i == AUD_RPC_URL_AENC) {
			if (pthread_create(&s_pthRpc[i], NULL, rpc_server_aud_listenAenc, p) < 0) {
				fprintf(stderr, "pthread_create failed\n");
				free(p);
				nn_close(fd);
			}
		} else if (i == AUD_RPC_URL_ADEC) {
			if (pthread_create(&s_pthRpc[i], NULL, rpc_server_aud_listenAdec, p) < 0) {
				fprintf(stderr, "pthread_create failed\n");
				free(p);
				nn_close(fd);
			}
		}
	}

	s_master = true;
	printf("rpc_server_init\n");
#if DUMP_SERVER_CLIENT
	fd_server = fopen("server.raw", "wb");
#endif
	return 0;
}

int rpc_server_audio_deinit(void)
{
	s_rpc_server_run = false;
	int ServerCnt = AUD_RPC_SERVER_CNT;

	for (int i = 0; i < ServerCnt; i++) {
		if (s_pthRpc[i] != NULL) {
			pthread_join(s_pthRpc[i], NULL);
			s_pthRpc[i] = 0;
		}
	}
	printf("AUD rpc_server_deinit\n");
	return 0;
}

bool isAudioMaster(void)
{
	return s_master;
}
