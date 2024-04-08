// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "phy_reg.h"
#include "phy_316.h"
#include "phy_jtag.h"
#include "general_ops.h"


#define OPMODE_PLLCFG	0x06 // Mode of Operation and PLL  Dividers Control Register
#define PLLCURRCTRL		0x10 // PLL Current Control Register
#define PLLDIVCTRL		0x11 // PLL Dividers Control Register
#define TXTERM			0x19 // Transmission Termination Register
#define VLEVCTRL		0x0E // Voltage Level Control Register
#define CKSYMTXCTRL		0x09 // Clock Symbol and Transmitter Control Register

#define LT_1_65GBPS_TXTERM 		0x0007
#define LT_1_65GBPS_VLEVCTRL 	0x01A0
#define LT_1_65GBPS_CKSYMTXCTRL 0x8088

#define LT_3_40GBPS_TXTERM 		0x0000
#define LT_3_40GBPS_VLEVCTRL 	0x0120
#define LT_3_40GBPS_CKSYMTXCTRL 0x83F8

#define GT_3_40GBPS_TXTERM 		0x0000
#define GT_3_40GBPS_VLEVCTRL 	0x0140
#define GT_3_40GBPS_CKSYMTXCTRL 0x80F6

#define LT_1_65GBPS LT_1_65GBPS_TXTERM, LT_1_65GBPS_VLEVCTRL, LT_1_65GBPS_CKSYMTXCTRL
#define LT_3_40GBPS LT_3_40GBPS_TXTERM, LT_3_40GBPS_VLEVCTRL, LT_3_40GBPS_CKSYMTXCTRL
#define GT_3_40GBPS GT_3_40GBPS_TXTERM, GT_3_40GBPS_VLEVCTRL, GT_3_40GBPS_CKSYMTXCTRL

//rterm		0x19	d_tx_term[2:0]	TXTERM
//txlvl		0x0E	sup_tx_lvl[4:0] TXLVL
//cksymon	0x09	ck_symon[3:0] 	SYMON
//traon		0x09 	tx_traon 		TRAON
//trbon		0x09	tx_trbon		TRBON


//	Data rate		rterm	txlvl/cklvl	cksymon		txsymon		pre-empth	slopeboost
//					0x19	0x0E		0x09		0x09		0x09		0x09
//																traon/trbon
//	<=1.65			3'b100	5'b01111	4'b1000		4'b1100		1'b0/1'b0	4'b0000
//	>1.65 & < 3.4	3'b100	5'b01100	4'b1000		4'b1100		1'b1/1'b1	4'b0000
//	>3.4			3'b000	5'b01001	4'b0101		4'b1111 	1'b0/1'b0	4'b0000

static struct phy_config phy316[] = {
	{13.5, 1, 8, HDMI_14, 0x0003, 0x0280, 0x0650, LT_1_65GBPS},
	{13.5, 1, 10, HDMI_14, 0x1003, 0x0280, 0x0664, LT_1_65GBPS},
	{13.5, 1, 12, HDMI_14, 0x2003, 0x02C0, 0x0278, LT_1_65GBPS},
	{13.5, 1, 16, HDMI_14, 0x3002, 0x1280, 0x0650, LT_1_65GBPS},
	{13.5, 3, 8, HDMI_14, 0x0002, 0x1280, 0x0650, LT_1_65GBPS},
	{13.5, 3, 10, HDMI_14, 0x1002, 0x1280, 0x0664, LT_1_65GBPS},
	{13.5, 3, 12, HDMI_14, 0x2002, 0x12C0, 0x0278, LT_1_65GBPS},
	{13.5, 3, 16, HDMI_14, 0x3001, 0x2280, 0x0650, LT_1_65GBPS},
	{13.5, 7, 8, HDMI_14, 0x0001, 0x2280, 0x0650, LT_1_65GBPS},
	{13.5, 7, 10, HDMI_14, 0x1001, 0x2280, 0x0664, LT_1_65GBPS},
	{13.5, 7, 12, HDMI_14, 0x2001, 0x22C0, 0x0278, LT_1_65GBPS},
	{13.5, 7, 16, HDMI_14, 0x3000, 0x3280, 0x0650, LT_3_40GBPS},
	{18, 2, 8, HDMI_14, 0x0002, 0x1280, 0x063C, LT_1_65GBPS},
	{18, 2, 10, HDMI_14, 0x1002, 0x1280, 0x064B, LT_1_65GBPS},
	{18, 2, 12, HDMI_14, 0x2002, 0x12C0, 0x025A, LT_1_65GBPS},
	{18, 2, 16, HDMI_14, 0x3001, 0x2280, 0x063C, LT_1_65GBPS},
	{18, 5, 8, HDMI_14, 0x0001, 0x2280, 0x063C, LT_1_65GBPS},
	{18, 5, 10, HDMI_14, 0x1001, 0x2280, 0x064B, LT_1_65GBPS},
	{18, 5, 12, HDMI_14, 0x2001, 0x22C0, 0x025A, LT_1_65GBPS},
	{18, 5, 16, HDMI_14, 0x3000, 0x3280, 0x063C, LT_3_40GBPS},
	{21.6, 4, 8, HDMI_14, 0x0001, 0x2281, 0x0632, LT_1_65GBPS},
	{21.6, 4, 12, HDMI_14, 0x2001, 0x22C0, 0x024B, LT_1_65GBPS},
	{21.6, 4, 16, HDMI_14, 0x3000, 0x3281, 0x0632, LT_3_40GBPS},
	{21.6, 9, 8, HDMI_14, 0x0000, 0x3281, 0x0632, LT_3_40GBPS},
	{21.6, 9, 12, HDMI_14, 0x2000, 0x32C0, 0x024B, LT_3_40GBPS},
	{21.6, 9, 16, HDMI_20, 0x3640, 0x32C0, 0x0264, GT_3_40GBPS},
	{24, 8, 8, HDMI_14, 0x0000, 0x3282, 0x062D, LT_3_40GBPS},
	{24, 8, 16, HDMI_20, 0x3640, 0x32C0, 0x025A, GT_3_40GBPS},
	{25.175, 0, 8, HDMI_14, 0x0003, 0x0283, 0x0628, LT_1_65GBPS},
	{25.175, 0, 10, HDMI_14, 0x1003, 0x0281, 0x0632, LT_1_65GBPS},
	{25.175, 0, 12, HDMI_14, 0x2003, 0x02C2, 0x023C, LT_1_65GBPS},
	{25.175, 0, 16, HDMI_14, 0x3002, 0x1283, 0x0628, LT_1_65GBPS},
	{27, 0, 8, HDMI_14, 0x0003, 0x0283, 0x0628, LT_1_65GBPS},
	{27, 0, 10, HDMI_14, 0x1003, 0x0281, 0x0632, LT_1_65GBPS},
	{27, 0, 12, HDMI_14, 0x2003, 0x02C2, 0x023C, LT_1_65GBPS},
	{27, 0, 16, HDMI_14, 0x3002, 0x1283, 0x0628, LT_1_65GBPS},
	{27, 1, 8, HDMI_14, 0x0002, 0x1283, 0x0628, LT_1_65GBPS},
	{27, 1, 10, HDMI_14, 0x1002, 0x1281, 0x0632, LT_1_65GBPS},
	{27, 1, 12, HDMI_14, 0x2002, 0x12C2, 0x023C, LT_1_65GBPS},
	{27, 1, 16, HDMI_14, 0x3001, 0x2283, 0x0628, LT_1_65GBPS},
	{27, 3, 8, HDMI_14, 0x0001, 0x2283, 0x0628, LT_1_65GBPS},
	{27, 3, 10, HDMI_14, 0x1001, 0x2281, 0x0632, LT_1_65GBPS},
	{27, 3, 12, HDMI_14, 0x2001, 0x22C2, 0x023C, LT_1_65GBPS},
	{27, 3, 16, HDMI_14, 0x3000, 0x3283, 0x0628, LT_3_40GBPS},
	{27, 7, 8, HDMI_14, 0x0000, 0x3283, 0x0628, LT_3_40GBPS},
	{27, 7, 10, HDMI_14, 0x1000, 0x3281, 0x0632, LT_3_40GBPS},
	{27, 7, 12, HDMI_14, 0x2000, 0x32C2, 0x023C, LT_3_40GBPS},
	{27, 7, 16, HDMI_20, 0x3640, 0x32C0, 0x0250, GT_3_40GBPS},
	{31.5, 0, 8, HDMI_14, 0x0003, 0x0283, 0x0628, LT_1_65GBPS},
	{33.75, 0, 8, HDMI_14, 0x0003, 0x0283, 0x0628, LT_1_65GBPS},
	{35.5, 0, 8, HDMI_14, 0x0003, 0x0283, 0x0628, LT_1_65GBPS},
	{36, 0, 8, HDMI_14, 0x0003, 0x0285, 0x0228, LT_1_65GBPS},
	{36, 2, 8, HDMI_14, 0x0001, 0x2285, 0x061E, LT_1_65GBPS},
	{36, 2, 10, HDMI_14, 0x1009, 0x2280, 0x064B, LT_1_65GBPS},
	{36, 2, 12, HDMI_14, 0x2001, 0x22C3, 0x022D, LT_1_65GBPS},
	{36, 2, 16, HDMI_14, 0x3000, 0x3285, 0x061E, LT_3_40GBPS},
	{36, 5, 8, HDMI_14, 0x0000, 0x3285, 0x061E, LT_3_40GBPS},
	{36, 5, 10, HDMI_14, 0x1008, 0x3280, 0x064B, LT_3_40GBPS},
	{36, 5, 12, HDMI_14, 0x2000, 0x32C3, 0x022D, LT_3_40GBPS},
	{36, 5, 16, HDMI_20, 0x3640, 0x32C2, 0x023C, GT_3_40GBPS},
	{40, 0, 8, HDMI_14, 0x0003, 0x0285, 0x0228, LT_1_65GBPS},
	{43.2, 4, 8, HDMI_14, 0x0000, 0x3203, 0x0619, LT_3_40GBPS},
	{43.2, 4, 12, HDMI_14, 0x2008, 0x32C0, 0x024B, LT_3_40GBPS},
	{43.2, 4, 16, HDMI_20, 0x3640, 0x32C2, 0x0232, GT_3_40GBPS},
	{44.9, 0, 8, HDMI_14, 0x0003, 0x0285, 0x0228, LT_1_65GBPS},
	{49.5, 0, 8, HDMI_14, 0x0002, 0x1183, 0x0614, LT_1_65GBPS},
	{50, 0, 8, HDMI_14, 0x0002, 0x1183, 0x0614, LT_1_65GBPS},
	{50.35, 0, 8, HDMI_14, 0x0002, 0x1183, 0x0614, LT_1_65GBPS},
	{50.35, 0, 10, HDMI_14, 0x1002, 0x1203, 0x0619, LT_1_65GBPS},
	{50.35, 0, 12, HDMI_14, 0x2002, 0x1202, 0x021E, LT_1_65GBPS},
	{50.35, 0, 16, HDMI_14, 0x3001, 0x2183, 0x0614, LT_1_65GBPS},
	{54, 0, 8, HDMI_14, 0x0002, 0x1183, 0x0614, LT_1_65GBPS},
	{54, 0, 10, HDMI_14, 0x1002, 0x1203, 0x0619, LT_1_65GBPS},
	{54, 0, 12, HDMI_14, 0x2002, 0x1202, 0x021E, LT_1_65GBPS},
	{54, 0, 16, HDMI_14, 0x3001, 0x2183, 0x0614, LT_1_65GBPS},
	{54, 1, 8, HDMI_14, 0x0001, 0x2183, 0x0614, LT_1_65GBPS},
	{54, 1, 10, HDMI_14, 0x1001, 0x2203, 0x0619, LT_1_65GBPS},
	{54, 1, 12, HDMI_14, 0x2001, 0x2202, 0x021E, LT_1_65GBPS},
	{54, 1, 16, HDMI_14, 0x3000, 0x3183, 0x0614, LT_3_40GBPS},
	{54, 3, 8, HDMI_14, 0x0000, 0x3183, 0x0614, LT_3_40GBPS},
	{54, 3, 10, HDMI_14, 0x1018, 0x3280, 0x0664, LT_3_40GBPS},
	{54, 3, 12, HDMI_14, 0x2000, 0x3202, 0x021E, LT_3_40GBPS},
	{54, 3, 16, HDMI_20, 0x3640, 0x3285, 0x0228, GT_3_40GBPS},
	{56.25, 0, 8, HDMI_14, 0x0002, 0x1183, 0x0614, LT_1_65GBPS},
	{59.4, 0, 8, HDMI_14, 0x0002, 0x1183, 0x0614, LT_1_65GBPS},
	{59.4, 0, 10, HDMI_14, 0x1002, 0x1182, 0x0219, LT_1_65GBPS},
	{59.4, 0, 12, HDMI_14, 0x2002, 0x1202, 0x021E, LT_1_65GBPS},
	{59.4, 0, 16, HDMI_14, 0x3001, 0x2183, 0x0614, LT_1_65GBPS},
	{65, 0, 8, HDMI_14, 0x0002, 0x1183, 0x0614, LT_1_65GBPS},
	{68.25, 0, 8, HDMI_14, 0x0002, 0x1183, 0x0614, LT_1_65GBPS},
	{71, 0, 8, HDMI_14, 0x0002, 0x1183, 0x0614, LT_1_65GBPS},
	{72, 0, 8, HDMI_14, 0x0002, 0x1142, 0x0214, LT_1_65GBPS},
	{72, 0, 10, HDMI_14, 0x1002, 0x1182, 0x0219, LT_1_65GBPS},
	{72, 0, 12, HDMI_14, 0x2001, 0x2141, 0x060F, LT_1_65GBPS},
	{72, 0, 16, HDMI_14, 0x3001, 0x2142, 0x0214, LT_1_65GBPS},
	{72, 2, 8, HDMI_14, 0x0000, 0x3141, 0x060F, LT_3_40GBPS},
	{72, 2, 10, HDMI_14, 0x1018, 0x3280, 0x064B, LT_3_40GBPS},
	{72, 2, 12, HDMI_14, 0x2008, 0x32C3, 0x022D, LT_3_40GBPS},
	{72, 2, 16, HDMI_20, 0x3640, 0x3202, 0x021E, GT_3_40GBPS},
	{73.25, 0, 8, HDMI_14, 0x0002, 0x1142, 0x0214, LT_1_65GBPS},
	{74.25, 0, 8, HDMI_14, 0x0002, 0x1142, 0x0214, LT_1_65GBPS},
	{74.25, 0, 10, HDMI_14, 0x1009, 0x2203, 0x0619, LT_1_65GBPS},
	{74.25, 0, 12, HDMI_14, 0x2001, 0x2141, 0x060F, LT_1_65GBPS},
	{74.25, 0, 16, HDMI_14, 0x3001, 0x2142, 0x0214, LT_1_65GBPS},
	{75, 0, 8, HDMI_14, 0x0002, 0x1142, 0x0214, LT_1_65GBPS},
	{78.75, 0, 8, HDMI_14, 0x0002, 0x1142, 0x0214, LT_1_65GBPS},
	{79.5, 0, 8, HDMI_14, 0x0002, 0x1142, 0x0214, LT_1_65GBPS},
	{82.5, 0, 8, HDMI_14, 0x0002, 0x1142, 0x0214, LT_1_65GBPS},
	{82.5, 0, 10, HDMI_14, 0x1009, 0x2203, 0x0619, LT_1_65GBPS},
	{82.5, 0, 12, HDMI_14, 0x2001, 0x2141, 0x060F, LT_1_65GBPS},
	{82.5, 0, 16, HDMI_14, 0x3001, 0x2142, 0x0214, LT_1_65GBPS},
	{83.5, 0, 8, HDMI_14, 0x0002, 0x1142, 0x0214, LT_1_65GBPS},
	{85.5, 0, 8, HDMI_14, 0x0002, 0x1142, 0x0214, LT_1_65GBPS},
	{88.75, 0, 8, HDMI_14, 0x0002, 0x1142, 0x0214, LT_1_65GBPS},
	{90, 0, 8, HDMI_14, 0x0002, 0x1142, 0x0214, LT_1_65GBPS},
	{90, 0, 10, HDMI_14, 0x1009, 0x2203, 0x0619, LT_1_65GBPS},
	{90, 0, 12, HDMI_14, 0x2001, 0x2141, 0x060F, LT_1_65GBPS},
	{90, 0, 16, HDMI_14, 0x3001, 0x2142, 0x0214, LT_3_40GBPS},
	{94.5, 0, 8, HDMI_14, 0x0001, 0x20C0, 0x060A, LT_1_65GBPS},
	{99, 0, 8, HDMI_14, 0x0001, 0x20C0, 0x060A, LT_1_65GBPS},
	{99, 0, 10, HDMI_14, 0x1009, 0x2203, 0x0619, LT_1_65GBPS},
	{99, 0, 12, HDMI_14, 0x2001, 0x2100, 0x020F, LT_1_65GBPS},
	{99, 0, 16, HDMI_14, 0x3000, 0x30C0, 0x060A, LT_3_40GBPS},
	{100.7, 0, 8, HDMI_14, 0x0001, 0x20C0, 0x060A, LT_1_65GBPS},
	{100.7, 0, 10, HDMI_14, 0x1009, 0x2203, 0x0619, LT_1_65GBPS},
	{100.7, 0, 12, HDMI_14, 0x2001, 0x2100, 0x020F, LT_1_65GBPS},
	{100.7, 0, 16, HDMI_14, 0x3000, 0x30C0, 0x060A, LT_3_40GBPS},
	{101, 0, 8, HDMI_14, 0x0001, 0x20C0, 0x060A, LT_1_65GBPS},
	{102.25, 0, 8, HDMI_14, 0x0001, 0x20C0, 0x060A, LT_1_65GBPS},
	{106.5, 0, 8, HDMI_14, 0x0001, 0x20C0, 0x060A, LT_1_65GBPS},
	{108, 0, 8, HDMI_14, 0x0001, 0x20C0, 0x060A, LT_1_65GBPS},
	{108, 0, 10, HDMI_14, 0x1009, 0x2203, 0x0619, LT_1_65GBPS},
	{108, 0, 12, HDMI_14, 0x2001, 0x2100, 0x020F, LT_1_65GBPS},
	{108, 0, 16, HDMI_14, 0x3000, 0x30C0, 0x060A, LT_3_40GBPS},
	{108, 1, 8, HDMI_14, 0x0000, 0x30C0, 0x060A, LT_3_40GBPS},
	{108, 1, 10, HDMI_14, 0x1018, 0x3281, 0x0632, LT_3_40GBPS},
	{108, 1, 12, HDMI_14, 0x2000, 0x3100, 0x020F, LT_3_40GBPS},
	{108, 1, 16, HDMI_20, 0x3640, 0x3142, 0x0214, GT_3_40GBPS},
	{108, 2, 8, HDMI_20, 0x0640, 0x3142, 0x0214, LT_3_40GBPS},
	{108, 2, 10, HDMI_20, 0x1640, 0x31C0, 0x0019, GT_3_40GBPS},
	{115.5, 0, 8, HDMI_14, 0x0001, 0x20C0, 0x060A, LT_1_65GBPS},
	{117.5, 0, 8, HDMI_14, 0x0001, 0x20C0, 0x060A, LT_1_65GBPS},
	{118.8, 0, 8, HDMI_14, 0x0001, 0x20C0, 0x060A, LT_1_65GBPS},
	{118.8, 0, 10, HDMI_14, 0x1009, 0x2182, 0x0219, LT_1_65GBPS},
	{118.8, 0, 12, HDMI_14, 0x2001, 0x2100, 0x020F, LT_3_40GBPS},
	{118.8, 0, 16, HDMI_14, 0x3000, 0x30C0, 0x060A, LT_3_40GBPS},
	{119, 0, 8, HDMI_14, 0x0001, 0x20C0, 0x060A, LT_1_65GBPS},
	{121.75, 0, 8, HDMI_14, 0x0001, 0x20C0, 0x060A, LT_1_65GBPS},
	{122.5, 0, 8, HDMI_14, 0x0001, 0x20C0, 0x060A, LT_1_65GBPS},
	{135, 0, 8, HDMI_14, 0x0001, 0x20C0, 0x060A, LT_1_65GBPS},
	{136.75, 0, 8, HDMI_14, 0x0001, 0x20C0, 0x060A, LT_1_65GBPS},
	{140.25, 0, 8, HDMI_14, 0x0001, 0x20C0, 0x060A, LT_1_65GBPS},
	{144, 0, 8, HDMI_14, 0x0001, 0x2080, 0x020A, LT_1_65GBPS},
	{144, 0, 10, HDMI_14, 0x1009, 0x2182, 0x0219, LT_3_40GBPS},
	{144, 0, 12, HDMI_14, 0x2008, 0x3141, 0x060F, LT_3_40GBPS},
	{144, 0, 16, HDMI_14, 0x3000, 0x3080, 0x020A, LT_3_40GBPS},
	{146.25, 0, 8, HDMI_14, 0x0001, 0x2080, 0x020A, LT_1_65GBPS},
	{148.25, 0, 8, HDMI_14, 0x0001, 0x2080, 0x020A, LT_1_65GBPS},
	{148.5, 0, 8, HDMI_14, 0x0001, 0x2080, 0x020A, LT_1_65GBPS},
	{148.5, 0, 10, HDMI_14, 0x1018, 0x3203, 0x0619, LT_3_40GBPS},
	{148.5, 0, 12, HDMI_14, 0x2008, 0x3141, 0x060F, LT_3_40GBPS},
	{148.5, 0, 16, HDMI_14, 0x3000, 0x3080, 0x020A, LT_3_40GBPS},
	{154, 0, 8, HDMI_14, 0x0001, 0x2080, 0x020A, LT_1_65GBPS},
	{156, 0, 8, HDMI_14, 0x0001, 0x2080, 0x020A, LT_1_65GBPS},
	{157, 0, 8, HDMI_14, 0x0001, 0x2080, 0x020A, LT_1_65GBPS},
	{157.5, 0, 8, HDMI_14, 0x0001, 0x2080, 0x020A, LT_1_65GBPS},
	{162, 0, 8, HDMI_14, 0x0001, 0x2080, 0x020A, LT_1_65GBPS},
	{162, 0, 8, HDMI_14, 0x0001, 0x2080, 0x020A, LT_1_65GBPS},
	{165, 0, 8, HDMI_14, 0x0001, 0x2080, 0x020A, LT_1_65GBPS},
	{165, 0, 10, HDMI_14, 0x1018, 0x3203, 0x0619, LT_3_40GBPS},
	{165, 0, 12, HDMI_14, 0x2008, 0x3141, 0x060F, LT_3_40GBPS},
	{165, 0, 16, HDMI_14, 0x3000, 0x3080, 0x020A, LT_3_40GBPS},
	{175.5, 0, 8, HDMI_14, 0x0001, 0x2080, 0x020A, LT_3_40GBPS},
	{179.5, 0, 8, HDMI_14, 0x0001, 0x2080, 0x020A, LT_3_40GBPS},
	{180, 0, 8, HDMI_14, 0x0001, 0x2080, 0x020A, LT_3_40GBPS},
	{180, 0, 10, HDMI_14, 0x1018, 0x3203, 0x0619, LT_3_40GBPS},
	{180, 0, 12, HDMI_14, 0x2008, 0x3141, 0x060F, LT_3_40GBPS},
	{180, 0, 16, HDMI_20, 0x3640, 0x3080, 0x020A, GT_3_40GBPS},
	{182.75, 0, 8, HDMI_14, 0x0001, 0x2080, 0x020A, LT_3_40GBPS},
	{185.625, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{185.625, 0, 10, HDMI_14, 0x1018, 0x3203, 0x0619, LT_3_40GBPS},
	{185.625, 0, 12, HDMI_14, 0x2008, 0x3141, 0x060F, LT_3_40GBPS},
	{185.625, 0, 16, HDMI_20, 0x3640, 0x3080, 0x020A, GT_3_40GBPS},
	{187, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{187.25, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{189, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{193.25, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{198, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{198, 0, 10, HDMI_14, 0x1018, 0x3203, 0x0619, LT_3_40GBPS},
	{198, 0, 12, HDMI_14, 0x2008, 0x3100, 0x020F, LT_3_40GBPS},
	{198, 0, 16, HDMI_20, 0x3640, 0x3080, 0x020A, GT_3_40GBPS},
	{202.5, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{204.75, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{208, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{214.75, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{216, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{216, 0, 10, HDMI_14, 0x1018, 0x3203, 0x0619, LT_3_40GBPS},
	{216, 0, 12, HDMI_14, 0x2008, 0x3100, 0x020F, LT_3_40GBPS},
	{216, 0, 16, HDMI_20, 0x3640, 0x3080, 0x020A, GT_3_40GBPS},
	{218.25, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{229.5, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{234, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{237.6, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{237.6, 0, 10, HDMI_14, 0x1018, 0x3182, 0x0219, LT_3_40GBPS},
	{237.6, 0, 12, HDMI_20, 0x2648, 0x3100, 0x020F, GT_3_40GBPS},
	{237.6, 0, 16, HDMI_20, 0x3640, 0x30C0, 0x000A, GT_3_40GBPS},
	{245.25, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{245.5, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{261, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{268.25, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{268.5, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{281.25, 0, 8, HDMI_14, 0x0000, 0x3040, 0x0605, LT_3_40GBPS},
	{288, 0, 8, HDMI_14, 0x0000, 0x3041, 0x0205, LT_3_40GBPS},
	{288, 0, 10, HDMI_20, 0x1658, 0x3182, 0x0219, GT_3_40GBPS},
	{288, 0, 12, HDMI_20, 0x2648, 0x3100, 0x020F, GT_3_40GBPS},
	{288, 0, 16, HDMI_20, 0x3640, 0x30C0, 0x000A, GT_3_40GBPS},
	{297, 0, 8, HDMI_14, 0x0000, 0x3041, 0x0205, LT_3_40GBPS},
	{297, 0, 10, HDMI_20, 0x1658, 0x3182, 0x0219, GT_3_40GBPS},
	{297, 0, 12, HDMI_20, 0x2648, 0x3100, 0x020F, GT_3_40GBPS},
	{297, 0, 16, HDMI_20, 0x3640, 0x30C0, 0x000A, GT_3_40GBPS},
	{317, 0, 8, HDMI_14, 0x0000, 0x3041, 0x0205, LT_3_40GBPS},
	{330, 0, 8, HDMI_14, 0x0000, 0x3041, 0x0205, LT_3_40GBPS},
	{330, 0, 10, HDMI_20, 0x1658, 0x3182, 0x0219, GT_3_40GBPS},
	{330, 0, 12, HDMI_20, 0x2648, 0x3100, 0x000F, GT_3_40GBPS},
	{333.25, 0, 8, HDMI_14, 0x0000, 0x3041, 0x0205, LT_3_40GBPS},
	{340, 0, 8, HDMI_14, 0x0000, 0x3041, 0x0205, LT_3_40GBPS},
	{348.5, 0, 8, HDMI_20, 0x0640, 0x3041, 0x0205, GT_3_40GBPS},
	{356.5, 0, 8, HDMI_20, 0x0640, 0x3041, 0x0205, GT_3_40GBPS},
	{360, 0, 8, HDMI_20, 0x0640, 0x3041, 0x0205, GT_3_40GBPS},
	{360, 0, 10, HDMI_20, 0x1658, 0x3182, 0x0219, GT_3_40GBPS},
	{360, 0, 12, HDMI_20, 0x2648, 0x3100, 0x000F, GT_3_40GBPS},
	{371.25, 0, 8, HDMI_20, 0x0640, 0x3041, 0x0205, GT_3_40GBPS},
	{371.25, 0, 10, HDMI_20, 0x1658, 0x3182, 0x0219, GT_3_40GBPS},
	{371.25, 0, 12, HDMI_20, 0x2648, 0x3100, 0x000F, GT_3_40GBPS},
	{380.5, 0, 8, HDMI_20, 0x0640, 0x3041, 0x0205, GT_3_40GBPS},
	{396, 0, 8, HDMI_20, 0x0640, 0x3041, 0x0205, GT_3_40GBPS},
	{396, 0, 10, HDMI_20, 0x1658, 0x31C0, 0x0019, GT_3_40GBPS},
	{396, 0, 12, HDMI_20, 0x2648, 0x3100, 0x000F, GT_3_40GBPS},
	{432, 0, 8, HDMI_20, 0x0640, 0x3041, 0x0205, GT_3_40GBPS},
	{432, 0, 10, HDMI_20, 0x1658, 0x31C0, 0x0019, GT_3_40GBPS},
	{475.2, 0, 8, HDMI_20, 0x0640, 0x3080, 0x0005, GT_3_40GBPS},
	{475.2, 0, 10, HDMI_20, 0x1658, 0x31C0, 0x0019, GT_3_40GBPS},
	{495, 0, 8, HDMI_20, 0x0640, 0x3080, 0x0005, GT_3_40GBPS},
	{505.25, 0, 8, HDMI_20, 0x0640, 0x3080, 0x0005, GT_3_40GBPS},
	{552.75, 0, 8, HDMI_20, 0x0640, 0x3080, 0x0005, GT_3_40GBPS},
	{594, 0, 8, HDMI_20, 0x0640, 0x3080, 0x0005, GT_3_40GBPS},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};


int phy316_configure_supported(hdmi_tx_dev_t *dev, double pClk, color_depth_t color,
		pixel_repetition_t pixel)
{
	int i   = 0;

	// Color resolution 0 is 8 bit color depth
	if (color == 0)
		color = COLOR_DEPTH_8;

	pClk = phy_get_freq(pClk);

	for (i = 0; phy316[i].clock != 0; i++ ){
		if(double_is_equal(pClk, phy316[i].clock) &&
				(color == phy316[i].color) &&
				(pixel == phy316[i].pixel)){
			return TRUE;
		}
	}

	return FALSE;
}

struct phy_config * phy316_get_configs(double pClk, color_depth_t color,
		pixel_repetition_t pixel)
{

	int i = 0;

	for (i = 0; phy316[i].clock != 0; i++ ){

		pClk = phy_get_freq(pClk);

		if(double_is_equal(pClk, phy316[i].clock) &&
				(color == phy316[i].color) &&
				(pixel == phy316[i].pixel)){
			return &(phy316[i]);
		}
	}

	return NULL;
}

int phy316_configure(hdmi_tx_dev_t *dev, double pClk, color_depth_t color, pixel_repetition_t pixel)
{
	int i   = 0;
	u32 phyRead = 0;
	u8 lock = 0;
	struct phy_config * config = NULL;
	u32 value = 0;

	// Color resolution 0 is 8 bit color depth
	if (color == 0)
		color = COLOR_DEPTH_8;

	config = phy316_get_configs(pClk, color, pixel);

	if (config == NULL) {
		LOGGER(SNPS_ERROR,"Configuration for clk %f color depth %d"
				  " pixel repetition %d not found", pClk, color, pixel);
		return -1;
	}

	// Set PHYTXCONFIG txphy_hdmi_i2c_jtagz=1(I2C)
	value = dev_read(dev, 0xb0020);
	value |= (1 << 9);
	dev_write(dev, 0xb0020, value);

	// TC Initial
	board_ZcalReset(1);

	mc_phy_reset(dev, 1);

	dev_write_mask(dev, PHY_CONF0, PHY_CONF0_TXPWRON_MASK, 0);
	dev_write_mask(dev, PHY_CONF0, PHY_CONF0_PDDQ_MASK, 1);
	dev_write_mask(dev, PHY_CONF0, PHY_CONF0_SVSRET_MASK, 1);

	// Start IO calibration
	board_ZcalReset(0);

	// Wait for zcal done
	do{
		snps_sleep(5);
	} while(!(dev_read(dev, 0xb0010) & 0x1) && (i++ < PHY_TIMEOUT));

	if (i>=PHY_TIMEOUT){
		LOGGER(SNPS_ERROR,"Timeout waiting for zcal done");
	}

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

	phy_write(dev, PLLDIVCTRL, config->pllgmpctrl);
	if(phy_read(dev, PLLDIVCTRL, &phyRead) || (phyRead != config->pllgmpctrl))
		LOGGER(SNPS_ERROR, "%s:PLLDIVCTRL Mismatch Write 0x%04x Read 0x%04x",
				__func__, config->pllgmpctrl , phyRead);

	phy_write(dev, TXTERM, config->txterm);
	if(phy_read(dev, TXTERM, &phyRead) || (phyRead != config->txterm))
		LOGGER(SNPS_ERROR, "%s:TXTERM Mismatch Write 0x%04x Read 0x%04x",
				__func__, config->txterm , phyRead);

	phy_write(dev, VLEVCTRL, config->vlevctrl);
	if(phy_read(dev, VLEVCTRL, &phyRead) || (phyRead != config->vlevctrl))
		LOGGER(SNPS_ERROR, "%s:VLEVCTRL Mismatch Write 0x%04x Read 0x%04x",
				__func__, config->vlevctrl , phyRead);

	phy_write(dev, CKSYMTXCTRL, config->cksymtxctrl);
	if(phy_read(dev, CKSYMTXCTRL, &phyRead) || (phyRead != config->cksymtxctrl))
		LOGGER(SNPS_ERROR, "%s:CKSYMTXCTRL Mismatch Write 0x%04x Read 0x%04x",
				__func__, config->cksymtxctrl , phyRead);

	dev_write_mask(dev, PHY_CONF0, PHY_CONF0_PDDQ_MASK, 0);
	dev_write_mask(dev, PHY_CONF0, PHY_CONF0_TXPWRON_MASK, 1);

	/* wait PHY_TIMEOUT no of cycles at most for the PLL lock signal to raise ~around 20us max */
	// Wait for TX ready
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
