/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HDMIFORUMVSDB_H_
#define HDMIFORUMVSDB_H_

#include "../hdmitx_dev.h"
#include "util/types.h"

/* HDMI 2.0 HF_VSDB */
typedef struct {
	u32 mIeee_Oui;

	u8 mValid;

	u8 mVersion;

	u8 mMaxTmdsCharRate;

	u8 m3D_OSD_Disparity;

	u8 mDualView;

	u8 mIndependentView;

	u8 mLTS_340Mcs_scramble;

	u8 mRR_Capable;

	u8 mSCDC_Present;

	u8 mDC_30bit_420;

	u8 mDC_36bit_420;

	u8 mDC_48bit_420;

} hdmiforumvsdb_t;

void hdmiforumvsdb_reset(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb);

int hdmiforumvsdb_parse(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb, u8 * data);

#endif	/* HDMIFORUMVSDB_H_ */
