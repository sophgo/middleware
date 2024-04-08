// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "app/cmd_line/video_cmd.h"

#include <hdmitx_dev.h>

#include "includes.h"
#include "hdmitx_ipk_api/hdmitx_ipk_api.h"
#include "hdmi_tx_app.h"
#include "platform.h"
#include "util/general_ops.h"
#include "app/edid/edid.h"
#include "app/video/video.h"
#include "app/cmd_line/cmd_interface.h"
#include "core/packets.h"
#include "core/video_params.h"
#include "app/hdmitx.h"
#include "ipc/ipc_msg.h"

#define YESNO(var) (var ? "Yes": "No")
#define MARK(var) (var ? "*": "")

struct sync_info_s{
	u32 vid_hsa_pixels;
	u32 vid_hbp_pixels;
	u32 vid_hfp_pixels;
	u32 vid_hline_pixels;
	u32 vid_vsa_lines;
	u32 vid_vbp_lines;
	u32 vid_vfp_lines;
	u32 vid_active_lines;
	bool vid_vsa_pos_polarity;
	bool vid_hsa_pos_polarity;
};

void mode_help(void)
{
	printf("Mode usage: m <hdmi|dvi>\n");
}

void mode_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	struct hdmi_mode * user_cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_INFO, "Improper arguments");
		return;
	}

	user_cfg = app_get_user_config(app);

	if(user_cfg == NULL){
		CMD_LOGGER(SNPS_ERROR, "Improper user_cfg pointer value");
		return;
	}

	if(cmd->params == NULL){
		CMD_LOGGER(SNPS_INFO, "Invalid command parameters");
		mode_help();
		return;
	}

	if(command_match(cmd->params, "HDMI")){
		user_cfg->pVideo.mHdmi = HDMI;
	}
	else if(command_match(cmd->params, "DVI")){
		user_cfg->pVideo.mHdmi = DVI;
	}
	else{
		CMD_LOGGER(SNPS_INFO, "Mode [%s] is not supported", cmd->params);
		mode_help();
	}
	printf("user_cfg->pVideo.mHdmi:%d\n",user_cfg->pVideo.mHdmi);
	CMD_LOGGER(SNPS_INFO, "Mode set to %s", user_cfg->pVideo.mHdmi == HDMI ? "HDMI" : "DVI");
}


void svd_help(void){
	printf("SVD usage: svd <1-107>  [frequency (in MHz)] \n");
}

void svd_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd){
	uint32_t code = 0;
	double refresh_rate = 0.0;
	struct hdmi_mode * user_cfg = NULL;
	char outcode[150] = {0,0};
	dtd_t dtd;
	int def = 0, length = 0;
	struct sync_info_s sync_info;
	int ret = 0;

	char *code_token = NULL;
	char *refresh_rate_token = NULL;

	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_INFO, "Improper arguments");
		return;
	}

	user_cfg = app_get_user_config(app);

	if(user_cfg == NULL){
		CMD_LOGGER(SNPS_ERROR, "Improper user_cfg pointer value");
		return;
	}

	if(cmd->params == NULL){
		CMD_LOGGER(SNPS_INFO, "Invalid command parameters");
		svd_help();
		return;
	}

	code_token = strsep((char **)&cmd->params, " ");
	code = strtol(code_token, NULL, 10);
	if((code < 1) || (code > 107)){
		svd_help();
		return;
	}

	// if(edid_tx_supports_cea_code(code) == FALSE){
	// 	CMD_LOGGER(SNPS_ERROR, "CEA code [%d] is not supported by TX", code);
	// 	return;
	// }

	refresh_rate_token = strsep((char **)&cmd->params, " ");
	if(refresh_rate_token){
		refresh_rate = strtod(refresh_rate_token, NULL);
	} else {
		def = 1;//no refresh rate received from the user - using the default
	}


	if(dtd_fill(&app->hdmi_tx, &dtd, code, refresh_rate) == FALSE){

		return;
	}
	
	sync_info.vid_hsa_pixels = dtd.mHSyncPulseWidth;
	sync_info.vid_hbp_pixels = dtd.mHBlanking;
	sync_info.vid_hfp_pixels = dtd.mHSyncOffset;
	sync_info.vid_hline_pixels = dtd.mHActive;
	sync_info.vid_vsa_lines = dtd.mVSyncPulseWidth;
	sync_info.vid_vbp_lines = dtd.mVBlanking;
	sync_info.vid_vfp_lines = dtd.mVSyncOffset;
	sync_info.vid_active_lines = dtd.mVActive;
	sync_info.vid_vsa_pos_polarity = dtd.mVSyncPolarity;
	sync_info.vid_hsa_pos_polarity = dtd.mHSyncPolarity;

	LOGGER(SNPS_INFO,"Set disp pattern gen in driver.\n");
	ret = ioctl(app->hdmi_tx_driver, FB_HDMI_SET_DISP, &(sync_info));
	if(ret < 0){
		printf("IOCTL error [%d], could not set disp pattern gen\n", ret);
		return;
	}

	//workaround for supporting
	if((code == 93) ||
			(code == 94) ||
			(code == 95) ||
			(code == 98) ) {
		if(user_cfg->pVideo.mHdmiVideoFormat == HDMI_3D_FORMAT ){
			CMD_LOGGER(SNPS_INFO, "3D is not supported in Extended video modes, disabling 3d");
			user_cfg->pVideo.m3dStructure = UNDEFINED_3D;
			user_cfg->pVideo.m3dExtData = UNDEFINED_EXTDATA_3D;
		}
		user_cfg->pVideo.mHdmiVideoFormat = HDMI_EXT_RES_FORMAT;
		//user_cfg->pVideo.mHdmiVic = videoParams_GetHdmiVicCode(user_cfg->pVideo.mDtd.mCode);

	}
	else {
		if(user_cfg->pVideo.mHdmiVideoFormat == HDMI_EXT_RES_FORMAT ) //not change to normal format in 3d modes
			user_cfg->pVideo.mHdmiVideoFormat = HDMI_NORMAL_FORMAT;
	}

	memcpy(&user_cfg->pVideo.mDtd, &dtd, sizeof(dtd_t));

	length += sprintf(outcode, "CEA VIC=%3d:", user_cfg->pVideo.mDtd.mCode);

	if(user_cfg->pVideo.mDtd.mInterlaced)
		length += sprintf(outcode + length," %4dx%di", user_cfg->pVideo.mDtd.mHActive, user_cfg->pVideo.mDtd.mVActive*2);
	else
		length += sprintf(outcode + length," %4dx%dp", user_cfg->pVideo.mDtd.mHActive, user_cfg->pVideo.mDtd.mVActive);

	length += sprintf(outcode + length," @ %3.3fHz%s",dtd_get_refresh_rate(&dtd), def? "(default)": "");

	CMD_LOGGER(SNPS_INFO, "%s", outcode);
}

void video3d_cmd_help(void)
{
	printf("3D  [<None|FramePacking|TopBottom|SideBySideHalf>]\n");
}

void video3d_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd){
	struct hdmi_mode * user_cfg = NULL;
	sink_edid_t * sink_cap = NULL;
	//uint32_t code = 0;
	hdmi_3d_structure_t struct3d = -1;

	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_INFO, "Improper arguments");
		return;
	}

	user_cfg = app_get_user_config(app);

	if(user_cfg == NULL){
		CMD_LOGGER(SNPS_ERROR, "Improper user_cfg pointer value");
		return;
	}

	int j, i = 0;
	if(cmd == NULL || cmd->params == NULL){

		sink_cap = app->mode.sink_cap;
		//sink_cap = app->tx_sink_cap; //debug

		if(sink_cap == NULL){
			CMD_LOGGER(SNPS_ERROR, "Sink capabilities not defined");
			return;
		}

		if(sink_cap->edid_mHdmivsdb.m3dPresent == 0){
			CMD_LOGGER(SNPS_ERROR,"Sink doesn't support 3D.\n");
			return;
		}

		hdmivsdb_t * vsdb = &(sink_cap->edid_mHdmivsdb);
		shortVideoDesc_t * svdArray = sink_cap->edid_mSvd;


		CMD_LOGGER(SNPS_INFO,"3D modes supported:");
		for (i = 0; i < MAX_VIC_WITH_3D; i++) {
			for (j = 0; j < MAX_HDMI_3DSTRUCT; j++) {
				if(vsdb->mVideo3dStruct[i][j])
					CMD_LOGGER(SNPS_INFO,"\tSVD %3d 3D structure %s", svdArray[i].mCode, get_3d_structure_name(j));
			}
		}
		return;
	}

	if(command_match(cmd->params, "None")) {

		user_cfg->pVideo.mHdmiVideoFormat = HDMI_NORMAL_FORMAT;
		user_cfg->pVideo.m3dStructure = (u8) UNDEFINED_3D;
		user_cfg->pVideo.m3dExtData = UNDEFINED_EXTDATA_3D;
		CMD_LOGGER(SNPS_INFO, "3D mode set to %s", get_3d_structure_name(user_cfg->pVideo.m3dStructure));
		return;
	}

	sink_cap = app->mode.sink_cap;

	if(sink_cap == NULL){
		CMD_LOGGER(SNPS_ERROR, "Sink capabilities not defined");
		return;
	}

	if(sink_cap->edid_mHdmivsdb.m3dPresent == 0){
		CMD_LOGGER(SNPS_INFO,"Sink doesn't support 3D. Mode will not be applied.\n");
		return;
	}

	if(command_match(cmd->params, "FramePacking")){
		struct3d = FRAME_PACKING_3D;
	} else if(command_match(cmd->params, "TopBottom")){
		struct3d = TOP_AND_BOTTOM_3D;
	} else if(command_match(cmd->params, "SideBySideHalf")){
		struct3d = SIDE_BY_SIDE_3D;
	} else {
		CMD_LOGGER(SNPS_INFO,"Invalid 3d structure\n");
		video3d_cmd_help();
		return;
	}

	user_cfg->pVideo.mHdmiVideoFormat = HDMI_3D_FORMAT;
	user_cfg->pVideo.m3dStructure = (u8) struct3d;
	user_cfg->pVideo.m3dExtData = UNDEFINED_EXTDATA_3D;

	//printf("Config with VIC %d 3D struct %d\n", svdArray[i].mCode, user_cfg->pVideo.m3dStructure);

	/*
	for (i = 0; i < MAX_VIC_WITH_3D; i++) {
		if(svdArray[i].mCode == code){

			user_cfg->pVideo.mHdmiVideoFormat = HDMI_3D_FORMAT;
			user_cfg->pVideo.m3dStructure = (u8) struct3d;
			user_cfg->pVideo.m3dExtData = 0;

			printf("Config with VIC %d 3D struct %d\n", svdArray[i].mCode, user_cfg->pVideo.m3dStructure);
			return;
		}
	}*/

	CMD_LOGGER(SNPS_INFO, "3D mode set to %s", get_3d_structure_name(user_cfg->pVideo.m3dStructure));
	return;
}


void color_resolution_help(void)
{
	printf("ColorDepth <8|10|12|16> - number of bits per color component\n");
}

void color_resolution_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	uint32_t color_res = 0;
	struct hdmi_mode * user_cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_INFO, "Improper arguments");
		return;
	}

	user_cfg = app_get_user_config(app);

	if(user_cfg == NULL){
		CMD_LOGGER(SNPS_ERROR, "Improper user_cfg pointer value");
		return;
	}

	if(cmd->params == NULL){
		CMD_LOGGER(SNPS_INFO, "Invalid command parameters");
		color_resolution_help();
		return;
	}

	color_res = strtol(cmd->params, NULL, 10);

	/* Limited to TMDS Clock 297 Mhz and no Deep Color support in YCC420 */
	if((user_cfg->pVideo.mDtd.mPixelClock > 297.00) || (user_cfg->pVideo.mEncodingIn == YCC420)){
		CMD_LOGGER(SNPS_WARN, "Limited color depth to 8-bpp (current mode does not support higher)");
		user_cfg->pVideo.mColorResolution = COLOR_DEPTH_8;
	}
	else{
		switch(color_res){
		case COLOR_DEPTH_8:
			user_cfg->pVideo.mColorResolution = COLOR_DEPTH_8;
			break;
		case COLOR_DEPTH_10:
			user_cfg->pVideo.mColorResolution = COLOR_DEPTH_10;
			break;
		case COLOR_DEPTH_12:
			user_cfg->pVideo.mColorResolution = COLOR_DEPTH_12;
			break;
		case COLOR_DEPTH_16:
			user_cfg->pVideo.mColorResolution = COLOR_DEPTH_16;
			break;
		default:
			CMD_LOGGER(SNPS_ERROR, "Color depth [%d bits/color] is not supported", color_res);
			color_resolution_help();
			return;
		}
	}
	CMD_LOGGER(SNPS_INFO, "Color Depth set to %d bits/color", color_res);
}


void encoding_help(void)
{
	printf("Encoding usage: e <rgb|ycc444|ycc422|ycc420>\n");
}

void encoding_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	struct hdmi_mode * user_cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_INFO, "Improper arguments");
		return;
	}

	user_cfg = app_get_user_config(app);

	if(user_cfg == NULL){
		CMD_LOGGER(SNPS_ERROR, "Improper user_cfg pointer value");
		return;
	}

	if(cmd->params == NULL){
		CMD_LOGGER(SNPS_INFO, "Invalid command parameters");
		encoding_help();
		return;
	}

	if(strcmp(cmd->params, "rgb") == 0){
		user_cfg->pVideo.mEncodingIn = RGB;
		user_cfg->pVideo.mEncodingOut = RGB;
	}
	else if(strcmp(cmd->params, "ycc444") == 0){
		user_cfg->pVideo.mEncodingIn = YCC444;
		user_cfg->pVideo.mEncodingOut = YCC444;
	}
	else if(strcmp(cmd->params, "ycc422") == 0){
		user_cfg->pVideo.mEncodingIn = YCC422;
		user_cfg->pVideo.mEncodingOut = YCC422;
	}
	else if(strcmp(cmd->params, "ycc420") == 0){
		user_cfg->pVideo.mEncodingIn = YCC420;
		user_cfg->pVideo.mEncodingOut = YCC420;
	}
	else{
		CMD_LOGGER(SNPS_ERROR, "Encoding [%s] not supported", cmd->params);
		encoding_help();
		return;
	}
	CMD_LOGGER(SNPS_INFO, "Pixel Encoding set to %s", getEncodingString(user_cfg->pVideo.mEncodingIn));
}


void sink_vic_help(void)
{

}

void sink_vic_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	int i = 0;
	int hdmi_vic = 0;
	sink_edid_t * sink_cap = NULL;

	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_INFO, "Improper arguments");
		return;
	}

	sink_cap = app->mode.sink_cap;

	if(sink_cap == NULL){
		CMD_LOGGER(SNPS_ERROR, "%s:sink_cap is NULL", __func__);
		return;
	}

	printf("HDMI Rx (Sink) is 2.0 = %s\n", sink_cap->edid_m20Sink? "Yes":"No");
	if(sink_cap->edid_m20Sink == 0) /* Sink is HDMI 1.4 */
	{
		for(i = 0; (i < 4) && (sink_cap->edid_mHdmivsdb.mHdmiVic[i] != 0); i++){
			dtd_t dtd;
			int hActive = 0;
			int vActive = 0;
			hdmi_vic = sink_cap->edid_mHdmivsdb.mHdmiVic[i];

			printf("  HDMI VIC=%3d: (VIC=%3d)", hdmi_vic, videoParams_GetCeaVicCode(hdmi_vic));
			if(dtd_fill(&app->hdmi_tx, &dtd, videoParams_GetCeaVicCode(hdmi_vic), 0) == FALSE){
				CMD_LOGGER(SNPS_ERROR, "Could not fill DTD with CEA VIC %d", videoParams_GetCeaVicCode(hdmi_vic));
				return;
			}
			hActive = dtd.mHActive;
			vActive = dtd.mVActive;

			if(dtd.mInterlaced){
				vActive *= 2;
			}

			printf("%dx%d%c\n", hActive, vActive, dtd.mInterlaced ? 'i' : 'p');
		}
	}

	for(i = 0; (i < 128) && (sink_cap->edid_mSvd[i].mCode != 0); i++){
		dtd_t dtd;
		int hActive = 0;
		int vActive = 0;
		int cea_mode = sink_cap->edid_mSvd[i].mCode;

		if(sink_cap->edid_mSvd[i].mNative){
			printf("* CEA VID=%3d: ", cea_mode);
		}
		else{
			printf("  CEA VID=%3d: ", cea_mode);
		}

		if(dtd_fill(&app->hdmi_tx, &dtd, cea_mode, 0) == FALSE){
			continue;
		}
		hActive = dtd.mHActive;
		vActive = dtd.mVActive;

		if(dtd.mInterlaced){
			vActive *= 2;
		}

		printf("%dx%d%c", hActive, vActive, dtd.mInterlaced ? 'i' : 'p');

		if(sink_cap->edid_m20Sink && sink_cap->edid_mSvd[i].mLimitedToYcc420)
			printf(" (Limited to YCbCr 4:2:0)");
		if(sink_cap->edid_m20Sink && sink_cap->edid_mSvd[i].mYcc420)
			printf(" (Supports YCbCr 4:2:0)");
		if(edid_tx_supports_cea_code(cea_mode) == FALSE){
			printf(" - not supported by TX");
		}
		printf("\n");
	}
}


void video_report_cmd_help(void)
{

}

void video_report_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	char * video_info = NULL, *token = NULL, *user_input, *aux;
	char * userComment = NULL;
	int testResult = 0;


	if(app == NULL){
		CMD_LOGGER(SNPS_INFO, "Improper arguments");
		return;
	}

	user_input = (char *) malloc(CMD_SIZE);
	aux = user_input;
	video_info = get_videoinfo(&app->mode.pVideo);
	CMD_LOGGER(SNPS_INFO,"\n*** Current video configurations ***");
	CMD_LOGGER(SNPS_INFO,"%s\n", video_info);
	do {
		CMD_LOGGER(SNPS_INFO,"The output image is correct? (Yes/No/Quit + [Comment]): ");
		user_input = aux;
		get_cmd_line(app,user_input);
		//scanf("%s", user_input);
		if(/*(user_input == NULL) || */(strlen(user_input) == 0)){
			CMD_LOGGER(SNPS_INFO,"No input received - try again\n");
			continue;
		}

		token = strsep(&user_input, " ");

		if(user_input != NULL){
			userComment = user_input;
		}

		if(command_match(token, "Yes")){
			testResult = 1;
			break;
		}

		if(command_match(token, "No")){
			testResult = 0;
			break;
		}

		if(command_match(token, "Quit")){
			CMD_LOGGER(SNPS_INFO,"Report canceled by User");
			free(aux);
			free(video_info);
			free(user_input);
			return;
		}

		CMD_LOGGER(SNPS_INFO,"Invalid Input - try again\n"); //Information invalid - try again

	} while (1);

	CMD_LOGGER(SNPS_INFO,"%s - %s : %s\n", video_info, testResult? "PASS":"FAIL", userComment != NULL? userComment : "");
	hdmitx_log(app->hdmi_tx_report, SNPS_REPORT, "%s - %s : %s", video_info, testResult? "PASS":"FAIL", userComment != NULL? userComment : "");
	free(aux);
	free(video_info);
	free(user_input);
}

void video_show_help(void){

}

void video_show_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd){
	struct hdmi_mode display_cfg;
	struct hdmi_mode * user_cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	memcpy(&display_cfg, &(app->mode), sizeof(struct hdmi_mode));

	user_cfg = app_get_user_config(app);

	update_video_cfg(&(user_cfg->pVideo), &(display_cfg.pVideo));

	if(user_cfg == NULL){
		CMD_LOGGER(SNPS_ERROR, "Improper user_cfg pointer value");
		return;
	}

	printf("\n*** Current video configurations ***\n");
	print_videoinfo(&app->mode.pVideo);

	printf("\n\n");
	printf("*** New video configurations - run Apply to make them active ***\n");
	print_videoinfo(&(display_cfg.pVideo));
	printf("\n");
}


void video_apply_help(void)
{

}

void video_apply_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	struct hdmi_mode * user_cfg = NULL;


	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	user_cfg = app_get_user_config(app);

	if(user_cfg == NULL){
		CMD_LOGGER(SNPS_ERROR, "Improper user_cfg pointer value");
		return;
	}

	pthread_mutex_lock(&(app->mutex));

	// Initial VGA configuration
	if(hdmitx_vga_init(app) != TRUE){
		LOGGER(SNPS_ERROR, "hdmitx_configure_vga failed");
	}

	update_video_cfg(&(user_cfg->pVideo), &(app->mode.pVideo));

	// Using YCC420 means that we are in HDMI 2.0
	if(app->mode.pVideo.mEncodingOut == YCC420){
		app->mode.pVideo.mHdmi20 = 1;
		//dev_write_mask(&app->hdmi_tx, (0x1032<<2), 0xff, 0);
		//dev_write_mask(&app->hdmi_tx, (0x1033<<2), 0xff, 0);
	}else
	{
		app->mode.pVideo.mHdmi20 = 0;
	}

	// video_bridge_write(app, VG_SWRSTZ, 0); //Stop video generator to reduce the interrupts for the ESM
	// audio_bridge_write(app, AG_SWRSTZ, 0); //Stop audio generator to reduce the interrupts for the ESM

	// if(app->pattern_load_enable == 1)
	// 	video_generator_set_pattern(app->mode.pVideo.mEncodingIn);

	printf("Changing video mode to:\n");
	print_videoinfo(&(app->mode.pVideo));

	if(app->mode.sink_cap != NULL){
		if (app->mode.sink_cap->edid_mHdmivsdb.mSupportsAi == 0){
			packets_StopSendIsrc1(&app->hdmi_tx);
			packets_StopSendIsrc2(&app->hdmi_tx);
			packets_StopSendAcp(&app->hdmi_tx);
		}
		else{
			packets_AudioContentProtection(&app->hdmi_tx,0,NULL,0,1);//Type 0 doesn't need any dependent fields 9.3.3 HDMI 1.4 spec
			packets_IsrcPackets(&app->hdmi_tx,0x01,NULL,0,1);//Configuring without packet info in ISRC
		}
	}
	printf("video_apply_cmd_1\n");
	// video_generator_config(app);
	// audio_generator_config(app);
	printf("video_apply_cmd_2\n");
	if (!api_Configure(&app->hdmi_tx, &app->mode.pVideo, &app->mode.pAudio,
			&app->mode.pProduct, &app->mode.pHdcp, app->hdmi_tx_phy)) {
		hdmitx_logger(SNPS_ERROR, "API configure", error_Get());
	}

	app->user_cfg = NULL;
	free(user_cfg);
	printf("video_apply_cmd_3\n");
	// ELLIPTIC Workaround - set internal hpd flag for the hdcp_tx to restart the authentication process
	// if (send_hdmi_status(STATUS_HDMI_HDCP_REAUTH_REQUEST) < 0)
	// 	hdmitx_logger(SNPS_ERROR, "FAIL to request HDCP HPD re authentication");
	printf("video_apply_cmd_4\n");
	pthread_mutex_unlock(&(app->mutex));
}

void printfc(FILE *fp, char *buff)
{
	printf("%s", buff);
	if (fp)
		fprintf(fp, "%s", buff);
}

void video_autotest_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	struct hdmi_mode user_cfg_bckp;
	struct hdmi_mode * user_cfg = NULL;
	shortVideoDesc_t * test_vic[128];
	encoding_t test_enc[4];
	uint32_t test_cd[4];
	uint16_t idx_vic;
	uint8_t idx_enc, idx_cd, vic_max, enc_max, cd_max;
	uint32_t code;
	double refresh_rate = 0.0;
	char report_name[32] = {0};
	char cmd_data[CMD_SIZE] = {0};
	bool exit;
	int command, verdict, cmd_idx;
	FILE * fp = NULL;
	sink_edid_t * sink;

	if ((app == NULL) || (cmd  == NULL)) {
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	if (app->mode.sink_cap == NULL) {
		CMD_LOGGER(SNPS_ERROR, "sink_cap is NULL");
		return;
	}
	sink = app->mode.sink_cap;

	user_cfg = app_get_user_config(app);
	if (user_cfg == NULL) {
		CMD_LOGGER(SNPS_ERROR, "Improper user_cfg pointer value");
		return;
	}

	// Fill user_cfg with current video settings
	user_cfg->pVideo.mHdmi = app->mode.pVideo.mHdmi;
	code = app->mode.pVideo.mDtd.mCode;
	if((code == 93) || (code == 94) || (code == 95) || (code == 98) ) {
		user_cfg->pVideo.mHdmiVideoFormat = HDMI_EXT_RES_FORMAT;
	} else {
		user_cfg->pVideo.mHdmiVideoFormat = HDMI_NORMAL_FORMAT;
	}
	user_cfg->pVideo.mEncodingIn = app->mode.pVideo.mEncodingIn;
	user_cfg->pVideo.mEncodingOut = app->mode.pVideo.mEncodingOut;
	user_cfg->pVideo.mColorResolution = app->mode.pVideo.mColorResolution;

	// Query report name
	printf("[Test Report Name]# ");
	exit = false;
	cmd_idx = 0;
	while (!exit) {
		command = getchar();
		switch (command) {
		case '\n':
			exit = true;
			break;
		case 0x8: // backspace
			cmd_data[cmd_idx] = 0;
			if (cmd_idx != 0)
				cmd_idx--;
			break;
		default:
			if ((cmd_idx) >= CMD_SIZE - 1)
				continue;
			cmd_data[cmd_idx++] = (char)command;
			break;
		}
	}
	if (cmd_data[0] != 0) {
		memcpy(report_name, cmd_data, cmd_idx);
		sprintf(cmd_data, "%s.txt", report_name);
		fp = fopen(cmd_data, "w");
	}

	// Backup current user config
	memcpy(&user_cfg_bckp, user_cfg, sizeof(struct hdmi_mode));

	// Get all supported Pixel Encoding formats
	test_enc[0] = RGB;
	enc_max = 1;
	if (sink->edid_mYcc444Support)
		test_enc[enc_max++] = YCC444;
	if (sink->edid_mYcc422Support)
		test_enc[enc_max++] = YCC422;
	test_enc[enc_max++] = YCC420;
	
	
	// Get all supported Color Depths
	test_cd[0] = COLOR_DEPTH_8;
	cd_max = 1;
	if (sink->edid_mHdmivsdb.mDeepColor30)
		test_cd[cd_max++] = COLOR_DEPTH_10;
	if (sink->edid_mHdmivsdb.mDeepColor36 || sink->edid_mYcc422Support)
		test_cd[cd_max++] = COLOR_DEPTH_12;
	if (sink->edid_mHdmivsdb.mDeepColor48)
		test_cd[cd_max++] = COLOR_DEPTH_16;

	// Get all supported VICs and print supported feature table
	vic_max = 0;
	printfc(fp, "From Rx EDID:\n");
	sprintf(cmd_data, "Name: %s\n", sink->edid_mMonitorName);
	printfc(fp, cmd_data);
	printfc(fp, "---------------------------------\n");
	printfc(fp, "| VIC | TX | Y444 | Y422 | Y420 |\n");
	printfc(fp, "---------------------------------\n");
	for(idx_vic = 0; (idx_vic < 128) && (sink->edid_mSvd[idx_vic].mCode != 0); idx_vic++) {
		// Check if current VIC is supported by Tx and add it to test list
		verdict = edid_tx_supports_cea_code(sink->edid_mSvd[idx_vic].mCode);
		if (verdict) //using exit variable (bool) to test tx support
			test_vic[vic_max++] = &sink->edid_mSvd[idx_vic];
		
		// Print table
		if (sink->edid_mSvd[idx_vic].mLimitedToYcc420) {
			sprintf(cmd_data, "| %3d | %2s | %4s | %4s | %4s |\n",
					sink->edid_mSvd[idx_vic].mCode,
					MARK(verdict), "", "", "*");
			printfc(fp, cmd_data);
		} else {
			sprintf(cmd_data, "| %3d | %2s | %4s | %4s | %4s |\n",
					sink->edid_mSvd[idx_vic].mCode,
					MARK(verdict),
					MARK(sink->edid_mYcc444Support),
					MARK(sink->edid_mYcc422Support),
					MARK(sink->edid_mSvd[idx_vic].mYcc420));
			printfc(fp, cmd_data);
		}
	}
	printfc(fp, "---------------------------------\n\n");
	printfc(fp, "---------------------------------\n");
	printfc(fp, "| Encoding | CD30 | CD36 | CD48 |\n");
	printfc(fp, "---------------------------------\n");
	sprintf(cmd_data, "| %8s | %4s | %4s | %4s |\n",
			"RGB",
			MARK(sink->edid_mHdmivsdb.mDeepColor30),
			MARK(sink->edid_mHdmivsdb.mDeepColor36),
			MARK(sink->edid_mHdmivsdb.mDeepColor48));
	printfc(fp, cmd_data);
	printfc(fp, "---------------------------------\n");
	sprintf(cmd_data, "| %8s | %4s | %4s | %4s |\n",
			"YCC444",
			sink->edid_mHdmivsdb.mDeepColorY444 ? MARK(sink->edid_mHdmivsdb.mDeepColor30) : "",
			sink->edid_mHdmivsdb.mDeepColorY444 ? MARK(sink->edid_mHdmivsdb.mDeepColor36) : "",
			sink->edid_mHdmivsdb.mDeepColorY444 ? MARK(sink->edid_mHdmivsdb.mDeepColor48) : "");
	printfc(fp, cmd_data);
	printfc(fp, "---------------------------------\n");
	sprintf(cmd_data, "| %8s | %4s | %4s | %4s |\n",
			"YCC420",
			MARK(sink->edid_mHdmiForumvsdb.mDC_30bit_420),
			MARK(sink->edid_mHdmiForumvsdb.mDC_36bit_420),
			MARK(sink->edid_mHdmiForumvsdb.mDC_48bit_420));
	printfc(fp, cmd_data);
	printfc(fp, "---------------------------------\n\n");
	
///////////////////////////////////////////////////////////////////////
	for(idx_vic = 0; idx_vic < vic_max; idx_vic++) {
		if (fp)
			fprintf(fp, "--------------------------------------------------------------\n");
		
		for(idx_enc = 0; idx_enc < enc_max; idx_enc++) {
			for(idx_cd = 0; idx_cd < cd_max; idx_cd++) {
				
				// Allocate new user config
				user_cfg = app_get_user_config(app);
				
				// Fill user config with each test config
				user_cfg->pVideo.mHdmi = HDMI;
				
				// Set VIC
				code = test_vic[idx_vic]->mCode;
				if ((code == 93) || (code == 94) || (code == 95) || (code == 98) ) {
					user_cfg->pVideo.mHdmiVideoFormat = HDMI_EXT_RES_FORMAT;
				} else {
					user_cfg->pVideo.mHdmiVideoFormat = HDMI_NORMAL_FORMAT;
				}
				
				if (dtd_fill(&app->hdmi_tx, &user_cfg->pVideo.mDtd, code, refresh_rate) == FALSE) {
					CMD_LOGGER(SNPS_ERROR, "Test: Failed to fill dtd structure!\n");
					continue;
				}
				
				// Set Pixel Encoding
				user_cfg->pVideo.mEncodingIn = test_enc[idx_enc];
				user_cfg->pVideo.mEncodingOut = test_enc[idx_enc];
				
				// Set Color Depth
				user_cfg->pVideo.mColorResolution = test_cd[idx_cd];
				
				
				// Check if current VIC is limmeted to YCC420
				if ((user_cfg->pVideo.mEncodingOut != YCC420) && (test_vic[idx_vic]->mLimitedToYcc420 == 1))
					continue;
				// Check if current VIC has support for YCC420
				if (user_cfg->pVideo.mEncodingOut == YCC420) {
					if ((test_vic[idx_vic]->mYcc420 == 0) && (test_vic[idx_vic]->mLimitedToYcc420 == 0)) {
						continue;
					} else {
						// Check if current VIC supports selected color depth
						if ((user_cfg->pVideo.mColorResolution == COLOR_DEPTH_10) && (sink->edid_mHdmiForumvsdb.mDC_30bit_420 == 0))
							continue;
						if ((user_cfg->pVideo.mColorResolution == COLOR_DEPTH_12) && (sink->edid_mHdmiForumvsdb.mDC_36bit_420 == 0))
							continue;
						if ((user_cfg->pVideo.mColorResolution == COLOR_DEPTH_16) && (sink->edid_mHdmiForumvsdb.mDC_48bit_420 == 0))
							continue;
					}
				}
				
				// Check if YCC444 has support for Color Depth
				if ((user_cfg->pVideo.mEncodingOut == YCC444) && (sink->edid_mHdmivsdb.mDeepColorY444 == 0) && (user_cfg->pVideo.mColorResolution != COLOR_DEPTH_8))
					continue;
				// Check if 12 bit was only added becasue of YCC422 support
				if ((user_cfg->pVideo.mEncodingOut != YCC422) && (sink->edid_mHdmivsdb.mDeepColor36 == 0) && (user_cfg->pVideo.mColorResolution == COLOR_DEPTH_12))
					continue;
				// Use YCC422 with CD12 only
				if ((user_cfg->pVideo.mEncodingOut == YCC422) && (user_cfg->pVideo.mColorResolution != COLOR_DEPTH_12))
					continue;
				
				
				// Limited to TMDS Clock 297 Mhz and no Deep Color support in YCC420
				if ((code == 96) || (code == 97) || (code == 106) || (code == 107)) {
					if ((user_cfg->pVideo.mEncodingOut != YCC420) || (user_cfg->pVideo.mColorResolution != COLOR_DEPTH_8))
						continue;
				} else if (user_cfg->pVideo.mDtd.mPixelClock == 297.00) {
					if ((user_cfg->pVideo.mEncodingOut != YCC420) && (user_cfg->pVideo.mColorResolution != COLOR_DEPTH_8))
						continue;
				}
				
				// Print output notification with current video settings
				printf("##################################################\n");
				
				// Apply current video setting
				video_apply_cmd(app, cmd);
				
				// Query the use for the test verdict
				do {
					printf("[(p) PASS | (f) FAIL | (r) REPEAT | (e) EXIT]# ");
					verdict = 0;
					exit = false;
					while (!exit) {
						command = getchar();
						switch (command) {
						case '\n':
							exit = true;
							break;
						case 'p':
						case 'f':
						case 'r':
						case 'e':
							verdict = command;
							break;
						default:
							break;
						}
					}
				} while ((verdict != 'p') && (verdict != 'f') &&
					  (verdict != 'r') && (verdict != 'e'));
				
				// Repeat current video settings
				if (verdict == 'r') {
					idx_cd--;
					continue;
				}
				
				// Abort
				if (verdict == 'e') {
					goto exit_fun;
				}
				
				if (verdict == 'f') {
					memset(cmd_data, 0, CMD_SIZE);
					cmd_idx = 0;
					exit = false;
					printf("[Fail cause]# ");
					while (!exit) {
						command = getchar();
						switch (command) {
						case '\n':
							cmd_data[cmd_idx] = '\0';
							exit = true;
							break;
						case 0x8: // backspace
							cmd_data[cmd_idx] = 0;
							if (cmd_idx != 0)
								cmd_idx--;
							break;
						default:
							if ((cmd_idx) >= CMD_SIZE - 1)
								continue;
							cmd_data[cmd_idx++] = (char)command;
							break;
						}
					}
				}
				
				// Print data to report
				if (fp) {
					if (verdict == 'p') {
						fprintf(fp, "VIC=%3d | Encoding=%9s | ColorDepth=%02d | Result: Passed\n",
								user_cfg->pVideo.mDtd.mCode, getEncodingString(user_cfg->pVideo.mEncodingIn),
								user_cfg->pVideo.mColorResolution);
					} else {
						fprintf(fp, "VIC=%3d | Encoding=%9s | ColorDepth=%2d | Result: Failed - %s\n",
								user_cfg->pVideo.mDtd.mCode, getEncodingString(user_cfg->pVideo.mEncodingIn),
								user_cfg->pVideo.mColorResolution, cmd_data);
					}
				}
			}
		}
	}
	
///////////////////////////////////////////////////////////////////////
exit_fun:
	if (fp) {
		fprintf(fp, "--------------------------------------------------------------\n");
		fclose(fp);
	}
	
	printf("#################### COMPLETE ####################\n");
	
	// Allocate new user config
	user_cfg = app_get_user_config(app);
	
	// Restore user config and apply it
	memcpy(user_cfg, &user_cfg_bckp, sizeof(struct hdmi_mode));
	video_apply_cmd(app, cmd);
}

void video_customtest_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	struct hdmi_mode user_cfg_bckp;
	struct hdmi_mode * user_cfg = NULL;
	shortVideoDesc_t test_vic[128];
	uint32_t rx_vic[128] = {0};
	uint32_t ycc420_vic[128] = {0};
	encoding_t test_enc[4];
	uint32_t test_cd[4];
	uint16_t idx_vic, i;
	uint8_t idx_enc, idx_cd, vic_max, enc_max, cd_max;
	uint32_t code;
	double refresh_rate = 0.0;
	char report_name[32] = {0};
	char cmd_data[CMD_SIZE] = {0};
	bool exit, found;
	int command, verdict, cmd_idx;
	FILE * fp = NULL;
	char * tok, * aux;
	bool ycc444, ycc422, cd30, cd36, cd48, ycc444cd, ycc420cd30, ycc420cd36, ycc420cd48;
	

	if ((app == NULL) || (cmd  == NULL)) {
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	user_cfg = app_get_user_config(app);
	if (user_cfg == NULL) {
		CMD_LOGGER(SNPS_ERROR, "Improper user_cfg pointer value");
		return;
	}

	// Fill user_cfg with current video settings
	user_cfg->pVideo.mHdmi = app->mode.pVideo.mHdmi;
	code = app->mode.pVideo.mDtd.mCode;
	if((code == 93) || (code == 94) || (code == 95) || (code == 98) ) {
		user_cfg->pVideo.mHdmiVideoFormat = HDMI_EXT_RES_FORMAT;
	} else {
		user_cfg->pVideo.mHdmiVideoFormat = HDMI_NORMAL_FORMAT;
	}
	user_cfg->pVideo.mEncodingIn = app->mode.pVideo.mEncodingIn;
	user_cfg->pVideo.mEncodingOut = app->mode.pVideo.mEncodingOut;
	user_cfg->pVideo.mColorResolution = app->mode.pVideo.mColorResolution;

	// Query report name
	printf("[Test Report Name]# ");
	exit = false;
	cmd_idx = 0;
	while (!exit) {
		command = getchar();
		switch (command) {
		case '\n':
			exit = true;
			break;
		case 0x8: // backspace
			cmd_data[cmd_idx] = 0;
			if (cmd_idx != 0)
				cmd_idx--;
			break;
		default:
			if ((cmd_idx) >= CMD_SIZE - 1)
				continue;
			cmd_data[cmd_idx++] = (char)command;
			break;
		}
	}
	if (cmd_data[0] != 0) {
		memcpy(report_name, cmd_data, cmd_idx);
		sprintf(cmd_data, "%s.txt", report_name);
		fp = fopen(cmd_data, "w");
	}

	// Backup current user config
	memcpy(&user_cfg_bckp, user_cfg, sizeof(struct hdmi_mode));

	// Get all supported RX VICS
	do {
		memset(cmd_data, 0, CMD_SIZE);
		cmd_idx = 0;
		exit = false;
		printf("[Supported VICs (not YCC420)]# ");
		while (!exit) {
			command = getchar();
			switch (command) {
			case '\n':
				cmd_data[cmd_idx] = '\0';
				exit = true;
				break;
			case 0x8: // backspace
				cmd_data[cmd_idx] = 0;
				if (cmd_idx != 0)
					cmd_idx--;
				break;
			default:
				if ((cmd_idx) >= CMD_SIZE - 1)
					continue;
				cmd_data[cmd_idx++] = (char)command;
				break;
			}
		}
		aux = &cmd_data[0];
		for (vic_max = 0; vic_max < 128;) {
			tok = strsep(&aux, ";, :-");
			if (tok == NULL)
				break;
			code = strtol(tok, NULL, 10);
			if ((code >= 1) && (code < 220)) {
				rx_vic[vic_max++] = code;
			} else if (code == 0) {
				break;
			}
		}
	} while (vic_max == 0);
	
	// Query YCC444 support
	do {
		printf("[Supports YCC444? (y/n)]# ");
		verdict = 0;
		exit = false;
		while (!exit) {
			command = getchar();
			switch (command) {
			case '\n':
				exit = true;
				break;
			case 'y':
				verdict = command;
				ycc444 = true;
				break;
			case 'n':
				verdict = command;
				ycc444 = false;
				break;
			default:
				break;
			}
		}
	} while ((verdict != 'y') && (verdict != 'n'));
	
	// Query YCC422 support
	do {
		printf("[Supports YCC422? (y/n)]# ");
		verdict = 0;
		exit = false;
		while (!exit) {
			command = getchar();
			switch (command) {
			case '\n':
				exit = true;
				break;
			case 'y':
				verdict = command;
				ycc422 = true;
				break;
			case 'n':
				verdict = command;
				ycc422 = false;
				break;
			default:
				break;
			}
		}
	} while ((verdict != 'y') && (verdict != 'n'));
	
	// Query Color Depth 30 support
	do {
		printf("[Supports 30bpp? (y/n)]# ");
		verdict = 0;
		exit = false;
		while (!exit) {
			command = getchar();
			switch (command) {
			case '\n':
				exit = true;
				break;
			case 'y':
				verdict = command;
				cd30 = true;
				break;
			case 'n':
				verdict = command;
				cd30 = false;
				break;
			default:
				break;
			}
		}
	} while ((verdict != 'y') && (verdict != 'n'));
	
	// Query Color Depth 36 support
	do {
		printf("[Supports 36bpp? (y/n)]# ");
		verdict = 0;
		exit = false;
		while (!exit) {
			command = getchar();
			switch (command) {
			case '\n':
				exit = true;
				break;
			case 'y':
				verdict = command;
				cd36 = true;
				break;
			case 'n':
				verdict = command;
				cd36 = false;
				break;
			default:
				break;
			}
		}
	} while ((verdict != 'y') && (verdict != 'n'));
	
	// Query Color Depth 48 support
	do {
		printf("[Supports 48bpp? (y/n)]# ");
		verdict = 0;
		exit = false;
		while (!exit) {
			command = getchar();
			switch (command) {
			case '\n':
				exit = true;
				break;
			case 'y':
				verdict = command;
				cd48 = true;
				break;
			case 'n':
				verdict = command;
				cd48 = false;
				break;
			default:
				break;
			}
		}
	} while ((verdict != 'y') && (verdict != 'n'));
	
	if (ycc444) {
		// Query YCC444 Color Depth support
		do {
			printf("[YCC444 supports color depth? (y/n)]# ");
			verdict = 0;
			exit = false;
			while (!exit) {
				command = getchar();
				switch (command) {
				case '\n':
					exit = true;
					break;
				case 'y':
					verdict = command;
					ycc444cd = true;
					break;
				case 'n':
					verdict = command;
					ycc444cd = false;
					break;
				default:
					break;
				}
			}
		} while ((verdict != 'y') && (verdict != 'n'));
	} else {
		ycc444cd = false;
	}
	
	// Get all YCC420 supported VICS
	memset(cmd_data, 0, CMD_SIZE);
	cmd_idx = 0;
	exit = false;
	printf("[YCC420 supported VICs]# ");
	while (!exit) {
		command = getchar();
		switch (command) {
		case '\n':
			cmd_data[cmd_idx] = '\0';
			exit = true;
			break;
		case 0x8: // backspace
			cmd_data[cmd_idx] = 0;
			if (cmd_idx != 0)
				cmd_idx--;
			break;
		default:
			if ((cmd_idx) >= CMD_SIZE - 1)
				continue;
			cmd_data[cmd_idx++] = (char)command;
			break;
		}
	}
	aux = &cmd_data[0];
	for (vic_max = 0; vic_max < 128;) {
		tok = strsep(&aux, ";, :-");
		if (tok == NULL)
			break;
		code = strtol(tok, NULL, 10);
		if ((code >= 1) && (code < 220)) {
			ycc420_vic[vic_max++] = code;
		} else if (code == 0) {
			break;
		}
	}
	
	if (vic_max == 0) {
		ycc420cd30 = false;
		ycc420cd36 = false;
		ycc420cd48 = false;
	} else {
		// Query YCC420 Color Depth 30 support
		do {
			printf("[YCC420 supports 30bpp? (y/n)]# ");
			verdict = 0;
			exit = false;
			while (!exit) {
				command = getchar();
				switch (command) {
				case '\n':
					exit = true;
					break;
				case 'y':
					verdict = command;
					ycc420cd30 = true;
					break;
				case 'n':
					verdict = command;
					ycc420cd30 = false;
					break;
				default:
					break;
				}
			}
		} while ((verdict != 'y') && (verdict != 'n'));
		
		// Query YCC420 color Depth 36 support
		do {
			printf("[YCC420 supports 36bpp? (y/n)]# ");
			verdict = 0;
			exit = false;
			while (!exit) {
				command = getchar();
				switch (command) {
				case '\n':
					exit = true;
					break;
				case 'y':
					verdict = command;
					ycc420cd36 = true;
					break;
				case 'n':
					verdict = command;
					ycc420cd36 = false;
					break;
				default:
					break;
				}
			}
		} while ((verdict != 'y') && (verdict != 'n'));
		
		// Query YCC420 Color Depth 48 support
		do {
			printf("[YCC420 supports 48bpp? (y/n)]# ");
			verdict = 0;
			exit = false;
			while (!exit) {
				command = getchar();
				switch (command) {
				case '\n':
					exit = true;
					break;
				case 'y':
					verdict = command;
					ycc420cd48 = true;
					break;
				case 'n':
					verdict = command;
					ycc420cd48 = false;
					break;
				default:
					break;
				}
			}
		} while ((verdict != 'y') && (verdict != 'n'));
	}
	
	test_enc[0] = RGB;
	enc_max = 1;
	if (ycc444)
		test_enc[enc_max++] = YCC444;
	if (ycc422)
		test_enc[enc_max++] = YCC422;
	test_enc[enc_max++] = YCC420;
	
	
	// Get all supported Color Depths
	test_cd[0] = COLOR_DEPTH_8;
	cd_max = 1;
	if (cd30)
		test_cd[cd_max++] = COLOR_DEPTH_10;
	if (cd36 || ycc422)
		test_cd[cd_max++] = COLOR_DEPTH_12;
	if (cd48)
		test_cd[cd_max++] = COLOR_DEPTH_16;

	// Get all supported VICs and print supported feature table
	memset(test_vic, 0, 128*sizeof(shortVideoDesc_t));
	printfc(fp, "From User Inputs:\n");
	printfc(fp, "---------------------------------\n");
	printfc(fp, "| VIC | TX | Y444 | Y422 | Y420 |\n");
	printfc(fp, "---------------------------------\n");
	vic_max = 0;
	for (idx_vic = 0; (idx_vic < 128) && (rx_vic[idx_vic] != 0); idx_vic++) {
		found = false;
		// Check if current VIC is supported by Tx and add it to test list
		verdict = edid_tx_supports_cea_code(rx_vic[idx_vic]);
		if (verdict) {
			test_vic[vic_max].mCode = rx_vic[idx_vic];
			for (i = 0; (i < 128) && (ycc420_vic[i] != 0); i++) {
				if (ycc420_vic[i] == rx_vic[idx_vic]) {
					test_vic[vic_max].mYcc420 = 1;
					found = true;
					break;
				}
			}
			vic_max++;
		}
		sprintf(cmd_data, "| %3d | %2s | %4s | %4s | %4s |\n",
				rx_vic[idx_vic],
				MARK(verdict),
				MARK(ycc444),
				MARK(ycc422),
				MARK(found));
		printfc(fp, cmd_data);
	}
	for (i = 0; (i < 128) && (ycc420_vic[i] != 0); i++) {
		for (idx_vic = 0; (idx_vic < 128) && (rx_vic[idx_vic] != 0); idx_vic++) {
			if (ycc420_vic[i] == rx_vic[idx_vic]) {
				break;
			}
		}
		if ((idx_vic == 128) || (rx_vic[idx_vic] == 0)) {
			// Check if current VIC is supported by Tx and add it to test list
			verdict = edid_tx_supports_cea_code(ycc420_vic[i]);
			if (verdict) {
				test_vic[vic_max].mCode = ycc420_vic[i];
				test_vic[vic_max].mLimitedToYcc420 = 1;
				vic_max++;
			}
			sprintf(cmd_data, "| %3d | %2s | %4s | %4s | %4s |\n",
					ycc420_vic[i],
					MARK(verdict), "", "", "*");
			printfc(fp, cmd_data);
		}
	}
	printfc(fp, "---------------------------------\n\n");
	printfc(fp, "---------------------------------\n");
	printfc(fp, "| Encoding | CD30 | CD36 | CD48 |\n");
	printfc(fp, "---------------------------------\n");
	sprintf(cmd_data, "| %8s | %4s | %4s | %4s |\n",
			"RGB", MARK(cd30), MARK(cd36), MARK(cd48));
	printfc(fp, cmd_data);
	printfc(fp, "---------------------------------\n");
	sprintf(cmd_data, "| %8s | %4s | %4s | %4s |\n",
			"YCC444", ycc444cd ? MARK(cd30) : "",
			ycc444cd ? MARK(cd36) : "", ycc444cd ? MARK(cd48) : "");
	printfc(fp, cmd_data);
	printfc(fp, "---------------------------------\n");
	sprintf(cmd_data, "| %8s | %4s | %4s | %4s |\n",
			"YCC420",
			MARK(ycc420cd30), MARK(ycc420cd36), MARK(ycc420cd48));
	printfc(fp, cmd_data);
	printfc(fp, "---------------------------------\n\n");
	
///////////////////////////////////////////////////////////////////////
	for(idx_vic = 0; idx_vic < vic_max; idx_vic++) {
		if (fp)
			fprintf(fp, "--------------------------------------------------------------\n");
		
		for(idx_enc = 0; idx_enc < enc_max; idx_enc++) {
			for(idx_cd = 0; idx_cd < cd_max; idx_cd++) {
				
				// Allocate new user config
				user_cfg = app_get_user_config(app);
				
				// Fill user config with each test config
				user_cfg->pVideo.mHdmi = HDMI;
				
				// Set VIC
				code = test_vic[idx_vic].mCode;
				if ((code == 93) || (code == 94) || (code == 95) || (code == 98) ) {
					user_cfg->pVideo.mHdmiVideoFormat = HDMI_EXT_RES_FORMAT;
				} else {
					user_cfg->pVideo.mHdmiVideoFormat = HDMI_NORMAL_FORMAT;
				}
				
				if (dtd_fill(&app->hdmi_tx, &user_cfg->pVideo.mDtd, code, refresh_rate) == FALSE) {
					CMD_LOGGER(SNPS_ERROR, "Test: Failed to fill dtd structure!\n");
					continue;
				}
				
				// Set Pixel Encoding
				user_cfg->pVideo.mEncodingIn = test_enc[idx_enc];
				user_cfg->pVideo.mEncodingOut = test_enc[idx_enc];
				
				// Set Color Depth
				user_cfg->pVideo.mColorResolution = test_cd[idx_cd];
				
				
				// Check if current VIC is limmeted to YCC420
				if ((user_cfg->pVideo.mEncodingOut != YCC420) && (test_vic[idx_vic].mLimitedToYcc420 == 1)) {
					continue;
				}
				// Check if current VIC has support for YCC420
				if (user_cfg->pVideo.mEncodingOut == YCC420) {
					if ((test_vic[idx_vic].mYcc420 == 0) &&  (test_vic[idx_vic].mLimitedToYcc420 == 0)) {
						continue;
					} else {
						// Check if current VIC supports selected color depth
						if ((user_cfg->pVideo.mColorResolution == COLOR_DEPTH_10) && (ycc420cd30 == 0)) {
							continue;
						}
						if ((user_cfg->pVideo.mColorResolution == COLOR_DEPTH_12) && (ycc420cd36 == 0)) {
							continue;
						}
						if ((user_cfg->pVideo.mColorResolution == COLOR_DEPTH_16) && (ycc420cd48 == 0)) {
							continue;
						}
					}
				}
				
				// Check if YCC444 has support for Color Depth
				if ((user_cfg->pVideo.mEncodingOut == YCC444) && (ycc444cd == 0) && (user_cfg->pVideo.mColorResolution != COLOR_DEPTH_8)) {
					continue;
				}
				// Check if 12 bit was only added becasue of YCC422 support
				if ((user_cfg->pVideo.mEncodingOut != YCC422) && (cd36 == 0) && (user_cfg->pVideo.mColorResolution == COLOR_DEPTH_12)) {
					continue;
				}
				// Use YCC422 with CD12 only
				if ((user_cfg->pVideo.mEncodingOut == YCC422) && (user_cfg->pVideo.mColorResolution != COLOR_DEPTH_12)) {
					continue;
				}
				
				// Limited to TMDS Clock 297 Mhz and no Deep Color support in YCC420
				if ((code == 96) || (code == 97) || (code == 106) || (code == 107)) {
					if ((user_cfg->pVideo.mEncodingOut != YCC420) || (user_cfg->pVideo.mColorResolution != COLOR_DEPTH_8)) {
						continue;
					}
				} else if (user_cfg->pVideo.mDtd.mPixelClock == 297.00) {
					if ((user_cfg->pVideo.mEncodingOut != YCC420) && (user_cfg->pVideo.mColorResolution != COLOR_DEPTH_8)) {
						continue;
					}
				}
				
				// Print output notification with current video settings
				printf("##################################################\n");
				
				// Apply current video setting
				video_apply_cmd(app, cmd);
				
				// Query the use for the test verdict
				do {
					printf("[(p) PASS | (f) FAIL | (r) REPEAT | (e) EXIT]# ");
					verdict = 0;
					exit = false;
					while (!exit) {
						command = getchar();
						switch (command) {
						case '\n':
							exit = true;
							break;
						case 'p':
						case 'f':
						case 'r':
						case 'e':
							verdict = command;
							break;
						default:
							break;
						}
					}
				} while ((verdict != 'p') && (verdict != 'f') &&
					  (verdict != 'r') && (verdict != 'e'));
				
				// Repeat current video settings
				if (verdict == 'r') {
					idx_cd--;
					continue;
				}
				
				// Abort
				if (verdict == 'e') {
					goto exit_fun;
				}
				
				if (verdict == 'f') {
					memset(cmd_data, 0, CMD_SIZE);
					cmd_idx = 0;
					exit = false;
					printf("[Fail cause]# ");
					while (!exit) {
						command = getchar();
						switch (command) {
						case '\n':
							cmd_data[cmd_idx] = '\0';
							exit = true;
							break;
						case 0x8: // backspace
							cmd_data[cmd_idx] = 0;
							if (cmd_idx != 0)
								cmd_idx--;
							break;
						default:
							if ((cmd_idx) >= CMD_SIZE - 1)
								continue;
							cmd_data[cmd_idx++] = (char)command;
							break;
						}
					}
				}
				
				// Print data to report
				if (fp) {
					if (verdict == 'p') {
						fprintf(fp, "VIC=%3d | Encoding=%9s | ColorDepth=%02d | Result: Passed\n",
								user_cfg->pVideo.mDtd.mCode, getEncodingString(user_cfg->pVideo.mEncodingIn),
								user_cfg->pVideo.mColorResolution);
					} else {
						fprintf(fp, "VIC=%3d | Encoding=%9s | ColorDepth=%2d | Result: Failed - %s\n",
								user_cfg->pVideo.mDtd.mCode, getEncodingString(user_cfg->pVideo.mEncodingIn),
								user_cfg->pVideo.mColorResolution, cmd_data);
					}
				}
			}
		}
	}
	
///////////////////////////////////////////////////////////////////////
exit_fun:
	if (fp) {
		fprintf(fp, "--------------------------------------------------------------\n");
		fclose(fp);
	}
	
	printf("#################### COMPLETE ####################\n");
	
	// Allocate new user config
	user_cfg = app_get_user_config(app);
	
	// Restore user config and apply it
	memcpy(user_cfg, &user_cfg_bckp, sizeof(struct hdmi_mode));
	video_apply_cmd(app, cmd);
}
