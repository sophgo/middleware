#ifndef __OV48B2Q_CMOS_EX_H_
#define __OV48B2Q_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

#define OV48B2Q_LINEAR_PD_USE_WDR 1
#define OV48B2Q_I2C_ADDR_1 0x36
#define OV48B2Q_I2C_ADDR_2 0x7f

enum ov48b2q_linear_regs_e {
	LINEAR_SHR_L = 0,
	LINEAR_SHR_H,
	LINEAR_GAIN_L,
	LINEAR_GAIN_H,
	LINEAR_VMAX_L,
	LINEAR_VMAX_H,
	LINEAR_REGS_NUM
};

enum ov48b2q_linear_pd_regs_e {
	LINEAR_PD_SHR_L = 0,
	LINEAR_PD_SHR_H,
	LINEAR_PD_AGAIN_COARSE,
	LINEAR_PD_AGAIN_FINE,
	LINEAR_PD_DGAIN_COARSE,
	LINEAR_PD_DGAIN_FINE_H,
	LINEAR_PD_DGAIN_FINE_L,
	LINEAR_PD_VMAX_L,
	LINEAR_PD_VMAX_H,
	LINEAR_PD_REGS_NUM
};

enum ov48b2q_wdr_regs_e {
	WDR_SHR0_L = 0,
	WDR_SHR0_H,
	WDR_SHR1_L,
	WDR_SHR1_H,
	WDR_GAIN_L,
	WDR_GAIN_H,
	WDR_GAIN_SHORT_L,
	WDR_GAIN_SHORT_H,
	WDR_VMAX_L,
	WDR_VMAX_H,
	WDR_REGS_NUM
};

typedef enum _OV48B2Q_MODE_E {
	OV48B2Q_MODE_8M30 = 0,
	OV48B2Q_MODE_12M10,
	OV48B2Q_MODE_LINEAR_NUM,
	OV48B2Q_MODE_8M30_WDR = OV48B2Q_MODE_LINEAR_NUM,
	OV48B2Q_MODE_12M10_PD,
	OV48B2Q_MODE_NUM
} OV48B2Q_MODE_E;

typedef struct _OV48B2Q_MODE_S {
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
} OV48B2Q_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastOv48b2q[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunOv48b2q_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunOv48b2q_AddrInfo[];
extern CVI_U16 g_au16Ov48b2q_GainMode[];
extern const CVI_U8 ov48b2q_i2c_addr;
extern const CVI_U32 ov48b2q_addr_byte;
extern const CVI_U32 ov48b2q_data_byte;
extern void ov48b2q_init(VI_PIPE ViPipe);
extern void ov48b2q_exit(VI_PIPE ViPipe);
extern int ov48b2q_i2c_exit(VI_PIPE ViPipe);
extern void ov48b2q_standby(VI_PIPE ViPipe);
extern void ov48b2q_restart(VI_PIPE ViPipe);
extern int  ov48b2q_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  ov48b2q_read_register(VI_PIPE ViPipe, int addr);
extern void ov48b2q_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  ov48b2q_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __OV48B2Q_CMOS_EX_H_ */
