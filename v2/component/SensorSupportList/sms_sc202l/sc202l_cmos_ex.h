#ifndef __sc202l_CMOS_EX_H_
#define __sc202l_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

enum sc202l_linear_regs_e {
    LINEAR_HOLD_START,
	LINEAR_EXP_H_ADDR,
    LINEAR_EXP_M_ADDR,
	LINEAR_EXP_L_ADDR,
	LINEAR_AGAIN_ADDR,
	LINEAR_DGAIN_ADDR,
    LINEAR_DGAIN_FINE_ADDR,
	LINEAR_VMAX_H_ADDR,
	LINEAR_VMAX_L_ADDR,
    LINEAR_HOLD_END,
	LINEAR_REGS_NUM
};

typedef enum _SC202L_MODE_E {
	SC202L_MODE_1920X1080P30_MASTER = 0,
	SC202L_MODE_1920X1080P30_SLAVE,
	SC202L_MODE_LINEAR_NUM,
	SC202L_MODE_NUM
} SC202L_MODE_E;

typedef struct _sc202l_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} SC202L_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastSC202L[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunSC202L_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunSC202L_AddrInfo[];
extern CVI_U16 g_au16SC202L_GainMode[];
extern CVI_U16 g_au16SC202L_L2SMode[];
extern const CVI_U8 sc202l_i2c_addr;
extern const CVI_U32 sc202l_addr_byte;
extern const CVI_U32 sc202l_data_byte;
extern void sc202l_init(VI_PIPE ViPipe);
extern void sc202l_exit(VI_PIPE ViPipe);
extern int  sc202l_i2c_exit(VI_PIPE ViPipe);
extern void sc202l_standby(VI_PIPE ViPipe);
extern void sc202l_restart(VI_PIPE ViPipe);
extern int  sc202l_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  sc202l_read_register(VI_PIPE ViPipe, int addr);
extern void sc202l_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  sc202l_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __sc202l_CMOS_EX_H_ */
