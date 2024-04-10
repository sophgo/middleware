/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef INCLUDE_APP_HDCP_HDCP_H_
#define INCLUDE_APP_HDCP_HDCP_H_

#include "includes.h"
#include "hdmi_tx_app.h"

void reset_hdcp_params(struct hdmi_tx_app *app);

void update_hdcp_cfg(hdcpParams_t * user, hdcpParams_t * cfg);

void hdcp_init_keys(struct hdmi_tx_app *app);

#endif /* INCLUDE_APP_HDCP_HDCP_H_ */
