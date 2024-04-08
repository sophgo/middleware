// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "video/video_packetizer_reg.h"
#include "core/video_packetizer.h"
#include "bsp/access.h"
#include "util/log.h"

u8 vp_PixelPackingPhase(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	return (u8)(dev_read(dev, VP_STATUS) & 0xF);
}

void vp_ColorDepth(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	/* color depth */
	dev_write_mask(dev, VP_PR_CD, VP_PR_CD_COLOR_DEPTH_MASK, value);
}

void vp_PixelPackingDefaultPhase(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);
	dev_write_mask(dev, VP_STUFF, VP_STUFF_IDEFAULT_PHASE_MASK, bit);
}

void vp_PixelRepetitionFactor(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	/* desired factor */
	dev_write_mask(dev, VP_PR_CD, VP_PR_CD_DESIRED_PR_FACTOR_MASK, value);
	/* enable stuffing */
	dev_write_mask(dev, VP_STUFF, VP_STUFF_PR_STUFFING_MASK, 1);
	/* enable block */
	dev_write_mask(dev, VP_CONF, VP_CONF_PR_EN_MASK, (value > 1) ? 1 : 0);
	/* bypass block */
	dev_write_mask(dev, VP_CONF, VP_CONF_BYPASS_SELECT_MASK, (value > 1) ? 0 : 1);
}

void vp_Ycc422RemapSize(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	dev_write_mask(dev, VP_REMAP, VP_REMAP_YCC422_SIZE_MASK, value);
}

void vp_OutputSelector(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	if (value == 0) {	/* pixel packing */
		dev_write_mask(dev, VP_CONF, VP_CONF_BYPASS_EN_MASK, 0);
		/* enable pixel packing */
		dev_write_mask(dev, VP_CONF, VP_CONF_PP_EN_MASK, 1);
		dev_write_mask(dev, VP_CONF, VP_CONF_YCC422_EN_MASK, 0);
	} else if (value == 1) {	/* YCC422 */
		dev_write_mask(dev, VP_CONF, VP_CONF_BYPASS_EN_MASK, 0);
		dev_write_mask(dev, VP_CONF, VP_CONF_PP_EN_MASK, 0);
		/* enable YCC422 */
		dev_write_mask(dev, VP_CONF, VP_CONF_YCC422_EN_MASK, 1);
	} else if (value == 2 || value == 3) {	/* bypass */
		/* enable bypass */
		dev_write_mask(dev, VP_CONF, VP_CONF_BYPASS_EN_MASK, 1);
		dev_write_mask(dev, VP_CONF, VP_CONF_PP_EN_MASK, 0);
		dev_write_mask(dev, VP_CONF, VP_CONF_YCC422_EN_MASK, 0);
	} else {
		LOGGER(SNPS_ERROR,"%s Wrong output option: %d", __func__, value);
		return;
	}

	/* YCC422 stuffing */
	dev_write_mask(dev, VP_STUFF, VP_STUFF_YCC422_STUFFING_MASK, 1);
	/* pixel packing stuffing */
	dev_write_mask(dev, VP_STUFF, VP_STUFF_PP_STUFFING_MASK, 1);

	/* output selector */
	dev_write_mask(dev, VP_CONF, VP_CONF_OUTPUT_SELECTOR_MASK, value);
}
