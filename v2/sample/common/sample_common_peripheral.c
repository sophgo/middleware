#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "sample_comm.h"

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64

int SAMPLE_COMM_GPIO_Export(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	if (!GPIO_IN_RANGE(gpio)) {
		perror("invalid gpio num");
		return -1;
	}
	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);

	return 0;
}

int SAMPLE_COMM_GPIO_Unexport(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	if (!GPIO_IN_RANGE(gpio)) {
		perror("invalid gpio num");
		return -1;
	}
	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);

	write(fd, buf, len);
	close(fd);
	return 0;
}

int SAMPLE_COMM_GPIO_SetDirection(unsigned int gpio, unsigned int out_flag)
{
	int fd;
	char buf[MAX_BUF];

	if (!GPIO_IN_RANGE(gpio)) {
		perror("invalid gpio num");
		return -1;
	}
	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR"/gpio%d/direction", gpio);
	if (access(buf, 0) == -1)
		SAMPLE_COMM_GPIO_Export(gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}
	//printf("mark %d , %s\n",out_flag, buf);
	if (out_flag)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);

	close(fd);
	return 0;
}

int SAMPLE_COMM_GPIO_SetValue(unsigned int gpio, unsigned int value)
{
	int fd;
	char buf[MAX_BUF];

	if (!GPIO_IN_RANGE(gpio)) {
		perror("invalid gpio num");
		return -1;
	}
	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR"/gpio%d/value", gpio);
	if (access(buf, 0) == -1)
		SAMPLE_COMM_GPIO_Export(gpio);

	SAMPLE_COMM_GPIO_SetDirection(gpio, 1); //output

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value");
		return fd;
	}

	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);

	close(fd);
	return 0;
}

int SAMPLE_COMM_GPIO_GetValue(unsigned int gpio, unsigned int *value)
{
	int fd;
	char buf[MAX_BUF];
	char ch;

	if (!GPIO_IN_RANGE(gpio)) {
		perror("invalid gpio num");
		return -1;
	}
	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
	if (access(buf, 0) == -1)
		SAMPLE_COMM_GPIO_Export(gpio);

	SAMPLE_COMM_GPIO_SetDirection(gpio, 0); //input

	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		perror("gpio/get-value");
		return fd;
	}

	read(fd, &ch, 1);

	if (ch != '0')
		*value = 1;
	else
		*value = 0;

	close(fd);
	return 0;
}

/*pwm*/
#define SYSFS_PWM_DIR "/sys/class/pwm/pwmchip0/"
int SAMPLE_COMM_PWM_SetParm(int chn, int period, int duty_cycle)
{
	int fd;
	char buf[MAX_BUF], buf1[MAX_BUF];

	if (!((chn >= 0) && (chn <= 3))) {
		printf("pwm chanel 0 ~ 3\n");
		return -1;
	}

	snprintf(buf, sizeof(buf), SYSFS_PWM_DIR"/pwm%d/period", chn);
	if (access(buf, 0) == -1) {
		fd = open(SYSFS_PWM_DIR"/export", O_WRONLY);
		if (fd < 0) {
			printf("open export error\n");
			return -1;
		}
		if (chn == 0)
			write(fd, "0", strlen("0"));
		else if (chn == 1)
			write(fd, "1", strlen("1"));
		else if (chn == 2)
			write(fd, "2", strlen("2"));
		else if (chn == 3)
			write(fd, "3", strlen("3"));

		close(fd);
	}

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		printf("open period error\n");
		return -1;
	}
	snprintf(buf1, sizeof(buf1), "%d", period);
	write(fd, buf1, sizeof(buf1));
	close(fd);

	snprintf(buf, sizeof(buf), SYSFS_PWM_DIR"/pwm%d/duty_cycle", chn);
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		printf("open duty_cycle error\n");
		return -1;
	}
	snprintf(buf1, sizeof(buf1), "%d", duty_cycle);
	write(fd, buf1, sizeof(buf1));
	close(fd);
	return 0;
}
int SAMPLE_COMM_PWM_Enable(int chn, int en)
{
	int fd;
	char buf[MAX_BUF];

	if (!((chn >= 0) && (chn <= 3))) {
		printf("pwm chanel 0 ~ 3\n");
		return -1;
	}
	snprintf(buf, sizeof(buf), SYSFS_PWM_DIR"/pwm%d/enable", chn);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		printf("open period error\n");
		return -1;
	}

	if (en)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);

	close(fd);
	return 0;
}

/*i2c*/
CVI_S32 SAMPLE_COMM_I2C_Write(CVI_S32 file, CVI_U16 addr, CVI_U16 reg, CVI_U16 val, CVI_U16 reg_w, CVI_U16 val_w)
{
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[2];
	int ret;
	unsigned char temp[4];

	messages[0].addr = addr;
	messages[0].flags = 0;
	messages[0].len = reg_w + val_w;

	switch (reg_w) {
	case 1:
		temp[0] = reg;
		switch (val_w) {
		case 1:
			temp[1] = val;
			break;
		case 2:
			temp[1] = val >> 8;
			temp[2] = val;
			break;
		default:
			printf("No support of this value width\n");
		}
		break;
	case 2:
		temp[0] = reg >> 8;
		temp[1] = reg;
		switch (val_w) {
		case 1:
			temp[2] = val;
			break;
		case 2:
			temp[2] = val >> 8;
			temp[3] = val;
			break;
		default:
			printf("No support of this value width\n");
		}
		break;
	default:
		printf("No support of this register width\n");
		return CVI_FAILURE;
	}

	messages[0].buf = temp;
	/* Send the request to the kernel and get the result back */
	packets.msgs = messages;
	packets.nmsgs = 1;
	ret = ioctl(file, I2C_RDWR, &packets);

	if (ret < 0) {
		perror("Unable to send data");
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_I2C_Read(CVI_S32 file, CVI_U16 addr, CVI_U16 reg, CVI_U16 reg_w, CVI_U8 *r_val)
{
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[2];
	int ret;
	unsigned char temp[2];

	messages[0].addr = addr;
	messages[0].flags = 0;
	messages[0].len = reg_w;

	switch (reg_w) {
	case 1:
		temp[0] = reg;
		break;
	case 2:
		temp[0] = reg >> 8;
		temp[1] = reg;
		break;
	default:
		printf("No support of this register width\n");
		return CVI_FAILURE;
	}

	messages[0].buf = temp;
	/* The data will get returned in this structure */
	messages[1].addr = addr;
	/* | I2C_M_NOSTART */
	messages[1].flags = I2C_M_RD;
	messages[1].len = 1;
	messages[1].buf = r_val;

	/* Send the request to the kernel and get the result back */
	packets.msgs = messages;
	packets.nmsgs = 2;
	ret = ioctl(file, I2C_RDWR, &packets);

	if (ret < 0) {
		perror("Unable to send data");
		return ret;
	}

	//printf("get val=%x\n", *r_val);

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_I2C_Open(CVI_CHAR *i2c_bus)
{
	//eg. i2c_bus = "dev/i2c-2"
	return open(i2c_bus, O_RDWR);
}

CVI_S32 SAMPLE_COMM_I2C_Close(CVI_S32 i2c_file)
{
	return close(i2c_file);
}
