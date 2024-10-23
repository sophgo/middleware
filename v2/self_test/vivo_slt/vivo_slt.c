
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <md5sum.h>
#include <inttypes.h>

#include <fcntl.h>		/* low-level i/o */
#include "cvi_buffer.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_comm_isp.h"
#include "cvi_comm_sns.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_isp.h"
#include "cvi_sns_ctrl.h"
#include "sample_comm.h"

static SAMPLE_VI_CONFIG_S g_stViConfig;
static SAMPLE_INI_CFG_S g_stIniCfg;
char md5_golden_str[MD5_DIGEST_LENGTH * 2 + 1];

static void sys_handle_signal(int nSignal, siginfo_t *si, void *arg)
{
	UNUSED(nSignal);
	UNUSED(si);
	UNUSED(arg);

	if (g_stViConfig.s32WorkingViNum != 0) {
		SAMPLE_COMM_VI_DestroyIsp(&g_stViConfig);
		SAMPLE_COMM_VI_DestroyVi(&g_stViConfig);
	}
	SAMPLE_COMM_SYS_Exit();
	exit(1);
}

static int sys_vi_init(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MMF_VERSION_S stVersion;
	SAMPLE_INI_CFG_S stIniCfg;
	SAMPLE_VI_CONFIG_S stViConfig;
	LOG_LEVEL_CONF_S log_conf;

	memset(&stVersion, 0, sizeof(MMF_VERSION_S));
	memset(&stIniCfg, 0, sizeof(SAMPLE_INI_CFG_S));
	memset(&stViConfig, 0, sizeof(SAMPLE_VI_CONFIG_S));
	memset(&log_conf, 0, sizeof(LOG_LEVEL_CONF_S));

	CVI_SYS_GetVersion(&stVersion);
	SAMPLE_PRT("MMF Version:%s\n", stVersion.version);

	log_conf.enModId = CVI_ID_LOG;
	log_conf.s32Level = CVI_DBG_INFO;
	CVI_LOG_SetLevelConf(&log_conf);

	// Get config from ini if found.
	s32Ret = SAMPLE_COMM_VI_ParseIni(&stIniCfg);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Parse fail\n");
	} else {
		SAMPLE_PRT("Parse complete\n");
	}

	//Set sensor number
	CVI_VI_SetDevNum(stIniCfg.devNum);
	/************************************************
	 * step1:  Config VI
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_IniToViCfg(&stIniCfg, &stViConfig);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	for (CVI_S32 i = 0; i < stViConfig.s32WorkingViNum; i++) {
		stViConfig.astViInfo[i].stChnInfo.enPixFormat =
			SAMPLE_COMM_VI_GetYuvBypassSts(stIniCfg.enSnsType[i])
			? PIXEL_FORMAT_YUYV : PIXEL_FORMAT_NV21;
	}

	memcpy(&g_stViConfig, &stViConfig, sizeof(SAMPLE_VI_CONFIG_S));
	memcpy(&g_stIniCfg, &stIniCfg, sizeof(SAMPLE_INI_CFG_S));

	/************************************************
	 * step2:  Get input size
	 ************************************************/
	CVI_U32 u32BlkSize, u32BlkRotSize;
	PIC_SIZE_E enPicSize;
	VB_CONFIG_S stVbConf;
	SIZE_S stSize;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 0;

	for (CVI_S32 i = 0; i < stViConfig.s32WorkingViNum; i++) {
		bool createNewPool = true;

		s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stIniCfg.enSnsType[i], &enPicSize);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VI_GetSizeBySensor failed with %#x\n", s32Ret);
			return s32Ret;
		}

		s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
			return s32Ret;
		}

		u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
					stViConfig.astViInfo[i].stChnInfo.enPixFormat,
					DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkRotSize = COMMON_GetPicBufferSize(stSize.u32Height, stSize.u32Width,
					stViConfig.astViInfo[i].stChnInfo.enPixFormat,
					DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkSize = u32BlkSize > u32BlkRotSize ? u32BlkSize : u32BlkRotSize;

		for (CVI_U32 j = 0; j < stVbConf.u32MaxPoolCnt; j++) {
			if (stVbConf.astCommPool[j].u32BlkSize == u32BlkSize) {
				stVbConf.astCommPool[j].u32BlkCnt += 2;
				createNewPool = false;
				break;
			}
		}
		if (createNewPool) {
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkSize = u32BlkSize;
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkCnt = 4;
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].enRemapMode = VB_REMAP_MODE_CACHED;
			stVbConf.u32MaxPoolCnt++;
		}
	}

	if (stVbConf.u32MaxPoolCnt == 1) {
		stVbConf.astCommPool[0].u32BlkCnt += 2;
	}

	/************************************************
	 * step3:  Init modules
	 ************************************************/
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = sys_handle_signal;
	sa.sa_flags = SA_SIGINFO|SA_RESETHAND; // Reset signal handler to system default after signal triggered
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "system init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_PLAT_VI_INIT(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static void sys_vi_deinit(void)
{
	SAMPLE_COMM_VI_DestroyIsp(&g_stViConfig);

	SAMPLE_COMM_VI_DestroyVi(&g_stViConfig);

	SAMPLE_COMM_SYS_Exit();
}

static CVI_S32 _vi_get_chn_md5(CVI_U32 chn, CVI_U32 index)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	VI_CROP_INFO_S crop_info = {0};
	unsigned char md5_result[MD5_DIGEST_LENGTH];
	char md5_str[MD5_DIGEST_LENGTH * 2 + 1];

	if (CVI_VI_GetChnFrame(0, chn, &stVideoFrame, 3000) == 0) {
		size_t image_size = stVideoFrame.stVFrame.u32Length[0] + stVideoFrame.stVFrame.u32Length[1]
				  + stVideoFrame.stVFrame.u32Length[2];
		CVI_VOID *vir_addr;
		CVI_U32 plane_offset, u32LumaSize, u32ChromaSize;
		u32LumaSize =  stVideoFrame.stVFrame.u32Stride[0] * stVideoFrame.stVFrame.u32Height;
		u32ChromaSize =  stVideoFrame.stVFrame.u32Stride[1] * stVideoFrame.stVFrame.u32Height / 2;
		CVI_VI_GetChnCrop(0, chn, &crop_info);
		if (crop_info.bEnable) {
			u32LumaSize = ALIGN((crop_info.stCropRect.u32Width * 8 + 7) >> 3, DEFAULT_ALIGN) *
				ALIGN(crop_info.stCropRect.u32Height, 2);
			u32ChromaSize = (ALIGN(((crop_info.stCropRect.u32Width >> 1) * 8 + 7) >> 3, DEFAULT_ALIGN) *
				ALIGN(crop_info.stCropRect.u32Height, 2)) >> 1;
		}
		vir_addr = CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], image_size);
		CVI_SYS_IonInvalidateCache(stVideoFrame.stVFrame.u64PhyAddr[0], vir_addr, image_size);
		plane_offset = 0;
		for (int i = 0; i < 3; i++) {
			if (stVideoFrame.stVFrame.u32Length[i] != 0) {
				stVideoFrame.stVFrame.pu8VirAddr[i] = vir_addr + plane_offset;
				plane_offset += stVideoFrame.stVFrame.u32Length[i];
				CVI_TRACE_LOG(CVI_DBG_WARN,
					   "plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) length(%d)\n",
					   i, stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Stride[i],
					   stVideoFrame.stVFrame.u32Length[i]);
				MD5((void *)stVideoFrame.stVFrame.pu8VirAddr[i]
					,	(i == 0) ? u32LumaSize : u32ChromaSize, md5_result);
				for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
					sprintf(&md5_str[i*2], "%02x", (unsigned int)md5_result[i]);
				}
			}
		}
		CVI_SYS_Munmap(vir_addr, image_size);

		if (CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame) != 0)
			SAMPLE_PRT("CVI_VI_ReleaseChnFrame NG\n");

		if (strcasecmp(md5_golden_str, md5_str) == 0) {
			SAMPLE_PRT("chn:%u frame:%u md5str:%s pass!\n", chn, index, md5_str);
			return CVI_SUCCESS;
		} else {
			SAMPLE_PRT("chn:%u frame:%u md5str:%s fail!\n", chn, index, md5_str);
			return CVI_FAILURE;
		}
	} else {
		SAMPLE_PRT("CVI_VI_GetChnFrame NG\n");
		return -2;
	}
}

static CVI_S32 vivo_slt_test(CVI_U32 chn, CVI_U32 loop)
{
	FILE *md5_golden_fd;
	char filename[64];
	CVI_U32 pass = 0, fail = 0, i = 1;
	CVI_S32 ret = 0;

	sprintf(filename, "md5_golden%d.txt", chn);

	md5_golden_fd = fopen(filename, "r");
	if (md5_golden_fd == NULL) {
		SAMPLE_PRT("not find md5 file!\n");
		return CVI_FAILURE;
	}

	fgets(md5_golden_str, sizeof(md5_golden_str), md5_golden_fd);
	fclose(md5_golden_fd);
	SAMPLE_PRT("chn:%u md5_golden_str:%s\n", chn, md5_golden_str);

	while (i <= loop) {
		ret = _vi_get_chn_md5(chn, i);
		if (ret == CVI_SUCCESS) {
			pass++;
		} else if (ret == CVI_FAILURE) {
			fail++;
		} else if (ret == -2) {
			fail++;
			break;
		}
		i++;
	}

	SAMPLE_PRT("chn:%u loop:%u pass:%u fail:%u\n", chn, loop, pass, fail);

	if (pass != loop) {
		return CVI_FAILURE;
	} else {
		return CVI_SUCCESS;
	}
}

int main(int argc, char **argv)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if(argc != 3) {
		SAMPLE_PRT("use:vivo_slt chn loop\n");
		return CVI_FAILURE;
	}

	s32Ret = sys_vi_init();
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	usleep(500 * 1000);

	system("stty erase ^H");

	s32Ret = vivo_slt_test(atoi(argv[1]), atoi(argv[2]));
	SAMPLE_PRT("vivo_slt_test %s\n", s32Ret == CVI_SUCCESS ? "pass" : "fail");

	sys_vi_deinit();

	return s32Ret;
}

