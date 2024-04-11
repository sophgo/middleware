#ifndef _LVDS_PARAM_EK79202_H_
#define _LVDS_PARAM_EK79202_H_

#include "../../../include/cvi_comm_vo.h"

const VO_LVDS_ATTR_S lvds_ek79202_cfg = {
	.mode = VO_LVDS_MODE_VESA,
	.out_bits = VO_LVDS_OUT_8BIT,
	.chn_num = 1,
	.data_big_endian = 0,
	.lane_id = {VO_LVDS_LANE_0, VO_LVDS_LANE_1, VO_LVDS_LANE_2, VO_LVDS_LANE_3, VO_LVDS_LANE_CLK},
	.lane_pn_swap = {false, false, false, false, false},
	.stSyncInfo = {
		.u16Hpw = 10,
		.u16Hbb = 88,
		.u16Hfb = 62,
		.u16Hact = 1280,
		.u16Vpw = 4,
		.u16Vbb = 23,
		.u16Vfb = 11,
		.u16Vact = 800,
		.bIvs = 0,
		.bIhs = 0,
		.u16FrameRate = 60
	},
	.pixelclock = 72403,
	.backlight_pin = {
		// .gpio_num = GPIOE_02,
		.active = GPIO_ACTIVE_HIGH,
	},
};

#else
#error "_LVDS_PARAM_ multi-delcaration!!"
#endif // _LVDS_PARAM_EK79202_H_
