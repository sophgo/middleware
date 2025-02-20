#include <cvi_type.h>
#include "cvi_bin.h"
#include "cvi_base.h"
#include "vpss_bin.h"
#include "cvi_vpss.h"
#include <cvi_defines.h>
#include "vpss_ioctl.h"

extern CVI_S32 get_vpss_fd();
static CVI_S32 get_vpss_ctx_proc_amp(VPSS_BIN_DATA *pBinData)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;
	struct vpss_all_proc_amp_cfg cfg = {0};

	s32Ret = vpss_get_all_proc_amp(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "vpss_get_all_proc_amp fail\n");
		return s32Ret;
	}
	for (int i = 0; i < VPSS_MAX_GRP_NUM; ++i)
		memcpy(pBinData[i].proc_amp, cfg.proc_amp[i], sizeof(pBinData[i].proc_amp));

	return CVI_SUCCESS;
}

/**************************************************************************
 *   Bin related APIs.
 **************************************************************************/
CVI_S32 vpss_bin_getbinsize(enum CVI_BIN_SECTION_ID id, CVI_U32 *size)
{
	*size = sizeof(VPSS_BIN_DATA) * VPSS_MAX_GRP_NUM;
	UNUSED(id);

	return CVI_SUCCESS;
}

CVI_S32 vpss_bin_getparamfrombin(enum CVI_BIN_SECTION_ID id, CVI_U8 *addr, CVI_U32 size)
{
	CVI_U32 u32DataSize = 0;
	VPSS_BIN_DATA *pstVpssBinData = get_vpssbindata_addr();

	vpss_bin_getbinsize(id, &u32DataSize);
	memset(pstVpssBinData, 0, u32DataSize);
	if (size > u32DataSize) {
		CVI_TRACE_VPSS(CVI_DBG_WARN, "Bin size(%d) > max size(%d).\n", size, u32DataSize);
		return CVI_FAILURE;
	}
	memcpy(pstVpssBinData, addr, size);
	set_loadbin_state(CVI_TRUE);
	UNUSED(id);

	return CVI_SUCCESS;
}

CVI_S32 vpss_bin_setparamtobuf(enum CVI_BIN_SECTION_ID id, CVI_U8 *buffer)
{
	CVI_S32 ret = CVI_SUCCESS;
	CVI_U32 u32DataSize = 0;
	VPSS_BIN_DATA *pstVpssBinData = get_vpssbindata_addr();
	VPSS_BIN_DATA stVpssCtxProcAmp[VPSS_MAX_GRP_NUM] = {0};

	get_vpss_ctx_proc_amp(stVpssCtxProcAmp);
	vpss_bin_getbinsize(id, &u32DataSize);
	for (int i = 0; i < VPSS_MAX_GRP_NUM; ++i) {
		memcpy(pstVpssBinData[i].proc_amp, stVpssCtxProcAmp[i].proc_amp, sizeof(pstVpssBinData[i].proc_amp));
	}
	memcpy(buffer, pstVpssBinData, u32DataSize);
	UNUSED(id);

	return ret;
}

/**************************************************************************
 *   Index bin related APIs.
 **************************************************************************/
CVI_S32 vpss_getIndexBinParam(CVI_U8 *buf, VPSS_Parameter_Structures *pst, CVI_U32 indexOffset)
{
	CVI_S32 ret = CVI_SUCCESS;

	if (buf == NULL || pst == NULL) {
		return CVI_FAILURE;
	}

	if (indexOffset <= 0) {
		return CVI_FAILURE;
	}

	ret = vpss_getBinParam_autogen(CVI_BIN_ID_VPSS - CVI_BIN_ID_ISP0, buf, pst, indexOffset);

	return ret;
}

CVI_S32 vpss_index_bin_getparamfrombin(CVI_U8 *buffer, enum CVI_BIN_SECTION_ID id,
										CVI_U32 binSize, CVI_U32 indexOffset)
{
	CVI_U32 u32DataSize = 0;
	VPSS_BIN_DATA *pstVpssBinData = get_vpssbindata_addr();
	VPSS_Parameter_Structures *param = (VPSS_Parameter_Structures *)malloc(sizeof(VPSS_Parameter_Structures));

	if (buffer == NULL || indexOffset <= 0) {
		free(param);
		return CVI_FAILURE;
	}

	vpss_getIndexBinParam(buffer, param, indexOffset);

	vpss_bin_getbinsize(id, &u32DataSize);
	if (binSize > u32DataSize) {
		CVI_TRACE_VPSS(CVI_DBG_WARN, "Bin size(%d) > max size(%d).\n", binSize, u32DataSize);
		return CVI_FAILURE;
	}
	memset(pstVpssBinData, 0, u32DataSize);
	memcpy(pstVpssBinData, &param->vpss_bin_data, binSize);
	set_loadbin_state(CVI_TRUE);
	free(param);
	UNUSED(id);

	return CVI_SUCCESS;
}



