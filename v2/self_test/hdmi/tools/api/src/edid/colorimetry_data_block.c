// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "edid/colorimetry_data_block.h"
#include "util/bit_operation.h"
#include "util/log.h"

void colorimetry_data_block_reset(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb)
{
	cdb->mByte3 = 0;
	cdb->mByte4 = 0;
	cdb->mValid = FALSE;
}

int colorimetry_data_block_parse(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb, u8 * data)
{
	LOG_TRACE();
	colorimetry_data_block_reset(dev, cdb);
	if ((data != 0) && (bit_field(data[0], 0, 5) == 0x03) &&
		(bit_field(data[0], 5, 3) == 0x07) && (bit_field(data[1], 0, 7) == 0x05)) {
		cdb->mByte3 = data[2];
		cdb->mByte4 = data[3];
		cdb->mValid = TRUE;
		return TRUE;
	}
	return FALSE;
}

int supports_xv_ycc709(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb)
{
	return (bit_field(cdb->mByte3, 1, 1) == 1) ? TRUE : FALSE;
}

int supports_xv_ycc601(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb)
{
	return (bit_field(cdb->mByte3, 0, 1) == 1) ? TRUE : FALSE;
}

int supports_s_ycc601(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb)
{
	return (bit_field(cdb->mByte3, 2, 1) == 1) ? TRUE : FALSE;
}

int supports_adobe_ycc601(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb)
{
	return (bit_field(cdb->mByte3, 3, 1) == 1) ? TRUE : FALSE;
}

int supports_adobe_rgb(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb)
{
	return (bit_field(cdb->mByte3, 4, 1) == 1) ? TRUE : FALSE;
}

int supports_metadata0(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb)
{
	return (bit_field(cdb->mByte4, 0, 1) == 1) ? TRUE : FALSE;
}

int supports_metadata1(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb)
{
	return (bit_field(cdb->mByte4, 1, 1) == 1) ? TRUE : FALSE;
}

int supports_metadata2(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb)
{
	return (bit_field(cdb->mByte4, 2, 1) == 1) ? TRUE : FALSE;
}

int supports_metadata3(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb)
{
	return (bit_field(cdb->mByte4, 3, 1) == 1) ? TRUE : FALSE;
}
