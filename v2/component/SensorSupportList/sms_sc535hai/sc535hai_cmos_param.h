#ifndef __SC535HAI_CMOS_PARAM_H_
#define __SC535HAI_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
#include "sc535hai_cmos_ex.h"

static const SC535HAI_MODE_S g_astSC535HAI_mode[SC535HAI_MODE_NUM] = {
	[SC535HAI_MODE_1920P30_4L_MASTER] = {
		.name = "1920p30_4l_master",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2560,
				.u32Height = 1920,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1920,
			},
			.stMaxSize = {
				.u32Width = 2560,
				.u32Height = 1920,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 1.03, /* 2250 * 30 / 0xFFF0*/
		.u32HtsDef = 6000,
		.u32VtsDef = 2000,
		.stExp[0] = {
			.u32Min = 4,
			.u32Max = 2000 - 8,
			.u32Def = 1992,  // 30fps
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 81607,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16128,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC535HAI_MODE_1920P30_4L_SLAVE] = {
		.name = "1920p30_4l_slave",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2560,
				.u32Height = 1920,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1920,
			},
			.stMaxSize = {
				.u32Width = 2560,
				.u32Height = 1920,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 1.03, /* 2250 * 30 / 0xFFF0*/
		.u32HtsDef = 6000,
		.u32VtsDef = 2000,
		.stExp[0] = {
			.u32Min = 4,
			.u32Max = 2000 - 8,
			.u32Def = 1992,  // 30fps
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 81607,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16128,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC535HAI_MODE_1920P30_2L_MASTER] = {
		.name = "1920p30_2l_master",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2560,
				.u32Height = 1920,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1920,
			},
			.stMaxSize = {
				.u32Width = 2560,
				.u32Height = 1920,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 1.03, /* 2250 * 30 / 0xFFF0*/
		.u32HtsDef = 3000,
		.u32VtsDef = 2000,
		.stExp[0] = {
			.u32Min = 4,
			.u32Max = 2000 - 8,
			.u32Def = 1992,  // 30fps
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 81607,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16128,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC535HAI_MODE_1920P30_2L_SLAVE] = {
		.name = "1920p30_2l_slave",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2560,
				.u32Height = 1920,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1920,
			},
			.stMaxSize = {
				.u32Width = 2560,
				.u32Height = 1920,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 1.03, /* 2250 * 30 / 0xFFF0*/
		.u32HtsDef = 3000,
		.u32VtsDef = 2000,
		.stExp[0] = {
			.u32Min = 4,
			.u32Max = 2000 - 8,
			.u32Def = 1992,  // 30fps
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 81607,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16128,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC535HAI_MODE_1920P15_4L_MASTER] = {
		.name = "1920p15_4l_master",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2560,
				.u32Height = 1920,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1920,
			},
			.stMaxSize = {
				.u32Width = 2560,
				.u32Height = 1920,
			},
		},
		.f32MaxFps = 15,
		.f32MinFps = 1.03, /* 4500 * 15 / 0xFFF0*/
		.u32HtsDef = 6000,
		.u32VtsDef = 4000,
		.stExp[0] = {
			.u32Min = 4,
			.u32Max = 4000 - 8,
			.u32Def = 1992,
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 81607,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16128,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC535HAI_MODE_1920P15_4L_SLAVE] = {
		.name = "1920p15_4l_slave",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2560,
				.u32Height = 1920,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1920,
			},
			.stMaxSize = {
				.u32Width = 2560,
				.u32Height = 1920,
			},
		},
		.f32MaxFps = 15,
		.f32MinFps = 1.03, /* 4500 * 15 / 0xFFF0*/
		.u32HtsDef = 6000,
		.u32VtsDef = 4000,
		.stExp[0] = {
			.u32Min = 4,
			.u32Max = 4000 - 8,
			.u32Def = 1992,
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 81607,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16128,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC535HAI_MODE_1920P15_2L_MASTER] = {
		.name = "1920p15_2l_master",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2560,
				.u32Height = 1920,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1920,
			},
			.stMaxSize = {
				.u32Width = 2560,
				.u32Height = 1920,
			},
		},
		.f32MaxFps = 15,
		.f32MinFps = 1.03, /* 4500 * 15 / 0xFFF0*/
		.u32HtsDef = 3000,
		.u32VtsDef = 4000,
		.stExp[0] = {
			.u32Min = 4,
			.u32Max = 4000 - 8,
			.u32Def = 1992,
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 81607,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16128,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC535HAI_MODE_1920P15_2L_SLAVE] = {
		.name = "1920p15_2l_slave",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2560,
				.u32Height = 1920,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1920,
			},
			.stMaxSize = {
				.u32Width = 2560,
				.u32Height = 1920,
			},
		},
		.f32MaxFps = 15,
		.f32MinFps = 1.03, /* 4500 * 15 / 0xFFF0*/
		.u32HtsDef = 3000,
		.u32VtsDef = 4000,
		.stExp[0] = {
			.u32Min = 4,
			.u32Max = 4000 - 8,
			.u32Def = 1992,
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 81607,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16128,
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
		.stManual = {260, 260, 260, 260, 0, 0, 0, 0
#ifdef ARCH_CV182X
			, 1093, 1093, 1093, 1093
#endif
		},
		.stAuto = {
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
#ifdef ARCH_CV182X
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				/*8*/1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093},
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				/*8*/1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093},
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				/*8*/1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093},
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				/*8*/1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093},
#endif
		},
	},
};

struct combo_dev_attr_s sc535hai_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_600M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {1, 2, 0, 3, 4},
		.pn_swap = {1, 1, 1, 1, 1},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
	},
	.mclk = {
		.cam = 0,
		.freq = CAMPLL_FREQ_27M,
	},
	.devno = 0,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __SC535HAI_CMOS_PARAM_H_ */
