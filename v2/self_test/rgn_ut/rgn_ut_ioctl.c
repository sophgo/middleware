#include "rgn_ut.h"

int _sc_set_rgn(int fd, struct cvi_rgn_cfg *cfg)
{
	S_CTRL_PTR(fd, cfg, RGN_IOCTL_SC_SET_RGN);
}

int _disp_set_rgn(int fd, struct cvi_rgn_cfg *cfg)
{
	S_CTRL_PTR(fd, cfg, RGN_IOCTL_DISP_SET_RGN);
}
