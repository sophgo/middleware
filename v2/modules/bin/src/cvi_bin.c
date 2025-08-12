#include <sys/stat.h>
#include "stdlib.h"
#include "cvi_bin.h"
#include "cvi_vi.h"
#include <cvi_comm_sys.h>
#include <cvi_comm_vi.h>
#include "isp_bin.h"
#include "vpss_bin.h"
#include "vo_bin.h"
#include "md5.h"

#define MAGIC_NUMBER 0x1835 /*the macro is used as magic number and can't be modified!*/

//following info is V1.0 and V1.1 bin header field. difference of V1.0 and V1.1 is end of  V1.1's file that
//contains the whole file PQbin's md5 value.

/***The following macro comes from module/bin/cv18xx/isp_bin/src/isp_bin.c***/
//CVI_BIN_EXTRA_S.Desc[624-1023] 400 bytes
//[pqbin md5 50] + [reserved 50]
//[tool version 40] + [isp commitId 10] + [sensorNum 9] + [sensorName1 27] + [sensorName2 27]
//[sensorName3 27]  + [sensorName4 27] + [sensorName5 27]  + [sensorName6 27]  + [sensorName7 27]  + [sensorName8 27]
//[isp branch 20] + [version 4] + [generate mode - A:auto M:Manual 1]

#define PQBIN_VERSION_CHECK_V13 "V1.3"	// The lowest version used for verification
										// v1.3 newly added index of pqbin
#define DESC_SIZE 624
#define BIN_MD5_SIZE 50
#define PQBIN_RESERVE_SIZE 50
#define TOOLVERSION_SIZE 40
#define BIN_COMMIT_SIZE 10
#define SENSORNUM_SIZE 9
#define SENSORNAME_SIZE 27
#define BIN_GERRIT_SIZE 20
#define PQBINVERSION_SIZE 4
#define PQBINCREATE_MODE_SIZE 1
#define SUPPORT_VI_MAX_PIPE_NUM 8
/****************************************************************************/
struct BIN_BUF_INFO {
	CVI_U32 u32BlkSize[CVI_BIN_ID_MAX];
	CVI_U32 u32BinParaSize;
	CVI_U32 u32IndexParaSize;
	CVI_U32 u32ParaTotalSize;
	CVI_U32 u32SensorNumber;
	CVI_CHAR achBinVersion[PQBINVERSION_SIZE + 1];
	CVI_CHAR achBinMode[PQBINCREATE_MODE_SIZE + 1];
	CVI_CHAR achBinMD5Code[BIN_MD5_SIZE + 1];
	CVI_CHAR achTotalParaMD5Value[MD5_STRING_LEN];
};

enum INFO_MODE {
	BLOCK_SIZE_ERR,
	PARSE_SUCCESS_FROM_BIN_INDEX,
	IMPORT_FAIL
};

static struct BIN_BUF_INFO g_stBinBufInfo = {0};
static CVI_BOOL g_bUseOldLoadAPI = CVI_TRUE;
static CVI_U32 g_u32CurIspBinSize;

static char userBinName[WDR_MODE_MAX][BIN_FILE_LENGTH] = {
	"/mnt/cfg/param/cvi_sdr_bin", "/mnt/cfg/param/cvi_sdr_bin",
	"/mnt/cfg/param/cvi_sdr_bin", "/mnt/cfg/param/cvi_wdr_bin",
	"/mnt/cfg/param/cvi_wdr_bin", "/mnt/cfg/param/cvi_wdr_bin",
	"/mnt/cfg/param/cvi_wdr_bin", "/mnt/cfg/param/cvi_wdr_bin",
	"/mnt/cfg/param/cvi_wdr_bin", "/mnt/cfg/param/cvi_wdr_bin",
	"/mnt/cfg/param/cvi_wdr_bin", "/mnt/cfg/param/cvi_wdr_bin" };
static CVI_S32 _setBinNameImp(WDR_MODE_E wdrMode, const CVI_CHAR *binName);
static CVI_S32 _getBinNameImp(CVI_CHAR *binName);
static CVI_S32 check_is_register_id(enum CVI_BIN_SECTION_ID id);
static CVI_S32 id_is_valid_IspID(enum CVI_BIN_SECTION_ID id);
static CVI_S32 get_index_para_from_buffer(enum CVI_BIN_SECTION_ID id, CVI_U8 *buf, CVI_U32 binSize);
static CVI_BOOL check_bin_file_is_new_version(CVI_CHAR *pchVersion);
static CVI_S32 write_md5_value_to_buf(CVI_U8 *buf, CVI_U32 u32BufSize);
static void print_import_info(enum CVI_BIN_SECTION_ID id, enum INFO_MODE mode);

static pfn_cvi_bin_getbinsize getBinSizeFunc[CVI_BIN_ID_MAX] = {
	header_bin_getBinSize, /*CVI_BIN_ID_HEADER*/
	isp_bin_getBinSize, /*CVI_BIN_ID_ISP0*/
	isp_bin_getBinSize, /*CVI_BIN_ID_ISP1*/
	isp_bin_getBinSize, /*CVI_BIN_ID_ISP2*/
	isp_bin_getBinSize, /*CVI_BIN_ID_ISP3*/
	isp_bin_getBinSize, /*CVI_BIN_ID_ISP4*/
	isp_bin_getBinSize, /*CVI_BIN_ID_ISP5*/
	isp_bin_getBinSize, /*CVI_BIN_ID_ISP6*/
	isp_bin_getBinSize, /*CVI_BIN_ID_ISP7*/
	vpss_bin_getbinsize,	/*CVI_BIN_ID_VPSS*/
	NULL, /*CVI_BIN_ID_VDEC*/
	NULL, /*CVI_BIN_ID_VENC*/
	vo_bin_getbinsize, /*CVI_BIN_ID_VO0*/
	vo_bin_getbinsize, /*CVI_BIN_ID_VO1*/
};
static pfn_cvi_bin_getparamfrombin getParamFromBinFunc[CVI_BIN_ID_MAX] = {
	header_bin_getBinParam, /*CVI_BIN_ID_HEADER*/
	isp_bin_getBinParam, /*CVI_BIN_ID_ISP0*/
	isp_bin_getBinParam, /*CVI_BIN_ID_ISP1*/
	isp_bin_getBinParam, /*CVI_BIN_ID_ISP2*/
	isp_bin_getBinParam, /*CVI_BIN_ID_ISP3*/
	isp_bin_getBinParam, /*CVI_BIN_ID_ISP4*/
	isp_bin_getBinParam, /*CVI_BIN_ID_ISP5*/
	isp_bin_getBinParam, /*CVI_BIN_ID_ISP6*/
	isp_bin_getBinParam, /*CVI_BIN_ID_ISP7*/
	vpss_bin_getparamfrombin, /*CVI_BIN_ID_VPSS*/
	NULL, /*CVI_BIN_ID_VDEC*/
	NULL, /*CVI_BIN_ID_VENC*/
	vo_bin_getparamfrombin, /*CVI_BIN_ID_VO0*/
	vo_bin_getparamfrombin, /*CVI_BIN_ID_VO1*/
};
static pfn_cvi_bin_setparamtobuf setParamToBufFunc[CVI_BIN_ID_MAX] = {
	header_bin_setBinParambuf, /*CVI_BIN_ID_HEADER*/
	isp_bin_setBinParambuf, /*CVI_BIN_ID_ISP0*/
	isp_bin_setBinParambuf, /*CVI_BIN_ID_ISP1*/
	isp_bin_setBinParambuf, /*CVI_BIN_ID_ISP2*/
	isp_bin_setBinParambuf, /*CVI_BIN_ID_ISP3*/
	isp_bin_setBinParambuf, /*CVI_BIN_ID_ISP4*/
	isp_bin_setBinParambuf, /*CVI_BIN_ID_ISP5*/
	isp_bin_setBinParambuf, /*CVI_BIN_ID_ISP6*/
	isp_bin_setBinParambuf, /*CVI_BIN_ID_ISP7*/
	vpss_bin_setparamtobuf,	/*CVI_BIN_ID_VPSS*/
	NULL, /*CVI_BIN_ID_VDEC*/
	NULL, /*CVI_BIN_ID_VENC*/
	vo_bin_setparamtobuf, /*CVI_BIN_ID_VO0*/
	vo_bin_setparamtobuf, /*CVI_BIN_ID_VO1*/
};

static pfn_cvi_indexbin_getparamfrombin getIndexBinParamFromBinFunc[CVI_BIN_ID_MAX] = {
	NULL, /*CVI_INDEX_BIN_ID_HEADER*/
	isp_index_bin_getparamfrombin,  /*CVI_BIN_ID_ISP0*/
	isp_index_bin_getparamfrombin,  /*CVI_BIN_ID_ISP1*/
	isp_index_bin_getparamfrombin,  /*CVI_BIN_ID_ISP2*/
	isp_index_bin_getparamfrombin,  /*CVI_BIN_ID_ISP3*/
	isp_index_bin_getparamfrombin,  /*CVI_BIN_ID_ISP4*/
	isp_index_bin_getparamfrombin,  /*CVI_BIN_ID_ISP5*/
	isp_index_bin_getparamfrombin,  /*CVI_BIN_ID_ISP6*/
	isp_index_bin_getparamfrombin,  /*CVI_BIN_ID_ISP7*/
	vpss_index_bin_getparamfrombin, /*CVI_BIN_ID_VPSS*/
	NULL, /*CVI_BIN_ID_VDEC*/
	NULL, /*CVI_BIN_ID_VENC*/
	vo_index_bin_getparamfrombin,   /*CVI_BIN_ID_VO0*/
	vo_index_bin_getparamfrombin,   /*CVI_BIN_ID_VO1*/
};

static CVI_S32 _isSectionIdValid(enum CVI_BIN_SECTION_ID id)
{
	CVI_S32 ret = CVI_SUCCESS;

	if ((id < CVI_BIN_ID_MIN) || (id >= CVI_BIN_ID_MAX)) {
		ret = CVI_FAILURE;
	}

	return ret;
}

static struct BIN_BUF_INFO *get_current_buf_info(void)
{
	return &g_stBinBufInfo;
}

static CVI_S32 get_bin_Info_from_buf(CVI_U8 *buf, struct BIN_BUF_INFO *pstBufInfo, CVI_U32 u32DataLen)
{
	CVI_S32 ret = CVI_SUCCESS;
	CVI_BIN_HEADER *pstHeader = (CVI_BIN_HEADER *)buf;
	CVI_BIN_INDEX_HEADER *pstIdxHeader = NULL;
	CVI_CHAR *pchDesc = (CVI_CHAR *)pstHeader->extraInfo.Desc;
	CVI_U32 u32BinSize = 0;
	CVI_U32 u32IndexSize = 0;
	CVI_CHAR achSensorNum[SENSORNUM_SIZE];

	/*get sensor number,version and mode from buffer.*/
	memset(achSensorNum, 0, SENSORNUM_SIZE);
	memset(pstBufInfo->achBinVersion, 0, sizeof(pstBufInfo->achBinVersion));
	memset(pstBufInfo->achBinMode, 0, sizeof(pstBufInfo->achBinMode));
	memset(pstBufInfo->achBinMD5Code, 0, sizeof(pstBufInfo->achBinMD5Code));
	pchDesc += DESC_SIZE;
	strncpy(pstBufInfo->achBinMD5Code, pchDesc, BIN_MD5_SIZE);
	pchDesc += BIN_MD5_SIZE;
	pchDesc += PQBIN_RESERVE_SIZE;
	pchDesc += TOOLVERSION_SIZE;
	pchDesc += BIN_COMMIT_SIZE;
	snprintf(achSensorNum, sizeof(achSensorNum), "%s", pchDesc);
	pstBufInfo->u32SensorNumber = (CVI_U32)atoi(achSensorNum);
	pchDesc += SENSORNUM_SIZE;
	pchDesc += SENSORNAME_SIZE * SUPPORT_VI_MAX_PIPE_NUM;
	pchDesc += BIN_GERRIT_SIZE;
	strncpy(pstBufInfo->achBinVersion, pchDesc, PQBINVERSION_SIZE);
	pchDesc += PQBINVERSION_SIZE;
	strncpy(pstBufInfo->achBinMode, pchDesc, PQBINCREATE_MODE_SIZE);

	/*get bin size total size.*/
	for (CVI_U32 idx = CVI_BIN_ID_MIN; idx < CVI_BIN_ID_MAX; idx++) {
		if (getBinSizeFunc[idx] != NULL) {
			pstBufInfo->u32BlkSize[idx] = pstHeader->size[idx];
			u32BinSize += pstHeader->size[idx];
		}
	}

	/*get index size total size.*/
	if (check_bin_file_is_new_version(pstBufInfo->achBinVersion)) {
		pstIdxHeader = (CVI_BIN_INDEX_HEADER *)(buf + u32BinSize);
		u32IndexSize = pstIdxHeader->size[0];
	}

	/*set bufinfo struct.*/
	pstBufInfo->u32BinParaSize = u32BinSize;
	pstBufInfo->u32IndexParaSize = u32IndexSize;
	pstBufInfo->u32ParaTotalSize = u32BinSize + u32IndexSize;
	if (strncmp(pstBufInfo->achBinVersion, PQBIN_VERSION_CHECK_V13, PQBINVERSION_SIZE) >= 0) {
		snprintf(pstBufInfo->achTotalParaMD5Value, MD5_STRING_LEN, "%s",
					(CVI_CHAR *)(buf + u32DataLen - MD5_STRING_LEN));

		pstBufInfo->u32ParaTotalSize += MD5_STRING_LEN;
	}

	return ret;
}

static CVI_S32 id_is_valid_IspID(enum CVI_BIN_SECTION_ID id)
{
	if ((id <= CVI_BIN_ID_ISP7) && (id >= CVI_BIN_ID_ISP0)) {
		return CVI_SUCCESS;
	}

	return CVI_FAILURE;
}

static CVI_S32 check_is_register_id(enum CVI_BIN_SECTION_ID id)
{
	if (id_is_valid_IspID(id) == CVI_SUCCESS) {
		if (CVI_VI_QueryDevStatus(id - CVI_BIN_ID_ISP0) != CVI_SUCCESS) {
			return CVI_BIN_MODULE_NOT_REGISTER_ERROR;
		}
	}

	if (((id < CVI_BIN_ID_MAX) && (getBinSizeFunc[id] == NULL)) || (id > CVI_BIN_ID_MAX)) {
		return CVI_BIN_MODULE_NOT_REGISTER_ERROR;
	}

	return CVI_SUCCESS;
}

static void print_import_info(enum CVI_BIN_SECTION_ID idx, enum INFO_MODE mode)
{
	switch (idx) {
	case CVI_BIN_ID_HEADER: {
		if (mode == BLOCK_SIZE_ERR) {
			CVI_TRACE_SYS(CVI_DBG_WARN, "The size of header is 0, import failed for header!\n");
		} else if (mode == IMPORT_FAIL) {
			CVI_TRACE_SYS(CVI_DBG_WARN, "Import failed for header!\n");
		}
		break;
	}
	case CVI_BIN_ID_ISP0:
	case CVI_BIN_ID_ISP1:
	case CVI_BIN_ID_ISP2:
	case CVI_BIN_ID_ISP3:
	case CVI_BIN_ID_ISP4:
	case CVI_BIN_ID_ISP5:
	case CVI_BIN_ID_ISP6:
	case CVI_BIN_ID_ISP7: {
		if (mode == BLOCK_SIZE_ERR) {
			CVI_TRACE_SYS(
			CVI_DBG_WARN,
			"The size of sensor_%d is 0, import failed for sensor_%d!\n",
			idx - CVI_BIN_ID_ISP0, idx - CVI_BIN_ID_ISP0
			);
		} else if (mode == PARSE_SUCCESS_FROM_BIN_INDEX) {
			CVI_TRACE_LOG(CVI_DBG_WARN, "Get para of sensor_%d by bin index table!\n", idx - CVI_BIN_ID_ISP0);
		} else if (mode == IMPORT_FAIL) {
			CVI_TRACE_SYS(CVI_DBG_WARN, "Import failed for sensor_%d!\n", idx - CVI_BIN_ID_ISP0);
		}
		break;
	}
	case CVI_BIN_ID_VPSS: {
		if (mode == BLOCK_SIZE_ERR) {
			CVI_TRACE_SYS(CVI_DBG_WARN, "The size of VPSS is 0, import failed for VPSS!\n");
		} else if (mode == PARSE_SUCCESS_FROM_BIN_INDEX) {
			CVI_TRACE_LOG(CVI_DBG_WARN, "Get para of VPSS by bin index table!\n");
		} else if (mode == IMPORT_FAIL) {
			CVI_TRACE_SYS(CVI_DBG_WARN, "Import failed for VPSS!\n");
		}
		break;
	}
	case CVI_BIN_ID_VO0:
	case CVI_BIN_ID_VO1: {
		if (mode == BLOCK_SIZE_ERR) {
			CVI_TRACE_SYS(
			CVI_DBG_WARN,
			"The size of vo_%d is 0, import failed for vo_%d!\n",
			idx - CVI_BIN_ID_VO0, idx - CVI_BIN_ID_VO0
			);
		} else if (mode == PARSE_SUCCESS_FROM_BIN_INDEX) {
			CVI_TRACE_LOG(CVI_DBG_WARN, "Get para of vo_%d by bin index table!\n", idx - CVI_BIN_ID_VO0);
		} else if (mode == IMPORT_FAIL) {
			CVI_TRACE_SYS(CVI_DBG_WARN, "Import failed for vo_%d!\n", idx - CVI_BIN_ID_VO0);
		}
		break;
	}
	case CVI_BIN_ID_VDEC:
	case CVI_BIN_ID_VENC:
	default:
		break;
	}
}

/* Static function */
static CVI_S32 _setBinNameImp(WDR_MODE_E wdrMode, const CVI_CHAR *binName)
{
	if (binName == NULL) {
		CVI_TRACE_SYS(CVI_DBG_WARN, "binName is NULL\n");
		return CVI_FAILURE;
	}

	CVI_U32 len = (CVI_U32)strlen(binName);

	if (len >= BIN_FILE_LENGTH) {
		CVI_TRACE_SYS(CVI_DBG_WARN, "Set bin name failed, strlen(%u) >= %d!\n", len, BIN_FILE_LENGTH);
		return CVI_FAILURE;
	}

	strncpy(userBinName[wdrMode], binName, BIN_FILE_LENGTH);

	return CVI_SUCCESS;
}

static CVI_S32 _getBinNameImp(CVI_CHAR *binName)
{
	if (binName == NULL) {
		CVI_TRACE_SYS(CVI_DBG_WARN, "binName is NULL\n");
		return CVI_FAILURE;
	}

	VI_DEV_ATTR_S pstDevAttr;
	WDR_MODE_E wdrMode;

	CVI_VI_GetDevAttr(0, &pstDevAttr);
	wdrMode = pstDevAttr.stWDRAttr.enWDRMode;

	sprintf(binName, "%s", userBinName[wdrMode]);

	return CVI_SUCCESS;
}

static CVI_BOOL check_bin_file_is_new_version(CVI_CHAR *pchVersion)
{
	if (strncmp(pchVersion, PQBIN_VERSION_CHECK_V13, strlen(PQBIN_VERSION_CHECK_V13)) >= 0) {
		return CVI_TRUE;
	} else {
		return CVI_FALSE;
	}
}

static CVI_S32 check_bin_file_validity(CVI_U8 *buf, CVI_U32 u32DataLen)
{
	CVI_S32 ret = CVI_SUCCESS;
	CVI_U8 au8Md5Value[MD5_STRING_LEN] = { 0 };
	CVI_BIN_HEADER *pstHeader = (CVI_BIN_HEADER *)buf;
	struct BIN_BUF_INFO *pstBufInfo = get_current_buf_info();

	if (pstHeader->chipId != MAGIC_NUMBER) {
		ret = CVI_BIN_FILE_ERROR;
		CVI_TRACE_SYS(CVI_DBG_ERR, "PQbin file isn't valid!\n");
		goto FINISH_HANDLER;
	}

	/*Get PQbin information from buf.*/
	get_bin_Info_from_buf(buf, pstBufInfo, u32DataLen);

	/*calcute md5 of buf.*/
	if (strncmp(pstBufInfo->achBinVersion, PQBIN_VERSION_CHECK_V13, PQBINVERSION_SIZE) >= 0) {
		calcute_md5_value(buf, u32DataLen - MD5_STRING_LEN, au8Md5Value);
		if (strncmp((char *)au8Md5Value, (char *)pstBufInfo->achTotalParaMD5Value,
			MD5_STRING_LEN) != 0) {
			ret = CVI_BIN_DATA_ERR;
			CVI_TRACE_SYS(CVI_DBG_ERR, "Data of PQbin file isn't normal!\n");
		}
	} else {
		ret = CVI_BIN_FILE_ERROR;
		CVI_TRACE_SYS(LOG_ERR,
			"The version of bin file is %s, cannot meet the minimum version %s!\n",
			pstBufInfo->achBinVersion, PQBIN_VERSION_CHECK_V13);
		goto FINISH_HANDLER;
	}

FINISH_HANDLER:
	return ret;
}

static CVI_S32 get_index_para_from_buffer(enum CVI_BIN_SECTION_ID id, CVI_U8 *buf, CVI_U32 binSize)
{
	CVI_S32 ret = CVI_SUCCESS;
	CVI_U32 indexOffset = 0;

	ret = _isSectionIdValid(id);
	if (ret != CVI_SUCCESS) {
		ret = CVI_BIN_ID_ERROR;
		CVI_TRACE_SYS(LOG_ERR, "Invalid id(%d)\n", id);
		goto ERROR_HANDLER;
	}

	if (id == CVI_BIN_ID_HEADER) {
		goto ERROR_HANDLER;
	}

	if (buf == NULL) {
		ret = CVI_BIN_NULL_POINT;
		CVI_TRACE_SYS(LOG_ERR, "input buffer pointer is null!");
		goto ERROR_HANDLER;
	}
	if (getIndexBinParamFromBinFunc[id] == NULL) {
		ret = CVI_BIN_NULL_POINT;
		CVI_TRACE_SYS(LOG_ERR, "get index callback isn't registered!");
		goto ERROR_HANDLER;
	}

	/*get addr of bin header*/
	CVI_BIN_HEADER *header = (CVI_BIN_HEADER *)buf;

	for (CVI_U32 idx = CVI_BIN_ID_MIN; idx < CVI_BIN_ID_MAX; idx++) {
		indexOffset += header->size[idx];
	}

	/*load bin para to struct by index*/
	ret = getIndexBinParamFromBinFunc[id](buf, id, binSize, indexOffset);

ERROR_HANDLER:

	return ret;
}

static CVI_S32 write_md5_value_to_buf(CVI_U8 *buf, CVI_U32 u32BufSize)
{
	CVI_S32 ret = CVI_SUCCESS;
	CVI_U8 au8Md5Value[MD5_STRING_LEN] = { 0 };
	CVI_U32 u32ValidDataLen = u32BufSize - MD5_STRING_LEN;

	calcute_md5_value(buf, u32ValidDataLen, au8Md5Value);
	memcpy(buf + u32ValidDataLen, au8Md5Value, MD5_STRING_LEN);

	return ret;
}

static enum CVI_BIN_SECTION_ID get_existent_id_in_bin(enum CVI_BIN_SECTION_ID idx, CVI_BIN_HEADER *pstHeader)
{
	if ((idx >= CVI_BIN_ID_ISP0) && (idx <= CVI_BIN_ID_ISP7)) {
		if (pstHeader->size[idx] == 0) {
			for (CVI_U32 id = CVI_BIN_ID_ISP0; id <= CVI_BIN_ID_ISP7; id++) {
				if (pstHeader->size[id] != 0) {
					return id;
				}
			}
		}
	}

	return idx;
}

static CVI_S32 reset_id_size_in_bin(enum CVI_BIN_SECTION_ID src_id, enum CVI_BIN_SECTION_ID dst_id, const CVI_U8 *buf)
{
	CVI_BIN_HEADER *pstHeader = (CVI_BIN_HEADER *)buf;
	CVI_U8 *addr = (CVI_U8 *)buf;
	CVI_U32 secSize = pstHeader->size[CVI_BIN_ID_HEADER] - sizeof(CVI_U32) - sizeof(CVI_BIN_EXTRA_S);
	CVI_U32 secCnt = secSize / sizeof(CVI_U32);
	CVI_BIN_INDEX_HEADER *pheader_index = NULL;

	g_u32CurIspBinSize = pstHeader->size[src_id];
	pstHeader->size[src_id] = 0;
	pstHeader->size[dst_id] = g_u32CurIspBinSize;

	for (CVI_U32 idx = CVI_BIN_ID_MIN; idx < secCnt; idx++) {
		addr += pstHeader->size[idx];
	}
	pheader_index = (CVI_BIN_INDEX_HEADER *)(addr);
	pheader_index->size[src_id] = 0;
	pheader_index->size[dst_id] = g_u32CurIspBinSize;

	CVI_U32 offsetData = pstHeader->size[CVI_BIN_ID_HEADER];

	for (CVI_U32 idx = CVI_BIN_ID_ISP0; idx < secCnt; idx++) {
		pheader_index->offsetData[idx] = offsetData;
		offsetData += pheader_index->size[idx];
	}

	return CVI_SUCCESS;
}

static CVI_S32 restore_init_id_size_in_bin(enum CVI_BIN_SECTION_ID src_id, enum CVI_BIN_SECTION_ID dst_id,
	const CVI_U8 *buf)
{
	CVI_BIN_HEADER *pstHeader = (CVI_BIN_HEADER *)buf;
	CVI_U8 *addr = (CVI_U8 *)buf;
	CVI_U32 secSize = pstHeader->size[CVI_BIN_ID_HEADER] - sizeof(CVI_U32) - sizeof(CVI_BIN_EXTRA_S);
	CVI_U32 secCnt = secSize / sizeof(CVI_U32);
	CVI_BIN_INDEX_HEADER *pheader_index = NULL;

	pstHeader->size[dst_id] = 0;
	pstHeader->size[src_id] = g_u32CurIspBinSize;

	for (CVI_U32 idx = CVI_BIN_ID_MIN; idx < secCnt; idx++) {
		addr += pstHeader->size[idx];
	}
	pheader_index = (CVI_BIN_INDEX_HEADER *)(addr);
	pheader_index->size[dst_id] = 0;
	pheader_index->size[src_id] = g_u32CurIspBinSize;

	CVI_U32 offsetData = pstHeader->size[CVI_BIN_ID_HEADER];

	for (CVI_U32 idx = CVI_BIN_ID_ISP0; idx < secCnt; idx++) {
		pheader_index->offsetData[idx] = offsetData;
		offsetData += pheader_index->size[idx];
	}

	return CVI_SUCCESS;
}

static CVI_BOOL is_target_id(enum CVI_BIN_SECTION_ID src_id, enum CVI_BIN_SECTION_ID dst_id)
{
	CVI_BOOL ret = CVI_TRUE;

	if (src_id != CVI_BIN_ID_MIN) {
		if ((src_id != dst_id) && (dst_id != CVI_BIN_ID_MAX)) {
			ret = CVI_FALSE;
		}
	}

	return ret;
}

CVI_U32 CVI_BIN_GetBinTotalLen(void)
{
	return CVI_BIN_GetSingleISPBinLen(CVI_BIN_ID_ALL);
}

CVI_S32 CVI_BIN_ExportBinData(CVI_U8 *pu8Buffer, CVI_U32 u32DataLength)
{
	return CVI_BIN_ExportSingleISPBinData(CVI_BIN_ID_ALL, pu8Buffer, u32DataLength);
}

CVI_S32 CVI_BIN_ImportBinData(CVI_U8 *pu8Buffer, CVI_U32 u32DataLength)
{
	CVI_S32 ret, tmpRet;

	if (pu8Buffer == NULL) {
		return CVI_BIN_NULL_POINT;
	}
	if (u32DataLength == 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "input length of buffer can't be 0!\n");
		return CVI_BIN_SIZE_ERR;
	}

	#ifdef RPC_MULTI_PROCESS
	if (!isMaster()) {
		for (CVI_U32 idx = CVI_BIN_ID_MIN; idx < CVI_BIN_ID_MAX; idx++) {
			ret = rpc_client_bin_loadparamfrombin(idx, pu8Buffer);
		}
		return ret;
	}
	#endif

	CVI_BIN_HEADER *pstHeader = (CVI_BIN_HEADER *)pu8Buffer;
	CVI_U8 *pu8BlockAddr = (CVI_U8 *)pu8Buffer;
	CVI_BOOL bBlkSizeInvalid = CVI_FALSE;
	CVI_U8 *pu8BlockTempAddr = (CVI_U8 *)pu8Buffer;

	ret = check_bin_file_validity(pu8Buffer, u32DataLength);
	if (ret == (CVI_S32)CVI_BIN_FILE_ERROR) {
		return ret;
	}

	ret = CVI_FAILURE;
	set_loadbin_state(CVI_FALSE);

	for (CVI_U32 idx = CVI_BIN_ID_MIN; idx < CVI_BIN_ID_MAX; idx++) {
		if (idx <= CVI_BIN_ID_ISP7) {
			pu8BlockAddr = pu8Buffer;
		} else {
			pu8BlockAddr = pu8BlockTempAddr;
		}
		pu8BlockTempAddr += pstHeader->size[idx];
		bBlkSizeInvalid = pstHeader->size[idx] == 0 ? CVI_TRUE : CVI_FALSE;

		if ((getParamFromBinFunc[idx] != NULL) && (check_is_register_id(idx) == CVI_SUCCESS)) {
			if (bBlkSizeInvalid) {
				print_import_info(idx, BLOCK_SIZE_ERR);
			} else {
				tmpRet = getParamFromBinFunc[idx](idx, pu8BlockAddr, pstHeader->size[idx]);
				if (tmpRet == CVI_SUCCESS) {
					ret = CVI_SUCCESS;
				} else {
					tmpRet = get_index_para_from_buffer(idx, pu8Buffer, pstHeader->size[idx]);
					if (tmpRet == CVI_SUCCESS) {
						print_import_info(idx, PARSE_SUCCESS_FROM_BIN_INDEX);
						ret = CVI_SUCCESS;
					} else {
						print_import_info(idx, IMPORT_FAIL);
					}
				}
			}
		}
	}

	return ret;
}

CVI_S32 CVI_BIN_SaveParamToBin(FILE *fp, CVI_BIN_EXTRA_S *extraInfo)
{
#ifdef RPC_MULTI_PROCESS
	if (!isMaster()) {
		return rpc_client_bin_saveparamtobin(fp, extraInfo);
	}
#endif

	CVI_S32 ret = CVI_SUCCESS;
	CVI_BIN_HEADER header = { 0 };
	CVI_U32 u32TempLen = 0;
	CVI_U32 u32DataLength = CVI_BIN_GetBinTotalLen();

	CVI_U8 *pu8Buffer = (CVI_U8 *)malloc(sizeof(CVI_U8) * u32DataLength);

	header.chipId = MAGIC_NUMBER;
	header.extraInfo = *extraInfo;
	header.size[CVI_BIN_ID_HEADER] = sizeof(CVI_BIN_HEADER);

	memcpy(pu8Buffer, &header, sizeof(CVI_BIN_HEADER));

	CVI_BIN_ExportBinData(pu8Buffer, u32DataLength);

	u32TempLen = fwrite(pu8Buffer, sizeof(CVI_U8) * u32DataLength, 1, fp);
	if (u32TempLen != 1) {
		ret = CVI_FAILURE;
		CVI_TRACE_SYS(CVI_DBG_WARN, "Write bin data to file fail!\n");
	}

	return ret;
}

CVI_S32 CVI_BIN_LoadParamFromBin(enum CVI_BIN_SECTION_ID id, CVI_U8 *buf)
{
#ifdef RPC_MULTI_PROCESS
	if (!isMaster()) {
		return rpc_client_bin_loadparamfrombin(id, buf);
	}
#endif
	if (buf == NULL) {
		return CVI_BIN_NULL_POINT;
	}

	CVI_S32 ret = CVI_SUCCESS;
	CVI_BOOL bBlkSizeValid = CVI_TRUE;
	CVI_BOOL bReuseBin = CVI_FALSE;
	CVI_U32 src_id = 0;

	if (g_bUseOldLoadAPI) {
		ret = check_bin_file_validity(buf, 0);
		if (ret == (CVI_S32)CVI_BIN_FILE_ERROR) {
			goto ERROR_HANDLER;
		}
	}
	ret = _isSectionIdValid(id);
	if (ret != CVI_SUCCESS) {
		ret = CVI_BIN_ID_ERROR;
		CVI_TRACE_SYS(CVI_DBG_WARN, "Invalid id(%d)\n", id);
		goto ERROR_HANDLER;
	}

	const CVI_CHAR *pchModuleName[CVI_BIN_ID_MAX] = {
					"Header", "Sensor_0", "Sensor_1", "Sensor_2",
					"Sensor_3", "Sensor_4", "Sensor_5", "Sensor_6",
					"Sensor_7", "Vpss", "Vdec", "Venc", "Vo_0", "Vo_1"};
	CVI_BIN_HEADER *pstHeader = (CVI_BIN_HEADER *)buf;
	CVI_U32 secSize = pstHeader->size[CVI_BIN_ID_HEADER] - sizeof(CVI_U32) - sizeof(CVI_BIN_EXTRA_S);
	CVI_U32 secCnt = secSize / sizeof(CVI_U32);
	CVI_U8 *addr = (CVI_U8 *)buf;
	CVI_U8 *temp_addr = (CVI_U8 *)buf;

	if (secCnt < id) {
		ret = CVI_BIN_ID_ERROR;
		CVI_TRACE_SYS(CVI_DBG_WARN, "Module(%d) not contained in this BIN\n", id);
	} else {
		for (CVI_U32 idx = CVI_BIN_ID_MIN; idx < id; idx++) {
			temp_addr += pstHeader->size[idx];
		}
		if (id > CVI_BIN_ID_ISP7) {
			addr = temp_addr;
		}
		ret = check_is_register_id(id);
		if ((getParamFromBinFunc[id] != NULL) && (ret == CVI_SUCCESS)) {
			src_id = get_existent_id_in_bin(id, pstHeader);
			if (id != src_id) {
				reset_id_size_in_bin(src_id, id, buf);
				bReuseBin = CVI_TRUE;
				CVI_TRACE_SYS(CVI_DBG_WARN, "%s use %s bin data!\n", pchModuleName[id],
					pchModuleName[src_id]);
			}
			bBlkSizeValid = pstHeader->size[id] == 0 ? CVI_FALSE : CVI_TRUE;
			if (bBlkSizeValid) {
				ret = getParamFromBinFunc[id](id, addr, pstHeader->size[id]);
				if (ret != CVI_SUCCESS) {
					CVI_TRACE_SYS(CVI_DBG_WARN, "Get cur block(%d) para from index bin!\n", id);
					ret = get_index_para_from_buffer(id, buf, pstHeader->size[id]);
				}
			} else {
				CVI_TRACE_SYS(CVI_DBG_WARN, "The block(%d) data in PQbin file is empty!\n", id);
				ret = CVI_BIN_MODULE_IS_EMPTY_ERROR;
			}
		}
	}
	if (bReuseBin) {
		restore_init_id_size_in_bin(src_id, id, buf);
	}

ERROR_HANDLER:
	return ret;
}

CVI_S32 CVI_BIN_LoadParamFromBinEx(enum CVI_BIN_SECTION_ID id, CVI_U8 *buf, CVI_U32 u32DataLength)
{
	CVI_S32 ret = CVI_SUCCESS;

	if (u32DataLength == 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "input length of buffer can't be 0!\n");
		return CVI_BIN_SIZE_ERR;
	}

	g_bUseOldLoadAPI = CVI_FALSE;
	ret = check_bin_file_validity(buf, u32DataLength);
	if (ret == (CVI_S32)CVI_BIN_FILE_ERROR) {
		goto ERROR_HANDLER;
	}
	ret = CVI_BIN_LoadParamFromBin(id, buf);
	g_bUseOldLoadAPI = CVI_TRUE;

ERROR_HANDLER:
	return ret;
}

CVI_S32 CVI_BIN_GetSingleISPBinLen(enum CVI_BIN_SECTION_ID id)
{
	CVI_BIN_HEADER stHeader = { 0 };
	CVI_BIN_INDEX_HEADER idxHeader = { 0 };
	CVI_U32 u32ParaTotalSize = 0;

	if (check_is_register_id(id) != CVI_SUCCESS) {
		return 0;
	}

	cvi_bin_getIndexInitHeader(&idxHeader);

	for (CVI_U32 idx = CVI_BIN_ID_MIN; idx <= id && idx != CVI_BIN_ID_MAX; idx++) {
		if (is_target_id(idx, id) != CVI_TRUE) {
			continue;
		}
		if ((getBinSizeFunc[idx] != NULL) && (check_is_register_id(idx) == CVI_SUCCESS)) {
			getBinSizeFunc[idx](idx, (CVI_U32 *)&(stHeader.size[idx]));
		}
		u32ParaTotalSize += stHeader.size[idx];
	}

	/*plus size of index_header index_table.*/
	u32ParaTotalSize += idxHeader.size[0];

	/*plus length of Md5.*/
	u32ParaTotalSize += MD5_STRING_LEN;

	return u32ParaTotalSize;
}

CVI_S32 CVI_BIN_ExportSingleISPBinData(enum CVI_BIN_SECTION_ID id, CVI_U8 *pu8Buffer, CVI_U32 u32DataLength)
{
	if (pu8Buffer == NULL) {
		return CVI_BIN_NULL_POINT;
	}
	if (check_is_register_id(id) != CVI_SUCCESS) {
		return CVI_BIN_MODULE_NOT_REGISTER_ERROR;
	}

	CVI_S32 ret = CVI_SUCCESS;
	CVI_BIN_HEADER *pstHeader = (CVI_BIN_HEADER *)pu8Buffer;
	CVI_BIN_INDEX_HEADER idxHeader = { 0 };
	CVI_U8 *pu8BufStartAddr = pu8Buffer;
	CVI_S32 u32CheckBufSize = u32DataLength;

	if (u32CheckBufSize <= 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Data of PQbin file isn't normal!\n");
		return CVI_BIN_DATA_ERR;
	}

	pstHeader->chipId = MAGIC_NUMBER;

	if (cvi_bin_getIndexInitHeader(&idxHeader) != CVI_SUCCESS) {
		return CVI_BIN_INDEX_HEADER_INIT_ERROR;
	}

	for (CVI_U32 idx = CVI_BIN_ID_MIN; idx <= id && idx != CVI_BIN_ID_MAX; idx++) {
		if (is_target_id(idx, id) != CVI_TRUE) {
			continue;
		}
		if ((getBinSizeFunc[idx] != NULL) && (check_is_register_id(idx) == CVI_SUCCESS)) {
			getBinSizeFunc[idx](idx, (CVI_U32 *)&(pstHeader->size[idx]));

			if (idx != 0) {
				idxHeader.size[idx] = pstHeader->size[idx];
			}
		}
		u32CheckBufSize -= pstHeader->size[idx];

		if (u32CheckBufSize <= 0) break;
	}

	if (u32CheckBufSize <= 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "no more buffer space for PQbin!\n");
		return CVI_BIN_SAPCE_ERR;
	}

	if (cvi_bin_computeIndexHeaderOffset(&idxHeader, sizeof(CVI_BIN_HEADER)) != CVI_SUCCESS) {
		return CVI_BIN_INDEX_HEADER_INIT_ERROR;
	}

	/*get bin param to buf*/
	for (CVI_U32 idx = CVI_BIN_ID_MIN; idx <= id && idx != CVI_BIN_ID_MAX; idx++) {
		if (is_target_id(idx, id) != CVI_TRUE) {
			continue;
		}
		if ((setParamToBufFunc[idx] != NULL) && (check_is_register_id(idx) == CVI_SUCCESS)) {
			ret = setParamToBufFunc[idx](idx, pu8Buffer);
		}
		pu8Buffer += pstHeader->size[idx];
	}

	Entry *entrys = (Entry *)malloc(idxHeader.size[0] - sizeof(CVI_BIN_INDEX_HEADER));

	ret = cvi_bin_getIndexTable(entrys);

	/*get bin index to buf*/
	u32CheckBufSize -= idxHeader.size[0];

	if (u32CheckBufSize <= 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "no more buffer space for PQbin!\n");
		return CVI_BIN_SAPCE_ERR;
	}

	memcpy(pu8Buffer, &idxHeader, sizeof(CVI_BIN_INDEX_HEADER));
	pu8Buffer += sizeof(CVI_BIN_INDEX_HEADER);
	memcpy(pu8Buffer, entrys, idxHeader.size[0] - sizeof(CVI_BIN_INDEX_HEADER));

	/*remove md5 length.*/
	u32CheckBufSize -= MD5_STRING_LEN;

	if (u32CheckBufSize < 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "no more buffer space for PQbin!\n");
		return CVI_BIN_SAPCE_ERR;
	}

	write_md5_value_to_buf(pu8BufStartAddr, u32DataLength);

	free(entrys);

	return ret;
}

CVI_S32 CVI_BIN_GetBinExtraAttr(FILE *fp, CVI_BIN_EXTRA_S *extraInfo)
{
	CVI_S32 ret = CVI_SUCCESS;
	CVI_BIN_HEADER header;

	fread(&header, sizeof(CVI_BIN_HEADER), 1, fp);
	memcpy(extraInfo, &(header.extraInfo), sizeof(CVI_BIN_EXTRA_S));

	return ret;
}

CVI_S32 CVI_BIN_SetBinName(WDR_MODE_E wdrMode, const CVI_CHAR *binName)
{
#ifdef RPC_MULTI_PROCESS
	if (!isMaster()) {
		return rpc_client_bin_setbinname(wdrMode, binName);
	}
#endif
	if (binName == NULL) {
		CVI_TRACE_SYS(CVI_DBG_WARN, "binName is NULL\n");
		return CVI_FAILURE;
	}

	return _setBinNameImp(wdrMode, binName);
}

CVI_S32 CVI_BIN_GetBinName(CVI_CHAR *binName)
{
#ifdef RPC_MULTI_PROCESS
	if (!isMaster()) {
		return rpc_client_bin_getbinname(binName);
	}
#endif

	if (binName == NULL) {
		CVI_TRACE_SYS(CVI_DBG_WARN, "binName is NULL\n");
		return CVI_FAILURE;
	}

	return _getBinNameImp(binName);
}

// set the params of ispBinBypass, indicatting which param to be bypassed
CVI_S32 CVI_ISP_BIN_SetBypassParams(enum CVI_BIN_SECTION_ID id, ISP_BIN_BYPASS_U *ispBinBypass)
{
	CVI_S32 ret = CVI_SUCCESS;

	if (ispBinBypass == NULL) {
		return CVI_FAILURE;
	}

	if (id_is_valid_IspID(id) != CVI_SUCCESS) {
		return CVI_BIN_ID_ERROR;
	}

	ret = isp_bin_setBinBypassParams(id - CVI_BIN_ID_ISP0, ispBinBypass);

	return ret;
}

// get the params of ispBinBypass, indicatting which param to be bypassed
CVI_S32 CVI_ISP_BIN_GetBypassParams(enum CVI_BIN_SECTION_ID id, ISP_BIN_BYPASS_U *ispBinBypass)
{
	CVI_S32 ret = CVI_SUCCESS;

	if (ispBinBypass == NULL) {
		return CVI_FAILURE;
	}

	if (id_is_valid_IspID(id) != CVI_SUCCESS) {
		return CVI_BIN_ID_ERROR;
	}

	ret = isp_bin_getBinBypassParams(id - CVI_BIN_ID_ISP0, ispBinBypass);

	return ret;
}