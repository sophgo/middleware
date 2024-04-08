/*
 * Copyright (c) 2013-2018 Andreas Unterweger
 *
 */

/**
 * @file
 * Simple audio converter from transcode_aac.c
 */

#include <string.h>
//#include "cvi_audio_interface_tinyalsa.h"
/* The output bit rate in bit/s */
#define OUTPUT_BIT_RATE 96000
/* The number of output channels */
#define OUTPUT_CHANNELS 2
#define CVI_MODIFIED 1
int argv3;

#if 0
int dbg_lev = 2;

#define ERR_PRINTF(fmt, args...) \
	do { \
		if (dbg_lev > 0) \
			fprintf(stderr, "[cviaudio][error][%s][%d] "fmt, __func__, __LINE__, ##args);\
	} while (0)

#define DBG_PRINTF(fmt, args...) \
	do { \
		if (dbg_lev > 1) \
			fprintf(stderr, "[cviaudio][info] "fmt, ##args);\
	} while (0)


ST_RES_INFO gstResInfo = {0};
ST_RES_INFO gstResInfo_aout = {0};
int set_resample_info(ST_RES_INFO res_info)
{
	memset(&gstResInfo, 0, sizeof(ST_RES_INFO));
	DBG_PRINTF("--------->set_resample_info\n");
	memcpy(&gstResInfo, &res_info, sizeof(ST_RES_INFO));
	return CVI_SUCCESS;
}

ST_RES_INFO get_resample_info(void)
{
	DBG_PRINTF("get_resample_info\n");
	return gstResInfo;
}

int set_resample_info_aout(ST_RES_INFO res_info)
{
	memset(&gstResInfo_aout, 0, sizeof(ST_RES_INFO));
	DBG_PRINTF("--------->set_resample_info aout\n");
	memcpy(&gstResInfo_aout, &res_info, sizeof(ST_RES_INFO));
	return CVI_SUCCESS;
}

ST_RES_INFO get_resample_info_aout(void)
{
	DBG_PRINTF("get_resample_info aout\n");
	return gstResInfo_aout;
}
#endif

