/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

/**
 * @file
 * E-EDID reader and parser
 * Initiates and handles I2C communications to read E-EDID then
 * parses read blocks and retains the information in corresponding
 * data structures
 */

#ifndef EDID_H_
#define EDID_H_

#include "../hdmitx_dev.h"
#include "dtd.h"
#include "short_video_desc.h"
#include "short_audio_desc.h"
#include "hdmivsdb.h"
#include "hdmiforumvsdb.h"
#include "monitor_range_limits.h"
#include "video_cap_data_block.h"
#include "colorimetry_data_block.h"
#include "speaker_alloc_data_block.h"
#include "util/types.h"
#include "edid/edid_type.h"

typedef enum {
	EDID_ERROR = 0, EDID_IDLE, EDID_READING, EDID_DONE
} edid_status_t;


int edid_extension_read(hdmi_tx_dev_t *dev, int block, u8 * edid_ext);
int edid_read(hdmi_tx_dev_t *dev, struct edid * edid);


#endif	/* EDID_H_ */
