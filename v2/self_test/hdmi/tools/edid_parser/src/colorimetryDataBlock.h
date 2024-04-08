/*
 * colorimetry_data_block.h
 *
 *  Created on: Jul 22, 2010
 * 
 * Synopsys Inc.
 * SG DWC PT02
 */

#ifndef COLORIMETRYDATABLOCK_H_
#define COLORIMETRYDATABLOCK_H_

#include "includes.h"
#include "bit_operation.h"
/**
 * @file
 * Colorimetry Data Block class.
 * Holds and parses the Colorimetry datablock information.
 */

typedef struct {
	u8 mByte3;

	u8 mByte4;

	int mValid;

} colorimetryDataBlock_t;

void colorimetry_data_block_reset(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

int colorimetry_data_block_parse(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb, u8 * data);

int supports_xv_ycc709(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

int supports_xv_ycc601(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

int supports_s_ycc601(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

int supports_adobe_ycc601(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

int supports_adobe_rgb(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

int supports_metadata0(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

int supports_metadata1(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

int supports_metadata2(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

int supports_metadata3(hdmi_tx_dev_t *dev, colorimetryDataBlock_t * cdb);

#endif				/* COLORIMETRYDATABLOCK_H_ */
