/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef INCLUDE_SCDC_MENU_H_
#define INCLUDE_SCDC_MENU_H_

#include "app/cmd_line/cmd_interface.h"
#include "app/cmd_line/scdc_cmd.h"
#include "includes.h"


extern  menu_t main_menu[];

static menu_t scdc_menu[] = {
	{"/scdc",    "SCDC operations",                         		BANNER_MARK, NULL,      NULL,            			NULL, 							0},
	{"scdcRR",   "<ENable|DIsable|Show> Read Request support",   	ACTION,      NULL,      scdc_enable_rr_cmd, 	 	scdc_enable_rr_help_cmd, 		0},
	{"TestRR",   "<delayms>",   									ACTION,      NULL,      scdc_test_rr_cmd,  			scdc_test_rr_help_cmd, 			0},
	{"ScrambStat", "Scrambling Status",   							ACTION,      NULL,      scdc_scrambling_stat_cmd,  	scdc_scrambling_stat_help_cmd, 	0},
	{"SCRAMble", "<Enable|Disable> Scrambling ",  					ACTION,      NULL,      scdc_scrambling_enable_cmd, scdc_scrambling_enable_help_cmd,0},
	{"ReaD",     "<addr> [size]",   								ACTION,      NULL,      scdc_read_cmd,  			scdc_read_help_cmd, 			0},
	{"WRite",    "<addr> <data>",   								ACTION,      NULL,      scdc_write_cmd, 			scdc_write_help_cmd, 			0},
	{"ReadCounters",     "Read rx counters and checksum",		ACTION,      NULL,      scdc_read_cnt_cmd, 			scdc_read_cnt_help_cmd, 			0},
	{"Help",     "This help ",                               		ACTION,      NULL,      help_cmd,        			NULL, 							0},
	{"Back",     "goes to the previous menu",                		EXIT_MENU,   main_menu, NULL,            			NULL, 							0},
	{NULL, NULL, END_MARK, NULL, NULL, NULL, 0}
};


#endif /* INCLUDE_SCDC_MENU_H_ */
