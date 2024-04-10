/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef INCLUDE_APP_SIGNAL_HANDLER_H_
#define INCLUDE_APP_SIGNAL_HANDLER_H_

#include "hdmitx.h"

/**
 * Signal handling functions
 *
 */
void dwc_hdmi_tx_handler(int signum);

void video_brigde_handler(int signum);

void audio_bridge_handler(int signum);

void tx_phy_if_handler(int signum);

void hdcp_handler(int signum);

void dwc_hdmi_tx_cec_handler(int signum);

int register_signal_handler(struct hdmi_tx_app *app);

int test_signal_handlers(struct hdmi_tx_app *app);

void hpd_event_handler(void *param);

void edid_event_handler(void *param);

void cec_handler();

#endif /* INCLUDE_APP_SIGNAL_HANDLER_H_ */
