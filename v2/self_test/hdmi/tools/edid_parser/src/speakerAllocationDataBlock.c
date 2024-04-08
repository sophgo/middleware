/*
 * speakerAllocationDataBlock.c
 *
 *  Created on: Jul 22, 2010
 *
 *  Synopsys Inc.
 *  SG DWC PT02
 */

#include "speaker_alloc_data_block.h"


void speaker_alloc_data_block_reset(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t * sadb)
{
	sadb->mByte1 = 0;
	sadb->mValid = FALSE;
}

int speaker_alloc_data_block_parse(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t * sadb,
				     u8 * data)
{
	
	speaker_alloc_data_block_reset(dev, sadb);
	if (data != 0 && bit_field(data[0], 0, 5) == 0x03
	    && bit_field(data[0], 5, 3) == 0x04) {
		sadb->mByte1 = data[1];
		sadb->mValid = TRUE;
		return TRUE;
	}
	return FALSE;
}

int speakerAllocationDataBlock_SupportsFlFr(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t * sadb)
{
	return (bit_field(sadb->mByte1, 0, 1) == 1) ? TRUE : FALSE;
}

int speakerAllocationDataBlock_SupportsLfe(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t * sadb)
{
	return (bit_field(sadb->mByte1, 1, 1) == 1) ? TRUE : FALSE;
}

int speakerAllocationDataBlock_SupportsFc(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t * sadb)
{
	return (bit_field(sadb->mByte1, 2, 1) == 1) ? TRUE : FALSE;
}

int speakerAllocationDataBlock_SupportsRlRr(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t * sadb)
{
	return (bit_field(sadb->mByte1, 3, 1) == 1) ? TRUE : FALSE;
}

int speakerAllocationDataBlock_SupportsRc(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t * sadb)
{
	return (bit_field(sadb->mByte1, 4, 1) == 1) ? TRUE : FALSE;
}

int speakerAllocationDataBlock_SupportsFlcFrc(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t *
					      sadb)
{
	return (bit_field(sadb->mByte1, 5, 1) == 1) ? TRUE : FALSE;
}

int speakerAllocationDataBlock_SupportsRlcRrc(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t *
					      sadb)
{
	return (bit_field(sadb->mByte1, 6, 1) == 1) ? TRUE : FALSE;
}

u8
get_channell_alloc_code(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t
						    * sadb)
{
	/* TODO use an array instead */
	switch (sadb->mByte1) {
	case 1:
		return 0;
	case 3:
		return 1;
	case 5:
		return 2;
	case 7:
		return 3;
	case 17:
		return 4;
	case 19:
		return 5;
	case 21:
		return 6;
	case 23:
		return 7;
	case 9:
		return 8;
	case 11:
		return 9;
	case 13:
		return 10;
	case 15:
		return 11;
	case 25:
		return 12;
	case 27:
		return 13;
	case 29:
		return 14;
	case 31:
		return 15;
	case 73:
		return 16;
	case 75:
		return 17;
	case 77:
		return 18;
	case 79:
		return 19;
	case 33:
		return 20;
	case 35:
		return 21;
	case 37:
		return 22;
	case 39:
		return 23;
	case 49:
		return 24;
	case 51:
		return 25;
	case 53:
		return 26;
	case 55:
		return 27;
	case 41:
		return 28;
	case 43:
		return 29;
	case 45:
		return 30;
	case 47:
		return 31;
	default:
		return (u8) (-1);
	}
}

u8
speakerAllocationDataBlock_GetSpeakerAllocationByte(hdmi_tx_dev_t *dev, speakerAllocationDataBlock_t
						    * sadb)
{
	return sadb->mByte1;
}
