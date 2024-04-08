// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "frame_composer/frame_composer_reg.h"
#include "core/fc_audio_info.h"
#include "bsp/access.h"
#include "util/general_ops.h"
#include "util/log.h"


void fc_channel_count(hdmi_tx_dev_t *dev, u8 noOfChannels)
{
	LOG_TRACE1(noOfChannels);
	printf("noOfChannels:%d\n", noOfChannels);
	dev_write_mask(dev, FC_AUDICONF0, FC_AUDICONF0_CC_MASK, noOfChannels);
}

void fc_sample_freq(hdmi_tx_dev_t *dev, u8 sf)
{
	LOG_TRACE1(sf);
	dev_write_mask(dev, FC_AUDICONF1, FC_AUDICONF1_SF_MASK, sf);
}

void fc_allocate_channels(hdmi_tx_dev_t *dev, u8 ca)
{
	LOG_TRACE1(ca);
	printf("allocation channel:%d\n", ca);
	dev_write(dev, FC_AUDICONF2, ca);
}

void fc_level_shift_value(hdmi_tx_dev_t *dev, u8 lsv)
{
	LOG_TRACE1(lsv);
	dev_write_mask(dev, FC_AUDICONF3, FC_AUDICONF3_LSV_MASK, lsv);
}

void fc_down_mix_inhibit(hdmi_tx_dev_t *dev, u8 prohibited)
{
	LOG_TRACE1(prohibited);
	dev_write_mask(dev, FC_AUDICONF3, FC_AUDICONF3_DM_INH_MASK, (prohibited ? 1 : 0));
}

void fc_coding_type(hdmi_tx_dev_t *dev, u8 codingType)
{
	LOG_TRACE1(codingType);
	dev_write_mask(dev, FC_AUDICONF0, FC_AUDICONF0_CT_MASK, codingType);
}

void fc_sampling_size(hdmi_tx_dev_t *dev, u8 ss)
{
	LOG_TRACE1(ss);
	dev_write_mask(dev, FC_AUDICONF1, FC_AUDICONF1_SS_MASK, ss);
}

void fc_audio_info_config(hdmi_tx_dev_t *dev, audioParams_t * audio)
{
	u8 channel_count = audio_channel_count(dev, audio);
	LOG_TRACE();

	fc_channel_count(dev, channel_count);
	fc_allocate_channels(dev, audio->mChannelAllocation);
	fc_level_shift_value(dev, audio->mLevelShiftValue);
	fc_down_mix_inhibit(dev, audio->mDownMixInhibitFlag);

	LOGGER(SNPS_DEBUG, "Audio channel count = %d", channel_count);
	LOGGER(SNPS_DEBUG, "Audio channel allocation = %d", audio->mChannelAllocation);
	LOGGER(SNPS_DEBUG, "Audio level shift = %d", audio->mLevelShiftValue);

	if ((audio->mCodingType == ONE_BIT_AUDIO) || (audio->mCodingType == DST)) {
		double sampling_freq = audio->mSamplingFrequency;

		/* Audio InfoFrame sample frequency when OBA or DST */
		if (double_is_equal(sampling_freq, 32.000)) {
			fc_sample_freq(dev, 1);
		}
		else if (double_is_equal(sampling_freq, 44.100)) {
			fc_sample_freq(dev, 2);
		}
		else if (double_is_equal(sampling_freq, 48.000)) {
			fc_sample_freq(dev, 3);
		}
		else if (double_is_equal(sampling_freq, 88.200)) {
			fc_sample_freq(dev, 4);
		}
		else if (double_is_equal(sampling_freq, 96.000)) {
			fc_sample_freq(dev, 5);
		}
		else if (double_is_equal(sampling_freq, 176.400)) {
			fc_sample_freq(dev, 6);
		}
		else if (double_is_equal(sampling_freq, 192.000)) {
			fc_sample_freq(dev, 7);
		}
		else {
			fc_sample_freq(dev, 0);
		}
	} else {
		fc_sample_freq(dev, 0);	/* otherwise refer to stream header (0) */
	}

	fc_coding_type(dev, 1);	/* for HDMI refer to stream header  (0) */
	fc_sampling_size(dev, 24);	/* for HDMI refer to stream header  (0) */
}
