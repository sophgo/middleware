// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "app/cmd_line/expert_cmd.h"
#include "app/cec/cec_app.h"
#include "app/signal_handler.h"

#include "app/cmd_line/cmd_interface.h"
#include "includes.h"
#include "hdmi_tx_app.h"

#include "app/edid/edid.h"
#include "video_bridge.h"
#include "hdmitx_ipk_api/access_ipk.h"
#include "bsp/i2cm.h"

void edid_tx_check_help(void)
{
	printf("EdidTx usage: et <Enable|Disable>\n");
}

void edid_tx_check_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	if(cmd->params == NULL){
		CMD_LOGGER(SNPS_ERROR, "Invalid command parameters");
		edid_tx_check_help();
		return;
	}

	if(command_match(cmd->params, "Enable")){
		CMD_LOGGER(SNPS_INFO, "Enable EDID TX checks");
		app->edid_tx_checks = 1;
		edid_read_hdmitx_cap(NULL);
	}
	else if(command_match(cmd->params, "Disable")){
		CMD_LOGGER(SNPS_INFO, "Disable EDID TX checks");
		app->edid_tx_checks = 0;
	}	
}


void edid_tx_reload_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}
	CMD_LOGGER(SNPS_INFO, "Reload EDID TX capabilities");
	edid_read_hdmitx_cap(app->edid_tx_capabilites_file);
}

void edid_rx_dump_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	char cmd_data[CMD_SIZE] = {0};
	int cmd_idx = 0;
	bool exit = false;
	int command, blck, i;
	FILE * fp = NULL;
	
	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}
	
	if (!phy_hot_plug_state(&app->hdmi_tx)) {
		printf("Failed to retrieve EDID since there's no device connected!\n");
		return;
	}
	
	printf("[Filename]# ");
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
			if ((cmd_idx) >= CMD_SIZE - 5)
				continue;
			cmd_data[cmd_idx++] = (char)command;
			break;
		}
	}
	
	strcat(cmd_data, ".txt");
	fp = fopen(cmd_data, "w");
	
	if (fp == NULL) {
		CMD_LOGGER(SNPS_ERROR, "Unable to create file");
		return;
	}
	
	fprintf(fp, "EDID BLOCK 0:\n");
	for (i = 0; i < sizeof(struct edid); i++) {
		fprintf(fp, "%02X ", ((u8 *)&app->mode.edid)[i]);
		if ((i + 1) % 16 == 0)
			fprintf(fp, "\n");
	}
	for (blck = 0; blck < 3; blck++) {
		fprintf(fp, "EDID BLOCK %d:\n", blck+1);
		for (i = 0; i < 128; i++) {
			fprintf(fp, "%02X ", app->mode.edid_ext[blck][i]);
			if ((i + 1) % 16 == 0)
				fprintf(fp, "\n");
		}
	}
	fclose(fp);
}

void pattern_reload(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	encoding_t encoding;

	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	if(command_match(cmd->params, "RGB")){
		encoding = RGB;
	}
	else if(command_match(cmd->params, "YCC444")){
		encoding = YCC444;
	}
	else if(command_match(cmd->params, "YCC422")){
		encoding = YCC422;
	}
	else if(command_match(cmd->params, "YCC420")){
		encoding = YCC420;
	}
	else{
		CMD_LOGGER(SNPS_ERROR, "Encoding not supported");
		return;
	}

	video_generator_set_pattern(encoding);
}

void chess_pattern_help(void)
{
	printf("ChessPattern <Enable|Disable>");
}

void chess_pattern_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	if(cmd->params == NULL){
		CMD_LOGGER(SNPS_ERROR, "Invalid command parameters");
		chess_pattern_help();
		return;
	}

	if(command_match(cmd->params, "Enable")){
		CMD_LOGGER(SNPS_INFO, "Enable Chess board pattern");
		video_chess_board_config(app);
		video_generator_pattern_mode(app, 0x2);
	}
	else if(command_match(cmd->params, "Disable")){
		CMD_LOGGER(SNPS_INFO, "Disable Chess board pattern");
		video_generator_pattern_mode(app, 0x0);
	}
}

void disable_pattern_load(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	if(cmd->params == NULL){
		CMD_LOGGER(SNPS_ERROR, "Invalid command parameters");
		edid_tx_check_help();
		return;
	}

	if(command_match(cmd->params, "Enable")){
		CMD_LOGGER(SNPS_INFO, "Pattern loading is enable");
		app->pattern_load_enable = 1;
	}
	else if(command_match(cmd->params, "Disable")){
		CMD_LOGGER(SNPS_INFO, "Pattern loading is disable");
		app->pattern_load_enable = 0;
	}
}


void svd_tx_list_load_help(void)
{
	printf("SupportedVicList - Print supported VIC list for HDMI TX\n");
}

void svd_tx_list_load_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	int i = 0;
	int hdmi_vic = 0;
	sink_edid_t * sink_cap = NULL;

	if(app == NULL){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	sink_cap = app->tx_sink_cap;

	if(sink_cap == NULL){
		CMD_LOGGER(SNPS_ERROR, "HDMI TX EDID empty");
		return;
	}

	printf("HDMI TX is 2.0 = %s\n", sink_cap->edid_m20Sink? "Yes":"No");
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

		if(edid_tx_supports_cea_code(cea_mode) == FALSE){
			printf(" - not supported by TX");
		}
		printf("\n");
	}
}


void hdmitx_read_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	u32 addr = 0;
	u32 value;
	char *addr_token = NULL;

	if((app == NULL) || cmd->params == NULL){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		hdmitx_read_cmd_help();
		return;
	}

	addr_token = strsep((char **)&cmd->params, " ");
	if(addr_token == NULL){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		hdmitx_read_cmd_help();
		return;
	}

	addr = strtoul(addr_token, NULL, 0);
	value = hdmitx_read(addr);


	printf("HDMITX read [0x%x]=0x%x \n", addr, value);
}

void hdmitx_read_cmd_help(void)
{
	printf("Read <address>\n");
}


void hdmitx_write_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	u32 addr = 0;
	u32 value;
	char *addr_token = NULL;
	char *value_token = NULL;

	if((app == NULL) || cmd->params == NULL){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		hdmitx_write_cmd_help();
		return;
	}


	addr_token = strsep((char **)&cmd->params, " ");
	if(cmd->params == NULL){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		hdmitx_write_cmd_help();
		return;
	}
	addr = strtoul(addr_token, NULL, 0);

	if(cmd->params == NULL){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		hdmitx_write_cmd_help();
		return;
	}

	value_token = strsep((char **)&cmd->params, " ");
	if(value_token == NULL){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		hdmitx_write_cmd_help();
		return;
	}
	value = strtoul(value_token, NULL, 0);

	hdmitx_write(addr, value);

	printf("HDMITX Write [0x%x]=0x%x \n", addr, value);
}

void hdmitx_write_cmd_help(void)
{
	printf("Write <address> <value>\n");
}

void i2c_bus_clear_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	if(app == NULL){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	i2c_bus_clear(&app->hdmi_tx);
}

void i2c_bus_clear_cmd_help(void)
{
	printf("clear - Activate I2C clear bus function\n");
}
