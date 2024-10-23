#ifndef MODULES_VPU_INCLUDE_STITCH_IOCTL_H_
#define MODULES_VPU_INCLUDE_STITCH_IOCTL_H_

#include <sys/ioctl.h>

#include <stitch_uapi.h>

CVI_S32 cvi_stitch_init(CVI_S32 fd);
CVI_S32 cvi_stitch_deinit(CVI_S32 fd);
CVI_S32 cvi_stitch_set_src_attr(CVI_S32 fd, STITCH_GRP_SRC_ATTR_S *attr);
CVI_S32 cvi_stitch_get_src_attr(CVI_S32 fd, STITCH_GRP_SRC_ATTR_S *attr);
CVI_S32 cvi_stitch_set_chn_attr(CVI_S32 fd, STITCH_GRP_CHN_ATTR_S *attr);
CVI_S32 cvi_stitch_get_chn_attr(CVI_S32 fd, STITCH_GRP_CHN_ATTR_S *attr);
CVI_S32 cvi_stitch_set_op_attr(CVI_S32 fd, STITCH_GRP_OP_ATTR_S *attr);
CVI_S32 cvi_stitch_get_op_attr(CVI_S32 fd, STITCH_GRP_OP_ATTR_S *attr);
CVI_S32 cvi_stitch_set_wgt_attr(CVI_S32 fd, STITCH_GRP_WGT_ATTR_S *attr);
CVI_S32 cvi_stitch_get_wgt_attr(CVI_S32 fd, STITCH_GRP_WGT_ATTR_S *attr);
CVI_S32 cvi_stitch_set_regx(CVI_S32 fd, CVI_U8 regx);
CVI_S32 cvi_stitch_grp_enable(CVI_S32 fd, STITCH_GRP_ATTR_S *grpAttr);
CVI_S32 cvi_stitch_grp_disable(CVI_S32 fd, STITCH_GRP_ATTR_S *grpAttr);
CVI_S32 cvi_stitch_reset(CVI_S32 fd);
CVI_S32 cvi_stitch_send_src_frame(CVI_S32 fd, struct stitch_grp_src_frm_cfg *cfg);
CVI_S32 cvi_stitch_send_chn_frame(CVI_S32 fd, struct stitch_grp_chn_frm_cfg *cfg);
CVI_S32 cvi_stitch_get_chn_frame(CVI_S32 fd, struct stitch_grp_chn_frm_cfg *cfg);
CVI_S32 cvi_stitch_release_chn_frame(CVI_S32 fd, struct stitch_grp_chn_frm_cfg *cfg);
CVI_S32 cvi_stitch_attach_vbpool(CVI_S32 fd, const struct stitch_grp_vb_pool_cfg *cfg);
CVI_S32 cvi_stitch_detach_vbpool(CVI_S32 fd, const struct stitch_grp_vb_pool_cfg *cfg);
CVI_S32 cvi_stitch_dump_reginfo(CVI_S32 fd);
CVI_S32 cvi_stitch_suspend(CVI_S32 fd);
CVI_S32 cvi_stitch_resume(CVI_S32 fd);

CVI_S32 cvi_stitch_init_grp(CVI_S32 fd, STITCH_GRP_ATTR_S *grpAttr);
CVI_S32 cvi_stitch_deinit_grp(CVI_S32 fd, STITCH_GRP_ATTR_S *grpAttr);
CVI_S32 cvi_stitch_get_available_grp(CVI_S32 fd, STITCH_GRP *pVpssGrp);

#endif /* MODULES_VPU_INCLUDE_STITCH_IOCTL_H_ */
