// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "util/bit_operation.h"
#include "util/log.h"
#include "edid/hdmiforumvsdb.h"

void hdmiforumvsdb_reset(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb)
{
	forumvsdb->mValid = FALSE;
	forumvsdb->mIeee_Oui = 0;
	forumvsdb->mVersion = 0;
	forumvsdb->mMaxTmdsCharRate = 0;
	forumvsdb->mSCDC_Present = FALSE;
	forumvsdb->mRR_Capable = FALSE;
	forumvsdb->mLTS_340Mcs_scramble = FALSE;
	forumvsdb->mIndependentView = FALSE;
	forumvsdb->mDualView = FALSE;
	forumvsdb->m3D_OSD_Disparity = FALSE;
	forumvsdb->mDC_30bit_420 = FALSE;
	forumvsdb->mDC_36bit_420 = FALSE;
	forumvsdb->mDC_48bit_420 = FALSE;
}

int hdmiforumvsdb_parse(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb, u8 * data)
{
	u16 blockLength;
	LOG_TRACE();
	hdmiforumvsdb_reset(dev, forumvsdb);
	if (data == 0) {
		return FALSE;
	}
	if (bit_field(data[0], 5, 3) != 0x3) {
		LOGGER(SNPS_ERROR,"Invalid datablock tag");
		return FALSE;
	}
	LOGGER(SNPS_DEBUG,"block TAG ok");
	blockLength = bit_field(data[0], 0, 5);
	if (blockLength < 7) {
		LOGGER(SNPS_ERROR,"Invalid minimum length");
		return FALSE;
	}
	LOGGER(SNPS_DEBUG,"size ok");
	if (byte_to_dword(0x00, data[3], data[2], data[1]) != 0xC45DD8) {
		LOGGER(SNPS_ERROR,"HDMI IEEE registration identifier not valid");
		return FALSE;
	}
	LOGGER(SNPS_DEBUG,"ieeeid TAG ok");
	forumvsdb->mVersion = bit_field(data[4], 0, 7);
	forumvsdb->mMaxTmdsCharRate = bit_field(data[5], 0, 7);
	forumvsdb->mSCDC_Present = bit_field(data[6], 7, 1);
	forumvsdb->mRR_Capable = bit_field(data[6], 6, 1);
	forumvsdb->mLTS_340Mcs_scramble = bit_field(data[6], 3, 1);
	forumvsdb->mIndependentView = bit_field(data[6], 2, 1);
	forumvsdb->mDualView = bit_field(data[6], 1, 1);
	forumvsdb->m3D_OSD_Disparity = bit_field(data[6], 0, 1);
	forumvsdb->mDC_48bit_420 = bit_field(data[7], 2, 1);
	forumvsdb->mDC_36bit_420 = bit_field(data[7], 1, 1);
	forumvsdb->mDC_30bit_420 = bit_field(data[7], 0, 1);
	forumvsdb->mValid = TRUE;

#if 1
	LOGGER(SNPS_INFO,"version %d", bit_field(data[4], 0, 7));
	LOGGER(SNPS_INFO,"Max_TMDS_Charater_rate %d", bit_field(data[5], 0, 7));
	LOGGER(SNPS_INFO,"SCDC_Present %d", bit_field(data[6], 7, 1));
	LOGGER(SNPS_INFO,"RR_Capable %d", bit_field(data[6], 6, 1));
	LOGGER(SNPS_INFO,"LTE_340Mcsc_scramble %d", bit_field(data[6], 3, 1));
	LOGGER(SNPS_INFO,"Independent_View %d", bit_field(data[6], 2, 1));
	LOGGER(SNPS_INFO,"Dual_View %d", bit_field(data[6], 1, 1));
	LOGGER(SNPS_INFO,"3D_OSD_Disparity %d", bit_field(data[6], 0, 1));
	LOGGER(SNPS_INFO,"DC_48bit_420 %d", bit_field(data[7], 2, 1));
	LOGGER(SNPS_INFO,"DC_36bit_420 %d", bit_field(data[7], 1, 1));
	LOGGER(SNPS_INFO,"DC_30bit_420 %d", bit_field(data[7], 0, 1));
#endif
	return TRUE;
}
