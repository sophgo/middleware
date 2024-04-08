// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "includes.h"
#include "hdmitx_ipk_api/hdmitx_ipk_api.h"
#include "hdmi_tx_app.h"
#include "platform.h"
#include "scdc/scrambling.h"

#include "../../../include/app/cmd_line/cmd_interface.h"
#include "app/edid/edid_parser.h"

void scdc_enable_rr_help_cmd()
{
	printf("scdcRR <ENable|DIsable|Show>\n");
	printf("\t\tENable:  Set ReadRequest bit to 1\n");
	printf("\t\tDIsable: Set ReadRequest bit to 0\n");
	printf("\t\tShow:    Show current status\n");
}

int _scdc_supported(struct hdmi_tx_app *app)
{
	sink_edid_t * sink = app->mode.sink_cap;
	if(sink->edid_mHdmiForumvsdb.mSCDC_Present == 0){
		printf("Sink does not support SCDC\n");
		return -1;
	}
	return 0;
}

int _scdc_RR_supported(struct hdmi_tx_app *app)
{
	sink_edid_t * sink = app->mode.sink_cap;
	if(sink->edid_mHdmiForumvsdb.mRR_Capable == 0){
		printf("Sink does not support SCDC\n");
		return -1;
	}
	return 0;
}

int _scdc_scrambling_Lte340(struct hdmi_tx_app *app)
{
	sink_edid_t * sink = app->mode.sink_cap;
	if(sink->edid_mHdmiForumvsdb.mLTS_340Mcs_scramble  == 0){
		printf("Sink does not support Scrambling below 340Mcs\n");
		return -1;
	}
	return 0;
}

void scdc_enable_rr_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	if(_scdc_supported(app)){
		printf("SCDC not supported\n");
		return;
	}

	if(command_match(cmd->params, "ENable")){
		scdc_enable_rr(&app->hdmi_tx, TRUE);

	}else if(command_match(cmd->params, "DIsable")){
		scdc_enable_rr(&app->hdmi_tx, FALSE);

	}else if(command_match(cmd->params, "Show")){
		u8 flag = 0;
		if(scdc_get_rr_flag(&app->hdmi_tx, &flag)){
			printf("Read request flag read failed\n");
			return;
		}
		printf("Read request flag : 0x%02X\n", flag);

	}else{
		scdc_enable_rr_help_cmd();
		return;
	}
}

void scdc_test_rr_help_cmd()
{
	printf("TestRR <delayms> \n");
	printf("\t\tdelayms:  Value from 0x00 - 0x7F\n");
}

void scdc_test_rr_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	uint16_t delayms = 0;
	uint8_t rr_upd_flag = 0;
	char *delayms_token = NULL;

	if(_scdc_supported(app)){
		printf("SCDC not supported\n");
		return;
	}

	if(_scdc_RR_supported(app))
		return;

	if(cmd->params == NULL){
		LOGGER(SNPS_ERROR, "Invalid parameters");
		scdc_test_rr_help_cmd();
		return;
	}

	delayms_token = strsep((char **)&cmd->params, " ");
	delayms = strtol(delayms_token, NULL, 0);
	if(((int16_t)delayms < 0) || (delayms > 0x7F)){
		scdc_test_rr_help_cmd();
		return;
	}

	scdc_enable_rr(&app->hdmi_tx, TRUE);
	scdc_test_rr(&app->hdmi_tx, delayms);
	rr_upd_flag = scdc_test_rr_update_flag(&app->hdmi_tx);
	printf("Read Request Test Update Flag %d\n", rr_upd_flag);

}

void scdc_scrambling_stat_help_cmd()
{
	printf("SCRAMble - Scrambling Status\n");
}

void scdc_scrambling_stat_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	if(_scdc_supported(app)){
		printf("SCDC not supported\n");
		return;
	}

	printf("Scrambling Status Flag %d\n", scdc_scrambling_status(&app->hdmi_tx));
}

void scdc_scrambling_enable_help_cmd()
{
	printf("SCRAMble <ENable|DIsable> - Scrambling Enable/Disable\n");
}

void scdc_scrambling_enable_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	if(_scdc_supported(app)){
		printf("SCDC not supported\n");
		return;
	}	

	if(command_match(cmd->params, "ENable")){
		if(_scdc_scrambling_Lte340(app)){
			printf("Scrambling not supported\n");
			return;
		}
		printf("Enabling Scrambling\n");
		scrambling(&app->hdmi_tx, TRUE);
	}else if(command_match(cmd->params, "DIsable")){
		printf("Disabling Scrambling\n");
		scrambling(&app->hdmi_tx, FALSE);
	}else{
		scdc_scrambling_enable_help_cmd();
		return;
	}
}

void scdc_read_help_cmd()
{
	printf("ReaD <addr> [size]\n");
	printf("\t\taddr:  Value from 0x01 - 0xFF\n");
	printf("\t\tsize:  Number of bytes (default 1)\n");
}

void scdc_read_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	uint32_t addr = 0;
	uint32_t size = 1;
	char *addr_token = NULL;
	char *size_token = NULL;
	u8 * data;
	int i;

	if(_scdc_supported(app)){
		printf("SCDC not supported\n");
		return;
	}

	if(cmd->params == NULL){
		LOGGER(SNPS_ERROR, "Invalid parameters");
		scdc_read_help_cmd();
		return;
	}

	addr_token = strsep((char **)&cmd->params, " ");
	addr = strtol(addr_token, NULL, 0);
	if((addr < 1) || (addr > 0xFF)){
		scdc_read_help_cmd();
		return;
	}

	size_token = strsep((char **)&cmd->params, " ");
	if(size_token != NULL){
		size = strtol(size_token, NULL, 0);
		if((size < 1) || (size + addr > 0xFF)){
			scdc_read_help_cmd();
			return;
		}
	}

	data = (u8 *) malloc(sizeof(u8) * size);

	printf("Read [0x%02x] = ", addr);
	if(scdc_read(&app->hdmi_tx, (u8) addr, (u8) size,  data)){
		printf("FAILED\n");
		return;
	}

	for (i=0; i < size;){
		printf("0x%02x ", data[i++]);
		if(((i%6) == 0) && (i < size))
			printf("\n");
	}
	printf("\n");
}

void scdc_write_help_cmd()
{
	printf("WRite <addr> <data>\n");
	printf("\t\taddr:  Value 0x02, 0x10, 0x11, 0x20, 0x30, 0xC0 and 0xDE to 0xFF\n");
	printf("\t\tdata:  data to write 0x00-0xFF\n");
}

void scdc_write_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	uint32_t addr = 0;
	uint32_t data = 0;
	char *addr_token = NULL;
	char *data_token = NULL;

	if(_scdc_supported(app)){
		printf("SCDC not supported\n");
		return;
	}

	if(cmd->params == NULL){
		LOGGER(SNPS_ERROR, "Invalid parameters");
		scdc_write_help_cmd();
		return;
	}

	addr_token = strsep((char **)&cmd->params, " ");
	addr = strtol(addr_token, NULL, 0);
	if((addr != 2) && (addr != 0x10) && (addr != 0x11) &&
			(addr != 0x20) && (addr != 0x30) && (addr != 0xC0) &&
			((addr < 0xDE) || (addr > 0xFF))){
		scdc_write_help_cmd();
		return;
	}

	data_token = strsep((char **)&cmd->params, " ");
	if(data_token == NULL){
		scdc_write_help_cmd();
		return;
	}

	data = strtol(data_token, NULL, 0);
	if(((int32_t)data < 0) || (data > 0xFF)){
		scdc_write_help_cmd();
		return;
	}
	printf("Write[0x%02x]=0x%02x - ", addr, data);
	if(scdc_write(&app->hdmi_tx, (u8) addr, (u8) data, (u8*) &data)){
		printf("FAILED\n");
		return;
	}
	printf("OK\n");
}

void scdc_read_cnt_help_cmd()
{
	printf("ReadCounters\n");
}

void scdc_read_cnt_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	u8 data[8];
	u16 cnt_data[3];
	u8 cnt_valid[3];
	u8 chs = 0;
	int i, j;

	if(_scdc_supported(app)){
		printf("SCDC not supported\n");
		return;
	}

	printf("Read Counters = ");
	if(scdc_read_cnt(&app->hdmi_tx, &data[0])){
		printf("FAILED\n");
		return;
	}

	for (i = 0, j = 0; i < 3 && j < 7; i++, j += 2) { 
		cnt_data[i] = data[j + 1];
		cnt_data[i] &= 0x7f;
		cnt_data[i] <<= 8;
		cnt_data[i] |= data[j];

		cnt_valid[i] = data[j + 1];
		cnt_valid[i] &= 0x80;

		chs += data[j];
		chs += data[j + 1];
	}
	chs = 0xff - chs;
	chs +=1 ;

	printf("CH0=[%s][%u], CH1=[%s][%u], CH2=[%s][%u], CheckSum=[%s][0x%.2x]\n",
			cnt_valid[0] ? "V": "NV", cnt_data[0],
			cnt_valid[1] ? "V": "NV", cnt_data[1],
			cnt_valid[2] ? "V": "NV", cnt_data[2],
			chs == data[6] ? "V" : "NV",
			data[6]);
}
