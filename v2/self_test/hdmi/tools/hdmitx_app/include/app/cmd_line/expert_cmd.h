/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef EXPERT_CMD_H_
#define EXPERT_CMD_H_

#include "commands.h"

void edid_tx_check_help(void);
void edid_tx_check_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void edid_tx_reload_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void edid_rx_dump_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void pattern_reload(struct hdmi_tx_app *app, cmd_list_t *cmd);

void chess_pattern_help(void);
void chess_pattern_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void disable_pattern_load_help(void);
void disable_pattern_load(struct hdmi_tx_app *app, cmd_list_t *cmd);

void svd_tx_list_load_help(void);
void svd_tx_list_load_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void hdmitx_read_cmd_help(void);
void hdmitx_read_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void hdmitx_write_cmd_help(void);
void hdmitx_write_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void i2c_bus_clear_cmd_help(void);
void i2c_bus_clear_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

#endif /* EXPERT_CMD_H_ */
