#ifndef __IMX662_CMOS_EX_H_
#define __IMX662_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

/* [TODO] ======== Temporarily definitions start ========*/
//typedef struct _AWB_SENSOR_DEFAULT_S {
//	CVI_U16    u16InitRgain;           /*Init WB gain*/
//	CVI_U16    u16InitGgain;
//	CVI_U16    u16InitBgain;
//	CVI_U8     u8AWBRunInterval;       /*RW;AWB Run Interval*/
//} AWB_SENSOR_DEFAULT_S;

/* [TODO] ======== Temporarily definitions end ========*/
enum imx662_linear_regs_e {
	LINEAR_HOLD = 0,
	LINEAR_SHR0_0,
	LINEAR_SHR0_1,
	LINEAR_SHR0_2,
	LINEAR_GAIN,
	LINEAR_HCG,
	LINEAR_VMAX_0,
	LINEAR_VMAX_1,
	LINEAR_VMAX_2,
	LINEAR_REL,
	LINEAR_REGS_NUM
};

enum imx662_dol2_regs_e {
	DOL2_HOLD = 0,
	DOL2_SHR0_0,
	DOL2_SHR0_1,
	DOL2_SHR0_2,
	DOL2_GAIN,
	DOL2_HCG,
	DOL2_RHS1_0,
	DOL2_RHS1_1,
	DOL2_RHS1_2,
	DOL2_SHR1_0,
	DOL2_SHR1_1,
	DOL2_SHR1_2,
	DOL2_VMAX_0,
	DOL2_VMAX_1,
	DOL2_VMAX_2,
	DOL2_REL,
	DOL2_REGS_NUM
};

typedef enum _IMX662_MODE_E {
	IMX662_MODE_1080P30 = 0,
	IMX662_MODE_LINEAR_NUM,
	IMX662_MODE_1080P30_WDR = IMX662_MODE_LINEAR_NUM,
	IMX662_MODE_NUM
} IMX662_MODE_E;

typedef struct _IMX662_STATE_S {
	CVI_U8       u8Hcg;
	CVI_U32      u32BRL;
	CVI_U32      u32RHS1;
	CVI_U32      u32RHS1_MAX;
} IMX662_STATE_S;

typedef struct _IMX662_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_S stAgain[2];
	SNS_ATTR_S stDgain[2];
	CVI_U16 u16RHS1;
	CVI_U16 u16BRL;
	CVI_U16 u16OpbSize;
	CVI_U16 u16MarginVtop;
	CVI_U16 u16MarginVbot;
	char name[64];
} IMX662_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastImx662[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunImx662_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunImx662_AddrInfo[];
extern CVI_U16 g_au16Imx662_GainMode[];
extern const CVI_U8 imx662_i2c_addr;
extern const CVI_U32 imx662_addr_byte;
extern const CVI_U32 imx662_data_byte;
extern void imx662_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern void imx662_init(VI_PIPE ViPipe);
extern void imx662_exit(VI_PIPE ViPipe);
extern int  imx662_i2c_exit(VI_PIPE ViPipe);
extern void imx662_standby(VI_PIPE ViPipe);
extern void imx662_restart(VI_PIPE ViPipe);
extern int  imx662_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  imx662_read_register(VI_PIPE ViPipe, int addr);
extern int  imx662_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __IMX662_CMOS_EX_H_ */
