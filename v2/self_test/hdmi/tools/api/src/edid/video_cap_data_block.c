// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "edid/video_cap_data_block.h"
#include "util/bit_operation.h"
#include "util/log.h"

void video_cap_data_block_reset(hdmi_tx_dev_t *dev, videoCapabilityDataBlock_t * vcdb)
{
	vcdb->mQuantizationRangeSelectable = FALSE;
	vcdb->mPreferredTimingScanInfo = 0;
	vcdb->mItScanInfo = 0;
	vcdb->mCeScanInfo = 0;
	vcdb->mValid = FALSE;
}

int video_cap_data_block_parse(hdmi_tx_dev_t *dev, videoCapabilityDataBlock_t * vcdb, u8 * data)
{
	LOG_TRACE();
	video_cap_data_block_reset(dev, vcdb);
	/* check tag code and extended tag */
	if ((data != 0) && (bit_field(data[0], 5, 3) == 0x7) &&
		(bit_field(data[1], 0, 8) == 0x0) && (bit_field(data[0], 0, 5) == 0x2)) {	/* so far VCDB is 2 bytes long */
		vcdb->mCeScanInfo = bit_field(data[2], 0, 2);
		vcdb->mItScanInfo = bit_field(data[2], 2, 2);
		vcdb->mPreferredTimingScanInfo = bit_field(data[2], 4, 2);
		vcdb->mQuantizationRangeSelectable = (bit_field(data[2], 6, 1) == 1) ? TRUE : FALSE;
		vcdb->mValid = TRUE;
		return TRUE;
	}
	return FALSE;
}
