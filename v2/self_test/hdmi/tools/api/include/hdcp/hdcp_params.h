/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HDCPPARAMS_H_
#define HDCPPARAMS_H_
#include "../hdmitx_dev.h"
#include "util/types.h"

void hdcp_params_reset(hdmi_tx_dev_t *dev, hdcpParams_t * params);

#endif	/* HDCPPARAMS_H_ */
