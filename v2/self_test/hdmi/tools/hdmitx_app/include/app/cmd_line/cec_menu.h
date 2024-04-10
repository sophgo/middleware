/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef INCLUDE_CEC_MENU_H_
#define INCLUDE_CEC_MENU_H_

#include "app/cmd_line/cmd_interface.h"
#include "app/cmd_line/cec_cmd.h"
#include "includes.h"

extern  menu_t main_menu[];

static menu_t cec_menu[] = {
		{"/CEC",          		"CEC Configuration Menu",                   BANNER_MARK, NULL,      NULL,                 		NULL,  0},
		{"Cec",          		"<Enable|Disable>  - CEC Communication",    ACTION,      NULL,      cec_enable_cmd,    NULL, 0},
		{"StandBy",				"<Enable|Disable>  - CEC Standby Mode",     ACTION,      NULL,      cec_standby_cmd,    NULL, 0},
		{"OneTouchPlay",		"Send CEC OneTouchPlay Message",            ACTION,      NULL,      cec_onetouch_play_cmd,    NULL,    0},
		{"SetActiveSource",		"Enables Tx as CEC Active Source",          ACTION,      NULL,      cec_setactive_source_cmd,    NULL, 0},
		{"GetMenuLanguage",		"Send Get Menu Language Message",			ACTION,      NULL,      cec_getmenu_language_cmd,    NULL, 0},
		{"SetMenuLanguage",		"Changes Menu Language",					ACTION,      NULL,      cec_setmenu_language_cmd,    NULL, 0},
		//{"CecSend",          	"Sends CEC message",               			ACTION,      NULL,      cec_send_msg_cmd,    NULL, 0},
		//{"CecRxMsg",          "Wait and Receive CEC message",             ACTION,      NULL,      cec_wait_receive_msg_cmd,    NULL, 0},
		{"DeBug",	          	"CEC registers Status",               		ACTION,      NULL,      cec_debug_cmd,    NULL, 0},
		{"StaTus",          	"CEC Status",               				ACTION,      NULL,      cec_status_cmd,    NULL, 0},
		{"Help",            	"This help ",                               ACTION,      NULL,      help_cmd,             		NULL,  0},
		{"Back",            	"goes to the previous menu",                EXIT_MENU,   main_menu, NULL,                 		NULL,  0},
		{NULL, NULL, END_MARK, NULL, NULL, NULL, 0}
};

#endif /* TOOLS_HDMITX_APP_INCLUDE_APP_CMD_LINE_CEC_MENU_H_ */
