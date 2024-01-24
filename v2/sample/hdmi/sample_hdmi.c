#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <pthread.h>
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
#define MCODE_720P 69

CVI_S32 AUDIO_MAP(CVI_HDMI_ATTR* attr, CVI_CHAR * filename, CVI_U64* pu64PhyAddr,
				  CVI_VOID ** ppVirAddr, const CVI_CHAR* strName)
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

	fread(p,u32Len,1,fd);
	memcpy(*ppVirAddr, p, u32Len);

	attr->audio_start_vaddr = *pu64PhyAddr;
	attr->audio_stop_vaddr = *pu64PhyAddr + u32Len;
	SAMPLE_PRT("Start_addr:0x%lx, Stop_addr:0x%lx\n",attr->audio_start_vaddr, attr->audio_stop_vaddr);
	SAMPLE_PRT("vaddr:%p\n", *ppVirAddr);

    free(p);
	return 0;
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
			printf("%02X", edid_data.edid[i-1]);
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
	printf("sink's  audio info number : %d\n", capability.audio_info_num);
	printf("sink's max audio channels : %d\n", capability.audio_info[0].audio_chn);
	printf("supported max sample rate by sink:%uhz\n", capability.audio_info[0].max_bit_rate);
	printf("supported sample rate by sink:\n");

	for(i=0; i<(int)(capability.audio_info[0].support_sample_rate_num); i++){
		printf("  %uhz\n", capability.audio_info[0].support_sample_rate[i]);
	}

	printf("supported max bit depth by sink:%ubit\n", capability.audio_info[0].support_bit_depth[0]);
	printf("supported bit depth by sink:\n");
	for(i=0; i<(int)(capability.audio_info[0].support_bit_depth_num); i++){
		printf("  %ubit\n", capability.audio_info[0].support_bit_depth[i]);
	}

	printf("sink's  video latency : %d\n", capability.video_latency);
	printf("sink's  audio latency : %d\n", capability.audio_latency);
	printf("sink's  interlaced video latency : %d\n", capability.interlaced_video_latency);
	printf("sink's  interlaced audio latency : %d\n", capability.interlaced_audio_latency);
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
	printf("Usage : %s <mCode> <pixel_clk> <force_output> <pixel_repeat> <hdcp14_en> <csc_en> <audio_en> <csc_fmt_in> <csc_fmt_out> \
					   <avmute_en> <audio_mute_en> <set_infoframe> <audio_file>\n", sPrgNm);
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
	printf("Example : ./sample_hdmi 16 148500 0 0 0 0 0 0 0 0 0 0 ./audio.file \n");
	printf("          (test pixel repeat should set set_infoframe = 1) \n");
}

CVI_S32 main(CVI_S32 argc, CVI_CHAR *argv[])
{
	CVI_S32 s32Ret = CVI_FAILURE;
	CVI_HDMI_ATTR setAttr;
	CVI_HDMI_CALLBACK_FUNC callback_func;
	CVI_U64 u64PhyAddr = 0;
	CVI_VOID *pVirAddr;
	CVI_CHAR strName[] = "hdmi_sample";
	CVI_S32 mcode, pixel_clk, hdcp14_en, csc_en, audio_en, fmt_in, fmt_out, avmute,
			audio_mute, set_infoframe, force_output, pixel_repeat;

	if (argc < 13) {
		SAMPLE_HDMI_Usage(argv[0]);
		return CVI_FAILURE;
	}

	if (!strncmp(argv[1], "-h", 2)) {
		SAMPLE_HDMI_Usage(argv[0]);
		return CVI_SUCCESS;
	}

	mcode = atoi(argv[1]);
	pixel_clk = atoi(argv[2]);
	force_output = atoi(argv[3]);
	pixel_repeat = atoi(argv[4]);
	hdcp14_en = atoi(argv[5]);
	csc_en = atoi(argv[6]);
	audio_en = atoi(argv[7]);
	fmt_in = atoi(argv[8]);
	fmt_out = atoi(argv[9]);
	avmute = atoi(argv[10]);
	audio_mute = atoi(argv[11]);
	set_infoframe = atoi(argv[12]);

	memset(&setAttr, 0, sizeof(setAttr));

	s32Ret = CVI_SYS_Init();
	if(s32Ret){
		printf("CVI_SYS_Init error\n");
		return CVI_FAILURE;
	}

	if(audio_en){
		CVI_CHAR* filename;
		filename = argv[13];
		if(!filename) {
			printf("audio file is not set\n");
			return CVI_FAILURE;
		}
		int ret = AUDIO_MAP(&setAttr, filename, &u64PhyAddr, &pVirAddr, strName);
		if(ret){
			printf("AUDIO MAP error\n");
			goto audio_err;
		}
	}

	if(!mcode || !pixel_clk)
	{
		printf("mcode or pixel_clk should be no-zero\n");
		return CVI_FAILURE;
	}

	setAttr.hdmi_en = true;
	setAttr.audio_en = audio_en;
	setAttr.bit_depth = CVI_HDMI_BIT_DEPTH_24;
	setAttr.video_format = mcode;
	setAttr.pix_clk = pixel_clk;
	setAttr.hdmi_force_output = force_output;
	setAttr.deep_color_mode = CVI_HDMI_DEEP_COLOR_8BIT;
	setAttr.sample_rate = CVI_HDMI_SAMPLE_RATE_44K;
	setAttr.hdcp14_en = hdcp14_en;
	if(csc_en){
		setAttr.hdmi_video_input = fmt_in;
		setAttr.hdmi_video_output = fmt_out;
	} else {
		setAttr.hdmi_video_input = 0;
		setAttr.hdmi_video_output = 0;
	}

	s32Ret =  CVI_HDMI_Init();
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

	s32Ret =  CVI_HDMI_Start();
	if(s32Ret){
		printf("HDMI start error\n");
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

	printf("\n");
	printf("-- Press Enter to Exit --\n");
	getchar();

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

	CVI_SYS_IonFree(u64PhyAddr, pVirAddr);

	s32Ret = CVI_SYS_Exit();
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_HDMI_PRT("CVI_SYS_Exit error!\n");

	SAMPLE_HDMI_PRT("SAMPLE HDMI EXIT SUCCESS !\n");

	return s32Ret;

audio_err:
	if(u64PhyAddr || pVirAddr) {
		CVI_SYS_IonFree(u64PhyAddr, pVirAddr);
		CVI_SYS_Exit();
		return CVI_FAILURE;
	}
	return CVI_FAILURE;
}

