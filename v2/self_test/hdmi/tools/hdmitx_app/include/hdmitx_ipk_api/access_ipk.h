/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "includes.h"
#include "hdmi_tx_app.h"

/**
 * Access Functions
 */
int hdmitx_initialize(void);

int hdmitx_disable(void);

void hdmitx_write(u32 addr, u32 data);
u32 hdmitx_read(u32 addr);
