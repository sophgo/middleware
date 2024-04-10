/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef SRC_EDID_PARSER_H_
#define SRC_EDID_PARSER_H_

#include "edid/edid_type.h"

#include "edid/colorimetry_data_block.h"
#include "edid/dtd.h"
#include "edid/hdmiforumvsdb.h"
#include "edid/hdmivsdb.h"
#include "edid/monitor_range_limits.h"
#include "edid/short_audio_desc.h"
#include "edid/short_video_desc.h"
#include "edid/speaker_alloc_data_block.h"
#include "edid/video_cap_data_block.h"

#define EDID_DTD_ARRAY_SIZE  (32)
#define EDID_SVD_ARRAY_SIZE  (128)
#define EDID_SAD_ARRAY_SIZE  (128)
#define EDID_MONITOR_NAME_SIZE  (13)

typedef struct {

	/**
	 * Array to hold all the parsed Detailed Timing Descriptors.
	 */
	dtd_t edid_mDtd[32];

	unsigned int edid_mDtdIndex;
	/**
	 * array to hold all the parsed Short Video Descriptors.
	 */
	shortVideoDesc_t edid_mSvd[EDID_SVD_ARRAY_SIZE];

	shortVideoDesc_t tmpSvd;

	unsigned int edid_mSvdIndex;
	/**
	 * array to hold all the parsed Short Audio Descriptors.
	 */
	shortAudioDesc_t edid_mSad[EDID_SAD_ARRAY_SIZE];

	unsigned int edid_mSadIndex;

	/**
	 * A string to hold the Monitor Name parsed from EDID.
	 */
	char edid_mMonitorName[EDID_MONITOR_NAME_SIZE];

	int edid_mYcc444Support;

	int edid_mYcc422Support;

	int edid_mYcc420Support;

	int edid_mBasicAudioSupport;

	int edid_mUnderscanSupport;

	/**
	 *  If Sink is HDMI 2.0
	 */
	int edid_m20Sink;

	hdmivsdb_t edid_mHdmivsdb;

	hdmiforumvsdb_t edid_mHdmiForumvsdb;

	monitorRangeLimits_t edid_mMonitorRangeLimits;

	videoCapabilityDataBlock_t edid_mVideoCapabilityDataBlock;

	colorimetryDataBlock_t edid_mColorimetryDataBlock;

	speakerAllocationDataBlock_t edid_mSpeakerAllocationDataBlock;
} sink_edid_t;


int edid_parser(hdmi_tx_dev_t *dev, u8 * buffer, sink_edid_t *edidExt, u16 edid_size);

/**
 * Initialise the E-EDID reader
 * reset all internal variables
 * reset reader state
 * prepare I2C master
 * commence reading E-EDID memory from sink
 * @param baseAddr base address of controller
 * @param sfrClock external clock supplied to controller
 * @note: this version only works with 25MHz
 */
int edid_parser_Initialize(hdmi_tx_dev_t *dev, u16 sfrClock);
/**
 * @param baseAddr base address of controller
 * @return TRUE if successful
 */
int edid_parser_CeaExtReset(hdmi_tx_dev_t *dev, sink_edid_t *edidExt);

/**
 * Parses an EDID block of 128 bytes.
 * Each known group (based on location or CEA tag) is parsed using the constructor of its own datatype (class), called in this function accordingly, this function then adds the new valid data structures into the EDID library.
 * @param baseAddr base address of controller
 * @param buffer a pointer to buffer (of 128 bytes)
 * @return TRUE if successful
 */
int edid_parser_ParseBlock(hdmi_tx_dev_t *dev, u8 * buffer, sink_edid_t *edidExt);
/**
 * Parse Data Block data structures listed in the CEA Data Block Collection.
 * It identifies the block type and calls the appropriate class constructor to parse it.
 * It adds the new parsed block (object) to the respective vector arrays in the edid object.
 * @param baseAddr base address of controller
 * @param data a buffer array of bytes (pointer to the start of the block).
 * @return the length of the parsed Data Block in the Collection
 */
int edid_parser_ParseDataBlock(hdmi_tx_dev_t *dev, u8 * data, sink_edid_t *edidExt);

void edid_parser_updateYcc420(sink_edid_t *edidExt, u8 Ycc420All, u8 LimitedToYcc420All);

#endif
