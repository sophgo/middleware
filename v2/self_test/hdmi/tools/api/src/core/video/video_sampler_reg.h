/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef SRC_CORE_VIDEO_VIDEO_SAMPLER_REG_H_
#define SRC_CORE_VIDEO_VIDEO_SAMPLER_REG_H_

/*****************************************************************************
 *                                                                           *
 *                          Video Sampler Registers                          *
 *                                                                           *
 *****************************************************************************/

//Video Input Mapping and Internal Data Enable Configuration Register
#define TX_INVID0  0x00000800
#define TX_INVID0_VIDEO_MAPPING_MASK  0x0000001F //Video Input mapping (color space/color depth): 0x01: RGB 4:4:4/8 bits 0x03: RGB 4:4:4/10 bits 0x05: RGB 4:4:4/12 bits 0x07: RGB 4:4:4/16 bits 0x09: YCbCr 4:4:4 or 4:2:0/8 bits 0x0B: YCbCr 4:4:4 or 4:2:0/10 bits 0x0D: YCbCr 4:4:4 or 4:2:0/12 bits 0x0F: YCbCr 4:4:4 or 4:2:0/16 bits 0x16: YCbCr 4:2:2/8 bits 0x14: YCbCr 4:2:2/10 bits 0x12: YCbCr 4:2:2/12 bits 0x17: YCbCr 4:4:4 (IPI)/8 bits 0x18: YCbCr 4:4:4 (IPI)/10 bits 0x19: YCbCr 4:4:4 (IPI)/12 bits 0x1A: YCbCr 4:4:4 (IPI)/16 bits 0x1B: YCbCr 4:2:2 (IPI)/12 bits 0x1C: YCbCr 4:2:0 (IPI)/8 bits 0x1D: YCbCr 4:2:0 (IPI)/10 bits 0x1E: YCbCr 4:2:0 (IPI)/12 bits 0x1F: YCbCr 4:2:0 (IPI)/16 bits
#define TX_INVID0_INTERNAL_DE_GENERATOR_MASK  0x00000080 //Internal data enable (DE) generator enable

//Video Input Stuffing Enable Register
#define TX_INSTUFFING  0x00000804
#define TX_INSTUFFING_GYDATA_STUFFING_MASK  0x00000001 //- 0b: When the dataen signal is low, the value in the gydata[15:0] output is the one sampled from the corresponding input data
#define TX_INSTUFFING_RCRDATA_STUFFING_MASK  0x00000002 //- 0b: When the dataen signal is low, the value in the rcrdata[15:0] output is the one sampled from the corresponding input data
#define TX_INSTUFFING_BCBDATA_STUFFING_MASK  0x00000004 //- 0b: When the dataen signal is low, the value in the bcbdata[15:0] output is the one sampled from the corresponding input data

//Video Input gy Data Channel Stuffing Register 0
#define TX_GYDATA0  0x00000808
#define TX_GYDATA0_GYDATA_MASK  0x000000FF //This register defines the value of gydata[7:0] when TX_INSTUFFING[0] (gydata_stuffing) is set to 1b

//Video Input gy Data Channel Stuffing Register 1
#define TX_GYDATA1  0x0000080C
#define TX_GYDATA1_GYDATA_MASK  0x000000FF //This register defines the value of gydata[15:8] when TX_INSTUFFING[0] (gydata_stuffing) is set to 1b

//Video Input rcr Data Channel Stuffing Register 0
#define TX_RCRDATA0  0x00000810
#define TX_RCRDATA0_RCRDATA_MASK  0x000000FF //This register defines the value of rcrydata[7:0] when TX_INSTUFFING[1] (rcrdata_stuffing) is set to 1b

//Video Input rcr Data Channel Stuffing Register 1
#define TX_RCRDATA1  0x00000814
#define TX_RCRDATA1_RCRDATA_MASK  0x000000FF //This register defines the value of rcrydata[15:8] when TX_INSTUFFING[1] (rcrdata_stuffing) is set to 1b

//Video Input bcb Data Channel Stuffing Register 0
#define TX_BCBDATA0  0x00000818
#define TX_BCBDATA0_BCBDATA_MASK  0x000000FF //This register defines the value of bcbdata[7:0] when TX_INSTUFFING[2] (bcbdata_stuffing) is set to 1b

//Video Input bcb Data Channel Stuffing Register 1
#define TX_BCBDATA1  0x0000081C
#define TX_BCBDATA1_BCBDATA_MASK  0x000000FF //This register defines the value of bcbdata[15:8] when TX_INSTUFFING[2] (bcbdata_stuffing) is set to 1b

#endif /* SRC_CORE_VIDEO_VIDEO_SAMPLER_REG_H_ */
