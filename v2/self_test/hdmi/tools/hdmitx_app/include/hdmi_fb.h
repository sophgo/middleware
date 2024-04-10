// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HDMI_FB_H_
#define HDMI_FB_H_

//#include <stdint.h>

/**
 * Structure that interfaces with the IOCTL of the framebuffer. To the IOCTL
 * that support read and write parameters use this structure to get and set
 * data from and to the driver
 */
typedef struct {
	uint32_t address;
	uint32_t value;
} fb_ioctl_data;

/**
 * IOCTL defines
 */
/**
 * @short IOCTL to read a byte from HDMI TX CORE
 * - fb_data->address -> address to read
 * - fb_data->value -> return value
 */
#define FB_HDMI_CORE_READ		0x1000

/**
 * @short IOCTL to write a byte to HDMI TX CORE
 * fb_data->address -> address to write to
 * fb_data->value -> value to write
 */
#define FB_HDMI_CORE_WRITE		0x1001

/**
 * @short IOCTL to read a byte from HDMI TX CLOCK MANAGER
 * - fb_data->address -> address to read
 * - fb_data->value -> return value
 */
#define FB_HDMI_CLK_MNG_READ		0x1002

/**
 * @short IOCTL to write a byte to HDMI TX CLOCK MANAGER
 * fb_data->address -> address to write to
 * fb_data->value -> value to write
 */
#define FB_HDMI_CLK_MNG_WRITE		0x1003

/**
 * @short IOCTL to read a byte from HDMI TX RESET MANAGER
 * - fb_data->address -> address to read
 * - fb_data->value -> return value
 */
#define FB_HDMI_RST_MNG_READ		0x1004

/**
 * @short IOCTL to write a byte to HDMI TX RESET MANAGER
 * fb_data->address -> address to write to
 * fb_data->value -> value to write
 */
#define FB_HDMI_RST_MNG_WRITE		0x1005

/**
 * @short IOCTL to read a byte from HDMI TX HDCP
 * - fb_data->address -> address to read
 * - fb_data->value -> return value
 */
#define FB_HDMI_HDCP_READ		0x1006

/**
 * @short IOCTL to write a byte to HDMI TX HDCP
 * fb_data->address -> address to write to
 * fb_data->value -> value to write
 */
#define FB_HDMI_HDCP_WRITE		0x1007

/**
 * @short IOCTL to read a byte from HDMI TX VIDEO BRIDGE
 * - fb_data->address -> address to read
 * - fb_data->value -> return value
 */
#define FB_HDMI_VID_BRIDGE_READ		0x1008

/**
 * @short IOCTL to write a byte to HDMI TX VIDEO BRIDGE
 * fb_data->address -> address to write to
 * fb_data->value -> value to write
 */
#define FB_HDMI_VID_BRIDGE_WRITE	0x1009

/**
 * @short IOCTL to read a byte from HDMI TX AUDIO BRIDGE
 * - fb_data->address -> address to read
 * - fb_data->value -> return value
 */
#define FB_HDMI_AUD_BRIDGE_READ		0x100A

/**
 * @short IOCTL to write a byte to HDMI TX AUDIO BRIDGE
 * fb_data->address -> address to write to
 * fb_data->value -> value to write
 */
#define FB_HDMI_AUD_BRIDGE_WRITE	0x100B

/**
 * @short IOCTL to read a byte from HDMI TX PHY INTERFACE
 * - fb_data->address -> address to read
 * - fb_data->value -> return value
 */
#define FB_HDMI_PHY_IF_READ		0x100C

/**
 * @short IOCTL to write a byte to HDMI TX PHY INTERFACE
 * fb_data->address -> address to write to
 * fb_data->value -> value to write
 */
#define FB_HDMI_PHY_IF_WRITE		0x100D

/**
 * @short IOCTL to get the device base address
 * fb_data->value -> base address
 */
#define FB_HDMI_BASE_ADDR		0x100E

#define FB_HDMI_SET_HPD			0x100F

#define FB_HDMI_GET_HDCP22		0x1010

#define FB_HDMI_FLUSH_SIG		0x1011

#define FB_HDMI_SET_DISP		0x1012

/**
 * Interrupt signals
 */
#define SIG_DWC_HDMI_TX 43
#define SIG_VID_BRIDGE  44
#define SIG_AUD_BRIDGE  45
#define SIG_TXPHY_IF   	46
#define SIG_HDCP        47
#define SIG_CEC_DETECT  48
#define SIG_EDID        49


#endif /* HDMI_FB_H_ */
