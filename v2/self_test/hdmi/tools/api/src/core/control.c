// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "core/control.h"
#include "core/main_controller.h"
#include "core/irq.h"
#include "util/log.h"
#include "util/error.h"


int control_Initialize(hdmi_tx_dev_t *dev)
{
	mc_disable_all_clocks(dev);
	return TRUE;
}

int control_Standby(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	mc_disable_all_clocks(dev);
	return TRUE;
}


//int control_InterruptMute(hdmi_tx_dev_t *dev, u8 value)
//{
//	LOG_TRACE1(value);
//
//	if(value)
//		irq_mute(dev);
//	else
//		irq_unmute(dev);
//
//	return TRUE;
//}

//int control_InterruptCecClear(hdmi_tx_dev_t *dev, u8 value)
//{
//	LOG_TRACE1(value);
//	halInterrupt_CecClear(dev, value);
//	return TRUE;
//}
//
//u8 control_InterruptCecState(hdmi_tx_dev_t *dev)
//{
//	LOG_TRACE();
//	return halInterrupt_CecState(dev);
//}
//
//int control_InterruptEdidClear(hdmi_tx_dev_t *dev, u8 value)
//{
//	LOG_TRACE1(value);
//	halInterrupt_I2cDdcClear(dev, value);
//	return TRUE;
//}
//
//u8 control_InterruptEdidState(hdmi_tx_dev_t *dev)
//{
//	LOG_TRACE();
//	return halInterrupt_I2cDdcState(dev);
//}
//
//int control_InterruptI2cPhyClear(hdmi_tx_dev_t *dev, u8 value)
//{
//	LOG_TRACE();
//	halInterrupt_I2cPhyClear(dev, value);
//	return TRUE;
//}
//
//u8 control_InterruptI2cPhyState(hdmi_tx_dev_t *dev)
//{
//	LOG_TRACE();
//	return halInterrupt_I2cPhyState(dev);
//}
//
//int control_InterruptPhyClear(hdmi_tx_dev_t *dev, u8 value)
//{
//	LOG_TRACE1(value);
//	halInterrupt_PhyClear(dev, value);
//	return TRUE;
//}
//
//u8 control_InterruptPhyState(hdmi_tx_dev_t *dev)
//{
//	LOG_TRACE();
//	return halInterrupt_PhyState(dev);
//}
//
//u8 control_InterruptAudioDmaState(hdmi_tx_dev_t *dev)
//{
//	LOG_TRACE();
//	return halInterrupt_AudioDmaState(dev);
//}
//
//int control_InterruptAudioDmaClear(hdmi_tx_dev_t *dev, u8 value)
//{
//	LOG_TRACE1(value);
//	halInterrupt_AudioDmaClear(dev, value);
//	return TRUE;
//}

int control_InterruptClearAll(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
//	halInterrupt_AudioPacketsClear(dev, (u8) (0xFF));
//	halInterrupt_OtherPacketsClear(dev, (u8) (0xFF));
//	halInterrupt_PacketsOverflowClear(dev, (u8) (0xFF));
//	halInterrupt_AudioSamplerClear(dev, (u8) (0xFF));
//	halInterrupt_PhyClear(dev, (u8) (0xFF));
//	halInterrupt_I2cDdcClear(dev, (u8) (0xFF));
//	halInterrupt_CecClear(dev, (u8) (0xFF));
//	halInterrupt_VideoPacketizerClear(dev, (u8) (0xFF));
//	halInterrupt_I2cPhyClear(dev, (u8) (0xFF));
	irq_clear_all(dev);
	return TRUE;
}
