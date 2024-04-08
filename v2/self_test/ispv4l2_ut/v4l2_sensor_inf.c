#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <linux/sns_v4l2_uapi.h>

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

static int g_sensor_fd;

int get_sensor_fd(int dev)
{
	char devicename[64];
	int fd;
	int index = 2 + dev;

	if (g_sensor_fd > 0)
		return g_sensor_fd;

	sprintf(devicename, "/dev/v4l-subdev%d", index);
	fd = open(devicename, O_RDWR | O_NONBLOCK);
	if(fd < 0) {
		printf("open %s fail !\n", devicename);
		return -1;
	} else {
		printf("open %s success !\n", devicename);
		g_sensor_fd = fd;
	}
	return fd;
}

int test_sensor_ctrl(int dev)
{
	int sns_type;
	int orient = 2;
	int fd;

	fd = get_sensor_fd(dev);
	if(fd <= 0) {
		printf("get fd fail\n");
		return -1;
	}

	if(ioctl(fd, SNS_V4L2_GET_TYPE, &sns_type) < 0) {
		printf("test get sensor tpye fail !\n");
		return -1;
	} else {
		printf("test get sensor tpye(%d) success\n", sns_type);
	}

	if(ioctl(fd, SNS_V4L2_SET_MIRROR_FLIP, &orient) < 0) {
		printf("test set sensor orientation fail !\n");
		return -1;
	} else {
		printf("test set sensor orientation success:%d\n", orient);
	}

	return 0;
}