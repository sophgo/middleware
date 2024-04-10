/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef CEC_H
#define CEC_H

#include "../hdmitx_dev.h"

/** Broadcast logical address */
#define BCST_ADDR   (0x0FUL)
/** Control register bitfield */
#define    FRAME_TYP1   (0x1 << 2)
/** Control register bitfield */
#define    FRAME_TYP0   (0x1 << 1)

/******************************************************************************
 * define
 *****************************************************************************/
int cec_msgRx(hdmi_tx_dev_t * dev, char *buf, unsigned size,int timeout);
int cec_msgTx(hdmi_tx_dev_t * dev, const char *buf, unsigned size, unsigned retry);
int cec_Init(hdmi_tx_dev_t * dev);
int cec_Disable(hdmi_tx_dev_t * dev, int wakeup);
int cec_CfgLogicAddr(hdmi_tx_dev_t * dev, unsigned addr, int enable);
int cec_CfgStandbyMode(hdmi_tx_dev_t * dev, int enable);
int cec_CfgSignalFreeTime(hdmi_tx_dev_t * dev, int time);
int cec_GetSend(hdmi_tx_dev_t * dev);
int cec_SetSend(hdmi_tx_dev_t * dev);
int cec_IntClear(hdmi_tx_dev_t * dev, unsigned char mask);
int cec_IntDisable(hdmi_tx_dev_t * dev, unsigned char mask);
int cec_IntEnable(hdmi_tx_dev_t * dev, unsigned char mask);
int cec_IntStatus(hdmi_tx_dev_t * dev, unsigned char mask);
int cec_CfgTxBuf(hdmi_tx_dev_t * dev, const char *buf, unsigned size);
int cec_CfgRxBuf(hdmi_tx_dev_t * dev, char *buf, unsigned size);
int cec_GetLocked(hdmi_tx_dev_t * dev);
int cec_SetLocked(hdmi_tx_dev_t * dev);
int cec_CfgBroadcastNAK(hdmi_tx_dev_t * dev, int enable);
int cec_ctrlReceiveFrame(hdmi_tx_dev_t * dev, char *buf, unsigned size);
int cec_ctrlSendFrame(hdmi_tx_dev_t * dev, const char *buf, unsigned size, unsigned src, unsigned dst);
/******************************************************************************
 *
 *****************************************************************************/

#endif /* CEC_H */
