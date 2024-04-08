// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "phy_reg.h"
#include "phy_305.h"
#include "phy_jtag.h"
#include "general_ops.h"

#define OPMODE_PLLCFG 0x06 //Mode of Operation and PLL  Dividers Control Register
#define CKSYMTXCTRL   0x09 //Clock Symbol and Transmitter Control Register
#define PLLCURRCTRL   0x10 //PLL Current Control Register
#define VLEVCTRL      0x0E //Voltage Level Control Register
#define PLLGMPCTRL    0x11 //PLL Gmp Control Register
#define TXTERM        0x19 //Transmission Termination Register

static struct phy_config phy305[] = {
      // clock      pixel                   color             opmode     oppllcfg  pllcurrctrl pllgmpctrl txterm  vlevctrl  cksymtxctrl
		{13.5,   	PIXEL_REPETITION_1,   	COLOR_DEPTH_8,    HDMI_14,    0x0003,    0x0380,    0x0650,    0x04,    0x200,    0x80C8},
		{13.5,    	PIXEL_REPETITION_1,   	COLOR_DEPTH_10,   HDMI_14,    0x1003,    0x0380,    0x0664,    0x04,    0x200,    0x80C8},
		{13.5,    	PIXEL_REPETITION_1,   	COLOR_DEPTH_12,   HDMI_14,    0x2003,    0x03C0,    0x0278,    0x04,    0x200,    0x80C8},
		{13.5,    	PIXEL_REPETITION_1,   	COLOR_DEPTH_16,   HDMI_14,    0x3002,    0x1380,    0x0650,    0x04,    0x200,    0x80C8},
		{13.5,    	PIXEL_REPETITION_3,   	COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1380,    0x0650,    0x04,    0x200,    0x80C8},
		{13.5,    	PIXEL_REPETITION_3,   	COLOR_DEPTH_10,   HDMI_14,    0x1002,    0x1380,    0x0664,    0x04,    0x200,    0x80C8},
		{13.5,    	PIXEL_REPETITION_3,   	COLOR_DEPTH_12,   HDMI_14,    0x2002,    0x13C0,    0x0278,    0x04,    0x200,    0x80C8},
		{13.5,    	PIXEL_REPETITION_3,   	COLOR_DEPTH_16,   HDMI_14,    0x3001,    0x2380,    0x0650,    0x04,    0x200,    0x80C8},
		{13.5,    	PIXEL_REPETITION_7,   	COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2380,    0x0650,    0x04,    0x200,    0x80C8},
		{13.5,    	PIXEL_REPETITION_7,   	COLOR_DEPTH_10,   HDMI_14,    0x1001,    0x2380,    0x0664,    0x04,    0x200,    0x80C8},
		{13.5,    	PIXEL_REPETITION_7,   	COLOR_DEPTH_12,   HDMI_14,    0x2001,    0x23C0,    0x0278,    0x04,    0x200,    0x80C8},
		{13.5,    	PIXEL_REPETITION_7,   	COLOR_DEPTH_16,   HDMI_14,    0x3000,    0x3380,    0x0650,    0x04,    0x200,    0x80C8},
		{18,    	PIXEL_REPETITION_2,   	COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1380,    0x063C,    0x04,    0x200,    0x80C8},
		{18,    	PIXEL_REPETITION_2,   	COLOR_DEPTH_10,   HDMI_14,    0x1002,    0x1380,    0x064B,    0x04,    0x200,    0x80C8},
		{18,    	PIXEL_REPETITION_2,   	COLOR_DEPTH_12,   HDMI_14,    0x2002,    0x13C0,    0x025A,    0x04,    0x200,    0x80C8},
		{18,    	PIXEL_REPETITION_2,   	COLOR_DEPTH_16,   HDMI_14,    0x3001,    0x2380,    0x063C,    0x04,    0x200,    0x80C8},
		{18,    	PIXEL_REPETITION_5,   	COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2380,    0x063C,    0x04,    0x200,    0x80C8},
		{18,    	PIXEL_REPETITION_5,   	COLOR_DEPTH_10,   HDMI_14,    0x1001,    0x2380,    0x064B,    0x04,    0x200,    0x80C8},
		{18,    	PIXEL_REPETITION_5,   	COLOR_DEPTH_12,   HDMI_14,    0x2001,    0x23C0,    0x025A,    0x04,    0x200,    0x80C8},
		{18,    	PIXEL_REPETITION_5,   	COLOR_DEPTH_16,   HDMI_14,    0x3000,    0x3380,    0x063C,    0x04,    0x200,    0x80C8},
		{21.6,    	PIXEL_REPETITION_4,   	COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2341,    0x0632,    0x04,    0x200,    0x80C8},
		{21.6,    	PIXEL_REPETITION_4,   	COLOR_DEPTH_12,   HDMI_14,    0x2001,    0x23C0,    0x024B,    0x04,    0x200,    0x80C8},
		{21.6,    	PIXEL_REPETITION_4,   	COLOR_DEPTH_16,   HDMI_14,    0x3000,    0x3341,    0x0632,    0x04,    0x200,    0x80C8},
		{21.6,    	PIXEL_REPETITION_9,   	COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3341,    0x0632,    0x04,    0x200,    0x80C8},
		{21.6,    	PIXEL_REPETITION_9,   	COLOR_DEPTH_12,   HDMI_14,    0x2000,    0x33C0,    0x024B,    0x04,    0x200,    0x80C8},
		{21.6,    	PIXEL_REPETITION_9,   	COLOR_DEPTH_16,   HDMI_20,    0x3640,    0x33C0,    0x0264,    0x00,    0x200,    0x80F5},
		{24,    	PIXEL_REPETITION_8,   	COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3342,    0x062D,    0x04,    0x200,    0x80C8},
		{24,    	PIXEL_REPETITION_8,   	COLOR_DEPTH_16,   HDMI_20,    0x3640,    0x33C0,    0x025A,    0x00,    0x200,    0x80F5},
		{25.175,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0003,    0x0343,    0x0628,    0x04,    0x200,    0x80C8},
		{25.175,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1003,    0x0341,    0x0632,    0x04,    0x200,    0x80C8},
		{25.175,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2003,    0x03C2,    0x023C,    0x04,    0x200,    0x80C8},
		{25.175,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3002,    0x1343,    0x0628,    0x04,    0x200,    0x80C8},
		{27,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0003,    0x0343,    0x0628,    0x04,    0x200,    0x80C8},
		{27,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1003,    0x0341,    0x0632,    0x04,    0x200,    0x80C8},
		{27,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2003,    0x03C2,    0x023C,    0x04,    0x200,    0x80C8},
		{27,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3002,    0x1343,    0x0628,    0x04,    0x200,    0x80C8},
		{27,    	PIXEL_REPETITION_1,    	COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1343,    0x0628,    0x04,    0x200,    0x80C8},
		{27,    	PIXEL_REPETITION_1,    	COLOR_DEPTH_10,   HDMI_14,    0x1002,    0x1341,    0x0632,    0x04,    0x200,    0x80C8},
		{27,    	PIXEL_REPETITION_1,    	COLOR_DEPTH_12,   HDMI_14,    0x2002,    0x13C2,    0x023C,    0x04,    0x200,    0x80C8},
		{27,    	PIXEL_REPETITION_1,    	COLOR_DEPTH_16,   HDMI_14,    0x3001,    0x2343,    0x0628,    0x04,    0x200,    0x80C8},
		{27,    	PIXEL_REPETITION_3,    	COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2343,    0x0628,    0x04,    0x200,    0x80C8},
		{27,    	PIXEL_REPETITION_3,    	COLOR_DEPTH_10,   HDMI_14,    0x1001,    0x2341,    0x0632,    0x04,    0x200,    0x80C8},
		{27,    	PIXEL_REPETITION_3,    	COLOR_DEPTH_12,   HDMI_14,    0x2001,    0x23C2,    0x023C,    0x04,    0x200,    0x80C8},
		{27,    	PIXEL_REPETITION_3,    	COLOR_DEPTH_16,   HDMI_14,    0x3000,    0x3343,    0x0628,    0x04,    0x200,    0x80C8},
		{27,    	PIXEL_REPETITION_7,    	COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3343,    0x0628,    0x04,    0x200,    0x80C8},
		{27,    	PIXEL_REPETITION_7,    	COLOR_DEPTH_10,   HDMI_14,    0x1000,    0x3341,    0x0632,    0x04,    0x200,    0x80C8},
		{27,    	PIXEL_REPETITION_7,    	COLOR_DEPTH_12,   HDMI_14,    0x2000,    0x33C2,    0x023C,    0x04,    0x200,    0x80C8},
		{27,    	PIXEL_REPETITION_7,    	COLOR_DEPTH_16,   HDMI_20,    0x3640,    0x33C0,    0x0250,    0x00,    0x200,    0x80F5},
		{31.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0003,    0x0343,    0x0628,    0x04,    0x200,    0x80C8},
		{33.75,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0003,    0x0343,    0x0628,    0x04,    0x200,    0x80C8},
		{35.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0003,    0x0343,    0x0628,    0x04,    0x200,    0x80C8},
		{36,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0003,    0x0285,    0x0228,    0x04,    0x200,    0x80C8},
		{36,    	PIXEL_REPETITION_2,    	COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2285,    0x061E,    0x04,    0x200,    0x80C8},
		{36,    	PIXEL_REPETITION_2,    	COLOR_DEPTH_10,   HDMI_14,    0x1009,    0x2380,    0x064B,    0x04,    0x200,    0x80C8},
		{36,    	PIXEL_REPETITION_2,    	COLOR_DEPTH_12,   HDMI_14,    0x2001,    0x2303,    0x022D,    0x04,    0x200,    0x80C8},
		{36,    	PIXEL_REPETITION_2,    	COLOR_DEPTH_16,   HDMI_14,    0x3000,    0x3285,    0x061E,    0x04,    0x200,    0x80C8},
		{36,    	PIXEL_REPETITION_5,    	COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3285,    0x061E,    0x04,    0x200,    0x80C8},
		{36,    	PIXEL_REPETITION_5,    	COLOR_DEPTH_10,   HDMI_14,    0x1008,    0x3380,    0x064B,    0x04,    0x200,    0x80C8},
		{36,    	PIXEL_REPETITION_5,    	COLOR_DEPTH_12,   HDMI_14,    0x2000,    0x3303,    0x022D,    0x04,    0x200,    0x80C8},
		{36,    	PIXEL_REPETITION_5,    	COLOR_DEPTH_16,   HDMI_20,    0x3640,    0x33C2,    0x023C,    0x00,    0x200,    0x80F5},
		{40,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0003,    0x0285,    0x0228,    0x04,    0x200,    0x80C8},
		{43.2,    	PIXEL_REPETITION_4,    	COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3203,    0x0619,    0x04,    0x200,    0x80C8},
		{43.2,    	PIXEL_REPETITION_4,    	COLOR_DEPTH_12,   HDMI_14,    0x2008,    0x33C0,    0x024B,    0x04,    0x200,    0x80C8},
		{43.2,    	PIXEL_REPETITION_4,    	COLOR_DEPTH_16,   HDMI_20,    0x3640,    0x3342,    0x0232,    0x00,    0x200,    0x80F5},
		{44.9,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0003,    0x0285,    0x0228,    0x04,    0x200,    0x80C8},
		{49.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1183,    0x0614,    0x04,    0x200,    0x80C8},
		{50,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1183,    0x0614,    0x04,    0x200,    0x80C8},
		{50.35,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1183,    0x0614,    0x04,    0x200,    0x80C8},
		{50.35,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1002,    0x1203,    0x0619,    0x04,    0x200,    0x80C8},
		{50.35,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2002,    0x1202,    0x021E,    0x04,    0x200,    0x80C8},
		{50.35,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3001,    0x2183,    0x0614,    0x04,    0x200,    0x80C8},
		{54,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1183,    0x0614,    0x04,    0x200,    0x80C8},
		{54,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1002,    0x1203,    0x0619,    0x04,    0x200,    0x80C8},
		{54,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2002,    0x1202,    0x021E,    0x04,    0x200,    0x80C8},
		{54,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3001,    0x2183,    0x0614,    0x04,    0x200,    0x80C8},
		{54,    	PIXEL_REPETITION_1,   	COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2183,    0x0614,    0x04,    0x200,    0x80C8},
		{54,    	PIXEL_REPETITION_1,   	COLOR_DEPTH_10,   HDMI_14,    0x1001,    0x2203,    0x0619,    0x04,    0x200,    0x80C8},
		{54,    	PIXEL_REPETITION_1,   	COLOR_DEPTH_12,   HDMI_14,    0x2001,    0x2202,    0x021E,    0x04,    0x200,    0x80C8},
		{54,    	PIXEL_REPETITION_1,   	COLOR_DEPTH_16,   HDMI_14,    0x3000,    0x3183,    0x0614,    0x04,    0x200,    0x80C8},
		{54,    	PIXEL_REPETITION_3,   	COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3183,    0x0614,    0x04,    0x200,    0x80C8},
		{54,    	PIXEL_REPETITION_3,   	COLOR_DEPTH_10,   HDMI_14,    0x1018,    0x3380,    0x0664,    0x04,    0x200,    0x80C8},
		{54,    	PIXEL_REPETITION_3,   	COLOR_DEPTH_12,   HDMI_14,    0x2000,    0x3202,    0x021E,    0x04,    0x200,    0x80C8},
		{54,    	PIXEL_REPETITION_3,   	COLOR_DEPTH_16,   HDMI_20,    0x3640,    0x3285,    0x0228,    0x00,    0x200,    0x80F5},
		{56.25,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1183,    0x0614,    0x04,    0x200,    0x80C8},
		{59.4,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1183,    0x0614,    0x04,    0x200,    0x80C8},
		{59.4,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1002,    0x1182,    0x0219,    0x04,    0x200,    0x80C8},
		{59.4,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2002,    0x1202,    0x021E,    0x04,    0x200,    0x80C8},
		{59.4,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3001,    0x2183,    0x0614,    0x04,    0x200,    0x80C8},
		{65,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1183,    0x0614,    0x04,    0x200,    0x80C8},
		{68.25,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1183,    0x0614,    0x04,    0x200,    0x80C8},
		{71,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1183,    0x0614,    0x04,    0x200,    0x80C8},
		{72,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1142,    0x0214,    0x04,    0x200,    0x80C8},
		{72,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1002,    0x1182,    0x0219,    0x04,    0x200,    0x80C8},
		{72,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2001,    0x2141,    0x060F,    0x04,    0x200,    0x80C8},
		{72,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3001,    0x2142,    0x0214,    0x04,    0x200,    0x80C8},
		{72,    	PIXEL_REPETITION_2,    	COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3141,    0x060F,    0x04,    0x200,    0x80C8},
		{72,    	PIXEL_REPETITION_2,    	COLOR_DEPTH_10,   HDMI_14,    0x1018,    0x3380,    0x064B,    0x04,    0x200,    0x80C8},
		{72,    	PIXEL_REPETITION_2,    	COLOR_DEPTH_12,   HDMI_14,    0x2008,    0x3303,    0x022D,    0x04,    0x200,    0x80C8},
		{72,    	PIXEL_REPETITION_2,    	COLOR_DEPTH_16,   HDMI_20,    0x3640,    0x3202,    0x021E,    0x00,    0x200,    0x80F5},
		{73.25,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1142,    0x0214,    0x04,    0x200,    0x80C8},
		{74.25,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1142,    0x0214,    0x04,    0x200,    0x80C8},
		{74.25,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1009,    0x2203,    0x0619,    0x04,    0x200,    0x80C8},
		{74.25,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2001,    0x2141,    0x060F,    0x04,    0x200,    0x80C8},
		{74.25,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3001,    0x2142,    0x0214,    0x04,    0x200,    0x80C8},
		{75,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1142,    0x0214,    0x04,    0x200,    0x80C8},
		{78.75,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1142,    0x0214,    0x04,    0x200,    0x80C8},
		{79.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1142,    0x0214,    0x04,    0x200,    0x80C8},
		{82.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1142,    0x0214,    0x04,    0x200,    0x80C8},
		{82.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1009,    0x2203,    0x0619,    0x04,    0x200,    0x80C8},
		{82.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2001,    0x2141,    0x060F,    0x04,    0x200,    0x80C8},
//		{82.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3001,    0x2142,    0x0214,    0x04,    0x200,    0x80C8},
		{82.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3001,    0x2142,    0x0214,    0x04,    0x180,    0x82C8},
		{83.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1142,    0x0214,    0x04,    0x200,    0x80C8},
		{85.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1142,    0x0214,    0x04,    0x200,    0x80C8},
		{88.75,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1142,    0x0214,    0x04,    0x200,    0x80C8},
		{90,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0002,    0x1142,    0x0214,    0x04,    0x200,    0x80C8},
		{90,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1009,    0x2203,    0x0619,    0x04,    0x200,    0x80C8},
		{90,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2001,    0x2141,    0x060F,    0x04,    0x200,    0x80C8},
		{90,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3001,    0x2142,    0x0214,    0x04,    0x200,    0x80C8},
		{94.5,  	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x20C0,    0x060A,    0x04,    0x200,    0x80C8},
		{99,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x20C0,    0x060A,    0x04,    0x200,    0x80C8},
		{99,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1009,    0x2203,    0x0619,    0x04,    0x200,    0x80C8},
		{99,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2001,    0x2100,    0x020F,    0x04,    0x200,    0x80C8},
//		{99,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3000,    0x30C0,    0x060A,    0x04,    0x200,    0x80C8},
		{99,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3000,    0x30C0,    0x060A,    0x04,    0x180,    0x82C8},
		{100.7,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x20C0,    0x060A,    0x04,    0x200,    0x80C8},
		{100.7,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1009,    0x2203,    0x0619,    0x04,    0x200,    0x80C8},
		{100.7,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2001,    0x2100,    0x020F,    0x04,    0x200,    0x80C8},
		{100.7,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3000,    0x30C0,    0x060A,    0x04,    0x200,    0x80C8},
		{101,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x20C0,    0x060A,    0x04,    0x200,    0x80C8},
		{102.25,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x20C0,    0x060A,    0x04,    0x200,    0x80C8},
		{106.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x20C0,    0x060A,    0x04,    0x200,    0x80C8},
		{108,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x20C0,    0x060A,    0x04,    0x200,    0x80C8},
		{108,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1009,    0x2203,    0x0619,    0x04,    0x200,    0x80C8},
		{108,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2001,    0x2100,    0x020F,    0x04,    0x200,    0x80C8},
//		{108,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3000,    0x30C0,    0x060A,    0x04,    0x200,    0x80C8},
		{108,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3000,    0x30C0,    0x060A,    0x04,    0x180,    0x82C8},
		{108,    	PIXEL_REPETITION_1,    	COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x30C0,    0x060A,    0x04,    0x200,    0x80C8},
		{108,    	PIXEL_REPETITION_1,    	COLOR_DEPTH_10,   HDMI_14,    0x1018,    0x3341,    0x0632,    0x04,    0x200,    0x80C8},
		{108,    	PIXEL_REPETITION_1,    	COLOR_DEPTH_12,   HDMI_14,    0x2000,    0x3100,    0x020F,    0x04,    0x200,    0x80C8},
//		{108,    	PIXEL_REPETITION_1,    	COLOR_DEPTH_16,   HDMI_20,    0x3640,    0x3142,    0x0214,    0x00,    0x200,    0x80F5},
		{108,    	PIXEL_REPETITION_1,    	COLOR_DEPTH_16,   HDMI_20,    0x3640,    0x3142,    0x0214,    0x00,    0x180,    0x82C8},
		{108,    	PIXEL_REPETITION_2,    	COLOR_DEPTH_8,    HDMI_20,    0x0640,    0x3142,    0x0214,    0x00,    0x200,    0x80F5},
		{108,    	PIXEL_REPETITION_2,    	COLOR_DEPTH_10,   HDMI_20,    0x1640,    0x31C0,    0x0019,    0x00,    0x200,    0x80F5},
		{115.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x20C0,    0x060A,    0x04,    0x200,    0x80C8},
		{117.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x20C0,    0x060A,    0x04,    0x200,    0x80C8},
		{118.8,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x20C0,    0x060A,    0x04,    0x200,    0x80C8},
		{118.8,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1009,    0x2182,    0x0219,    0x04,    0x200,    0x80C8},
//		{118.8,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2001,    0x2100,    0x020F,    0x04,    0x200,    0x80C8},
//		{118.8,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3000,    0x30C0,    0x060A,    0x04,    0x200,    0x80C8},
		{118.8,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2001,    0x2100,    0x020F,    0x04,    0x180,    0x82C8},
		{118.8,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3000,    0x30C0,    0x060A,    0x04,    0x180,    0x82C8},
		{119,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x20C0,    0x060A,    0x04,    0x200,    0x80C8},
		{121.75,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x20C0,    0x060A,    0x04,    0x200,    0x80C8},
		{122.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x20C0,    0x060A,    0x04,    0x200,    0x80C8},
		{135,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x20C0,    0x060A,    0x04,    0x200,    0x80C8},
		{136.75,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x20C0,    0x060A,    0x04,    0x200,    0x80C8},
		{140.25,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x20C0,    0x060A,    0x04,    0x200,    0x80C8},
		{144,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2080,    0x020A,    0x04,    0x200,    0x80C8},
		{144,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1009,    0x2182,    0x0219,    0x04,    0x200,    0x80C8},
		{144,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2008,    0x3141,    0x060F,    0x04,    0x200,    0x80C8},
		{144,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3000,    0x3080,    0x020A,    0x04,    0x200,    0x80C8},
		{146.25,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2080,    0x020A,    0x04,    0x200,    0x80C8},
		{148.25,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2080,    0x020A,    0x04,    0x200,    0x80C8},
		{148.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2080,    0x020A,    0x04,    0x200,    0x80C8},
//		{148.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1018,    0x3203,    0x0619,    0x04,    0x200,    0x80C8},
//		{148.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2008,    0x3141,    0x060F,    0x04,    0x200,    0x80C8},
		{148.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1018,    0x3203,    0x0619,    0x04,    0x180,    0x82C8},
		{148.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2008,    0x3141,    0x060F,    0x04,    0x180,    0x82C8},
		{148.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3000,    0x3080,    0x020A,    0x04,    0x180,    0x82C8},
		{154,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2080,    0x020A,    0x04,    0x200,    0x80C8},
		{156,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2080,    0x020A,    0x04,    0x200,    0x80C8},
		{157,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2080,    0x020A,    0x04,    0x200,    0x80C8},
		{157.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2080,    0x020A,    0x04,    0x200,    0x80C8},
		{162,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2080,    0x020A,    0x04,    0x200,    0x80C8},
		{162,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2080,    0x020A,    0x04,    0x200,    0x80C8},
		{165,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2080,    0x020A,    0x04,    0x180,    0x82C8},
		{165,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1018,    0x3203,    0x0619,    0x04,    0x180,    0x82C8},
		{165,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2008,    0x3141,    0x060F,    0x04,    0x180,    0x82C8},
		{165,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_14,    0x3000,    0x3080,    0x020A,    0x04,    0x180,    0x82C8},
		{175.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2080,    0x020A,    0x04,    0x180,    0x82C8},
		{179.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2080,    0x020A,    0x04,    0x180,    0x82C8},
		{180,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2080,    0x020A,    0x04,    0x180,    0x82C8},
		{180,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1018,    0x3203,    0x0619,    0x04,    0x180,    0x82C8},
		{180,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2008,    0x3141,    0x060F,    0x04,    0x180,    0x82C8},
		{180,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_20,    0x3640,    0x3080,    0x020A,    0x00,    0x180,    0x82C8},
		{182.75,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0001,    0x2080,    0x020A,    0x04,    0x180,    0x82C8},
		{185.625,   PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{185.625,   PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1018,    0x3203,    0x0619,    0x04,    0x180,    0x82C8},
		{185.625,   PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2008,    0x3141,    0x060F,    0x04,    0x180,    0x82C8},
		{185.625,   PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_20,    0x3640,    0x3080,    0x020A,    0x00,    0x180,    0x82C8},
		{187,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{187.25,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{189,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{193.25,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{198,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{198,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1018,    0x3203,    0x0619,    0x04,    0x180,    0x82C8},
		{198,   	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2008,    0x3100,    0x020F,    0x04,    0x180,    0x82C8},
		{198,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_20,    0x3640,    0x3080,    0x020A,    0x00,    0x180,    0x82C8},
		{202.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{204.75,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{208,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{214.75,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{216,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{216,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1018,    0x3203,    0x0619,    0x04,    0x180,    0x82C8},
		{216,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_14,    0x2008,    0x3100,    0x020F,    0x04,    0x180,    0x82C8},
		{216,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_20,    0x3640,    0x3080,    0x020A,    0x00,    0x180,    0x82C8},
		{218.25,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
//		{222.75,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_20,    0x3640,    0x3080,    0x020A,    0x00,    0x180,    0x82C8},
		{229.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{234,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{237.6,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{237.6,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_14,    0x1018,    0x3182,    0x0219,    0x04,    0x180,    0x82C8},
		{237.6,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_20,    0x2648,    0x3100,    0x020F,    0x00,    0x180,    0x82C8},
		{237.6,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_20,    0x3640,    0x3080,    0x000A,    0x00,    0x180,    0x82C8},
		{245.25,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{245.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{261,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{268.25,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{268.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{281.25,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3040,    0x0605,    0x04,    0x180,    0x82C8},
		{288,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3041,    0x0205,    0x04,    0x180,    0x82C8},
		{288,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_20,    0x1658,    0x3182,    0x0219,    0x00,    0x180,    0x82C8},
		{288,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_20,    0x2648,    0x3100,    0x020F,    0x00,    0x180,    0x82C8},
		{288,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_20,    0x3640,    0x3080,    0x000A,    0x00,    0x180,    0x82C8},
		{297,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3041,    0x0205,    0x04,    0x180,    0x82C8},
		{297,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_20,    0x1658,    0x3182,    0x0219,    0x00,    0x180,    0x82C8},
		{297,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_20,    0x2648,    0x3100,    0x020F,    0x00,    0x180,    0x82C8},
		{297,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_16,   HDMI_20,    0x3640,    0x3080,    0x000A,    0x00,    0x180,    0x82C8},
		{317,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3041,    0x0205,    0x04,    0x180,    0x82C8},
		{330,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3041,    0x0205,    0x04,    0x180,    0x82C8},
		{330,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_20,    0x1658,    0x3182,    0x0219,    0x00,    0x180,    0x80F5},
		{330,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_20,    0x2648,    0x3100,    0x000F,    0x00,    0x180,    0x80F5},
		{333.25,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3041,    0x0205,    0x04,    0x180,    0x82C8},
		{340,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_14,    0x0000,    0x3041,    0x0205,    0x04,    0x100,    0x82C8},
		{348.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_20,    0x0640,    0x3041,    0x0205,    0x00,    0x100,    0x80F5},
		{356.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_20,    0x0640,    0x3041,    0x0205,    0x00,    0x100,    0x80F5},
		{360,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_20,    0x0640,    0x3041,    0x0205,    0x00,    0x100,    0x80F5},
		{360,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_20,    0x1658,    0x3182,    0x0219,    0x00,    0x100,    0x80F5},
		{360,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_20,    0x2648,    0x3100,    0x000F,    0x00,    0x100,    0x80F5},
		{371.25,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_20,    0x0640,    0x3041,    0x0205,    0x00,    0x100,    0x80F5},
		{371.25,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_20,    0x1658,    0x3182,    0x0219,    0x00,    0x100,    0x80F5},
		{371.25,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_20,    0x2648,    0x3100,    0x000F,    0x00,    0x100,    0x80F5},
		{380.5,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_20,    0x0640,    0x3041,    0x0205,    0x00,    0x100,    0x80F5},
		{396,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_20,    0x0640,    0x3041,    0x0205,    0x00,    0x100,    0x80F5},
		{396,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_20,    0x1658,    0x31C0,    0x0019,    0x00,    0x100,    0x80F5},
		{396,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_12,   HDMI_20,    0x2648,    0x3100,    0x000F,    0x00,    0x100,    0x80F5},
		{432,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_20,    0x0640,    0x3041,    0x0205,    0x00,    0x100,    0x80F5},
		{432,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_20,    0x1658,    0x31C0,    0x0019,    0x00,    0x100,    0x80F5},
		{475.2,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_20,    0x0640,    0x3041,    0x0005,    0x00,    0x100,    0x80F5},
		{475.2,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_10,   HDMI_20,    0x1658,    0x31C0,    0x0019,    0x00,    0x100,    0x80F5},
		{495,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_20,    0x0640,    0x3041,    0x0005,    0x00,    0x100,    0x80F5},
		{505.25,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_20,    0x0640,    0x3041,    0x0005,    0x00,    0x100,    0x80F5},
		{552.75,    PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_20,    0x0640,    0x3041,    0x0005,    0x00,    0x100,    0x80F5},
		{594,    	PIXEL_REPETITION_OFF,   COLOR_DEPTH_8,    HDMI_20,    0x0640,    0x3041,    0x0005,    0x00,    0x100,    0x80F5},
		{0,  		PIXEL_REPETITION_OFF, 	COLOR_DEPTH_INVALID, HDMI_14, 0, 0, 0, 0, 0, 0}
};

int phy305_configure_supported(hdmi_tx_dev_t *dev, double pClk, color_depth_t color,
		pixel_repetition_t pixel)
{
	int i   = 0;

	// Color resolution 0 is 8 bit color depth
	if (color == 0)
		color = COLOR_DEPTH_8;

	pClk = phy_get_freq(pClk);

	for (i = 0; phy305[i].clock != 0; i++ ){
		if(double_is_equal(pClk, phy305[i].clock) &&
				(color == phy305[i].color) &&
				(pixel == phy305[i].pixel)){
			return TRUE;
		}
	}

	return FALSE;
}

struct phy_config * phy305_get_configs(double pClk, color_depth_t color,
		pixel_repetition_t pixel)
{

	int i = 0;

	for (i = 0; phy305[i].clock != 0; i++ ){

		pClk = phy_get_freq(pClk);

		if(double_is_equal(pClk, phy305[i].clock) &&
				(color == phy305[i].color) &&
				(pixel == phy305[i].pixel)){
			return &(phy305[i]);
		}
	}

	return NULL;
}

int phy305_configure(hdmi_tx_dev_t *dev, double pClk, color_depth_t color, pixel_repetition_t pixel)
{
	int i   = 0;
	u32 phyRead = 0;
	u8 lock = 0;
	struct phy_config * config = NULL;
	u32 value = 0;

	// Color resolution 0 is 8 bit color depth
	if (color == 0)
		color = COLOR_DEPTH_8;

	config = phy305_get_configs(pClk, color, pixel);

	if (config == NULL) {
		LOGGER(SNPS_ERROR,"Configuration for clk %f color depth %d"
				  " pixel repetition %d", pClk, color, pixel);
		return -1;
	}

	value = dev_read(dev, 0xb0020);
	value |= (1 << 9);
	dev_write(dev, 0xb0020, value);

	board_ZcalReset(1);

	mc_phy_reset(dev, 1);

	dev_write_mask(dev, PHY_CONF0, PHY_CONF0_TXPWRON_MASK, 0);
	dev_write_mask(dev, PHY_CONF0, PHY_CONF0_PDDQ_MASK, 1);
	dev_write_mask(dev, PHY_CONF0, PHY_CONF0_SVSRET_MASK, 0);

	board_ZcalReset(0);

	do{
		snps_sleep(5);
	} while(!(dev_read(dev, 0xb0010) & 0x1) && (i++ < PHY_TIMEOUT));
	
	dev_write_mask(dev, PHY_CONF0, PHY_CONF0_SVSRET_MASK, 1);

	mc_phy_reset(dev, 0);

	phy_reconfigure_interface(dev);

	phy_write(dev, OPMODE_PLLCFG, config->oppllcfg);
	if(phy_read(dev, OPMODE_PLLCFG, &phyRead) || (phyRead != config->oppllcfg))
		LOGGER(SNPS_ERROR, "%s:OPMODE_PLLCFG Mismatch Write 0x%04x Read 0x%04x",
				__func__, config->oppllcfg , phyRead);

	phy_write(dev, PLLCURRCTRL, config->pllcurrctrl);
	if(phy_read(dev, PLLCURRCTRL, &phyRead) || (phyRead != config->pllcurrctrl))
		LOGGER(SNPS_ERROR, "%s:PLLCURRCTRL Mismatch Write 0x%04x Read 0x%04x",
				__func__, config->pllcurrctrl , phyRead);

	phy_write(dev, PLLGMPCTRL, config->pllgmpctrl);
	if(phy_read(dev, PLLGMPCTRL, &phyRead) || (phyRead != config->pllgmpctrl))
		LOGGER(SNPS_ERROR, "%s:PLLGMPCTRL Mismatch Write 0x%04x Read 0x%04x",
				__func__, config->pllgmpctrl , phyRead);

	phy_write(dev, TXTERM, config->txterm);
	if(phy_read(dev, TXTERM, &phyRead) || (phyRead != config->txterm))
		LOGGER(SNPS_ERROR, "%s:TXTERM Mismatch Write 0x%04x Read 0x%04x",
				__func__, config->txterm , phyRead);

	phy_write(dev, CKSYMTXCTRL, config->cksymtxctrl);
	if(phy_read(dev, CKSYMTXCTRL, &phyRead) || (phyRead != config->cksymtxctrl))
		LOGGER(SNPS_ERROR, "%s:CKSYMTXCTRL Mismatch Write 0x%04x Read 0x%04x",
				__func__, config->cksymtxctrl , phyRead);

	phy_write(dev, VLEVCTRL, config->vlevctrl);
	if(phy_read(dev, VLEVCTRL, &phyRead) || (phyRead != config->vlevctrl))
		LOGGER(SNPS_ERROR, "%s:VLEVCTRL Mismatch Write 0x%04x Read 0x%04x",
				__func__, config->vlevctrl , phyRead);

	dev_write_mask(dev, PHY_CONF0, PHY_CONF0_PDDQ_MASK, 0);
	dev_write_mask(dev, PHY_CONF0, PHY_CONF0_TXPWRON_MASK, 1);

	/* wait PHY_TIMEOUT no of cycles at most for the PLL lock signal to raise ~around 20us max */
	for (i = 0; i < PHY_TIMEOUT; i++) {
		snps_sleep(5);
		lock = phy_phase_lock_loop_state(dev);
		if (lock & 0x1) {
			LOGGER(SNPS_DEBUG,"PHY PLL locked");
			return TRUE;
		}
	}

	error_set(ERR_PHY_NOT_LOCKED);
	LOGGER(SNPS_ERROR,"PHY PLL not locked");
	return FALSE;
}
