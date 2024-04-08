#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cvi_audio_shm.h"

//function definition
uint64_t rpc_aud_get_boot_time(void)
{
	struct timespec timeo;
	uint64_t time_us = 0;

	clock_gettime(CLOCK_MONOTONIC, &timeo);
	time_us = (uint64_t)timeo.tv_sec * 1000000 + timeo.tv_nsec / 1000;
	return time_us;
}

int checkIsAencStreamHead(AUD_AENC_STREAM_CBLK *pstCblk)
{
	if (pstCblk && (pstCblk->s32SyncCode == 0xa5a5))
		return 1;

	return 0;
}

int checkIsAinFrameHead(AUD_AIN_FRAME_CBLK *pstCblk)
{
	if (pstCblk && (pstCblk->s32SyncCode == 0xa1a1))
		return 1;

	return 0;
}

int checkIsAoFrameHead(AUD_AO_FRAME_CBLK *pstCblk)
{
	if (pstCblk && (pstCblk->s32SyncCode == 0xa2a2))
		return 1;

	return 0;
}

int checkIsAdecFrameHead(AUD_ADEC_FRAME_CBLK *pstCblk)
{
	if (pstCblk && (pstCblk->s32SyncCode == 0xa3a3))
		return 1;

	return 0;
}


int init_aenc_share_memory(AENC_CHN AeChn)
{
	printf("init share memory for aenc chn[%d]\n", AeChn);
	memset(stAencStreamShmInfo[AeChn].cAencStrmShmName, 0,
		MAX_RPC_AUD_STREAM_NAME_LEN);

	snprintf(stAencStreamShmInfo[AeChn].cAencStrmShmName,
		MAX_RPC_AUD_STREAM_NAME_LEN,
		"%s_%d",
		RPC_AUD_STREAM_SHM_NAME,
		AeChn);

	memset(stAencStreamShmInfo[AeChn].cAencSemReadName,
		0,
		MAX_RPC_AUD_STREAM_NAME_LEN);

	snprintf(stAencStreamShmInfo[AeChn].cAencSemReadName,
		MAX_RPC_AUD_STREAM_NAME_LEN,
		"%s_%d",
		RPC_AUD_STREAM_SEM_READ_NAME,
		AeChn);

	memset(stAencStreamShmInfo[AeChn].cAencSemWriteName,
		0,
		MAX_RPC_AUD_STREAM_NAME_LEN);

	snprintf(stAencStreamShmInfo[AeChn].cAencSemWriteName,
		MAX_RPC_AUD_STREAM_NAME_LEN,
		"%s_%d",
		RPC_AUD_STREAM_SEM_WRITE_NAME, AeChn);


	stAencStreamShmInfo[AeChn].g_shm_fd = open(stAencStreamShmInfo[AeChn].cAencStrmShmName, O_CREAT | O_RDWR, 0777);
	if (stAencStreamShmInfo[AeChn].g_shm_fd < 0) {
		perror("open");
		return -1;
	}

	close(stAencStreamShmInfo[AeChn].g_shm_fd);

	stAencStreamShmInfo[AeChn].g_shm_key = ftok(stAencStreamShmInfo[AeChn].cAencStrmShmName, 20 + AeChn);
	stAencStreamShmInfo[AeChn].g_shm_id =
		shmget(stAencStreamShmInfo[AeChn].g_shm_key,
			RPC_AUD_STREAM_SHARED_MEMORY_SIZE,
			IPC_CREAT | 0777);
	if (stAencStreamShmInfo[AeChn].g_shm_id < 0 || stAencStreamShmInfo[AeChn].g_shm_key < 0) {
		printf("aenc shmget[%d, %d]\n", stAencStreamShmInfo[AeChn].g_shm_key,
			stAencStreamShmInfo[AeChn].g_shm_id);
		return -1;
	}

	stAencStreamShmInfo[AeChn].m_pSemRead =
		sem_open(stAencStreamShmInfo[AeChn].cAencSemReadName,
			O_RDWR | O_CREAT, 0644, 1);

	if (!stAencStreamShmInfo[AeChn].m_pSemRead) {
		perror("client sem open error.");
		return -1;
	}

	stAencStreamShmInfo[AeChn].m_pSemWrite =
		sem_open(stAencStreamShmInfo[AeChn].cAencSemWriteName,
			O_RDWR | O_CREAT, 0644, 1);

	if (!stAencStreamShmInfo[AeChn].m_pSemWrite) {
		perror("client sem open error.");
		return -1;
	}

	stAencStreamShmInfo[AeChn].g_shm_ptr = (char *)shmat(stAencStreamShmInfo[AeChn].g_shm_id, NULL, 0);
	if (!stAencStreamShmInfo[AeChn].g_shm_ptr) {
		printf("sharem get failed.\n");
		return -1;
	}

	stAencStreamShmInfo[AeChn].streamPtr = (CVI_U8 *)malloc(RPC_AUD_STREAM_ONEFRMAE_MAX_SIZE);
	if (!stAencStreamShmInfo[AeChn].streamPtr) {
		printf("malloc  failed.\n");
		return -1;
	}

	stAencStreamShmInfo[AeChn].m_queue_index = 0;
	stAencStreamShmInfo[AeChn].m_queue_cap =
		RPC_AUD_STREAM_SHARED_MEMORY_SIZE / RPC_AUD_STREAM_ONEFRMAE_MAX_SIZE;
	stAencStreamShmInfo[AeChn].channel = AeChn;
	stAencStreamShmInfo[AeChn].bInit = 1;

	return 1;
}

int deinit_aenc_share_memory(AENC_CHN AeChn)
{
	stAencStreamShmInfo[AeChn].bRuning = 0;

	if (stAencStreamShmInfo[AeChn].streamPtr) {
		free(stAencStreamShmInfo[AeChn].streamPtr);
		stAencStreamShmInfo[AeChn].streamPtr = NULL;
	}

	if (stAencStreamShmInfo[AeChn].g_shm_ptr)
		shmdt(stAencStreamShmInfo[AeChn].g_shm_ptr);

	if (stAencStreamShmInfo[AeChn].g_shm_id)
		shmctl(stAencStreamShmInfo[AeChn].g_shm_id, IPC_RMID, NULL);

	if (stAencStreamShmInfo[AeChn].m_pSemRead) {
		sem_close(stAencStreamShmInfo[AeChn].m_pSemRead);
		sem_unlink(stAencStreamShmInfo[AeChn].cAencSemReadName);
	}

	if (stAencStreamShmInfo[AeChn].m_pSemWrite) {
		sem_close(stAencStreamShmInfo[AeChn].m_pSemWrite);
		sem_unlink(stAencStreamShmInfo[AeChn].cAencSemWriteName);
	}

	return 1;
}

int deinit_ain_share_memory(AI_CHN AiChn)
{
	stAinStreamShmInfo[AiChn].bRuning = 0;
	if (stAinStreamShmInfo[AiChn].streamPtr) {
		free(stAinStreamShmInfo[AiChn].streamPtr);
		stAinStreamShmInfo[AiChn].streamPtr = NULL;
	}

	if (stAinStreamShmInfo[AiChn].g_shm_ptr) {
		shmdt(stAinStreamShmInfo[AiChn].g_shm_ptr);
	}

	if (stAinStreamShmInfo[AiChn].g_shm_id)
		shmctl(stAinStreamShmInfo[AiChn].g_shm_id, IPC_RMID, NULL);

	if (stAinStreamShmInfo[AiChn].m_pSemRead) {
		sem_close(stAinStreamShmInfo[AiChn].m_pSemRead);
		sem_unlink(stAinStreamShmInfo[AiChn].cAinSemReadName);
	}

	if (stAinStreamShmInfo[AiChn].m_pSemWrite) {
		sem_close(stAinStreamShmInfo[AiChn].m_pSemWrite);
		sem_unlink(stAinStreamShmInfo[AiChn].cAinSemWriteName);
	}

	return 1;
}


int init_ain_share_memory(AI_CHN AiChn)
{

	printf("init share memory for Aichn[%d]\n", AiChn);
	snprintf(stAinStreamShmInfo[AiChn].cAinStreamShmName, MAX_RPC_AUD_STREAM_NAME_LEN, "%s_%d",
			RPC_AI_STREAM_SHM_NAME, AiChn);
	memset(stAinStreamShmInfo[AiChn].cAinSemReadName, 0, MAX_RPC_AUD_STREAM_NAME_LEN);
	snprintf(stAinStreamShmInfo[AiChn].cAinSemReadName, MAX_RPC_AUD_STREAM_NAME_LEN, "%s_%d",
			RPC_AI_STREAM_SEM_READ_NAME, AiChn);
	memset(stAinStreamShmInfo[AiChn].cAinSemWriteName, 0, MAX_RPC_AUD_STREAM_NAME_LEN);
	snprintf(stAinStreamShmInfo[AiChn].cAinSemWriteName, MAX_RPC_AUD_STREAM_NAME_LEN, "%s_%d",
			RPC_AI_STREAM_SEM_WRITE_NAME, AiChn);


	stAinStreamShmInfo[AiChn].g_shm_fd = open(stAinStreamShmInfo[AiChn].cAinStreamShmName,
					    O_CREAT | O_RDWR, 0777);
	if (stAinStreamShmInfo[AiChn].g_shm_fd < 0) {
		perror("ai share mem open faile");
		return -1;
	}
	close(stAinStreamShmInfo[AiChn].g_shm_fd);

	stAinStreamShmInfo[AiChn].g_shm_key = ftok(stAinStreamShmInfo[AiChn].cAinStreamShmName, 20 + AiChn);
	stAinStreamShmInfo[AiChn].g_shm_id = shmget(stAinStreamShmInfo[AiChn].g_shm_key,
						RPC_AUD_STREAM_SHARED_MEMORY_SIZE, IPC_CREAT | 0777);
	if (stAinStreamShmInfo[AiChn].g_shm_id < 0 || stAinStreamShmInfo[AiChn].g_shm_key < 0) {
		printf("client ai shmget[%d, %d]\n",
			stAinStreamShmInfo[AiChn].g_shm_key, stAinStreamShmInfo[AiChn].g_shm_id);
		return -1;
	}

	stAinStreamShmInfo[AiChn].m_pSemRead = sem_open(stAinStreamShmInfo[AiChn].cAinSemReadName,
							O_RDWR | O_CREAT, 0644, 1);
	if (!stAinStreamShmInfo[AiChn].m_pSemRead) {
		perror("ai client sem open error.");
		return -1;
	}

	stAinStreamShmInfo[AiChn].m_pSemWrite = sem_open(stAinStreamShmInfo[AiChn].cAinSemWriteName,
							O_RDWR | O_CREAT, 0644, 1);
	if (!stAinStreamShmInfo[AiChn].m_pSemWrite) {
		perror("ai client sem open error.");
		return -1;
	}

	stAinStreamShmInfo[AiChn].g_shm_ptr = (char *)shmat(
			stAinStreamShmInfo[AiChn].g_shm_id, NULL, 0);
	if (!stAinStreamShmInfo[AiChn].g_shm_ptr) {
		printf("ai client sharem get failed.\n");
		return -1;
	}
	stAinStreamShmInfo[AiChn].streamPtr = (CVI_U8 *)malloc(
			RPC_AUD_STREAM_ONEFRMAE_MAX_SIZE);
	if (!stAinStreamShmInfo[AiChn].streamPtr) {
		printf("malloc	failed.\n");
		return -1;
	}

	stAinStreamShmInfo[AiChn].m_queue_index = 0;
	stAinStreamShmInfo[AiChn].m_queue_cap = RPC_AUD_STREAM_SHARED_MEMORY_SIZE /
							RPC_AUD_STREAM_ONEFRMAE_MAX_SIZE;
	//stAinStreamShmInfo[AiChn].bRuning = 1;
	stAinStreamShmInfo[AiChn].bInit = 1;
	return 1;
}


int init_ao_share_memory(AO_CHN AoChn)
{
	printf("init share memory for Aochn[%d]\n", AoChn);
	snprintf(stAoStreamShmInfo[AoChn].cAoStreamShmName, MAX_RPC_AUD_STREAM_NAME_LEN, "%s_%d",
			RPC_AO_STREAM_SHM_NAME, AoChn);
	memset(stAoStreamShmInfo[AoChn].cAoSemReadName, 0, MAX_RPC_AUD_STREAM_NAME_LEN);
	snprintf(stAoStreamShmInfo[AoChn].cAoSemReadName, MAX_RPC_AUD_STREAM_NAME_LEN, "%s_%d",
			RPC_AO_STREAM_SEM_READ_NAME, AoChn);
	memset(stAoStreamShmInfo[AoChn].cAoSemWriteName, 0, MAX_RPC_AUD_STREAM_NAME_LEN);
	snprintf(stAoStreamShmInfo[AoChn].cAoSemWriteName, MAX_RPC_AUD_STREAM_NAME_LEN, "%s_%d",
			RPC_AO_STREAM_SEM_WRITE_NAME, AoChn);


	stAoStreamShmInfo[AoChn].g_shm_fd = open(stAoStreamShmInfo[AoChn].cAoStreamShmName,
					O_CREAT | O_RDWR, 0777);
	if (stAoStreamShmInfo[AoChn].g_shm_fd < 0) {
		perror("ao share mem open faile");
		return -1;
	}
	close(stAoStreamShmInfo[AoChn].g_shm_fd);

	stAoStreamShmInfo[AoChn].g_shm_key = ftok(stAoStreamShmInfo[AoChn].cAoStreamShmName, 20 + AoChn);
	stAoStreamShmInfo[AoChn].g_shm_id = shmget(stAoStreamShmInfo[AoChn].g_shm_key,
						RPC_AUD_STREAM_SHARED_MEMORY_SIZE, IPC_CREAT | 0777);
	if (stAoStreamShmInfo[AoChn].g_shm_id < 0 || stAoStreamShmInfo[AoChn].g_shm_key < 0) {
		printf("ao shmget[%d, %d]\n", stAoStreamShmInfo[AoChn].g_shm_key, stAoStreamShmInfo[AoChn].g_shm_id);
		return -1;
	}

	stAoStreamShmInfo[AoChn].m_pSemRead = sem_open(stAoStreamShmInfo[AoChn].cAoSemReadName,
							O_RDWR | O_CREAT, 0644, 1);
	if (!stAoStreamShmInfo[AoChn].m_pSemRead) {
		perror("ao client sem open error.");
		printf("[Error][%s][%d]\n", __func__, __LINE__);
		return -1;
	}

	stAoStreamShmInfo[AoChn].m_pSemWrite = sem_open(stAoStreamShmInfo[AoChn].cAoSemWriteName,
							O_RDWR | O_CREAT, 0644, 1);
	if (!stAoStreamShmInfo[AoChn].m_pSemWrite) {
		perror("ao client sem open error.");
		printf("[Error][%s][%d]\n", __func__, __LINE__);
		return -1;
	}

	stAoStreamShmInfo[AoChn].g_shm_ptr = (char *)shmat(
			stAoStreamShmInfo[AoChn].g_shm_id, NULL, 0);
	if (!stAoStreamShmInfo[AoChn].g_shm_ptr) {
		printf("ao client sharem get failed.\n");
		printf("[Error][%s][%d]\n", __func__, __LINE__);
		return -1;
	}
	stAoStreamShmInfo[AoChn].streamPtr = (CVI_U8 *)malloc(
			RPC_AUD_STREAM_ONEFRMAE_MAX_SIZE);
	if (!stAoStreamShmInfo[AoChn].streamPtr) {
		printf("malloc	failed.\n");
		printf("[Error][%s][%d]\n", __func__, __LINE__);
		return -1;
	}

	stAoStreamShmInfo[AoChn].m_queue_index = 0;
	stAoStreamShmInfo[AoChn].m_queue_cap = RPC_AUD_STREAM_SHARED_MEMORY_SIZE /
							RPC_AUD_STREAM_ONEFRMAE_MAX_SIZE;
	//stAoStreamShmInfo[AoChn].bRuning = 1;
	stAoStreamShmInfo[AoChn].bInit = 1;

	return 1;
}

int deinit_ao_share_memory(AO_CHN AoChn)
{
	stAoStreamShmInfo[AoChn].bRuning = 0;
	if (stAoStreamShmInfo[AoChn].streamPtr) {
		free(stAoStreamShmInfo[AoChn].streamPtr);
		stAoStreamShmInfo[AoChn].streamPtr = NULL;
	}

	if (stAoStreamShmInfo[AoChn].g_shm_ptr) {
		shmdt(stAoStreamShmInfo[AoChn].g_shm_ptr);
	}
	if (stAoStreamShmInfo[AoChn].g_shm_id)
		shmctl(stAoStreamShmInfo[AoChn].g_shm_id, IPC_RMID, NULL);

	if (stAoStreamShmInfo[AoChn].m_pSemRead) {
		sem_close(stAoStreamShmInfo[AoChn].m_pSemRead);
		sem_unlink(stAoStreamShmInfo[AoChn].cAoSemReadName);
	}

	if (stAoStreamShmInfo[AoChn].m_pSemWrite) {
		sem_close(stAoStreamShmInfo[AoChn].m_pSemWrite);
		sem_unlink(stAoStreamShmInfo[AoChn].cAoSemWriteName);
	}

	return 1;
}




int init_adec_share_memory(ADEC_CHN AdChn)
{
	printf("init share memory for AdChn[%d]\n", AdChn);
	snprintf(stAdecStreamShmInfo[AdChn].cAdecStreamShmName, MAX_RPC_AUD_STREAM_NAME_LEN, "%s_%d",
			RPC_ADEC_STREAM_SHM_NAME, AdChn);
	memset(stAdecStreamShmInfo[AdChn].cAdecSemReadName, 0, MAX_RPC_AUD_STREAM_NAME_LEN);
	snprintf(stAdecStreamShmInfo[AdChn].cAdecSemReadName, MAX_RPC_AUD_STREAM_NAME_LEN, "%s_%d",
			RPC_ADEC_STREAM_SEM_READ_NAME, AdChn);
	memset(stAdecStreamShmInfo[AdChn].cAdecSemWriteName, 0, MAX_RPC_AUD_STREAM_NAME_LEN);
	snprintf(stAdecStreamShmInfo[AdChn].cAdecSemWriteName, MAX_RPC_AUD_STREAM_NAME_LEN, "%s_%d",
			RPC_ADEC_STREAM_SEM_WRITE_NAME, AdChn);

	stAdecStreamShmInfo[AdChn].g_shm_fd = open(stAdecStreamShmInfo[AdChn].cAdecStreamShmName,
					O_CREAT | O_RDWR, 0777);
	if (stAdecStreamShmInfo[AdChn].g_shm_fd < 0) {
		perror("adec share mem open faile");
		return -1;
	}
	close(stAdecStreamShmInfo[AdChn].g_shm_fd);


	stAdecStreamShmInfo[AdChn].g_shm_key = ftok(stAdecStreamShmInfo[AdChn].cAdecStreamShmName, 20 + AdChn);
	stAdecStreamShmInfo[AdChn].g_shm_id = shmget(stAdecStreamShmInfo[AdChn].g_shm_key,
						RPC_AUD_STREAM_SHARED_MEMORY_SIZE, IPC_CREAT | 0777);
	if (stAdecStreamShmInfo[AdChn].g_shm_id < 0 || stAdecStreamShmInfo[AdChn].g_shm_key < 0) {
		printf("adec shmget[%d, %d]\n",
			stAdecStreamShmInfo[AdChn].g_shm_key, stAdecStreamShmInfo[AdChn].g_shm_id);
		return -1;
	}

	stAdecStreamShmInfo[AdChn].m_pSemRead = sem_open(stAdecStreamShmInfo[AdChn].cAdecSemReadName,
							O_RDWR | O_CREAT, 0644, 1);
	if (!stAdecStreamShmInfo[AdChn].m_pSemRead) {
		perror("adec client sem open error.");
		return -1;
	}

	stAdecStreamShmInfo[AdChn].m_pSemWrite = sem_open(stAdecStreamShmInfo[AdChn].cAdecSemWriteName,
							O_RDWR | O_CREAT, 0644, 1);
	if (!stAdecStreamShmInfo[AdChn].m_pSemWrite) {
		perror("adec client sem open error.");
		return -1;
	}

	stAdecStreamShmInfo[AdChn].g_shm_ptr = (char *)shmat(
			stAdecStreamShmInfo[AdChn].g_shm_id, NULL, 0);
	if (!stAdecStreamShmInfo[AdChn].g_shm_ptr) {
		printf("adec client sharem get failed.\n");
		return -1;
	}
	stAdecStreamShmInfo[AdChn].streamPtr = (CVI_U8 *)malloc(
			RPC_AUD_STREAM_ONEFRMAE_MAX_SIZE);
	if (!stAdecStreamShmInfo[AdChn].streamPtr) {
		printf("malloc	failed.\n");
		return -1;
	}

	stAdecStreamShmInfo[AdChn].m_queue_index = 0;
	stAdecStreamShmInfo[AdChn].m_queue_cap = RPC_AUD_STREAM_SHARED_MEMORY_SIZE /
							RPC_AUD_STREAM_ONEFRMAE_MAX_SIZE;
	//stAdecStreamShmInfo[AdChn].bRuning = 1;
	stAdecStreamShmInfo[AdChn].bInit = 1;

	return 1;
}

int deinit_adec_share_memory(ADEC_CHN AdChn)
{
	stAdecStreamShmInfo[AdChn].bRuning = 0;
	if (stAdecStreamShmInfo[AdChn].streamPtr) {
		free(stAdecStreamShmInfo[AdChn].streamPtr);
		stAdecStreamShmInfo[AdChn].streamPtr = NULL;
	}

	if (stAdecStreamShmInfo[AdChn].g_shm_ptr) {
		shmdt(stAdecStreamShmInfo[AdChn].g_shm_ptr);
	}

	if (stAdecStreamShmInfo[AdChn].g_shm_id)
		shmctl(stAdecStreamShmInfo[AdChn].g_shm_id, IPC_RMID, NULL);

	if (stAdecStreamShmInfo[AdChn].m_pSemRead) {
		sem_close(stAdecStreamShmInfo[AdChn].m_pSemRead);
		sem_unlink(stAdecStreamShmInfo[AdChn].cAdecSemReadName);
	}

	if (stAdecStreamShmInfo[AdChn].m_pSemWrite) {
		sem_close(stAdecStreamShmInfo[AdChn].m_pSemWrite);
		sem_unlink(stAdecStreamShmInfo[AdChn].cAdecSemWriteName);
	}

	return 1;
}


