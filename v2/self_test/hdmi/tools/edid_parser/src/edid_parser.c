/*
 * edid_parser.c
 *
 *  Created on: Feb 4, 2015
 */

#include "edid_parser.h"
#include "dtd.h"

const unsigned DTD_SIZE = 0x12;

int _edid_struture_parser(hdmi_tx_dev_t *dev, struct edid * edid)
{
	int i;
	char * monitorName;
	if(edid->header[0] != 0){
		LOGGER(SNPS_ERROR,"Invalid Header\n");
		return -1;
	}

	for (i=0; i < 4; i++) {
		struct detailed_timing * detailed_timing = &(edid->detailed_timings[i]);
		if(detailed_timing->pixel_clock == 0){
			struct detailed_non_pixel * npixel = &(detailed_timing->data.other_data);
			struct detailed_data_monitor_range * mrange = NULL;
			switch (npixel->type){
			case EDID_DETAIL_MONITOR_NAME: monitorName = (char *) &(npixel->data.str.str);
			printf("Monitor name: %s\n", monitorName);
			break;
			case EDID_DETAIL_MONITOR_RANGE:
				mrange = &(npixel->data.range);
				break;

			}
		}
		else { //Detailed Timing Definition
			struct detailed_pixel_timing * ptiming = &(detailed_timing->data.pixel_data);
			printf(" time: %d\n", detailed_timing->pixel_clock * 10000);
			printf("hactive_hblank_hi: %d\n", ptiming->hactive_hblank_hi);
		}
	}
	return 0;


}

int _edid_cea_extension_parser(hdmi_tx_dev_t *dev, u8 * buffer, edidCeaExt_t *edidExt)
{
	if (buffer[1] < 0x03){
		LOGGER(SNPS_ERROR,"Invalid version for CEA Extension block, only rev 3 is supported\n");
		return -1;
	}
	int i = 0;
	int c = 0;
	dtd_t tmpDtd;
	u8 offset = buffer[2];
	edidExt->edid_mYcc422Support = bit_field(buffer[3],	4, 1) == 1;
	edidExt->edid_mYcc444Support = bit_field(buffer[3],	5, 1) == 1;
	edidExt->edid_mBasicAudioSupport = bit_field(buffer[3], 6, 1) == 1;
	edidExt->edid_mUnderscanSupport = bit_field(buffer[3], 7, 1) == 1;
	if (offset != 4) {
		for (i = 4; i < offset;
				i += edid_ParseDataBlock(dev, buffer + i, edidExt)) ;
	}
	/* last is checksum */
	for (i = offset, c = 0;
			i < (sizeof(buffer) - 1) && c < 6;
			i += DTD_SIZE, c++) {
		if (dtd_parse(dev, &tmpDtd, buffer + i) == TRUE) {
			if (edidExt->edid_mDtdIndex < (sizeof(edidExt->edid_mDtd)
					/ sizeof(dtd_t))) {
				edidExt->edid_mDtd[edidExt->edid_mDtdIndex++] = tmpDtd;
				LOGGER(SNPS_NOTICE,"edid_mDtd code %d\n", edidExt->edid_mDtd[edidExt->edid_mDtdIndex].mCode);
				LOGGER(SNPS_NOTICE,"edid_mDtd limited to Ycc420? %d\n", edidExt->edid_mDtd[edidExt->edid_mDtdIndex].mLimitedToYcc420);
				LOGGER(SNPS_NOTICE,"edid_mDtd supports Ycc420? %d\n", edidExt->edid_mDtd[edidExt->edid_mDtdIndex].mYcc420);
			} else {
				//error_set(ERR_DTD_BUFFER_FULL);
				LOGGER(SNPS_ERROR,"buffer full - DTD ignored\n");
			}
		}
	}
	return TRUE;
}

int edid_parser(hdmi_tx_dev_t *dev, u8 * buffer, edidCeaExt_t *edidExt, u16 edid_size)
{
	int edidCnt = 0;
	while(edidCnt < edid_size){
		switch (buffer[edidCnt]){
		case 0x00:
			_edid_struture_parser(dev, (struct edid *) buffer + edidCnt);
			break;
		case CEA_EXT:
			_edid_cea_extension_parser(dev, buffer + edidCnt, edidExt);
			break;
		case VTB_EXT:
		case DI_EXT:
		case LS_EXT:
		case MI_EXT:
		default:
			LOGGER(SNPS_ERROR,"Block 0x%02x not supported\n", buffer[0]);
		}
		edidCnt = edidCnt + 128;
	}

	return TRUE;
}
