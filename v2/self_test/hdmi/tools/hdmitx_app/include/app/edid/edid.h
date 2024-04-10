/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef INCLUDE_APP_EDID_EDID_H_
#define INCLUDE_APP_EDID_EDID_H_

#include "app/edid/edid_parser.h"
#include <hdmitx_dev.h>
#include "hdmi_tx_app.h"
// #include "includes.h"

void edid_init_sink_cap(sink_edid_t * sink);

void edid_read_cap(void);

int edid_read_hdmitx_cap(char * edid_tx_cap_file);

int edid_tx_supports_cea_code(u32 cea_code);

void edid_set_prefered(struct hdmi_tx_app *app, dtd_t *dtd);

void edid_set_video_prefered(hdmi_tx_dev_t *dev, sink_edid_t * sink_cap, videoParams_t * pVideo);

void edid_set_audio_prefered(hdmi_tx_dev_t *dev, sink_edid_t * sink_cap, audioParams_t * pAudio);

#endif /* INCLUDE_APP_EDID_EDID_H_ */
