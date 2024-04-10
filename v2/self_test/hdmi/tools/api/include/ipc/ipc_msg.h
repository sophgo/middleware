/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef __IPC_MSG_H__
#define __IPC_MSG_H__

#define MSGSZ 128
#define IPC_GET_STATUS_TIMEOUT 10000
#define IPC_SEND_STATUS_TIMEOUT 10000

enum ipc_status {
	STATUS_HDMI_LOAD_REQUEST,
	STATUS_HDMI_KILL_REQUEST,
	STATUS_HDMI_HDCP_REAUTH_REQUEST,
	STATUS_HDCP_FIRMWARE_KILLED,
	STATUS_HDCP_READY
};

enum ipc_msg_type {
	HDCP_STATUS_GET,
	HDCP_STATUS_REPORT,
	HDMI_TX_STATUS_GET,
	HDMI_TX_STATUS_REPORT
};

typedef struct msg_buf {
    long mtype;
	enum ipc_status status;
	char mtext[MSGSZ];
} message_buf;

enum ipc_keys{
	IPC_KEY_HDCP_STATUS_GET = 1234,
	IPC_KEY_HDCP_STATUS_RSP = 1235,
	IPC_KEY_HDMI_STATUS_GET = 1236,
	IPC_KEY_HDMI_STATUS_RSP = 1237,
};

int msg_clr(key_t key);
int msg_send(key_t key, message_buf *sbuf);
int msg_get(key_t key, message_buf *rbuf, int timeoutms);

int get_status(int get_key, int rsp_key);
int send_status(int get_key, int rsp_key, int status);

int get_hdcp_status();
int send_hdcp_status(int status);

int get_hdmi_status();
int send_hdmi_status(int status);

#endif /* __IPC_MSG_H__ */