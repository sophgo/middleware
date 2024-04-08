// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "edid/speaker_alloc_data_block.h"
#include "util/bit_operation.h"
#include "util/log.h"

typedef struct speaker_alloc_code {
	unsigned char byte;
	unsigned char code;
} speaker_alloc_code_t;

static speaker_alloc_code_t alloc_codes[] = {
		{1,  0},
		{3,  1},
		{5,  2},
		{7,  3},
		{17, 4},
		{19, 5},
		{21, 6},
		{23, 7},
		{9,  8},
		{11, 9},
		{13, 10},
		{15, 11},
		{25, 12},
		{27, 13},
		{29, 14},
		{31, 15},
		{73, 16},
		{75, 17},
		{77, 18},
		{79, 19},
		{33, 20},
		{35, 21},
		{37, 22},
		{39, 23},
		{49, 24},
		{51, 25},
		{53, 26},
		{55, 27},
		{41, 28},
		{43, 29},
		{45, 30},
		{47, 31},
		{0, 0}
};

void speaker_alloc_data_block_reset(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t * sadb)
{
	sadb->mByte1 = 0;
	sadb->mValid = FALSE;
}

int speaker_alloc_data_block_parse(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t * sadb, u8 * data)
{
	LOG_TRACE();
	speaker_alloc_data_block_reset(dev, sadb);
	if ((data != 0) && (bit_field(data[0], 0, 5) == 0x03) && (bit_field(data[0], 5, 3) == 0x04)) {
		sadb->mByte1 = data[1];
		sadb->mValid = TRUE;
		return TRUE;
	}
	return FALSE;
}

int get_channell_alloc_code(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t * sadb)
{
	int i = 0;
	for(i = 0; alloc_codes[i].byte != 0; i++){
		if(sadb->mByte1 == alloc_codes[i].byte){
			return alloc_codes[i].code;
		}
	}
	return -1;
}
