#include "vi_ut.h"
#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <ctype.h>

#define UT_INFO(_case, _func, _flags)			\
	[(_case)] = {					\
		.case_no = _case,			\
		.flags = _flags,			\
		.name = #_case,				\
		.func = _func,				\
	}

enum CASE_UT {
	CASE_BEGIN,
	CASE_VI_IOCTRL,
	CASE_VI_MMAP,
	CASE_VI_POLL,
	CASE_RAW_REPLAY,
	CASE_RAW_REPLAY_MANUAL,
	CASE_SENSOR_ON_THE_FLY,
	CASE_SENSOR_FE_BE_DRAM_POST_DRAM,
	CASE_SENSOR_FE_DRAM_BE_POST_DRAM,
	CASE_PATGEN_ON_THE_FLY,
	CASE_PATGEN_FE_BE_DRAM_POST_DRAM,
	CASE_PATGEN_FE_DRAM_BE_POST_DRAM,
	CASE_PATGEN_FE_DRAM_BE_POST_SC,
	CASE_HDR_PATGEN_FE_BE_DRAM_POST_DRAM,
	CASE_HDR_PATGEN_FE_DRAM_BE_POST_DRAM,
	CASE_DUMP_VI_YUV_FRAME,
	CASE_DUMP_VI_RAW_FRAME,
	CASE_DUMP_VI_SMOOTH_RAW_FRAME,
	CASE_DUMP_VI_REGISTER,
	CASE_SHOW_PROC_VI,
	CASE_SHOW_PROC_VI_DBG,
	CASE_SET_CHN_CROP,
	CASE_SET_CHN_ROTATION,
	CASE_SET_CHN_FLIP_MIRROR,
	CASE_VI_MULTI_PROCESSES_TEST,
	CASE_VI_SDK_TEST,
	CASE_VI_PLD_TEST,
	CASE_VI_SLT_TEST,
	CASE_MAX,
};

struct vi_ut_info {
	unsigned int case_no;
	unsigned int flags;
	const char * const name;
	int (*func)(void *p);
};

SAMPLE_VI_CONFIG_S stViConfig;
VI_UT_CTX vi_ut_ctx;
bool g_is_patgen = false;

static int vi_poll_test(int loop)
{
	int fd = -1;
	fd_set efds;
	struct timeval tv;
	CVI_S32 ret = CVI_SUCCESS;

	if (open_device(VI_DEV_NAME, &fd) == -1) {
		fprintf(stderr, "VI open failed\n");
		ret = CVI_FAILURE;
		goto exit_err;
	}

	while (loop-- > 0) {
		FD_ZERO(&efds);
		FD_SET(fd, &efds);

		tv.tv_sec = 0;
		tv.tv_usec = 500 * 1000;

		ret = select(fd + 1, NULL, NULL, &efds, &tv);
		if (ret == -1) {
			if (errno == EINTR)
				continue;
			fprintf(stderr, "select error\n");
			ret = CVI_FAILURE;
			goto exit_err;
		}

		if (ret == 0) {
			fprintf(stderr, "select timeout\n");
			ret = CVI_FAILURE;
			goto exit_err;
		}

		// check if done
		if (FD_ISSET(fd, &efds)) {
			struct vi_ext_control ec1;
			struct vi_event ev;

			memset(&ec1, 0, sizeof(ec1));
			memset(&ev, 0, sizeof(ev));
			ec1.id = VI_IOCTL_DQEVENT;
			ec1.ptr = (void *)&ev;
			if (ioctl(fd, VI_IOC_G_CTRL, &ec1) < 0) {
				fprintf(stderr, "VI_IOC_G_CTRL - %s NG\n", __func__);
				ret = CVI_FAILURE;
				goto exit_err;
			}

			switch (ev.type) {
			case VI_EVENT_PRE0_SOF:
			case VI_EVENT_PRE1_SOF:
			case VI_EVENT_PRE0_EOF:
			case VI_EVENT_PRE1_EOF:
			case VI_EVENT_POST0_EOF:
			case VI_EVENT_POST1_EOF:
			case VI_EVENT_ISP_PROC_READ:
			{
				VI_UT_PRT("dq event=%d, frm_num=%d\n",
						ev.type, ev.frame_sequence);
				break;
			}
			default:
				break;
			}
		}
	}

	VI_UT_PRT("-\n");

	if (close_device(&fd) == -1) {
		fprintf(stderr, "vi close failed\n");
		ret = CVI_FAILURE;
		goto exit_err;
	}

	ret = CVI_SUCCESS;
exit_err:
	return ret;
}

static CVI_VOID *vi_event_thread(CVI_VOID *data)
{
	CVI_S32 ret = CVI_SUCCESS;

	UNUSED(data);

	prctl(PR_SET_NAME, "vi_event_thread");

	ret = vi_poll_test(300);
	if (ret != CVI_SUCCESS) {
		VI_UT_PRT("poll test fail\n");
	}

	pthread_exit(NULL);
}

static void _isp_thread_test(void)
{
	CVI_S32 s32Ret = 0;
	struct sched_param param;
	pthread_attr_t attr;
	pthread_t thread;

	param.sched_priority = 80;

	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	s32Ret = pthread_create(&thread, &attr, (void *)vi_event_thread, NULL);
	if (s32Ret != 0) {
		VI_UT_PRT("create vi event thread failed!, error: %d, %s\r\n",
					s32Ret, strerror(s32Ret));
	}
	pthread_join(thread, NULL);
}

static int _vi_set_tuning_dis(CVI_U8 pipe, CVI_U8 feCtrl, CVI_U8 beCtrl, CVI_U8 postCtrl)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	pid_t status;
	char cmd[128] = {0};

	sprintf(cmd, "echo %d,%d,%d,%d > /sys/module/%s/parameters/tuning_dis",
			pipe, feCtrl, beCtrl, postCtrl, CHIP_TYPE);
	VI_UT_PRT("%s\n", cmd);
	status = system(cmd);

	if (status == -1) {
		VI_UT_PRT("system call error\n");
		s32Ret =  CVI_FAILURE;
	} else {
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 0) {
				VI_UT_PRT("run shell script successfully.\n");
			} else {
				VI_UT_PRT("run shell script fail, script exit code: %d\n",
						WEXITSTATUS(status));
				s32Ret =  CVI_FAILURE;
			}
		} else {
			VI_UT_PRT("exit status = [%d]\n", WEXITSTATUS(status));
		}
	}

	return s32Ret;
}

static CVI_S32 _vi_ut_proc_mmap(int fd)
{
#if 0
	void *vi_ut_shared_mem = NULL;

	vi_ut_shared_mem = mmap(NULL, VI_SHARE_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (vi_ut_shared_mem == MAP_FAILED) {
		VI_UT_PRT("vi proc mmap fail!\n");
		return CVI_FAILURE;
	}

	if (munmap((void *)vi_ut_shared_mem, VI_SHARE_MEM_SIZE) != 0) {
		VI_UT_PRT("vi proc ummap fail!\n");
		return CVI_FAILURE;
	}
#endif
	fd = fd;
	return CVI_SUCCESS;
}

static CVI_S32 vi_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = vi_ut_plat_sys_init();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("vi_ut_plat_sys_init failed. s32Ret: 0x%x !\n", s32Ret);
		goto error;
	}

	if (vi_ut_ctx.is_vpss_online) {
		s32Ret = vi_ut_sys_config_online_mode();
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("_sys_config_online_mode failed. s32Ret: 0x%x !\n", s32Ret);
			goto error;
		}
	}

	s32Ret = vi_ut_plat_vi_init();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("vi_ut_plat_vi_init failed. s32Ret: 0x%x !\n", s32Ret);
		goto error;
	}

	if (vi_ut_ctx.is_vpss_online) {
		s32Ret = vi_ut_vpss_config_online_mode();
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("_vpss_config_online_mode failed. s32Ret: 0x%x !\n", s32Ret);
			goto error;
		}
	}
error:
	return s32Ret;
}

static int case_vi_ioctrl(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int fd = -1;

	UNUSED(p);

	if (vi_ut_ctx.is_test_mode) {
		s32Ret = vi_ut_plat_sys_init();
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("vi_ut_plat_sys_init failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	fd = CVI_VI_GetPipeFd(0);

	if (fd < 0) {
		VI_UT_PRT("CVI_VI_GetPipeFd failed.\n");
		return CVI_FAILURE;
	}

	s32Ret = vi_ioctl_test(fd);

	return s32Ret;
}

static int case_vi_mmap(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	if (vi_ut_ctx.is_test_mode) {
		s32Ret = vi_ut_plat_sys_init();
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("vi_ut_plat_sys_init failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	int fd = -1;

	fd = CVI_VI_GetPipeFd(0);
	if (fd < 0) {
		VI_UT_PRT("CVI_VI_GetPipeFd failed.\n");
	}

	s32Ret = _vi_ut_proc_mmap(fd);

	return CVI_SUCCESS;
}

static int case_vi_poll(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	if (vi_ut_ctx.is_test_mode) {
		vi_ut_ctx.is_get_dpcm_mode	= 1;
		vi_ut_ctx.is_enable_sensor	= 0;
		vi_ut_ctx.is_be_online		= 0;
		vi_ut_ctx.is_post_online	= 1;
		vi_ut_ctx.is_patgen_enable	= 1;
		vi_ut_ctx.is_hdr_enable		= 0;
		vi_ut_ctx.is_vpss_online	= 0;

		s32Ret = vi_test();
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	s32Ret = vi_poll_test(300);

	return s32Ret;
}

static int case_sensor_onthefly(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	vi_ut_ctx.is_get_dpcm_mode	= 1;
	vi_ut_ctx.is_enable_sensor	= 1;
	vi_ut_ctx.is_be_online		= 1;
	vi_ut_ctx.is_post_online	= 1;
	vi_ut_ctx.is_patgen_enable	= 0;
	vi_ut_ctx.is_hdr_enable		= 0;
	vi_ut_ctx.is_vpss_online	= 0;

	s32Ret = vi_test();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		s32Ret = vi_ut_get_chn_frame(0);
	}

	return CVI_SUCCESS;
}

static int case_raw_replay(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	vi_ut_ctx.is_get_dpcm_mode	= 1;
	vi_ut_ctx.is_enable_sensor	= 0;
	vi_ut_ctx.is_be_online		= 0;
	vi_ut_ctx.is_post_online	= 1;
	vi_ut_ctx.is_patgen_enable	= 0;
	vi_ut_ctx.is_hdr_enable		= 0;
	vi_ut_ctx.is_vpss_online	= 0;

	s32Ret = vi_raw_replay_test();

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		s32Ret = vi_ut_get_chn_frame(0);
	}

	return s32Ret;
}

static int case_raw_replay_manual(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		VI_UT_PRT("This test case is not support under auto test.\n");
		return s32Ret;
	}

	s32Ret = vi_raw_replay_manual_test();

	return s32Ret;
}

static int case_patgen_onthefly(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	vi_ut_ctx.is_get_dpcm_mode	= 1;
	vi_ut_ctx.is_enable_sensor	= 0;
	vi_ut_ctx.is_be_online		= 1;
	vi_ut_ctx.is_post_online	= 1;
	vi_ut_ctx.is_patgen_enable	= 1;
	vi_ut_ctx.is_hdr_enable		= 0;
	vi_ut_ctx.is_vpss_online	= 0;

	s32Ret = vi_test();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		s32Ret = vi_ut_get_chn_frame(0);
	}

	return CVI_SUCCESS;
}

static int case_sensor_fe_be_dram_post_dram(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	vi_ut_ctx.is_get_dpcm_mode	= 1;
	vi_ut_ctx.is_enable_sensor	= 1;
	vi_ut_ctx.is_be_online		= 1;
	vi_ut_ctx.is_post_online	= 0;
	vi_ut_ctx.is_patgen_enable	= 0;
	vi_ut_ctx.is_hdr_enable		= 0;
	vi_ut_ctx.is_vpss_online	= 0;

	s32Ret = vi_test();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		s32Ret = vi_ut_get_chn_frame(0);
	}

	return CVI_SUCCESS;
}

static int case_sensor_fe_dram_be_post_dram(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	vi_ut_ctx.is_get_dpcm_mode	= 1;
	vi_ut_ctx.is_enable_sensor	= 1;
	vi_ut_ctx.is_be_online		= 0;
	vi_ut_ctx.is_post_online	= 1;
	vi_ut_ctx.is_patgen_enable	= 0;
	vi_ut_ctx.is_hdr_enable		= 0;
	vi_ut_ctx.is_vpss_online	= 0;

	s32Ret = vi_test();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		s32Ret = vi_ut_get_chn_frame(0);
	}

	return CVI_SUCCESS;
}

static int case_patgen_fe_be_dram_post_dram(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	vi_ut_ctx.is_get_dpcm_mode	= 1;
	vi_ut_ctx.is_enable_sensor	= 0;
	vi_ut_ctx.is_be_online		= 1;
	vi_ut_ctx.is_post_online	= 0;
	vi_ut_ctx.is_patgen_enable	= 1;
	vi_ut_ctx.is_hdr_enable		= 0;
	vi_ut_ctx.is_vpss_online	= 0;

	s32Ret = vi_test();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		s32Ret = vi_ut_get_chn_frame(0);
	}

	return s32Ret;
}

static int case_patgen_fe_dram_be_post_dram(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	vi_ut_ctx.is_get_dpcm_mode	= 1;
	vi_ut_ctx.is_enable_sensor	= 0;
	vi_ut_ctx.is_be_online		= 0;
	vi_ut_ctx.is_post_online	= 1;
	vi_ut_ctx.is_patgen_enable	= 1;
	vi_ut_ctx.is_hdr_enable		= 0;
	vi_ut_ctx.is_vpss_online	= 0;

	s32Ret = vi_test();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		s32Ret = vi_ut_get_chn_frame(0);
	}

	return s32Ret;
}

static int case_hdr_patgen_fe_be_dram_post_dram(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	vi_ut_ctx.is_get_dpcm_mode	= 1;
	vi_ut_ctx.is_enable_sensor	= 0;
	vi_ut_ctx.is_be_online		= 1;
	vi_ut_ctx.is_post_online	= 0;
	vi_ut_ctx.is_patgen_enable	= 1;
	vi_ut_ctx.is_hdr_enable		= 1;
	vi_ut_ctx.is_vpss_online	= 0;

	s32Ret = vi_test();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		s32Ret = vi_ut_get_chn_frame(0);
	}

	return s32Ret;
}

static int case_hdr_patgen_fe_dram_be_post_dram(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	vi_ut_ctx.is_get_dpcm_mode	= 1;
	vi_ut_ctx.is_enable_sensor	= 0;
	vi_ut_ctx.is_be_online		= 0;
	vi_ut_ctx.is_post_online	= 1;
	vi_ut_ctx.is_patgen_enable	= 1;
	vi_ut_ctx.is_hdr_enable		= 1;
	vi_ut_ctx.is_vpss_online	= 0;

	s32Ret = vi_test();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		s32Ret = vi_ut_get_chn_frame(0);
	}

	return s32Ret;
}

static int case_patgen_fe_dram_be_post_sc(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	vi_ut_ctx.is_get_dpcm_mode	= 1;
	vi_ut_ctx.is_enable_sensor	= 0;
	vi_ut_ctx.is_be_online		= 1;
	vi_ut_ctx.is_post_online	= 0;
	vi_ut_ctx.is_patgen_enable	= 1;
	vi_ut_ctx.is_hdr_enable		= 0;
	vi_ut_ctx.is_vpss_online	= 1;

	s32Ret = vi_test();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
	}

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		s32Ret = vi_ut_get_vpss_chn_frame(0);
	}

	return s32Ret;
}

static int case_dump_vi_yuv_frame(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 loop = 1, tmp;
	CVI_U32 ok = 0, ng = 0;
	CVI_U8  chn = 0;
	struct timespec start, end;

	UNUSED(p);

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		vi_ut_ctx.is_get_dpcm_mode	= 1;
		vi_ut_ctx.is_be_online		= 1;
		vi_ut_ctx.is_post_online	= 0;
		vi_ut_ctx.is_hdr_enable		= 0;
		vi_ut_ctx.is_vpss_online	= 0;
		if (g_is_patgen) {
			vi_ut_ctx.is_enable_sensor	= 0;
			vi_ut_ctx.is_patgen_enable	= 1;
		} else {
			vi_ut_ctx.is_enable_sensor	= 1;
			vi_ut_ctx.is_patgen_enable	= 0;
		}

		s32Ret = vi_test();
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	} else {
		VI_UT_PRT("Get frm from which chn(0~1): ");
		scanf("%d", &tmp);
		chn = tmp;
		VI_UT_PRT("how many loops to do(11111) is infinite: ");
		scanf("%d", &loop);
	}

	while (loop > 0) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		if (vi_ut_get_chn_frame(chn) == CVI_SUCCESS) {
			++ok;
			clock_gettime(CLOCK_MONOTONIC, &end);
			VI_UT_PRT("ms consumed: %f\n", (CVI_FLOAT)vi_ut_diff_in_us(start, end)/1000);
		} else
			++ng;

		if (loop != 11111)
			loop--;
	}
	VI_UT_PRT("VI GetChnFrame OK(%d) NG(%d)\n", ok, ng);

	VI_UT_PRT("Dump VI yuv TEST-PASS\n");

	return s32Ret;
}

static int case_set_chn_crop(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_CROP_INFO_S pstCropInfo;
	VI_CROP_INFO_S pstCropInfo_bak;

	UNUSED(p);

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		vi_ut_ctx.is_get_dpcm_mode	= 1;
		vi_ut_ctx.is_be_online		= 1;
		vi_ut_ctx.is_post_online	= 0;
		vi_ut_ctx.is_hdr_enable		= 0;
		vi_ut_ctx.is_vpss_online	= 0;
		if (g_is_patgen) {
			vi_ut_ctx.is_enable_sensor	= 0;
			vi_ut_ctx.is_patgen_enable	= 1;
		} else {
			vi_ut_ctx.is_enable_sensor	= 1;
			vi_ut_ctx.is_patgen_enable	= 0;
		}

		s32Ret = vi_test();
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}

		pstCropInfo.stCropRect.s32X = 0;
		pstCropInfo.stCropRect.s32Y = 0;
		pstCropInfo.stCropRect.u32Width = 960;
		pstCropInfo.stCropRect.u32Height = 540;
	} else {
		VI_UT_PRT("Set Chn Crop. plz set:\n");
		VI_UT_PRT("input x:\n");
		scanf("%d", &pstCropInfo.stCropRect.s32X);
		VI_UT_PRT("input y:\n");
		scanf("%d", &pstCropInfo.stCropRect.s32Y);
		VI_UT_PRT("input width:\n");
		scanf("%d", &pstCropInfo.stCropRect.u32Width);
		VI_UT_PRT("input height:\n");
		scanf("%d", &pstCropInfo.stCropRect.u32Height);
	}

	pstCropInfo.bEnable = CVI_TRUE;
	s32Ret = CVI_VI_SetChnCrop(0, 0, &pstCropInfo);
	if (s32Ret == CVI_SUCCESS) {
		VI_UT_PRT("CVI_VI_SetChnCrop TEST-PASS\n");
	} else {
		VI_UT_PRT("CVI_VI_SetChnCrop TEST-FAIL\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_VI_GetChnCrop(0, 0, &pstCropInfo_bak);
	if (s32Ret == CVI_SUCCESS) {
		if ((pstCropInfo_bak.stCropRect.s32X == pstCropInfo.stCropRect.s32X) &&
		    (pstCropInfo_bak.stCropRect.s32Y == pstCropInfo.stCropRect.s32Y) &&
		    (pstCropInfo_bak.stCropRect.u32Width == pstCropInfo.stCropRect.u32Width) &&
		    (pstCropInfo_bak.stCropRect.u32Height == pstCropInfo.stCropRect.u32Height)) {
			VI_UT_PRT("CVI_VI_GetChnCrop TEST-PASS\n");
		} else {
			VI_UT_PRT("CVI_VI_GetChnCrop TEST-FAIL\n");
			return CVI_FAILURE;
		}
	} else {
		VI_UT_PRT("CVI_VI_GetChnCrop TEST-FAIL\n");
		return CVI_FAILURE;
	}

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		s32Ret = vi_ut_get_chn_frame(0);
	}

	return s32Ret;
}

static int case_set_chn_rotation(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ROTATION_E rot;
	ROTATION_E rot_bak;
	int tmp;

	UNUSED(p);

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		vi_ut_ctx.is_get_dpcm_mode	= 1;
		vi_ut_ctx.is_be_online		= 1;
		vi_ut_ctx.is_post_online	= 0;
		vi_ut_ctx.is_hdr_enable		= 0;
		vi_ut_ctx.is_vpss_online	= 0;
		if (g_is_patgen) {
			vi_ut_ctx.is_enable_sensor	= 0;
			vi_ut_ctx.is_patgen_enable	= 1;
		} else {
			vi_ut_ctx.is_enable_sensor	= 1;
			vi_ut_ctx.is_patgen_enable	= 0;
		}

		s32Ret = vi_test();
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}

		rot = ROTATION_90;
	} else {
		VI_UT_PRT("Rotation 0(0)/1(90)/2(180)/3(270): ");
		scanf("%d", &tmp);
		rot = tmp;
	}

	s32Ret = CVI_VI_SetChnRotation(0, 0, rot);
	if (s32Ret == CVI_SUCCESS) {
		VI_UT_PRT("CVI_VI_SetChnRotation TEST-PASS\n");
	} else {
		VI_UT_PRT("CVI_VI_SetChnRotation TEST-FAIL\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_VI_GetChnRotation(0, 0, &rot_bak);
	if (s32Ret == CVI_SUCCESS) {
		if (rot_bak == rot) {
			VI_UT_PRT("CVI_VI_GetChnRotation TEST-PASS\n");
		} else {
			VI_UT_PRT("CVI_VI_GetChnRotation TEST-FAIL\n");
			return CVI_FAILURE;
		}
	} else {
		VI_UT_PRT("CVI_VI_GetChnRotation TEST-FAIL\n");
		return CVI_FAILURE;
	}

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		s32Ret = vi_ut_get_chn_frame(0);
	}

	return s32Ret;
}

static int case_set_chn_flip_mirror(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_BOOL flip, mirror;
	CVI_BOOL flip_bak, mirror_bak;
	int tmp;

	UNUSED(p);

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		vi_ut_ctx.is_get_dpcm_mode	= 1;
		vi_ut_ctx.is_be_online		= 1;
		vi_ut_ctx.is_post_online	= 0;
		vi_ut_ctx.is_hdr_enable		= 0;
		vi_ut_ctx.is_vpss_online	= 0;
		if (g_is_patgen) {
			vi_ut_ctx.is_enable_sensor	= 0;
			vi_ut_ctx.is_patgen_enable	= 1;
		} else {
			vi_ut_ctx.is_enable_sensor	= 1;
			vi_ut_ctx.is_patgen_enable	= 0;
		}

		s32Ret = vi_test();
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}

		flip = 1;
		mirror = 1;
	} else {
		VI_UT_PRT("flip enable/disable(1/0): ");
		scanf("%d", &tmp);
		flip = tmp;
		VI_UT_PRT("mirror enable/disable(1/0): ");
		scanf("%d", &tmp);
		mirror = tmp;
	}

	s32Ret = CVI_VI_SetChnFlipMirror(0, 0, flip, mirror);
	if (s32Ret == CVI_SUCCESS) {
		VI_UT_PRT("CVI_VI_SetChnFlipMirror TEST-PASS\n");
	} else {
		VI_UT_PRT("CVI_VI_SetChnFlipMirror TEST-FAIL\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_VI_GetChnFlipMirror(0, 0, &flip_bak, &mirror_bak);
	if (s32Ret == CVI_SUCCESS) {
		if ((flip_bak == flip) &&
		    (mirror_bak == mirror)) {
			VI_UT_PRT("CVI_VI_GetChnFlipMirror TEST-PASS\n");
		} else {
			VI_UT_PRT("CVI_VI_GetChnFlipMirror TEST-FAIL\n");
			return CVI_FAILURE;
		}
	} else {
		VI_UT_PRT("CVI_VI_GetChnFlipMirror TEST-FAIL\n");
		return CVI_FAILURE;
	}

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		s32Ret = vi_ut_get_chn_frame(0);
	}

	return s32Ret;
}

static int case_show_proc_vi(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		vi_ut_ctx.is_get_dpcm_mode	= 1;
		vi_ut_ctx.is_be_online		= 1;
		vi_ut_ctx.is_post_online	= 0;
		vi_ut_ctx.is_hdr_enable		= 0;
		vi_ut_ctx.is_vpss_online	= 0;
		if (g_is_patgen) {
			vi_ut_ctx.is_enable_sensor	= 0;
			vi_ut_ctx.is_patgen_enable	= 1;
		} else {
			vi_ut_ctx.is_enable_sensor	= 1;
			vi_ut_ctx.is_patgen_enable	= 0;
		}

		s32Ret = vi_test();
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}

		usleep(1000 * 1000);
	}

	system("cat /proc/soph/vi");

	return s32Ret;
}

static int case_show_proc_vi_dbg(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		vi_ut_ctx.is_get_dpcm_mode	= 1;
		vi_ut_ctx.is_be_online		= 1;
		vi_ut_ctx.is_post_online	= 0;
		vi_ut_ctx.is_hdr_enable		= 0;
		vi_ut_ctx.is_vpss_online	= 0;
		if (g_is_patgen) {
			vi_ut_ctx.is_enable_sensor	= 0;
			vi_ut_ctx.is_patgen_enable	= 1;
		} else {
			vi_ut_ctx.is_enable_sensor	= 1;
			vi_ut_ctx.is_patgen_enable	= 0;
		}

		s32Ret = vi_test();
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}

		usleep(1000 * 1000);
	}

	system("cat /proc/soph/vi_dbg");

	return s32Ret;
}

static CVI_S32 case_dump_vi_raw_frame(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVideoFrame[2];
	VI_DUMP_ATTR_S attr;
	VI_PIPE_ATTR_S pipe_attr;
	struct timeval tv1;
	int frm_num = 1, j = 0;
	CVI_U32 dev = 0, loop = 1, set_rawdump_crop = 0;
	struct timespec start, end;

	UNUSED(p);

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		vi_ut_ctx.is_get_dpcm_mode	= 1;
		vi_ut_ctx.is_be_online		= 1;
		vi_ut_ctx.is_post_online	= 0;
		vi_ut_ctx.is_hdr_enable		= 0;
		vi_ut_ctx.is_vpss_online	= 0;
		if (g_is_patgen) {
			vi_ut_ctx.is_enable_sensor	= 0;
			vi_ut_ctx.is_patgen_enable	= 1;
		} else {
			vi_ut_ctx.is_enable_sensor	= 1;
			vi_ut_ctx.is_patgen_enable	= 0;
		}

		s32Ret = vi_test();
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	} else {
		VI_UT_PRT("To get raw dump from dev(0~1): ");
		scanf("%d", &dev);
		VI_UT_PRT("How many loops to do (1~60): ");
		scanf("%d", &loop);
		VI_UT_PRT("Set rawdump crop or not? (0:no, 1:yes): ");
		scanf("%d", &set_rawdump_crop);
	}

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

	if (set_rawdump_crop != 0) {
		s32Ret = vi_ut_set_rawdump_crop(dev, stVideoFrame);
		if (s32Ret == CVI_FAILURE)
			return CVI_FAILURE;
	}

	if (loop > 60) {
		VI_UT_PRT("loop (%d) > 60\n", loop);
		return CVI_FAILURE;
	}

	while (loop > 0) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		frm_num = 1;

		CVI_VI_GetPipeFrame(dev, stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			loop--;
			continue;
		}

		if (stVideoFrame[1].stVFrame.u64PhyAddr[0] != 0)
			frm_num = 2;

		gettimeofday(&tv1, NULL);

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
						"./vi_%d_%s_%s_w_%d_h_%d_x_%d_y_%d_tv_%ld_%ld.raw",
						dev, (j == 0) ? "LE" : "SE", order_id,
						stVideoFrame[j].stVFrame.u32Width,
						stVideoFrame[j].stVFrame.u32Height,
						stVideoFrame[j].stVFrame.s16OffsetLeft,
						stVideoFrame[j].stVFrame.s16OffsetTop,
						tv1.tv_sec, tv1.tv_usec);

				VI_UT_PRT("dump image %s\n", img_name);

				output = fopen(img_name, "wb");

				fwrite(ptr, image_size, 1, output);
				fclose(output);
				free(ptr);
			}
		}

		CVI_VI_ReleasePipeFrame(dev, stVideoFrame);

		clock_gettime(CLOCK_MONOTONIC, &end);
		VI_UT_PRT("ms consumed: %f\n", (CVI_FLOAT)vi_ut_diff_in_us(start, end) / 1000);

		loop--;
	}

	VI_UT_PRT("Dump VI raw TEST-PASS\n");

	return s32Ret;
}

static int case_dump_vi_smooth_raw_frame(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int tmp;
	VI_PIPE ViPipe = 0;
	VB_POOL PoolID;
	VB_POOL_CONFIG_S cfg;
	VI_SMOOTH_RAW_DUMP_INFO_S stDumpInfo;
	VIDEO_FRAME_INFO_S stVideoFrame[2];
	VI_DEV_ATTR_S stDevAttr;
	VI_PIPE_ATTR_S stPipeAttr;
	VI_CHN_ATTR_S stChnAttr;
	CVI_U8 BlkCnt = 2, TotalFrameCnt = 3;
	CVI_U64 u64PhyAddr, *phy_addr_list = CVI_NULL;
	VB_BLK vb_blk;
	int frm_num = 1, j = 0;
	struct timeval tv1;
	CVI_U32 dev_frm_w, dev_frm_h, frm_w, frm_h;
	CVI_U32 set_smooth_rawdump_crop = 0;
	CVI_U32 crop_x = 0, crop_y = 0, crop_w = 0, crop_h = 0;

	UNUSED(p);

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		vi_ut_ctx.is_get_dpcm_mode	= 1;
		vi_ut_ctx.is_be_online		= 1;
		vi_ut_ctx.is_post_online	= 0;
		vi_ut_ctx.is_hdr_enable		= 0;
		vi_ut_ctx.is_vpss_online	= 0;
		if (g_is_patgen) {
			vi_ut_ctx.is_enable_sensor	= 0;
			vi_ut_ctx.is_patgen_enable	= 1;
		} else {
			vi_ut_ctx.is_enable_sensor	= 1;
			vi_ut_ctx.is_patgen_enable	= 0;
		}

		s32Ret = vi_test();
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	} else {
		VI_UT_PRT("The vi dev to dump =");
		scanf("%d", &tmp);
		ViPipe = tmp;
		VI_UT_PRT("The ring buf number to create =");
		scanf("%d", &tmp);
		BlkCnt = tmp;
		VI_UT_PRT("The total frame number to get =");
		scanf("%d", &tmp);
		TotalFrameCnt = tmp;
		VI_UT_PRT("Set smooth rawdump crop or not? (0:no, 1:yes): ");
		scanf("%d", &set_smooth_rawdump_crop);
	}

	CVI_VI_GetDevAttr((VI_DEV)ViPipe, &stDevAttr);
	CVI_VI_GetChnAttr(0, (VI_CHN)ViPipe, &stChnAttr);
	CVI_VI_GetPipeAttr(ViPipe, &stPipeAttr);
	stPipeAttr.enCompressMode = vi_ut_ctx.enCompressMode;
	CVI_VI_SetPipeAttr(ViPipe, &stPipeAttr);

	dev_frm_w = stDevAttr.stSize.u32Width;
	dev_frm_h = stDevAttr.stSize.u32Height;
	if (set_smooth_rawdump_crop != 0) {
		VI_UT_PRT("dev(%d) input size : w(%d), h(%d)\n",
			ViPipe, dev_frm_w, dev_frm_h);

		VI_UT_PRT("To set smooth rawdump crop x: ");
		scanf("%d", &crop_x);
		VI_UT_PRT("To set smooth rawdump crop y: ");
		scanf("%d", &crop_y);
		VI_UT_PRT("To set smooth rawdump crop w: ");
		scanf("%d", &crop_w);
		VI_UT_PRT("To set smooth rawdump crop h: ");
		scanf("%d", &crop_h);

		if (crop_x % 2 || crop_y % 2 || crop_w % 2 || crop_h % 2) {
			VI_UT_PRT("crop_x(%d)_y(%d)_w(%d)_h(%d) must be multiple of 2.\n",
				crop_x, crop_y, crop_w, crop_h);
			return CVI_FAILURE;
		}

		if ((crop_x + crop_w) > stDevAttr.stSize.u32Width ||
			(crop_y + crop_h) > stDevAttr.stSize.u32Height) {
			VI_UT_PRT("crop_x(%d)+w(%d) or y(%d)+h(%d) is bigger than dev_w(%d)_h(%d)\n",
				crop_x, crop_w, crop_y, crop_h, stDevAttr.stSize.u32Width, stDevAttr.stSize.u32Height);
			return CVI_FAILURE;
		}

		frm_w = crop_w;
		frm_h = crop_h;
	} else {
		frm_w = dev_frm_w;
		frm_h = dev_frm_h;
	}

	frm_num = (stDevAttr.stWDRAttr.enWDRMode == WDR_MODE_2To1_LINE) ? 2 : 1;
	cfg.u32BlkCnt = frm_num * BlkCnt;
	cfg.u32BlkSize = VI_GetRawBufferSize(frm_w, frm_h,
					PIXEL_FORMAT_RGB_BAYER_12BPP,
					stPipeAttr.enCompressMode,
					16,
					(stChnAttr.stSize.u32Width > 2304) ? CVI_TRUE : CVI_FALSE);

	VI_UT_PRT("Create VB pool cnt(%d) blksize(0x%x)\n",
			cfg.u32BlkCnt, cfg.u32BlkSize);

	PoolID = CVI_VB_CreatePool(&cfg);
	if (PoolID == VB_INVALID_POOLID) {
		VI_UT_PRT("create vb pool failed\n");
		s32Ret = CVI_FAILURE;
		return s32Ret;
	}

	phy_addr_list = malloc(sizeof(*phy_addr_list) * cfg.u32BlkCnt);
	if (phy_addr_list == CVI_NULL) {
		VI_UT_PRT("malloc phy_addr_list failed\n");
		s32Ret = CVI_FAILURE;
		return s32Ret;
	}

	for (CVI_U32 i = 0; i < cfg.u32BlkCnt; i++) {
		vb_blk = CVI_VB_GetBlock(PoolID, cfg.u32BlkSize);
		if (vb_blk == VB_INVALID_HANDLE) {
			VI_UT_PRT("get VB blk failed\n");
			s32Ret = CVI_FAILURE;
			return s32Ret;
		}
		u64PhyAddr = CVI_VB_Handle2PhysAddr(vb_blk);
		*(phy_addr_list + i) = u64PhyAddr;
		VI_UT_PRT("i=%d, vb_blk=%"PRIx64", addr(0x%"PRIx64"), phy_addr(0x%"PRIx64")\n",
					i, (intmax_t)vb_blk, u64PhyAddr, *(phy_addr_list + i));
	}

	memset(&stDumpInfo, 0, sizeof(stDumpInfo));
	stDumpInfo.ViPipe = ViPipe;
	stDumpInfo.u8BlkCnt = BlkCnt;
	stDumpInfo.phy_addr_list = phy_addr_list;
	// set rawdump crop info in stDumpInfo
	stDumpInfo.stCropRect.s32X = crop_x;
	stDumpInfo.stCropRect.s32Y = crop_y;
	stDumpInfo.stCropRect.u32Width = crop_w;
	stDumpInfo.stCropRect.u32Height = crop_h;

	s32Ret = CVI_VI_StartSmoothRawDump(&stDumpInfo);
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("start failed\n");
		s32Ret = CVI_FAILURE;
		return s32Ret;
	}

	for (int i = 0; i < TotalFrameCnt; i++) {
		memset(stVideoFrame, 0, sizeof(stVideoFrame));
		s32Ret = CVI_VI_GetSmoothRawDump(ViPipe, stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("[%d] get frame failed\n", i);
			continue;
		}

		VI_UT_PRT("[%d] get roi frame addr(0x%"PRIx64") length(%d), number(%d)\n", i,
				stVideoFrame[0].stVFrame.u64PhyAddr[0],
				stVideoFrame[0].stVFrame.u32Length[0],
				stVideoFrame[0].stVFrame.u32TimeRef);

		if (stVideoFrame[1].stVFrame.u64PhyAddr[0] != 0) {
			VI_UT_PRT("[%d] get roi frame addr(0x%"PRIx64") length(%d) number(%d)\n", i,
					stVideoFrame[1].stVFrame.u64PhyAddr[0],
					stVideoFrame[1].stVFrame.u32Length[0],
					stVideoFrame[1].stVFrame.u32TimeRef);
		}

		// save raw file
		gettimeofday(&tv1, NULL);

		for (j = 0; j < frm_num; j++) {
			size_t image_size = stVideoFrame[j].stVFrame.u32Length[0];
			unsigned char *ptr = calloc(1, image_size);
			FILE *output;
			char img_name[128] = {0,}, order_id[8] = {0,};

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
					"./vi_%d_%s_%s_w_%d_h_%d_x_%d_y_%d_tv_%ld_%ld.raw",
					ViPipe, (j == 0) ? "LE" : "SE", order_id,
					stVideoFrame[j].stVFrame.u32Width,
					stVideoFrame[j].stVFrame.u32Height,
					stVideoFrame[j].stVFrame.s16OffsetLeft,
					stVideoFrame[j].stVFrame.s16OffsetTop,
					tv1.tv_sec, tv1.tv_usec);

			VI_UT_PRT("dump image %s\n", img_name);

			output = fopen(img_name, "wb");

			fwrite(ptr, image_size, 1, output);
			fclose(output);
			free(ptr);
		}

		s32Ret = CVI_VI_PutSmoothRawDump(ViPipe, stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("[%d] release frame failed\n", i);
			continue;
		}
	}

	s32Ret = CVI_VI_StopSmoothRawDump(&stDumpInfo);
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("stop failed\n");
		s32Ret = CVI_FAILURE;
		return s32Ret;
	}

	for (CVI_U32 i = 0; i < cfg.u32BlkCnt; i++) {
		u64PhyAddr = *(phy_addr_list + i);
		vb_blk = CVI_VB_PhysAddr2Handle(u64PhyAddr);
		if (vb_blk != VB_INVALID_HANDLE) {
			CVI_VB_ReleaseBlock(vb_blk);
		}
	}

	if (phy_addr_list != CVI_NULL) {
		free(phy_addr_list);
		phy_addr_list = CVI_NULL;
	}
	s32Ret = CVI_VB_DestroyPool(PoolID);

	return s32Ret;
}

static int case_vi_multi_process_test(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	vi_ut_ctx.is_get_dpcm_mode	= 1;
	vi_ut_ctx.is_enable_sensor	= 0;
	vi_ut_ctx.is_be_online		= 0;
	vi_ut_ctx.is_post_online	= 1;
	vi_ut_ctx.is_patgen_enable	= 1;
	vi_ut_ctx.is_hdr_enable		= 0;
	vi_ut_ctx.is_vpss_online	= 0;

	s32Ret = vi_test();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = system("./vi_ut_client 1");
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("client fail\n");
	}

	return s32Ret;
}

static int _vi_sdk_test(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_U8	chn = 0;
	CVI_S32 ret = CVI_SUCCESS;

	ret = CVI_VI_GetPipeFd(0);
	if (ret > 0) {
		VI_UT_PRT("CVI_VI_GetPipeFd TEST-PASS\n");
	} else {
		VI_UT_PRT("CVI_VI_GetPipeFd TEST-FAIL\n");
		return CVI_FAILURE;
	}

	ret = CVI_VI_CloseFd();
	if (ret == CVI_SUCCESS) {
		VI_UT_PRT("CVI_VI_CloseFd TEST-PASS\n");
	} else {
		VI_UT_PRT("CVI_VI_CloseFd TEST-FAIL\n");
		return CVI_FAILURE;
	}

	if (vi_ut_get_chn_frame(chn) == CVI_SUCCESS)
		VI_UT_PRT("CVI_VI_GetChnFrame TEST-PASS\n");
	else {
		VI_UT_PRT("CVI_VI_GetChnFrame TEST-FAIL\n");
		return CVI_FAILURE;
	}

	VIDEO_FRAME_INFO_S stVideoFrame[2];
	VI_DUMP_ATTR_S attr;
	CVI_U32 dev = 0;

	memset(stVideoFrame, 0, sizeof(stVideoFrame));

	stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stVideoFrame[1].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

	attr.bEnable = 1;
	attr.u32Depth = 0;
	attr.enDumpType = VI_DUMP_TYPE_RAW;
	CVI_VI_SetPipeDumpAttr(dev, &attr);

	attr.bEnable = 0;
	attr.enDumpType = VI_DUMP_TYPE_IR;
	CVI_VI_GetPipeDumpAttr(dev, &attr);

	ret = CVI_VI_GetPipeFrame(dev, stVideoFrame, 1000);
	if (ret == CVI_SUCCESS)
		VI_UT_PRT("CVI_VI_GetPipeFrame TEST-PASS\n");
	else {
		VI_UT_PRT("CVI_VI_GetPipeFrame TEST-FAIL\n");
		return CVI_FAILURE;
	}

	ret = CVI_VI_ReleasePipeFrame(dev, stVideoFrame);
	if (ret == CVI_SUCCESS)
		VI_UT_PRT("CVI_VI_ReleasePipeFrame TEST-PASS\n");
	else {
		VI_UT_PRT("CVI_VI_ReleasePipeFrame TEST-FAIL\n");
		return CVI_FAILURE;
	}

	ret = CVI_VI_SetPipeFrameSource(0, VI_PIPE_FRAME_SOURCE_DEV);
	if (ret  == CVI_SUCCESS)
		VI_UT_PRT("CVI_VI_SetPipeFrameSource TEST-PASS\n");
	else {
		VI_UT_PRT("CVI_VI_SetPipeFrameSource TEST-FAIL\n");
		return CVI_FAILURE;
	}

	VI_PIPE_FRAME_SOURCE_E penSource = VI_PIPE_FRAME_SOURCE_USER_BE;

	ret = CVI_VI_GetPipeFrameSource(0, &penSource);
	if (ret == CVI_SUCCESS)
		if (penSource == VI_PIPE_FRAME_SOURCE_DEV)
			VI_UT_PRT("CVI_VI_GetPipeFrameSource TEST-PASS\n");
		else {
			VI_UT_PRT("CVI_VI_GetPipeFrameSource TEST-FAIL\n");
			return CVI_FAILURE;
		}
	else {
		VI_UT_PRT("CVI_VI_GetPipeFrameSource TEST-FAIL\n");
		return CVI_FAILURE;
	}

	VI_CHN_ATTR_S stChnAttr;

	ret = CVI_VI_GetChnAttr(0, 0, &stChnAttr);
	if (ret != CVI_SUCCESS) {
		VI_UT_PRT("CVI_VI_GetChnAttr fail\n");
		return CVI_FAILURE;
	}

	VI_CROP_INFO_S pstCropInfo;

	pstCropInfo.bEnable = CVI_TRUE;
	pstCropInfo.stCropRect.s32X = ALIGN_DOWN(stChnAttr.stSize.u32Width >> 2, 2);
	pstCropInfo.stCropRect.s32Y = ALIGN_DOWN(stChnAttr.stSize.u32Height >> 2, 2);
	pstCropInfo.stCropRect.u32Width = ALIGN_DOWN(stChnAttr.stSize.u32Width >> 1, 2);
	pstCropInfo.stCropRect.u32Height = ALIGN_DOWN(stChnAttr.stSize.u32Height >> 1, 2);

	ret = CVI_VI_SetChnCrop(0, 0, &pstCropInfo);
	if (ret == CVI_SUCCESS)
		VI_UT_PRT("CVI_VI_SetChnCrop TEST-PASS\n");
	else {
		VI_UT_PRT("CVI_VI_SetChnCrop TEST-FAIL\n");
		return CVI_FAILURE;
	}

	VI_DEV_TIMING_ATTR_S stTimingAttr;

	stTimingAttr.bEnable = CVI_TRUE;
	stTimingAttr.s32FrmRate = 30;

	ret = CVI_VI_SetDevTimingAttr(0, &stTimingAttr);
	if (ret == CVI_SUCCESS)
		VI_UT_PRT("CVI_VI_SetDevTimingAttr TEST-PASS\n");
	else {
		VI_UT_PRT("CVI_VI_SetDevTimingAttr TEST-FAIL\n");
		return CVI_FAILURE;
	}

	stTimingAttr.bEnable = CVI_FALSE;
	stTimingAttr.s32FrmRate = 20;

	ret = CVI_VI_GetDevTimingAttr(0, &stTimingAttr);
	if (ret == CVI_SUCCESS)
		if (stTimingAttr.bEnable == CVI_TRUE && stTimingAttr.s32FrmRate == 30)
			VI_UT_PRT("CVI_VI_GetDevTimingAttr TEST-PASS\n");
		else {
			VI_UT_PRT("CVI_VI_GetDevTimingAttr TEST-FAIL\n");
			return CVI_FAILURE;
		}
	else {
		VI_UT_PRT("CVI_VI_GetDevTimingAttr TEST-FAIL\n");
		return CVI_FAILURE;
	}

	CVI_S32 i;
	CVI_S32 s32ViNum;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	for (i = 0; i < pstViConfig->s32WorkingViNum; i++) {
		s32ViNum  = pstViConfig->as32WorkingViId[i];
		pstViInfo = &pstViConfig->astViInfo[s32ViNum];

		if (SAMPLE_COMM_VI_StopViChn(pstViInfo) == CVI_SUCCESS)
			VI_UT_PRT("CVI_VI_DisableChn TEST-PASS\n");
		else {
			VI_UT_PRT("CVI_VI_DisableChn TEST-FAIL\n");
			return CVI_FAILURE;
		}

		if (SAMPLE_COMM_VI_StopViPipe(pstViInfo) == CVI_SUCCESS)
			VI_UT_PRT("CVI_VI_StopPipe and CVI_VI_DestroyPipe TEST-PASS\n");
		else {
			VI_UT_PRT("CVI_VI_StopPipe and CVI_VI_DestroyPipe TEST-FAIL\n");
			return CVI_FAILURE;
		}

		if (SAMPLE_COMM_VI_StopDev(pstViInfo) == CVI_SUCCESS)
			VI_UT_PRT("CVI_VI_DisableDev TEST-PASS\n");
		else {
			VI_UT_PRT("CVI_VI_DisableDev TEST-FAIL\n");
			return CVI_FAILURE;
		}
	}

	return CVI_SUCCESS;
}

static int case_vi_sdk_test(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	vi_ut_ctx.is_get_dpcm_mode	= 1;
	vi_ut_ctx.is_enable_sensor	= 1;
	vi_ut_ctx.is_be_online		= 1;
	vi_ut_ctx.is_post_online	= 0;
	vi_ut_ctx.is_patgen_enable	= 0;
	vi_ut_ctx.is_hdr_enable		= 0;
	vi_ut_ctx.is_vpss_online	= 0;

	s32Ret = vi_test();
	if (s32Ret == CVI_SUCCESS)
		s32Ret = _vi_sdk_test(&stViConfig);

	return s32Ret;
}

static int case_dump_vi_register(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_PIPE ViPipe = 0;
	char name[64] = {0};
	FILE *fp = NULL;
	VI_DUMP_REGISTER_TABLE_S	reg_tbl;
	ISP_INNER_STATE_INFO_S		stInnerStateInfo;

	UNUSED(p);

	if (vi_ut_ctx.is_test_mode && !vi_ut_ctx.is_fpga_ip_test) {
		vi_ut_ctx.is_get_dpcm_mode	= 1;
		vi_ut_ctx.is_be_online		= 1;
		vi_ut_ctx.is_post_online	= 0;
		vi_ut_ctx.is_hdr_enable		= 0;
		vi_ut_ctx.is_vpss_online	= 0;
		if (g_is_patgen) {
			vi_ut_ctx.is_enable_sensor	= 0;
			vi_ut_ctx.is_patgen_enable	= 1;
		} else {
			vi_ut_ctx.is_enable_sensor	= 1;
			vi_ut_ctx.is_patgen_enable	= 0;
		}

		s32Ret = vi_test();
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}

		usleep(1000 * 1000);
	}

	if (CVI_ISP_QueryInnerStateInfo(ViPipe, &stInnerStateInfo) != CVI_SUCCESS) {
		VI_UT_PRT("CVI_ISP_QueryInnerStateInfo fail");
		return CVI_FAILURE;
	}

	reg_tbl.MlscGainLut.RGain = stInnerStateInfo.mlscGainTable.RGain;
	reg_tbl.MlscGainLut.GGain = stInnerStateInfo.mlscGainTable.GGain;
	reg_tbl.MlscGainLut.BGain = stInnerStateInfo.mlscGainTable.BGain;

	snprintf(name, 64, "vi_dump_register.json");
	fp = fopen(name, "w");
	if (fp == NULL) {
		VI_UT_PRT("open %s fail!!!\n", name);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VI_DumpHwRegisterToFile(ViPipe, fp, &reg_tbl);
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("CVI_VI_DumpHwRegisterToFile failed with %#x\n", s32Ret);
		fclose(fp);
		return s32Ret;
	}

	fclose(fp);
	VI_UT_PRT("Dump register pass\n");

	return s32Ret;
}

static int case_vi_pld_test(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	vi_ut_ctx.is_get_dpcm_mode	= 1;
	vi_ut_ctx.is_enable_sensor	= 0;
	vi_ut_ctx.is_be_online		= 0;
	vi_ut_ctx.is_post_online	= 1;
	vi_ut_ctx.is_patgen_enable	= 1;
	vi_ut_ctx.is_hdr_enable		= 0;
	vi_ut_ctx.is_vpss_online	= 0;

	s32Ret = vi_test();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	_isp_thread_test();

	return s32Ret;
}

static int case_vi_slt_test(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(p);

	vi_ut_ctx.is_get_dpcm_mode	= 1;
	vi_ut_ctx.is_enable_sensor	= 1;
	vi_ut_ctx.is_be_online		= 0;
	vi_ut_ctx.is_post_online	= 1;
	vi_ut_ctx.is_patgen_enable	= 0;
	vi_ut_ctx.is_hdr_enable		= 0;
	vi_ut_ctx.is_vpss_online	= 0;

	s32Ret = vi_ut_slt("./target.yuv");

	return s32Ret;
}

static const struct vi_ut_info vi_uts[] = {
	UT_INFO(CASE_VI_IOCTRL,				 			case_vi_ioctrl,							0),
	UT_INFO(CASE_VI_MMAP,				 			case_vi_mmap,				 			0),
	UT_INFO(CASE_VI_POLL,				 			case_vi_poll,				 			0),
	UT_INFO(CASE_RAW_REPLAY,			 			case_raw_replay,			 			0),
	UT_INFO(CASE_RAW_REPLAY_MANUAL,			 		case_raw_replay_manual,		 			0),
	UT_INFO(CASE_SENSOR_ON_THE_FLY,			 		case_sensor_onthefly,			 		1),
	UT_INFO(CASE_SENSOR_FE_BE_DRAM_POST_DRAM,		case_sensor_fe_be_dram_post_dram,		0),
	UT_INFO(CASE_SENSOR_FE_DRAM_BE_POST_DRAM,		case_sensor_fe_dram_be_post_dram,		0),
	UT_INFO(CASE_PATGEN_ON_THE_FLY,			 		case_patgen_onthefly,			 		1),
	UT_INFO(CASE_PATGEN_FE_BE_DRAM_POST_DRAM,		case_patgen_fe_be_dram_post_dram,		0),
	UT_INFO(CASE_PATGEN_FE_DRAM_BE_POST_DRAM,		case_patgen_fe_dram_be_post_dram,		0),
	UT_INFO(CASE_PATGEN_FE_DRAM_BE_POST_SC,			case_patgen_fe_dram_be_post_sc,	 		0),
	UT_INFO(CASE_HDR_PATGEN_FE_BE_DRAM_POST_DRAM,	case_hdr_patgen_fe_be_dram_post_dram,	0),
	UT_INFO(CASE_HDR_PATGEN_FE_DRAM_BE_POST_DRAM,	case_hdr_patgen_fe_dram_be_post_dram,	0),
	UT_INFO(CASE_DUMP_VI_YUV_FRAME,			 		case_dump_vi_yuv_frame,		 			0),
	UT_INFO(CASE_DUMP_VI_RAW_FRAME,			 		case_dump_vi_raw_frame,		 			0),
	UT_INFO(CASE_DUMP_VI_SMOOTH_RAW_FRAME,		 	case_dump_vi_smooth_raw_frame,		 	0),
	UT_INFO(CASE_DUMP_VI_REGISTER,			 		case_dump_vi_register,			 		0),
	UT_INFO(CASE_SHOW_PROC_VI,			 			case_show_proc_vi,			 			0),
	UT_INFO(CASE_SHOW_PROC_VI_DBG,			 		case_show_proc_vi_dbg,			 		0),
	UT_INFO(CASE_SET_CHN_CROP,			 			case_set_chn_crop,			 			0),
	UT_INFO(CASE_SET_CHN_ROTATION,			 		case_set_chn_rotation,			 		0),
	UT_INFO(CASE_SET_CHN_FLIP_MIRROR,		 		case_set_chn_flip_mirror,		 		0),
	UT_INFO(CASE_VI_MULTI_PROCESSES_TEST,		 	case_vi_multi_process_test,		 		0),
	UT_INFO(CASE_VI_SDK_TEST,			 			case_vi_sdk_test,			 			0),
	UT_INFO(CASE_VI_PLD_TEST,			 			case_vi_pld_test,			 			0),
	UT_INFO(CASE_VI_SLT_TEST,			 			case_vi_slt_test,			 			0),
};

#define VI_UTS ARRAY_SIZE(vi_uts)

static const char *strlwc(const char *in, char *out, unsigned int len)
{
	unsigned int i = 0;

	if (in == NULL || out == NULL || len == 0)
		return NULL;

	while (in[i] != '\0' && i < len - 1) {
		out[i] = (char)tolower((int)in[i]);
		i++;
	}
	out[i] = '\0';
	return out;
}

static void _vi_ut_show_help(void)
{
	uint32_t i = 1;
	char low_name[64] = {0};

	for (; i < VI_UTS; i++) {
		memset(low_name, 0, sizeof(low_name));
		strlwc(vi_uts[i].name, low_name, strlen(vi_uts[i].name) + 1);
		VI_UT_PRT("%4d: %s\n", vi_uts[i].case_no, low_name);
	}
	VI_UT_PRT(" 255: exit\n");
}

static CVI_S32 _vi_ut_handle_op(unsigned int op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	const struct vi_ut_info *info = NULL;

	if (op > VI_UTS)
		return s32Ret;

	info = &vi_uts[op];

	if (vi_ut_ctx.is_test_mode && info->flags)
		return s32Ret;

	if (vi_ut_ctx.is_test_mode) {
		_vi_set_tuning_dis(0, 1, 1, 1);
		VI_UT_PRT("disable tuning_dis\n");
	}

	if (info->func)
		s32Ret = info->func(&vi_ut_ctx);

	if (vi_ut_ctx.is_test_mode) {
		_vi_set_tuning_dis(0, 0, 0, 0);
	}

	return s32Ret;
}

static int run_case_for_fpga(int argc, char *argv[])
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	const struct vi_ut_info *info = NULL;

	s32Ret = vi_fpga_parse_arg(argc, argv);
	if (s32Ret < 0) {
		VI_UT_PRT("Param incorrect!!!\n");
		return s32Ret;
	}

	if (vi_ut_ctx.u32Case > VI_UTS) {
		VI_UT_PRT("Param incorrect!!!\n");
		s32Ret = CVI_FAILURE;
		return s32Ret;
	}

	if (vi_ut_ctx.is_test_mode) {
		s32Ret = _vi_set_tuning_dis(0, 1, 1, 1);
	}

	info = &vi_uts[vi_ut_ctx.u32Case];
	if (info->func) {
		s32Ret = info->func(&vi_ut_ctx);
	}

	usleep(2 * 1000 * 1000);

	if (vi_ut_ctx.is_custom_flow) {
		s32Ret = vi_fpga_dosomething();
	}

	if (vi_ut_ctx.is_dump_yuv) {//dump yuv.
		s32Ret = vi_fpga_dump_yuv(0);
	}

	if (vi_ut_ctx.is_dump_raw) { //dump raw.
		s32Ret = vi_fpga_dump_raw(0);
	}

	if (vi_ut_ctx.is_test_mode) {
		s32Ret = _vi_set_tuning_dis(0, 0, 0, 0);
	}

	return s32Ret;
}

int main(int argc, char *argv[])
{
	unsigned int op;
	CVI_S32 s32Ret;
	CVI_BOOL abChnEnable[VPSS_MAX_CHN_NUM] = {CVI_TRUE, };
	int argv_offset = 0;

	UNUSED(argc);
	UNUSED(argv);

	system("stty erase ^H");

	vi_ut_ctx.pid = -1;

	if (argc >= 3) {
		if (strcmp(argv[2], "CI") == 0) {
			g_is_patgen = true;
			argv_offset = 1;
		}
	}

	if (argc >= 2) {
		if (argc == 2 + argv_offset) {
			vi_ut_ctx.is_test_mode = 1;

			op = (CVI_S32)atoi(argv[1]);
			s32Ret = _vi_ut_handle_op(op);
			VI_UT_PRT("vi ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
		} else {
			//for fpga test, need close 3dnr
			vi_ut_ctx.is_3dnr_off = 1;
			vi_ut_ctx.is_test_mode = 1;
			vi_ut_ctx.is_fpga_ip_test = 1;

			s32Ret = run_case_for_fpga(argc, argv);
		}
	} else {
		do {
			_vi_ut_show_help();
			scanf("%u", &op);
			s32Ret = _vi_ut_handle_op(op);
			if (s32Ret != CVI_SUCCESS) {
				VI_UT_PRT("op(%d) failed with %#x!\n", op, s32Ret);
				break;
			}
			if (vi_ut_ctx.pid == 0)
				break;
		} while (op != 255);
	}

	vi_ut_plat_vi_deinit();

	if (vi_ut_ctx.is_vpss_online)
		SAMPLE_COMM_VPSS_Stop(VPSS_ONLINE_GRP_0, abChnEnable);

	CVI_SYS_Exit();
	CVI_VB_Exit();

	return 0;
}

