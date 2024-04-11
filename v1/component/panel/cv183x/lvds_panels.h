#ifndef _LVDS_PANELS_H_
#define _LVDS_PANELS_H_

#include "../../../include/cvi_comm_vo.h"

#ifdef LVDS_PANEL_EK79202
#include "lvds_ek79202.h"
const VO_LVDS_ATTR_S *pstLvdsAttr = &lvds_ek79202_cfg;
#else
#include "lvds_ek79202.h"
const VO_LVDS_ATTR_S *pstLvdsAttr = &lvds_ek79202_cfg;
#endif

#endif // _LVDS_PANELS_H_
