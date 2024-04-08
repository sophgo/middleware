/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef SRC_CORE_COLOR_SPACE_COLOR_SPACE_REG_H_
#define SRC_CORE_COLOR_SPACE_COLOR_SPACE_REG_H_

/*****************************************************************************
 *                                                                           *
 *                      Color Space Converter Registers                      *
 *                                                                           *
 *****************************************************************************/

//Color Space Converter Interpolation and Decimation Configuration Register
#define CSC_CFG  0x00010400
#define CSC_CFG_DECMODE_MASK  0x00000003 //Chroma decimation configuration: decmode[1:0] | Chroma Decimation 00 | decimation disabled 01 | Hd (z) =1 10 | Hd(z)=1/ 4 + 1/2z^(-1 )+1/4 z^(-2) 11 | Hd(z)x2^(11)= -5+12z^(-2) - 22z^(-4)+39z^(-8) +109z^(-10) -204z^(-12)+648z^(-14) + 1024z^(-15) +648z^(-16) -204z^(-18) +109z^(-20)- 65z^(-22) +39z^(-24) -22z^(-26) +12z^(-28)-5z^(-30)
#define CSC_CFG_SPARE_1_MASK  0x0000000C //This is a spare register with no associated functionality
#define CSC_CFG_INTMODE_MASK  0x00000030 //Chroma interpolation configuration: intmode[1:0] | Chroma Interpolation 00 | interpolation disabled 01 | Hu (z) =1 + z^(-1) 10 | Hu(z)=1/ 2 + z^(-11)+1/2 z^(-2) 11 | interpolation disabled
#define CSC_CFG_SPARE_2_MASK  0x00000040 //This is a spare register with no associated functionality
#define CSC_CFG_CSC_LIMIT_MASK  0x00000080 //When set (1'b1), the range limitation values defined in registers csc_mat_uplim and csc_mat_dnlim are applied to the output of the Color Space Conversion matrix

//Color Space Converter Scale and Deep Color Configuration Register
#define CSC_SCALE  0x00010404
#define CSC_SCALE_CSCSCALE_MASK  0x00000003 //Defines the cscscale[1:0] scale factor to apply to all coefficients in Color Space Conversion
#define CSC_SCALE_SPARE_MASK  0x0000000C //The is a spare register with no associated functionality
#define CSC_SCALE_CSC_COLOR_DEPTH_MASK  0x000000F0 //Color space converter color depth configuration: csc_colordepth[3:0] | Action 0000 | 24 bit per pixel video (8 bit per component)

//Color Space Converter Matrix A1 Coefficient Register MSB Notes: - The coefficients used in the CSC matrix use only 15 bits for the internal computations
#define CSC_COEF_A1_MSB  0x00010408
#define CSC_COEF_A1_MSB_CSC_COEF_A1_MSB_MASK  0x000000FF //Color Space Converter Matrix A1 Coefficient Register MSB

//Color Space Converter Matrix A1 Coefficient Register LSB Notes: - The coefficients used in the CSC matrix use only 15 bits for the internal computations
#define CSC_COEF_A1_LSB  0x0001040C
#define CSC_COEF_A1_LSB_CSC_COEF_A1_LSB_MASK  0x000000FF //Color Space Converter Matrix A1 Coefficient Register LSB

//Color Space Converter Matrix A2 Coefficient Register MSB Color Space Conversion A2 coefficient
#define CSC_COEF_A2_MSB  0x00010410
#define CSC_COEF_A2_MSB_CSC_COEF_A2_MSB_MASK  0x000000FF //Color Space Converter Matrix A2 Coefficient Register MSB

//Color Space Converter Matrix A2 Coefficient Register LSB Color Space Conversion A2 coefficient
#define CSC_COEF_A2_LSB  0x00010414
#define CSC_COEF_A2_LSB_CSC_COEF_A2_LSB_MASK  0x000000FF //Color Space Converter Matrix A2 Coefficient Register LSB

//Color Space Converter Matrix A3 Coefficient Register MSB Color Space Conversion A3 coefficient
#define CSC_COEF_A3_MSB  0x00010418
#define CSC_COEF_A3_MSB_CSC_COEF_A3_MSB_MASK  0x000000FF //Color Space Converter Matrix A3 Coefficient Register MSB

//Color Space Converter Matrix A3 Coefficient Register LSB Color Space Conversion A3 coefficient
#define CSC_COEF_A3_LSB  0x0001041C
#define CSC_COEF_A3_LSB_CSC_COEF_A3_LSB_MASK  0x000000FF //Color Space Converter Matrix A3 Coefficient Register LSB

//Color Space Converter Matrix A4 Coefficient Register MSB Color Space Conversion A4 coefficient
#define CSC_COEF_A4_MSB  0x00010420
#define CSC_COEF_A4_MSB_CSC_COEF_A4_MSB_MASK  0x000000FF //Color Space Converter Matrix A4 Coefficient Register MSB

//Color Space Converter Matrix A4 Coefficient Register LSB Color Space Conversion A4 coefficient
#define CSC_COEF_A4_LSB  0x00010424
#define CSC_COEF_A4_LSB_CSC_COEF_A4_LSB_MASK  0x000000FF //Color Space Converter Matrix A4 Coefficient Register LSB

//Color Space Converter Matrix B1 Coefficient Register MSB Color Space Conversion B1 coefficient
#define CSC_COEF_B1_MSB  0x00010428
#define CSC_COEF_B1_MSB_CSC_COEF_B1_MSB_MASK  0x000000FF //Color Space Converter Matrix B1 Coefficient Register MSB

//Color Space Converter Matrix B1 Coefficient Register LSB Color Space Conversion B1 coefficient
#define CSC_COEF_B1_LSB  0x0001042C
#define CSC_COEF_B1_LSB_CSC_COEF_B1_LSB_MASK  0x000000FF //Color Space Converter Matrix B1 Coefficient Register LSB

//Color Space Converter Matrix B2 Coefficient Register MSB Color Space Conversion B2 coefficient
#define CSC_COEF_B2_MSB  0x00010430
#define CSC_COEF_B2_MSB_CSC_COEF_B2_MSB_MASK  0x000000FF //Color Space Converter Matrix B2 Coefficient Register MSB

//Color Space Converter Matrix B2 Coefficient Register LSB Color Space Conversion B2 coefficient
#define CSC_COEF_B2_LSB  0x00010434
#define CSC_COEF_B2_LSB_CSC_COEF_B2_LSB_MASK  0x000000FF //Color Space Converter Matrix B2 Coefficient Register LSB

//Color Space Converter Matrix B3 Coefficient Register MSB Color Space Conversion B3 coefficient
#define CSC_COEF_B3_MSB  0x00010438
#define CSC_COEF_B3_MSB_CSC_COEF_B3_MSB_MASK  0x000000FF //Color Space Converter Matrix B3 Coefficient Register MSB

//Color Space Converter Matrix B3 Coefficient Register LSB Color Space Conversion B3 coefficient
#define CSC_COEF_B3_LSB  0x0001043C
#define CSC_COEF_B3_LSB_CSC_COEF_B3_LSB_MASK  0x000000FF //Color Space Converter Matrix B3 Coefficient Register LSB

//Color Space Converter Matrix B4 Coefficient Register MSB Color Space Conversion B4 coefficient
#define CSC_COEF_B4_MSB  0x00010440
#define CSC_COEF_B4_MSB_CSC_COEF_B4_MSB_MASK  0x000000FF //Color Space Converter Matrix B4 Coefficient Register MSB

//Color Space Converter Matrix B4 Coefficient Register LSB Color Space Conversion B4 coefficient
#define CSC_COEF_B4_LSB  0x00010444
#define CSC_COEF_B4_LSB_CSC_COEF_B4_LSB_MASK  0x000000FF //Color Space Converter Matrix B4 Coefficient Register LSB

//Color Space Converter Matrix C1 Coefficient Register MSB Color Space Conversion C1 coefficient
#define CSC_COEF_C1_MSB  0x00010448
#define CSC_COEF_C1_MSB_CSC_COEF_C1_MSB_MASK  0x000000FF //Color Space Converter Matrix C1 Coefficient Register MSB

//Color Space Converter Matrix C1 Coefficient Register LSB Color Space Conversion C1 coefficient
#define CSC_COEF_C1_LSB  0x0001044C
#define CSC_COEF_C1_LSB_CSC_COEF_C1_LSB_MASK  0x000000FF //Color Space Converter Matrix C1 Coefficient Register LSB

//Color Space Converter Matrix C2 Coefficient Register MSB Color Space Conversion C2 coefficient
#define CSC_COEF_C2_MSB  0x00010450
#define CSC_COEF_C2_MSB_CSC_COEF_C2_MSB_MASK  0x000000FF //Color Space Converter Matrix C2 Coefficient Register MSB

//Color Space Converter Matrix C2 Coefficient Register LSB Color Space Conversion C2 coefficient
#define CSC_COEF_C2_LSB  0x00010454
#define CSC_COEF_C2_LSB_CSC_COEF_C2_LSB_MASK  0x000000FF //Color Space Converter Matrix C2 Coefficient Register LSB

//Color Space Converter Matrix C3 Coefficient Register MSB Color Space Conversion C3 coefficient
#define CSC_COEF_C3_MSB  0x00010458
#define CSC_COEF_C3_MSB_CSC_COEF_C3_MSB_MASK  0x000000FF //Color Space Converter Matrix C3 Coefficient Register MSB

//Color Space Converter Matrix C3 Coefficient Register LSB Color Space Conversion C3 coefficient
#define CSC_COEF_C3_LSB  0x0001045C
#define CSC_COEF_C3_LSB_CSC_COEF_C3_LSB_MASK  0x000000FF //Color Space Converter Matrix C3 Coefficient Register LSB

//Color Space Converter Matrix C4 Coefficient Register MSB Color Space Conversion C4 coefficient
#define CSC_COEF_C4_MSB  0x00010460
#define CSC_COEF_C4_MSB_CSC_COEF_C4_MSB_MASK  0x000000FF //Color Space Converter Matrix C4 Coefficient Register MSB

//Color Space Converter Matrix C4 Coefficient Register LSB Color Space Conversion C4 coefficient
#define CSC_COEF_C4_LSB  0x00010464
#define CSC_COEF_C4_LSB_CSC_COEF_C4_LSB_MASK  0x000000FF //Color Space Converter Matrix C4 Coefficient Register LSB

//Color Space Converter Matrix Output Up Limit Register MSB For more details, refer to the HDMI 1
#define CSC_LIMIT_UP_MSB  0x00010468
#define CSC_LIMIT_UP_MSB_CSC_LIMIT_UP_MSB_MASK  0x000000FF //Color Space Converter Matrix Output Upper Limit Register MSB

//Color Space Converter Matrix output Up Limit Register LSB For more details, refer to the HDMI 1
#define CSC_LIMIT_UP_LSB  0x0001046C
#define CSC_LIMIT_UP_LSB_CSC_LIMIT_UP_LSB_MASK  0x000000FF //Color Space Converter Matrix Output Upper Limit Register LSB

//Color Space Converter Matrix output Down Limit Register MSB For more details, refer to the HDMI 1
#define CSC_LIMIT_DN_MSB  0x00010470
#define CSC_LIMIT_DN_MSB_CSC_LIMIT_DN_MSB_MASK  0x000000FF //Color Space Converter Matrix output Down Limit Register MSB

//Color Space Converter Matrix output Down Limit Register LSB For more details, refer to the HDMI 1
#define CSC_LIMIT_DN_LSB  0x00010474
#define CSC_LIMIT_DN_LSB_CSC_LIMIT_DN_LSB_MASK  0x000000FF //Color Space Converter Matrix Output Down Limit Register LSB

#endif /* SRC_CORE_COLOR_SPACE_COLOR_SPACE_REG_H_ */
