/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef COLORIMETRYDATABLOCK_H_
#define COLORIMETRYDATABLOCK_H_

#include "../hdmitx_dev.h"
#include "util/types.h"

/**
 * @file
 * Colorimetry Data Block class.
 * Holds and parses the Colorimetry data-block information.
 */

typedef struct {
	u8 mByte3;

	u8 mByte4;

	int mValid;

} colorimetryDataBlock_t;

void colorimetry_data_block_reset(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

int colorimetry_data_block_parse(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb, u8 * data);

int supports_xv_ycc709(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

int supports_xv_ycc601(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

int supports_s_ycc601(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

int supports_adobe_ycc601(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

int supports_adobe_rgb(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

int supports_metadata0(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

int supports_metadata1(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

int supports_metadata2(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

int supports_metadata3(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

#endif	/* COLORIMETRYDATABLOCK_H_ */
