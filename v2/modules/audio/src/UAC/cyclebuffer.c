#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cyclebuffer.h"

#define DEBUG(x) //x

//static cycleBuffer  gst_cbuffer;


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

void CycleBufferDestory(void *pstCb)
{
	cycleBuffer *pstCBTmp = (cycleBuffer *)pstCb;

	if ((pstCb == NULL)  || !pstCBTmp->isInit) {
	printf("error param, CircleBufferDestory pstCb is NULL.\n");
	return;
	}

	if (pstCBTmp->buf) {
		//free
		free(pstCBTmp->buf);
	}

	memset(pstCBTmp, 0, sizeof(cycleBuffer));
	DEBUG(printf("CycleBufferDestory .\n"));
	//return;
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
	if (outbuf  == NULL) {
		pthread_mutex_unlock(&pstCBTmp->lock);
		return 0;
	}

	if (pstCBTmp->size < readLen) {
		pthread_mutex_unlock(&pstCBTmp->lock);
		return 0;
	}

	canReadLen = pstCBTmp->wroffset-pstCBTmp->rdoffser;
	if (canReadLen <= 0)
		canReadLen += pstCBTmp->buflen;

	if (canReadLen < readLen) {
		//readlen
		readLen = canReadLen;
	}

	if (readLen < pstCBTmp->buflen - pstCBTmp->rdoffser) {
		memcpy(outbuf, &pstCBTmp->buf[pstCBTmp->rdoffser], readLen);
	} else {
		memcpy(outbuf, &pstCBTmp->buf[pstCBTmp->rdoffser], pstCBTmp->buflen - pstCBTmp->rdoffser);
		memcpy(&outbuf[pstCBTmp->buflen - pstCBTmp->rdoffser],
				pstCBTmp->buf,
				readLen - (pstCBTmp->buflen - pstCBTmp->rdoffser));
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

	if (pstCBTmp->size == pstCBTmp->buflen) {
		pthread_mutex_unlock(&pstCBTmp->lock);
		return 0;
	}

	canWriteLen =  pstCBTmp->rdoffser -  pstCBTmp->wroffset;
	if (canWriteLen <= 0)
		canWriteLen +=  pstCBTmp->buflen;

	if (wrireLen > canWriteLen)
		return -1;

	if (wrireLen < (pstCBTmp->buflen - pstCBTmp->wroffset))
		memcpy(&pstCBTmp->buf[pstCBTmp->wroffset], inbuf, wrireLen);
	else {
		memcpy(&pstCBTmp->buf[pstCBTmp->wroffset], inbuf, pstCBTmp->buflen -  pstCBTmp->wroffset);
		memcpy(pstCBTmp->buf, &inbuf[pstCBTmp->buflen - pstCBTmp->wroffset],
			wrireLen - (pstCBTmp->buflen - pstCBTmp->wroffset));
	}

	pstCBTmp->wroffset = (wrireLen + pstCBTmp->wroffset) %  pstCBTmp->buflen;
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



