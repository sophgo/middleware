// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "core/audio_spdif.h"
#include "core/audio_i2s.h"
#include "audio/audio_sample_spdif_reg.h"
#include "bsp/access.h"
#include "util/log.h"

void _audio_spdif_reset_fifo(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	dev_write_mask(dev, AUD_SPDIF0, AUD_SPDIF0_SW_AUDIO_FIFO_RST_MASK, 1);
}

void _audio_spdif_non_linear_pcm(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);
	dev_write_mask(dev, AUD_SPDIF1, AUD_SPDIF1_SETNLPCM_MASK, bit);

	// Put HBR mode disabled
	// TODO: check if this is the better place for this
	dev_write_mask(dev, AUD_SPDIF1, AUD_SPDIF1_SPDIF_HBR_MODE_MASK, bit);
}

void _audio_spdif_data_width(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	dev_write_mask(dev, AUD_SPDIF1, AUD_SPDIF1_SPDIF_WIDTH_MASK, value);
}

void audio_spdif_interrupt_mask(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	dev_write_mask(dev, AUD_SPDIFINT, AUD_SPDIFINT_SPDIF_FIFO_FULL_MASK_MASK |
									  AUD_SPDIFINT_SPDIF_FIFO_EMPTY_MASK_MASK, value);
}

void _audio_spdif_enable_inputs(hdmi_tx_dev_t *dev, u8 inputs)
{
	LOG_TRACE1(inputs);
	dev_write_mask(dev, AUD_SPDIF2, AUD_SPDIF2_IN_EN_MASK, inputs);
}

void audio_spdif_config(hdmi_tx_dev_t *dev, audioParams_t *audio)
{
	// Disable I2S
	audio_i2s_select(dev, 0);

	_audio_spdif_reset_fifo(dev);
	audio_spdif_interrupt_mask(dev, 3);
	_audio_spdif_enable_inputs(dev, 0xF);
	_audio_spdif_non_linear_pcm(dev, (audio->mCodingType == PCM) ? 0 : 1);
//	_audio_spdif_data_width(dev, audio->mSampleSize);
}
