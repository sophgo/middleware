// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include <hdmitx_dev.h>

#include "includes.h"
#include "hdmitx_ipk_api/hdmitx_ipk_api.h"
#include "hdmi_tx_app.h"
#include "platform.h"
#include <limits.h>
#include "menus.h"
#include "util/general_ops.h"
#include "app/cmd_line/audio_menu.h"
#include "app/cmd_line/video_menu.h"
#include "app/cmd_line/scdc_menu.h"
#include "app/cmd_line/cec_menu.h"
#include "app/cmd_line/phy_menu.h"
#include "app/cmd_line/reg_menu.h"
#include "app/cmd_line/phyack_menu.h"
#include "app/cmd_line/cmd_interface.h"
#include "app/cmd_line/main_cmd.h"


menu_t main_menu[] = {
	{"",       "Main Menu",                      BANNER_MARK, NULL,         NULL,     	NULL, 0},
	{"Video",  "Video configurations Menu",      MENU,        video_menu,   NULL,     	NULL, 0},
	{"Audio",  "Audio configurations Menu",      MENU,        audio_menu,   NULL,     	NULL, 0},
	{"HDCP",   "HDCP configurations Menu",       MENU,        hdcp_menu,    NULL,     	NULL, 0},
	{"SCDC",   "Scrambling configurations Menu", MENU,        scdc_menu,    NULL,     	NULL, 0},
	{"CEC",	   "CEC configurations Menu",		 MENU,        cec_menu,     NULL,     	NULL, 0},
	{"Phy",    "Phy configurations Menu",        MENU,        phy_menu,     NULL,       NULL, 0},
	{"Reg",    "Reg configurations Menu",        MENU,        reg_menu,     NULL,       NULL, 0},
#ifdef PHY_THIRD_PARTY
	{"ACK",    "ThirdParty PHY Antcreek",	     MENU,        phyack_menu,  NULL,       	NULL, CMD_EXTERN},
#endif
	{"EXpert", "EXpert configurations Menu",     MENU,        expert_menu,  NULL,     	NULL, 0},
//	{"EXPSNPS","Enable expert options",          ACTION,      NULL,         expert_cmd, 	NULL, CMD_INVISIBLE},
	{"Version","SW Version",		             ACTION,      NULL,      	sw_version_cmd,	NULL, 0},
	{"Help",   "Help",                           ACTION,      NULL,         help_cmd, 	NULL, 0},
	{"Exit",   "Exit application",               EXIT_MENU,   NULL,         NULL,     	NULL, 0},
	{NULL, NULL, END_MARK, NULL, NULL, NULL, 0},
};


/**
 * @short Print the SW version
 * @param[in,out] app Main Application structure
 * @param[in,out] cmd Command list
 * return void
 */
void sw_version_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	printf("HDMI Application SW version = %d.%d%c\n", app->sw_version_major,app->sw_version_minor,app->sw_version_letter[0]);
}

void main_apply_help_cmd(){
	printf("Apply existing user configuration");
}
