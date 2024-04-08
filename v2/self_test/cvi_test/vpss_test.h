#ifndef SELF_TEST_VPSS_INCLUDE_H_
#define SELF_TEST_VPSS_INCLUDE_H_

struct vpss_test_ops {
	CVI_S32 (*vi_test)(SIZE_S stSize, bool fail_pause);
	CVI_S32 (*in_fmt_test)(bool fail_pause);
	CVI_S32 (*out_fmt_test)(bool fail_pause);
	CVI_S32 (*yuv_2k_test)(bool fail_pause);
	CVI_S32 (*yuv_4k_test)(bool fail_pause);
	CVI_S32 (*rgb_test)(bool fail_pause);
	CVI_S32 (*dual_yuv_test)(bool fail_pause);
	CVI_S32 (*dual_combo_test)(bool fail_pause);
	CVI_S32 (*param_grp_test)(bool fail_pause);
	CVI_S32 (*param_chn_test)(bool fail_pause);
	CVI_S32 (*grp_attr_stress_test)(void);
	CVI_S32 (*chn_attr_stress_test)(void);
};

extern struct vpss_test_ops vpss_test_ops_1835;
extern struct vpss_test_ops vpss_test_ops_1822;
extern struct vpss_test_ops vpss_test_ops_mars;

#endif // SELF_TEST_VPSS_INCLUDE_H_
