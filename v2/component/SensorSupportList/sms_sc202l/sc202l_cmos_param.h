#ifndef __SC202L_CMOS_PARAM_H_
#define __SC202L_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
#include "sc202l_cmos_ex.h"

static const SC202L_MODE_S g_astSC202L_mode[SC202L_MODE_NUM] = {
	[SC202L_MODE_1920X1080P30_MASTER] = {
		.name = "1920X1080P30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stMaxSize = {
				.u32Width = 1920,
				.u32Height = 1080,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.51,  /* u32VtsDef * f32MaxFps / 0xFFFF*/
		.u32HtsDef = 2133,
		.u32VtsDef = 1125,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 2242,     /*{16’h320e,16’h320f}*2 - 8*/
			.u16Def = 2240,
			.u16Step = 2,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 59310,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 4096,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC202L_MODE_1920X1080P30_SLAVE] = {
		.name = "1920X1080P30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stMaxSize = {
				.u32Width = 1920,
				.u32Height = 1080,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.51,  /* u32VtsDef * f32MaxFps / 0xFFFF*/
		.u32HtsDef = 2133,
		.u32VtsDef = 1125,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 2242,     /*{16’h320e,16’h320f}*2 - 8*/
			.u16Def = 2240,
			.u16Step = 2,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 59310,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 4096,
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
		.stManual = {
			260, 260, 260, 260, 0, 0, 0, 0
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
		},
	},
};

struct combo_dev_attr_s sc202l_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {2, 0, -1, -1, -1},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.pn_swap = {0, 0, 0, 0, 0},
	},
	.mclk = {
		.cam = 0,
		.freq = CAMPLL_FREQ_24M,
	},
	.devno = 0,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __SC202L_CMOS_PARAM_H_ */
