/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef VIDEOCAPABILITYDATABLOCK_H_
#define VIDEOCAPABILITYDATABLOCK_H_

#include "../hdmitx_dev.h"
#include "util/types.h"

/**
 * @file
 * Video Capability Data Block.
 * (videoCapabilityDataBlock_t * vcdbCEA Data Block Tag Code 0).
 * Parse and hold information from EDID data structure.
 * For detailed handling of this structure, refer to documentation of the functions
 */

typedef struct {
	int mQuantizationRangeSelectable;

	u8 mPreferredTimingScanInfo;

	u8 mItScanInfo;

	u8 mCeScanInfo;

	int mValid;
} videoCapabilityDataBlock_t;

void video_cap_data_block_reset(hdmi_tx_dev_t *dev, videoCapabilityDataBlock_t * vcdb);

int video_cap_data_block_parse(hdmi_tx_dev_t *dev, videoCapabilityDataBlock_t * vcdb, u8 * data);

#endif	/* VIDEOCAPABILITYDATABLOCK_H_ */
