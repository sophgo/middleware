/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef SRC_CORE_VIDEO_VIDEO_PACKETIZER_REG_H_
#define SRC_CORE_VIDEO_VIDEO_PACKETIZER_REG_H_

/*****************************************************************************
 *                                                                           *
 *                         Video Packetizer Registers                        *
 *                                                                           *
 *****************************************************************************/

//Video Packetizer Packing Phase Status Register
#define VP_STATUS  0x00002000
#define VP_STATUS_PACKING_PHASE_MASK  0x0000000F //Read only register that holds the "packing phase" output of the Video Packetizer block

//Video Packetizer Pixel Repetition and Color Depth Register
#define VP_PR_CD  0x00002004
#define VP_PR_CD_DESIRED_PR_FACTOR_MASK  0x0000000F //Desired pixel repetition factor configuration
#define VP_PR_CD_COLOR_DEPTH_MASK  0x000000F0 //The Color depth configuration is described as the following, with the action stated corresponding to color_depth[3:0]: - 0000b: 24 bits per pixel video (8 bits per component)

//Video Packetizer Stuffing and Default Packing Phase Register
#define VP_STUFF  0x00002008
#define VP_STUFF_PR_STUFFING_MASK  0x00000001 //Pixel repeater stuffing control
#define VP_STUFF_PP_STUFFING_MASK  0x00000002 //Pixel packing stuffing control
#define VP_STUFF_YCC422_STUFFING_MASK  0x00000004 //YCC 422 remap stuffing control
#define VP_STUFF_ICX_GOTO_P0_ST_MASK  0x00000008 //Reserved
#define VP_STUFF_IFIX_PP_TO_LAST_MASK  0x00000010 //Reserved
#define VP_STUFF_IDEFAULT_PHASE_MASK  0x00000020 //Controls the default phase packing machine used according to HDMI 1

//Video Packetizer YCC422 Remapping Register
#define VP_REMAP  0x0000200C
#define VP_REMAP_YCC422_SIZE_MASK  0x00000003 //YCC 422 remap input video size ycc422_size[1:0] 00b: YCC 422 16-bit input video (8 bits per component) 01b: YCC 422 20-bit input video (10 bits per component) 10b: YCC 422 24-bit input video (12 bits per component) 11b: Reserved

//Video Packetizer Output, Bypass and Enable Configuration Register
#define VP_CONF  0x00002010
#define VP_CONF_OUTPUT_SELECTOR_MASK  0x00000003 //Video Packetizer output selection output_selector[1:0] 00b: Data from pixel packing block 01b: Data from YCC 422 remap block 10b: Data from 8-bit bypass block 11b: Data from 8-bit bypass block
#define VP_CONF_BYPASS_SELECT_MASK  0x00000004 //bypass_select 0b: Data from pixel repeater block 1b: Data from input of Video Packetizer block
#define VP_CONF_YCC422_EN_MASK  0x00000008 //YCC 422 select enable
#define VP_CONF_PR_EN_MASK  0x00000010 //Pixel repeater enable
#define VP_CONF_PP_EN_MASK  0x00000020 //Pixel packing enable
#define VP_CONF_BYPASS_EN_MASK  0x00000040 //Bypass enable

//Video Packetizer Interrupt Mask Register
#define VP_MASK  0x0000201C
#define VP_MASK_OINTEMPTYBYP_MASK  0x00000001 //Mask bit for Video Packetizer 8-bit bypass FIFO empty
#define VP_MASK_OINTFULLBYP_MASK  0x00000002 //Mask bit for Video Packetizer 8-bit bypass FIFO full
#define VP_MASK_OINTEMPTYREMAP_MASK  0x00000004 //Mask bit for Video Packetizer pixel YCC 422 re-mapper FIFO empty
#define VP_MASK_OINTFULLREMAP_MASK  0x00000008 //Mask bit for Video Packetizer pixel YCC 422 re-mapper FIFO full
#define VP_MASK_OINTEMPTYPP_MASK  0x00000010 //Mask bit for Video Packetizer pixel packing FIFO empty
#define VP_MASK_OINTFULLPP_MASK  0x00000020 //Mask bit for Video Packetizer pixel packing FIFO full
#define VP_MASK_OINTEMPTYREPET_MASK  0x00000040 //Mask bit for Video Packetizer pixel repeater FIFO empty
#define VP_MASK_OINTFULLREPET_MASK  0x00000080 //Mask bit for Video Packetizer pixel repeater FIFO full

#endif /* SRC_CORE_VIDEO_VIDEO_PACKETIZER_REG_H_ */
