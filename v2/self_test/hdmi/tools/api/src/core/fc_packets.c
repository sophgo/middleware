// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "frame_composer/frame_composer_reg.h"
#include "core/fc_packets.h"
#include "bsp/access.h"
#include "util/log.h"

void fc_packets_QueuePriorityHigh(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	dev_write_mask(dev, FC_CTRLQHIGH, FC_CTRLQHIGH_ONHIGHATTENDED_MASK, value);
}

void fc_packets_QueuePriorityLow(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	dev_write_mask(dev, FC_CTRLQLOW, FC_CTRLQLOW_ONLOWATTENDED_MASK, value);
}

void fc_packets_MetadataFrameInterpolation(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	dev_write_mask(dev, FC_DATAUTO1, FC_DATAUTO1_AUTO_FRAME_INTERPOLATION_MASK, value);
}

void fc_packets_MetadataFramesPerPacket(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	dev_write_mask(dev, FC_DATAUTO2, FC_DATAUTO2_AUTO_FRAME_PACKETS_MASK, value);
}

void fc_packets_MetadataLineSpacing(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	dev_write_mask(dev, FC_DATAUTO2, FC_DATAUTO2_AUTO_LINE_SPACING_MASK, value);
}

void fc_packets_AutoSend(hdmi_tx_dev_t *dev, u8 enable, u8 mask)
{
	LOG_TRACE2(enable, mask);
	dev_write_mask(dev, FC_DATAUTO0, (1 << mask), (enable ? 1 : 0));
}

void fc_packets_ManualSend(hdmi_tx_dev_t *dev, u8 mask)
{
	LOG_TRACE1(mask);
	dev_write_mask(dev, FC_DATMAN, (1 << mask), 1);
}

void fc_packets_disable_all(hdmi_tx_dev_t *dev)
{
	uint32_t value = ~( BIT(ACP_TX) | BIT(ISRC1_TX) | BIT(ISRC2_TX) | BIT(SPD_TX) | BIT(VSD_TX) );
	LOG_TRACE();
	dev_write(dev, FC_DATAUTO0, value & dev_read(dev, FC_DATAUTO0));
}

void fc_packets_metadata_config(hdmi_tx_dev_t *dev)
{
	fc_packets_MetadataFrameInterpolation(dev, 1);
	fc_packets_MetadataFramesPerPacket(dev, 1);
	fc_packets_MetadataLineSpacing(dev, 1);
}
