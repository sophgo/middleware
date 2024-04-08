/*
 * speaker_alloc_data_block.h
 *
 *  Created on: Jul 22, 2010
  * 
 * Synopsys Inc.
 * SG DWC PT02
 */

#ifndef SPEAKERALLOCATIONDATABLOCK_H_
#define SPEAKERALLOCATIONDATABLOCK_H_

#include "includes.h"
#include "bit_operation.h"

/** 
 * @file
 * SpeakerAllocation Data Block.
 * Holds and parse the Speaker Allocation datablock information.
 * For detailed handling of this structure, refer to documentation of the functions
 */

typedef struct {
	u8 mByte1;

	int mValid;
} speakerAllocationDataBlock_t;

void speaker_alloc_data_block_reset(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t * sadb);

int speaker_alloc_data_block_parse(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t * sadb,
				     u8 * data);

int speakerAllocationDataBlock_SupportsFlFr(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t *
					    sadb);

int speakerAllocationDataBlock_SupportsLfe(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t * sadb);

int speakerAllocationDataBlock_SupportsFc(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t * sadb);

int speakerAllocationDataBlock_SupportsRlRr(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t *
					    sadb);

int speakerAllocationDataBlock_SupportsRc(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t * sadb);

int speakerAllocationDataBlock_SupportsFlcFrc(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t *
					      sadb);

int speakerAllocationDataBlock_SupportsRlcRrc(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t *
					      sadb);
/**
 * @return the Channel Allocation code used in the Audio Infoframe to ease the translation process
 */
u8
get_channell_alloc_code(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t
						    * sadb);
/**
 * @return the whole byte of Speaker Allocation DataBlock where speaker allocation is indicated. User wishing to access and interpret it must know specifically how to parse it.
 */
u8
speakerAllocationDataBlock_GetSpeakerAllocationByte(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t
						    * sadb);

#endif				/* SPEAKERALLOCATIONDATABLOCK_H_ */
