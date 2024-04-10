/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef SHORTVIDEODESC_H_
#define SHORTVIDEODESC_H_

#include "../hdmitx_dev.h"
#include "util/types.h"

/**
 * @file
 * Short Video Descriptor.
 * Parse and hold Short Video Descriptors found in Video Data Block in EDID.
 */
/** For detailed handling of this structure, refer to documentation of the functions */
typedef struct {
	int 	mNative;

	unsigned mCode;

	unsigned mLimitedToYcc420;

	unsigned mYcc420;

} shortVideoDesc_t;

int svd_parse(hdmi_tx_dev_t *dev, shortVideoDesc_t * svd, u8 data);

#endif	/* SHORTVIDEODESC_H_ */
