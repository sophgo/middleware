#ifndef __SC235HAI_CMOS_EX_H_
#define __SC235HAI_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

enum sc235hai_linear_regs_e {
	LINEAR_HOLD_START,
	LINEAR_SHS_0_ADDR,
	LINEAR_SHS_1_ADDR,
	LINEAR_SHS_2_ADDR,
	LINEAR_AGAIN_0_ADDR,
	LINEAR_AGAIN_1_ADDR,
	LINEAR_DGAIN_0_ADDR,
	LINEAR_DGAIN_1_ADDR,
	LINEAR_VMAX_0_ADDR,
	LINEAR_VMAX_1_ADDR,
	LINEAR_HOLD_END,
	LINEAR_REGS_NUM
};

typedef enum _SC235HAI_MODE_E {
	SC235HAI_MODE_1080P15_1L = 0,
	SC235HAI_MODE_1080P15_1L_SLAVE,
	SC235HAI_MODE_1080P15_2L,
	SC235HAI_MODE_1080P30_2L,
	SC235HAI_MODE_1080P15_2L_SLAVE,
	SC235HAI_MODE_NUM
} SC235_MODE_E;

typedef struct _SC235HAI_STATE_S {
	CVI_U32		u32Sexp_MAX;	/* (2*{16’h3e23,16’h3e24} – 'd10)/2 */
} SC235HAI_STATE_S;

typedef struct _SC235HAI_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	CVI_U16 u16SexpMaxReg;		/* {16’h3e23,16’h3e24} */
	char name[64];
} SC235HAI_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastSC235HAI[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunSC235HAI_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunSC235HAI_AddrInfo[];
extern CVI_U16 g_au16SC235HAI_GainMode[];
extern CVI_U16 g_au16SC235HAI_L2SMode[];
extern const CVI_U8 sc235hai_i2c_addr;
extern const CVI_U32 sc235hai_addr_byte;
extern const CVI_U32 sc235hai_data_byte;
extern void sc235hai_init(VI_PIPE ViPipe);
extern void sc235hai_exit(VI_PIPE ViPipe);
extern int  sc235hai_i2c_exit(VI_PIPE ViPipe);
extern void sc235hai_standby(VI_PIPE ViPipe);
extern void sc235hai_restart(VI_PIPE ViPipe);
extern int  sc235hai_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  sc235hai_read_register(VI_PIPE ViPipe, int addr);
extern void sc235hai_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  sc235hai_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __SC235_CMOS_EX_H_ */
