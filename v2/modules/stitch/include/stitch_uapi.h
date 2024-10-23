/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: stitch_uapi.h
 * Description:
 */

#ifndef _U_STITCH_UAPI_H_
#define _U_STITCH_UAPI_H_

#include <cvi_comm_stitch.h>

#ifdef __cplusplus
extern "C" {
#endif

struct stitch_src_frm_cfg {
	STITCH_SRC_IDX src_id;
	VIDEO_FRAME_INFO_S VideoFrame;
	CVI_S32 MilliSec;
};

struct stitch_chn_frm_cfg {
	VIDEO_FRAME_INFO_S VideoFrame;
	CVI_S32 MilliSec;
};

struct stitch_vb_pool_cfg {
	void *reserved;
	__u32 VbPool;
};

struct stitch_grp_src_frm_cfg {
	STITCH_GRP StitchGrp;
	struct stitch_src_frm_cfg cfg;
};

struct stitch_grp_chn_frm_cfg {
	STITCH_GRP StitchGrp;
	struct stitch_chn_frm_cfg cfg;
};

struct stitch_grp_vb_pool_cfg {
	STITCH_GRP StitchGrp;
	struct stitch_vb_pool_cfg vb_pool_cfg;
};

typedef struct stitch_grp_attr {
	STITCH_GRP StitchGrp;
} STITCH_GRP_ATTR_S;

typedef struct _stitch_grp_src_bd_attr {
	STITCH_GRP StitchGrp;
	struct stitch_src_bd_attr src_bd_attr;
} stitch_grp_src_bd_attr;

typedef struct _stitch_grp_src_attr {
	STITCH_GRP StitchGrp;
	struct stitch_src_attr src_attr;
} STITCH_GRP_SRC_ATTR_S;

typedef struct _stitch_grp_bld_wgt_attr {
	STITCH_GRP StitchGrp;
	struct stitch_bld_wgt_attr bld_wgt_attr;
} STITCH_GRP_WGT_ATTR_S;

typedef struct _stitch_grp_chn_attr {
	SIZE_S img_size;
	STITCH_GRP StitchGrp;
	struct stitch_chn_attr chn_attr;
} STITCH_GRP_CHN_ATTR_S;

typedef struct _stitch_grp_op_attr {
	STITCH_GRP StitchGrp;
	struct stitch_op_attr opt_attr;
} STITCH_GRP_OP_ATTR_S;


// typedef struct stitch_grp_attr STITCH_GRP_ATTR_S;

/* Public */

#define CVI_STITCH_INIT          _IO('S',   0x00)
#define CVI_STITCH_DEINIT        _IO('S',   0x01)
#define CVI_STITCH_SET_SRC_ATTR  _IOW('S',  0x02, STITCH_GRP_SRC_ATTR_S)
#define CVI_STITCH_GET_SRC_ATTR  _IOWR('S', 0x03, STITCH_GRP_SRC_ATTR_S)
#define CVI_STITCH_SET_CHN_ATTR  _IOW('S',  0x04, STITCH_GRP_CHN_ATTR_S)
#define CVI_STITCH_GET_CHN_ATTR  _IOWR('S', 0x05, STITCH_GRP_CHN_ATTR_S)
#define CVI_STITCH_SET_OP_ATTR   _IOW('S',  0x06, STITCH_GRP_OP_ATTR_S)
#define CVI_STITCH_GET_OP_ATTR   _IOWR('S', 0x07, STITCH_GRP_OP_ATTR_S)
#define CVI_STITCH_SET_WGT_ATTR   _IOW('S',  0x08, STITCH_GRP_WGT_ATTR_S)
#define CVI_STITCH_GET_WGT_ATTR   _IOWR('S', 0x09, STITCH_GRP_WGT_ATTR_S)

#define CVI_STITCH_SET_REGX      _IOW('S',  0x0a, CVI_U8)
#define CVI_STITCH_GRP_ENABLE    _IOW('S',   0x0b, STITCH_GRP_ATTR_S)
#define CVI_STITCH_GRP_DISABLE   _IOW('S',   0x0c, STITCH_GRP_ATTR_S)
#define CVI_STITCH_SEND_SRC_FRM  _IOW('S',  0x0d, struct stitch_grp_src_frm_cfg)
#define CVI_STITCH_SEND_CHN_FRM  _IOW('S',  0x0e, struct stitch_grp_chn_frm_cfg)
#define CVI_STITCH_GET_CHN_FRM   _IOWR('S', 0x0f, struct stitch_grp_chn_frm_cfg)
#define CVI_STITCH_RLS_CHN_FRM   _IOW('S',  0x10, struct stitch_grp_chn_frm_cfg)
#define CVI_STITCH_ATTACH_VB_POOL _IOW('S', 0x11, struct stitch_grp_vb_pool_cfg)
#define CVI_STITCH_DETACH_VB_POOL _IOW('S', 0x12, struct stitch_grp_vb_pool_cfg)
#define CVI_STITCH_DUMP_REGS _IO('S', 0x13)
#define CVI_STITCH_RST _IO('S', 0x14)

#define CVI_STITCH_SUSPEND _IO('S',0x15)
#define CVI_STITCH_RESUME _IO('S',0x16)

#define CVI_STITCH_INIT_GRP          _IOW('S',   0x17, STITCH_GRP_ATTR_S)
#define CVI_STITCH_DEINIT_GRP        _IOW('S',   0x18, STITCH_GRP_ATTR_S)
#define CVI_STITCH_GET_AVAIL_GROUP   _IOWR('S', 0x19, STITCH_GRP)

/* Internal use */

#ifdef __cplusplus
}
#endif

#endif /* _U_VPSS_UAPI_H_ */
