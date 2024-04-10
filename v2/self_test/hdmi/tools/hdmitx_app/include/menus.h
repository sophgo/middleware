/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef MENUS_H_
#define MENUS_H_

#include "app/cmd_line/audio_cmd.h"
#include "app/cmd_line/cmd_interface.h"
#include "app/cmd_line/expert_cmd.h"
#include "app/cmd_line/hdcp_cmd.h"
#include "app/cmd_line/phy_cmd.h"
#include "app/cmd_line/video_cmd.h"
#include "includes.h"

extern  menu_t main_menu[];

/***********************************************************
 * Command declaration
 */

static menu_t hdcp_menu[] = {
	{"/hdcp",     "HDCP Parameters",                                                              BANNER_MARK, NULL,      NULL,                NULL, 0},
	{"Configure", "<Enable|Disable>  - bypass HDCP encryption",                                   ACTION,      NULL,      hdcp_enable_cmd,     hdcp_enable_help_cmd, 0},
	{"ByPass",    "<Enable|Disable>  - bypass HDCP encryption",                                   ACTION,      NULL,      hdcp_bypass_cmd,     hdcp_bypass_help_cmd, 0},
	{"Feature11", "<Enable|Disable>  - features 1.1 control",                                     ACTION,      NULL,      hdcp_feature11_cmd,  hdcp_feature11_help_cmd, 0},
	{"RICheck",   "<2sec|128f> - Ri check for 2sec even or 128 encrypted frames",                 ACTION,      NULL,      hdcp_richeck_cmd,    hdcp_richeck_help_cmd, 0},
	{"I2C",       "<Fast|Slow>  - I2C fast mode",                                                 ACTION,      NULL,      hdcp_i2c_cmd,        hdcp_i2c_help_cmd, 0},
	{"ELV",       "<Enable|Disable>  - Enhanced Link Verification",                               ACTION,      NULL,      hdcp_elv_cmd,        hdcp_elv_help_cmd, 0},
	{"NumDevices","<number>   - number of supported devices",                                     ACTION,      NULL,      hdcp_numdevices_cmd, hdcp_numdevices_help_cmd, 0},
	{"Show" ,     "<Cfg|Pending> - Show configured or pending configuration",                     ACTION,      NULL,      hdcp_show_cmd,       hdcp_show_help_cmd, 0},
	{"Help",      "This help ",                                                                   ACTION,      NULL,      help_cmd,            NULL, 0},
	{"Back",      "goes to the previous menu",                                                    EXIT_MENU,   main_menu, NULL,                NULL, 0},
	{NULL, NULL, END_MARK, NULL, NULL, NULL}
};

static menu_t expert_menu[] = {
	{"/expert",         "Expert mode",                                      BANNER_MARK, NULL,      NULL,                 NULL, 0},
	{"EdidTx",          "<Enable|Disable>  - EDID TX checks",               ACTION,      NULL,      edid_tx_check_cmd,    NULL, 0},
	{"EdidTxReload",    "Reload EDID TX capabilities",                      ACTION,      NULL,      edid_tx_reload_cmd,   NULL, 0},
	{"DumpEDID",        "Dump Rx EDID",                                     ACTION,      NULL,      edid_rx_dump_cmd,     NULL, 0},
	{"PatternReLoad",   "<RGB|YCC444|YCC422|YCC420> - Reload pattern",      ACTION,      NULL,      pattern_reload,       NULL, 0},
	{"ChessPattern",    "<Enable|Disable> - Chess board pattern",           ACTION,      NULL,      chess_pattern_cmd,    NULL, 0},
	{"PatternLoading",  "<Enable|Disable> - Pattern loading upon HPD",      ACTION,      NULL,      disable_pattern_load, NULL, 0},
	{"SupportedVicList","Print supported VIC list for HDMI TX",             ACTION,      NULL,      svd_tx_list_load_cmd, NULL, 0},
	{"Read",            "<addr> - read address",                            ACTION,      NULL,      hdmitx_read_cmd,      hdmitx_read_cmd_help, 0},
	{"Write",           "<addr> <value> - write value to address",          ACTION,      NULL,      hdmitx_write_cmd,     hdmitx_write_cmd_help, 0},
	{"Clear",           "Activate I2C bus clear function",                  ACTION,      NULL,      i2c_bus_clear_cmd,    NULL, 0},
	{"Help",            "This help ",                                       ACTION,      NULL,      help_cmd,             NULL, 0},
	{"Back",            "goes to the previous menu",                        EXIT_MENU,   main_menu, NULL,                 NULL, 0},
	{NULL, NULL, END_MARK, NULL, NULL, NULL}
};

#endif /* MENUS_H_ */
