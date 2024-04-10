/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef VIDEO_BRIDGE_H_
#define VIDEO_BRIDGE_H_

#include "includes.h"
#include "hdmi_tx_app.h"


#define	VG_SWRSTZ		0x00
#define	VG_CONF			0x04
#define	VG_PREP			0x08
#define	VG_HACTIVE0		0x0C
#define	VG_HACTIVE1		0x10
#define	VG_HBLANK0		0x14
#define	VG_HBLANK1		0x18
#define	VG_HDELAY0		0x1C
#define	VG_HDELAY1		0x20
#define	VG_HWIDTH0		0x24
#define	VG_HWIDTH1		0x28
#define	VG_VACTIVE0		0x2C
#define	VG_VACTIVE1		0x30
#define	VG_VBLANK0		0x34
#define	VG_VDELAY0		0x38
#define	VG_VWIDTH0		0x3C
#define	VG_VIDSOURCE		0x40
#define	VG_3DSTRUCT		0x44
#define	VG_IPI_CONF		0x48
#define	VG_RAM_ADDR0		0x80
#define	VG_RAM_ADDR1		0x84
#define	VG_WRT_RAM_CTRL		0x88
#define	VG_WRT_RAM_DATA		0x8C
#define	VG_WRT_RAM_STOP_ADDR0	0x90
#define	VG_WRT_RAM_STOP_ADDR1	0x94
#define	VG_WRT_BYTE_STOP	0x98
#define	VG_PATTERNMODE		0x9C
#define	VG_CBWIDTH0		0xA0
#define	VG_CBWIDTH1		0xA4
#define	VG_CBHEIGHT0		0xA8
#define	VG_CBHEIGHT1		0xAC
#define	VG_CBCOLORA0		0xB0
#define	VG_CBCOLORA1		0xB4
#define	VG_CBCOLORA2		0xB8
#define	VG_CBCOLORA3		0xBC
#define	VG_CBCOLORA4		0xC0
#define	VG_CBCOLORA5		0xC4
#define	VG_CBCOLORB0		0xC8
#define	VG_CBCOLORB1		0xCC
#define	VG_CBCOLORB2		0xD0
#define	VG_CBCOLORB3		0xD4
#define	VG_CBCOLORB4		0xD8
#define	VG_CBCOLORB5		0xDC


void video_bridge_write(struct hdmi_tx_app *app, uint32_t reg, uint32_t data);

uint32_t video_bridge_read(struct hdmi_tx_app *app, uint32_t reg);

int video_generator_config(struct hdmi_tx_app *app);

void video_chess_board_config(struct hdmi_tx_app *app);

void video_generator_pattern_mode(struct hdmi_tx_app *app, u8 value);

void video_generator_set_pattern(encoding_t encoding);

#endif /* VIDEO_BRIDGE_H_ */
