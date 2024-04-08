// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "app/cmd_line/audio_cmd.h"

#include "app/audio/audio.h"
#include "platform.h"
#include "util/general_ops.h"

#include "app/cmd_line/cmd_interface.h"
#include "app/hdmitx.h"
#include "video_bridge.h"
#include "audio_bridge.h"
#include "ipc/ipc_msg.h"

void coding_type_help(void)
{
	printf("Coding Types:\n");
	printf("\t1 - PCM\n");
	printf("\t2 - AC3\n");
	printf("\t3 - MPEG1\n");
	printf("\t4 - MP3\n");
	printf("\t5 - MPEG2\n");
	printf("\t6 - AAC\n");
	printf("\t7 - DTS\n");
	printf("\t8 - ATRAC\n");
	printf("\t9 - ONE BIT AUDIO\n");
	printf("\t10 - DOLBY DIGITAL PLUS\n");
	printf("\t11 - DTS HD\n");
}

void coding_type_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	int type = 0;
	struct hdmi_mode * user_cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	user_cfg = app_get_user_config(app);

	if(user_cfg == NULL){
		LOGGER(SNPS_ERROR, "Improper user_cfg pointer value");
		return;
	}

	if(cmd->params == NULL){
		LOGGER(SNPS_ERROR, "Invalid command parameters");
		coding_type_help();
		return;
	}

	if(command_match(cmd->params, "Help")){
		coding_type_help();
		return;
	}

	type = strtol(cmd->params, NULL, 10);
	if((type < 1) || (type > 11)){
		LOGGER(SNPS_ERROR, "Type [%d] is not supported", type);
		coding_type_help();
		return;
	}

	user_cfg->pAudio.mCodingType = type;
}


void sample_freq_help(void)
{
	printf("Sample Frequency values:\n");
	printf("\t32kHz,     44.1kHz,  48kHz\n");
	printf("\t64kHz,     88.2kHz,  96kHz\n");
	printf("\t128kHz,    176.4kHz, 192kHz\n");
	printf("\t256kHz,    352.8kHz, 512kHz\n");
	printf("\t705.6kHz,  768kHz,   1024kHz\n");
	printf("\t1411.2kHz, 1536kHz\n");
}

void sample_freq_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	double sample_freq = 0.0;
	struct hdmi_mode * user_cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	user_cfg = app_get_user_config(app);

	if(user_cfg == NULL){
		LOGGER(SNPS_ERROR, "Improper user_cfg pointer value");
		return;
	}

	if(cmd->params == NULL){
		LOGGER(SNPS_ERROR, "Invalid command parameters");
		sample_freq_help();
		return;
	}

	if(command_match(cmd->params, "Help")){
		sample_freq_help();
		return;
	}

	sample_freq = strtod(cmd->params, NULL);
	if(double_is_equal(sample_freq, 0.0) || (sample_freq < 0.0)){
		LOGGER(SNPS_ERROR, "Frequency %.3fkHz is not supported", sample_freq);
		sample_freq_help();
		return;
	}

	if(sample_freq > 1536.00){
		LOGGER(SNPS_ERROR, "Frequency %.3fkHz is higher than supported", sample_freq);
		LOGGER(SNPS_ERROR, "Frequency factor is in kHz", sample_freq);
		sample_freq_help();
	}

	user_cfg->pAudio.mSamplingFrequency = sample_freq;
}


void interface_type_help(void)
{
	printf("Interface type command usage:\n");
	printf("\t\tit <I2S|SPDIF>\n");
}

void interface_type_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	struct hdmi_mode * user_cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	user_cfg = app_get_user_config(app);

	if(user_cfg == NULL){
		LOGGER(SNPS_ERROR, "Improper user_cfg pointer value");
		return;
	}

	if(command_match(cmd->params, "I2S")){
		user_cfg->pAudio.mInterfaceType = I2S;
	}
	else if(command_match(cmd->params, "SPDIF")){
		user_cfg->pAudio.mInterfaceType = SPDIF;
		//NOTE: For now higher FS Factor support in SW is 512, support for higher FS is possible
		user_cfg->pAudio.mClockFsFactor = 512;
	}
	else if(command_match(cmd->params, "AHBDMA")){
		user_cfg->pAudio.mInterfaceType = DMA;
		//NOTE: For now higher FS Factor support in SW is 512, support for higher FS is possible
		// user_cfg->pAudio.mClockFsFactor = 512;
	}
	else{
		LOGGER(SNPS_ERROR, "Improper command parameter: %s", cmd->params);
		interface_type_help();
	}
}


void sample_size_help(void)
{
	printf("SampleSize usage: ss <16-24>\n");
}

void sample_size_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	int sample_size = 0;
	struct hdmi_mode * user_cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	user_cfg = app_get_user_config(app);

	if(user_cfg == NULL){
		LOGGER(SNPS_ERROR, "Improper user_cfg pointer value");
		return;
	}

	if(cmd->params == NULL){
		LOGGER(SNPS_ERROR, "Invalid command parameters");
		sample_size_help();
		return;
	}

	if(command_match(cmd->params, "Help")){
		sample_size_help();
		return;
	}

	sample_size = strtol(cmd->params, NULL, 10);
	if((sample_size < 16) || (sample_size > 24)){
		LOGGER(SNPS_ERROR, "Sample size of %d is not supported", sample_size);
		sample_size_help();
		return;
	}

	user_cfg->pAudio.mSampleSize = sample_size;
}


void fs_factor_help(void)
{
	printf("Supported FS Factor values:\n");
	printf("\t- 64, 128, 256, 512\n");
}

void fs_factor_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	int fsfactor = 0;
	struct hdmi_mode * user_cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	user_cfg = app_get_user_config(app);

	if(user_cfg == NULL){
		LOGGER(SNPS_ERROR, "Improper user_cfg pointer value");
		return;
	}

	if(cmd->params == NULL){
		LOGGER(SNPS_ERROR, "Invalid command parameters");
		fs_factor_help();
		return;
	}

	if(command_match(cmd->params, "Help")){
		fs_factor_help();
		return;
	}

	fsfactor = strtol(cmd->params, NULL, 10);
	if((fsfactor != 64) && (fsfactor != 128) && (fsfactor != 256) && (fsfactor != 512)){
		LOGGER(SNPS_ERROR, "FS factor %d is not supported", fsfactor);
		fs_factor_help();
		return;
	}
	
	//NOTE: For now higher FS Factor support in SW is 512, support for higher FS is possible
	if((fsfactor != 512) && (user_cfg->pAudio.mInterfaceType == SPDIF)){
		printf("SPDIF only supports FS Factor equal to 512.\n");
		fsfactor = 512;
	}

	user_cfg->pAudio.mClockFsFactor = fsfactor;
}


void audio_show_help(void)
{

}

void audio_show_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	struct hdmi_mode * user_cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	user_cfg = app_get_user_config(app);

	if(user_cfg == NULL){
		LOGGER(SNPS_ERROR, "Improper user_cfg pointer value");
		return;
	}

	printf("\n*** Current audio configurations ***\n");
	print_audioinfo(&app->mode.pAudio);

	printf("\n*** New audio configurations - run Apply to make them active ***\n");
	print_audioinfo(&user_cfg->pAudio);
}


void audio_apply_help(void)
{

}

void audio_apply_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	struct hdmi_mode * user_cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	user_cfg = app_get_user_config(app);

	if(user_cfg == NULL){
		LOGGER(SNPS_ERROR, "Improper user_cfg pointer value");
		return;
	}

	pthread_mutex_lock(&(app->mutex));

	// Initial VGA configuration
//	if(hdmitx_vga_init(app) != TRUE){
//		LOGGER(SNPS_ERROR, "hdmitx_configure_vga failed");
//	}

	update_audio_cfg(&(user_cfg->pAudio), &(app->mode.pAudio));

	// video_generator_config(app);
	// audio_generator_config(app);

	if (!api_Configure(&app->hdmi_tx, &app->mode.pVideo, &app->mode.pAudio,
			&app->mode.pProduct, &app->mode.pHdcp, app->hdmi_tx_phy)) {
		hdmitx_logger(SNPS_ERROR, "API configure", error_Get());
	}

	// ELLIPTIC Workaround - set internal hpd flag for the hdcp_tx to restart the authentication process
	if (send_hdmi_status(STATUS_HDMI_HDCP_REAUTH_REQUEST) < 0)
		hdmitx_logger(SNPS_ERROR, "FAIL to request HDCP HPD re authentication");

	app->user_cfg = NULL;
	free(user_cfg);

	pthread_mutex_unlock(&(app->mutex));
}
