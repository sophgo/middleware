/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef AUDIO_CMD_H_
#define AUDIO_CMD_H_

#include "commands.h"

void coding_type_help(void);
void coding_type_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void sample_freq_help(void);
void sample_freq_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void interface_type_help(void);
void interface_type_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void sample_size_help(void);
void sample_size_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void fs_factor_help(void);
void fs_factor_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void audio_show_help(void);
void audio_show_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void audio_apply_help(void);
void audio_apply_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

#endif /* AUDIO_CMD_H_ */
