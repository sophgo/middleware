/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_bin.c
 * Description:
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <cvi_common.h>
#include <cvi_defines.h>
#include "cvi_comm_isp.h"
#include "isp_bin.h"
#include "cvi_isp.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "3A_internal.h"

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#define CVI_TRACE_ISP_BIN(level, fmt, ...) \
	CVI_TRACE(level, CVI_ID_ISP, "%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

//CVI_BIN_EXTRA_S.Desc[624-1023] 400 bytes
//[pqbin md5 50] + [reserved 50]
//[tool version 40] + [isp commitId 10] + [sensorNum 9] + [sensorName1 27] + [sensorName2 27]
//[sensorName3 27]  + [sensorName4 27] + [sensorName5 27]  + [sensorName6 27]  + [sensorName7 27]  + [sensorName8 27]
//[isp branch 20] + [version 4] + [generate mode - A:auto M:Manual 1]
#define DESC_SIZE 624
#define BIN_MD5_SIZE 50
#define PQBIN_RESERVE_SIZE 50
#define TOOLVERSION_SIZE 40
#define BIN_COMMIT_SIZE 10
#define SENSORNUM_SIZE 9
#define SENSORNAME_SIZE 27
#define BIN_GERRIT_SIZE 20
#define PQBINVERSION_SIZE 4
#define PQBINVERSION "V1.3"  // The latest version, it will write to pqbin file
#define PQBINCREATE_MODE_SIZE 1
#define SUPPORT_VI_MAX_PIPE_NUM 8

typedef struct _SENSOR_INFO {
	CVI_U8 num;
	CVI_CHAR name[SUPPORT_VI_MAX_PIPE_NUM][SENSORNAME_SIZE];
} SENSOR_INFO;

static CVI_S32 isp_bin_getBinSizeImp(VI_PIPE ViPipe, CVI_U32 *binSize);
static CVI_S32 isp_bin_getBinParamImp(VI_PIPE ViPipe, CVI_U8 *addr, CVI_U32 binSize);
static CVI_S32 isp_bin_setBinParamImp(VI_PIPE ViPipe, FILE *fp);
static CVI_S32 isp_bin_setBinParamImptobuf(VI_PIPE ViPipe, unsigned char *buffer);
static CVI_S32 isp_bin_checkMd5(VI_PIPE ViPipe, CVI_U8 *addr, CVI_U32 binSize);
static CVI_S32 isp_bin_checkBinVersion(CVI_U8 *addr, CVI_U32 binSize);
static CVI_S32 isp_get_paramstruct(VI_PIPE ViPipe, ISP_Parameter_Structures *pstParaBuf);
static CVI_S32 isp_set_paramstruct(VI_PIPE ViPipe, ISP_Parameter_Structures *pstParaBuf);
static CVI_S32 isp_getIndexBinParam(VI_PIPE ViPipe, CVI_U8 *buf, ISP_Parameter_Structures *pst, CVI_U32 indexOffset);
static CVI_S32 isp_index_bin_getparamfrombinImp(VI_PIPE ViPipe, CVI_U8 *addr, CVI_U32 binSize, CVI_U32 indexOffset);

static ISP_BIN_BYPASS g_binBypassParams = {0};

CVI_S32 header_bin_getBinSize(enum CVI_BIN_SECTION_ID id, CVI_U32 *binSize)
{
	CVI_S32 ret = CVI_SUCCESS;

	*binSize = sizeof(CVI_BIN_HEADER);
	UNUSED(id);

	return ret;
}

CVI_S32 header_bin_getBinParam(enum CVI_BIN_SECTION_ID id, CVI_U8 *addr, CVI_U32 binSize)
{
	CVI_S32 ret = CVI_SUCCESS;

	ret = isp_bin_checkBinVersion(addr, binSize);
	UNUSED(id);

	return ret;
}

CVI_S32 isp_bin_getSensorInfo(SENSOR_INFO *sensorInfo)
{
	#define NAME_SIZE 20
	CVI_U8 i = 0;
	CVI_U8 sensor_num = 0;

	for (i = 0; i < SUPPORT_VI_MAX_PIPE_NUM; ++i) {
		char sensorNameEnv[NAME_SIZE] = {0};

		memset(sensorNameEnv, 0, NAME_SIZE);
		snprintf(sensorNameEnv, NAME_SIZE, "SENSORNAME%d", i);
		CVI_CHAR *pszEnv = getenv(sensorNameEnv);

		if (pszEnv != NULL) {
			sensor_num++;
			strncpy((*sensorInfo).name[i], pszEnv, SENSORNAME_SIZE);
		}
	}
	(*sensorInfo).num = sensor_num;

	return CVI_SUCCESS;
}

static CVI_S32 bin_get_repo_info(CVI_CHAR *gerritid, CVI_CHAR *commitid, CVI_CHAR *md5)
{
	//show current isp branch & commit
#ifndef ISP_BIN_GERRIT
#	define ISP_BIN_GERRIT "NULL"
#endif
#ifndef ISP_BIN_COMMIT
#	define ISP_BIN_COMMIT "NULL"
#endif
#ifndef ISP_BIN_MD5
#	define ISP_BIN_MD5 "Not Define Md5"
#endif
	size_t len = 0;

	if (commitid == NULL || gerritid == NULL) {
		return CVI_FAILURE;
	}

	len = strlen(ISP_BIN_GERRIT) + 1;
	strncpy(gerritid, ISP_BIN_GERRIT, len);
	len = strlen(ISP_BIN_COMMIT) + 1;
	strncpy(commitid, ISP_BIN_COMMIT, len);
	len = strlen(ISP_BIN_MD5) + 1;
	strncpy(md5, ISP_BIN_MD5, len);

	return CVI_SUCCESS;
}

CVI_S32 header_bin_GetHeaderDescInfo(CVI_CHAR *pOutDesc, enum CVI_BIN_CREATMODE inCreatMode)
{
	//Supplement isp branch and isp commitId to pqbin
	CVI_S32 ret = CVI_SUCCESS;
	SENSOR_INFO sensorInfo;
	CVI_CHAR binCommitId[BIN_COMMIT_SIZE + 1] = { 0 };
	CVI_CHAR binGerrit[BIN_GERRIT_SIZE + 1] = { 0 };
	CVI_CHAR binMd5[BIN_MD5_SIZE + 1] = { 0 };

	isp_bin_getSensorInfo(&sensorInfo);
	bin_get_repo_info(binGerrit, binCommitId, binMd5);

	pOutDesc += DESC_SIZE;
	strncpy(pOutDesc, binMd5, BIN_MD5_SIZE);
	pOutDesc += BIN_MD5_SIZE;
	pOutDesc += PQBIN_RESERVE_SIZE;
	pOutDesc += TOOLVERSION_SIZE;
	strncpy(pOutDesc, binCommitId, BIN_COMMIT_SIZE);
	pOutDesc += BIN_COMMIT_SIZE;
	if (inCreatMode != CVI_BIN_AUTO) {
		snprintf(pOutDesc, SENSORNUM_SIZE, "%d", sensorInfo.num);
	}
	pOutDesc += SENSORNUM_SIZE;
	if (inCreatMode != CVI_BIN_AUTO) {
		for (CVI_U8 i = 0; i < sensorInfo.num; ++i) {
			strncpy(pOutDesc + i * SENSORNAME_SIZE, sensorInfo.name[i], SENSORNAME_SIZE);
		}
	}
	pOutDesc += SENSORNAME_SIZE * SUPPORT_VI_MAX_PIPE_NUM;
	strncpy(pOutDesc, binGerrit, BIN_GERRIT_SIZE);
	pOutDesc += BIN_GERRIT_SIZE;
	strncpy(pOutDesc, PQBINVERSION, PQBINVERSION_SIZE);
	pOutDesc += PQBINVERSION_SIZE;
	if (inCreatMode == CVI_BIN_AUTO) {
		pOutDesc[0] = 'A';
	} else {
		pOutDesc[0] = 'M';
	}

	return ret;
}

CVI_S32 header_bin_setBinParambuf(enum CVI_BIN_SECTION_ID id, unsigned char *buffer)
{
	CVI_S32 ret = CVI_SUCCESS;
	CVI_BIN_HEADER *pstHeader = NULL;

	pstHeader = (CVI_BIN_HEADER *)buffer;
	CVI_CHAR *pDesc = (CVI_CHAR *)pstHeader->extraInfo.Desc;

	header_bin_GetHeaderDescInfo(pDesc, CVI_BIN_MANUAL);
	UNUSED(id);

	return ret;
}

CVI_S32 header_bin_setBinParam(enum CVI_BIN_SECTION_ID id, FILE *fp)
{
	CVI_S32 ret = CVI_SUCCESS;
	CVI_BIN_HEADER header;

	fseek(fp, -sizeof(CVI_BIN_HEADER), SEEK_CUR);
	fread(&header, sizeof(CVI_BIN_HEADER), 1, fp);
	fseek(fp, -sizeof(CVI_BIN_HEADER), SEEK_CUR);
	CVI_CHAR *pDesc = (CVI_CHAR *)header.extraInfo.Desc;

	header_bin_GetHeaderDescInfo(pDesc, CVI_BIN_MANUAL);

	fwrite(&header, sizeof(CVI_BIN_HEADER), 1, fp);
	UNUSED(id);

	return ret;
}

CVI_S32 isp_bin_getBinSize(enum CVI_BIN_SECTION_ID id, CVI_U32 *binSize)
{
	CVI_S32 ret = CVI_SUCCESS;

	ret = isp_bin_getBinSizeImp(id - CVI_BIN_ID_ISP0, binSize);

	return ret;
}

CVI_S32 isp_bin_getBinParam(enum CVI_BIN_SECTION_ID id, CVI_U8 *addr, CVI_U32 binSize)
{
	CVI_S32 ret = CVI_SUCCESS;

	ret = isp_bin_getBinParamImp(id - CVI_BIN_ID_ISP0, addr, binSize);

	return ret;
}

CVI_S32 isp_bin_setBinParambuf(enum CVI_BIN_SECTION_ID id, CVI_U8 *buffer)
{
	CVI_S32 ret = CVI_SUCCESS;

	ret = isp_bin_setBinParamImptobuf(id - CVI_BIN_ID_ISP0, buffer);

	return ret;
}

CVI_S32 isp_bin_setBinParam(enum CVI_BIN_SECTION_ID id, FILE *fp)
{
	CVI_S32 ret = CVI_SUCCESS;

	ret = isp_bin_setBinParamImp(id - CVI_BIN_ID_ISP0, fp);

	return ret;
}

static CVI_S32 isp_bin_getBinSizeImp(VI_PIPE ViPipe, CVI_U32 *binSize)
{
	CVI_S32 ret = CVI_SUCCESS;

	*binSize = sizeof(ISP_Parameter_Structures);

	UNUSED(ViPipe);

	return ret;
}

static CVI_S32 isp_bin_getBinParamImp(VI_PIPE ViPipe, CVI_U8 *addr, CVI_U32 binSize)
{
	CVI_S32 ret = CVI_SUCCESS;
	CVI_U8 *buf_ofs = addr;
	CVI_BIN_HEADER *pstHeader = (CVI_BIN_HEADER *)buf_ofs;

	if (isp_bin_checkMd5(ViPipe, addr, binSize) != CVI_SUCCESS) {
		CVI_TRACE_ISP_BIN(LOG_WARNING, "section[CVI_BIN_ID_ISP%d] Md5 not matched\n", ViPipe);
		return CVI_FAILURE;
	}
	for (CVI_S32 idx = CVI_BIN_ID_MIN; idx < CVI_BIN_ID_ISP0 + ViPipe; idx++) {
		buf_ofs += pstHeader->size[idx];
	}

	isp_set_paramstruct(ViPipe, (ISP_Parameter_Structures *)buf_ofs);

	CVI_TRACE_ISP_BIN(LOG_DEBUG, "apply bin setting to %d\n", ViPipe);

	return ret;
}

static CVI_S32 isp_bin_setBinParamImp(VI_PIPE ViPipe, FILE *fp)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_Parameter_Structures *pstParam = NULL;

	pstParam = (ISP_Parameter_Structures *)malloc(sizeof(ISP_Parameter_Structures));
	if (pstParam == NULL) {
		CVI_TRACE_ISP_BIN(LOG_ERR, "%s\n", "Allocate memory fail");
		return CVI_BIN_MALLOC_ERR;
	}

	isp_get_paramstruct(ViPipe, pstParam);
	fwrite(pstParam, sizeof(ISP_Parameter_Structures), 1, fp);

	free(pstParam);
	return ret;
}

static CVI_S32 isp_bin_setBinParamImptobuf(VI_PIPE ViPipe, unsigned char *buffer)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_Parameter_Structures *pstParam = NULL;

	pstParam = (ISP_Parameter_Structures *)malloc(sizeof(ISP_Parameter_Structures));
	if (pstParam == NULL) {
		CVI_TRACE_ISP_BIN(LOG_ERR, "%s\n", "Allocate memory fail");
		return CVI_BIN_MALLOC_ERR;
	}
	isp_get_paramstruct(ViPipe, pstParam);
	memcpy(buffer, pstParam, sizeof(ISP_Parameter_Structures));

	free(pstParam);
	return ret;
}

static CVI_S32 isp_index_bin_getparamfrombinImp(VI_PIPE ViPipe, CVI_U8 *addr, CVI_U32 binSize, CVI_U32 indexOffset)
{
	CVI_S32 ret = CVI_SUCCESS;

	ISP_Parameter_Structures *param = (ISP_Parameter_Structures *)malloc(sizeof(ISP_Parameter_Structures));

	isp_get_paramstruct(ViPipe, param); // get current params before import from index table

	isp_getIndexBinParam(ViPipe, addr, param, indexOffset);

	isp_set_paramstruct(ViPipe, param);

	CVI_TRACE_ISP_BIN(LOG_DEBUG, "apply bin setting to %d by index\n", ViPipe);

	free(param);
	UNUSED(binSize);

	return ret;
}

static CVI_S32 isp_getIndexBinParam(VI_PIPE ViPipe, CVI_U8 *buf, ISP_Parameter_Structures *pst, CVI_U32 indexOffset)
{
	CVI_S32 ret = CVI_SUCCESS;

	if (((ViPipe) < 0) || ((ViPipe) >= SUPPORT_VI_MAX_PIPE_NUM)) {
		CVI_TRACE_ISP_BIN(LOG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	if (buf == NULL || pst == NULL) {
		return CVI_FAILURE;
	}

	if (indexOffset <= 0) {
		return CVI_FAILURE;
	}

	ret = isp_getBinParam_autogen(ViPipe, buf, pst, indexOffset);

	return ret;
}

static CVI_S32 isp_bin_checkMd5(VI_PIPE ViPipe, CVI_U8 *addr, CVI_U32 binSize)
{
	CVI_S32 ret = CVI_SUCCESS;
	CVI_U32 size = 0;
	CVI_CHAR binCommitId[BIN_COMMIT_SIZE + 1] = {0};
	CVI_CHAR binGerrit[BIN_GERRIT_SIZE + 1] = {0};
	CVI_CHAR curMd5[BIN_MD5_SIZE + 1] = { 0 };
	CVI_CHAR binMd5[BIN_MD5_SIZE + 1] = { 0 };
	CVI_BIN_HEADER *header = (CVI_BIN_HEADER *)addr;
	CVI_BIN_EXTRA_S *pExtraInfo = &(header->extraInfo);

	strncpy(binMd5, (CVI_CHAR *)(pExtraInfo->Desc + DESC_SIZE), BIN_MD5_SIZE);
	bin_get_repo_info(binGerrit, binCommitId, curMd5);

	if (strcmp(binMd5, curMd5)) {
		CVI_TRACE_ISP_BIN(LOG_ERR, "%s%s%s",
			"md5 mismatch means that you will read by index in PQBIN, ",
			"but the disadvantage is that the boot speed is slightly reduced. ",
			"If you want to read bin in PQBIN, please make pqbin again.\n");
		CVI_TRACE_ISP_BIN(LOG_ERR, "please update to version (branch:%s commitId:%s)\n",
			binGerrit, binCommitId);
		ret = CVI_FAILURE;
	} else {
		isp_bin_getBinSizeImp(ViPipe, &size);
		if (size != binSize) {
			CVI_TRACE_ISP_BIN(LOG_ERR, "vi(%d), Bin Size not matched(%u vs %u),", ViPipe, size, binSize);
			CVI_TRACE_ISP_BIN(LOG_ERR, "%s%s",
				"Size mismatch means that someone added the pqbin structure ",
				"but it was not retrieved by the MD5 script. this must be fixed immediately!\n");
			CVI_TRACE_ISP_BIN(LOG_ERR, "please update to version (branch:%s commitId:%s)\n",
				binGerrit, binCommitId);
			ret = CVI_FAILURE;
		}
	}

	return ret;
}

static CVI_S32 isp_bin_checkBinVersion(CVI_U8 *addr, CVI_U32 binSize)
{
	printf("********************************************************************************\n");
	//get CVI_BIN_EXTRA_S
	CVI_BIN_HEADER *header = (CVI_BIN_HEADER *)addr;
	CVI_BIN_EXTRA_S *pExtraInfo = &(header->extraInfo);

	//get sensor info
	SENSOR_INFO sensorInfo;
	//show pqbin isp branch & commit & sensor
	CVI_CHAR toolVersion[TOOLVERSION_SIZE + 1] = {0};
	CVI_CHAR sensorNum[SENSORNUM_SIZE + 1] = {0};
	CVI_CHAR sensorName[SUPPORT_VI_MAX_PIPE_NUM][SENSORNAME_SIZE + 1] = {0};
	CVI_CHAR binCommitId[BIN_COMMIT_SIZE + 1] = {0};
	CVI_CHAR binGerrit[BIN_GERRIT_SIZE + 1] = {0};
	CVI_CHAR binMd5[BIN_MD5_SIZE + 1] = { 0 };
	CVI_CHAR pqbinVersion[PQBINVERSION_SIZE + 1] = { 0 };
	CVI_CHAR pqbinMode[PQBINCREATE_MODE_SIZE + 1] = { 0 };
	CVI_CHAR *pDesc = (CVI_CHAR *)pExtraInfo->Desc;

	//show curSw isp_bin branch & commit
	printf("cvi_bin_isp message\n");

	bin_get_repo_info(binGerrit, binCommitId, binMd5);
	isp_bin_getSensorInfo(&sensorInfo);

	printf("%-15s%-15s%-15s%-15s\n", "gerritId:", binGerrit, "commitId:", binCommitId);
	printf("%-15s%-15s\n", "md5:", binMd5);
	printf("%-15s%-15d\n", "sensorNum", sensorInfo.num);
	for (CVI_U8 i = 0; i < sensorInfo.num; ++i) {
		printf("%s%-5d%-15s\n", "sensorName", i, sensorInfo.name[i]);
	}
	printf("\n");

	//show pqbin isp branch & commit & sensor
	pDesc += DESC_SIZE;
	strncpy(binMd5, pDesc, BIN_MD5_SIZE);
	pDesc += BIN_MD5_SIZE;
	pDesc += PQBIN_RESERVE_SIZE;
	strncpy(toolVersion, pDesc, TOOLVERSION_SIZE);
	pDesc += TOOLVERSION_SIZE;
	strncpy(binCommitId, pDesc, BIN_COMMIT_SIZE);
	pDesc += BIN_COMMIT_SIZE;
	strncpy(sensorNum, pDesc, SENSORNUM_SIZE);
	pDesc += SENSORNUM_SIZE;
	for (CVI_U8 i = 0; i < atoi(sensorNum); ++i) {
		strncpy(sensorName[i], pDesc + i * SENSORNAME_SIZE, SENSORNAME_SIZE);
	}
	pDesc += SENSORNAME_SIZE * SUPPORT_VI_MAX_PIPE_NUM;
	strncpy(binGerrit, pDesc, BIN_GERRIT_SIZE);
	pDesc += BIN_GERRIT_SIZE;
	strncpy(pqbinVersion, pDesc, PQBINVERSION_SIZE);
	pDesc += PQBINVERSION_SIZE;
	strncpy(pqbinMode, pDesc, PQBINCREATE_MODE_SIZE);

	printf("PQBIN message\n");
	printf("%-15s%-15s%-15s%-15s\n", "gerritId:", binGerrit, "commitId:", binCommitId);
	printf("%-15s%-15s\n", "md5:", binMd5);
	printf("%-15s%-15s\n", "sensorNum", sensorNum);
	for (CVI_U8 i = 0; i < sensorInfo.num; ++i) {
		printf("%s%-5d%-15s\n", "sensorName", i, sensorName[i]);
	}
	printf("\n");

	//show pqbin pqtool branch & version & Mode
	printf("%-15s%-15s%-15s%-15s\n", "author:", pExtraInfo->Author, "desc:", pExtraInfo->Desc);
	printf("%-15s%-15s%-15s%-15s\n%-20s%-20s%-6s%-5s\n",
		"createTime:", pExtraInfo->Time, "version:", pqbinVersion,
		"tool Version:", toolVersion, "mode:", pqbinMode);
	printf("********************************************************************************\n");

	// check sensor
	for (CVI_U8 i = 0; i < sensorInfo.num; ++i) {
		if (strcmp(sensorInfo.name[i], sensorName[i]) != 0) {
			printf("sensorName(%d) mismatch, mwSns:%s != pqBinSns:%s\n",
				i, sensorInfo.name[i], sensorName[i]);
			return CVI_SUCCESS; //not check now
		}
	}

	//check md5
	if (strcmp(binMd5, ISP_BIN_MD5)) {
		printf("pqbin md5 mismatch, mwMd5:%s != pqBinMd5:%s\n", ISP_BIN_MD5, binMd5);
		return CVI_SUCCESS;
	}

	UNUSED(binSize);

	return CVI_SUCCESS;
}

static CVI_S32 isp_set_paramstruct(VI_PIPE ViPipe, ISP_Parameter_Structures *pstParaBuf)
{
	if (((ViPipe) < 0) || ((ViPipe) >= SUPPORT_VI_MAX_PIPE_NUM)) {
		CVI_TRACE_ISP_BIN(LOG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	if (pstParaBuf == CVI_NULL) {
		return CVI_FAILURE;
	}

	ISP_BIN_BYPASS_U binBypass = {0};

	CVI_S32 ret = isp_bin_getBinBypassParams(ViPipe, &binBypass);

	//Pub attr
	if (ret != CVI_SUCCESS || binBypass.bitBypassFrameRate != CVI_TRUE) {

		ISP_PUB_ATTR_S stPubAttr;

		CVI_ISP_GetPubAttr(ViPipe, &stPubAttr);
		stPubAttr.f32FrameRate = pstParaBuf->pub_attr.f32FrameRate;
		CVI_ISP_SetPubAttr(ViPipe, &stPubAttr);
	}

	// PRE_RAW
	CVI_ISP_SetBlackLevelAttr(ViPipe, &pstParaBuf->blc);
	CVI_ISP_SetLblcAttr(ViPipe, &pstParaBuf->lblc);
	CVI_ISP_SetLblcLutAttr(ViPipe, &pstParaBuf->lblcLut);
	CVI_ISP_SetRgbirAttr(ViPipe, &pstParaBuf->rgbir);
	CVI_ISP_SetDPDynamicAttr(ViPipe, &pstParaBuf->dpc_dynamic);
	CVI_ISP_SetDPStaticAttr(ViPipe, &pstParaBuf->dpc_static);
	CVI_ISP_SetCrosstalkAttr(ViPipe, &pstParaBuf->crosstalk);

	// Raw-top
	CVI_ISP_SetNRAttr(ViPipe, &pstParaBuf->bnr);
	CVI_ISP_SetNRFilterAttr(ViPipe, &pstParaBuf->bnr_filter);
	CVI_ISP_SetDemosaicAttr(ViPipe, &pstParaBuf->demosaic);
	CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &pstParaBuf->demosaic_demoire);
	CVI_ISP_SetRGBCACAttr(ViPipe, &pstParaBuf->rgbcac);
	CVI_ISP_SetLCACAttr(ViPipe, &pstParaBuf->lcac);
	CVI_ISP_SetDisAttr(ViPipe, &pstParaBuf->disAttr);
	CVI_ISP_SetDisConfig(ViPipe, &pstParaBuf->disConfig);

	// RGB-top
	CVI_ISP_SetMeshShadingAttr(ViPipe, &pstParaBuf->mlsc);
	CVI_ISP_SetMeshShadingGainLutAttr(ViPipe, &pstParaBuf->mlscLUT);
	CVI_ISP_SetSaturationAttr(ViPipe, &pstParaBuf->Saturation);
	CVI_ISP_SetCCMAttr(ViPipe, &pstParaBuf->ccm);
	CVI_ISP_SetCCMSaturationAttr(ViPipe, &pstParaBuf->ccm_saturation);
	CVI_ISP_SetColorToneAttr(ViPipe, &pstParaBuf->colortone);
	CVI_ISP_SetFSWDRAttr(ViPipe, &pstParaBuf->fswdr);
	CVI_ISP_SetWDRExposureAttr(ViPipe, &pstParaBuf->WDRExposure);
	CVI_ISP_SetDRCAttr(ViPipe, &pstParaBuf->drc);
	CVI_ISP_SetGammaAttr(ViPipe, &pstParaBuf->gamma);
	CVI_ISP_SetAutoGammaAttr(ViPipe, &pstParaBuf->autoGamma);
	CVI_ISP_SetDehazeAttr(ViPipe, &pstParaBuf->dehaze);
	CVI_ISP_SetClutAttr(ViPipe, &pstParaBuf->clut);
	CVI_ISP_SetClutHslAttr(ViPipe, &pstParaBuf->clut_hsl);
	CVI_ISP_SetCSCAttr(ViPipe, &pstParaBuf->csc);
	CVI_ISP_SetVCAttr(ViPipe, &pstParaBuf->vc_motion);

	// YUV-top
	CVI_ISP_SetDCIAttr(ViPipe, &pstParaBuf->dci);
	CVI_ISP_SetLDCIAttr(ViPipe, &pstParaBuf->ldci);
	CVI_ISP_SetPreSharpenAttr(ViPipe, &pstParaBuf->presharpen);
	CVI_ISP_SetTNRAttr(ViPipe, &pstParaBuf->tnr);
	CVI_ISP_SetTNRNoiseModelAttr(ViPipe, &pstParaBuf->tnr_noise_model);
	CVI_ISP_SetTNRLumaMotionAttr(ViPipe, &pstParaBuf->tnr_luma_motion);
	CVI_ISP_SetTNRGhostAttr(ViPipe, &pstParaBuf->tnr_ghost);
	CVI_ISP_SetTNRMtPrtAttr(ViPipe, &pstParaBuf->tnr_mt_prt);
	CVI_ISP_SetTNRMotionAdaptAttr(ViPipe, &pstParaBuf->tnr_motion_adapt);
	CVI_ISP_SetTNRMATFAttr(ViPipe, &pstParaBuf->tnr_matf);
	CVI_ISP_SetYNRAttr(ViPipe, &pstParaBuf->ynr);
	CVI_ISP_SetYNRFilterAttr(ViPipe, &pstParaBuf->ynr_filter);
	CVI_ISP_SetYNRMotionNRAttr(ViPipe, &pstParaBuf->ynr_motion);
	CVI_ISP_SetCNRAttr(ViPipe, &pstParaBuf->cnr);
	CVI_ISP_SetCNRMotionNRAttr(ViPipe, &pstParaBuf->cnr_motion);
	CVI_ISP_SetCACAttr(ViPipe, &pstParaBuf->cac);
	CVI_ISP_SetSharpenAttr(ViPipe, &pstParaBuf->sharpen);
	CVI_ISP_SetCAAttr(ViPipe, &pstParaBuf->ca);
	CVI_ISP_SetCA2Attr(ViPipe, &pstParaBuf->ca2);
	CVI_ISP_SetYContrastAttr(ViPipe, &pstParaBuf->ycontrast);

	// TEAISP
	CVI_TEAISP_BNR_SetAttr(ViPipe, &pstParaBuf->teaisp_bnr);
	CVI_TEAISP_BNR_SetNoiseProfileAttr(ViPipe, &pstParaBuf->teaisp_bnr_np);
	CVI_TEAISP_DRC_SetAttr(ViPipe, &pstParaBuf->teaisp_drc);
	CVI_TEAISP_PQ_SetAttr(ViPipe, &pstParaBuf->teaisp_pq);

	// Other
	CVI_ISP_SetNoiseProfileAttr(ViPipe, &pstParaBuf->np);
	CVI_ISP_SetMonoAttr(ViPipe, &pstParaBuf->mono);

	/*---------------------3A----------------------------*/
	// AE
	CVI_ISP_SetWDRExposureAttr(ViPipe, &pstParaBuf->WDRExpAttr);
	CVI_ISP_SetExposureAttr(ViPipe, &pstParaBuf->ExpAttr);
	CVI_ISP_SetAERouteAttr(ViPipe, &pstParaBuf->AeRouteAttr);
	CVI_ISP_SetAERouteAttrEx(ViPipe, &pstParaBuf->AeRouteAttrEx);
	CVI_ISP_SetSmartExposureAttr(ViPipe, &pstParaBuf->AeSmartExposureAttr);
	CVI_ISP_SetIrisAttr(ViPipe, &pstParaBuf->AeIrisAttr);
	CVI_ISP_SetDcirisAttr(ViPipe, &pstParaBuf->AeDcirisAttr);
	CVI_ISP_SetAERouteSFAttr(ViPipe, &pstParaBuf->AeRouteSFAttr);
	CVI_ISP_SetAERouteSFAttrEx(ViPipe, &pstParaBuf->AeRouteSFAttrEx);

	// AWB
	CVI_ISP_SetWBAttr(ViPipe, &pstParaBuf->WBAttr);
	CVI_ISP_SetAWBAttrEx(ViPipe, &pstParaBuf->AWBAttrEx);
	CVI_ISP_SetWBCalibration(ViPipe, &pstParaBuf->WBCalib);
	CVI_ISP_SetWBCalibrationEx(ViPipe, &pstParaBuf->WBCalibEx);
	CVI_ISP_SetStatisticsConfig(ViPipe, &pstParaBuf->StatCfg);

	return CVI_SUCCESS;
}

static CVI_S32 isp_get_paramstruct(VI_PIPE ViPipe, ISP_Parameter_Structures *pstParaBuf)
{
	if (((ViPipe) < 0) || ((ViPipe) >= SUPPORT_VI_MAX_PIPE_NUM)) {
		CVI_TRACE_ISP_BIN(LOG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	if (pstParaBuf == CVI_NULL) {
		return CVI_FAILURE;
	}

	//Pub attr
	CVI_ISP_GetPubAttr(ViPipe, &pstParaBuf->pub_attr);

	// PRE_RAW
	CVI_ISP_GetBlackLevelAttr(ViPipe, &pstParaBuf->blc);
	CVI_ISP_GetLblcAttr(ViPipe, &pstParaBuf->lblc);
	CVI_ISP_GetLblcLutAttr(ViPipe, &pstParaBuf->lblcLut);
	CVI_ISP_GetRgbirAttr(ViPipe, &pstParaBuf->rgbir);
	CVI_ISP_GetDPDynamicAttr(ViPipe, &pstParaBuf->dpc_dynamic);
	CVI_ISP_GetDPStaticAttr(ViPipe, &pstParaBuf->dpc_static);
	CVI_ISP_GetCrosstalkAttr(ViPipe, &pstParaBuf->crosstalk);

	// Raw-top
	CVI_ISP_GetNRAttr(ViPipe, &pstParaBuf->bnr);
	CVI_ISP_GetNRFilterAttr(ViPipe, &pstParaBuf->bnr_filter);
	CVI_ISP_GetDemosaicAttr(ViPipe, &pstParaBuf->demosaic);
	CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &pstParaBuf->demosaic_demoire);
	CVI_ISP_GetRGBCACAttr(ViPipe, &pstParaBuf->rgbcac);
	CVI_ISP_GetLCACAttr(ViPipe, &pstParaBuf->lcac);
	CVI_ISP_GetDisAttr(ViPipe, &pstParaBuf->disAttr);
	CVI_ISP_GetDisConfig(ViPipe, &pstParaBuf->disConfig);

	// RGB-top
	CVI_ISP_GetMeshShadingAttr(ViPipe, &pstParaBuf->mlsc);
	CVI_ISP_GetMeshShadingGainLutAttr(ViPipe, &pstParaBuf->mlscLUT);
	CVI_ISP_GetSaturationAttr(ViPipe, &pstParaBuf->Saturation);
	CVI_ISP_GetCCMAttr(ViPipe, &pstParaBuf->ccm);
	CVI_ISP_GetCCMSaturationAttr(ViPipe, &pstParaBuf->ccm_saturation);
	CVI_ISP_GetColorToneAttr(ViPipe, &pstParaBuf->colortone);
	CVI_ISP_GetFSWDRAttr(ViPipe, &pstParaBuf->fswdr);
	CVI_ISP_GetWDRExposureAttr(ViPipe, &pstParaBuf->WDRExposure);
	CVI_ISP_GetDRCAttr(ViPipe, &pstParaBuf->drc);
	CVI_ISP_GetGammaAttr(ViPipe, &pstParaBuf->gamma);
	CVI_ISP_GetAutoGammaAttr(ViPipe, &pstParaBuf->autoGamma);
	CVI_ISP_GetDehazeAttr(ViPipe, &pstParaBuf->dehaze);
	CVI_ISP_GetClutAttr(ViPipe, &pstParaBuf->clut);
	CVI_ISP_GetClutHslAttr(ViPipe, &pstParaBuf->clut_hsl);
	CVI_ISP_GetCSCAttr(ViPipe, &pstParaBuf->csc);
	CVI_ISP_GetVCAttr(ViPipe, &pstParaBuf->vc_motion);

	// YUV-top
	CVI_ISP_GetDCIAttr(ViPipe, &pstParaBuf->dci);
	CVI_ISP_GetLDCIAttr(ViPipe, &pstParaBuf->ldci);
	CVI_ISP_GetPreSharpenAttr(ViPipe, &pstParaBuf->presharpen);
	CVI_ISP_GetTNRAttr(ViPipe, &pstParaBuf->tnr);
	CVI_ISP_GetTNRNoiseModelAttr(ViPipe, &pstParaBuf->tnr_noise_model);
	CVI_ISP_GetTNRLumaMotionAttr(ViPipe, &pstParaBuf->tnr_luma_motion);
	CVI_ISP_GetTNRGhostAttr(ViPipe, &pstParaBuf->tnr_ghost);
	CVI_ISP_GetTNRMtPrtAttr(ViPipe, &pstParaBuf->tnr_mt_prt);
	CVI_ISP_GetTNRMotionAdaptAttr(ViPipe, &pstParaBuf->tnr_motion_adapt);
	CVI_ISP_GetTNRMATFAttr(ViPipe, &pstParaBuf->tnr_matf);
	CVI_ISP_GetYNRAttr(ViPipe, &pstParaBuf->ynr);
	CVI_ISP_GetYNRFilterAttr(ViPipe, &pstParaBuf->ynr_filter);
	CVI_ISP_GetYNRMotionNRAttr(ViPipe, &pstParaBuf->ynr_motion);
	CVI_ISP_GetCNRAttr(ViPipe, &pstParaBuf->cnr);
	CVI_ISP_GetCNRMotionNRAttr(ViPipe, &pstParaBuf->cnr_motion);
	CVI_ISP_GetCACAttr(ViPipe, &pstParaBuf->cac);
	CVI_ISP_GetSharpenAttr(ViPipe, &pstParaBuf->sharpen);
	CVI_ISP_GetCAAttr(ViPipe, &pstParaBuf->ca);
	CVI_ISP_GetCA2Attr(ViPipe, &pstParaBuf->ca2);
	CVI_ISP_GetYContrastAttr(ViPipe, &pstParaBuf->ycontrast);

	// TEAISP
	CVI_TEAISP_BNR_GetAttr(ViPipe, &pstParaBuf->teaisp_bnr);
	CVI_TEAISP_BNR_GetNoiseProfileAttr(ViPipe, &pstParaBuf->teaisp_bnr_np);
	CVI_TEAISP_DRC_GetAttr(ViPipe, &pstParaBuf->teaisp_drc);
	CVI_TEAISP_PQ_GetAttr(ViPipe, &pstParaBuf->teaisp_pq);

	// other
	CVI_ISP_GetNoiseProfileAttr(ViPipe, &pstParaBuf->np);
	CVI_ISP_GetMonoAttr(ViPipe, &pstParaBuf->mono);

	/*---------------------3A----------------------------*/
	// AE
	CVI_ISP_GetWDRExposureAttr(ViPipe, &pstParaBuf->WDRExpAttr);
	CVI_ISP_GetExposureAttr(ViPipe, &pstParaBuf->ExpAttr);
	CVI_ISP_GetAERouteAttr(ViPipe, &pstParaBuf->AeRouteAttr);
	CVI_ISP_GetAERouteAttrEx(ViPipe, &pstParaBuf->AeRouteAttrEx);
	CVI_ISP_GetSmartExposureAttr(ViPipe, &pstParaBuf->AeSmartExposureAttr);
	CVI_ISP_GetIrisAttr(ViPipe, &pstParaBuf->AeIrisAttr);
	CVI_ISP_GetDcirisAttr(ViPipe, &pstParaBuf->AeDcirisAttr);
	CVI_ISP_GetAERouteSFAttr(ViPipe, &pstParaBuf->AeRouteSFAttr);
	CVI_ISP_GetAERouteSFAttrEx(ViPipe, &pstParaBuf->AeRouteSFAttrEx);

	// AWB
	CVI_ISP_GetWBAttr(ViPipe, &pstParaBuf->WBAttr);
	CVI_ISP_GetAWBAttrEx(ViPipe, &pstParaBuf->AWBAttrEx);
	CVI_ISP_GetWBCalibration(ViPipe, &pstParaBuf->WBCalib);
	CVI_ISP_GetWBCalibrationEx(ViPipe, &pstParaBuf->WBCalibEx);
	CVI_ISP_GetStatisticsConfig(ViPipe, &pstParaBuf->StatCfg);

	return CVI_SUCCESS;
}

CVI_S32 isp_bin_setBinBypassParams(VI_PIPE ViPipe, ISP_BIN_BYPASS_U *ispBinBypass)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BIN_BYPASS_U binBypassParam = {0};

	if (((ViPipe) < 0) || ((ViPipe) >= SUPPORT_VI_MAX_PIPE_NUM)) {
		CVI_TRACE_ISP_BIN(LOG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	if (ispBinBypass == NULL) {
		return CVI_FAILURE;
	}

	memcpy(&binBypassParam, ispBinBypass, sizeof(ISP_BIN_BYPASS_U));
	g_binBypassParams.ispBinBypass[ViPipe] = binBypassParam;

	return ret;
}

CVI_S32 isp_bin_getBinBypassParams(VI_PIPE ViPipe, ISP_BIN_BYPASS_U *ispBinBypass)
{
	CVI_S32 ret = CVI_SUCCESS;

	if (((ViPipe) < 0) || ((ViPipe) >= SUPPORT_VI_MAX_PIPE_NUM)) {
		CVI_TRACE_ISP_BIN(LOG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	if (ispBinBypass == NULL) {
		return CVI_FAILURE;
	}

	memcpy(ispBinBypass, &g_binBypassParams.ispBinBypass[ViPipe], sizeof(ISP_BIN_BYPASS_U));

	return ret;
}

/**************************************************************************
 *   Index bin related APIs.
 **************************************************************************/
CVI_S32 cvi_bin_getIndexInitHeader(CVI_BIN_INDEX_HEADER *idx_header)
{
	CVI_S32 ret = CVI_SUCCESS;

	if (idx_header == NULL) {
		return CVI_FAILURE;
	}
	ret = cvi_bin_getIndexInitHeader_autogen(idx_header);

	return ret;
}

CVI_S32 cvi_bin_getIndexTable(Entry *entrys)
{
	CVI_S32 ret = CVI_SUCCESS;

	if (entrys == NULL) {
		return CVI_FAILURE;
	}
	ret = cvi_bin_getIndexTable_autogen(entrys);

	return ret;
}

CVI_S32 isp_index_bin_getparamfrombin(CVI_U8 *buffer, enum CVI_BIN_SECTION_ID id,
										CVI_U32 binSize, CVI_U32 indexOffset)
{
	CVI_S32 ret = CVI_SUCCESS;

	ret = isp_index_bin_getparamfrombinImp(id - CVI_BIN_ID_ISP0, buffer, binSize, indexOffset);

	return ret;
}

CVI_S32 cvi_bin_computeIndexHeaderOffset(CVI_BIN_INDEX_HEADER *idx_header, CVI_U32 bin_header_size)
{
	CVI_S32 ret = CVI_SUCCESS;

	if (idx_header == NULL || bin_header_size == 0) {
		return CVI_FAILURE;
	}

	ret = cvi_bin_computeIndexHeaderOffset_autogen(idx_header, bin_header_size);

	return ret;
}