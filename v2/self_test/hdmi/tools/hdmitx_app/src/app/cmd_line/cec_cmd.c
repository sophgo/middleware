// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "app/cmd_line/cmd_interface.h"
#include "app/cmd_line/cec_cmd.h"
#include "app/cec/cec_app.h"
#include "app/signal_handler.h"
#include "includes.h"

void cec_enable_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	if(cmd->params == NULL){
		CMD_LOGGER(SNPS_ERROR, "Invalid command parameters");
		return;
	}
	if(command_match(cmd->params, "Enable")){
		CMD_LOGGER(SNPS_INFO, "Enabled CEC communication");
		app->hdmi_tx.snps_hdmi_ctrl.cec_on = 1;
		app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.cec_state = CEC_NOT_INIT;
		cec_Init(&app->hdmi_tx);
		cec_handler();
	}
	else if(command_match(cmd->params, "Disable")){
		CMD_LOGGER(SNPS_INFO, "Disabled CEC communication");
		app->hdmi_tx.snps_hdmi_ctrl.cec_on = 0;
		app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.cec_state = CEC_NOT_INIT;
		app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.logical_address = 0xf;
		cec_IntDisable(&app->hdmi_tx, 0xff);
	}
}

void cec_standby_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	char *buffer;
	buffer = malloc(sizeof(char)*16);
	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	if(cmd->params == NULL){
		CMD_LOGGER(SNPS_ERROR, "Invalid command parameters");
		return;
	}
	if(command_match(cmd->params, "Enable")){
		if(app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.power_state != STANDBY)
		{
			if(app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.active_source)
			{
				/* send broadcast message <Inactive Source> */
				buffer[0]=CEC_OPCODE_INACTIVE_SOURCE;
				buffer[1] = app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.physical_address >> 8;
				buffer[2] = app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.physical_address;
				if(!cec_ctrlSendFrame(&app->hdmi_tx, buffer, 3,
						app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.logical_address, TV))
					LOGGER(SNPS_ERROR, "CEC: Error sending <Standby>.\n");
			}
			/* send broadcast message <Standby> */
			buffer[0]=CEC_OPCODE_STANDBY;
			if(!cec_ctrlSendFrame(&app->hdmi_tx, buffer, 1,
					app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.logical_address, UNREGISTERED_BROADCAST))
				LOGGER(SNPS_ERROR, "CEC: Error sending <Standby>.\n");

			app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.power_state = STANDBY;
			app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.active_source = 0;
			printf("CEC: Device Set to standby mode\n");
		}
		else
		{
			printf("CEC: Device already set to standby mode\n");
		}

	}
	else if(command_match(cmd->params, "Disable")){
		if(app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.power_state == STANDBY)
		{
			app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.power_state = ON;
			CMD_LOGGER(SNPS_INFO, "CEC: Set device power on");
		}
		else
			CMD_LOGGER(SNPS_INFO, "CEC: device already in power on mode");
	}
	free(buffer);
}

void cec_onetouch_play_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	char *buffer;
	buffer = malloc(sizeof(char)*16);

	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}
	buffer[0] = CEC_OPCODE_IMAGE_VIEW_ON;
	if(!cec_ctrlSendFrame(&app->hdmi_tx, buffer, 1,
			app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.logical_address, TV))
		LOGGER(SNPS_ERROR, "CEC: Error sending <ImageView On>.\n");
	buffer[0] = CEC_OPCODE_ACTIVE_SOURCE;
	buffer[1] = app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.physical_address >> 8;
	buffer[2] = app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.physical_address;
	if(!cec_ctrlSendFrame(&app->hdmi_tx, buffer, 3,
			app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.logical_address, UNREGISTERED_BROADCAST))
		LOGGER(SNPS_ERROR, "CEC: Error sending <Active Source>.\n");
	app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.active_source = 1;
	free(buffer);

}

void cec_getmenu_language_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	char *buffer;
	buffer = malloc(sizeof(char)*16);

	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}
	buffer[0] = CEC_OPCODE_GET_MENU_LANGUAGE;
	if(!cec_ctrlSendFrame(&app->hdmi_tx, buffer, 1,
			app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.logical_address, TV))
		LOGGER(SNPS_ERROR, "CEC: Error sending <Get Menu Language>.\n");
	free(buffer);

}

void cec_setmenu_language_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	char language[5];

	if(cmd->params == NULL){
		LOGGER(SNPS_ERROR, "Invalid parameters");
		return;
	}
	strcpy(language, "..");
	strcat(language, strsep((char **)&cmd->params, " "));

	if(!cec_update_menu_language(&app->hdmi_tx, language))
		LOGGER(SNPS_ERROR, "Unsupported or invalid language");

}


void cec_setactive_source_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	char *buffer;
	buffer = malloc(sizeof(char)*16);

	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}
	buffer[0] = CEC_OPCODE_ACTIVE_SOURCE;
	buffer[1] = app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.physical_address >> 8;
	buffer[2] = app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.physical_address;
	if(!cec_ctrlSendFrame(&app->hdmi_tx, buffer, 3,
			app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.logical_address, UNREGISTERED_BROADCAST))
		LOGGER(SNPS_ERROR, "CEC: Error sending <Active Source>.\n");
	app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.active_source = 1;
	LOGGER(SNPS_NOTICE, "CEC: SNPS TX defined as active source.");
	free(buffer);

}

void cec_send_msg_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	printf("Sent %d bytes\n", cec_ctrlSendFrame(&app->hdmi_tx, "0x01", 1, 1, 2));
}

void cec_wait_receive_msg_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	char *buffer;
	buffer = malloc(sizeof(char)*16);
	if(!cec_ctrlReceiveFrame(&app->hdmi_tx, buffer, 3))
		printf("error sending frames\n");
	else
		printf("%s %s\n", __func__, buffer);
	free(buffer);
}

void cec_status_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	CMD_LOGGER(SNPS_NOTICE, "CEC Enabled: %s", app->hdmi_tx.snps_hdmi_ctrl.cec_on ? " Yes" : "No");
	CMD_LOGGER(SNPS_NOTICE, "CEC Physical Address: %x", app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.physical_address);
	CMD_LOGGER(SNPS_NOTICE, "CEC Logical Address: %d", app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.logical_address);
	CMD_LOGGER(SNPS_NOTICE, "CEC Active Source: %s", app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.active_source ? " Yes" : "No");
	CMD_LOGGER(SNPS_NOTICE, "CEC Power state: %s", app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.power_state == ON ?
			"Power ON": app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.power_state == STANDBY ? "Standby" : "Transition Power On/Off");
	CMD_LOGGER(SNPS_NOTICE, "CEC Current Language: %s", app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.language);
}

void cec_debug_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	CMD_LOGGER(SNPS_NOTICE, "IH_CEC_STAT0  	0x00000418 0x%x\n", dev_read( &app->hdmi_tx, 0x00000418));
	CMD_LOGGER(SNPS_NOTICE, "CEC_CTRL	0x0001F400 0x%x\n", dev_read( &app->hdmi_tx, CEC_CTRL       ));
	CMD_LOGGER(SNPS_NOTICE, "CEC_MASK	0x0001F408 0x%x\n", dev_read( &app->hdmi_tx, CEC_MASK       ));
	CMD_LOGGER(SNPS_NOTICE, "CEC_ADDR_L	0x0001F414 0x%x\n", dev_read( &app->hdmi_tx, CEC_ADDR_L     ));
	CMD_LOGGER(SNPS_NOTICE, "CEC_ADDR_H	0x0001F418 0x%x\n", dev_read( &app->hdmi_tx, CEC_ADDR_H     ));
	CMD_LOGGER(SNPS_NOTICE, "CEC_TX_CNT	0x0001F41C 0x%x\n", dev_read( &app->hdmi_tx, CEC_TX_CNT     ));
	CMD_LOGGER(SNPS_NOTICE, "CEC_RX_CNT	0x0001F420 0x%x\n", dev_read( &app->hdmi_tx, CEC_RX_CNT     ));
	CMD_LOGGER(SNPS_NOTICE, "CEC_TX_DATA	0x0001F440 0x%x\n", dev_read( &app->hdmi_tx, CEC_TX_DATA));
	CMD_LOGGER(SNPS_NOTICE, "CEC_RX_DATA	0x0001F480 0x%x\n", dev_read( &app->hdmi_tx, CEC_RX_DATA));
	CMD_LOGGER(SNPS_NOTICE, "CEC_LOCK	0x0001F4C0 0x%x\n", dev_read( &app->hdmi_tx, CEC_LOCK       ));
	CMD_LOGGER(SNPS_NOTICE, "CEC_WAKEUPCTRL  0x0001F4C4 0x%x\n", dev_read( &app->hdmi_tx, CEC_WAKEUPCTRL));
}
