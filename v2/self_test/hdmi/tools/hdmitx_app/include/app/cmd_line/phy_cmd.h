/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef PHY_CMD_H_
#define PHY_CMD_H_

#include "commands.h"

void phy_id_cmd_help(void);
void phy_id_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void phy_interface_cmd_help(void);
void phy_interface_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void phy_read_cmd_help(void);
void phy_read_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void phy_write_cmd_help(void);
void phy_write_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

void phy_odelay_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);
void phy_odelay_cmd_help(void);

void phy_odelay_calibration_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);
void phy_odelay_calibration_cmd_help(void);


#endif /* PHY_CMD_H_ */
