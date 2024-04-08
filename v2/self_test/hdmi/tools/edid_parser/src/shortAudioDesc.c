/*
 * shortAudioDesc.c
 *
 *  Created on: Jul 22, 2010
 *
 *  Synopsys Inc.
 *  SG DWC PT02
 */
#include "short_audio_desc.h"
#include "bit_operation.h"


void sad_reset(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	sad->mFormat = 0;
	sad->mMaxChannels = 0;
	sad->mSampleRates = 0;
	sad->mByte3 = 0;
}

int sad_parse(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad, u8 * data)
{
	
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

u8 shortAudioDesc_GetByte3(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	return sad->mByte3;
}

u8 shortAudioDesc_GetFormatCode(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	return sad->mFormat;
}

u8 shortAudioDesc_GetMaxChannels(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	return sad->mMaxChannels;
}

u8 shortAudioDesc_GetSampleRates(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	return sad->mSampleRates;
}

int sad_support32k(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 0, 1) ==
		1) ? TRUE : FALSE;
}

int sad_support44k1(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 1, 1) ==
		1) ? TRUE : FALSE;
}

int sad_support48k(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 2, 1) ==
		1) ? TRUE : FALSE;
}

int sad_support88k2(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 3, 1) ==
		1) ? TRUE : FALSE;
}

int sad_support96k(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 4, 1) ==
		1) ? TRUE : FALSE;
}

int sad_support176k4(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 5, 1) ==
		1) ? TRUE : FALSE;
}

int sad_support192k(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 6, 1) ==
		1) ? TRUE : FALSE;
}

u8 shortAudioDesc_GetMaxBitRate(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	if (sad->mFormat > 1 && sad->mFormat < 9) {
		return sad->mByte3;
	}
	LOGGER(SNPS_NOTICE,"Information is not valid for this format");
	return 0;
}

int sad_support16bit(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	if (sad->mFormat == 1) {
		return (bit_field(sad->mByte3, 0, 1) ==
			1) ? TRUE : FALSE;
	}
	LOGGER(SNPS_NOTICE,"Information is not valid for this format");
	return FALSE;
}

int sad_support20bit(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	if (sad->mFormat == 1) {
		return (bit_field(sad->mByte3, 1, 1) ==
			1) ? TRUE : FALSE;
	}
	LOGGER(SNPS_NOTICE,"Information is not valid for this format");
	return FALSE;
}

int sad_support24bit(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad)
{
	if (sad->mFormat == 1) {
		return (bit_field(sad->mByte3, 2, 1) ==
			1) ? TRUE : FALSE;
	}
	LOGGER(SNPS_NOTICE,"Information is not valid for this format");
	return FALSE;
}
