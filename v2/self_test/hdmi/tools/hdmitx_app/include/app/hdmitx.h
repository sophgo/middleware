/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef INCLUDE_APP_HDMITX_H_
#define INCLUDE_APP_HDMITX_H_

#define SW_VERSION_MAJOR 2
#define SW_VERSION_MINOR 14
#define SW_VERSION_LETTER "a"

#include "hdmi_tx_app.h"
#include "includes.h"
#include "hdmitx_ipk_api/hdmitx_ipk_api.h"
#include "platform.h"

int hdmitx_init_app(struct hdmi_tx_app *app, int phy, char * device, char * edid_tx_cap_file, int interface, int verbose, int phy_debug);
int hdmitx_set_phy(struct hdmi_tx_app *app, int phy, int debug_mode);
int hdmitx_configure_vga(struct hdmi_tx_app *app);
int hdmitx_vga_init(struct hdmi_tx_app *app);

#endif /* INCLUDE_APP_HDMITX_H_ */
