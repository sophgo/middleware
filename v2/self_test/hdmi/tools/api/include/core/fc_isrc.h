/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HALFRAMECOMPOSERISRC_H_
#define HALFRAMECOMPOSERISRC_H_

#include "../hdmitx_dev.h"
#include "util/types.h"


/*
 * Configure the ISRC packet status
 * @param code
 * 001 - Starting Position
 * 010 - Intermediate Position
 * 100 - Ending Position
 * @param baseAddr block base address
 */
void fc_isrc_status(hdmi_tx_dev_t *dev, u8 code);

/*
 * Configure the validity bit in the ISRC packets
 * @param validity: 1 if valid
 * @param baseAddr block base address
 */
void fc_isrc_valid(hdmi_tx_dev_t *dev, u8 validity);

/*
 * Configure the cont bit in the ISRC1 packets
 * When a subsequent ISRC2 Packet is transmitted, the ISRC_Cont field shall be set and shall be clear otherwise.
 * @param isContinued 1 when set
 * @param baseAddr block base address
 */
void fc_isrc_cont(hdmi_tx_dev_t *dev, u8 isContinued);

/*
 * Configure the ISRC 1 Codes
 * @param codes
 * @param length
 * @param baseAddr block base address
 */
void fc_isrc_isrc1_codes(hdmi_tx_dev_t *dev, u8 * codes, u8 length);

/*
 * Configure the ISRC 2 Codes
 * @param codes
 * @param length
 * @param baseAddr block base address
 */
void fc_isrc_isrc2_codes(hdmi_tx_dev_t *dev, u8 * codes, u8 length);

#endif	/* HALFRAMECOMPOSERISRC_H_ */
