// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "core/video_sampler.h"
#include "bsp/access.h"
#include "util/log.h"
#include "video/video_sampler_reg.h"


void halVideoSampler_InternalDataEnableGenerator(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);
	dev_write_mask(dev, TX_INVID0, TX_INVID0_INTERNAL_DE_GENERATOR_MASK, (bit ? 1 : 0));
}

void halVideoSampler_VideoMapping(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	dev_write_mask(dev, TX_INVID0, TX_INVID0_VIDEO_MAPPING_MASK, value);
}

void halVideoSampler_StuffingGy(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	dev_write(dev, (TX_GYDATA0), (u8) (value >> 0));
	dev_write(dev, (TX_GYDATA1), (u8) (value >> 8));
	dev_write_mask(dev, TX_INSTUFFING, TX_INSTUFFING_GYDATA_STUFFING_MASK, 1);
}

void halVideoSampler_StuffingRcr(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	dev_write(dev, (TX_RCRDATA0), (u8) (value >> 0));
	dev_write(dev, (TX_RCRDATA1), (u8) (value >> 8));
	dev_write_mask(dev, TX_INSTUFFING, TX_INSTUFFING_RCRDATA_STUFFING_MASK, 1);
}

void halVideoSampler_StuffingBcb(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	dev_write(dev, (TX_BCBDATA0), (u8) (value >> 0));
	dev_write(dev, (TX_BCBDATA1), (u8) (value >> 8));
	dev_write_mask(dev, TX_INSTUFFING, TX_INSTUFFING_BCBDATA_STUFFING_MASK, 1);
}

void video_sampler_config(hdmi_tx_dev_t *dev, u8 map_code)
{
	halVideoSampler_InternalDataEnableGenerator(dev, 0);
	halVideoSampler_VideoMapping(dev, map_code);
	halVideoSampler_StuffingGy(dev, 0);
	halVideoSampler_StuffingRcr(dev, 0);
	halVideoSampler_StuffingBcb(dev, 0);
}
