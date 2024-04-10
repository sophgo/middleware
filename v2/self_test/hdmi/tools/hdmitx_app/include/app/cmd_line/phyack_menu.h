/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifdef PHY_THIRD_PARTY

#ifndef INCLUDE_PHYACK_MENU_H_
#define INCLUDE_PHYACK_MENU_H_

#include "app/cmd_line/cmd_interface.h"
#include "app/cmd_line/phyack_cmd.h"
#include "app/cmd_line/phy_cmd.h"
#include "includes.h"

extern  menu_t main_menu[];

static menu_t phyack_menu[] = {
	{"/phyack",  "PHY ThirdParty Antcreek Parameters",   	BANNER_MARK, 				NULL,      NULL,                 NULL, 						0},
	{"AckID",        "Display Antcreek Id",             ACTION,      				NULL,      phyack_id_cmd,        phyack_id_cmd_help, 		0},
	{"AckCONFig",    "[Voltage|Gain|SusClock|PowerClock|RefClock] [value]",	ACTION, NULL,      phyack_config_cmd,    phyack_config_cmd_help, 	0},
	{"AckREGs",      "",		   	 					ACTION,      				NULL,      phyack_regs_cmd,      phyack_regs_cmd_help, 		0},
	{"AckColdBoot",  "<freq 25-540> <sus_clk 19.2|25>", ACTION,      				NULL,      phyack_coldboot_cmd,  phyack_coldboot_cmd_help, 	0},
	{"AckSoftReset", "<Enable|Disable>", 		    	ACTION,      				NULL,      phyack_softreset_cmd, phyack_softreset_cmd_help, 0},
	{"AckSwing",     "<voltage> <gain>", 		    	ACTION,      				NULL,      phyack_swing_cmd,  	 phyack_swing_cmd_help, 	0},
	{"AckPLL",       "<ref clock> <data rate>", 	    ACTION,      				NULL,      phyack_pll_cmd,  	 phyack_pll_cmd_help, 		0},
	{"PhyRead",      "<addr> - Read from PHY",          ACTION,      				NULL,      phyack_read_cmd,      phy_read_cmd_help, 		0},
	{"PhyWrite",     "<addr> <value> - Write to PHY",   ACTION,      				NULL,      phyack_write_cmd,     phy_write_cmd_help, 		0},
	{"Help",         "This help ",                      ACTION,      				NULL,      help_cmd,             NULL, 						0},
	{"Back",         "goes to the previous menu",       EXIT_MENU,   				main_menu, NULL,                 NULL, 						0},
	{NULL, NULL, END_MARK, NULL, NULL, NULL, 0}
};


#endif /* INCLUDE_PHYACK_MENU_H_ */

#endif
