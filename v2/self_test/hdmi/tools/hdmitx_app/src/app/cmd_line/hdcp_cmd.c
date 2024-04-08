// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "../../../include/app/cmd_line/hdcp_cmd.h"

#include "app/hdcp/hdcp.h"

#include <hdmitx_dev.h>

#include "includes.h"
#include "hdmitx_ipk_api/hdmitx_ipk_api.h"
#include "hdmi_tx_app.h"
#include "platform.h"
#include <limits.h>
#include "hdcp/hdcp.h"
#include "app/hdmitx.h"

#include "app/cmd_line/cmd_interface.h"

void hdcp_bypass_help_cmd()
{
	printf("ByPass <Enable|Disable>\n");
	printf("\tEnable: Bypasses the HDCP encryption\n");
	printf("\tDisable: HDCP encryption required\n");
}

void hdcp_bypass_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	struct hdmi_mode * cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	cfg = app_get_user_config(app);

	if(cfg == NULL){
		LOGGER(SNPS_ERROR, "Improper cfg pointer value");
		return;
	}

	if (cmd->params == NULL) {
		LOGGER(SNPS_ERROR, "Invalid command parameters");
		hdcp_bypass_help_cmd();
		return;
	}

	if(command_match(cmd->params, "Enable")){
		cfg->pHdcp.bypass = TRUE;
	}
	else if(command_match(cmd->params, "Disable")){
		cfg->pHdcp.bypass = FALSE;
	}
	else{
		hdcp_bypass_help_cmd();
		return;
	}
	update_hdcp_cfg(&(cfg->pHdcp), &(app->mode.pHdcp));
}

void hdcp_feature11_help_cmd()
{
	printf("Feature11 <Enable|Disable>\n");
	printf("\tEnable: enables the Features 1.1\n");
	printf("\tDisable: disables the Features 1.1\n");
}

void hdcp_feature11_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	struct hdmi_mode * cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	cfg = app_get_user_config(app);

	if(cfg == NULL){
		LOGGER(SNPS_ERROR, "Improper cfg pointer value");
		return;
	}

	if (cmd->params == NULL) {
		LOGGER(SNPS_ERROR, "Invalid command parameters");
		hdcp_feature11_help_cmd();
		return;
	}

	if(command_match(cmd->params, "Enable")){
		cfg->pHdcp.mEnable11Feature = TRUE;
	}
	else if(command_match(cmd->params, "Disable")){
		cfg->pHdcp.mEnable11Feature = FALSE;
	}
	else{
		hdcp_feature11_help_cmd();
		return;
	}
	update_hdcp_cfg(&(cfg->pHdcp), &(app->mode.pHdcp));
}

void hdcp_richeck_help_cmd()
{
	printf("RICheck <2sec|128f>\n");
	printf("\t2sec: Ri check at every 2s even\n");
	printf("\t128f: Ri check synchronously to every 128 encrypted frame\n");
}

void hdcp_richeck_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	struct hdmi_mode * cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	cfg = app_get_user_config(app);

	if(cfg == NULL){
		LOGGER(SNPS_ERROR, "Improper cfg pointer value");
		return;
	}

	if (cmd->params == NULL) {
		LOGGER(SNPS_ERROR, "Invalid command parameters");
		hdcp_richeck_help_cmd();
		return;
	}

	if(command_match(cmd->params, "2sec")){
		cfg->pHdcp.mRiCheck = TRUE;
	}
	else if(command_match(cmd->params, "128f")){
		cfg->pHdcp.mRiCheck = FALSE;
	}
	else{
		hdcp_richeck_help_cmd();
		return;
	}
	update_hdcp_cfg(&(cfg->pHdcp), &(app->mode.pHdcp));
}

void hdcp_i2c_help_cmd()
{
	printf("I2C <Fast|Slow>\n");
	printf("\tFast: Fast mode\n");
	printf("\tSlow: Slow mode\n");
}

void hdcp_i2c_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	struct hdmi_mode * cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	cfg = app_get_user_config(app);

	if(cfg == NULL){
		LOGGER(SNPS_ERROR, "Improper cfg pointer value");
		return;
	}

	if (cmd->params == NULL) {
		LOGGER(SNPS_ERROR, "Invalid command parameters");
		hdcp_i2c_help_cmd();
		return;
	}

	if(command_match(cmd->params, "Fast")){
		cfg->pHdcp.mI2cFastMode = TRUE;
	}
	else if(command_match(cmd->params, "Slow")){
		cfg->pHdcp.mI2cFastMode = FALSE;
	}
	else{
		hdcp_i2c_help_cmd();
		return;
	}
	update_hdcp_cfg(&(cfg->pHdcp), &(app->mode.pHdcp));
}

void hdcp_elv_help_cmd()
{
	printf("EVL <Enable|Disable>\n");
	printf("\tEnable: enables Enhanced Link Verification\n");
	printf("\tDisable: disables Enhanced Link Verification\n");
}

void hdcp_elv_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	struct hdmi_mode * cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	cfg = app_get_user_config(app);

	if(cfg == NULL){
		LOGGER(SNPS_ERROR, "Improper cfg pointer value");
		return;
	}

	if (cmd->params == NULL) {
		LOGGER(SNPS_ERROR, "Invalid command parameters");
		hdcp_elv_help_cmd();
		return;
	}

	if(command_match(cmd->params, "Enable")){
		cfg->pHdcp.mEnhancedLinkVerification = TRUE;
	}
	else if(command_match(cmd->params, "Disable")){
		cfg->pHdcp.mEnhancedLinkVerification = FALSE;
	}
	else{
		hdcp_elv_help_cmd();
		return;
	}
	update_hdcp_cfg(&(cfg->pHdcp), &(app->mode.pHdcp));
}

void hdcp_numdevices_help_cmd()
{
	printf("NumDevices <1-127>\n");
}

void hdcp_numdevices_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	unsigned long int num_devices = 0;
	struct hdmi_mode * cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	cfg = app_get_user_config(app);

	if(cfg == NULL){
		LOGGER(SNPS_ERROR, "Improper cfg pointer value");
		return;
	}

	if (cmd->params == NULL) {
		LOGGER(SNPS_ERROR, "Invalid command parameters");
		hdcp_numdevices_help_cmd();
		return;
	}

	if(!parse_ulong(cmd->params, &num_devices) || (num_devices > 0x7F)){
		LOGGER(SNPS_ERROR, "Invalid number of devices %lu - maximum supported 127", num_devices);
		hdcp_numdevices_help_cmd();
		return;
	}

	cfg->pHdcp.maxDevices = (u8) num_devices;
	update_hdcp_cfg(&(cfg->pHdcp), &(app->mode.pHdcp));
}



void hdcp_enable_help_cmd()
{
	printf("Configure usage: c <Enable|Disable>\n");
	printf("\tEnable: Forces HDCP enable\n");
	printf("\tDisable: Forces HDCP disable\n");
}

void hdcp_enable_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	if((app == NULL) || (cmd  == NULL)){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	if(cmd->params == NULL){
		LOGGER(SNPS_ERROR, "Invalid command parameters");
		hdcp_enable_help_cmd();
		return;
	}

	if(command_match(cmd->params, "Enable")){
		app->hdmi_tx.snps_hdmi_ctrl.hdcp_on = TRUE;
		app->mode.pHdcp.bypass = FALSE;
		printf("HDCP Enable\n");
	}
	else if(command_match(cmd->params, "Disable")){
		app->hdmi_tx.snps_hdmi_ctrl.hdcp_on = FALSE;
		app->mode.pHdcp.bypass = TRUE;
		hdcp_rxdetect(&app->hdmi_tx, 0);
		hdcp_disable_encryption(&app->hdmi_tx, TRUE);
		printf("HDCP Disabled\n");
	}
	else{
		hdcp_enable_help_cmd();
		return;
	}

	pthread_mutex_lock(&(app->mutex));

	// Initial VGA configuration
	if(hdmitx_vga_init(app) != TRUE){
		LOGGER(SNPS_ERROR, "hdmitx_configure_vga failed");
	}

	// video_generator_config(app);
	// audio_generator_config(app);

	api_Configure(&app->hdmi_tx, &app->mode.pVideo, &app->mode.pAudio,
			&app->mode.pProduct, &app->mode.pHdcp, app->hdmi_tx_phy);
	pthread_mutex_unlock(&(app->mutex));
}

void debug_help_cmd()
{
	printf("Debug usage: d <Enable|Disable>\n");
}

void debug_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	struct hdmi_mode * cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	cfg = app_get_user_config(app);

	if(cfg == NULL){
		LOGGER(SNPS_ERROR, "Improper cfg pointer value");
		return;
	}

	if(cmd->params == NULL){
		LOGGER(SNPS_ERROR, "Invalid command parameters");
		debug_help_cmd();
		return;
	}

	if(command_match(cmd->params, "Enable")){
		app->verbose = TRUE;
		printf("Debug Enable\n");
	}
	else if(command_match(cmd->params, "Disable")){
		app->verbose = FALSE;
		printf("Debug Disabled\n");

	}
	else{
		debug_help_cmd();
		return;
	}
	update_hdcp_cfg(&(cfg->pHdcp), &(app->mode.pHdcp));
}


void print_hdcpinfo(hdcpParams_t * phdcp)
{
	if(phdcp == NULL){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	if(phdcp->bypass != -1)
		printf("\tBypass %s\n", phdcp->bypass == TRUE ? "Enable" : "Disable");
	if(phdcp->mEnable11Feature != -1)
		printf("\tFeature11 %s\n", phdcp->mEnable11Feature == TRUE ? "Enable" : "Disable");
	if(phdcp->mRiCheck != -1)
		printf("\tRICheck %s\n", phdcp->mRiCheck == TRUE ? "2sec" : "128f");
	if(phdcp->mI2cFastMode != -1)
		printf("\tI2C %s\n", phdcp->mI2cFastMode == TRUE ? "Fast" : "Slow");
	if(phdcp->mEnhancedLinkVerification != -1)
		printf("\tELV %s\n", phdcp->mEnhancedLinkVerification == TRUE ? "Enable" : "Disable");
	if(phdcp->maxDevices != 0)
		printf("\tMax number of Devices %u\n", phdcp->maxDevices);
}

void hdcp_show_help_cmd(){
	printf("Show usage: s <Cfg|Pending>\n");
	printf("\tCfg: Show configured settings\n");
	printf("\tPending: Show configuration pending for apply\n");
}

void hdcp_show_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	hdcpParams_t * phdcp = NULL;
	struct hdmi_mode * cfg = NULL;

	if((app == NULL) || (cmd  == NULL)){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	cfg = app_get_user_config(app);

	if(cfg == NULL){
		LOGGER(SNPS_ERROR, "Improper cfg pointer value");
		return;
	}

	if(cmd->params == NULL){
		LOGGER(SNPS_ERROR, "Invalid command parameters");
		hdcp_show_help_cmd();
		return;
	}

	if(command_match(cmd->params, "Cfg")){
		phdcp = &(app->mode.pHdcp);
		printf("Configured settings\n");
	}
	else if(command_match(cmd->params, "Pending")){
		phdcp = &(cfg->pHdcp);
		printf("Configuration pending for apply\n");
	}
	else {
		LOGGER(SNPS_ERROR, "Invalid option %s\n", cmd->params);
		hdcp_show_help_cmd();
		return;
	}

	print_hdcpinfo(phdcp);
}



