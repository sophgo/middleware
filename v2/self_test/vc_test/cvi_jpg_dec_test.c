#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/cvi_vc_drv_ioctl.h>


char *str_cat(char *dest, size_t max_len, const char *src)
{
	size_t dest_len = strlen(dest);
	size_t  n = 0;
	size_t i;

	if (dest_len >= max_len) {
		return dest;
	}

	n = max_len - dest_len;

	for (i = 0 ; i < n && src[i] != '\0' ; i++) {
		dest[dest_len + i] = src[i];
	}

	dest[dest_len + i] = '\0';

	return dest;
}
int main(int argc, char *argv[])
{
	int fd;
	char buf[512] = {0};
	int i;
	int ret;

	memset(buf, 0, 512);

	for (i = 0; i < argc; i++) {
		str_cat(buf, 512, argv[i]);
		str_cat(buf, 512, " ");
	}

	fd = open("/dev/cvi_vc_dec0", O_RDWR | O_DSYNC);

	if (fd <= 0) {
		printf("open vdec device failed\n");
		return -1;
	}

	ret = ioctl(fd, CVI_VC_VDEC_DEC_JPEG_TEST, buf);

	if (ret < 0) {
		printf("test failed,ret=%d\n", ret);
		return -1;
	}

	printf("test ok,ret = %d\n", ret);

	return 0;
}
