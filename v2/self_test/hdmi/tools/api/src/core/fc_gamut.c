// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "frame_composer/frame_composer_reg.h"
#include "core/fc_gamut.h"
#include "bsp/access.h"
#include "util/log.h"


#define FC_GMD_PB_SIZE 			28

void fc_gamut_Profile(hdmi_tx_dev_t *dev, u8 profile)
{
	LOG_TRACE1(profile);
	dev_write_mask(dev, FC_GMD_HB,FC_GMD_HB_GMDGBD_PROFILE_MASK, profile);
}

void fc_gamut_AffectedSeqNo(hdmi_tx_dev_t *dev, u8 no)
{
	LOG_TRACE1(no);
	dev_write_mask(dev, FC_GMD_HB, FC_GMD_HB_GMDAFFECTED_GAMUT_SEQ_NUM_MASK, no);
}

void fc_gamut_PacketsPerFrame(hdmi_tx_dev_t *dev, u8 packets)
{
	LOG_TRACE1(packets);
	dev_write_mask(dev, FC_GMD_CONF, FC_GMD_CONF_GMDPACKETSINFRAME_MASK, packets);
}

void fc_gamut_PacketLineSpacing(hdmi_tx_dev_t *dev, u8 lineSpacing)
{
	LOG_TRACE1(lineSpacing);
	dev_write_mask(dev, FC_GMD_CONF, FC_GMD_CONF_GMDPACKETLINESPACING_MASK, lineSpacing);
}

void fc_gamut_Content(hdmi_tx_dev_t *dev, const u8 * content, u8 length)
{
	u8 i = 0;
	LOG_TRACE1(content[0]);
	if (length > (FC_GMD_PB_SIZE)) {
		length = (FC_GMD_PB_SIZE);
		LOGGER(SNPS_WARN,"Gamut Content Truncated");
	}

	for (i = 0; i < length; i++)
		dev_write(dev, FC_GMD_PB0 + (i*4), content[i]);
}

void fc_gamut_enable_tx(hdmi_tx_dev_t *dev, u8 enable)
{
	LOG_TRACE1(enable);
	if(enable)
		enable = 1; // ensure value is 1
	dev_write_mask(dev, FC_GMD_EN, FC_GMD_EN_GMDENABLETX_MASK, enable);
}

void fc_gamut_UpdatePacket(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	dev_write_mask(dev, FC_GMD_UP, FC_GMD_UP_GMDUPDATEPACKET_MASK, 1);
}

u8 fc_gamut_CurrentSeqNo(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	return (u8)(dev_read(dev, FC_GMD_STAT) & 0xF);
}

u8 fc_gamut_PacketSeq(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	return (u8)(((dev_read(dev, FC_GMD_STAT)) >> 4) & 0x3);
}

u8 fc_gamut_NoCurrentGbd(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	return (u8)(((dev_read(dev, FC_GMD_STAT)) >> 7) & 0x1);
}

void fc_gamut_config(hdmi_tx_dev_t *dev)
{
	// P0
	fc_gamut_Profile(dev, 0x0);

	// P0
	fc_gamut_PacketsPerFrame(dev, 0x1);
	fc_gamut_PacketLineSpacing(dev, 0x1);
}

void fc_gamut_packet_config(hdmi_tx_dev_t *dev, const u8 * gbdContent, u8 length)
{
	fc_gamut_enable_tx(dev, 1);
	fc_gamut_AffectedSeqNo(dev, (fc_gamut_CurrentSeqNo(dev) + 1) % 16); /* sequential */
	fc_gamut_Content(dev, gbdContent, length);
	fc_gamut_UpdatePacket(dev); /* set next_field to 1 */
}
