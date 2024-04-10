/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef INCLUDE_REG_SCAN_H_
#define INCLUDE_REG_SCAN_H_

#include "../hdmitx_dev.h"
#include "../../../hdmitx_app/include/includes.h"
#include "../../src/core/identification/identification_reg.h"
#include "../core/interrupt/interrupt_reg.h"
#include "../../src/core/video/video_sampler_reg.h"
#include "../../src/core/video/video_packetizer_reg.h"
#include "../../src/core/frame_composer/frame_composer_reg.h"
#include "../../src/phy/phy_reg.h"
#include "../../src/core/audio/audio_packetizer_reg.h"
#include "../../src/core/main_controller/main_controller_reg.h"
#include "../../src/core/color_space/color_space_reg.h"
#include "../../src/hdcp/hdcp_reg.h"
#include "../../src/bsp/eddc_reg.h"

#define FIRST_WRITE  0x00
#define SECOND_WRITE 0xff
#define END_FLAG     0xffffffff

//AudioDMA Registers

#define AHB_DMA_CONF0               0x0000D800
#define AHB_DMA_START               0x0000D804
#define AHB_DMA_STOP                0x0000D808
#define AHB_DMA_THRSLD              0x0000D80C
#define AHB_DMA_STRADDR_SET0        0x0000D810
#define AHB_DMA_STPADDR_SET0        0x0000D820
#define AHB_DMA_BSTRADDR            0x0000D830
#define AHB_DMA_MBLENGTH0           0x0000D840
#define AHB_DMA_MBLENGTH1           0x0000D844
#define AHB_DMA_MASK                0x0000D850
#define AHB_DMA_CONF1               0x0000D858
#define AHB_DMA_BUFFMASK            0x0000D864
#define AHB_DMA_MASK1               0x0000D86C
#define AHB_DMA_STATUS              0x0000D870
#define AHB_DMA_CONF2               0x0000D874
#define AHB_DMA_STRADDR_SET1        0x0000D880
#define AHB_DMA_STPADDR_SET1        0x0000D890



bool reg_scan_read(uint32_t offset, hdmi_tx_dev_t dev_t, uint32_t read_value);
void reg_scan_write(uint32_t offset, hdmi_tx_dev_t dev_t, uint32_t write_value);
void reg_read_write(uint32_t reg_name,
                            hdmi_tx_dev_t dev_t,
                            uint32_t read_value_defaule,
                            uint32_t read_value_0,
                            uint32_t read_value_1,
                            uint32_t write_value_0,
                            uint32_t write_value_1);
void reg_scan(hdmi_tx_dev_t dev_t);


#endif


