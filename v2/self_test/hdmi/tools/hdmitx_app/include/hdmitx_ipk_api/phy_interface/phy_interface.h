/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef PHY_INTERFACE_H_
#define PHY_INTERFACE_H_

#include "includes.h"
#include "hdmi_tx_app.h"

void phy_if_write(struct hdmi_tx_app *app, uint32_t reg, uint32_t data);

uint32_t phy_if_read(struct hdmi_tx_app *app, uint32_t reg);

int phy_data_output_delay(uint32_t lines,  uint32_t delay);

uint32_t phy_data_output_delay_get();

#endif /* PHY_INTERFACE_H_ */
