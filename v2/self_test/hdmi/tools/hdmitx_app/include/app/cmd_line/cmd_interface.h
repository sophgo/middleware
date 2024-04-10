/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef COMMAND_INTERFACE_H_
#define COMMAND_INTERFACE_H_

#include "commands.h"
#include "includes.h"
#include "hdmi_tx_app.h"


#define CMD_LOGGER(level, format, ...)  \
	do { if (1) {printf(format, ##__VA_ARGS__); printf("\n"); }} while (0)

/**
 * Gets the character from the command line.
 * Returns 0 to quit the application and 1 when a command is available
 */
int get_cmd_line(struct hdmi_tx_app *app, char *cmd);

/**
 * Parses the command string into a command list (cmd_list_t)
 * Returns 1 if parsing successful
 * Returns 0 if parsing is not successful
 */
cmd_list_t * parse_cmd_line(struct hdmi_tx_app *app, char *cmd);
void free_cmd_list(cmd_list_t *cmd_list);
/**
 *
 */
int execute_cmd_list(struct hdmi_tx_app *app, cmd_list_t *cmd_list);
int execute_cmd_old(struct hdmi_tx_app *app, menu_t *cmd_menu, cmd_list_t *cmd);

int get_string_caps(const char * string, char * caps);
int command_match(const char * command,const char * operation);


void help_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void expert_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

#endif /* COMMAND_INTERFACE_H_ */
