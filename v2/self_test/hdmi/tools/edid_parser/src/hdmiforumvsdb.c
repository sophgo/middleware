/*
 * hdmiforumvsdb.c
 *
 *  Created on: Jun 4, 2014
 *      Author: 
 */
#include "bit_operation.h"
#include "hdmiforumvsdb.h"

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
	if (byte_to_dword(0x00, data[3], data[2], data[1]) !=
	    0xC45DD8) {
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
	forumvsdb->mDC_30bit_420 = bit_field(data[7], 2, 1);
	forumvsdb->mDC_36bit_420 = bit_field(data[7], 1, 1);
	forumvsdb->mDC_48bit_420 = bit_field(data[7], 0, 1);
	forumvsdb->mValid = TRUE;

#if 1
	LOGGER(SNPS_NOTICE,"version %d", bit_field(data[4], 0, 7));
	LOGGER(SNPS_NOTICE,"Max_TMDS_Charater_rate %d",
		    bit_field(data[5], 0, 7));
	LOGGER(SNPS_NOTICE,"SCDC_Present %d", bit_field(data[6], 7, 1));
	LOGGER(SNPS_NOTICE,"RR_Capable %d", bit_field(data[6], 6, 1));
	LOGGER(SNPS_NOTICE,"LTE_340Mcsc_scramble %d",
		    bit_field(data[6], 3, 1));
	LOGGER(SNPS_NOTICE,"Independent_View %d", bit_field(data[6], 2, 1));
	LOGGER(SNPS_NOTICE,"Dual_View %d", bit_field(data[6], 1, 1));
	LOGGER(SNPS_NOTICE,"3D_OSD_Disparity %d", bit_field(data[6], 0, 1));
	LOGGER(SNPS_NOTICE,"DC_48bit_420 %d", bit_field(data[7], 2, 1));
	LOGGER(SNPS_NOTICE,"DC_36bit_420 %d", bit_field(data[7], 1, 1));
	LOGGER(SNPS_NOTICE,"DC_30bit_420 %d", bit_field(data[7], 0, 1));
#endif
	return TRUE;
}

int hdmiforumvsdb_GetDeepColor30_420(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb)
{
	return forumvsdb->mDC_30bit_420;
}

int hdmiforumvsdb_GetDeepColor36_420(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb)
{
	return forumvsdb->mDC_36bit_420;
}

int hdmivsdb_GetDeepColor48_420(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb)
{
	return forumvsdb->mDC_48bit_420;
}

int hdmivsdb_GetSCDC_Present(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb)
{
	return forumvsdb->mSCDC_Present;
}

int hdmivsdb_GetDual_View(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb)
{
	return forumvsdb->mDualView;
}

u8 hdmivsdb_GetMaxTmdsCharRate(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb)
{
	return forumvsdb->mMaxTmdsCharRate;
}

u16 hdmivsdb_Get3D_OSD_Disparity(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb)
{
	return forumvsdb->m3D_OSD_Disparity;
}

u16 hdmivsdb_GetVersion(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb)
{
	return forumvsdb->mVersion;
}

u16 hdmivsdb_GetIndependent_View(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb)
{
	return forumvsdb->mIndependentView;
}

u16 hdmivsdb_GetRR_Capable(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb)
{
	return forumvsdb->mRR_Capable;
}

u16 hdmivsdb_GetLTS_340Msc_scramble(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb)
{
	return forumvsdb->mLTS_340Mcs_scramble;
}
