// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "edid/short_video_desc.h"
#include "util/bit_operation.h"
#include "util/log.h"

void svd_reset(hdmi_tx_dev_t *dev, shortVideoDesc_t * svd)
{
	svd->mNative = FALSE;
	svd->mCode = 0;
}

int svd_parse(hdmi_tx_dev_t *dev, shortVideoDesc_t * svd, u8 data)
{
	svd_reset(dev, svd);
	svd->mNative = (bit_field(data, 7, 1) == 1) ? TRUE : FALSE;
	svd->mCode = bit_field(data, 0, 7);
	svd->mLimitedToYcc420 = 0;
	svd->mYcc420 = 0;
	return TRUE;
}
