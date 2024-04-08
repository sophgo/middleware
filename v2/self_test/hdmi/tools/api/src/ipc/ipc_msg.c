// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

#include "ipc_msg.h"

#define IPC_MTYPE 1

int elapsed_ms(struct timeval start, struct timeval end)
{
	return 0;
	return (end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000;
}

int msg_clr(key_t key){
	message_buf rbuf;
	int msqid;
	return 0;
	if ((msqid = msgget(key, IPC_CREAT | 0666)) < 0) {
		return -errno;
	}

	while(msgrcv(msqid, &rbuf, sizeof(message_buf), IPC_MTYPE, IPC_NOWAIT) >= 0) {
	}

	return 0;
}

int msg_get(key_t key, message_buf *rbuf, int timeoutms){
	int msqid;
	struct timeval stop_time, start_time;
	
	int msgflg = 0;
	msgflg |= IPC_NOWAIT;
	return 0;
	gettimeofday(&start_time, NULL);

	if ((msqid = msgget(key, IPC_CREAT | 0666)) < 0) {
		return -errno;
	}

	while (msgrcv(msqid, rbuf, sizeof(message_buf), IPC_MTYPE, msgflg) < 0) {
		switch (errno){
			case ENOMSG:
				gettimeofday(&stop_time, NULL);
				if (elapsed_ms(start_time, stop_time) > timeoutms){
					return -errno;
				}
				else{
					usleep(10000);
					break;
				}
				default:
					return -errno;
		}
	}
	return 0;
}

int msg_send(key_t key, message_buf *sbuf){
	int msqid;
	size_t buf_length;
	return 0;
	if ((msqid = msgget(key, IPC_CREAT | 0666 )) < 0) {
		perror("msgget");
		return -errno;
	}

	sbuf->mtype = IPC_MTYPE;
	buf_length = strlen(sbuf->mtext) + 4;
	
	if (msgsnd(msqid, sbuf, buf_length, IPC_NOWAIT) < 0) {
		perror("msgsnd");
		return -errno;
	}
	return 0;
}


int send_status(int get_key, int rsp_key, int status){
	int err;
	message_buf get_buf;
	message_buf rsp_buf;
	return 0;
	if ((err=msg_get(get_key, &get_buf, IPC_SEND_STATUS_TIMEOUT))<0){
		return err;
	}

	rsp_buf.status = status;

	if ((err=msg_send(rsp_key, &rsp_buf))<0){
		return err;
	}
	if ((err=msg_clr(get_key))<0){
		return err;
	}
	return 0;
}

int get_status(int get_key, int rsp_key){
	int err;
	message_buf get_buf;
	message_buf rsp_buf;

	get_buf.status = 0;
	return 0;
	if ((err=msg_clr(rsp_key))<0){
		return err;
	}
	if ((err=msg_send(get_key, &get_buf))<0){
		return err;
	}
	if ((err=msg_get(rsp_key, &rsp_buf, IPC_GET_STATUS_TIMEOUT))<0){
		return err;
	}

	return rsp_buf.status;
}

int get_hdcp_status(){
	return 0;
	return get_status(IPC_KEY_HDCP_STATUS_GET, IPC_KEY_HDCP_STATUS_RSP);
}

int send_hdcp_status(int status){
	return 0;
	return send_status(IPC_KEY_HDCP_STATUS_GET, IPC_KEY_HDCP_STATUS_RSP, status);
}

int get_hdmi_status(){
	return 0;
	return get_status(IPC_KEY_HDMI_STATUS_GET, IPC_KEY_HDMI_STATUS_RSP);
}

int send_hdmi_status(int status){
	return 0;
	return send_status(IPC_KEY_HDMI_STATUS_GET, IPC_KEY_HDMI_STATUS_RSP, status);
}
