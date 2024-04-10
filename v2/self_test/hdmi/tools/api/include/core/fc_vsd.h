/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HALFRAMECOMPOSERVSD_H_
#define HALFRAMECOMPOSERVSD_H_

#include "../hdmitx_dev.h"
#include "util/types.h"

/*
 * Configure the 24 bit IEEE Registration Identifier
 * @param baseAddr Block base address
 * @param id vendor unique identifier
 */
void fc_vsd_vendor_OUI(hdmi_tx_dev_t *dev, u32 id);

/*
 * Configure the Vendor Payload to be carried by the InfoFrame
 * @param info array
 * @param length of the array
 * @return 0 when successful and 1 on error
 */
u8 fc_vsd_vendor_payload(hdmi_tx_dev_t *dev, const u8 * data, unsigned short length);

#endif				/* HALFRAMECOMPOSERVSD_H_ */
