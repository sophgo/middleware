#include "vi_ut.h"
#include <fcntl.h>

#define PREFIX		"/mnt/sd/fpga/"

enum VI_TEST_CASE {
	VI_TEST_RGBGAMMA_ENABLE = 1,
	VI_TEST_RGBGAMMA_ENABLE_TBL_INVERSE,
	VI_TEST_CNR_ENABLE,
	VI_TEST_CNR_SET_LUT,
	VI_TEST_LCAC_ENABLE_MEDIUM_STRENGTH,
	VI_TEST_LCAC_ENABLE_STRONG_STRENGTH,
	VI_TEST_BNR_ENABLE_WEAK_STRENGTH,
	VI_TEST_BNR_ENABLE_STRONG_STRENGTH,
	VI_TEST_YNR_ENABLE,
	VI_TEST_DHZ_ENABLE,
	VI_TEST_BLC_GAIN_800,
	VI_TEST_BLC_GAIN_B00,
	VI_TEST_BLC_OFFSET_1FF,
	VI_TEST_BLC_GAIN_800_OFFSET_1FF,
	VI_TEST_BLC_GAIN_800_OFFSET_1FF_2ND_OFFSET_1FF,
	VI_TEST_WBG_GAIN_800,
	VI_TEST_DPC_ENABLE,
	VI_TEST_DPC_ENABLE_STATIC_ONLY,
	VI_TEST_DPC_ENABLE_DYNAMIC_ONLY,
	VI_TEST_PREEE_ENABLE,
	VI_TEST_EE_ENABLE,
	VI_TEST_CCM_ENABLE,
	VI_TEST_YCURVE_ENABLE_TBL_INVERSE,
	VI_TEST_YGAMMA_ENABLE,
	VI_TEST_YGAMMA_ENABLE_TBL_INVERSE,
	VI_TEST_YGAMMA_LUT_MEM0_CHECK,
	VI_TEST_YGAMMA_LUT_MEM1_CHECK,
	VI_TEST_3DNR_ENABLE_MOTION_MAP_OUT,
	VI_TEST_FUSION_LE_OUTPUT,
	VI_TEST_FUSION_SE_OUTPUT,
	VI_TEST_AE_HIST_ENABLE,
	VI_TEST_HIST_V_ENABLE_LUMA_MODE,
	VI_TEST_HIST_V_ENABLE,
	VI_TEST_HIST_V_ENABLE_OFFX64_OFFY32,
	VI_TEST_DCI_ENABLE,
	VI_TEST_DCI_ENABLE_DEMO_MODE,
	VI_TEST_LTM_CHECK_GLOBAL_TONE,
	VI_TEST_LTM_CHECK_DARK_TONE,
	VI_TEST_LTM_CHECK_BRIGHT_TONE,
	VI_TEST_LTM_CHECK_ALL_TONE_EE_ENABLE,
	VI_TEST_GMS_ENABLE,
	VI_TEST_AF_ENABLE,
	VI_TEST_LSC_ENABLE,
	VI_TEST_LDCI_ENABLE,
	VI_TEST_LDCI_ENABLE_TONE_CURVE_LUT_P_1023,
	VI_TEST_RGBCAC_ON_DEFAULT,
	VI_TEST_RGBCAC_ON_STRENGTH_MAX,
	VI_TEST_HIST_V_DISABLE,
	VI_TEST_HIST_V_ENABLE_ALL_FF_WHITE,
	VI_TEST_HIST_V_ENABLE_ALL_FF_BLACK,
	VI_TEST_RGBGAMMA_ENABLE_HW_AUTO_ENABLE,
	VI_TEST_RGB_DITHER_OFF,
	VI_TEST_RGB_DITHER_ON,
	VI_TEST_YUV_DITHER_OFF,
	VI_TEST_YUV_DITHER_ON,
	VI_TEST_CLUT_ON,
	VI_TEST_DCI_ENABLE_DEMO_MODE_ALL_FF_WHITE,
	VI_TEST_DCI_ENABLE_DEMO_MODE_ALL_FF_BLACK,
	VI_TEST_CACP_OFF,
	VI_TEST_CA_ON_MODE_0,
	VI_TEST_CP_ON_MODE_1,
	VI_TEST_CP_LITE_OFF,
	VI_TEST_CP_LITE_ON,
	VI_TEST_CNR_OFF,
	VI_TEST_CNR_HW_AUTO_ENABLE_ISO_2,
	VI_TEST_YNR_OFF,
	VI_TEST_YNR_HW_AUTO_ON_ISO_0,
	VI_TEST_YNR_HW_AUTO_ON_ISO_2,
	VI_TEST_CROP_YUV,
	VI_TEST_CROP_RAW,
	VI_TEST_PREEE_EE_DISABLE,
};

typedef struct VI_CASE {
	uint8_t	case_no;
	char	name[32];
	char	function[128];
} VI_CASE_T;

static VI_CASE_T vi_case[] = {
	{0, "NORMAL", "NORMAL"},
	{VI_TEST_RGBGAMMA_ENABLE, "RGBGAMMA", "rgbgamma enable"},
	{VI_TEST_RGBGAMMA_ENABLE_TBL_INVERSE, "RGBGAMMA", "RGBgamma enable, tbl inverse"},
	{VI_TEST_CNR_ENABLE, "CNR", "CNR enable"},
	{VI_TEST_CNR_SET_LUT, "CNR", "CNR set lut"},
	{VI_TEST_LCAC_ENABLE_MEDIUM_STRENGTH, "LCAC", "LCAC enable, default medium strength setting"},
	{VI_TEST_LCAC_ENABLE_STRONG_STRENGTH, "LCAC", "LCAC enable, strong strength setting\n"},
	{VI_TEST_BNR_ENABLE_WEAK_STRENGTH, "BNR", "BNR enable, weak strength\n"},
	{VI_TEST_BNR_ENABLE_STRONG_STRENGTH, "BNR", "BNR enable, strong strength\n"},
	{VI_TEST_YNR_ENABLE, "YNR", "YNR enable\n"},
	{VI_TEST_DHZ_ENABLE, "DHZ", "DHZ enable\n"},
	{VI_TEST_BLC_GAIN_800, "BLC", "BLC be_le enable, gain 0x800\n"},
	{VI_TEST_BLC_GAIN_B00, "BLC", "BLC be_le enable, gain 0xB00\n"},
	{VI_TEST_BLC_OFFSET_1FF, "BLC", "BLC be_le enable, offset 0x1ff\n"},
	{VI_TEST_BLC_GAIN_800_OFFSET_1FF, "BLC", "BLC be_le enable, gain 0x800 offset 0x1ff\n"},
	{VI_TEST_BLC_GAIN_800_OFFSET_1FF_2ND_OFFSET_1FF, "BLC",
				"BLC be_le enable, gain 0x800 offset 0x1ff 2nd offset 0x1ff\n"},
	{VI_TEST_WBG_GAIN_800, "WBG", "WBG rawtop_le enable, gain 0x800\n"},
	{VI_TEST_DPC_ENABLE, "DPC", "DPC enable\n"},
	{VI_TEST_DPC_ENABLE_STATIC_ONLY, "DPC", "DPC enable static only\n"},
	{VI_TEST_DPC_ENABLE_DYNAMIC_ONLY, "DPC", "DPC enable dynamic only\n"},
	{VI_TEST_PREEE_ENABLE, "PREEE_EE", "PREEE enable\n"},
	{VI_TEST_EE_ENABLE, "PREEE_EE", "EE enable\n"},
	{VI_TEST_PREEE_EE_DISABLE, "PREEE_EE", "PREEE EE disable\n"},
	{VI_TEST_CCM_ENABLE, "CCM", "CCM enable\n"},
	{VI_TEST_YCURVE_ENABLE_TBL_INVERSE, "YCURVE", "YCURVE enable, tbl inverse\n"},
	{VI_TEST_YGAMMA_ENABLE, "Ygamma", "Ygamma enable\n"},
	{VI_TEST_YGAMMA_ENABLE_TBL_INVERSE, "Ygamma", "Ygamma enable, tbl inverse\n"},
	{VI_TEST_YGAMMA_LUT_MEM0_CHECK, "Ygamma", "Ygamma LUT MEM0 Check\n"},
	{VI_TEST_YGAMMA_LUT_MEM1_CHECK, "Ygamma", "Ygamma LUT MEM1 Check\n"},
	{VI_TEST_3DNR_ENABLE_MOTION_MAP_OUT, "TNR", "TNR enable, motion map output\n"},
	{VI_TEST_FUSION_LE_OUTPUT, "FUSION", "ltm disable, fusion output long\n"},
	{VI_TEST_FUSION_SE_OUTPUT, "FUSION", "ltm disable, fusion output short\n"},
	{VI_TEST_AE_HIST_ENABLE, "AE_HIST", "AE_HIST enable\n"},
	{VI_TEST_HIST_V_ENABLE_LUMA_MODE, "HIST_V", "HIST_V enable, luma mode\n"},
	{VI_TEST_HIST_V_ENABLE, "HIST_V", "HIST_V enable\n"},
	{VI_TEST_HIST_V_ENABLE_OFFX64_OFFY32, "HIST_V", "HIST_V enable, Offx64_Offy32\n"},
	{VI_TEST_DCI_ENABLE, "DCI", "DCI enable\n"},
	{VI_TEST_DCI_ENABLE_DEMO_MODE, "DCI", "DCI enable, demo_mode\n"},
	{VI_TEST_LTM_CHECK_GLOBAL_TONE, "LTM", "ltm enable, dark_tone/bright_tone/ee_en disable\n"},
	{VI_TEST_LTM_CHECK_DARK_TONE, "LTM", "ltm enable, bright_tone/ee_en disable\n"},
	{VI_TEST_LTM_CHECK_BRIGHT_TONE, "LTM", "ltm enable, bright_tone/ee_en disable\n"},
	{VI_TEST_LTM_CHECK_ALL_TONE_EE_ENABLE, "LTM", "ltm enable, all enable\n"},
	{VI_TEST_GMS_ENABLE, "GMS", "gms enable\n"},
	{VI_TEST_AF_ENABLE, "AF", "af enable\n"},
	{VI_TEST_LSC_ENABLE, "LSC", "lsc enable\n"},
	{VI_TEST_LDCI_ENABLE, "LDCI", "ldci enable\n"},
	{VI_TEST_LDCI_ENABLE_TONE_CURVE_LUT_P_1023, "LDCI", "ldci enable, TONE_CURVE_LUT_P_1023\n"},
	{VI_TEST_RGBCAC_ON_DEFAULT, "RGBCAC", "rgbcac enable, default"},
	{VI_TEST_RGBCAC_ON_STRENGTH_MAX, "RGBCAC", "rgbcac enable, PURPLE_TH_0xFF_STRENGTH_0xFF"},
	{VI_TEST_HIST_V_DISABLE, "HIST_V", "HIST_V disable\n"},
	{VI_TEST_HIST_V_ENABLE_ALL_FF_WHITE, "HIST_V", "HIST_V enable, all_ff_white\n"},
	{VI_TEST_HIST_V_ENABLE_ALL_FF_BLACK, "HIST_V", "HIST_V enable, all_ff_black\n"},
	{VI_TEST_RGBGAMMA_ENABLE_HW_AUTO_ENABLE, "RGBGAMMA", "RGBgamma enable, hw auto enable\n"},
	{VI_TEST_RGB_DITHER_OFF, "RGBDITHER_YUVDITHER", "RGB DITHER disable\n"},
	{VI_TEST_RGB_DITHER_ON, "RGBDITHER_YUVDITHER", "RGB DITHER enable\n"},
	{VI_TEST_YUV_DITHER_OFF, "RGBDITHER_YUVDITHER", "YUV DITHER disable\n"},
	{VI_TEST_YUV_DITHER_ON, "RGBDITHER_YUVDITHER", "YUV DITHER enable\n"},
	{VI_TEST_CLUT_ON,  "CLUT", "CLUT enable\n"},
	{VI_TEST_DCI_ENABLE_DEMO_MODE_ALL_FF_WHITE, "DCI", "DCI enable, demo_mode all_ff_white\n"},
	{VI_TEST_DCI_ENABLE_DEMO_MODE_ALL_FF_BLACK, "DCI", "DCI enable, demo_mode all_ff_black\n"},
	{VI_TEST_CACP_OFF, "CACP_related", "CACP_OFF\n"},
	{VI_TEST_CA_ON_MODE_0, "CACP_related", "CA ON MODE 0\n"},
	{VI_TEST_CP_ON_MODE_1, "CACP_related", "CP ON MODE 1\n"},
	{VI_TEST_CP_LITE_OFF, "CACP_related", "CP LITE OFF\n"},
	{VI_TEST_CP_LITE_ON, "CACP_related", "CP LITE ON\n"},
	{VI_TEST_CNR_OFF, "CNR", "CNR ALL OFF"},
	{VI_TEST_CNR_HW_AUTO_ENABLE_ISO_2, "CNR", "CNR hw auto enable, iso 2"},
	{VI_TEST_YNR_OFF, "YNR", "YNR OFF\n"},
	{VI_TEST_YNR_HW_AUTO_ON_ISO_0, "YNR", "YNR ON hw auto iso 0\n"},
	{VI_TEST_YNR_HW_AUTO_ON_ISO_2, "YNR", "YNR ON hw auto iso 2\n"},
	{VI_TEST_CROP_YUV, "CROP", "crop yuv\n"},
	{VI_TEST_CROP_RAW, "CROP", "crop raw\n"}
};

CVI_S32 check_case_dir(char *path)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	pid_t status;
	char cmd[128] = {0};

	if (access(path, F_OK) != 0) {
		sprintf(cmd, "mkdir -p %s", path);
		VI_UT_PRT("%s\n", cmd);
		status = system(cmd);

		if (status == -1) {
			VI_UT_PRT("system call error\n");
			s32Ret = CVI_FAILURE;
		} else {
			if (WIFEXITED(status)) {
				if (WEXITSTATUS(status) == 0) {
					VI_UT_PRT("run shell script successfully.\n");
				} else {
					VI_UT_PRT("run shell script fail, script exit code: %d\n",
							WEXITSTATUS(status));
					s32Ret = CVI_FAILURE;
				}
			} else {
				VI_UT_PRT("exit status = [%d]\n", WEXITSTATUS(status));
			}
		}

	}

	return s32Ret;
}

CVI_S32 get_vi_ip_testcase(void)
{
	FILE *fp = NULL;
	char buf[16] = {0};
	int size = 0, sizeRead = 0;
	char node[128] = {0};

	sprintf(node, "/sys/module/%s/parameters/vi_ip_test_case", CHIP_TYPE);

	fp = fopen(node, "r");
	if (fp == CVI_NULL) {
		VI_UT_PRT("open %s file error\n", node);
		return CVI_FAILURE;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	sizeRead = fread(buf, 1, size, fp);
	if (sizeRead != size) {
		VI_UT_PRT("read 0x%x, expected 0x%x\n", sizeRead, size);
	}
	fclose(fp);

	VI_UT_PRT("vi_ip_test_case = %s\n", buf);

	return (CVI_S32)atoi(buf);
}

CVI_S32 file_to_case_dir(char *filename)
{
	CVI_U8 i = 0;
	pid_t status;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 case_no = get_vi_ip_testcase();
	char path[64] = {0};
	char cmd[128] = {0};

	for (i = 0; i < ARRAY_SIZE(vi_case); i++) {
		if (case_no == vi_case[i].case_no)
			break;
	}

	if (i == ARRAY_SIZE(vi_case)) {
		VI_UT_PRT("no case in caseMap, please add!!!\n");
		return CVI_FAILURE;
	}

	sprintf(path, PREFIX"%s", vi_case[i].name);
	s32Ret = check_case_dir(path);
	if (s32Ret < 0) {
		return CVI_FAILURE;
	}

	sprintf(cmd, "mv %s %s", filename, path);
	VI_UT_PRT("%s\n", cmd);

	status = system(cmd);

	if (status == -1) {
		VI_UT_PRT("system call error\n");
		s32Ret = CVI_FAILURE;
	} else {
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 0) {
				VI_UT_PRT("run shell script successfully.\n");
			} else {
				VI_UT_PRT("run shell script fail, script exit code: %d\n",
						WEXITSTATUS(status));
				s32Ret = CVI_FAILURE;
			}
		} else {
			VI_UT_PRT("exit status = [%d]\n", WEXITSTATUS(status));
		}
	}

	return s32Ret;
}

CVI_S32 vi_fpga_dosomething(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 case_no = get_vi_ip_testcase();
	CVI_CHAR img_name[128] = {0};
	FILE *output;
	CVI_VOID *vir_addr;
	struct cvi_isp_sts_mem mem_info[2];
	struct cvi_vip_memblock mem_block;
	CVI_BOOL need_save_to_file = true;
	int fd = -1;

	fd = CVI_VI_GetPipeFd(0);

	mem_info[0].raw_num = 0;

	G_CTRL_PTR(fd, mem_info, VI_IOCTL_STS_MEM);

	switch (case_no) {
	case VI_TEST_HIST_V_ENABLE_LUMA_MODE: //HIST_V enable, luma mode
	{
		snprintf(img_name, sizeof(img_name), "%d_case_luma.bin", case_no);
		mem_block = mem_info[0].hist_edge_v;
		break;
	}
	case VI_TEST_HIST_V_ENABLE: //HIST_V enable
	{
		snprintf(img_name, sizeof(img_name), "%d_case_0.bin", case_no);
		mem_block = mem_info[0].hist_edge_v;
		break;
	}
	case VI_TEST_HIST_V_ENABLE_OFFX64_OFFY32: //HIST_V enable, Offx64_Offy32
	{
		snprintf(img_name, sizeof(img_name), "%d_case_offx64_offy32.bin", case_no);
		mem_block = mem_info[0].hist_edge_v;
		break;
	}
	case VI_TEST_HIST_V_DISABLE: //HIST_V disable
	{
		snprintf(img_name, sizeof(img_name), "%d_case_disable.bin", case_no);
		mem_block = mem_info[0].hist_edge_v;
		break;
	}
	case VI_TEST_HIST_V_ENABLE_ALL_FF_WHITE: //HIST_V enable all ff white
	{
		snprintf(img_name, sizeof(img_name), "%d_case_all_ff_white.bin", case_no);
		mem_block = mem_info[0].hist_edge_v;
		break;
	}
	case VI_TEST_HIST_V_ENABLE_ALL_FF_BLACK: //HIST_V enable all ff black
	{
		snprintf(img_name, sizeof(img_name), "%d_case_all_ff_black.bin", case_no);
		mem_block = mem_info[0].hist_edge_v;
		break;
	}
	case VI_TEST_GMS_ENABLE: //gms dump
	{
		snprintf(img_name, sizeof(img_name), "case_%d_gms_dump.bin", case_no);
		mem_block = mem_info[0].gms;
		break;
	}
	case VI_TEST_AF_ENABLE: //af dump
	{
		snprintf(img_name, sizeof(img_name), "case_%d_af_dump.bin", case_no);
		mem_block = mem_info[0].af;
		break;
	}
	case VI_TEST_LSC_ENABLE:
	{
		if (strlen(vi_ut_ctx.binpath) == 0) {
			VI_UT_PRT("bin file not exsit\n");
			break;
		}

		FILE *input;

		need_save_to_file = false;

		mem_block.raw_num = 0;

		G_CTRL_PTR(fd, &mem_block, VI_IOCTL_GET_LSC_PHY_BUF);

		VI_UT_PRT("LSC addr[0x%llx] size[0x%x]\n",
				mem_block.phy_addr, mem_block.size);
		vir_addr = CVI_SYS_Mmap(mem_block.phy_addr, mem_block.size);

		input = fopen(vi_ut_ctx.binpath, "r");
		if (input == NULL) {
			VI_UT_PRT("FAIL to OPEN %s\n", img_name);
			s32Ret = CVI_FAILURE;
			break;
		}

		fread(vir_addr, mem_block.size, 1, input);
		fclose(input);

		CVI_SYS_Munmap(vir_addr, mem_block.size);
		usleep(2 * 1000 * 1000);
		break;
	}
	case VI_TEST_LDCI_ENABLE:
	{
		//TODO
		break;
	}
	case VI_TEST_DCI_ENABLE:
	{
		snprintf(img_name, sizeof(img_name), "%d_dci_dump.bin", case_no);
		mem_block = mem_info[0].dci;
		break;
	}
	case VI_TEST_DCI_ENABLE_DEMO_MODE_ALL_FF_WHITE:
	{
		snprintf(img_name, sizeof(img_name), "%d_dci_dump_all_white.bin", case_no);
		mem_block = mem_info[0].dci;
		break;
	}
	case VI_TEST_DCI_ENABLE_DEMO_MODE_ALL_FF_BLACK:
	{
		snprintf(img_name, sizeof(img_name), "%d_dci_dump_all_black.bin", case_no);
		mem_block = mem_info[0].dci;
		break;
	}
	case VI_TEST_AE_HIST_ENABLE:
	{
		CVI_U64 phy_addr;
		CVI_U32 size;
		int i = 0;

		need_save_to_file = false;

		for (i = 0; i < 2; i++) {
			phy_addr = (i == 0) ? mem_info[0].ae_le.phy_addr : mem_info[0].ae_se.phy_addr;
			size = (i == 0) ? mem_info[0].ae_le.size : mem_info[0].ae_se.size;
			if (phy_addr == 0)
				continue;
			vir_addr = CVI_SYS_Mmap(phy_addr, size);

			VI_UT_PRT("AE addr[0x%#"PRIx64"] size[0x%x]\n", phy_addr, size);
			if (i == 0)
				snprintf(img_name, sizeof(img_name), "%d_le_total.bin", case_no);
			else
				snprintf(img_name, sizeof(img_name), "%d_se_total.bin", case_no);

			output = fopen(img_name, "wb");
			if (output == NULL) {
				VI_UT_PRT("FAIL to OPEN %s\n", img_name);
				s32Ret = CVI_FAILURE;
				CVI_SYS_Munmap(vir_addr, size);
				break;
			}

			fwrite(vir_addr, size, 1, output);

			fflush(output);
			fclose(output);

			CVI_SYS_Munmap(vir_addr, size);

			file_to_case_dir(img_name);
		}
		break;
	}
	case VI_TEST_CROP_YUV:
	{
		vi_ut_ctx.is_crop_yuv = 1;
		need_save_to_file = false;
		break;
	}
	case VI_TEST_CROP_RAW:
	{
		vi_ut_ctx.is_crop_raw = 1;
		need_save_to_file = false;
		break;
	}
	default:
		break;
	}

	if (need_save_to_file) {

		VI_UT_PRT("addr[0x%llx] size[0x%x]\n", mem_block.phy_addr, mem_block.size);

		output = fopen(img_name, "wb");
		if (output == NULL) {
			VI_UT_PRT("FAIL to OPEN %s\n", img_name);
			return CVI_FAILURE;
		}

		vir_addr = CVI_SYS_Mmap(mem_block.phy_addr, mem_block.size);

		memset(vir_addr, 0, mem_block.size);

		usleep(5 * 1000 * 1000);

		if (case_no == VI_TEST_GMS_ENABLE)
			fwrite(vir_addr, 0xC00, 1, output);
		else
			fwrite(vir_addr, mem_block.size, 1, output);

		fflush(output);
		fclose(output);

		CVI_SYS_Munmap(vir_addr, mem_block.size);

		file_to_case_dir(img_name);
	}

	return s32Ret;
}

CVI_S32 vi_fpga_dump_yuv(CVI_U8 chn)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVideoFrame;

	memset(&stVideoFrame, 0, sizeof(stVideoFrame));

	if (vi_ut_ctx.is_crop_yuv) {
		VI_CROP_INFO_S pstCropInfo;

		pstCropInfo.stCropRect.s32X = 0;
		pstCropInfo.stCropRect.s32Y = 0;
		pstCropInfo.stCropRect.u32Width = 960;
		pstCropInfo.stCropRect.u32Height = 540;

		pstCropInfo.bEnable = CVI_TRUE;
		CVI_VI_SetChnCrop(0, 0, &pstCropInfo);

		VI_UT_PRT("set chn crop\n");
		usleep(1 * 1000 * 1000);
	}

	if (CVI_VI_GetChnFrame(0, chn, &stVideoFrame, 3000) == 0) {
		CVI_CHAR img_name[128] = {0, };
		CVI_S32 case_no = get_vi_ip_testcase();

		snprintf(img_name, sizeof(img_name), "testcase_%d_chn_%d.yuv", case_no, chn);

		s32Ret = vi_ut_save_frame2file(img_name, &stVideoFrame);

		if (CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame) != 0)
			VI_UT_PRT("CVI_VI_ReleaseChnFrame NG\n");

		if (file_to_case_dir(img_name) == CVI_FAILURE)
			return s32Ret;

		return s32Ret;
	}

	VI_UT_PRT("CVI_VI_GetChnFrame NG\n");
	s32Ret = CVI_FAILURE;
	return s32Ret;
}

CVI_S32 vi_fpga_dump_raw(CVI_U8 pipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 case_no = get_vi_ip_testcase();
	VIDEO_FRAME_INFO_S stVideoFrame[2];
	VI_DUMP_ATTR_S attr;
	VI_PIPE_ATTR_S pipe_attr;

	int frm_num = 1, j = 0;
	CVI_U32 dev = pipe;

	memset(stVideoFrame, 0, sizeof(stVideoFrame));

	stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stVideoFrame[1].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

	// Config pipe_attr.enCompressMode
	CVI_VI_GetPipeAttr(0, &pipe_attr);
	pipe_attr.enCompressMode = vi_ut_ctx.enCompressMode;
	CVI_VI_SetPipeAttr(0, &pipe_attr);

	attr.bEnable = 1;
	attr.u32Depth = 0;
	attr.enDumpType = VI_DUMP_TYPE_RAW;
	CVI_VI_SetPipeDumpAttr(dev, &attr);

	VI_UT_PRT("Enable(%d), DumpType(%d):\n", attr.bEnable, attr.enDumpType);

	if (vi_ut_ctx.is_crop_raw) {
		s32Ret = vi_ut_set_rawdump_crop(dev, stVideoFrame);
		if (s32Ret == CVI_FAILURE)
			return s32Ret;

		VI_UT_PRT("set pipe crop\n");
		usleep(1 * 1000 * 1000);
	}

	if (CVI_VI_GetPipeFrame(dev, stVideoFrame, 3000) == CVI_SUCCESS) {
		if (stVideoFrame[1].stVFrame.u64PhyAddr[0] != 0)
			frm_num = 2;

		for (j = 0; j < frm_num; j++) {
			size_t image_size = stVideoFrame[j].stVFrame.u32Length[0];
			unsigned char *ptr = calloc(1, image_size);
			FILE *output;
			char img_name[128] = {0,}, order_id[8] = {0,};

			if (attr.enDumpType == VI_DUMP_TYPE_RAW) {
				stVideoFrame[j].stVFrame.pu8VirAddr[0]
					= CVI_SYS_Mmap(stVideoFrame[j].stVFrame.u64PhyAddr[0]
						, stVideoFrame[j].stVFrame.u32Length[0]);
				VI_UT_PRT("paddr(%#"PRIx64") vaddr(%p)\n",
							stVideoFrame[j].stVFrame.u64PhyAddr[0],
							stVideoFrame[j].stVFrame.pu8VirAddr[0]);

				memcpy(ptr, (const void *)stVideoFrame[j].stVFrame.pu8VirAddr[0],
					stVideoFrame[j].stVFrame.u32Length[0]);
				CVI_SYS_Munmap((void *)stVideoFrame[j].stVFrame.pu8VirAddr[0],
						stVideoFrame[j].stVFrame.u32Length[0]);

				switch (stVideoFrame[j].stVFrame.enBayerFormat) {
				default:
				case BAYER_FORMAT_BG:
					snprintf(order_id, sizeof(order_id), "BG");
					break;
				case BAYER_FORMAT_GB:
					snprintf(order_id, sizeof(order_id), "GB");
					break;
				case BAYER_FORMAT_GR:
					snprintf(order_id, sizeof(order_id), "GR");
					break;
				case BAYER_FORMAT_RG:
					snprintf(order_id, sizeof(order_id), "RG");
					break;
				}

				snprintf(img_name, sizeof(img_name),
						"./testcase_%d_vi_%d_%s_%s_w_%d_h_%d_x_%d_y_%d.raw",
						case_no,
						dev, (j == 0) ? "LE" : "SE", order_id,
						stVideoFrame[j].stVFrame.u32Width,
						stVideoFrame[j].stVFrame.u32Height,
						stVideoFrame[j].stVFrame.s16OffsetLeft,
						stVideoFrame[j].stVFrame.s16OffsetTop);

				VI_UT_PRT("dump image %s\n", img_name);

				output = fopen(img_name, "wb");

				fwrite(ptr, image_size, 1, output);
				fclose(output);
				free(ptr);

				file_to_case_dir(img_name);
			}
		}
		CVI_VI_ReleasePipeFrame(dev, stVideoFrame);
		return s32Ret;
	}

	VI_UT_PRT("CVI_VI_GetPipeFrame NG\n");
	s32Ret = CVI_FAILURE;
	return s32Ret;
}

static void printf_usage(void)
{
	printf("usage: vi_ut -c 4 -t -p /mnt/sd/test.raw");
	printf("\t-c [case_no]; -t test mode must\n");
	printf("\t-p [filepath_le] for raw_replay case");
	printf("\t-q [filepath_se] for raw_replay case");
	printf("\t-l [binpath] which bin you will load\n");
	printf("\t-d dpcm; -h hdr mode;\n");
	printf("\t-r dump raw; -y dump yuv;\n");
	printf("\t-u dosomething which define by userself;\n");
}

CVI_S32 vi_fpga_parse_arg(int argc, char *argv[])
{
	int ret = -1;

	while ((ret = getopt(argc, argv, "c:hudtryp:q:l:")) != -1) {
		switch (ret) {
		case 'c':
			vi_ut_ctx.u32Case = atoi(optarg);
			break;
		case 'h':
			vi_ut_ctx.is_hdr_on = 1;
			break;
		case 'u':
			vi_ut_ctx.is_custom_flow = 1;
			break;
		case 'd':
			vi_ut_ctx.is_dpcm_on = 1;
			break;
		case 't':
			vi_ut_ctx.is_test_mode = 1;
			break;
		case 'r':
			vi_ut_ctx.is_dump_raw = 1;
			break;
		case 'y':
			vi_ut_ctx.is_dump_yuv = 1;
			break;
		case 'p':
			memcpy(vi_ut_ctx.filepath_le, optarg, strlen(optarg));
			break;
		case 'q':
			memcpy(vi_ut_ctx.filepath_se, optarg, strlen(optarg));
			break;
		case 'l':
			memcpy(vi_ut_ctx.binpath, optarg, strlen(optarg));
			break;
		default:
			printf_usage();
			return -1;
		}
	}

	VI_UT_PRT("\ncase[%d], is_test_mode[%d] is_custom_flow[%d]\nrawreplay_file le[%s] se[%s] binpath[%s]\n",
			vi_ut_ctx.u32Case, vi_ut_ctx.is_test_mode, vi_ut_ctx.is_custom_flow,
			vi_ut_ctx.filepath_le, vi_ut_ctx.filepath_se, vi_ut_ctx.binpath);
	VI_UT_PRT("is_hdr_on %d, is_dpcm_on %d, is_dump_raw %d, is_dump_yuv %d\n",
			vi_ut_ctx.is_hdr_on, vi_ut_ctx.is_dpcm_on,
			vi_ut_ctx.is_dump_raw, vi_ut_ctx.is_dump_yuv);
	return 0;
}
