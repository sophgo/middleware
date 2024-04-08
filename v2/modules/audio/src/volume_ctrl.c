#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include "alog.h"
#include "volume_ctrl.h"


#define MAX_STEPS 100

#define Qfactor (13)

// -42 to 0 db
static int tabQf[MAX_STEPS+1] = {
	0, 68, 71, 75, 78, 82, 86, 91, 95, 100,
	105, 110, 116, 122, 128, 134, 141, 148, 155, 163,
	171, 179, 188, 197, 207, 217, 228, 240, 251, 264,
	277, 291, 305, 320, 336, 353, 371, 389, 408, 428,
	450, 472, 495, 520, 546, 573, 601, 631, 662, 695,
	730, 766, 804, 844, 885, 929, 975, 1024, 1074, 1128,
	1184, 1242, 1304, 1368, 1436, 1507, 1582, 1661, 1743, 1829,
	1920, 2015, 2115, 2220, 2330, 2445, 2566, 2693, 2827, 2967,
	3114, 3268, 3430, 3600, 3779, 3966, 4162, 4369, 4585, 4812,
	5051, 5301, 5564, 5839, 6129, 6432, 6751, 7085, 7436, 7805,
	8192

};

static pthread_mutex_t g_volume_ctrl_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct CVI_VOLUME_CTRL_INFOtag {
	int bInit;
	int mVolIndex;

} CVI_VOLUME_CTRL_INFO, *CVI_VOLUME_CTRL_INFO_PTR;


VOL_HANDLE cvitek_volume_ctrl_create(void)
{
	CVI_VOLUME_CTRL_INFO_PTR pstVolInfo;

	pthread_mutex_lock(&g_volume_ctrl_lock);
	pstVolInfo = (CVI_VOLUME_CTRL_INFO_PTR)malloc(sizeof(CVI_VOLUME_CTRL_INFO));
	if (!pstVolInfo) {
		pthread_mutex_unlock(&g_volume_ctrl_lock);
		return NULL;
	}
	pstVolInfo->bInit = 1;
	pstVolInfo->mVolIndex = 100;

	pthread_mutex_unlock(&g_volume_ctrl_lock);
	return pstVolInfo;
}

int cvitek_set_volume_index(VOL_HANDLE volHandle, int index)
{
	CVI_VOLUME_CTRL_INFO_PTR pstVolHandle = (CVI_VOLUME_CTRL_INFO_PTR)volHandle;

	if (!pstVolHandle) {
		log_error("invalid params.\n");
		return -1;
	}

	if (!pstVolHandle->bInit) {
		log_error("not init.\n");
		return -2;
	}

	if (index < 0 || index > MAX_STEPS) {
		log_error("volume[0-100],invalid index:%d\n", index);
		return -3;
	}
	pthread_mutex_lock(&g_volume_ctrl_lock);
	pstVolHandle->mVolIndex = index;
	pthread_mutex_unlock(&g_volume_ctrl_lock);

	return 0;
}

int cvitek_get_volume_index(VOL_HANDLE volHandle)
{
	CVI_VOLUME_CTRL_INFO_PTR pstVolHandle = (CVI_VOLUME_CTRL_INFO_PTR)volHandle;

	if (!pstVolHandle) {
		log_error("invalid params.\n");
		return -1;
	}

	if (!pstVolHandle->bInit) {
		log_error("not init.\n");
		return -2;
	}

	return pstVolHandle->mVolIndex;
}

int vol_ctrl_process(VOL_HANDLE volHandle, short *sIn, short *sOut, int len)
{
	CVI_VOLUME_CTRL_INFO_PTR pstVolHandle = (CVI_VOLUME_CTRL_INFO_PTR)volHandle;

	if (!pstVolHandle) {
		log_error("invalid params.\n");
		return -1;
	}

	if (!pstVolHandle->bInit) {
		log_error("not init.\n");
		return -2;
	}

	if (!sIn || !sOut || !len) {
		log_error("not init.\n");
		return -3;
	}

	for (int i = 0; i < len; i++) {
		sOut[i] = ((int)sIn[i] * tabQf[pstVolHandle->mVolIndex] + (1 << (Qfactor-1))) >> Qfactor;
		sOut[i] = sOut[i] < 0x7fff ? sOut[i] : 0x7fff;
		sOut[i] = sOut[i] > -0x7fff ? sOut[i] : -0x7fff;
	}

	return 0;
}


void cvitek_volume_ctrl_destroy(VOL_HANDLE volHandle)
{
	CVI_VOLUME_CTRL_INFO_PTR pstVolInfo = (CVI_VOLUME_CTRL_INFO_PTR)volHandle;

	if (pstVolInfo && pstVolInfo->bInit)
		free(pstVolInfo);
	else
		log_error("pstVolInfo:%p, init:%d\n", pstVolInfo, pstVolInfo->bInit);

}


