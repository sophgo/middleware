/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */
 
#ifdef PHY_THIRD_PARTY

#ifndef PHYACK_CMD_H_
#define PHYACK_CMD_H_

#include "commands.h"

void phyack_id_cmd_help(void);
void phyack_id_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void phyack_coldboot_cmd_help(void);
void phyack_coldboot_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void phyack_softreset_cmd_help(void);
void phyack_softreset_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void phyack_swing_cmd_help(void);
void phyack_swing_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void phyack_pll_cmd_help(void);
void phyack_pll_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void phyack_read_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void phyack_write_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void phyack_config_cmd_help(void);
void phyack_config_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void phyack_regs_cmd_help(void);
void phyack_regs_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

#endif /* PHYACK_CMD_H_ */

#endif
