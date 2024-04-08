// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "core/audio_i2s.h"

#include "audio/audio_sample_reg.h"
#include "bsp/access.h"
#include "util/log.h"

void _audio_i2s_reset_fifo(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	dev_write_mask(dev, AUD_CONF0, AUD_CONF0_SW_AUDIO_FIFO_RST_MASK, 1);
}

void _audio_i2s_data_enable(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	dev_write_mask(dev, AUD_CONF0, AUD_CONF0_I2S_IN_EN_MASK, value);
}

void _audio_i2s_data_mode(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	dev_write_mask(dev, AUD_CONF1, AUD_CONF1_I2S_MODE_MASK, value);
}

void _audio_i2s_data_width(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	dev_write_mask(dev, AUD_CONF1, AUD_CONF1_I2S_WIDTH_MASK, value);
}

void audio_i2s_select(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);
	dev_write_mask(dev, AUD_CONF0, AUD_CONF0_I2S_SELECT_MASK, bit);
}

void audio_i2s_interrupt_mask(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	dev_write_mask(dev, AUD_INT, AUD_INT_FIFO_FULL_MASK_MASK |
								 AUD_INT_FIFO_EMPTY_MASK_MASK, value);
}

void audio_i2s_configure(hdmi_tx_dev_t * dev, audioParams_t * audio)
{
	audio_i2s_select(dev, 1);

	_audio_i2s_data_enable(dev, 0xf);

	/* ATTENTION: fixed I2S data mode (standard) */
	_audio_i2s_data_mode(dev, 0x0);
	_audio_i2s_data_width(dev, audio->mSampleSize);
	audio_i2s_interrupt_mask(dev, 3);
	_audio_i2s_reset_fifo(dev);
}
