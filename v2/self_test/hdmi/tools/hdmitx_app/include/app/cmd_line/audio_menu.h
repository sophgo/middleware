/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef INCLUDE_APP_CMD_LINE_AUDIO_MENU_H_
#define INCLUDE_APP_CMD_LINE_AUDIO_MENU_H_

#include "app/cmd_line/cmd_interface.h"
#include "app/cmd_line/audio_cmd.h"
#include "includes.h"

extern  menu_t main_menu[];

static menu_t audio_menu[] = {
	{"/audio",          "Audio Parameters",                         BANNER_MARK, NULL,      NULL,               NULL, 0},
	{"SamplingFreq",    "<kHz|Help> - Sampling frequency",          ACTION,      NULL,      sample_freq_cmd,    NULL, 0},
	{"InterfaceType",   "<I2S|SPDIF|AHBDMA> - Interface type",      ACTION,      NULL,      interface_type_cmd, NULL, 0},
	{"CodingType",      "<1-11|Help> - Coding type",                ACTION,      NULL,      coding_type_cmd,    NULL, 0},
	{"SampleSize",      "<16-24> - Sample size",                    ACTION,      NULL,      sample_size_cmd,    NULL, 0},
	{"FSFactor",        "<64|128|256|512> - Audio Clock FS factor", ACTION,      NULL,      fs_factor_cmd,      NULL, 0},
	{"Show",            "Show current and new configurations ",     ACTION,      NULL,      audio_show_cmd,     NULL, 0},
	{"Apply",           "Apply configurations ",                    ACTION,      NULL,      audio_apply_cmd,    NULL, 0},
	{"Help",            "This help ",                               ACTION,      NULL,      help_cmd,           NULL, 0},
	{"Back",            "goes to the previous menu",                EXIT_MENU,   main_menu, NULL,               NULL, 0},
	{NULL, NULL, END_MARK, NULL, NULL, NULL, 0}
};

#endif /* INCLUDE_APP_CMD_LINE_AUDIO_MENU_H_ */
