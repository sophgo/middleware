#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <pthread.h>
#include <getopt.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "cvi_buffer.h"
#include "cvi_sys.h"
#include "cvi_hdmi.h"
#include "sample_comm.h"

#define SAMPLE_HDMI_PRT(fmt...) \
	do { \
		printf("[%s]-%d: ", __func__, __LINE__); \
		printf(fmt); \
	} while (0)

static struct option long_options[] = {
	{"mcode", required_argument, NULL, 'm'},
	{"pixel_clk", required_argument, NULL, 'p'},
	{"force_output", required_argument, NULL, 'f'},
	{"pixel_repeat", required_argument, NULL, 'r'},
	{"hdcp14_en", required_argument, NULL, 'h'},
	{"csc_en", required_argument, NULL, 'c'},
	{"audio_en", required_argument, NULL, 'a'},
	{"fmt_in", required_argument, NULL, 'i'},
	{"fmt_out", required_argument, NULL, 'o'},
	{"avmute", required_argument, NULL, 'v'},
	{"audio_mute", required_argument, NULL, 'u'},
	{"set_infoframe", required_argument, NULL, 's'},
	{"exit_flag", required_argument, NULL, 'e'},
	{"audio_file", required_argument, NULL, 'n'},
	{"Width", required_argument, NULL, 'W'},
	{"Height", required_argument, NULL, 'H'},
	{"FPS", required_argument, NULL, 'F'},
	{"Pattern", required_argument, NULL, 'P'},
	{NULL, 0, NULL, 0}
};

/* Audio send thread: send one frame, wait for playback duration, then send next */
static volatile int g_audio_send_stop;
static int g_audio_send_started;
static pthread_t g_audio_send_tid;

/*
 * Convert 16-bit PCM to HDMI AHB DMA 32-bit format.
 * Each 2-byte LE sample -> 32-bit word with audio in bits [23:8], bits [7:0] = 0.
 */
static void hdmi_pcm_16bit_to_dma(const short *src, unsigned int *dst,
				   unsigned int num_samples)
{
	while (num_samples--)
		*dst++ = ((unsigned int)(unsigned short)(*src++)) << 8;
}

/*
 * Convert 24-bit packed PCM to HDMI AHB DMA 32-bit format.
 * Each 3-byte LE sample -> 32-bit word with audio in bits [23:0].
 */
static void hdmi_pcm_24bit_to_dma(const unsigned char *src, unsigned int *dst,
				   unsigned int num_samples)
{
	while (num_samples--) {
		*dst++ = (unsigned int)src[0]
		       | ((unsigned int)src[1] << 8)
		       | ((unsigned int)src[2] << 16);
		src += 3;
	}
}

static void *audio_send_thread(void *arg)
{
	CVI_HDMI_AUDIO_FRAME_S frame;
	CVI_U32 seq = 0;
	char *file = (char *)arg;
	FILE *fp;
	CVI_S32 channel = 2, period = 320, bitdepth = 24;
	CVI_S32 period_size = period * channel * (bitdepth / 8);
//	CVI_S32 sample_rate = 44100;
	CVI_S32 read_size;
	char *buf;

	fp = fopen(file, "rb+");
	if (fp == NULL) {
		printf("open file fail\n");
		return NULL;
	}
	buf = (char *)malloc(period_size);
	/* Frame send to HDMI (convert PCM to AHB DMA 32-bit format) */
	VB_POOL s_hdmiVbPool = VB_INVALID_POOLID;
	CVI_U32 dmaFrameSize = period * channel * 4;
	VB_BLK vbBlk = 0;
	CVI_U64 phyAddr = 0;
	CVI_VOID *virAddr;

    /* Lazy-init: create a dedicated VB pool for HDMI DMA */
	if (s_hdmiVbPool == VB_INVALID_POOLID) {
		VB_POOL_CONFIG_S stPoolCfg;

		memset(&stPoolCfg, 0, sizeof(stPoolCfg));
		stPoolCfg.u32BlkSize = dmaFrameSize;
		stPoolCfg.u32BlkCnt = 50;
		strncpy(stPoolCfg.acName, "hdmi_sample",
				sizeof(stPoolCfg.acName) - 1);
		s_hdmiVbPool = CVI_VB_CreatePool(&stPoolCfg);
		printf("HDMI audio DMA pool: id=%d, blkSize=%u,"
				"bitwidth=%d\n", s_hdmiVbPool, dmaFrameSize, bitdepth);
	}
	if (s_hdmiVbPool == VB_INVALID_POOLID)
		printf("alloc vb clk fail\n");

	while (!g_audio_send_stop) {
		read_size = fread(buf, 1, period_size, fp);
		if (read_size < period_size) {
			fseek(fp, 0, SEEK_SET);
			continue;
		} else {
			while (1) {
				vbBlk = CVI_VB_GetBlock(s_hdmiVbPool, dmaFrameSize);
				if (vbBlk == VB_INVALID_HANDLE) {
					usleep(100000);
					continue;
				} else
					break;
			}
			phyAddr = CVI_VB_Handle2PhysAddr(vbBlk);
			virAddr = (CVI_VOID *)CVI_SYS_Mmap(phyAddr, dmaFrameSize);
			if (!virAddr) {
				CVI_VB_ReleaseBlock(vbBlk);
				return NULL;
			}

			if (bitdepth == AUDIO_BIT_WIDTH_16) {
				hdmi_pcm_16bit_to_dma((const short *)buf, (unsigned int *)virAddr, period * channel);
			} else {
				hdmi_pcm_24bit_to_dma((const unsigned char *)buf, (unsigned int *)virAddr, period * channel);
			}

			CVI_SYS_IonFlushCache(phyAddr, virAddr, dmaFrameSize);
			CVI_SYS_Munmap(virAddr, dmaFrameSize);
		}
		memset(&frame, 0, sizeof(frame));
		frame.u64PhyAddr = phyAddr;
		frame.u32Len = dmaFrameSize;
		frame.u64VbBlk = vbBlk;
		frame.u32Seq = seq++;
		if (CVI_HDMI_SendAudioFrame(&frame) != CVI_SUCCESS)
			break;
		CVI_VB_ReleaseBlock(vbBlk);
		usleep(7000);
	}
	free(buf);
	fclose(fp);

	return NULL;
}

CVI_S32 Get_Attr()
{
	int ret;
	CVI_HDMI_ATTR GetAttr;
	memset(&GetAttr, 0, sizeof(GetAttr));

	ret = CVI_HDMI_GetAttr(&GetAttr);
	if(ret){
		printf("hdmi set attr error\n");
	}

	printf("-- HDMI Attribution --\n");
	printf("HDMI Enable ? %d\n", GetAttr.hdmi_en);
	printf("HDMI Video Format : %d\n", GetAttr.video_format);
	printf("HDMI Deep Color Mode : %d\n", GetAttr.deep_color_mode);
	printf("HDMI Pixel Clk : %u\n", GetAttr.pix_clk);
	printf("HDMI Video Input : %d\n", GetAttr.hdmi_video_input);
	printf("HDMI Video Output : %d\n", GetAttr.hdmi_video_output);
	printf("HDMI Hdcp1.4 On ? %d\n", GetAttr.hdcp14_en);
	printf("HDMI Audio Enable ? %d\n", GetAttr.audio_en);
	printf("HDMI Audio Sample Rate : %u\n", GetAttr.sample_rate);
	printf("HDMI Audio Sample Size : %u\n", GetAttr.bit_depth);
	printf("-- END --\n");

	return ret;

}

CVI_S32 Get_InfoFrame()
{
	int ret;
	CVI_HDMI_INFOFRAME Get_infoFrame;
	memset(&Get_infoFrame, 0, sizeof(Get_infoFrame));

	ret = CVI_HDMI_GetInfoFrame(&Get_infoFrame);
	if(ret){
		printf("hdmi set_audio_mute error\n");
		return CVI_FAILURE;
	}

	printf("-- HDMI InfoFrame --\n");
	printf("Pixel Repetition:%d\n", Get_infoFrame.infoframe_unit.avi_infoframe.pixel_repetition);
	printf("Video Color Space:%d\n", Get_infoFrame.infoframe_unit.avi_infoframe.color_space);
	printf("Video Timing Mode:%d\n", Get_infoFrame.infoframe_unit.avi_infoframe.timing_mode);
	printf("Video Colorimetry:%d\n", Get_infoFrame.infoframe_unit.avi_infoframe.colorimetry);
	printf("RGB Quant:%d\n", Get_infoFrame.infoframe_unit.avi_infoframe.rgb_quant);
	printf("Audio Channel Allocation:%d\n", Get_infoFrame.infoframe_unit.audio_infoframe.chn_alloc);
	printf("Audio Coding Type:%d\n", Get_infoFrame.infoframe_unit.audio_infoframe.coding_type);
	printf("Audio Sample Size:%d\n", Get_infoFrame.infoframe_unit.audio_infoframe.sample_size);
	printf("Audio Sampling Freq:%d\n", Get_infoFrame.infoframe_unit.audio_infoframe.sampling_freq);
	printf("-- End --\n");

	return ret;
}

CVI_S32 Get_Edid()
{
	int ret;
	CVI_HDMI_EDID edid_data;
	memset(&edid_data, 0, sizeof(CVI_HDMI_EDID));

	ret = CVI_HDMI_ForceGetEdid(&edid_data);
	if(ret){
		printf("hdmi force get edid error, ret:%d\n", ret);
		return ret;
	}

	if(edid_data.edid){
		printf("Edid Raw Data:\n");
		for(int i = 1; i <= 256; i++)
		{
			printf("%02X \n", edid_data.edid[i-1]);
			if(i%16 == 0)
			{
				printf("\n");
			}
		}
		printf("\n");
	}

	return ret;
}

CVI_S32 Get_Sink_Cap()
{
	int i = 0;
	int ret;
	int mcode = 0;
	CVI_HDMI_SINK_CAPABILITY capability;

	memset(&capability, 0, sizeof(capability));

	ret = CVI_HDMI_GetSinkCapability(&capability);
	if(ret){
		printf("hdmi get sink capability error\n");
		return ret;
	}

	printf("-- HDMI Sink Capability --\n");
	printf("Supported Video Mode:\n");
	while(capability.support_video_format[mcode].mcode)
	{
		printf("mcode:%3d  %4dx%4d%s@%0.2f  pixel_clk:%d\n", capability.support_video_format[mcode].mcode,
										capability.support_video_format[mcode].timing_info.hact,
										capability.support_video_format[mcode].timing_info.vact,
										capability.support_video_format[mcode].timing_info.interlace ? "i" : "p",
										((double)capability.support_video_format[mcode].fresh_rate) / 1000,
										capability.support_video_format[mcode].timing_info.pixel_clk);
		mcode++;
	}

	printf("\n");
	printf("sink is connected ? %d\n", capability.is_connected);
	printf("sink supports hdmi ? %d\n", capability.support_hdmi);
	printf("sink supports hdmi2.0 ? %d\n", capability.support_hdmi_2_0);
	printf("native_video_format:%d\n", capability.native_video_format);
	printf("sink supports ycbcr ? %d\n", capability.support_ycbcr);
	printf("version : %x\n", capability.version);
	printf("revision : %x\n", capability.revision);
	printf("sink supports xvycc601 ? %d\n", capability.support_xvycc601);
	printf("sink supports xvycc709 ? %d\n", capability.support_xvycc709);
	printf("hdcp1.4 is enabled by sink ? %d\n", capability.hdcp14_en);
	printf("hdmi video output mode : %d\n", capability.hdmi_video_output);
	printf("hdmi audio output channel : %d\n", capability.audio_info[0].audio_chn);
	printf("sink support_dvi_dual ? %d\n", capability.support_dvi_dual);
	printf("sink support_deepcolor_ycbcr444 ? %d\n", capability.support_deepcolor_ycbcr444);
	printf("sink support_deep_color_30bit ? %d\n", capability.support_deep_color_30bit);
	printf("sink support_deep_color_36bit ? %d\n", capability.support_deep_color_36bit);
	printf("sink support_deep_color_48bit ? %d\n", capability.support_deep_color_48bit);
	printf("sink support_y420_dc_30bit ? %d\n", capability.support_y420_dc_30bit);
	printf("sink support_y420_dc_36bit ? %d\n", capability.support_y420_dc_36bit);
	printf("sink support_y420_dc_48bit ? %d\n", capability.support_y420_dc_48bit);
	printf("sink support_ai ? %d\n", capability.support_ai);
	printf("sink max_tmds_clk ? %d\n", capability.max_tmds_clk);
	printf("sink ycc_quant_selectable ? %d\n", capability.ycc_quant_selectable);
	printf("sink rgb_quant_selectable ? %d\n", capability.rgb_quant_selectable);
	printf("sink's current hact : %d\n", capability.detailed_timing.detail_timing[0].hact);
	printf("sink's current vact : %d\n", capability.detailed_timing.detail_timing[0].vact);
	printf("sink's current hbb : %d\n", capability.detailed_timing.detail_timing[0].hbb);
	printf("sink's current hfb : %d\n", capability.detailed_timing.detail_timing[0].hfb);
	printf("sink's current hpw : %d\n", capability.detailed_timing.detail_timing[0].hpw);
	printf("sink's current vbb : %d\n", capability.detailed_timing.detail_timing[0].vbb);
	printf("sink's current vfb : %d\n", capability.detailed_timing.detail_timing[0].vfb);
	printf("sink's current vpw : %d\n", capability.detailed_timing.detail_timing[0].vpw);
	printf("sink's current mHSyncPolarity : %d\n", capability.detailed_timing.detail_timing[0].ihs);
	printf("sink's current mVSyncPolarity : %d\n", capability.detailed_timing.detail_timing[0].ivs);
	printf("sink's current mInterlaced : %d\n", capability.detailed_timing.detail_timing[0].interlace);
	printf("sink's current HImageSize : %d\n", capability.detailed_timing.detail_timing[0].img_width);
	printf("sink's current VImageSize : %d\n", capability.detailed_timing.detail_timing[0].img_height);
	printf("sink's current aspect_ratio_w : %d\n", capability.detailed_timing.detail_timing[0].aspect_ratio_w);
	printf("sink's current aspect_ratio_h : %d\n", capability.detailed_timing.detail_timing[0].aspect_ratio_h);
	printf("sink's current Pixel Clock : %d\n", capability.detailed_timing.detail_timing[0].pixel_clk);
	printf("sink's audio info number : %d\n", capability.audio_info_num);
	printf("sink's max audio channels : %d\n", capability.audio_info[0].audio_chn);
	printf("supported max sample rate by sink:%uhz\n", capability.audio_info[0].max_bit_rate);
	printf("supported sample rate by sink:\n");

	for(i=0; i<(int)(capability.audio_info[0].support_sample_rate_num); i++){
		printf(" %uhz\n", capability.audio_info[0].support_sample_rate[i]);
	}

	printf("supported max bit depth by sink:%ubit\n", capability.audio_info[0].support_bit_depth[0]);
	printf("supported bit depth by sink:\n");
	for(i=0; i<(int)(capability.audio_info[0].support_bit_depth_num); i++){
		printf(" %ubit\n", capability.audio_info[0].support_bit_depth[i]);
	}

	printf("sink's video latency : %d\n", capability.video_latency);
	printf("sink's audio latency : %d\n", capability.audio_latency);
	printf("sink's interlaced video latency : %d\n", capability.interlaced_video_latency);
	printf("sink's interlaced audio latency : %d\n", capability.interlaced_audio_latency);
	printf("-- End --\n");

	return ret;
}

CVI_VOID Hdmi_EventProc(CVI_HDMI_EVENT_TYPE event, CVI_VOID *private_data)
{
	(void)private_data;

	switch (event)
	{
	case CVI_HDMI_EVENT_HOTPLUG:
		printf("HPDPLUG EVENT\n");
		break;
	case CVI_HDMI_EVENT_NO_PLUG:
		printf("NOPLUG EVENT\n");
		break;
	case CVI_HDMI_EVENT_EDID_FAIL:
		printf("EDID FAILED EVENT\n");
		break;
	default:
		printf("Unrecognized EVENT\n");
		break;
	}
}

CVI_VOID SAMPLE_HDMI_Usage(CVI_CHAR *sPrgNm)
{
	printf("Usage : %s <mCode> <pixel_clk> <force_output> <pixel_repeat> <hdcp14_en> <csc_en> <audio_en> <csc_fmt_in> <csc_fmt_out> " \
	       "<avmute_en> <audio_mute_en> <set_infoframe> <exit_flag> <audio_file>\n", sPrgNm);
	printf("     mCode:\n");
	printf("\t     1:   640x480p-60\n");
	printf("\t     2:   720x480p-60\n");
	printf("\t     4:   1280x720p-60\n");
	printf("\t    16:  1920x1080p-60\n");
	printf("\t    90:  2560x1080p-60\n");
	printf("\t   102:  4096x2160p-60\n");
	printf("\t   107:  3840x2160p-60\n");
	printf("\t   108:  2560x1440p-60\n");
	printf("\t pic_fmt:  \n");
	printf("\t   0  :  RGB888\n");
	printf("\t   1  :  YUV444\n");
	printf("\t   2  :  YUV422\n");
	printf("Example 1 : ./sample_hdmi --Width 1920 --Height 1080 --FPS 60.00 --Pattern 1 --exit_flag 1\n");
	printf("Example 2 : ./sample_hdmi --mcode 16 --pixel_clk 148500 --force_output 0 --pixel_repeat 0 --hdcp14_en 0 --csc_en 0 --audio_en 1 " \
	       "--fmt_in 0 --fmt_out 0 --avmute 0 --audio_mute 0 --set_infoframe 1 --exit_flag 1 --audio_file ./audio.raw \n");
	printf("          (test pixel repeat should set set_infoframe = 1) \n");
	printf("          (loopback test should set exit_flag = 0) \n");
}

CVI_S32 main(CVI_S32 argc, CVI_CHAR *argv[])
{
	CVI_S32 s32Ret = CVI_FAILURE;
	CVI_HDMI_ATTR setAttr;
	CVI_HDMI_CALLBACK_FUNC callback_func;
	CVI_CHAR* filename = "";
	CVI_S32 mcode = 0, pixel_clk = 0, hdcp14_en = 0, csc_en = 0, audio_en = 0, fmt_in = 0, fmt_out = 0, avmute = 0;
	CVI_S32	audio_mute = 0, set_infoframe = 0, force_output = 0, pixel_repeat = 0, exit_flag = 0, opt = 0, option_index = 0;
	CVI_S32	width = 0, height = 0, pattern = 0, mcode_temp = 0;
	CVI_DOUBLE fps = 0.0;

	if (argc < 2 || !strncmp(argv[1], "-h", 2)) {
		SAMPLE_HDMI_Usage(argv[0]);
		return CVI_SUCCESS;
	}

	while ((opt = getopt_long(argc, argv, "W:H:F:P:m:p:f:r:h:c:a:i:o:v:u:s:e:n", long_options, &option_index)) != -1) {
			switch (opt) {
			case 'W':
				width = atoi(optarg);
				break;
			case 'H':
				height = atoi(optarg);
				break;
			case 'F':
				fps = atof(optarg);
				break;
			case 'P':
				pattern = atoi(optarg);
				break;
			case 'm':
				mcode = atoi(optarg);
				break;
			case 'p':
				pixel_clk = atoi(optarg);
				break;
			case 'f':
				force_output = atoi(optarg);
				break;
			case 'r':
				pixel_repeat = atoi(optarg);
				break;
			case 'h':
				hdcp14_en = atoi(optarg);
				break;
			case 'c':
				csc_en = atoi(optarg);
				break;
			case 'a':
				audio_en = atoi(optarg);
				break;
			case 'i':
				fmt_in = atoi(optarg);
				break;
			case 'o':
				fmt_out = atoi(optarg);
				break;
			case 'v':
				avmute = atoi(optarg);
				break;
			case 'u':
				audio_mute = atoi(optarg);
				break;
			case 's':
				set_infoframe = atoi(optarg);
				break;
			case 'e':
				exit_flag = atoi(optarg);
				break;
			case 'n':
				filename = optarg;
				break;
			default:
				SAMPLE_HDMI_Usage(argv[0]);
				return CVI_FAILURE;
        }
    }

	if (!width && !height && !fps){
		if (argc < 14) {
			SAMPLE_HDMI_Usage(argv[0]);
			return CVI_FAILURE;
		}
		if(!mcode || !pixel_clk)
		{
			printf("mcode or pixel_clk should be no-zero\n");
			return CVI_FAILURE;
		}
	} else if (!width || !height || !fps) {
		printf("width、height and fps should be no-zero\n");
		return CVI_FAILURE;
	}

	memset(&setAttr, 0, sizeof(setAttr));

	s32Ret = CVI_SYS_Init();
	if(s32Ret){
		printf("CVI_SYS_Init error\n");
		return CVI_FAILURE;
	}

	s32Ret = Get_Edid();
	if(s32Ret){
		printf("HDMI Get Edid error\n");
		return CVI_FAILURE;
	}

	s32Ret = Get_Sink_Cap();
	if(s32Ret){
		printf("HDMI Get Sink Capability error\n");
		return CVI_FAILURE;
	}

	if (width && height && fps) {
		CVI_HDMI_SINK_CAPABILITY capability;
		memset(&capability, 0, sizeof(capability));
		s32Ret = CVI_HDMI_GetSinkCapability(&capability);
		if(s32Ret){
			printf("hdmi get sink capability error\n");
			return s32Ret;
		}

		while(capability.support_video_format[mcode_temp].mcode)
		{
			if ((width == (CVI_S32)capability.support_video_format[mcode_temp].timing_info.hact) &&
				(height == (CVI_S32)capability.support_video_format[mcode_temp].timing_info.vact) &&
				(fps == ((((CVI_DOUBLE)capability.support_video_format[mcode_temp].fresh_rate) / 1000)))) {
					pixel_clk = capability.support_video_format[mcode_temp].timing_info.pixel_clk;
					mcode = capability.support_video_format[mcode_temp].mcode;
					break;
			}
			mcode_temp++;
		}

		if(!capability.support_video_format[mcode_temp].mcode) {
			printf("Output resolution is not supported, %dx%dp@%0.2f\n", width, height, fps);
			return CVI_FAILURE;
		}

		printf("######### output %dx%dp@%0.2f, mcode:%d, pixel_clk:%d\n", width, height, fps, mcode, pixel_clk);
	}

	if (pattern)
		system("devmem 0x67005094 32 0x701000a");
	else
		system("devmem 0x67005094 32 0x7010008");

	setAttr.hdmi_en = true;
	setAttr.audio_en = audio_en;
	setAttr.bit_depth = CVI_HDMI_BIT_DEPTH_24;
	setAttr.video_format = mcode;
	setAttr.pix_clk = pixel_clk;
	setAttr.hdmi_force_output = force_output;
	setAttr.deep_color_mode = CVI_HDMI_DEEP_COLOR_24BIT;
	setAttr.sample_rate = CVI_HDMI_SAMPLE_RATE_44K;
	setAttr.hdcp14_en = hdcp14_en;
	if(csc_en){
		setAttr.hdmi_video_input = fmt_in;
		setAttr.hdmi_video_output = fmt_out;
	} else {
		setAttr.hdmi_video_input = 0;
		setAttr.hdmi_video_output = 0;
	}

	s32Ret = CVI_HDMI_Init();
	if(s32Ret){
		printf("HDMI init error\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_HDMI_SetAttr(&setAttr);
	if(s32Ret){
		printf("HDMI set attr error\n");
		return CVI_FAILURE;
	}

	if(set_infoframe){
		CVI_HDMI_INFOFRAME infoframe;
		memset(&infoframe, 0, sizeof(infoframe));

		infoframe.infoframe_unit.avi_infoframe.pixel_repetition = pixel_repeat;
		infoframe.infoframe_unit.avi_infoframe.color_space = fmt_out;
		infoframe.infoframe_unit.avi_infoframe.timing_mode = mcode;
		infoframe.infoframe_unit.avi_infoframe.colorimetry = CVI_HDMI_COMMON_COLORIMETRY_ITU709;
		infoframe.infoframe_unit.avi_infoframe.rgb_quant = CVI_HDMI_RGB_QUANT_DEFAULT_RANGE;
		/*audio*/
		infoframe.infoframe_unit.audio_infoframe.chn_alloc = 0x0;
		infoframe.infoframe_unit.audio_infoframe.coding_type = CVI_HDMI_AUDIO_CODING_PCM;
		infoframe.infoframe_unit.audio_infoframe.sample_size = CVI_HDMI_AUDIO_SAMPLE_SIZE_24;
		infoframe.infoframe_unit.audio_infoframe.sampling_freq = CVI_HDMI_AUDIO_SAMPLE_FREQ_44100;

		s32Ret = CVI_HDMI_SetInfoFrame(&infoframe);
		if(s32Ret){
			printf("HDMI Set InfoFrame error\n");
			return CVI_FAILURE;
		}
	}

	s32Ret = CVI_HDMI_Start();
	if(s32Ret){
		printf("HDMI start error\n");
		return CVI_FAILURE;
	}

	s32Ret = Get_Attr();
	if(s32Ret){
		printf("HDMI Get Attr error\n");
		return CVI_FAILURE;
	}

	if(set_infoframe){
		s32Ret = Get_InfoFrame();
		if(s32Ret){
		printf("HDMI Get Attr error\n");
		return CVI_FAILURE;
		}
	}

	if(avmute){
		CVI_BOOL avmute_en = 1;
		s32Ret = CVI_HDMI_SetAvmute(&avmute_en);
		if(s32Ret){
			printf("HDMI Set Avmute error\n");
			return CVI_FAILURE;
		}
	}

	if(audio_mute){
		CVI_BOOL audio_mute_en = 1;
		s32Ret = CVI_HDMI_SetAudioMute(&audio_mute_en);
		if(s32Ret){
			printf("HDMI Set Audio Mute error\n");
			return CVI_FAILURE;
		}
	}

	callback_func.hdmi_event_callback = Hdmi_EventProc;
	callback_func.private_data = &setAttr;
	s32Ret = CVI_HDMI_RegisterCallback(&callback_func);
	if(s32Ret){
		printf("HDMI RegisterCallback error\n");
		return CVI_FAILURE;
	}
	sleep(1);
	g_audio_send_started = 0;
	if (audio_en) {
		if (pthread_create(&g_audio_send_tid, NULL, audio_send_thread, filename) == 0) {
			g_audio_send_started = 1;
			printf("HDMI audio send thread started (one frame, then next when done)\n");
		} else {
			printf("HDMI audio send thread create error\n");
		}
	}
	/* When audio enabled: send one frame at init, then send next when previous duration elapsed */
	sleep(15);/* wait for audio playback to start */
	if(exit_flag){
		printf("\n");

		if (audio_en && g_audio_send_started) {
			g_audio_send_stop = 1;
			pthread_join(g_audio_send_tid, NULL);
			g_audio_send_started = 0;
		}

		s32Ret = CVI_HDMI_UnRegisterCallback(&callback_func);
		if(s32Ret){
			printf("HDMI UnRegisterCallback error\n");
			return CVI_FAILURE;
		}

		s32Ret = CVI_HDMI_Stop();
		if (s32Ret != CVI_SUCCESS)
			SAMPLE_HDMI_PRT("HDMI STOP abnormally!\n");

		s32Ret = CVI_HDMI_DeInit();
		if (s32Ret != CVI_SUCCESS)
			SAMPLE_HDMI_PRT("HDMI DeInit abnormally!\n");

		s32Ret = CVI_SYS_Exit();
		if (s32Ret != CVI_SUCCESS)
			SAMPLE_HDMI_PRT("CVI_SYS_Exit error!\n");

		SAMPLE_HDMI_PRT("SAMPLE HDMI EXIT SUCCESS !\n");

		return s32Ret;
	}
}
