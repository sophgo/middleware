/*
 * monitorRangeLimits.c
 *
 *  Created on: Jul 22, 2010
 *
 *  Synopsys Inc.
 *  SG DWC PT02
 */

#include "monitor_range_limits.h"
#include "bit_operation.h"


void monitor_range_limits_reset(hdmi_tx_dev_t *dev, monitorRangeLimits_t * mrl)
{
	mrl->mMinVerticalRate = 0;
	mrl->mMaxVerticalRate = 0;
	mrl->mMinHorizontalRate = 0;
	mrl->mMaxHorizontalRate = 0;
	mrl->mMaxPixelClock = 0;
	mrl->mValid = FALSE;
}

int monitorRangeLimits_Parse(hdmi_tx_dev_t *dev, monitorRangeLimits_t * mrl, u8 * data)
{
	
	monitor_range_limits_reset(dev, mrl);
	if (data != 0 && data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0xFD && data[4] == 0x00) {	/* block tag */
		mrl->mMinVerticalRate = data[5];
		mrl->mMaxVerticalRate = data[6];
		mrl->mMinHorizontalRate = data[7];
		mrl->mMaxHorizontalRate = data[8];
		mrl->mMaxPixelClock = data[9];
		mrl->mValid = TRUE;
		return TRUE;
	}
	return FALSE;
}

u8 monitorRangeLimits_GetMaxHorizontalRate(hdmi_tx_dev_t *dev, monitorRangeLimits_t * mrl)
{
	return mrl->mMaxHorizontalRate;
}

u8 monitorRangeLimits_GetMaxPixelClock(hdmi_tx_dev_t *dev, monitorRangeLimits_t * mrl)
{
	return mrl->mMaxPixelClock;
}

u8 monitorRangeLimits_GetMaxVerticalRate(hdmi_tx_dev_t *dev, monitorRangeLimits_t * mrl)
{
	return mrl->mMaxVerticalRate;
}

u8 monitorRangeLimits_GetMinHorizontalRate(hdmi_tx_dev_t *dev, monitorRangeLimits_t * mrl)
{
	return mrl->mMinHorizontalRate;
}

u8 monitorRangeLimits_GetMinVerticalRate(hdmi_tx_dev_t *dev, monitorRangeLimits_t * mrl)
{
	return mrl->mMinVerticalRate;
}
