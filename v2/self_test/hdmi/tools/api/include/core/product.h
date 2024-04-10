/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef PRODUCTPARAMS_H_
#define PRODUCTPARAMS_H_

#include "../hdmitx_dev.h"
#include "util/types.h"

/** For detailed handling of this structure, refer to documentation of the functions */
typedef struct {
	/* Vendor Name of eight 7-bit ASCII characters */
	u8 mVendorName[8];

	u8 mVendorNameLength;

	/* Product name or description, consists of sixteen 7-bit ASCII characters */
	u8 mProductName[16];

	u8 mProductNameLength;

	/* Code that classifies the source device (CEA Table 15) */
	u8 mSourceType;

	/* oui 24 bit IEEE Registration Identifier */
	u32 mOUI;

	u8 mVendorPayload[24];

	u8 mVendorPayloadLength;

} productParams_t;

void product_reset(hdmi_tx_dev_t *dev, productParams_t * product);

u8 product_SetProductName(hdmi_tx_dev_t *dev, productParams_t *product, const u8 *data, u8 length);

u8 product_SetVendorName(hdmi_tx_dev_t *dev, productParams_t *product, const u8 *data, u8 length);

u8 product_SetVendorPayload(hdmi_tx_dev_t *dev, productParams_t *product, const u8 *data, u8 length);

u8 product_IsSourceProductValid(hdmi_tx_dev_t *dev, productParams_t *product);

u8 product_IsVendorSpecificValid(hdmi_tx_dev_t *dev, productParams_t *product);

#endif	/* PRODUCTPARAMS_H_ */
