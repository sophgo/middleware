/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "includes.h"
#include "hdmi_tx_app.h"

#define CMD_SIZE 500

typedef struct cmd_list_s{
	const char *command;
	const char *params;
	struct cmd_list_s *first;
	struct cmd_list_s *next;
}cmd_list_t;

typedef enum command_type{
	END_MARK = 0,
	BANNER_MARK,
	MENU,
	ACTION,
	EXIT_MENU,
}command_type_t;

typedef enum command_flags{
	CMD_EXPERT = 1,
	CMD_EXTERN = 2,
	CMD_INVISIBLE = 4,
}command_flags_t;


/**
 * Menu structure
 */
typedef struct menu_s{
	const char *command;
	const char *description;
	enum command_type type;
	struct menu_s *sub_menu;
	void (*command_function)(struct hdmi_tx_app *, cmd_list_t *);
	void (*command_help)();
	enum command_flags flags;
} menu_t;


/***********************************************************
 * Function prototypes
 */
void set_app_mode(struct hdmi_tx_app *app,  cmd_list_t * cmd);
app_mode_t get_app_mode(struct hdmi_tx_app *app);

void read_core_reg(uint32_t reg, uint32_t *value);

void write_core_reg(uint32_t reg, uint32_t value);

void phy_read_reg(uint32_t reg, uint32_t *value);

void phy_write_reg(uint32_t reg, uint32_t value);


#endif /* COMMANDS_H_ */
