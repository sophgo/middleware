/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef INCLUDE_REG_MENU_H_
#define INCLUDE_REG_MENU_H_

#include "app/cmd_line/cmd_interface.h"
#include "app/cmd_line/reg_cmd.h"
#include "includes.h"

extern  menu_t main_menu[];

static menu_t reg_menu[] = {
	{"/reg",                  "REG Parameters",              BANNER_MARK, NULL,      NULL,           NULL,    0},
	{"reg_scan",              "Scan Registers",              ACTION,      NULL,      reg_scan_cmd,   NULL,    0},
	{"Help",                  "This help ",                  ACTION,      NULL,      help_cmd,       NULL,    0},
	{"Back",                  "goes to the previous menu",   EXIT_MENU,   main_menu, NULL,           NULL,    0},
	{NULL, NULL, END_MARK, NULL, NULL, NULL, 0},
};


#endif /* INCLUDE_REG_MENU_H_ */
