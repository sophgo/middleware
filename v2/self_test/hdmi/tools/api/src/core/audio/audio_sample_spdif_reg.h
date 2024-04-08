/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef INCLUDE_CORE_AUDIO_AUDIO_SPDIF_H_
#define INCLUDE_CORE_AUDIO_AUDIO_SPDIF_H_

/*****************************************************************************
 *                                                                           *
 *                        Audio Sample SPDIF Registers                       *
 *                                                                           *
 *****************************************************************************/

//Audio SPDIF Software FIFO Reset Control Register 0 This register allows the system processor to reset audio FIFOs upon underflow/overflow error detection
#define AUD_SPDIF0  0x0000CC00
#define AUD_SPDIF0_SPARE_MASK  0x0000007F //This field is a "spare" bit with no associated functionality
#define AUD_SPDIF0_SW_AUDIO_FIFO_RST_MASK  0x00000080 //Audio FIFOs software reset Writing 0b: no action taken Writing 1b: Resets all audio FIFOs Reading from this register always returns 0b

//Audio SPDIF NLPCM and Width Configuration Register 1 This register configures the SPDIF data width
#define AUD_SPDIF1  0x0000CC04
#define AUD_SPDIF1_SPDIF_WIDTH_MASK  0x0000001F //SPDIF input data width SPDIF_width[4:0] | Action 00000b-01111b | Not used 10000b | 16-bit data samples at input 10001b | 17-bit data samples at input 10010b | 18-bit data samples at input 10011b | 19-bit data samples at input 10100b | 20-bit data samples at input 10101b | 21-bit data samples at input 10110b | 22-bit data samples at input 10111b | 23-bit data samples at input 11000b | 24-bit data samples at input 11001b-11111b | Not Used
#define AUD_SPDIF1_SPARE_MASK  0x00000020 //This field is a "spare" bit with no associated functionality
#define AUD_SPDIF1_SPDIF_HBR_MODE_MASK  0x00000040 //When set to 1'b1, this bit field indicates that the input stream has a High Bit Rate (HBR) to be transmitted in HDMI HBR packets
#define AUD_SPDIF1_SETNLPCM_MASK  0x00000080 //Select Non-Linear (1b) / Linear (0b) PCM mode

//Audio SPDIF FIFO Empty/Full Mask Register
#define AUD_SPDIFINT  0x0000CC08
#define AUD_SPDIFINT_SPDIF_FIFO_FULL_MASK_MASK  0x00000004 //SPDIF FIFO full mask
#define AUD_SPDIFINT_SPDIF_FIFO_EMPTY_MASK_MASK  0x00000008 //SPDIF FIFO empty mask

//Audio SPDIF Mask Interrupt Register 1 This register masks interrupts present in the SPDIF module
#define AUD_SPDIFINT1  0x0000CC0C
#define AUD_SPDIFINT1_FIFO_OVERRUN_MASK_MASK  0x00000010 //FIFO overrun mask

//Audio SPDIF Enable Configuration Register 2
#define AUD_SPDIF2  0x0000CC10
#define AUD_SPDIF2_IN_EN_MASK 0x0000000F

#endif /* INCLUDE_CORE_AUDIO_AUDIO_SPDIF_H_ */
