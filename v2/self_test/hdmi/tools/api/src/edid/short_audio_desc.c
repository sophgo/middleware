// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "edid/short_audio_desc.h"
#include "util/bit_operation.h"
#include "util/log.h"

void sad_reset(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	sad->mFormat = 0;
	sad->mMaxChannels = 0;
	sad->mSampleRates = 0;
	sad->mByte3 = 0;
}

int sad_parse(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad, u8 * data)
{
	LOG_TRACE();
	sad_reset(dev, sad);
	if (data != 0) {
		sad->mFormat = bit_field(data[0], 3, 4);
		sad->mMaxChannels = bit_field(data[0], 0, 3) + 1;
		sad->mSampleRates = bit_field(data[1], 0, 7);
		sad->mByte3 = data[2];
		return TRUE;
	}
	return FALSE;
}

int sad_support32k(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 0, 1) == 1) ? TRUE : FALSE;
}

int sad_support44k1(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 1, 1) == 1) ? TRUE : FALSE;
}

int sad_support48k(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 2, 1) == 1) ? TRUE : FALSE;
}

int sad_support88k2(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 3, 1) == 1) ? TRUE : FALSE;
}

int sad_support96k(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 4, 1) == 1) ? TRUE : FALSE;
}

int sad_support176k4(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 5, 1) == 1) ? TRUE : FALSE;
}

int sad_support192k(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 6, 1) == 1) ? TRUE : FALSE;
}

int sad_support16bit(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	if (sad->mFormat == 1) {
		return (bit_field(sad->mByte3, 0, 1) == 1) ? TRUE : FALSE;
	}
	LOGGER(SNPS_INFO,"Information is not valid for this format");
	return FALSE;
}

int sad_support20bit(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	if (sad->mFormat == 1) {
		return (bit_field(sad->mByte3, 1, 1) == 1) ? TRUE : FALSE;
	}
	LOGGER(SNPS_INFO,"Information is not valid for this format");
	return FALSE;
}

int sad_support24bit(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	if (sad->mFormat == 1) {
		return (bit_field(sad->mByte3, 2, 1) == 1) ? TRUE : FALSE;
	}
	LOGGER(SNPS_INFO,"Information is not valid for this format");
	return FALSE;
}
