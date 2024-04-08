// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
 *
 * File Name: rtos_sample.c
 * Description:
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdint.h>

#include "fast_image.h"
#include "rtos_cmdqu.h"
#include "dump_uart.h"

int main(int argc, char *argv[])
{
	int32_t ch;
	int32_t opt;
	uint32_t ip_id = 0, cmd_id = 0, param_ptr = 0, cmd_type = 0, reserved = 0;
	int fd, fd2;
	struct cmdqu_t cmdq;
	unsigned long size;
	int i;
	unsigned long addr;
	struct dump_uart_s dump_uart;
	char *buf_ptr;
	char context[0x100];
	FILE *DumpFile;

	for (i = 0; i < 0x100; i++)
		context[i] = 0xFF;

	while ((ch = getopt(argc, argv, "i:c:v:p:t:r")) != -1)
		switch (ch) {
		case 'i':
			opt = atol(optarg);
			if (opt < 0)
				opt = 0;
			ip_id = (uint32_t)opt;
			break;
		case 'c':
			opt = atol(optarg);
			if (opt < 0)
				opt = 0;
			cmd_id = (uint32_t)opt;
			break;
		case 'v':
			opt = atol(optarg);
			if (opt < 0)
				opt = 0;
			reserved = (uint32_t)opt;
			break;
		case 'p':
			opt = atol(optarg);
			if (opt < 0)
				opt = 0;
			param_ptr = (uint32_t)opt;
			break;
		case 't':
			opt = atol(optarg);
			if (opt < 0)
				opt = 0;
			cmd_type = (uint32_t)opt;
			break;
		default:
			break;
		}
	fd = open("/dev/cvi-rtos-cmdqu", O_RDWR);
	fd2 = open("/dev/cvi-fast-image", O_RDWR);
	cmdq.ip_id = ip_id;
	cmdq.cmd_id = cmd_id;
	cmdq.param_ptr = param_ptr;
	cmdq.resv.mstime = reserved;
	printf("cmd_type = %d\n", cmd_type);
	printf("cmdq.ip_id= %d\n", cmdq.ip_id);
	printf("cmdq.cmd_id = %d\n", cmdq.cmd_id);
	printf("cmdq.resv.mstime = %d\n", cmdq.resv.mstime);
	printf("cmdq.param_ptr =%x\n", cmdq.param_ptr);

	switch (cmd_type) {
	case CMDQU_SEND:
		ioctl(fd, RTOS_CMDQU_SEND, &cmdq);
		break;
	case CMDQU_REQUEST:
		ioctl(fd, RTOS_CMDQU_REQUEST, &cmdq);
		break;
	case CMDQU_REQUEST_FREE:
		ioctl(fd, RTOS_CMDQU_REQUEST_FREE, &cmdq);
		break;
	case CMDQU_SEND_WAIT:
		ioctl(fd, RTOS_CMDQU_SEND_WAIT, &cmdq);
		printf("wait cmdq.ip_id= %d\n", cmdq.ip_id);
		printf("wait cmdq.cmd_id = %d\n", cmdq.cmd_id);
		printf("wait cmdq.resv.mstime = %d\n", cmdq.resv.mstime);
		printf("wait cmdq.param_ptr =%x\n", cmdq.param_ptr);
		break;
	case CMDQU_SEND_WAKEUP:
		ioctl(fd, RTOS_CMDQU_SEND_WAKEUP, &cmdq);
		break;
	case FAST_SEND_STOP_REC:
		ioctl(fd2, FAST_IMAGE_SEND_STOP_REC, &cmdq);
		break;
	case FAST_SEND_QUERY_ISP_PADDR:
		ioctl(fd2, FAST_IMAGE_QUERY_ISP_PADDR, &addr);
		printf("FAST_SEND_QUERY_ISP_PADDR %lx\n", addr);
		break;
	case FAST_SEND_QUERY_ISP_VADDR:
		ioctl(fd2, FAST_IMAGE_QUERY_ISP_VADDR, &addr);
		printf("FAST_SEND_QUERY_ISP_VADDR %lx\n", addr);
		break;
	case FAST_SEND_QUERY_ISP_SIZE:
		ioctl(fd2, FAST_IMAGE_QUERY_ISP_SIZE, &size);
		printf("FAST_SEND_QUERY_ISP_SIZE %lx\n", size);
		break;
	case FAST_SEND_QUERY_ISP_CTXT:
		ioctl(fd2, FAST_IMAGE_QUERY_ISP_CTXT, &context);
		printf("FAST_SEND_QUERY_ISP_CTXT\n");
		for (i = 0; i < 0x100; i++) {
			printf("%x ", context[i]);
			if (i % 0x10 == 0xF)
				printf("\n");
		}
		break;
	case FAST_SEND_QUERY_IMG_PADDR:
		ioctl(fd2, FAST_IMAGE_QUERY_IMG_PADDR, &addr);
		printf("FAST_SEND_QUERY_IMG_PADDR %lx\n", addr);
		break;
	case FAST_SEND_QUERY_IMG_VADDR:
		ioctl(fd2, FAST_IMAGE_QUERY_IMG_VADDR, &addr);
		printf("FAST_SEND_QUERY_IMG_VADDR %lx\n", addr);
		break;
	case FAST_SEND_QUERY_IMG_SIZE:
		ioctl(fd2, FAST_IMAGE_QUERY_IMG_SIZE, &size);
		printf("FAST_SEND_QUERY_IMG_SIZE %lx\n", size);
		break;
	case FAST_SEND_QUERY_IMG_CTXT:
		ioctl(fd2, FAST_IMAGE_QUERY_IMG_CTXT, &context);
		printf("FAST_SEND_QUERY_IMG_CTXT\n");
		for (i = 0; i < 0x100; i++) {
			printf("%x ", context[i]);
			if (i % 0x10 == 0xF)
				printf("\n");
		}
		break;
	case FAST_SEND_QUERY_ENC_PADDR:
		ioctl(fd2, FAST_IMAGE_QUERY_ENC_PADDR, &addr);
		printf("FAST_SEND_QUERY_ENC_PADDR %lx\n", addr);
		break;
	case FAST_SEND_QUERY_ENC_VADDR:
		ioctl(fd2, FAST_IMAGE_QUERY_ENC_VADDR, &addr);
		printf("FAST_SEND_QUERY_ENC_VADDR %lx\n", addr);
		break;
	case FAST_SEND_QUERY_ENC_SIZE:
		ioctl(fd2, FAST_IMAGE_QUERY_ENC_SIZE, &size);
		printf("FAST_SEND_QUERY_ENC_SIZE %lx\n", size);
		break;
	case FAST_SEND_QUERY_ENC_CTXT:
		ioctl(fd2, FAST_IMAGE_QUERY_ENC_CTXT, &context);
		printf("FAST_SEND_QUERY_ENC_CTXT\n");
		for (i = 0; i < 0x100; i++) {
			printf("%x ", context[i]);
			if (i % 0x10 == 0xF)
				printf("\n");
		}
		break;
	case FAST_SEND_QUERY_FREE_ISP_ION:
		ioctl(fd2, FAST_IMAGE_QUERY_FREE_ISP_ION, &cmdq);
		printf("FAST_SEND_QUERY_FREE_ISP_ION\n");
		break;
	case FAST_SEND_QUERY_FREE_IMG_ION:
		ioctl(fd2, FAST_IMAGE_QUERY_FREE_IMG_ION, &cmdq);
		printf("FAST_SEND_QUERY_FREE_IMG_ION\n");
		break;
	case FAST_SEND_QUERY_FREE_ENC_ION:
		ioctl(fd2, FAST_IMAGE_QUERY_FREE_ENC_ION, &cmdq);
		printf("FAST_SEND_QUERY_FREE_ENC_ION\n");
		break;
	case FAST_SEND_QUERY_DUMP_EN:
		ioctl(fd2, FAST_IMAGE_QUERY_DUMP_EN, &cmdq);
		printf("FAST_SEND_QUERY_DUMP_EN\n");
		break;
	case FAST_SEND_QUERY_DUMP_DIS:
		ioctl(fd2, FAST_IMAGE_QUERY_DUMP_DIS, &cmdq);
		printf("FAST_SEND_QUERY_DUMP_DIS\n");
		break;
	case FAST_SEND_QUERY_DUMP_MSG_INFO:
		DumpFile = fopen("dump_log.txt", "w+");
		ioctl(fd2, FAST_IMAGE_QUERY_DUMP_MSG_INFO, &dump_uart);
		printf("FAST_SEND_QUERY_DUMP_MSG_INFO\n");
		printf("dump_uart.dump_uart_max_size =%x\n", dump_uart.dump_uart_max_size);
		buf_ptr = (char *) malloc(dump_uart.dump_uart_max_size + sizeof(struct dump_uart_s));
		memset(buf_ptr, 0, dump_uart.dump_uart_max_size + sizeof(struct dump_uart_s));
		ioctl(fd2, FAST_IMAGE_QUERY_DUMP_MSG, buf_ptr);
		printf("FAST_SEND_QUERY_DUMP_MSG\n");
		fwrite(buf_ptr, 1, dump_uart.dump_uart_max_size, DumpFile);
		free(buf_ptr);
		fflush(DumpFile);
		fclose(DumpFile);
		break;
	case FAST_SEND_QUERY_DUMP_JPG_INFO:
		size = (int) param_ptr;
		snprintf(context, sizeof(context), "img_%d.jpg", (int) size);
		DumpFile = fopen(context, "wb");
		ioctl(fd2, FAST_IMAGE_QUERY_DUMP_JPG_INFO, &size);
		printf("dump_jpg size =%lx\n", size);
		buf_ptr = malloc(size);
		memset(buf_ptr, 0, size);
		ioctl(fd2, FAST_IMAGE_QUERY_DUMP_JPG, buf_ptr);
		printf("FAST_SEND_QUERY_DUMP_MSG\n");
		fwrite(buf_ptr, 1, size, DumpFile);
		free(buf_ptr);
		fflush(DumpFile);
		fclose(DumpFile);
		break;
	case FAST_SEND_QUERY_TRACE_SNAPSHOT_START:
		ioctl(fd2, FAST_IMAGE_QUERY_TRACE_SNAPSHOT_START, &cmdq);
		printf("FAST_SEND_QUERY_TRACE_SNAPSHOT_START\n");
		break;
	case FAST_SEND_QUERY_TRACE_SNAPSHOT_STOP:
		size = (int) param_ptr;
		DumpFile = fopen("snapshotg.bin", "wb");
		ioctl(fd2, FAST_IMAGE_QUERY_TRACE_SNAPSHOT_STOP, &size);
		printf("dump_snapshot size =%lx\n", size);
		buf_ptr = malloc(size);
		memset(buf_ptr, 0, size);
		ioctl(fd2, FAST_IMAGE_QUERY_TRACE_SNAPSHOT_DUMP, buf_ptr);
		printf("FAST_SEND_QUERY_TRACE_SNAPSHOT_DUMP\n");
		fwrite(buf_ptr, 1, size, DumpFile);
		free(buf_ptr);
		fflush(DumpFile);
		fclose(DumpFile);
		break;

	default:
		printf("fault:%d\n", cmd_type);
		break;
	}

	close(fd);
	close(fd2);
}
