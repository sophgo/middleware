#include <cvi_type.h>
#include <cvi_comm_vo.h>
#include "vo_bin.h"
#include "cvi_base.h"
#include "cvi_bin.h"
#include "cvi_vo.h"

/**************************************************************************
 *   Bin related APIs.
 **************************************************************************/

CVI_S32 vo_bin_getbinsize(enum CVI_BIN_SECTION_ID id, CVI_U32 *size)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, size);
	*size = sizeof(VO_BIN_INFO_S);
	UNUSED(id);
	return CVI_SUCCESS;
}

CVI_S32 vo_bin_getparamfrombin(enum CVI_BIN_SECTION_ID id, CVI_U8 *addr, CVI_U32 size)
{
	CVI_U32 data_size;
	VO_BIN_INFO_S info_from_bin;
	CVI_S32 ret = CVI_SUCCESS;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, addr);

	vo_bin_getbinsize(id, &data_size);
	if (size > data_size) {
		CVI_TRACE_VO(CVI_DBG_ERR, "Bin size(%d) > max size(%d).\n", size, data_size);
		return CVI_FAILURE;
	}
	memcpy(&info_from_bin, addr, size);

	//check guard pattern
	if (info_from_bin.guard_magic != get_vo_bin_guardmagic_code()) {
		CVI_TRACE_VO(CVI_DBG_ERR, "readback guardpattern incorrect guard_magic(0x%x)\n",
			     info_from_bin.guard_magic);
	} else {
		CVI_TRACE_VO(CVI_DBG_DEBUG, "get param from bin success\n");
		ret = CVI_VO_SetGammaInfo(&(info_from_bin.gamma_info));
	}

	return ret;
}

CVI_S32 vo_bin_setparamtobuf(enum CVI_BIN_SECTION_ID id, CVI_U8 *buffer)
{
	CVI_U32 data_size = 0;
	VO_BIN_INFO_S *pstVoBinInfo = get_vo_bin_info_addr();

	MOD_CHECK_NULL_PTR(CVI_ID_VO, buffer);

	vo_bin_getbinsize(id, &data_size);
	memcpy(buffer, &pstVoBinInfo[id - CVI_BIN_ID_VO0], data_size);

	return CVI_SUCCESS;
}

/**************************************************************************
 *   Index bin related APIs.
 **************************************************************************/
CVI_S32 vo_getIndexBinParam(enum CVI_BIN_SECTION_ID id, CVI_U8 *buf,
					VO_Parameter_Structures *pst, CVI_U32 indexOffset)
{
	CVI_S32 ret = CVI_SUCCESS;

	if ((id < CVI_BIN_ID_MIN) || (id >= CVI_BIN_ID_MAX)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "Vo id %d value error\n", id);
		return CVI_FAILURE;
	}

	if (buf == NULL || pst == NULL) {
		return CVI_FAILURE;
	}

	if (indexOffset <= 0) {
		return CVI_FAILURE;
	}

	ret = vo_getBinParam_autogen(id - CVI_BIN_ID_ISP0, buf, pst, indexOffset);

	return ret;
}

CVI_S32 vo_index_bin_getparamfrombin(CVI_U8 *buffer, enum CVI_BIN_SECTION_ID id,
										CVI_U32 binSize, CVI_U32 indexOffset)
{
	CVI_U32 data_size;
	CVI_S32 ret = CVI_SUCCESS;

	if ((id < CVI_BIN_ID_MIN) || (id >= CVI_BIN_ID_MAX)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "Vo id %d value error\n", id);
		return CVI_FAILURE;
	}

	if (buffer == NULL || indexOffset <= 0) {
		return CVI_FAILURE;
	}

	vo_bin_getbinsize(id, &data_size);
	if (binSize > data_size) {
		CVI_TRACE_VO(CVI_DBG_ERR, "Bin size(%d) > max size(%d).\n", binSize, data_size);
		return CVI_FAILURE;
	}

	VO_Parameter_Structures *param = (VO_Parameter_Structures *)malloc(sizeof(VO_Parameter_Structures));

	vo_getIndexBinParam(id, buffer, param, indexOffset);

	//check guard pattern
	if (param->vo_bin_data.guard_magic != get_vo_bin_guardmagic_code()) {
		CVI_TRACE_VO(CVI_DBG_ERR, "readback guardpattern incorrect guard_magic(0x%x)\n",
			     param->vo_bin_data.guard_magic);
	} else {
		CVI_TRACE_VO(CVI_DBG_DEBUG, "get param from bin success\n");
		ret = CVI_VO_SetGammaInfo(&(param->vo_bin_data.gamma_info));
	}
	free(param);

	return ret;
}