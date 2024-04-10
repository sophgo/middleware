/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef SPEAKERALLOCATIONDATABLOCK_H_
#define SPEAKERALLOCATIONDATABLOCK_H_

#include "../hdmitx_dev.h"
#include "util/types.h"

/** 
 * @file
 * SpeakerAllocation Data Block.
 * Holds and parse the Speaker Allocation data block information.
 * For detailed handling of this structure, refer to documentation of the functions
 */

typedef struct {
	u8 mByte1;

	int mValid;
} speakerAllocationDataBlock_t;
//speaker_alloc_data_block_reset
void speaker_alloc_data_block_reset(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t * sadb);

int speaker_alloc_data_block_parse(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t * sadb, u8 * data);

/**
 * @return the Channel Allocation code used in the Audio Info frame to ease the translation process
 */
int get_channell_alloc_code(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t * sadb);

#endif				/* SPEAKERALLOCATIONDATABLOCK_H_ */
