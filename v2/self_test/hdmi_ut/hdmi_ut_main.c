#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <inttypes.h>

#include "cvi_hdmi.h"
#include "cvi_sys.h"
#include "vo_ioctl.h"
#include "sample_comm.h"

#define CSC_RGB888  0
#define CSC_YUV444  1
#define CSC_YUV422  2
#define MCODE_1080P 16
#define PIXEL_CLK_1080P 148500
#define MCODE_4k    97
#define PIXEL_CLK_4K    594000
#define PCM         1
#ifndef UNUSED
#define UNUSED(x) ((x) = (x))
#endif

#define NONE	"\033[m"
#define RED	    "\033[0;32;31m"
#define GREEN	"\033[0;32;32m"

int hdmi_fd = -1;
int vo_fd  = 1;

int Video_Mcode = 0;
int Pixel_Clk = 0;

typedef struct _HDMI_UT_FILE {
	CVI_U32 SimpleSize;
	CVI_U32 SimpleFreq;
	CVI_U8  Channel_Alloc;
	int  channel;
	char filename[30];
} HDMI_AUDIO_FILE;

HDMI_AUDIO_FILE hdmi_audio_file[] = {
	{.SimpleSize = 24, .SimpleFreq = 44100,    .Channel_Alloc = 0x0,   .channel = 2, .filename = "res/44.1k_2ch_24bit.raw"},
	{.SimpleSize = 24, .SimpleFreq = 192000,   .Channel_Alloc = 0x7f,  .channel = 8, .filename = "res/192k_8ch_24bit.raw"},
	{.SimpleSize = 0,  .SimpleFreq = 0,        .Channel_Alloc = 0,     .channel = 0, .filename = " "},
};

static CVI_S32 _hdmi_close_device(void)
{
	if (hdmi_fd == -1)
		return CVI_SUCCESS;

	if (-1 == close(hdmi_fd)) {
		fprintf(stderr, "%s: fd(%d) failure\n", __func__, hdmi_fd);
		return -1;
	}

	hdmi_fd = -1;

	return CVI_SUCCESS;
}

CVI_S32 AUDIO_MAP(CVI_HDMI_ATTR* attr, CVI_CHAR * filename, CVI_U64* pu64PhyAddr,
				  CVI_VOID **ppVirAddr, const CVI_CHAR* strName)
{
	FILE *fd;
	CVI_U32 u32Len;
	struct stat statbuf;

	fd=fopen(filename, "rb+");
	if(fd==NULL){
		perror("open");
		return 1;
	}

	stat(filename, &statbuf);
	u32Len = statbuf.st_size;
	SAMPLE_PRT("Audio File Length: %d\n", u32Len);

	CVI_SYS_IonAlloc(pu64PhyAddr, ppVirAddr, strName, u32Len);

	char *p = (char *)malloc(u32Len+1);
	if(p == NULL)
	{
		fclose(fd);
		return 0;
	}

	fread(p, u32Len, 1, fd);
	memcpy(*ppVirAddr, p, u32Len);

	attr->audio_start_paddr = *pu64PhyAddr;
	attr->audio_stop_paddr = *pu64PhyAddr + u32Len;
	SAMPLE_PRT("Start_addr:0x%lx, Stop_addr:0x%lx\n",attr->audio_start_paddr, attr->audio_stop_paddr);
	SAMPLE_PRT("vaddr:%p\n", *ppVirAddr);
    free(p);
	return 0;
}

CVI_VOID Hdmi_EventProc(CVI_HDMI_EVENT_TYPE event, CVI_VOID *private_data)
{
	printf("Private_data Content:%s\n", (CVI_CHAR*)private_data);

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

static CVI_S32 _hdmi_video_output(CVI_HDMI_ATTR* setAttr)
{
	CVI_S32 s32Ret;

	s32Ret =  CVI_HDMI_Init();
	if(s32Ret){
		SAMPLE_PRT("HDMI init error with %#x\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = CVI_HDMI_SetAttr(setAttr);
	if(s32Ret){
		SAMPLE_PRT("HDMI set attr error with %#x\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret =  CVI_HDMI_Start();
	if(s32Ret){
		SAMPLE_PRT("HDMI start error with %#x\n", s32Ret);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 hdmi_ut_stop(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_HDMI_Stop();
	if (s32Ret) {
		SAMPLE_PRT("HDMI Stop failed with %#x\n", s32Ret);
		return CVI_FAILURE;
	}

	return s32Ret;
}

static CVI_S32 _hdmi_ut_handle_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_HDMI_CALLBACK_FUNC callback_func;

	switch (op) {
	case 1:
	{
		CVI_HDMI_ATTR setAttr;
		memset(&setAttr, 0, sizeof(setAttr));
		printf("Mcode: ");
		scanf("%d", &Video_Mcode);
		getchar();
		printf("Pixel_Clk: ");
		scanf("%d", &Pixel_Clk);
		getchar();

		if(Video_Mcode == 0) {
			printf("Video_Mcode should not be zero\n");
			break;
		}
		if(Pixel_Clk > 594000 || Pixel_Clk <= 0) {
			printf("Invalid Pixel_Clk Value\n");
			break;
		}

		setAttr.hdmi_en = true;
		setAttr.audio_en = false;
		setAttr.hdcp14_en = false;
		setAttr.video_format = Video_Mcode;
		setAttr.pix_clk = Pixel_Clk;
		setAttr.hdmi_force_output = CVI_HDMI_FORCE_NULL;
		setAttr.hdmi_video_input = CSC_RGB888;
		setAttr.hdmi_video_output = CSC_RGB888;
		setAttr.bit_depth = CVI_HDMI_BIT_DEPTH_24;
		setAttr.deep_color_mode = CVI_HDMI_DEEP_COLOR_24BIT;

		s32Ret = _hdmi_video_output(&setAttr);
		if(s32Ret){
			SAMPLE_PRT("HDMI video output error with %#x\n", s32Ret);
			break;
		}

		break;
	}
	case 9: {
		CVI_HDMI_ATTR setAttr;
		memset(&setAttr, 0, sizeof(setAttr));

		setAttr.hdmi_en = true;
		setAttr.audio_en = false;
		setAttr.hdcp14_en = true;
		setAttr.video_format = MCODE_1080P;
		setAttr.pix_clk = PIXEL_CLK_1080P;
		setAttr.hdmi_force_output = CVI_HDMI_FORCE_NULL;
		setAttr.hdmi_video_input = CSC_RGB888;
		setAttr.hdmi_video_output = CSC_RGB888;
		setAttr.bit_depth = CVI_HDMI_BIT_DEPTH_24;
		setAttr.deep_color_mode = CVI_HDMI_DEEP_COLOR_24BIT;

		s32Ret = _hdmi_video_output(&setAttr);
		if(s32Ret){
			SAMPLE_PRT("HDMI video output error with %#x\n", s32Ret);
			break;
		}

		break;
	}

	case 10:
	case 11: {
		CVI_HDMI_ATTR setAttr;
		memset(&setAttr, 0, sizeof(setAttr));
		int csc_fmt_out = ((op == 10) ? CSC_YUV444 : CSC_YUV422);

		setAttr.hdmi_en = true;
		setAttr.audio_en = false;
		setAttr.hdcp14_en = false;
		setAttr.video_format = MCODE_1080P;
		setAttr.pix_clk = PIXEL_CLK_1080P;
		setAttr.hdmi_force_output = CVI_HDMI_FORCE_NULL;
		setAttr.hdmi_video_input = CSC_RGB888;
		setAttr.hdmi_video_output = csc_fmt_out;
		setAttr.bit_depth = CVI_HDMI_BIT_DEPTH_24;
		setAttr.deep_color_mode = CVI_HDMI_DEEP_COLOR_24BIT;

		s32Ret = _hdmi_video_output(&setAttr);
		if(s32Ret){
			SAMPLE_PRT("HDMI video output error with %#x\n", s32Ret);
			break;
		}

		break;
	}
	case 12: {
		CVI_HDMI_ATTR setAttr;
		CVI_CHAR * filename;
		CVI_U64 u64PhyAddr = 0;
	    CVI_VOID *pVirAddr;
	    CVI_CHAR strName[] = "hdmi_audio_2ch";
		memset(&setAttr, 0, sizeof(setAttr));

		setAttr.hdmi_en = true;
		setAttr.audio_en = true;
		setAttr.hdcp14_en = false;
		setAttr.video_format = MCODE_1080P;
		setAttr.pix_clk = PIXEL_CLK_1080P;
		setAttr.hdmi_force_output = CVI_HDMI_FORCE_NULL;
		setAttr.hdmi_video_input = CSC_RGB888;
		setAttr.hdmi_video_output = CSC_RGB888;
		setAttr.sample_rate = CVI_HDMI_SAMPLE_RATE_44K;
		setAttr.bit_depth = CVI_HDMI_BIT_DEPTH_24;
		setAttr.deep_color_mode = CVI_HDMI_DEEP_COLOR_24BIT;

		filename = hdmi_audio_file[0].filename;
		s32Ret = AUDIO_MAP(&setAttr, filename, &u64PhyAddr, &pVirAddr, strName);
		if(s32Ret){
			SAMPLE_PRT("HDMI AUDIO MAP error with %#x\n", s32Ret);
			goto audio_2ch_err;
		}

		s32Ret = _hdmi_video_output(&setAttr);
		if(s32Ret){
			SAMPLE_PRT("HDMI video output error with %#x\n", s32Ret);
			goto audio_2ch_err;
		}

		CVI_SYS_IonFree(u64PhyAddr, pVirAddr);
		break;

	audio_2ch_err:
		if(u64PhyAddr || pVirAddr) {
			CVI_SYS_IonFree(u64PhyAddr, pVirAddr);
			break;
		}

		break;
	}
	case 15: {
		CVI_HDMI_ATTR setAttr;
		CVI_HDMI_INFOFRAME infoframe;
		CVI_CHAR * filename = NULL;
		CVI_U64 u64PhyAddr = 0;
	    CVI_VOID *pVirAddr;
	    CVI_CHAR strName[] = "hdmi_audio_8ch";

		memset(&setAttr, 0, sizeof(setAttr));
		memset(&infoframe, 0, sizeof(infoframe));

		setAttr.hdmi_en = true;
		setAttr.audio_en = true;
		setAttr.hdcp14_en = false;
		setAttr.video_format = MCODE_4k;
		setAttr.pix_clk = PIXEL_CLK_4K;
		setAttr.hdmi_force_output = CVI_HDMI_FORCE_NULL;
		setAttr.hdmi_video_input = CSC_RGB888;
		setAttr.hdmi_video_output = CSC_RGB888;
		setAttr.sample_rate = CVI_HDMI_SAMPLE_RATE_192K;
		setAttr.bit_depth = CVI_HDMI_BIT_DEPTH_24;
		setAttr.deep_color_mode = CVI_HDMI_DEEP_COLOR_24BIT;

		infoframe.infoframe_unit.avi_infoframe.pixel_repetition = CVI_HDMI_PIXEL_REPET_NO;
		infoframe.infoframe_unit.avi_infoframe.color_space = CSC_RGB888;
		infoframe.infoframe_unit.avi_infoframe.timing_mode = MCODE_4k;
		infoframe.infoframe_unit.avi_infoframe.colorimetry = 0;
		infoframe.infoframe_unit.avi_infoframe.rgb_quant = 0;
		/*audio*/
		infoframe.infoframe_unit.audio_infoframe.chn_alloc = 0x1f;
		infoframe.infoframe_unit.audio_infoframe.coding_type = PCM;
		infoframe.infoframe_unit.audio_infoframe.sample_size = 24;
		infoframe.infoframe_unit.audio_infoframe.sampling_freq = 192000;

		filename = hdmi_audio_file[1].filename;
		s32Ret = AUDIO_MAP(&setAttr, filename, &u64PhyAddr, &pVirAddr, strName);
		if(s32Ret){
			SAMPLE_PRT("HDMI AUDIO MAP error with %#x\n", s32Ret);
			goto audio_8ch_err;
		}

		s32Ret =  CVI_HDMI_Init();
		if(s32Ret){
			SAMPLE_PRT("HDMI init error with %#x\n", s32Ret);
			goto audio_8ch_err;
		}

		s32Ret = CVI_HDMI_SetAttr(&setAttr);
		if(s32Ret){
			SAMPLE_PRT("HDMI set attr error with %#x\n", s32Ret);
			goto audio_8ch_err;
		}

		s32Ret = CVI_HDMI_SetInfoFrame(&infoframe);
		if(s32Ret){
			SAMPLE_PRT("HDMI set infoframe error with %#x\n", s32Ret);
			goto audio_8ch_err;
		}

		s32Ret =  CVI_HDMI_Start();
		if(s32Ret){
			SAMPLE_PRT("HDMI start error with %#x\n", s32Ret);
			goto audio_8ch_err;
		}

		CVI_SYS_IonFree(u64PhyAddr, pVirAddr);
		break;

	audio_8ch_err:
		if(u64PhyAddr || pVirAddr) {
			CVI_SYS_IonFree(u64PhyAddr, pVirAddr);
			break;
		}

		break;
	}

	case 30: {
		CVI_HDMI_EDID edid_data;
		memset(&edid_data, 0, sizeof(edid_data));

		s32Ret = CVI_HDMI_ForceGetEdid(&edid_data);
		if(s32Ret){
			SAMPLE_PRT("HDMI force get edid error with %#x\n", s32Ret);
			break;
		}

		if(edid_data.edid){
			printf("\n");
			printf("Edid Raw Data:\n");
			for(int i = 1; i <= 256; i++)
			{
				printf("%02X",edid_data.edid[i-1]);
				if(i%16 == 0)
				{
					printf("\n");
				}
			}
			printf("\n");
		}

		break;
	}

	case 50: {
		CVI_HDMI_ATTR setAttr, getAttr;
		memset(&setAttr, 0, sizeof(setAttr));
		memset(&getAttr, 0, sizeof(getAttr));

		printf("Mcode: ");
		scanf("%d", &Video_Mcode);
		getchar();
		printf("Pixel_Clk: ");
		scanf("%d", &Pixel_Clk);
		getchar();

		if(Video_Mcode == 0) {
			printf("Video_Mcode should not be zero\n");
			break;
		}
		if(Pixel_Clk > 594000 || Pixel_Clk <= 0) {
			printf("Invalid Pixel_Clk Value\n");
			break;
		}

		setAttr.hdmi_en = true;
		setAttr.audio_en = false;
		setAttr.hdcp14_en = false;
		setAttr.video_format = Video_Mcode;
		setAttr.bit_depth = CVI_HDMI_BIT_DEPTH_24;
		setAttr.deep_color_mode = CVI_HDMI_DEEP_COLOR_24BIT;
		setAttr.hdmi_video_input = CSC_RGB888;
		setAttr.hdmi_video_output = CSC_RGB888;
		setAttr.audio_start_paddr = 0;
		setAttr.audio_stop_paddr = 0;
		setAttr.sample_rate = CVI_HDMI_SAMPLE_RATE_44K;
		setAttr.pix_clk = Pixel_Clk;
		setAttr.hdmi_force_output = CVI_HDMI_FORCE_NULL;

		s32Ret = _hdmi_video_output(&setAttr);
		if(s32Ret){
			SAMPLE_PRT("HDMI video output error with %#x\n", s32Ret);
			break;
		}

		s32Ret = CVI_HDMI_GetAttr(&getAttr);
		if(s32Ret){
			SAMPLE_PRT("HDMI get attr error with %#x\n", s32Ret);
			break;
		}

		if (memcmp(&setAttr, &getAttr, sizeof(CVI_HDMI_ATTR))) {
			s32Ret = CVI_FAILURE;
			SAMPLE_PRT("hdmi attribute compare fail!\n");
			break;
		}

		break;
	}

	case 60: {
		int i, mcode = 0;

		CVI_HDMI_SINK_CAPABILITY capability;
		memset(&capability, 0, sizeof(capability));

		s32Ret = CVI_HDMI_GetSinkCapability(&capability);
		if(s32Ret){
			SAMPLE_PRT("HDMI get sink capability error\n");
			break;
		}

		printf("-- HDMI Dev info --\n");
		printf("\n");
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
		printf("sink's  audio info number : %d\n", capability.audio_info_num);
		printf("sink's max audio channels : %d\n", capability.audio_info[0].audio_chn);
		printf("supported max sample rate by sink:%uhz\n", capability.audio_info[0].max_bit_rate);
		printf("supported sample rate by sink:\n");

		for(i = 0; i < (int)(capability.audio_info[0].support_sample_rate_num); i++){
			printf("  %uhz\n", capability.audio_info[0].support_sample_rate[i]);
		}

		printf("supported max bit depth by sink:%ubit\n", capability.audio_info[0].support_bit_depth[0]);
		printf("supported bit depth by sink:\n");
		for(i = 0; i < (int)(capability.audio_info[0].support_bit_depth_num); i++){
			printf("  %ubit\n", capability.audio_info[0].support_bit_depth[i]);
		}

		printf("sink's  video latency : %d\n", capability.video_latency);
		printf("sink's  audio latency : %d\n", capability.audio_latency);
		printf("sink's  interlaced video latency : %d\n", capability.interlaced_video_latency);
		printf("sink's  interlaced audio latency : %d\n", capability.interlaced_audio_latency);
		printf("-- End --\n");

		break;
	}

	case 70: {
		CVI_HDMI_ATTR setAttr;
		CVI_HDMI_INFOFRAME infoframe;
		CVI_HDMI_INFOFRAME GetInfoFrame;
		CVI_CHAR * filename = NULL;
		CVI_U64 u64PhyAddr = 0;
	    CVI_VOID *pVirAddr;
	    CVI_CHAR strName[] = "hdmi_audio_8ch";

		memset(&setAttr, 0, sizeof(setAttr));
		memset(&infoframe, 0, sizeof(infoframe));
		memset(&GetInfoFrame, 0, sizeof(GetInfoFrame));

		printf("Video_Mcode: ");
		scanf("%d", &Video_Mcode);
		getchar();
		printf("Pixel_Clk: ");
		scanf("%d", &Pixel_Clk);
		getchar();

		if(Video_Mcode == 0) {
			printf("Video_Mcode should not be zero\n");
			break;
		}
		if(Pixel_Clk > 594000 || Pixel_Clk <= 0) {
			printf("Invalid Pixel_Clk Value\n");
			break;
		}

		setAttr.hdmi_en = true;
		setAttr.audio_en = true;
		setAttr.hdcp14_en = false;
		setAttr.video_format = Video_Mcode;
		setAttr.pix_clk = Pixel_Clk;
		setAttr.hdmi_force_output = CVI_HDMI_FORCE_NULL;

		setAttr.sample_rate = CVI_HDMI_SAMPLE_RATE_192K;
		setAttr.bit_depth = CVI_HDMI_BIT_DEPTH_24;
		setAttr.deep_color_mode = CVI_HDMI_DEEP_COLOR_24BIT;
		setAttr.hdmi_video_input = CSC_RGB888;
		setAttr.hdmi_video_output = CSC_RGB888;

		infoframe.infoframe_unit.avi_infoframe.pixel_repetition = CVI_HDMI_PIXEL_REPET_NO;
		infoframe.infoframe_unit.avi_infoframe.color_space = CSC_RGB888;
		infoframe.infoframe_unit.avi_infoframe.timing_mode = Video_Mcode;
		infoframe.infoframe_unit.avi_infoframe.colorimetry = CVI_HDMI_COMMON_COLORIMETRY_ITU709;
		infoframe.infoframe_unit.avi_infoframe.rgb_quant = CVI_HDMI_RGB_QUANT_DEFAULT_RANGE;
		/*audio*/
		infoframe.infoframe_unit.audio_infoframe.chn_alloc = 0x1f;
		infoframe.infoframe_unit.audio_infoframe.coding_type = PCM;
		infoframe.infoframe_unit.audio_infoframe.sample_size = CVI_HDMI_AUDIO_SAMPLE_SIZE_24;
		infoframe.infoframe_unit.audio_infoframe.sampling_freq = CVI_HDMI_AUDIO_SAMPLE_FREQ_192000;

		filename = hdmi_audio_file[1].filename;
		s32Ret = AUDIO_MAP(&setAttr, filename, &u64PhyAddr, &pVirAddr, strName);
		if(s32Ret){
			SAMPLE_PRT("HDMI AUDIO MAP error with %#x\n", s32Ret);
			goto set_infoframe_err;
		}

		s32Ret =  CVI_HDMI_Init();
		if(s32Ret){
			SAMPLE_PRT("HDMI init error with %#x\n", s32Ret);
			goto set_infoframe_err;
		}

		s32Ret = CVI_HDMI_SetAttr(&setAttr);
		if(s32Ret){
			SAMPLE_PRT("HDMI set attr error with %#x\n", s32Ret);
			goto set_infoframe_err;
		}

		s32Ret = CVI_HDMI_SetInfoFrame(&infoframe);
		if(s32Ret){
			SAMPLE_PRT("HDMI set infoframe error with %#x\n", s32Ret);
			goto set_infoframe_err;
		}

		s32Ret =  CVI_HDMI_Start();
		if(s32Ret){
			SAMPLE_PRT("HDMI start error with %#x\n", s32Ret);
			goto set_infoframe_err;
		}

		s32Ret =  CVI_HDMI_GetInfoFrame(&GetInfoFrame);
		if(s32Ret){
			printf("HDMI get audio mute error with %#x\n", s32Ret);
			goto set_infoframe_err;
		}

		if (memcmp(&infoframe, &GetInfoFrame, sizeof(CVI_HDMI_INFOFRAME))) {
			s32Ret = CVI_FAILURE;
			SAMPLE_PRT("hdmi infoframe compare failed!\n");
			goto set_infoframe_err;
		}

		CVI_SYS_IonFree(u64PhyAddr, pVirAddr);

		break;

	set_infoframe_err:
		if(u64PhyAddr || pVirAddr) {
			CVI_SYS_IonFree(u64PhyAddr, pVirAddr);
			break;
		}

		break;
	}

	case 80: {
		CVI_BOOL avmute_en = 1;
		s32Ret = CVI_HDMI_SetAvmute(&avmute_en);
		if(s32Ret){
			printf("hdmi set avmute error\n");
			break;
		}

		break;
	}

	case 90: {
		CVI_BOOL audio_mute_en = 1;
		s32Ret =CVI_HDMI_SetAudioMute(&audio_mute_en);
		if(s32Ret){
			SAMPLE_PRT("hdmi set audio mute error with %#x\n", s32Ret);
			break;
		}

		break;
	}

	case 100: {
		char private_data[30] = "Test Private_Data";
		callback_func.hdmi_event_callback = Hdmi_EventProc;
		callback_func.private_data = private_data;
		s32Ret = CVI_HDMI_RegisterCallback(&callback_func);
		if(s32Ret){
			printf("HDMI RegisterCallback error\n");
			break;
		}

		break;
	}

	case 110: {
		s32Ret = CVI_HDMI_UnRegisterCallback(&callback_func);
		if(s32Ret){
			printf("HDMI UnRegisterCallback error\n");
			break;
		}

		break;
	}

	case 120: {
		CVI_HDMI_ATTR setAttr;
		CVI_HDMI_INFOFRAME infoframe;
		int Pixel_RepeatTime;

		memset(&setAttr, 0, sizeof(setAttr));
		memset(&infoframe, 0, sizeof(infoframe));

		printf("Video_Mcode: ");
		scanf("%d", &Video_Mcode);
		getchar();
		printf("Pixel_Clk: ");
		scanf("%d", &Pixel_Clk);
		getchar();
		printf("Pixel_RepeatTime: ");
		scanf("%d", &Pixel_RepeatTime);
		getchar();

		if(Video_Mcode == 0) {
			printf("Video_Mcode should not be zero\n");
			break;
		}
		if(Pixel_Clk > 594000 || Pixel_Clk <= 0) {
			printf("Invalid Pixel_Clk Value\n");
			break;
		}

		setAttr.hdmi_en = true;
		setAttr.audio_en = false;
		setAttr.hdcp14_en = false;
		setAttr.video_format = Video_Mcode;
		setAttr.pix_clk = Pixel_Clk;
		setAttr.hdmi_force_output = CVI_HDMI_FORCE_HDMI;
		setAttr.sample_rate = CVI_HDMI_SAMPLE_RATE_192K;
		setAttr.bit_depth = CVI_HDMI_BIT_DEPTH_24;
		setAttr.deep_color_mode = CVI_HDMI_DEEP_COLOR_24BIT;
		setAttr.hdmi_video_input = CSC_RGB888;
		setAttr.hdmi_video_output = CSC_RGB888;

		infoframe.infoframe_unit.avi_infoframe.pixel_repetition = Pixel_RepeatTime;
		infoframe.infoframe_unit.avi_infoframe.color_space = CSC_RGB888;
		infoframe.infoframe_unit.avi_infoframe.timing_mode = Video_Mcode;
		infoframe.infoframe_unit.avi_infoframe.colorimetry = CVI_HDMI_COMMON_COLORIMETRY_ITU709;
		infoframe.infoframe_unit.avi_infoframe.rgb_quant = CVI_HDMI_RGB_QUANT_DEFAULT_RANGE;

		s32Ret =  CVI_HDMI_Init();
		if(s32Ret){
			SAMPLE_PRT("HDMI init error with %#x\n", s32Ret);
			break;
		}

		s32Ret = CVI_HDMI_SetAttr(&setAttr);
		if(s32Ret){
			SAMPLE_PRT("HDMI set attr error with %#x\n", s32Ret);
			break;
		}

		s32Ret = CVI_HDMI_SetInfoFrame(&infoframe);
		if(s32Ret){
			SAMPLE_PRT("HDMI set infoframe error with %#x\n", s32Ret);
			break;
		}

		s32Ret =  CVI_HDMI_Start();
		if(s32Ret){
			SAMPLE_PRT("HDMI start error with %#x\n", s32Ret);
			break;
		}

		break;
	}

	case 130: {
		s32Ret = CVI_HDMI_Start();
		if (s32Ret) {
			SAMPLE_PRT("HDMI Start failed with %#x\n", s32Ret);
			break;
		}
		break;
	}

	case 140: {
		s32Ret = CVI_HDMI_Stop();
		if (s32Ret) {
			SAMPLE_PRT("HDMI Stop failed with %#x\n", s32Ret);
			break;
		}
		break;
	}

	case 150: {
		s32Ret = CVI_HDMI_DeInit();
		if (s32Ret) {
			SAMPLE_PRT("HDMI Deinit failed with %#x\n", s32Ret);
			break;
		}
		break;
	}

	default:
		break;
	}

	if (s32Ret == CVI_SUCCESS)
		printf(GREEN"\n=== case(%d) pass ===\n"NONE"\n", op);
	else
		printf(RED"\n=== case(%d) fail===\n"NONE"\n", op);

	return s32Ret;
}

int main(int argc, char *argv[])
{
	int op;
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Init failed\n");
		return CVI_FAILURE;
	}

	if (argc >= 2) {
		op = (CVI_S32)atoi(argv[1]);
		s32Ret = _hdmi_ut_handle_op(op);
		SAMPLE_PRT("HDMI ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	} else {
		do {
			SAMPLE_PRT("1:   HDMI test video output\n");
			SAMPLE_PRT("9:   HDMI test HDCP1.4 (1920x1080p-60 video output)\n");
			SAMPLE_PRT("10:  HDMI test CSC (RGB888 in YUV444 out)\n");
			SAMPLE_PRT("11:  HDMI test CSC (RGB888 in YUV422 out)\n");
			SAMPLE_PRT("12:  HDMI test audio: 1920x1080p-60 video output and 44.1Khz 24bit 2ch audio output\n");
			SAMPLE_PRT("15:  HDMI test audio: 3840x2160p-60 video output and 192Khz 24bit 2ch audio output\n");
			SAMPLE_PRT("30:  HDMI test force get edid \n");
			SAMPLE_PRT("50:  HDMI test Set/Get attrbute\n");
			SAMPLE_PRT("60:  HDMI test Get sink capability\n");
			SAMPLE_PRT("70:  HDMI test Set/Get infoframe\n");
			SAMPLE_PRT("80:  HDMI test Set avmute\n");
			SAMPLE_PRT("90:  HDMI test Set audio mute\n");
			SAMPLE_PRT("100: HDMI test Register CallBack function\n");
			SAMPLE_PRT("110: HDMI test UnRegister CallBack function\n");
			SAMPLE_PRT("120: HDMI test Pixel Repetition\n");
			SAMPLE_PRT("130: HDMI test Start\n");
			SAMPLE_PRT("140: HDMI test Stop\n");
			SAMPLE_PRT("150: HDMI test DeInit\n");
			SAMPLE_PRT("255: exit\n");
			scanf("%d", &op);
			s32Ret = _hdmi_ut_handle_op(op);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("op(%d) failed with %#x!\n", op, s32Ret);
				break;
			}
		} while (op != 255);
	}

	s32Ret = hdmi_ut_stop();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("hdmi_ut_sys_stop failed\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_SYS_Exit();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Exit failed\n");
		return CVI_FAILURE;
	}

	_hdmi_close_device();

	return 0;
}

