// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "app/edid/edid_parser.h"
#include "edid/edid_type.h"
#include "dtd.h"
#include "util/bit_operation.h"
#include "includes.h"

const unsigned DTD_SIZE = 0x12;

int _edid_struture_parser(hdmi_tx_dev_t *dev, struct edid * edid, sink_edid_t * sink)
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

			switch (npixel->type){
			case EDID_DETAIL_MONITOR_NAME:
				monitorName = (char *) &(npixel->data.str.str);
				LOGGER(SNPS_INFO,"Monitor name: %s\n", monitorName);
				break;
			case EDID_DETAIL_MONITOR_RANGE:
				break;

			}
		}
		else { //Detailed Timing Definition
			struct detailed_pixel_timing * ptiming = &(detailed_timing->data.pixel_data);
			LOGGER(SNPS_INFO," time: %d\n", detailed_timing->pixel_clock * 10000);
			LOGGER(SNPS_INFO,"hactive_hblank_hi: %d\n", ptiming->hactive_hblank_hi);
		}
	}
	return TRUE;
}

int _edid_cea_extension_parser(hdmi_tx_dev_t *dev, u8 * buffer, sink_edid_t * edidExt)
{
	int i = 0;
	int c = 0;
	dtd_t tmpDtd;
	u8 offset = buffer[2];

	if (buffer[1] < 0){
		LOGGER(SNPS_ERROR, "Invalid version for CEA Extension block");
		return -1;
	}

	edidExt->edid_mYcc422Support = bit_field(buffer[3],	4, 1) == 1;
	edidExt->edid_mYcc444Support = bit_field(buffer[3],	5, 1) == 1;
	edidExt->edid_mBasicAudioSupport = bit_field(buffer[3], 6, 1) == 1;
	edidExt->edid_mUnderscanSupport = bit_field(buffer[3], 7, 1) == 1;
	if (offset != 4) {
		for (i = 4; i < offset; i += edid_parser_ParseDataBlock(dev, buffer + i, edidExt)) ;
	}
	/* last is checksum */
	for (i = offset, c = 0; i < (sizeof(buffer) - 1) && c < 6; i += DTD_SIZE, c++) {
		if (dtd_parse(dev, &tmpDtd, buffer + i) == TRUE) {
			if (edidExt->edid_mDtdIndex < (sizeof(edidExt->edid_mDtd) / sizeof(dtd_t)) - 1) {
				edidExt->edid_mDtd[edidExt->edid_mDtdIndex++] = tmpDtd;
				LOGGER(SNPS_INFO,"edid_mDtd code %d\n", edidExt->edid_mDtd[edidExt->edid_mDtdIndex].mCode);
				LOGGER(SNPS_INFO,"edid_mDtd limited to Ycc420? %d\n", edidExt->edid_mDtd[edidExt->edid_mDtdIndex].mLimitedToYcc420);
				LOGGER(SNPS_INFO,"edid_mDtd supports Ycc420? %d\n", edidExt->edid_mDtd[edidExt->edid_mDtdIndex].mYcc420);
			} else {
				LOGGER(SNPS_ERROR,"buffer full - DTD ignored\n");
			}
		}
	}
	return TRUE;
}

int edid_parser(hdmi_tx_dev_t *dev, u8 * buffer, sink_edid_t *edidExt, u16 edid_size)
{
	int ret = 0;
	switch (buffer[0]){
		case 0x00:
			ret = _edid_struture_parser(dev, (struct edid *) buffer, edidExt);
			break;
		case CEA_EXT:
			ret = _edid_cea_extension_parser(dev, buffer, edidExt);
			break;
		case VTB_EXT:
		case DI_EXT:
		case LS_EXT:
		case MI_EXT:
		default:
			LOGGER(SNPS_ERROR,"Block 0x%02x not supported\n", buffer[0]);
	}
	if(ret == TRUE)
		return TRUE;

	return FALSE;
}

int edid_parser_CeaExtReset(hdmi_tx_dev_t *dev, sink_edid_t *edidExt)
{
	unsigned i = 0;
	edidExt->edid_m20Sink = FALSE;
#if 1
	for (i = 0; i < sizeof(edidExt->edid_mMonitorName); i++) {
		edidExt->edid_mMonitorName[i] = 0;
	}
	edidExt->edid_mBasicAudioSupport = FALSE;
	edidExt->edid_mUnderscanSupport = FALSE;
	edidExt->edid_mYcc422Support = FALSE;
	edidExt->edid_mYcc444Support = FALSE;
	edidExt->edid_mYcc420Support = FALSE;
	edidExt->edid_mDtdIndex = 0;
	edidExt->edid_mSadIndex = 0;
	edidExt->edid_mSvdIndex = 0;
#endif
	hdmivsdb_reset(dev, &edidExt->edid_mHdmivsdb);
	hdmiforumvsdb_reset(dev, &edidExt->edid_mHdmiForumvsdb);
	monitor_range_limits_reset(dev, &edidExt->edid_mMonitorRangeLimits);
	video_cap_data_block_reset(dev, &edidExt->edid_mVideoCapabilityDataBlock);
	colorimetry_data_block_reset(dev, &edidExt->edid_mColorimetryDataBlock);
	speaker_alloc_data_block_reset(dev, &edidExt->edid_mSpeakerAllocationDataBlock);
	return TRUE;
}

int edid_parser_ParseDataBlock(hdmi_tx_dev_t *dev, u8 * data, sink_edid_t *edidExt)
{
	u8 tag = bit_field(data[0], 5, 3);
	u8 length = bit_field(data[0], 0, 5);
	u8 c = 0;
	shortAudioDesc_t tmpSad;
	shortVideoDesc_t tmpSvd;
	tmpSvd.mLimitedToYcc420 = 0;
	tmpSvd.mYcc420 = 0;
	u8 tmpYcc420All = 0;
	u8 tmpLimitedYcc420All = 0;
	u8 extendedTag = 0;
	u32 ieeeId = 0;
	int svdNr = 0;
	int i = 0;
	int icnt = 0;
	switch (tag) {
	case 0x1:		/* Audio Data Block */
		LOGGER(SNPS_DEBUG,"EDID: Audio datablock parsing\n");
		for (c = 1; c < (length + 1); c += 3) {
			sad_parse(dev, &tmpSad, data + c);
			if (edidExt->edid_mSadIndex < (sizeof(edidExt->edid_mSad) / sizeof(shortAudioDesc_t))) {
				edidExt->edid_mSad[edidExt->edid_mSadIndex++] = tmpSad;
			} else {
				LOGGER(SNPS_ERROR,"buffer full - SAD ignored\n");
			}
		}
		break;
	case 0x2:		/* Video Data Block */
		LOGGER(SNPS_DEBUG,"EDID: Video datablock parsing\n");
		for (c = 1; c < (length + 1); c++) {
			svd_parse(dev, &tmpSvd, data[c]);
			if (edidExt->edid_mSvdIndex < (sizeof(edidExt->edid_mSvd) / sizeof(shortVideoDesc_t))) {
				edidExt->edid_mSvd[edidExt->edid_mSvdIndex++] = tmpSvd;
			} else {
				LOGGER(SNPS_ERROR,"buffer full - SVD ignored\n");
			}
		}
		break;
	case 0x3:		/* Vendor Specific Data Block HDMI or HF */
		LOGGER(SNPS_DEBUG,"EDID: VSDB HDMI and HDMI-F\n ");
		ieeeId = byte_to_dword(0x00, data[3], data[2], data[1]);
		if (ieeeId == 0x000C03) {	/* HDMI */
			if (hdmivsdb_parse(dev, &edidExt->edid_mHdmivsdb, data) != TRUE) {
				LOGGER(SNPS_ERROR,"HDMI Vendor Specific Data Block corrupt");
				break;
			}
			LOGGER(SNPS_INFO,"EDID HDMI VSDB parsed");
		} else {
			if (ieeeId == 0xC45DD8) {	/* HDMI-F */
				LOGGER(SNPS_INFO,"Sink is HDMI 2.0 because haves HF-VSDB\n");
				edidExt->edid_m20Sink = TRUE;
				if (hdmiforumvsdb_parse(dev, &edidExt->edid_mHdmiForumvsdb, data) != TRUE) {
					LOGGER(SNPS_INFO,"HDMI Vendor Specific Data Block corrupt");
					break;
				} else {
#if 0
					if (edidExt->edid_mHdmiForumvsdb.mLTS_340Mcs_scramble == 1) {
						scrambling_Enable(baseAddr, 1);
						LOGGER(SNPS_INFO,"Scrambling enable by Sink HF-VSDB");
					} else {
						scrambling_Enable(baseAddr, 0);
						LOGGER(SNPS_INFO,"Scrambling disabled by Sink HF-VSDB");
					}
#endif
				}
			} else {
				LOGGER(SNPS_INFO,"Vendor Specific Data Block not parsed ieeeId: 0x%x",
						ieeeId);
			}
		}
		break;
	case 0x4:		/* Speaker Allocation Data Block */
		LOGGER(SNPS_DEBUG,"SAD block parsing");
		if (speaker_alloc_data_block_parse(dev, &edidExt->edid_mSpeakerAllocationDataBlock, data) != TRUE) {
			LOGGER(SNPS_ERROR,"Speaker Allocation Data Block corrupt");
		}
		break;
	case 0x7:{
		LOGGER(SNPS_DEBUG,"EDID CEA Extended field 0x07\n");
		extendedTag = data[1]; 
		switch (extendedTag) {
		case 0x00:	/* Video Capability Data Block */
			LOGGER(SNPS_DEBUG,"Video Capability Data Block\n");
			if (video_cap_data_block_parse(dev, &edidExt->edid_mVideoCapabilityDataBlock, data) != TRUE) {
				LOGGER(SNPS_ERROR,"Video Capability Data Block corrupt");
			}
			break;
		case 0x05:	/* Colorimetry Data Block */
			LOGGER(SNPS_DEBUG,"Colorimetry Data Block");
			if (colorimetry_data_block_parse(dev, &edidExt->edid_mColorimetryDataBlock, data) != TRUE) {
				LOGGER(SNPS_ERROR,"Colorimetry Data Block corrupt");
			}
			break;
		case 0x04:	/* HDMI Video Data Block */
			LOGGER(SNPS_INFO,"HDMI Video Data Block");
			break;
		case 0x12:	/* HDMI Audio Data Block */
			LOGGER(SNPS_INFO,"HDMI Audio Data Block");
			break;
		case 0xe:
			/** If it is a YCC420 VDB then VICs can ONLY be displayed in YCC 4:2:0 */
			LOGGER(SNPS_INFO,"YCBCR 4:2:0 Video Data Block\n");
			/** If Sink has YCC Datablocks it is HDMI 2.0 */
			edidExt->edid_m20Sink = TRUE;
			tmpLimitedYcc420All = (bit_field(data[0], 0, 5) == 1 ? 1 : 0);
			edid_parser_updateYcc420(edidExt, tmpYcc420All, tmpLimitedYcc420All);
			for (i = 0; i < length - 1; i++) {
				/** Lenght includes the tag byte*/
				tmpSvd.mCode = data[2 + i];
				tmpSvd.mNative = 0;
				tmpSvd.mLimitedToYcc420 = 1;
				int edid_cnt = 0;
				for (edid_cnt = 0;edid_cnt < edidExt->edid_mSvdIndex;edid_cnt++) {
					if (edidExt->edid_mSvd[edid_cnt].mCode == tmpSvd.mCode) {
						edidExt->edid_mSvd[edid_cnt].mLimitedToYcc420 =	1;
						goto concluded;
					}
				}
				if (edidExt->edid_mSvdIndex < (sizeof(edidExt->edid_mSvd) /  sizeof(shortVideoDesc_t)))
				{
					edidExt->edid_mSvd[edidExt->edid_mSvdIndex] = tmpSvd;
					edidExt->edid_mSvdIndex++;
				} else {
					LOGGER(SNPS_ERROR,"buffer full - YCC 420 DTD ignored");
				}
				concluded: ;
			}
			break;
		case 0x0f:
			/** If it is a YCC420 CDB then VIC can ALSO be displayed in YCC 4:2:0 */
			edidExt->edid_m20Sink = TRUE;
			LOGGER(SNPS_INFO,"YCBCR 4:2:0 Capability Map Data Block");
			svdNr = 0; 
			/* If YCC420 CMDB is bigger than 1, then there is SVD info to parse */
			if(length > 1){
				for (i = 0; i < length - 1; i++) {
					for (icnt = 0; icnt <= 7; icnt++) {
						/** Lenght includes the tag byte*/
						if (bit_field(data[2 + i], icnt, 1)) {
							svdNr = icnt + i*8;
							edidExt->edid_mSvd[svdNr].mYcc420 = 1;
						}
					}
				}
				/* Otherwise, all SVDs present at the Video Data Block support YCC420*/
			}else
			{
				tmpYcc420All = (bit_field(data[0], 0, 5) == 1 ? 1 : 0);
				edid_parser_updateYcc420(edidExt, tmpYcc420All, tmpLimitedYcc420All);
			}
#if 0
			LOGGER(SNPS_INFO,"data[0] = 0x%x", data[0]);
			LOGGER(SNPS_INFO,"data[1] = 0x%x", data[1]);
			LOGGER(SNPS_INFO,"data[2] = 0x%x", data[2]);
			LOGGER(SNPS_INFO,"data[3] = 0x%x", data[3]);
			LOGGER(SNPS_INFO,"data[4] = 0x%x", data[4]);
			LOGGER(SNPS_INFO,"data[5] = 0x%x", data[5]);
			LOGGER(SNPS_INFO,"data[6] = 0x%x", data[6]);
			LOGGER(SNPS_INFO,"data[7] = 0x%x", data[7]);
#endif
			break;
		default:
			LOGGER(SNPS_WARN,"Extended Data Block not parsed %d\n",
					extendedTag);
			break;
		}
		break;
	}
	default:
		LOGGER(SNPS_WARN,"Data Block not parsed %d\n", tag);
		break;
	}
	return length + 1;
}

void edid_parser_updateYcc420(sink_edid_t *edidExt, u8 Ycc420All, u8 LimitedToYcc420All)
{
	u16 edid_cnt = 0;
	for (edid_cnt = 0;edid_cnt < edidExt->edid_mSvdIndex;edid_cnt++) {
		switch (edidExt->edid_mSvd[edid_cnt].mCode){
		case 96:
		case 97:
		case 101:
		case 102:
		case 106:
		case 107:
			Ycc420All == 1 ? edidExt->edid_mSvd[edid_cnt].mYcc420 = Ycc420All : 0;
			LimitedToYcc420All == 1 ? edidExt->edid_mSvd[edid_cnt].mLimitedToYcc420 = LimitedToYcc420All : 0;
			break;
		default:
			break;
		}
	}
}
