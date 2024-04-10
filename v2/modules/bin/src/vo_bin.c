#include <linux/cvi_comm_vo.h>
#include "vo_bin.h"
#include "cvi_base.h"
#include "cvi_bin.h"

/**************************************************************************
 *   Bin related APIs.
 **************************************************************************/

CVI_S32 vo_bin_getbinsize(CVI_U32 *size)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, size);
	*size = sizeof(VO_BIN_INFO_S);
	return CVI_SUCCESS;
}

CVI_S32 vo_bin_getparamfrombin(CVI_U8 *addr, CVI_U32 size)
{
	CVI_U32 data_size;
	VO_BIN_INFO_S info_from_bin;
	VO_BIN_INFO_S *pstVoBinInfo = get_vo_bin_info_addr();

	MOD_CHECK_NULL_PTR(CVI_ID_VO, addr);

	vo_bin_getbinsize(&data_size);
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
		memcpy(pstVoBinInfo, &info_from_bin, sizeof(VO_BIN_INFO_S));
	}

	return CVI_SUCCESS;
}

CVI_S32 vo_bin_setparamtobuf(CVI_U8 *buffer)
{
	CVI_U32 data_size = 0;
	VO_BIN_INFO_S *pstVoBinInfo = get_vo_bin_info_addr();

	MOD_CHECK_NULL_PTR(CVI_ID_VO, buffer);

	vo_bin_getbinsize(&data_size);
	memcpy(buffer, pstVoBinInfo, data_size);

	return CVI_SUCCESS;
}

CVI_S32 vo_bin_setparamtobin(FILE *fp)
{
	CVI_U32 data_size = 0;
	VO_BIN_INFO_S *pstVoBinInfo = get_vo_bin_info_addr();

	MOD_CHECK_NULL_PTR(CVI_ID_VO, fp);

	vo_bin_getbinsize(&data_size);
	fwrite(pstVoBinInfo, data_size, 1, fp);

	return CVI_SUCCESS;
}

