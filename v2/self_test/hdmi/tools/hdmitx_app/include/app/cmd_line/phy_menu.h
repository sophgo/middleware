/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef INCLUDE_PHY_MENU_H_
#define INCLUDE_PHY_MENU_H_

#include "app/cmd_line/cmd_interface.h"
#include "app/cmd_line/phy_cmd.h"
#include "includes.h"

extern  menu_t main_menu[];

static menu_t phy_menu[] = {
	{"/phy",          "PHY Parameters",                 						 	BANNER_MARK, NULL,      NULL,              		NULL, 					0},
	{"PhyID",         "Display Phy Id",                  							ACTION,      NULL,      phy_id_cmd,        		phy_id_cmd_help, 		0},
	{"PhyInterface",  "[I2C|JTAG",             	     								ACTION,      NULL,      phy_interface_cmd, 		phy_interface_cmd_help, 0},
	{"PhyRead",       "<addr> - Read from PHY",          							ACTION,      NULL,      phy_read_cmd,      		phy_read_cmd_help, 		0},
	{"PhyWrite",      "<addr> <value> - Write to PHY",   							ACTION,      NULL,      phy_write_cmd,     		phy_write_cmd_help, 	0},
	{"OutputDelay",   "<delay(0-31)> - each step 78ps (minimum 0ps, maximum 2418ps)", ACTION,    NULL,      phy_odelay_cmd,   		phy_odelay_cmd_help, 	0},
	{"OutputDelayCal","<step(0-31)> ",   											ACTION,      NULL,      phy_odelay_calibration_cmd,     phy_odelay_calibration_cmd_help, 0},
	{"Help",          "This help ",                      							ACTION,      NULL,      help_cmd,          		NULL, 					0},
	{"Back",          "goes to the previous menu",       							EXIT_MENU,   main_menu, NULL,              		NULL, 					0},
	{NULL, NULL, END_MARK, NULL, NULL, NULL, 0}
};


#endif /* INCLUDE_PHY_MENU_H_ */
