/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HDCP_H_
#define HDCP_H_

#include "includes.h"
#include "hdmi_tx_app.h"

void hdcp_write(struct hdmi_tx_app *app, uint32_t data, uint32_t reg);

uint32_t hdcp_read(struct hdmi_tx_app *app, uint32_t reg);

#endif /* HDCP_H_ */
