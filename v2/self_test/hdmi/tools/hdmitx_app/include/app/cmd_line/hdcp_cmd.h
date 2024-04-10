/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef INCLUDE_HDCP_CMD_H_
#define INCLUDE_HDCP_CMD_H_

#include "commands.h"

void hdcp_bypass_help_cmd();
void hdcp_bypass_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void hdcp_feature11_help_cmd();
void hdcp_feature11_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void hdcp_richeck_help_cmd();
void hdcp_richeck_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void hdcp_i2c_help_cmd();
void hdcp_i2c_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void hdcp_elv_help_cmd();
void hdcp_elv_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void hdcp_numdevices_help_cmd();
void hdcp_numdevices_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void hdcp_show_help_cmd();
void hdcp_show_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void hdcp_enable_help_cmd();
void hdcp_enable_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void debug_help_cmd();
void debug_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void print_hdcpinfo(hdcpParams_t * phdcp);

#endif /* INCLUDE_HDCP_CMD_H_ */
