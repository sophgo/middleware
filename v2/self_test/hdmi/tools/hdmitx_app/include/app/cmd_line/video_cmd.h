/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef INCLUDE_VIDEO_CMD_H_
#define INCLUDE_VIDEO_CMD_H_

#include "commands.h"

void mode_help(void);
void mode_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void svd_help(void);
void svd_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void video3d_cmd_help(void);
void video3d_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void color_resolution_help(void);
void color_resolution_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void pixel_clock_help(void);
void pixel_clock_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void encoding_help(void);
void encoding_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void sink_vic_help(void);
void sink_vic_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void video_report_cmd_help(void);
void video_report_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void video_show_help(void);
void video_show_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void video_apply_help(void);
void video_apply_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void video_autotest_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);
void video_customtest_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

#endif /* INCLUDE_VIDEO_CMD_H_ */
