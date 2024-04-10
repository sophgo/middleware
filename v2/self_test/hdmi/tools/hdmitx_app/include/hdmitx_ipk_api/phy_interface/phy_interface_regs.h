/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HDMI_TX_PHY_IF_DEFINES_H_
#define HDMI_TX_PHY_IF_DEFINES_H_
/*****************************************************************************
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *****************************************************************************/

//Output Delays Cells Configuration Register
#define ODLYCFG  0x00000000
#define ODLYCFG_DLY_VALUE_SET_MASK  0x00000001 //Output Delay Cells Value Set signal This register triggers the configuration of the Odelay Cell selected by ODLYSEL register with the value present in ODLYCNTINVAL register The delay value is only set on the posedge of this register
#define ODLYCFG_DLY_SET_DATAN_CLOCK_MASK  0x00000002 //Output Delay Cells Value Set Data Clock selector 0: Set signal is only asserted in odelay data primitives 1: Set signal is only asserted in odelay clock primitives

//Output Delay Cells Select Register
#define ODLYSEL  0x00000004
#define ODLYSEL_DLY_SELECT_INPUT_MASK  0x000000FF //Output Delay Cells Select Register This register selects the odelay cell to be configured, in decimal mode, i

//Output Delay Cells Value In Register
#define ODLYCNTINVAL  0x00000008
#define ODLYCNTINVAL_DLY_COUNTER_IN_VALUE_MASK  0x0000001F //Output Delay Cells Value In Register Register used to configure the delay value of na odelay cell

//Output Delay Cells Value Out Register
#define ODLYCNTOUTVAL  0x0000000C
#define ODLYCNTOUTVAL_DLY_COUNTER_OUT_VALUE_MASK  0x0000001F //Output Delay Cells Value In Register This register return the actual delay value of the Idelay Cell selected by ODLYSEL register present at the access moment

//Tx PHY Digital IOs calibration done signal
#define PHYTXZCALDONE  0x00000010
#define PHYTXZCALDONE_TXPHY_ZCAL_DONE_MASK  0x00000001 //Tx PHY Digital IOs calibration done signal

//Tx PHY Configuration Signals
#define PHYTXCONFIG  0x00000020
#define PHYTXCONFIG_TXPHY_WIDE_XFACE_MASK  0x00000003 //Selects data input interface bus width of the Tx PHY
#define PHYTXCONFIG_TXPHY_GLUE_SLV_ADDR_MASK  0x0000000C //Tx PHY Glue Logic I2C device slave address
#define PHYTXCONFIG_TXPHY_HDMI_SLV_ADDR_MASK  0x00000030 //Tx PHY HDMI-MHL TX PHY I2C device slave address
#define PHYTXCONFIG_TXPHY_PSEL_MASK  0x00000040 //USB/MHL SWITCH
#define PHYTXCONFIG_TXPHY_SEL_MASK  0x00000180 //USB/MHL SWITCH

//Tx PHY BIST Test Interface
#define PHYTXBIST1  0x00000040
#define PHYTXBIST1_TXPHY_BISTEN_MASK  0x00000001 //Tx PHY Control enable signal for BIST Test

//Tx PHY BIST Test Interface
#define PHYTXBIST2  0x00000044
#define PHYTXBIST2_TXPHY_BISTDONE_MASK  0x00000001 //Tx PHY Indication of BIST Test completion
#define PHYTXBIST2_TXPHY_BISTOK_MASK  0x00000002 //Tx PHY Indication of BIST Test pass

//Tx PHY Scan Mode Interface
#define PHYTXSCAN1  0x00000050
#define PHYTXSCAN1_TXPHY_SCANCLK_MASK  0x00000001 //Tx PHY
#define PHYTXSCAN1_TXPHY_SCANRST_MASK  0x00000002 //Tx PHY
#define PHYTXSCAN1_TXPHY_SCANEN_MASK  0x00000004 //Tx PHY
#define PHYTXSCAN1_TXPHY_SCANMODE_MASK  0x00000008 //Tx PHY
#define PHYTXSCAN1_TXPHY_SCANIN_MASK  0x00000010 //Tx PHY

//Tx PHY Scan Mode Interface
#define PHYTXSCAN2  0x00000054
#define PHYTXSCAN2_TXPHY_SCANOUT_MASK  0x00000001 //Tx PHY Scan out

//Tx PHY Digital Test Interface
#define PHYTXDTB  0x00000060
#define PHYTXDTB_TXPHY_HDMI_DTB_MASK  0x00000003 //Tx PHY HDMI-MHL TX PHY Digital Test Bus


#endif /* HDMI_TX_PHY_IF_DEFINES_H_ */
