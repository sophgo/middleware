/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef INCLUDE_APP_VIDEO_VIDEO_H_
#define INCLUDE_APP_VIDEO_VIDEO_H_

#include "includes.h"
#include "hdmi_tx_app.h"

void update_dtd_cfg(dtd_t * user, dtd_t * cfg);

void update_video_cfg(videoParams_t * user, videoParams_t * cfg);

char * get_3d_structure_name(u8 m3dStructure);

char * get_videoinfo(videoParams_t *pVideo);

void print_videoinfo(videoParams_t *pVideo);

#endif /* INCLUDE_APP_VIDEO_VIDEO_H_ */
