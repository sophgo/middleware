#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/queue.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdatomic.h>
#include <sys/shm.h>
#include <fcntl.h> /* fcntl */
#include <unistd.h>
#include <inttypes.h>
#include <limits.h>
#include "atomic.h"
#include "alog.h"


//#if __GLIBC_PREREQ(2, 3)
#if defined FUTEX_WAIT || defined FUTEX_WAKE
#include <linux/futex.h>
#else
#define FUTEX_WAIT      0
#define FUTEX_WAKE      1
#endif

#ifndef __NR_futex
#define __NR_futex      240
#endif
//#endif

const struct timespec kForever = {INT_MAX /*tv_sec*/, 0 /*tv_nsec*/};
const struct timespec kNonBlocking = {0 /*tv_sec*/, 0 /*tv_nsec*/};


#define MAX_CHANNEL_COUNT  (32)
#define MAX_BUFFER_NAME_LEN (30)

#define SHM_CYCLEBUFFER_NAME "aud_share_shm"

#define CBLK_FUTEX_WAKE 1               // if event flag bit is set, then a deferred wake is pending

							 //note server here is for output
#define CBLK_FLAG_INIT				0x01
#define CBLK_FLAG_READ				0x02
#define CBLK_FLAG_WRITE				0x04
#define CBLK_FLAG_CHECKSIZE			0x08
#define CBLK_FLAG_RESET				0x10

#define CBLK_FLAG_INTERRUPT				0x8000

typedef struct ptread_sem {
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	int cnt;
} ptread_sem_s;


typedef struct {
				int32_t	syncCode;
				int32_t	mFutex;
				int32_t	mFlags;
				uint32_t	u32WritePos;
				uint32_t	u32ReadPos;
				uint32_t	u32Len;
				uint32_t	u32Boundary;
				char	data[0];
} AUDIO_CBLK;


typedef struct {
	int	bInit;
	int	g_shm_id;
	ptread_sem_s buffer_sem;
	AUDIO_CBLK *pCblk;
} SHARE_MEM_INFO;

static int mUseflag;
static SHARE_MEM_INFO gShareMemInfo[MAX_CHANNEL_COUNT] = {0};
static pthread_mutex_t g_share_init_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_share_mem_lock[MAX_CHANNEL_COUNT] = {
	[0 ... (MAX_CHANNEL_COUNT - 1)] = PTHREAD_MUTEX_INITIALIZER
};


static void cvi_sem_init(ptread_sem_s *sem)
{
	pthread_mutexattr_t ma;
	pthread_condattr_t cattr;

	pthread_condattr_init(&cattr);
	pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);

	pthread_mutexattr_init(&ma);
	pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);
	pthread_mutexattr_setrobust(&ma, PTHREAD_MUTEX_ROBUST);

	sem->cnt = 0;
	pthread_cond_init(&sem->cond, &cattr);
	pthread_mutex_init(&sem->mutex, &ma);

	//return;
}

static void cvi_sem_post(ptread_sem_s *sem)
{
	pthread_mutex_lock(&sem->mutex);
	sem->cnt++;
	pthread_cond_signal(&sem->cond);
	pthread_mutex_unlock(&sem->mutex);

	//return;
}

static int cvi_sem_wait(ptread_sem_s *sem, long msecs)
{
	struct timespec ts;
	unsigned long secs = msecs/1000;
	unsigned long add = 0;
	int ret;
	int s32RetStatus = 0;

	pthread_mutex_lock(&sem->mutex);
	if (sem->cnt > 0) {
		sem->cnt--;
		pthread_mutex_unlock(&sem->mutex);
		return 0;
	}

	if (msecs < 0) {
		ret = pthread_cond_wait(&sem->cond, &sem->mutex);
	} else if (msecs == 0) {
		ret = -1;
	} else {
		msecs = msecs%1000;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		msecs = msecs*1000*1000 + ts.tv_nsec;
		add = msecs / (1000*1000*1000);
		ts.tv_sec += (add + secs);
		ts.tv_nsec = msecs%(1000*1000*1000);

		ret = pthread_cond_timedwait(&sem->cond, &sem->mutex, &ts);
	}

	if (ret == 0) {
		if (sem->cnt > 0)
			sem->cnt--;
		s32RetStatus = 0;
	} else if (ret == EAGAIN || ret == ETIMEDOUT) {
		s32RetStatus = -2;
	} else {
		s32RetStatus = -1;
	}

	pthread_mutex_unlock(&sem->mutex);

	return s32RetStatus;
}

static void cvi_sem_destroy(ptread_sem_s *sem)
{
	pthread_cond_destroy(&sem->cond);
	pthread_mutex_destroy(&sem->mutex);
	//return;
}

static int get_valid_index(void)
{
	int flag = 0;

	for (int i = 1; i < MAX_CHANNEL_COUNT; i++) {
		flag = (1<<i) & mUseflag;//i=1 flag = 0;i=2; flag = 100 & 010 = 0;
		if (!flag) {
			mUseflag = mUseflag | (1<<i);//mUseflag = 10;mUseflag=110;
			return i;
		}
	}

	return -1;
}


int share_cyclebuffer_init(uint32_t length, int bClient)
{
	int index = -1;

	if (!length) {
		return -1;
	}
	pthread_mutex_lock(&g_share_init_lock);
	index = get_valid_index();
	if (index < 0) {
		pthread_mutex_unlock(&g_share_init_lock);
		return -2;
	}

	if (gShareMemInfo[index].bInit) {
		printf("%s index(%d) already inited.\n", __func__, index);
		goto INIT_ERROR;
	}

	gShareMemInfo[index].pCblk = (AUDIO_CBLK *)malloc(length+sizeof(AUDIO_CBLK));

	if (gShareMemInfo[index].pCblk == NULL) {
		perror("malloc failed.");
		goto INIT_ERROR;
	}

	if (!bClient) {
		memset(gShareMemInfo[index].pCblk, 0, sizeof(AUDIO_CBLK));
		gShareMemInfo[index].pCblk->mFutex = CBLK_FUTEX_WAKE;
		gShareMemInfo[index].pCblk->u32Len = length;
		gShareMemInfo[index].pCblk->u32Boundary = (1<<31) - (1<<31) % gShareMemInfo[index].pCblk->u32Len;
	}

	cvi_sem_init(&gShareMemInfo[index].buffer_sem);
	gShareMemInfo[index].bInit = 1;
	pthread_mutex_unlock(&g_share_init_lock);

	return index;

INIT_ERROR:
	gShareMemInfo[index].bInit = 0;
	mUseflag = mUseflag & (~(1<<index));
	pthread_mutex_unlock(&g_share_init_lock);
	return -1;

}

int share_cyclebuffer_client_write(uint32_t index, char *data, uint32_t writeLen, int32_t msecs)
{
	AUDIO_CBLK *pCblk;
	uint32_t u32ReadPos = 0;
	uint32_t u32WritePos = 0;
	int canWriteLen = 0;

	if (index > MAX_CHANNEL_COUNT) {
		return -1;
	}
	if (!gShareMemInfo[index].bInit || !data || !writeLen) {
		printf("%s index(%d).bInit:%d, writeLen:%d.\n", __func__, index, gShareMemInfo[index].bInit, writeLen);
		return -1;
	}
	pthread_mutex_lock(&g_share_mem_lock[index]);

	pCblk = gShareMemInfo[index].pCblk;

	if (!pCblk) {
		pthread_mutex_unlock(&g_share_mem_lock[index]);
		return -2;
	}
	cvitek_atomic_or(CBLK_FLAG_WRITE, &pCblk->mFlags);
	pthread_mutex_unlock(&g_share_mem_lock[index]);

	do {
		u32ReadPos = cvitek_atomic_acquire_load((const int32_t *)&pCblk->u32ReadPos);

		canWriteLen = (int)(pCblk->u32Len + u32ReadPos - pCblk->u32WritePos);
		if (canWriteLen < 0)
			canWriteLen += pCblk->u32Boundary;

		if ((int)writeLen > canWriteLen) {
			if (msecs == 0) {
				cvitek_atomic_and(~CBLK_FLAG_WRITE, &pCblk->mFlags);
				return 0;
			}
			int32_t old = cvitek_atomic_and(~CBLK_FUTEX_WAKE, &pCblk->mFutex);

			if ((old & CBLK_FUTEX_WAKE)) {
				int ret = cvi_sem_wait(&gShareMemInfo[index].buffer_sem, msecs);

				if (ret != 0 || !gShareMemInfo[index].bInit) {  //ret == 0 normal wakeup by server
					cvitek_atomic_or(CBLK_FUTEX_WAKE, &pCblk->mFutex);
					cvitek_atomic_and(~CBLK_FLAG_WRITE, &pCblk->mFlags);
					return ret;
				}
				continue;
			}
		}
		break;
	} while (1);

	u32WritePos = pCblk->u32WritePos % pCblk->u32Len;
	u32ReadPos = u32ReadPos % pCblk->u32Len;

	if (writeLen <= (pCblk->u32Len - u32WritePos))
		memcpy(&pCblk->data[u32WritePos], data, writeLen);
	else {
		memcpy(&pCblk->data[u32WritePos], data, pCblk->u32Len - u32WritePos);
		memcpy(pCblk->data, &data[pCblk->u32Len - u32WritePos],
			writeLen - (pCblk->u32Len - u32WritePos));
	}

	pCblk->u32WritePos += writeLen;
	if (pCblk->u32WritePos >= pCblk->u32Boundary)
		pCblk->u32WritePos -= pCblk->u32Boundary;

	cvitek_atomic_release_store(pCblk->u32WritePos, (int32_t *)&pCblk->u32WritePos);
	cvitek_atomic_and(~CBLK_FLAG_WRITE, &pCblk->mFlags);

	return writeLen;

}


int share_cyclebuffer_server_read(uint32_t index, char *outbuf, int readLen)
{
	AUDIO_CBLK *pCblk;
	uint32_t u32ReadPos = 0;
	uint32_t u32WritePos = 0;
	int canReadLen = 0;

	if (index > MAX_CHANNEL_COUNT) {
		printf("read failed, index:%d\n", index);
		return -1;
	}
	if (!gShareMemInfo[index].bInit || !outbuf || !readLen) {
		printf("read failed(%d,%p,%d)\n", gShareMemInfo[index].bInit, outbuf, readLen);
		return -1;
	}

	pthread_mutex_lock(&g_share_mem_lock[index]);
	pCblk = gShareMemInfo[index].pCblk;
	if (!pCblk) {
		pthread_mutex_unlock(&g_share_mem_lock[index]);
		return -2;
	}
	cvitek_atomic_or(CBLK_FLAG_READ, &pCblk->mFlags);
	pthread_mutex_unlock(&g_share_mem_lock[index]);

	u32WritePos = cvitek_atomic_acquire_load((const int32_t *)&pCblk->u32WritePos);
	canReadLen = u32WritePos - pCblk->u32ReadPos;
	if (canReadLen < 0)
		canReadLen += pCblk->u32Boundary;

	if (canReadLen < readLen) {
		cvitek_atomic_and(~CBLK_FLAG_READ, &pCblk->mFlags);
		return 0;
	}

	//update writepos and read pos
	u32WritePos = u32WritePos % pCblk->u32Len;
	u32ReadPos = pCblk->u32ReadPos % pCblk->u32Len;
	if ((uint32_t)readLen <= pCblk->u32Len - u32ReadPos) {
		memcpy(outbuf, &pCblk->data[u32ReadPos], readLen);
	} else {
		memcpy(outbuf, &pCblk->data[u32ReadPos], pCblk->u32Len - u32ReadPos);
		memcpy(&outbuf[pCblk->u32Len - u32ReadPos],
				pCblk->data,
				readLen - (pCblk->u32Len - u32ReadPos));
	}

	pCblk->u32ReadPos += readLen;
	if (pCblk->u32ReadPos >= pCblk->u32Boundary)
		pCblk->u32ReadPos -= pCblk->u32Boundary;
	//printf("share_cyclebuffer_read ReadPos(%u),len(%u)\n",pCblk->u32ReadPos,pCblk->u32Len);

	cvitek_atomic_release_store(pCblk->u32ReadPos, (int32_t *) &pCblk->u32ReadPos);
	int32_t old = cvitek_atomic_or(CBLK_FUTEX_WAKE, &pCblk->mFutex);

	if (!(old & CBLK_FUTEX_WAKE)) {
		cvi_sem_post(&gShareMemInfo[index].buffer_sem);
	}
	cvitek_atomic_and(~CBLK_FLAG_READ, &pCblk->mFlags);
	return readLen;
}


int share_cyclebuffer_server_write(uint32_t index, char *data, uint32_t writeLen)
{
	AUDIO_CBLK *pCblk;
	uint32_t u32ReadPos = 0;
	uint32_t u32WritePos = 0;
	int canWriteLen = 0;

	if (index > MAX_CHANNEL_COUNT) {
		return -1;
	}
	if (!gShareMemInfo[index].bInit || !data || !writeLen) {
		printf("server write failed(%d,%p,%d)\n", gShareMemInfo[index].bInit, data, writeLen);
		return -1;
	}

	pthread_mutex_lock(&g_share_mem_lock[index]);
	pCblk = gShareMemInfo[index].pCblk;
	if (!pCblk) {
		printf("%s, pCblk:%p", __func__, pCblk);
		pthread_mutex_unlock(&g_share_mem_lock[index]);
		return -2;
	}
	cvitek_atomic_or(CBLK_FLAG_WRITE, &pCblk->mFlags);
	pthread_mutex_unlock(&g_share_mem_lock[index]);

	u32ReadPos = cvitek_atomic_acquire_load((const int32_t *)&pCblk->u32ReadPos);
	canWriteLen = (int)(pCblk->u32Len + u32ReadPos - pCblk->u32WritePos);
	if (canWriteLen < 0)
		canWriteLen += pCblk->u32Boundary;

	if ((int)writeLen > canWriteLen) {
		cvitek_atomic_and(~CBLK_FLAG_WRITE, &pCblk->mFlags);
		return 0;
	}

	u32WritePos = pCblk->u32WritePos % pCblk->u32Len;
	u32ReadPos = u32ReadPos % pCblk->u32Len;

	if (writeLen <= (pCblk->u32Len - u32WritePos))
		memcpy(&pCblk->data[u32WritePos], data, writeLen);
	else {
		memcpy(&pCblk->data[u32WritePos], data, pCblk->u32Len - u32WritePos);
		memcpy(pCblk->data, &data[pCblk->u32Len - u32WritePos],
			writeLen - (pCblk->u32Len - u32WritePos));
	}

	pCblk->u32WritePos += writeLen;
	if (pCblk->u32WritePos >= pCblk->u32Boundary)
		pCblk->u32WritePos -= pCblk->u32Boundary;

	cvitek_atomic_release_store(pCblk->u32WritePos, (int32_t *)&pCblk->u32WritePos);
	int32_t old = cvitek_atomic_or(CBLK_FUTEX_WAKE, &pCblk->mFutex);

	if (!(old & CBLK_FUTEX_WAKE)) {
		//(void) syscall(__NR_futex, &pCblk->mFutex, FUTEX_WAKE, 1);
		cvi_sem_post(&gShareMemInfo[index].buffer_sem);
	}
	cvitek_atomic_and(~CBLK_FLAG_WRITE, &pCblk->mFlags);

	return writeLen;
}


int share_cyclebuffer_client_read(uint32_t index, char  *outbuf, int readLen, int32_t msecs)
{
	AUDIO_CBLK *pCblk;
	uint32_t u32ReadPos = 0;
	uint32_t u32WritePos = 0;
	int canReadLen = 0;

	if (index > MAX_CHANNEL_COUNT) {
		printf("read failed, index:%d\n", index);
		return -1;
	}
	if (!gShareMemInfo[index].bInit || !outbuf || !readLen) {
		printf("read failed(%d,%p,%d)\n", gShareMemInfo[index].bInit, outbuf, readLen);
		return -1;
	}

	pthread_mutex_lock(&g_share_mem_lock[index]);
	pCblk = gShareMemInfo[index].pCblk;
	if (!pCblk) {
		pthread_mutex_unlock(&g_share_mem_lock[index]);
		return -2;
	}
	cvitek_atomic_or(CBLK_FLAG_READ, &pCblk->mFlags);
	pthread_mutex_unlock(&g_share_mem_lock[index]);

	do {
		u32WritePos = cvitek_atomic_acquire_load((const int32_t *)&pCblk->u32WritePos);

		canReadLen = u32WritePos - pCblk->u32ReadPos;
			if (canReadLen < 0)
				canReadLen += pCblk->u32Boundary;

		if ((int)canReadLen < readLen) {
			if (msecs == 0) {
				cvitek_atomic_and(~CBLK_FLAG_READ, &pCblk->mFlags);
				return 0;
			}
			int32_t old = cvitek_atomic_and(~CBLK_FUTEX_WAKE, &pCblk->mFutex);

			if ((old & CBLK_FUTEX_WAKE)) {
				int ret = cvi_sem_wait(&gShareMemInfo[index].buffer_sem, msecs);
				if (ret != 0 || !gShareMemInfo[index].bInit) {  //ret == 0 normal wakeup by server
					cvitek_atomic_or(CBLK_FUTEX_WAKE, &pCblk->mFutex);
					cvitek_atomic_and(~CBLK_FLAG_READ, &pCblk->mFlags);
					return -4;
				}
				continue;
			}
		}
		break;
	} while (1);

	//update writepos and read pos
	u32WritePos = u32WritePos % pCblk->u32Len;
	u32ReadPos = pCblk->u32ReadPos % pCblk->u32Len;

	if ((uint32_t)readLen <= pCblk->u32Len - u32ReadPos) {
		memcpy(outbuf, &pCblk->data[u32ReadPos], readLen);
	} else {
		memcpy(outbuf, &pCblk->data[u32ReadPos], pCblk->u32Len - u32ReadPos);
		memcpy(&outbuf[pCblk->u32Len - u32ReadPos],
				pCblk->data,
				readLen - (pCblk->u32Len - u32ReadPos));
	}

	pCblk->u32ReadPos += readLen;
	if (pCblk->u32ReadPos >= pCblk->u32Boundary)
		pCblk->u32ReadPos -= pCblk->u32Boundary;
	//printf("share_cyclebuffer_read ReadPos(%u),len(%u)\n",pCblk->u32ReadPos,pCblk->u32Len);

	cvitek_atomic_release_store(pCblk->u32ReadPos, (int32_t *)&pCblk->u32ReadPos);

	cvitek_atomic_and(~CBLK_FLAG_READ, &pCblk->mFlags);
	return readLen;
}


int share_cyclebuffer_frameready_size(int index)
{
	AUDIO_CBLK *pCblk;
	uint32_t u32ReadPos = 0;
	uint32_t u32WritePos = 0;
	int canReadLen = 0;

	if (index > MAX_CHANNEL_COUNT) {
		return -1;
	}

	if (!gShareMemInfo[index].bInit) {
		return -2;
	}

	pthread_mutex_lock(&g_share_mem_lock[index]);
	pCblk = gShareMemInfo[index].pCblk;
	if (!pCblk) {
		pthread_mutex_unlock(&g_share_mem_lock[index]);
		return -3;
	}
	cvitek_atomic_or(CBLK_FLAG_CHECKSIZE, &pCblk->mFlags);
	pthread_mutex_unlock(&g_share_mem_lock[index]);

	u32WritePos = cvitek_atomic_acquire_load((const int32_t *)&pCblk->u32WritePos);
	u32ReadPos = cvitek_atomic_acquire_load((const int32_t *)&pCblk->u32ReadPos);
	canReadLen = u32WritePos - u32ReadPos;

	if (canReadLen < 0)
		canReadLen += pCblk->u32Boundary;

	cvitek_atomic_and(~CBLK_FLAG_CHECKSIZE, &pCblk->mFlags);
	return canReadLen;
}

int share_cyclebuffer_clear(int index)
{
	AUDIO_CBLK *pCblk;

	if (index > MAX_CHANNEL_COUNT) {
		return -1;
	}

	if (!gShareMemInfo[index].bInit) {
		return -2;
	}
	pthread_mutex_lock(&g_share_mem_lock[index]);
	pCblk = gShareMemInfo[index].pCblk;
	if (!pCblk) {
		pthread_mutex_unlock(&g_share_mem_lock[index]);
		return -3;
	}
	cvitek_atomic_or(CBLK_FLAG_RESET, &pCblk->mFlags);
	cvitek_atomic_release_store(0, (int32_t *)&pCblk->u32WritePos);
	cvitek_atomic_release_store(0, (int32_t *)&pCblk->u32ReadPos);
	cvitek_atomic_and(~CBLK_FLAG_CHECKSIZE, &pCblk->mFlags);
	pthread_mutex_unlock(&g_share_mem_lock[index]);

	return 0;
}

int share_cyclebuffer_total_buf_size(int index)
{
	AUDIO_CBLK *pCblk;

	if (index > MAX_CHANNEL_COUNT) {
		return -1;
	}

	if (!gShareMemInfo[index].bInit) {
		return -2;
	}
	pCblk = gShareMemInfo[index].pCblk;
	if (!pCblk) {
		return -3;
	}
	return gShareMemInfo[index].pCblk->u32Len;
}




void share_cyclebuffer_destroy(int index)
{
	AUDIO_CBLK *pCblk;
	uint32_t timeout = 0;
	int32_t old = 0;

	pthread_mutex_lock(&g_share_init_lock);
	if (!gShareMemInfo[index].bInit) {
		pthread_mutex_unlock(&g_share_init_lock);
		return;
	}
	gShareMemInfo[index].bInit = 0;
	pCblk = gShareMemInfo[index].pCblk;
	if (pCblk) {
		pthread_mutex_lock(&g_share_mem_lock[index]);
		while (cvitek_atomic_and(~CBLK_FLAG_INTERRUPT, &pCblk->mFlags)) {
			old = cvitek_atomic_or(CBLK_FUTEX_WAKE, &pCblk->mFutex);
			if (!(old & CBLK_FUTEX_WAKE)) {
				//(void) syscall(__NR_futex, &pCblk->mFutex, FUTEX_WAKE, 1);
				cvi_sem_post(&gShareMemInfo[index].buffer_sem);
			}

			usleep(1000);
			timeout++;
			if (timeout > 10000) {
				printf("destory buffer error,need check.\n");
				break;
			}
		}
		free(pCblk);
		gShareMemInfo[index].pCblk = NULL;
		pthread_mutex_unlock(&g_share_mem_lock[index]);
	}
	mUseflag = mUseflag & (~(1<<index));
	cvi_sem_destroy(&gShareMemInfo[index].buffer_sem);

	pthread_mutex_unlock(&g_share_init_lock);
}

