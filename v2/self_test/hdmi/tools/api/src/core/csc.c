// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "color_space/color_space_reg.h"
#include "core/csc.h"
#include "bsp/access.h"
#include "util/log.h"


void csc_Interpolation(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	/* 2-bit width */
	dev_write_mask(dev, CSC_CFG, CSC_CFG_INTMODE_MASK, value);
}

void csc_Decimation(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	/* 2-bit width */
	dev_write_mask(dev, CSC_CFG, CSC_CFG_DECMODE_MASK, value);
}

void csc_ColorDepth(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	/* 4-bit width */
	dev_write_mask(dev, CSC_SCALE, CSC_SCALE_CSC_COLOR_DEPTH_MASK, value);
}

void csc_ScaleFactor(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	/* 2-bit width */
	dev_write_mask(dev, CSC_SCALE, CSC_SCALE_CSCSCALE_MASK, value);
}

void csc_CoefficientA1(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	dev_write(dev, CSC_COEF_A1_LSB, (u8)(value));
	dev_write_mask(dev, CSC_COEF_A1_MSB, CSC_COEF_A1_MSB_CSC_COEF_A1_MSB_MASK, (u8)(value >> 8));
}

void csc_CoefficientA2(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	dev_write(dev, CSC_COEF_A2_LSB, (u8)(value));
	dev_write_mask(dev, CSC_COEF_A2_MSB, CSC_COEF_A2_MSB_CSC_COEF_A2_MSB_MASK, (u8)(value >> 8));
}

void csc_CoefficientA3(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	dev_write(dev, CSC_COEF_A3_LSB, (u8)(value));
	dev_write_mask(dev, CSC_COEF_A3_MSB, CSC_COEF_A3_MSB_CSC_COEF_A3_MSB_MASK, (u8)(value >> 8));
}

void csc_CoefficientA4(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	dev_write(dev, CSC_COEF_A4_LSB, (u8)(value));
	dev_write_mask(dev, CSC_COEF_A4_MSB, CSC_COEF_A4_MSB_CSC_COEF_A4_MSB_MASK, (u8)(value >> 8));
}

void csc_CoefficientB1(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	dev_write(dev, CSC_COEF_B1_LSB, (u8)(value));
	dev_write_mask(dev, CSC_COEF_B1_MSB, CSC_COEF_B1_MSB_CSC_COEF_B1_MSB_MASK, (u8)(value >> 8));
}

void csc_CoefficientB2(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	dev_write(dev, CSC_COEF_B2_LSB, (u8)(value));
	dev_write_mask(dev, CSC_COEF_B2_MSB, CSC_COEF_B2_MSB_CSC_COEF_B2_MSB_MASK, (u8)(value >> 8));
}

void csc_CoefficientB3(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	dev_write(dev, CSC_COEF_B3_LSB, (u8)(value));
	dev_write_mask(dev, CSC_COEF_B3_MSB, CSC_COEF_B3_MSB_CSC_COEF_B3_MSB_MASK, (u8)(value >> 8));
}

void csc_CoefficientB4(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	dev_write(dev, CSC_COEF_B4_LSB, (u8)(value));
	dev_write_mask(dev, CSC_COEF_B4_MSB, CSC_COEF_B4_MSB_CSC_COEF_B4_MSB_MASK, (u8)(value >> 8));
}

void csc_CoefficientC1(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	dev_write(dev, CSC_COEF_C1_LSB, (u8) (value));
	dev_write_mask(dev, CSC_COEF_C1_MSB, CSC_COEF_C1_MSB_CSC_COEF_C1_MSB_MASK, (u8)(value >> 8));
}

void csc_CoefficientC2(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	dev_write(dev, CSC_COEF_C2_LSB, (u8) (value));
	dev_write_mask(dev, CSC_COEF_C2_MSB, CSC_COEF_C2_MSB_CSC_COEF_C2_MSB_MASK, (u8)(value >> 8));
}

void csc_CoefficientC3(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	dev_write(dev, CSC_COEF_C3_LSB, (u8) (value));
	dev_write_mask(dev, CSC_COEF_C3_MSB, CSC_COEF_C3_MSB_CSC_COEF_C3_MSB_MASK, (u8)(value >> 8));
}

void csc_CoefficientC4(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	dev_write(dev, CSC_COEF_C4_LSB, (u8) (value));
	dev_write_mask(dev, CSC_COEF_C4_MSB, CSC_COEF_C4_MSB_CSC_COEF_C4_MSB_MASK, (u8)(value >> 8));
}

void csc_config(hdmi_tx_dev_t *dev, videoParams_t * video,
		unsigned interpolation, unsigned decimation, unsigned color_depth)
{
	csc_Interpolation(dev, interpolation);
	csc_Decimation(dev, decimation);
	csc_CoefficientA1(dev, video->mCscA[0]);
	csc_CoefficientA2(dev, video->mCscA[1]);
	csc_CoefficientA3(dev, video->mCscA[2]);
	csc_CoefficientA4(dev, video->mCscA[3]);
	csc_CoefficientB1(dev, video->mCscB[0]);
	csc_CoefficientB2(dev, video->mCscB[1]);
	csc_CoefficientB3(dev, video->mCscB[2]);
	csc_CoefficientB4(dev, video->mCscB[3]);
	csc_CoefficientC1(dev, video->mCscC[0]);
	csc_CoefficientC2(dev, video->mCscC[1]);
	csc_CoefficientC3(dev, video->mCscC[2]);
	csc_CoefficientC4(dev, video->mCscC[3]);
	csc_ScaleFactor(dev, video->mCscScale);
	csc_ColorDepth(dev, color_depth);
}
