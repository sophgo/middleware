/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HDMIVSDB_H_
#define HDMIVSDB_H_

#include "../hdmitx_dev.h"
#include "util/types.h"

#define MAX_HDMI_VIC		16
#define MAX_HDMI_3DSTRUCT	16
#define MAX_VIC_WITH_3D		16

/** For detailed handling of this structure, refer to documentation of the functions */
typedef struct {
	u16 mPhysicalAddress;

	int mSupportsAi;

	int mDeepColor30;

	int mDeepColor36;

	int mDeepColor48;

	int mDeepColorY444;

	int mDviDual;

	u16 mMaxTmdsClk;

	u16 mVideoLatency;

	u16 mAudioLatency;

	u16 mInterlacedVideoLatency;

	u16 mInterlacedAudioLatency;

	u32 mId;

	u8 mContentTypeSupport;

	u8 mImageSize;

	int mHdmiVicCount;

	u8 mHdmiVic[MAX_HDMI_VIC];

	int m3dPresent;

	int hdmi3dCount; //number of 3d entries

	int hdmi3dfirst; //number of 3d entries

	int hdmi3dfirststrc; //number of 3d entries

	int mVideo3dStruct[MAX_VIC_WITH_3D][MAX_HDMI_3DSTRUCT];	/* row index is the VIC number */

	int mDetail3d[MAX_VIC_WITH_3D][MAX_HDMI_3DSTRUCT];	/* index is the VIC number */

	int mValid;

} hdmivsdb_t;

void hdmivsdb_reset(hdmi_tx_dev_t *dev, hdmivsdb_t * vsdb);

/**
 * Parse an array of data to fill the hdmivsdb_t data strucutre
 * @param *vsdb pointer to the structure to be filled
 * @param *data pointer to the 8-bit data type array to be parsed
 * @return Success, or error code:
 * @return 1 - array pointer invalid
 * @return 2 - Invalid datablock tag
 * @return 3 - Invalid minimum length
 * @return 4 - HDMI IEEE registration identifier not valid
 * @return 5 - Invalid length - latencies are not valid
 * @return 6 - Invalid length - Interlaced latencies are not valid
 */
int hdmivsdb_parse(hdmi_tx_dev_t *dev, hdmivsdb_t * vsdb, u8 * data);

u16 get_index_supported_3dstructs(hdmi_tx_dev_t *dev, hdmivsdb_t * vsdb, u8 index);

u16 get_3dstruct_indexes(hdmi_tx_dev_t *dev, hdmivsdb_t * vsdb, u8 struct3d);

#endif	/* HDMIVSDB_H_ */
