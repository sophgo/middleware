#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include "cyclebuffer.h"

#define DEBUG(x) //x

int CycleBufferInit(void  **ppstCb, int bufLen)
{
	cycleBuffer *pstCBTmp;

	if (!ppstCb || bufLen == 0) {
		printf("error param, CycleBufferInit ppstCb:%p , bufLen:%d.\n", ppstCb, bufLen);
		return -1;

	}

	*ppstCb = (cycleBuffer *)malloc(sizeof(cycleBuffer));
	if (*ppstCb == NULL) {
		printf("CycleBufferInit malloc failed.\n");
		return -1;
	}

	pstCBTmp = (cycleBuffer *)(*ppstCb);

	pthread_mutex_init(&(pstCBTmp->lock), 0);

	memset(pstCBTmp, 0, sizeof(cycleBuffer));
	pstCBTmp->isInit = 1;

	pstCBTmp->buflen = bufLen;
	pstCBTmp->buf = (char *)malloc(bufLen);
	memset(pstCBTmp->buf, 0, bufLen);

	DEBUG(printf("CycleBufferInit buflen:%d.\n", bufLen));
	return 0;
}

void CycleBufferDestroy(void *pstCb)
{
	cycleBuffer *pstCBTmp = (cycleBuffer *)pstCb;

	if ((pstCb == NULL)  || !pstCBTmp->isInit) {
		printf("error param, CircleBufferDestroy pstCb is NULL.\n");
		return;
	}
	pthread_mutex_lock(&pstCBTmp->lock);

	if (pstCBTmp->buf) {
		//free
		free(pstCBTmp->buf);
		pstCBTmp->buf = NULL;
	}
	pthread_mutex_unlock(&pstCBTmp->lock);
	memset(pstCBTmp, 0, sizeof(cycleBuffer));
	if (pstCBTmp != NULL) {
		free(pstCBTmp);
		pstCb = NULL;
	}
	DEBUG(printf("CycleBufferDestroy .\n"));

}

int CycleBufferRead(void *pstCb, char  *outbuf, int readLen)
{
	int canReadLen = 0;

	cycleBuffer *pstCBTmp = (cycleBuffer *)pstCb;

	if ((pstCb == NULL) || !(pstCBTmp->isInit)) {
	printf("error param, CycleBufferRead pstCb is NULL.\n");
	return 0;

	}

	pthread_mutex_lock(&pstCBTmp->lock);
	if (outbuf	== NULL) {
		pthread_mutex_unlock(&pstCBTmp->lock);
		return 0;
	}

	if (pstCBTmp->size < readLen) {
		pthread_mutex_unlock(&pstCBTmp->lock);
		return 0;
	}

	if (readLen < 0) {
		printf("[Error][%s][%d] input len<0 [%d]\n", __func__, __LINE__, readLen);
		pthread_mutex_unlock(&pstCBTmp->lock);
		return 0;
	}

	canReadLen = pstCBTmp->wroffset-pstCBTmp->rdoffser;
	if (canReadLen <= 0)
		canReadLen += pstCBTmp->buflen;

	if (canReadLen < readLen) {
		//readlen
		if (canReadLen < 0) {
			printf("[Error][%s][%d], canReadLen[%d] readLen[%d], w:[%d] r:[%d]\n",
			__func__, __LINE__, canReadLen, readLen, pstCBTmp->wroffset, pstCBTmp->rdoffser);
			pthread_mutex_unlock(&pstCBTmp->lock);
			return 0;
		}

		readLen = canReadLen;
	}

	if (readLen <= (int)(pstCBTmp->buflen - pstCBTmp->rdoffser)) {

		if (outbuf == NULL || pstCBTmp->buf == NULL) {
			printf("[Error][%s][%d]outbuf[%p], pstCBTmp_buf[%p] rdoffser[%d]\n",
				__func__, __LINE__, outbuf, pstCBTmp->buf, pstCBTmp->rdoffser);
			pthread_mutex_unlock(&pstCBTmp->lock);
			return 0;

		}
		if ((int)(pstCBTmp->rdoffser) < 0) {
			printf("[Error][%s][%d]readLen[%d],bufLen[%d] rdoffser[%d]\n",
				__func__, __LINE__, readLen, pstCBTmp->buflen, pstCBTmp->rdoffser);
			pthread_mutex_unlock(&pstCBTmp->lock);
			return 0;
		}
			memcpy(outbuf, &pstCBTmp->buf[pstCBTmp->rdoffser], readLen);

	} else {
		int first_copy = pstCBTmp->buflen - pstCBTmp->rdoffser;

		if (first_copy < 0) {
			printf("[Error][%s][%d], first_copy[%d] pstCBTmp->buflen[%d],r:[%d]\n",
			__func__, __LINE__, first_copy, pstCBTmp->buflen, pstCBTmp->rdoffser);
			printf("[Error]Cannot copy negative length[%s][%d]\n", __func__, __LINE__);
			pthread_mutex_unlock(&pstCBTmp->lock);
			return 0;
		}

		if (first_copy > (readLen - 1)) {
			printf("[Error][%s]Exceed outbuf index line[%d]\n", __func__, __LINE__);
			printf("[%s]r:%d, w:%d, size:%d buflen:%d , readlen:%d\n",
				__func__,
				pstCBTmp->rdoffser, pstCBTmp->wroffset, pstCBTmp->size,
				pstCBTmp->buflen, readLen);
		}

		if ((readLen - (first_copy)) > readLen) {
			printf("[Error][%s]Exceed outbuf size line[%d]\n", __func__, __LINE__);
			printf("[%s]r:%d, w:%d, size:%d cycbuflen:%d , readlen:%d\n",
				__func__,
				pstCBTmp->rdoffser, pstCBTmp->wroffset, pstCBTmp->size,
				pstCBTmp->buflen, readLen);
		}

		memcpy(outbuf, &pstCBTmp->buf[pstCBTmp->rdoffser], first_copy);

		memcpy(&outbuf[first_copy],
				pstCBTmp->buf,
				readLen - (first_copy));
	}

	pstCBTmp->size -= readLen;
	pstCBTmp->rdoffser = (readLen + pstCBTmp->rdoffser) % pstCBTmp->buflen;
	DEBUG(printf("CycleBufferRead r:%d, w:%d, size:%d\n", pstCBTmp->rdoffser, pstCBTmp->wroffset, pstCBTmp->size));

	pthread_mutex_unlock(&pstCBTmp->lock);
	return readLen;
}

int CycleBufferWrite(void *pstCb, char *inbuf, int wrireLen)
{
	int canWriteLen = 0;

	cycleBuffer *pstCBTmp = (cycleBuffer *)pstCb;

	if ((pstCb == NULL) || !(pstCBTmp->isInit)) {
	printf("error param, CycleBufferWrite pstCb is NULL.\n");
	return 0;
	}

	pthread_mutex_lock(&pstCBTmp->lock);
	if ((pstCBTmp->buflen == 0) || (inbuf == NULL) || (wrireLen == 0)) {
		pthread_mutex_unlock(&pstCBTmp->lock);
		return 0;
	}

	if (wrireLen < 0) {
		printf("[Error][%s][%d] write len negativelen[%d]\n", __func__, __LINE__, wrireLen);
		pthread_mutex_unlock(&pstCBTmp->lock);
		return 0;
	}


	if (pstCBTmp->size == pstCBTmp->buflen) {
		pthread_mutex_unlock(&pstCBTmp->lock);
		return 0;
	}

	canWriteLen =  pstCBTmp->rdoffser -  pstCBTmp->wroffset;
	if (canWriteLen <= 0)
		canWriteLen +=	pstCBTmp->buflen;

	if (wrireLen > canWriteLen) {
		pthread_mutex_unlock(&pstCBTmp->lock);
		return 0;
	}

	if (wrireLen < (int)(pstCBTmp->buflen - pstCBTmp->wroffset))
		memcpy(&pstCBTmp->buf[pstCBTmp->wroffset], inbuf, wrireLen);
	else {
		memcpy(&pstCBTmp->buf[pstCBTmp->wroffset], inbuf, pstCBTmp->buflen -  pstCBTmp->wroffset);
		memcpy(pstCBTmp->buf, &inbuf[pstCBTmp->buflen - pstCBTmp->wroffset],
			wrireLen - (pstCBTmp->buflen - pstCBTmp->wroffset));
	}

	pstCBTmp->wroffset = (wrireLen + pstCBTmp->wroffset) %	pstCBTmp->buflen;
	pstCBTmp->size += wrireLen;

	pthread_mutex_unlock(&pstCBTmp->lock);
	DEBUG(printf("CycleBufWrite r:%d, w:%d, size:%d\n", pstCBTmp->rdoffser, pstCBTmp->wroffset, pstCBTmp->size));
	return wrireLen;
}


int CycleBufferDataLen(void *pstCb)
{
	cycleBuffer *pstCBTmp = (cycleBuffer *)pstCb;

	if ((pstCb == NULL) || !(pstCBTmp->isInit)) {
		printf("error param, CycleBufferWrite pstCb is NULL.\n");
		return 0;
	}
	return pstCBTmp->size;
}

int CycleBufferFreeSize(void *pstCb)
{
	cycleBuffer *pstCBTmp = (cycleBuffer *)pstCb;
	int free_size = 0;

	if ((pstCb == NULL) || !(pstCBTmp->isInit)) {
		printf("error param, CycleBufferWrite pstCb is NULL.\n");
		return 0;
	}
	pthread_mutex_lock(&pstCBTmp->lock);
	free_size = pstCBTmp->buflen - pstCBTmp->size;
	pthread_mutex_unlock(&pstCBTmp->lock);

	return free_size;
}

int CycleBufferWriteWait(void *pstCb, char *inbuf, int wrireLen, int timeoutMs)
{
	cycleBuffer *pstCBTmp = (cycleBuffer *)pstCb;
	int s32SendCnt = 0;
	int timecnt = 0;

	if ((pstCb == NULL) || !(pstCBTmp->isInit)) {
		printf("error param, CycleBufferWrite pstCb is NULL.\n");
		return 0;
	}

	if ((pstCBTmp->buflen == 0) || (inbuf == NULL) || (wrireLen == 0)) {
		return 0;
	}

	do {
		s32SendCnt = CycleBufferWrite(pstCb, inbuf, wrireLen);
		if (((timeoutMs >= 0) && (timecnt >= timeoutMs))) {
			break;
		}
		if (!s32SendCnt) {
			usleep(5000);
			timecnt += 5;
		}
	} while (!s32SendCnt);

	return s32SendCnt;
}




int CycleBufferSize(void *pstCb)
{
	cycleBuffer *pstCBTmp = (cycleBuffer *)pstCb;

	if ((pstCb == NULL) || !(pstCBTmp->isInit)) {
		printf("error param, CycleBufferWrite pstCb is NULL.\n");
		return 0;
	}
	return pstCBTmp->buflen;
}

void CycleBufferReset(void *pstCb)
{
	cycleBuffer *pstCBTmp = (cycleBuffer *)pstCb;

	pthread_mutex_lock(&pstCBTmp->lock);
	if ((pstCb == NULL) || !(pstCBTmp->isInit)) {
		printf("error param, CycleBufferWrite pstCb is NULL.\n");
		pthread_mutex_unlock(&pstCBTmp->lock);
		return;
	}

	pstCBTmp->size = 0;
	pstCBTmp->rdoffser = 0;
	pstCBTmp->wroffset = 0;

	pthread_mutex_unlock(&pstCBTmp->lock);
}





