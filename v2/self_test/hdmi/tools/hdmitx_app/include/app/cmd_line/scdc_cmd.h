/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef INCLUDE_SCDC_CMD_H_
#define INCLUDE_SCDC_CMD_H_

#include "commands.h"

void scdc_enable_rr_help_cmd();
void scdc_enable_rr_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void scdc_test_rr_help_cmd();
void scdc_test_rr_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void scdc_scrambling_stat_help_cmd();
void scdc_scrambling_stat_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void scdc_scrambling_enable_help_cmd();
void scdc_scrambling_enable_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void scdc_read_help_cmd();
void scdc_read_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void scdc_write_help_cmd();
void scdc_write_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void scdc_read_cnt_help_cmd();
void scdc_read_cnt_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

#endif /* INCLUDE_SCDC_CMD_H_ */
