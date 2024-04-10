/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "commands.h"

#ifndef TOOLS_HDMITX_APP_INCLUDE_APP_CMD_LINE_CEC_CMD_H_
#define TOOLS_HDMITX_APP_INCLUDE_APP_CMD_LINE_CEC_CMD_H_

void cec_enable_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);
void cec_standby_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);
void cec_onetouch_play_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);
void cec_setactive_source_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);
void cec_send_msg_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);
void cec_wait_receive_msg_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);
void cec_status_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);
void cec_debug_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);
void cec_getmenu_language_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);
void cec_setmenu_language_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);




#endif /* TOOLS_HDMITX_APP_INCLUDE_APP_CMD_LINE_CEC_CMD_H_ */
