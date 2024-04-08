/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef SRC_HDMITX_IPK_API_CLOCK_MNG_CLOCK_MNG_H_
#define SRC_HDMITX_IPK_API_CLOCK_MNG_CLOCK_MNG_H_

#include "includes.h"

#define FRAC_PRECISION 10

#define MMCM_RESET 			0x50020
#define MMCM_PIXELCLK_RST_MASK    	0x1
#define MMCM_AUDIOCLK_RST_MASK    	0x2

#define CLOCK_MANAGER 		0

/** DRP Address Map*/
#define DRP_CLKOUT5_REG_1	(CLOCK_MANAGER + 0x0018)
#define DRP_CLKOUT5_REG_2	(CLOCK_MANAGER + 0x001c)
#define DRP_CLKOUT0_REG_1	(CLOCK_MANAGER + 0x0020)
#define DRP_CLKOUT0_REG_2	(CLOCK_MANAGER + 0x0024)
#define DRP_CLKOUT1_REG_1	(CLOCK_MANAGER + 0x0028)
#define DRP_CLKOUT1_REG_2	(CLOCK_MANAGER + 0x002c)
#define DRP_CLKOUT2_REG_1	(CLOCK_MANAGER + 0x0030)
#define DRP_CLKOUT2_REG_2	(CLOCK_MANAGER + 0x0034)
#define DRP_CLKOUT3_REG_1	(CLOCK_MANAGER + 0x0038)
#define DRP_CLKOUT3_REG_2	(CLOCK_MANAGER + 0x003c)
#define DRP_CLKOUT4_REG_1	(CLOCK_MANAGER + 0x0040)
#define DRP_CLKOUT4_REG_2	(CLOCK_MANAGER + 0x0044)
#define DRP_CLKOUT6_REG_1	(CLOCK_MANAGER + 0x0048)
#define DRP_CLKOUT6_REG_2	(CLOCK_MANAGER + 0x004c)
#define DRP_CLKFBOUT_REG_1	(CLOCK_MANAGER + 0x0050)
#define DRP_CLKFBOUT_REG_2	(CLOCK_MANAGER + 0x0054)
#define DRP_DIVLOCK		(CLOCK_MANAGER + 0x0058)
#define DRP_LOCK_REG_1		(CLOCK_MANAGER + 0x0060)
#define DRP_LOCK_REG_2		(CLOCK_MANAGER + 0x0064)
#define DRP_LOCK_REG_3		(CLOCK_MANAGER + 0x0068)
#define DRP_POWER_REG		(CLOCK_MANAGER + 0x00a0)
#define DRP_DIGFILT_REG_1	(CLOCK_MANAGER + 0x0138)
#define DRP_DIGFILT_REG_2	(CLOCK_MANAGER + 0x013c)

enum bandwidth {
	LOW,
	HIGH
};

enum target_device {
	PIXEL,
	AUDIO
};

struct mmcm {
	double 		clock;
	uint32_t        mmcm_clkfbout_mult   ;
	uint32_t        mmcm_clkfbout_phase  ;
	uint8_t         mmcm_clkfbout_frac_en;
	uint32_t        mmcm_clkfbout_frac   ;
	enum bandwidth  mmcm_bandwidth       ;
	uint32_t        mmcm_divclk_divide   ;
	uint32_t        mmcm_clkout0_divide  ;
	uint32_t        mmcm_clkout0_phase   ;
	uint32_t        mmcm_clkout0_duty    ;
	uint32_t        mmcm_clkout0_frac    ;
	uint8_t         mmcm_clkout0_frac_en ;
	uint32_t        mmcm_clkout1_divide  ;
	uint32_t        mmcm_clkout1_phase   ;
	uint32_t        mmcm_clkout1_duty    ;
	uint32_t        mmcm_clkout2_divide  ;
	uint32_t        mmcm_clkout2_phase   ;
	uint32_t        mmcm_clkout2_duty    ;
	uint32_t        mmcm_clkout3_divide  ;
	uint32_t        mmcm_clkout3_phase   ;
	uint32_t        mmcm_clkout3_duty    ;
	uint32_t        mmcm_clkout4_divide  ;
	uint32_t        mmcm_clkout4_phase   ;
	uint32_t        mmcm_clkout4_duty    ;
	uint32_t        mmcm_clkout5_divide  ;
	uint32_t        mmcm_clkout5_phase   ;
	uint32_t        mmcm_clkout5_duty    ;
	uint32_t        mmcm_clkout6_divide  ;
	uint32_t        mmcm_clkout6_phase   ;
	uint32_t        mmcm_clkout6_duty    ;
	enum target_device device	     ;
	uint32_t	rclk_div_factor	     ;
};


struct divider {
	uint16_t low_time : 6;
	uint16_t high_time: 6;
	uint16_t no_count : 1;
	uint16_t w_edge   : 1;
	uint16_t reserved : 2;
} __attribute__((packed));

struct phase {
	uint16_t delay_time: 6;
	uint16_t phase_mux : 3;
	uint16_t mx   	   : 2;
	uint16_t reserved  : 5;
} __attribute__((packed));


struct count_calc {
	uint32_t low_time	: 6;
	uint32_t high_time	: 6;
	uint32_t reserved0      : 1;
	uint32_t phase_mux 	: 3;
	uint32_t delay_time	: 6;
	uint32_t no_count  	: 1;
	uint32_t w_edge  	: 1;
	uint32_t mx  		: 2;
	uint32_t wf_rise        : 1;
	uint32_t reserved1 	: 1; //28
	uint32_t divide_frac 	: 3;
	uint32_t reserved2 	: 1;
} __attribute__((packed));

typedef union {
	struct count_calc count_calc;
	uint32_t value;
} count_calc_t;


struct frac_count_calc {
	count_calc_t time;
	uint8_t frac_wf_fall 	:1;
	uint8_t frac_time 	:3;
	uint8_t reserved 	:4;
} __attribute__((packed));

#endif /* SRC_HDMITX_IPK_API_CLOCK_MNG_CLOCK_MNG_H_ */
