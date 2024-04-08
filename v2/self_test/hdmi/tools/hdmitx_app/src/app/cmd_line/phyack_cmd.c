// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifdef PHY_THIRD_PARTY

#include "app/cmd_line/phy_cmd.h"
#include "app/cmd_line/phyack_cmd.h"
#include "app/cmd_line/cmd_interface.h"

#include "util/general_ops.h"
#include "phy/phy.h"
#include "phy/phy_ack.h"

void phyack_id_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	u32 addr = 0x20;
	u32 value;

	if(app == NULL){
		LOGGER(SNPS_ERROR, "Improper arguments");
		return;
	}

	if((app->hdmi_tx_phy != PHY_MODEL_THIRD_PARTY_ACK) || (app->hdmi_tx_phy != PHY_MODEL_THIRD_PARTY_ANGELCREEK)){
		printf("ERROR: System not configured for ThirdParty Phy Antcreek\n");
		return;
	}

	if(phy_read(&app->hdmi_tx, (u16) addr, &value) == 0)
		printf("Antcreek Id: 0x%x\n",value & 0xF);
	else
		printf("ERROR: Antcreek access failed\n");
}

void phyack_id_cmd_help(void)
{
	printf("PhyAckID display Phy ID\n");
}


void phyack_coldboot_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	u32 freq 	= 0;
	double sus_clk  = 0.0;
	char *value_token = NULL;
	int error = 0;

	if((app == NULL) || (cmd  == NULL)){
		printf("ERROR: Improper arguments");
		return;
	}

	if(cmd->params == NULL){
		printf("ERROR: Invalid command parameters");
		phyack_coldboot_cmd_help();
		return;
	}

	value_token = strsep((char **)&cmd->params, " ");
	freq = strtol(value_token, NULL, 10);
	if(!phyack_freq_supported(freq)){
		printf("ERROR: Frequency %d not supported\n", freq);
		phyack_coldboot_cmd_help();
		return;
	}

	value_token = strsep((char **)&cmd->params, " ");
	sus_clk = strtod(value_token, NULL);
	if(double_is_equal(sus_clk, 19.2) || (sus_clk < 25)){
		printf("ERROR: Sus Clk value %.3f not supported\n", sus_clk);
		phyack_coldboot_cmd_help();
		return;
	}

	error =  phyack_powerup(&app->hdmi_tx, freq, sus_clk);

	if(error)
		printf("** Antcreek cold boot sequence failed **\n");
	else
		printf("Antcreek cold boot sequence complete\n");
}


void phyack_coldboot_cmd_help(void)
{
	printf("AckColdBoot <freq 25-540> <sus_clk 19.2|25>\n");
}

void phyack_softreset_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	if((app == NULL) || (cmd  == NULL)){
		printf("ERROR: Improper arguments");
		phyack_softreset_cmd_help();
		return;
	}

	if(cmd->params == NULL){
		printf("ERROR: Invalid command parameters");
		phyack_softreset_cmd_help();
		return;
	}

	if(command_match(cmd->params, "Enable")){
		printf("Antcreek soft reset enable");
		phyack_softreset(&app->hdmi_tx, 1);
	}
	else if(command_match(cmd->params, "Disable")){
		printf("Antcreek soft reset disable");
		phyack_softreset(&app->hdmi_tx, 0);
	}else {
		printf("ERROR: Invalid option");
		phyack_softreset_cmd_help();
	}
}

void phyack_softreset_cmd_help(void)
{
	printf("AckSoftReset <Enable|Disable>\n");
}

void phyack_swing_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	u32 voltage  = 0;
	double gain  = 0.0;
	char *value_token = NULL;
	int error = 0;

	if((app == NULL) || (cmd  == NULL)){
		printf("ERROR: Improper arguments\n");
		return;
	}

	if(cmd->params == NULL){
		printf("ERROR: Invalid command parameters\n");
		phyack_swing_cmd_help();
		return;
	}

	value_token = strsep((char **)&cmd->params, " ");
	if(value_token == NULL){
		printf("ERROR: Invalid command parameters\n");
		phyack_swing_cmd_help();
		return;
	}

	voltage = strtol(value_token, NULL, 10);

	value_token = strsep((char **)&cmd->params, " ");
	gain = strtod(value_token, NULL);

	if(!phyack_swing_config_supported(voltage, gain)){
		printf("ERROR: Parameters invalid. Voltage %dmV Gain %.1fdB\n", voltage, gain);
		phyack_swing_cmd_help();
		return;
	}

	error =  phyack_swing_prog(&app->hdmi_tx, voltage, gain);

	if(error)
		printf("** Antcreek swing program failed **\n");
	else
		printf("Antcreek swing program complete\n");
}


void phyack_swing_cmd_help(void)
{
	printf("AckSwing <voltage> <gain>\n");
	printf("\t\t  - <400mV|600mV|800mV|1200mV> <0.0dB>\n");
	printf("\t\t  - <400mV|600mV|800mV> <3.5dB>\n");
	printf("\t\t  - <400mV|600mV> <6dB>\n");
	printf("\t\t  - <400mV> <9.5dB>\n");
}

void phyack_pll_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	double refclock  = 0.0;
	double pClk  	 = 0.0;
	char *value_token = NULL;

	if((app == NULL) || (cmd  == NULL)){
		printf("ERROR: Improper arguments\n");
		return;
	}

	if(cmd->params == NULL){
		printf("ERROR: Invalid command parameters\n");
		phyack_pll_cmd_help();
		return;
	}

	value_token = strsep((char **)&cmd->params, " ");
	refclock = strtod(value_token, NULL);

	value_token = strsep((char **)&cmd->params, " ");
	pClk = strtod(value_token, NULL);

	if(!phyact_pll_config_supported(refclock, pClk)){
		printf("ERROR: Parameters invalid. Reference Clock %.3fMHz Data rate %.3fMHz\n", refclock, pClk);
		phyack_pll_cmd_help();
		return;
	}

	if(phyack_configure(&app->hdmi_tx, refclock, pClk))
		printf("Antcreek PLL program complete\n");
	else
		printf("** Antcreek PLL program failed **\n");
}


void phyack_pll_cmd_help(void)
{
	printf("AckPLL <ref clock> <data rate>\n");
	printf("\t\t reference clock - <19.2|26|38.4|100>\n");
	printf("\t\t data rate - <25 - 297>\n");
}

void phyack_read_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	if(phy_get_interface(&app->hdmi_tx) != PHY_EXTERN){
		printf("ERROR: PHY interface not defined for external phy's\n");
		return;
	}

	phy_read_cmd(app, cmd);
}

void phyack_write_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	if(phy_get_interface(&app->hdmi_tx) != PHY_EXTERN){
		printf("ERROR: PHY interface not defined for external phy's\n");
		return;
	}

	phy_write_cmd(app, cmd);
}

void phyack_config_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	char *token = NULL;
	char *value_token = NULL;
	int voltage;
	double sus_clock; //Mhz
	double gain;
	double pwclock; //Mhz
	double refclock;

	if(app == NULL){
		printf("ERROR: Improper arguments");
		phyack_config_cmd_help();
		return;
	}

	if((cmd  == NULL) || (cmd->params == NULL)){
		printf("PHY ThirdParty Antcreek: Voltage     %d\n",    app->hdmi_tx.ack_phy.voltage);
		printf("                    Gain        %2.1f\n", app->hdmi_tx.ack_phy.gain);
		printf("                    Sus Clock   %2.3f\n", app->hdmi_tx.ack_phy.sus_clock);
		printf("                    Power Clock %2.3f\n", app->hdmi_tx.ack_phy.pwclock);
		printf("                    Ref Clock   %2.3f\n", app->hdmi_tx.ack_phy.refclock);
		printf("                    Data rate   %2.3f\n", app->hdmi_tx.ack_phy.datarate);
	//	printf("               Status %d\n", app->hdmi_tx.ack_phy.datarate);
		return;
	}

	voltage = app->hdmi_tx.ack_phy.voltage;
	sus_clock = app->hdmi_tx.ack_phy.sus_clock;
	gain = app->hdmi_tx.ack_phy.gain;
	pwclock = app->hdmi_tx.ack_phy.pwclock;
	refclock = app->hdmi_tx.ack_phy.refclock;

	while(cmd->params != NULL){
		token = strsep((char **)&cmd->params, " ");
		value_token = strsep((char **)&cmd->params, " ");

		if(token == NULL || value_token == NULL){
			printf("ERROR: Invalid options\n");
			phyack_config_cmd_help();
			return;
		}

		if(command_match(token, "Voltage")){
			voltage = strtol(value_token, NULL, 10);
			continue;
		}
		if(command_match(token, "Gain")){
			gain = strtod(value_token, NULL);
			continue;
		}
		if(command_match(token, "SusClock")){
			sus_clock = strtod(value_token, NULL);
			continue;
		}
		if(command_match(token, "PowerClock")){
			pwclock = strtod(value_token, NULL);
			continue;
		}
		if(command_match(token, "RefClock")){
			refclock = strtod(value_token, NULL);
			continue;
		}

		printf("ERROR: Invalid option %s", token);
		return;
	}
	if(double_is_equal(sus_clock, 19.2) || (sus_clock < 25)){
		printf("ERROR: Sus Clock value %.3f not supported\n", sus_clock);
		phyack_config_cmd_help();
		return;
	}


	if(!phyack_swing_config_supported(voltage, gain)){
		printf("ERROR: Value pair not supported, voltage %dmV gain %.3fdB\n", voltage, gain);
		phyack_config_cmd_help();
		return;
	}

	if(!phyact_pll_config_supported(refclock, 25.0)){
		printf("ERROR: Reference Clock %.3fMHz not supported\n", refclock);
		phyack_config_cmd_help();
		return;
	}

	app->hdmi_tx.ack_phy.voltage = voltage;
	app->hdmi_tx.ack_phy.sus_clock = sus_clock;
	app->hdmi_tx.ack_phy.gain = gain;
	app->hdmi_tx.ack_phy.pwclock = pwclock;
	app->hdmi_tx.ack_phy.refclock = refclock;

	phyack_config_cmd(app, NULL);

	return;
}

void phyack_config_cmd_help(void)
{
	printf("AckCONFig [Voltage|Gain|SusClock|PowerClock|RefClock] [value]\n");
}

void phyack_regs_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	struct register_table * phy_write;
	int i = 0;

	if(app == NULL){
		LOGGER(SNPS_ERROR, "Improper arguments");
		phyack_regs_cmd_help();
		return;
	}

	if( app->hdmi_tx.ack_phy.phy_sim != NULL)
		phy_write = app->hdmi_tx.ack_phy.phy_sim;
	else if(app->hdmi_tx.ack_phy.phy_write == NULL)
		phy_write = app->hdmi_tx.ack_phy.phy_write;
	else {
		printf("Not supported with debug disable\n");
		return;
	}

	printf("ThirdParty Phy programmed registers\n");

	for(i = 0; i<0x2000; i++){
		if(phy_write[i].status)
			printf("\t[0x%06x] = 0x%08x\n", i, phy_write[i].value);
	}
	return;
}

void phyack_regs_cmd_help(void)
{
	printf("AckREGs \n");
}

#endif
