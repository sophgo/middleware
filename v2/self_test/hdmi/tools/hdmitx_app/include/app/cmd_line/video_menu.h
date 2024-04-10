/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef INCLUDE_VIDEO_MENU_H_
#define INCLUDE_VIDEO_MENU_H_

#include "app/cmd_line/cmd_interface.h"
#include "app/cmd_line/video_cmd.h"
#include "app/cmd_line/phy_cmd.h"
#include "includes.h"


extern  menu_t main_menu[];

static menu_t video_menu[] = {
	{"/video",          "Video Parameters",                                         BANNER_MARK,NULL,      NULL,                        NULL,                       0},
	{"SinkSupportedVic","Get sink supported VIC - through EDID",                    ACTION,     NULL,      sink_vic_cmd,                NULL,                       0},
	{"SVD",             "<1-107> [refresh in Hz] - Short Video Descriptor",	        ACTION,     NULL,      svd_cmd,                     NULL,                       0},
	{"Mode",            "<HDMI|DVI> - Video mode HDMI or DVI",                      ACTION,     NULL,      mode_cmd,                    NULL,                       0},
	{"3D",              "[<None|FramePacking|TopBottom|SideBySideHalf>]",           ACTION,     NULL,      video3d_cmd,                 video3d_cmd_help,           0},
	{"ColorDepth", 		"<8|10|12|16> - number of bits per color component",        ACTION,     NULL,      color_resolution_cmd,        NULL,                       0},
	{"Encoding",        "<rgb|ycc444|ycc422|ycc420> - Encoding",                    ACTION,     NULL,      encoding_cmd,                NULL,                       0},
//	{"OutputDelay",   	"<delay(0-31)> - each step 78ps (minimum 0ps, maximum 2418ps)", ACTION, NULL,      phy_odelay_cmd,              phy_odelay_cmd_help,        0},
//	{"OutputDelayCal",  "<step(0-31)> ",                                            ACTION,     NULL,      phy_odelay_calibration_cmd,  phy_odelay_calibration_cmd_help, 0},
//	{"ReportStatus",    "Log User report on video mode status",                     ACTION,     NULL,      video_report_cmd,            video_report_cmd_help,      0},
	{"Show",            "Show current and new configurations ",                     ACTION,     NULL,      video_show_cmd,              NULL,                       0},
	{"Apply",           "Apply configurations ",                                    ACTION,     NULL,      video_apply_cmd,             NULL,                       0},
	{"AutoTest",        "Sweep all supported VICs/Color Depth/Encodings",           ACTION,     NULL,      video_autotest_cmd,          NULL,                       0},
	{"CustomTest",      "Sweep a custom set of VICs/Color Depth/Encodings",         ACTION,     NULL,      video_customtest_cmd,        NULL,                       0},
	{"Help",            "This help ",                                               ACTION,     NULL,      help_cmd,                    NULL,                       0},
	{"Back",            "goes to the previous menu",                                EXIT_MENU,  main_menu, NULL,                        NULL,                       0},
	{NULL, NULL, END_MARK, NULL, NULL, NULL, 0}
};

#endif /* INCLUDE_VIDEO_MENU_H_ */
