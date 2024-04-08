// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "app/cmd_line/phy_cmd.h"
#include "app/cmd_line/cmd_interface.h"
#include "platform.h"

#include "hdmitx_ipk_api/phy_interface/phy_interface.h"
#include "identification/identification.h"
#include "phy/phy.h"

#define OUTPUT_DELAY_STEP 78

void phy_id_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	if(app == NULL){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	printf("PHY Id: %s\n", id_phy_string(&app->hdmi_tx));
}

void phy_id_cmd_help(void)
{
	printf("PhyId display Phy ID\n");
}


void phy_interface_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	char *inter_token = NULL;

	if((app == NULL) || cmd->params == NULL){
		phy_access_t interface = phy_get_interface(&app->hdmi_tx);
		switch (interface) {
			case PHY_ACCESS_UNDEFINED: printf("PHY Interface undefined\n"); break;
			case PHY_I2C:  printf("PHY Interface I2C\n"); break;
			case PHY_JTAG: printf("PHY Interface JTAG\n"); break;
			case PHY_EXTERN: printf("PHY Interface External\n"); break;
		}
		return;
	}

	if(phy_get_interface(&app->hdmi_tx) == PHY_EXTERN){
		printf("Interface change not supported for external phy's\n");
		return;
	}

	inter_token = strsep((char **)&cmd->params, " ");

	if(command_match(inter_token, "I2C")){
		if(phy_set_interface(&app->hdmi_tx, PHY_I2C) == 0)
			printf("PHY Interface changed to I2C\n");
		else
			printf("PHY Interface change failed\n");
		return;
	}
	else if(command_match(inter_token, "JTAG")){
		if(phy_set_interface(&app->hdmi_tx, PHY_JTAG) == 0)
			printf("PHY Interface changed to JTAG\n");
		else
			printf("PHY Interface change failed\n");
		return;
	}
	else{
		LOGGER(SNPS_ERROR, "Invalid interface selected");
		phy_interface_cmd_help();
		return;
	}
}


void phy_interface_cmd_help(void)
{
	printf("PhyInterface <I2C|JTAG>\n");
}


void phy_read_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	u32 addr = 0;
	u32 value;
	char *addr_token = NULL;

	if((app == NULL) || cmd->params == NULL){
		LOGGER(SNPS_ERROR, "Improper arguments");
		phy_read_cmd_help();
		return;
	}

	addr_token = strsep((char **)&cmd->params, " ");
	addr = strtol(addr_token, NULL, 0);
	if(addr > 0x2000){
		printf("Phy address too big\n");
		phy_read_cmd_help();
		return;
	}

	if(phy_read(&app->hdmi_tx, (u16) addr, &value)){
		LOGGER(SNPS_ERROR, "Phy read failed");
		return;
	}

	printf("PhyRead [0x%x]=0x%x \n", addr, value);
}

void phy_read_cmd_help(void)
{
	printf("PhyRead <address>\n");
}


void phy_write_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	u32 addr = 0;
	u32 value;
	char *addr_token = NULL;
	char *value_token = NULL;

	if((app == NULL) || cmd->params == NULL){
		LOGGER(SNPS_ERROR, "Improper arguments");
		phy_write_cmd_help();
		return;
	}


	addr_token = strsep((char **)&cmd->params, " ");
	addr = strtol(addr_token, NULL, 0);
	if(addr > 0xFF){
		phy_write_cmd_help();
		return;
	}

	value_token = strsep((char **)&cmd->params, " ");
	value = strtol(value_token, NULL, 0);
	if(value > 0xFFFF){
		phy_write_cmd_help();
		return;
	}

	if(phy_write(&app->hdmi_tx, (u16)addr, (u16)value)){
		LOGGER(SNPS_ERROR, "Phy write failed");
	}

	printf("PhyWrite [0x%x]=0x%x \n", addr, value);
}

void phy_write_cmd_help(void)
{
	printf("PhyWrite <address> <value>\n");
}


void phy_odelay_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	u32 delay = 0;
	char *delay_token = NULL;

	if((app == NULL) || cmd->params == NULL){
		printf("OutputDelay get %d\n", phy_data_output_delay_get());
		return;
	}

	delay_token = strsep((char **)&cmd->params, " ");
	delay = strtol(delay_token, NULL, 0);
	if(delay & (~0x1F)){
		printf("Delay value invalid\n");
		phy_odelay_cmd_help();
		return;
	}

	if(phy_data_output_delay(0xFF,  delay) != 0)
		printf("OutputDelay set to %d (%dps)\n", delay, delay * OUTPUT_DELAY_STEP);
}

void phy_odelay_cmd_help(void)
{
	printf("OutputDelay <delay(0-31)> - each step 78ps (minimum 0ps, maximum 2418ps)\n");
}

void phy_odelay_calibration_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	u32 delay = 0;
	u32 step_size = 0;
	char *token = NULL;
//Test code to test calibration only in a range of lines
#if 0
	u32 start = 0;
	u32 end = 0;
#endif

	if((app == NULL) || cmd->params == NULL){
		LOGGER(SNPS_ERROR, "Invalid arguments");
		phy_odelay_cmd_help();
		return;
	}
#if 0
	token = strsep((char **)&cmd->params, " ");
	start = strtol(token, NULL, 0);
	if(start > 29){
		printf("start value invalid\n");
		phy_odelay_cmd_help();
		return;
	}

	token = strsep((char **)&cmd->params, " ");
	end = strtol(token, NULL, 0);
	if(end > 29){
		printf("end value invalid\n");
		phy_odelay_cmd_help();
		return;
	}
#endif

	token = strsep((char **)&cmd->params, " ");
	step_size = strtol(token, NULL, 0);
	if(step_size & (~0x1F)){
		printf("step value invalid\n");
		phy_odelay_cmd_help();
		return;
	}

	while(1){
		int exit = 0;
		int image_ok = 0;

		if(delay >= 0x1F){
			break;
		}

#if 0
		int i;
		for(i=start; i <= end; i++){
			phy_data_output_delay(i, delay);
		}
#else
		phy_data_output_delay(0xFF, delay); //for all lines
#endif

#if 0
		CMD_LOGGER(SNPS_INFO,"OutputDelayCalibration: Pixel clock[%.3fMhz] delay %d for [%d-%d] status %s\n", app->hdmi_tx.snps_hdmi_ctrl.pixel_clock, delay,start, end, image_ok? "PASS": "FAILED");
#else
		CMD_LOGGER(SNPS_INFO,"OutputDelayCalibration: Pixel clock[%.3fMhz] delay %d status %s\n", app->hdmi_tx.snps_hdmi_ctrl.pixel_clock, delay, image_ok? "PASS": "FAILED");
#endif
		printf("The output image is correct? (y/n/q): \n");

		while(!exit){
			int command = getchar();
			switch(command){
			case '\n': continue;
			case 'y':
				image_ok = 1;
				exit = 1;
				break;
			case 'n':
				image_ok = 0;
				exit = 1;
				break;
			case 'q':
				printf("Calibration stopped\n");
				return;
			default:
				printf("Invalid input - (y/n/q): \n");
				continue;
			}
		}
		delay += step_size;

		if(delay > 0x1F) delay = 0x1F;
	};

	printf("OutputDelayCalibration complete\n");
}

void phy_odelay_calibration_cmd_help(void)
{
	printf("OutputDelayCalibration <step(0-31)>\n");
}

