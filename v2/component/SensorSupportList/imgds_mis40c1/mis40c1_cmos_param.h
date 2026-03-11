#ifndef __MIS40C1_CMOS_PARAM_H_
#define __MIS40C1_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
#include "mis40c1_cmos_ex.h"

static const MIS40C1_MODE_S g_astMIS40C1_mode[MIS40C1_MODE_NUM] = {
	[MIS40C1_MODE_1440P20_2L] = {
		.name = "1440p20_2L",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2564,
				.u32Height = 1444,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stMaxSize = {
				.u32Width = 2564,
				.u32Height = 1444,
			},
		},
		.f32MaxFps = 20,
		.f32MinFps = 1, /* 1500 * 30 / 0x7FFF*/
		.u32HtsDef = 3300,
		.u32VtsDef = 2000,
		.stExp[0] = {//exp_time
			.u32Min = 1,
			.u32Max = 2000 - 4, //exp_max
			.u32Def = 128,
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 65536,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 2032,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[MIS40C1_MODE_1440P20_1L] = {
		.name = "1440p20_1L",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2564,
				.u32Height = 1444,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stMaxSize = {
				.u32Width = 2564,
				.u32Height = 1444,
			},
		},
		.f32MaxFps = 20,
		.f32MinFps = 1,
		.u32HtsDef = 3300,
		.u32VtsDef = 1500,
		.stExp[0] = {//exp_time
			.u32Min = 1,
			.u32Max = 1500 - 4, //exp_max
			.u32Def = 128,
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 65536,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 2032,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
};

static ISP_CMOS_BLACK_LEVEL_S g_stIspBlcCalibratio = {
	.bUpdate = CVI_TRUE,
	.blcAttr = {
		.Enable = 1,
		.enOpType = OP_TYPE_AUTO,
		.stManual = {256, 256, 256, 256, 0, 0, 0, 0},
		.stAuto = {
		{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
		{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
		{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
		{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		},
	},
};

struct combo_dev_attr_s mis40c1_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {2, 0, 1, -1, -1},
		.pn_swap = {1, 1, 1, 0, 0},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.dphy = {
			.enable = 1,
			.hs_settle = 10,
		},
	},
	.mclk = {
		.cam = 1,
		.freq = CAMPLL_FREQ_24M,
	},
	.devno = 0,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __MIS40C1_CMOS_PARAM_H_ */
