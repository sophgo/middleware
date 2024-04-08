/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef SRC_CORE_AUDIO_AUDIO_SAMPLE_REG_H_
#define SRC_CORE_AUDIO_AUDIO_SAMPLE_REG_H_

/*****************************************************************************
 *                                                                           *
 *                           Audio Sample Registers                          *
 *                                                                           *
 *****************************************************************************/

//Audio I2S Software FIFO Reset, Select, and Enable Control Register 0 This register configures the I2S input enable that indicates which input I2S channels have valid data
#define AUD_CONF0  0x0000C400
#define AUD_CONF0_I2S_IN_EN_MASK  0x0000000F //Action I2S_in_en[0] - I2Sdata[0] enable I2S_in_en[1] - I2Sdata[1] enable I2S_in_en[2] - I2Sdata[2] enable I2S_in_en[3] - I2Sdata[3] enable
#define AUD_CONF0_SPARE_1_MASK  0x00000010 //This field is a "spare" bit with no associated functionality
#define AUD_CONF0_I2S_SELECT_MASK  0x00000020 //1b: Selects I2S Audio Interface 0b: Selects the second (SPDIF/GPA) interface, in configurations with more that one audio interface (DOUBLE/GDOUBLE)
#define AUD_CONF0_SPARE_2_MASK  0x00000040 //This field is a "spare" bit with no associated functionality
#define AUD_CONF0_SW_AUDIO_FIFO_RST_MASK  0x00000080 //Audio FIFOs software reset - Writing 0b: no action taken - Writing 1b: Resets all audio FIFOs Reading from this register always returns 0b

//Audio I2S Width and Mode Configuration Register 1 This register configures the I2S mode and data width of the input data
#define AUD_CONF1  0x0000C404
#define AUD_CONF1_I2S_WIDTH_MASK  0x0000001F //I2S input data width I2S_width[4:0] | Action 00000b-01111b | Not used 10000b | 16 bit data samples at input 10001b | 17 bit data samples at input 10010b | 18 bit data samples at input 10011b | 19 bit data samples at input 10100b | 20 bit data samples at input 10101b | 21 bit data samples at input 10110b | 22 bit data samples at input 10111b | 23 bit data samples at input 11000b | 24 bit data samples at input 11001b-11111b | Not Used
//I2S input data mode I2S_mode[4:0] | Action
//	000b | Standard I2S mode
//	001b | Right-justified I2S mode
//	010b | Left-justified I2S mode
//	011b | Burst 1 mode
//	100b | Burst 2 mode
#define AUD_CONF1_I2S_MODE_MASK  0x000000E0
//I2S FIFO status and interrupts
#define AUD_INT  0x0000C408
#define AUD_INT_FIFO_FULL_MASK_MASK  0x00000004 //FIFO full mask
#define AUD_INT_FIFO_EMPTY_MASK_MASK  0x00000008 //FIFO empty mask

//Audio I2S NLPCM and HBR configuration Register 2 This register configures the I2S Audio Data mapping
#define AUD_CONF2  0x0000C40C
#define AUD_CONF2_HBR_MASK  0x00000001 //I2S HBR Mode Enable
#define AUD_CONF2_NLPCM_MASK  0x00000002 //I2S NLPCM Mode Enable

//I2S Mask Interrupt Register This register masks the interrupts present in the I2S module
#define AUD_INT1  0x0000C410
#define AUD_INT1_FIFO_OVERRUN_MASK_MASK  0x00000010 //FIFO overrun mask

#endif /* SRC_CORE_AUDIO_AUDIO_SAMPLE_REG_H_ */
