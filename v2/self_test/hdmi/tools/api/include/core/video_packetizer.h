/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HALVIDEOPACKETIZER_H_
#define HALVIDEOPACKETIZER_H_

#include "../hdmitx_dev.h"
#include "util/types.h"

u8 vp_PixelPackingPhase(hdmi_tx_dev_t *dev);

void vp_ColorDepth(hdmi_tx_dev_t *dev, u8 value);

void vp_PixelPackingDefaultPhase(hdmi_tx_dev_t *dev, u8 bit);

void vp_PixelRepetitionFactor(hdmi_tx_dev_t *dev, u8 value);

void vp_Ycc422RemapSize(hdmi_tx_dev_t *dev, u8 value);

void vp_OutputSelector(hdmi_tx_dev_t *dev, u8 value);

#endif	/* HALVIDEOPACKETIZER_H_ */
