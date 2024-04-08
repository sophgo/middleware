#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include "buffers.h"

//NV21 64 beytes ALIGN PIC for test
#define PIC1_NV21 "pic1_nv21.yuv"
#define PIC2_NV21 "pic2_nv21_2560x1440.yuv"

struct bo *bo_obj[2];

int main()
{
	int fd;
	drmModeConnector *conn_dsi;
	drmModeConnector *conn_hdmi;
	drmModeRes *res;
	uint32_t crtc_dsi_id;
	uint32_t conn_dsi_id;
	uint32_t crtc_hdmi_id;
	uint32_t conn_hdmi_id;

	fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);

	res = drmModeGetResources(fd);
	crtc_dsi_id = res->crtcs[0];
	conn_dsi_id = res->connectors[0];
	crtc_hdmi_id = res->crtcs[1];
	conn_hdmi_id = res->connectors[1];

	conn_dsi = drmModeGetConnector(fd, conn_dsi_id);
	conn_hdmi = drmModeGetConnector(fd, conn_hdmi_id);

	bo_obj[0] = bo_create(fd, DRM_FORMAT_NV21,
		conn_dsi->modes[0].hdisplay, conn_dsi->modes[0].vdisplay,
		PIC1_NV21);

	bo_obj[1] = bo_create(fd, DRM_FORMAT_NV21,
		conn_hdmi->modes[0].hdisplay, conn_hdmi->modes[0].vdisplay,
		PIC2_NV21);

	drmModeSetCrtc(fd, crtc_dsi_id, bo_obj[0]->fb_id,
			0, 0, &conn_dsi_id, 1, &conn_dsi->modes[0]);


	drmModeSetCrtc(fd, crtc_hdmi_id, bo_obj[1]->fb_id,
			0, 0, &conn_hdmi_id, 1, &conn_hdmi->modes[0]);

	printf("drmModeSetCrtc\n");
	getchar();

	bo_destroy(bo_obj[0]);
	bo_destroy(bo_obj[1]);

	drmModeFreeConnector(conn_dsi);
	drmModeFreeConnector(conn_hdmi);
	drmModeFreeResources(res);

	close(fd);

	return 0;
}
