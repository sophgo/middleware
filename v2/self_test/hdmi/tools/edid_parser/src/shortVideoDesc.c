/*
 * shortVideoDesc.c
 *
 *  Created on: Jul 22, 2010
 *
 *  Synopsys Inc.
 *  SG DWC PT02
 */

#include "short_video_desc.h"


void shortVideoDesc_Reset(hdmi_tx_dev_t *dev, shortVideoDesc_t * svd)
{
	svd->mNative = FALSE;
	svd->mCode = 0;
}

int svd_parse(hdmi_tx_dev_t *dev, shortVideoDesc_t * svd, u8 data)
{
	shortVideoDesc_Reset(dev, svd);
	svd->mNative = (bit_field(data, 7, 1) == 1) ? TRUE : FALSE;
	svd->mCode = bit_field(data, 0, 7);
	svd->mLimitedToYcc420 = 0;
	svd->mYcc420 = 0;
	return TRUE;
}

unsigned shortVideoDesc_GetCode(hdmi_tx_dev_t *dev, shortVideoDesc_t * svd)
{
	return svd->mCode;
}

int shortVideoDesc_GetNative(hdmi_tx_dev_t *dev, shortVideoDesc_t * svd)
{
	return svd->mNative;
}
