#ifndef SELF_TEST_VI_INCLUDE_H_
#define SELF_TEST_VI_INCLUDE_H_

#include "sample_comm.h"
#include "../../modules/sys/include/cvi_base.h"

struct vi_test_ops {
	CVI_S32 (*vi_open)(void);
	CVI_S32 (*vi_init)(SAMPLE_VI_CONFIG_S *viCfg);
	CVI_S32 (*vi_close)(SAMPLE_VI_CONFIG_S *viCfg);
	CVI_S32 (*vi_ofl_ol_vpss)(SAMPLE_VI_CONFIG_S *viCfg, SAMPLE_INI_CFG_S *iniCfg);
};

extern struct vi_test_ops viOps;

#endif // SELF_TEST_VI_INCLUDE_H_
