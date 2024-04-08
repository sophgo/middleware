// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "identification.h"
#include "identification_reg.h"

#include "util/log.h"
#include "util/error.h"
#include "bsp/access.h"

u8 id_design(hdmi_tx_dev_t *dev)
{
	return dev_read(dev, DESIGN_ID);
}

u8 id_revision(hdmi_tx_dev_t *dev)
{
	return dev_read(dev, REVISION_ID);
}

u8 id_product_line(hdmi_tx_dev_t *dev)
{
	return dev_read(dev, PRODUCT_ID0);
}

u8 id_product_type(hdmi_tx_dev_t *dev)
{
	return dev_read(dev, PRODUCT_ID1);
}

int id_hdcp_support(hdmi_tx_dev_t *dev)
{
	if (dev_read_mask(dev, PRODUCT_ID1, PRODUCT_ID1_PRODUCT_ID1_HDCP_MASK) == 3)
		return TRUE;
	else
		return FALSE;
}

int id_hdcp14_support(hdmi_tx_dev_t *dev)
{
	if (dev_read_mask(dev, CONFIG0_ID, CONFIG0_ID_HDCP_MASK))
		return TRUE;
	else
		return FALSE;
}

int id_hdcp22_support(hdmi_tx_dev_t *dev)
{
	if (dev_read_mask(dev, CONFIG1_ID, CONFIG1_ID_HDCP22_EXT_MASK))
		return HDCP_22_EXT;
	else if (dev_read_mask(dev, CONFIG1_ID, CONFIG1_ID_HDCP22_SNPS_MASK))
		return HDCP_22_SNPS;
	else
		return FALSE;
}

int id_phy(hdmi_tx_dev_t *dev)
{
	return dev_read(dev, CONFIG2_ID);
}


char * id_phy_string(hdmi_tx_dev_t *dev)
{
	unsigned int phy_id = id_phy(dev);

	switch (phy_id) {
		case 0x00: return "Legacy PHY (HDMI TX PHY)";
		case 0xF2: return "PHY GEN2 (HDMI 3D TX PHY)";
		case 0xE2: return "PHY GEN2 (HDMI 3D TX PHY) + HEAC PHY";
		case 0xC2: return "PHY MHL COMBO (MHL+HDMI 2.0 TX PHY)";
		case 0xB2: return "PHY MHL COMBO (MHL+HDMI 2.0 TX PHY) + HEAC PHY";
		case 0xF3: return "PHY HDMI 20 (HDMI 2.0 TX PHY)";
		case 0xE3: return "PHY HDMI 20 (HDMI 2.0 TX PHY) + HEAC PHY";
		case 0xFE: return "External PHY";
	}

	return "Unknown PHY";
}

