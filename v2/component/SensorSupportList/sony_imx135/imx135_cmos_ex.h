#ifndef __IMX135_CMOS_EX_H_
#define __IMX135_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

enum imx135_linear_regs_e {
	LINEAR_HOLD_START = 0,
	LINEAR_SHR_L,
	LINEAR_SHR_H,
	LINEAR_AGAIN,
	LINEAR_DGAIN_GR_L,
	LINEAR_DGAIN_GR_H,
	LINEAR_DGAIN_R_L,
	LINEAR_DGAIN_R_H,
	LINEAR_DGAIN_B_L,
	LINEAR_DGAIN_B_H,
	LINEAR_DGAIN_GB_L,
	LINEAR_DGAIN_GB_H,
	LINEAR_VMAX_L,
	LINEAR_VMAX_H,
	LINEAR_FLIP_MIRROR,
	LINEAR_HOLD_END,
	LINEAR_REGS_NUM
};

enum imx135_dol2_regs_e {
	DOL2_HOLD = 0,
	DOL2_SHR0_L,
	DOL2_SHR0_M,
	DOL2_SHR0_H,
	DOL2_SHR1_L,
	DOL2_SHR1_M,
	DOL2_SHR1_H,
	DOL2_RHS1_L,
	DOL2_RHS1_M,
	DOL2_RHS1_H,
	DOL2_GAIN_L,
	DOL2_GAIN_H,
	DOL2_HCG,
	DOL2_HCG_SEF,
	DOL2_GAIN_SHORT_L,
	DOL2_GAIN_SHORT_H,
	DOL2_VMAX_L,
	DOL2_VMAX_M,
	DOL2_VMAX_H,
	DOL2_LINEAR_FLIP_MIRROR,
	DOL2_REL,
	DOL2_REGS_NUM
};

typedef enum _IMX135_MODE_E {
	IMX135_MODE_8M25 = 0,
	IMX135_MODE_LINEAR_NUM,
	IMX135_MODE_8M30_WDR = IMX135_MODE_LINEAR_NUM,
	IMX135_MODE_NUM
} IMX135_MODE_E;

typedef struct _IMX135_STATE_S {
	CVI_U8       u8Hcg;
	CVI_U32      u32BRL;
	CVI_U32      u32RHS1;
	CVI_U32      u32RHS1_MAX;
} IMX135_STATE_S;

typedef struct _IMX135_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	CVI_U16 u16RHS1;
	CVI_U16 u16BRL;
	CVI_U16 u16OpbSize;
	CVI_U16 u16MarginVtop;
	CVI_U16 u16MarginVbot;
	char name[64];
} IMX135_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastImx135[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunImx135_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunImx135_AddrInfo[];
extern CVI_U16 g_au16Imx135_GainMode[];
extern CVI_U8 imx135_i2c_addr;
extern const CVI_U32 imx135_addr_byte;
extern const CVI_U32 imx135_data_byte;
extern void imx135_init(VI_PIPE ViPipe);
extern void imx135_exit(VI_PIPE ViPipe);
extern int imx135_i2c_exit(VI_PIPE ViPipe);
extern void imx135_standby(VI_PIPE ViPipe);
extern void imx135_restart(VI_PIPE ViPipe);
extern int  imx135_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  imx135_read_register(VI_PIPE ViPipe, int addr);
extern void imx135_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  imx135_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __IMX135_CMOS_EX_H_ */
