/*
 * edid.c
 *
 *  Created on: Jul 23, 2010
 *  Synopsys Inc.
 *  SG DWC PT02
 */

#include "edid.h"
/**
 * Local variable hold the number of the currently read byte in a certain block of the EDID structure
 */
u8 edid_mCurrAddress;
/**
 * Local variable hold the number of the currently read block in the EDID structure.
 */
u8 edid_mCurrBlockNo;
/**
 * The accumulator of the block check sum value.
 */
u8 edid_mBlockSum;


int edid_CeaExtReset(hdmi_tx_dev_t *dev, edidCeaExt_t *edidExt)
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

int edid_ParseDataBlock(hdmi_tx_dev_t *dev, u8 * data, edidCeaExt_t *edidExt)
{
	u8 tag = bit_field(data[0], 5, 3);
	u8 length = bit_field(data[0], 0, 5);
	u8 c = 0;
	shortAudioDesc_t tmpSad;
	shortVideoDesc_t tmpSvd;
	switch (tag) {
	case 0x1:		/* Audio Data Block */
		LOGGER(SNPS_DEBUG,"EDID: Audio datablock parsing\n");
		for (c = 1; c < (length + 1); c += 3) {
			sad_parse(dev, &tmpSad, data + c);
			if (edidExt->edid_mSadIndex < (sizeof(edidExt->edid_mSad)
					/ sizeof(shortAudioDesc_t))) {
				edidExt->edid_mSad[edidExt->edid_mSadIndex++] = tmpSad;
			} else {
				//error_set(ERR_SHORT_AUDIO_DESC_BUFFER_FULL);
				LOGGER(SNPS_ERROR,"buffer full - SAD ignored\n");
			}
		}
		break;
	case 0x2:		/* Video Data Block */
		LOGGER(SNPS_DEBUG,"EDID: Video datablock parsing\n");
		for (c = 1; c < (length + 1); c++) {
			svd_parse(dev, &tmpSvd, data[c]);
			if (edidExt->edid_mSvdIndex < (sizeof(edidExt->edid_mSvd)
					/ sizeof(shortVideoDesc_t))) {
				edidExt->edid_mSvd[edidExt->edid_mSvdIndex++] = tmpSvd;
			} else {
				//error_set(ERR_SHORT_VIDEO_DESC_BUFFER_FULL);
				LOGGER(SNPS_ERROR,"buffer full - SVD ignored\n");
			}
		}
		break;
	case 0x3:		/* Vendor Specific Data Block  HDMI or HF */
		LOGGER(SNPS_DEBUG,"EDID: VSDB HDMI and HDMI-F\n ");
		u32 ieeeId = byte_to_dword(0x00, data[3], data[2],
				data[1]);
		if (ieeeId == 0x000C03) {	/* HDMI */
			if (hdmivsdb_parse(dev, &edidExt->edid_mHdmivsdb, data) != TRUE) {
				LOGGER(SNPS_ERROR,"HDMI Vendor Specific Data Block corrupt\n");
				break;
			}
			LOGGER(SNPS_NOTICE,"EDID HDMI VSDB parsed\n");
		} else {
			if (ieeeId == 0xC45DD8) {	/* HDMI-F */
				LOGGER(SNPS_NOTICE,"Sink is HDMI 2.0 because haves HF-VSDB\n");
				edidExt->edid_m20Sink = TRUE;
				if (hdmiforumvsdb_parse(dev, &edidExt->edid_mHdmiForumvsdb,
						data) != TRUE) {
					LOGGER(SNPS_NOTICE,"HDMI Vendor Specific Data Block corrupt\n");
					break;
				} else {
#if 0
					if (edidExt->edid_mHdmiForumvsdb.mLTS_340Mcs_scramble == 1) {
						scrambling_Enable(baseAddr, 1);
						LOGGER(SNPS_NOTICE,"Scrambling enable by Sink HF-VSDB");
					} else {
						scrambling_Enable(baseAddr, 0);
						LOGGER(SNPS_NOTICE,"Scrambling disabled by Sink HF-VSDB");
					}
#endif
				}
			} else {
				LOGGER(SNPS_NOTICE,"Vendor Specific Data Block not parsed ieeeId: 0x%x\n",
						ieeeId);
			}
		}
		break;
	case 0x4:		/* Speaker Allocation Data Block */
		LOGGER(SNPS_DEBUG,"SAD block parsing");
		if (speaker_alloc_data_block_parse
				(dev, &edidExt->edid_mSpeakerAllocationDataBlock, data) != TRUE) {
			LOGGER(SNPS_ERROR,"Speaker Allocation Data Block corrupt\n");
		}
		break;
	case 0x7:{
		LOGGER(SNPS_DEBUG,"EDID CEA Extended field 0x07\n");
		u8 extendedTag = data[1];
		switch (extendedTag) {
		case 0x00:	/* Video Capability Data Block */
			LOGGER(SNPS_DEBUG,"Video Capability Data Block\n");
			if (video_cap_data_block_parse
					(dev, &edidExt->edid_mVideoCapabilityDataBlock,
							data) != TRUE) {
				LOGGER(SNPS_ERROR,"Video Capability Data Block corrupt\n");
			}
			break;
		case 0x05:	/* Colorimetry Data Block */
			LOGGER(SNPS_DEBUG,"Colorimetry Data Block");
			if (colorimetry_data_block_parse
					(dev, &edidExt->edid_mColorimetryDataBlock,
							data) != TRUE) {
				LOGGER(SNPS_ERROR,"Colorimetry Data Block corrupt\n");
			}
			break;
		case 0x04:	/* HDMI Video Data Block */
			LOGGER(SNPS_NOTICE,"HDMI Video Data Block\n");
			break;
		case 0x12:	/* HDMI Audio Data Block */
			LOGGER(SNPS_NOTICE,"HDMI Audio Data Block\n");
			break;
		case 0xe:
			/** If it is a YCC420 VDB then can ONLY be displayed in YCC 4:2:0 */
			LOGGER(SNPS_NOTICE,"YCBCR 4:2:0 Video Data Block\n");
			/** If Sink has YCC Datablocks it is HDMI 2.0 */
			edidExt->edid_m20Sink = TRUE;
			int i = 0;
			for (i = 0;
					i < (bit_field(data[0], 0, 5)
							- 1); i++) {
				/** Lenght includes the tag byte*/
				tmpSvd.mCode = data[2 + i];
				tmpSvd.mNative = 0;
				tmpSvd.mLimitedToYcc420 = 1;
				int edid_cnt = 0;
				for (edid_cnt = 0;
						edid_cnt < edidExt->edid_mSvdIndex;
						edid_cnt++) {
					if (edidExt->edid_mSvd[edid_cnt].mCode ==
							tmpSvd.mCode) {
						edidExt->edid_mSvd[edid_cnt] =
								tmpSvd;
						goto concluded;
					}
				}
				if (edidExt->edid_mSvdIndex < (sizeof(edidExt->edid_mSvd) /  sizeof(shortVideoDesc_t)))
				{
					edidExt->edid_mSvd[edidExt->edid_mSvdIndex] =
							tmpSvd;
					edidExt->edid_mSvdIndex++;
				} else {
					LOGGER(SNPS_ERROR,"buffer full - YCC 420 DTD ignored");
				}
				concluded:
				LOGGER(SNPS_NOTICE,"-- %d",
						edidExt->edid_mSvd[edidExt->edid_mSvdIndex -
						                   1].mCode);
				LOGGER(SNPS_NOTICE,"-- %d",
						edidExt->edid_mSvd[edidExt->edid_mSvdIndex -
						                   1].mLimitedToYcc420);
				LOGGER(SNPS_NOTICE,"-- %d",
						edidExt->edid_mSvd[edidExt->edid_mSvdIndex -
						                   1].mYcc420);
			}
			break;
		case 0xf:
			/** If it is a YCC420 CDB then can ALSO be displayed in YCC 4:2:0 */
			edidExt->edid_m20Sink = TRUE;
			LOGGER(SNPS_NOTICE,"YCBCR 4:2:0 Capability Map Data Block");
			int svdNr = 0;
			int icnt = 0;
			for (icnt = 0;
					icnt
					< (bit_field(data[0], 0, 5)
							- 1); icnt++) {
				/** Lenght includes the tag byte*/
				svdNr = data[2 + icnt];
				tmpSvd.mCode =
						edidExt->edid_mSvd[svdNr - 1].mCode;
				tmpSvd.mYcc420 = 1;
				edidExt->edid_mSvd[svdNr - 1] = tmpSvd;
				LOGGER(SNPS_NOTICE,"%d",
						edidExt->edid_mSvd[svdNr - 1].mCode);
				LOGGER(SNPS_NOTICE,"%d",
						edidExt->edid_mSvd[svdNr -
						                   1].mLimitedToYcc420);
				LOGGER(SNPS_NOTICE,"%d",
						edidExt->edid_mSvd[svdNr -
						                   1].mYcc420);
			}
#if 0
			LOGGER(SNPS_NOTICE,"data[0] = 0x%x", data[0]);
			LOGGER(SNPS_NOTICE,"data[1] = 0x%x", data[1]);
			LOGGER(SNPS_NOTICE,"data[2] = 0x%x", data[2]);
			LOGGER(SNPS_NOTICE,"data[3] = 0x%x", data[3]);
			LOGGER(SNPS_NOTICE,"data[4] = 0x%x", data[4]);
			LOGGER(SNPS_NOTICE,"data[5] = 0x%x", data[5]);
			LOGGER(SNPS_NOTICE,"data[6] = 0x%x", data[6]);
			LOGGER(SNPS_NOTICE,"data[7] = 0x%x", data[7]);
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
