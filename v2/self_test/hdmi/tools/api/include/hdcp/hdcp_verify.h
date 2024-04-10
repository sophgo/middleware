/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HDCPVERIFY_H_
#define HDCPVERIFY_H_

#include "../hdmitx_dev.h"
#include "util/types.h"

typedef struct {
	u8 mLength[8];
	u8 mBlock[64];
	int mIndex;
	int mComputed;
	int mCorrupted;
	unsigned mDigest[5];
} sha_t;


#define KSV_MSK 	0x7F
#define VRL_LENGTH 	0x05
#define VRL_HEADER 	5
#define VRL_NUMBER 	3
#define HEADER 		10
#define SHAMAX 		20
#define DSAMAX 		20

void sha_reset(hdmi_tx_dev_t *dev, sha_t * sha);

int sha_result(hdmi_tx_dev_t *dev, sha_t * sha);

void sha_input(hdmi_tx_dev_t *dev, sha_t * sha, const u8 * data, size_t size);

void sha_process_block(hdmi_tx_dev_t *dev, sha_t * sha);

void sha_pad_message(hdmi_tx_dev_t *dev, sha_t * sha);

int hdcp_verify_dsa(hdmi_tx_dev_t *dev, const u8 * M, size_t n, const u8 * r, const u8 * s);

int hdcp_array_add(hdmi_tx_dev_t *dev, u8 * r, const u8 * a, const u8 * b, size_t n);

int hdcp_array_cmp(hdmi_tx_dev_t *dev, const u8 * a, const u8 * b, size_t n);

void hdcp_array_cpy(hdmi_tx_dev_t *dev, u8 * dst, const u8 * src, size_t n);

int hdcp_array_div(hdmi_tx_dev_t *dev, u8 * r, const u8 * D, const u8 * d, size_t n);

int hdcp_array_mac(hdmi_tx_dev_t *dev, u8 * r, const u8 * M, const u8 m, size_t n);

int hdcp_array_mul(hdmi_tx_dev_t *dev, u8 * r, const u8 * M, const u8 * m, size_t n);

void hdcp_array_set(hdmi_tx_dev_t *dev, u8 * dst, const u8 src, size_t n);

int hdcp_array_usb(hdmi_tx_dev_t *dev, u8 * r, const u8 * a, const u8 * b, size_t n);

void hdcp_array_swp(hdmi_tx_dev_t *dev, u8 * r, size_t n);

int hdcp_array_tst(hdmi_tx_dev_t *dev, const u8 * a, const u8 b, size_t n);

int hdcp_compute_exp(hdmi_tx_dev_t *dev, u8 * c, const u8 * M, const u8 * e, const u8 * p, size_t n, size_t nE);

int hdcp_compute_inv(hdmi_tx_dev_t *dev, u8 * out, const u8 * z, const u8 * a, size_t n);

int hdcp_compute_mod(hdmi_tx_dev_t *dev, u8 * dst, const u8 * src, const u8 * p, size_t n);

int hdcp_compute_mul(hdmi_tx_dev_t *dev, u8 * p, const u8 * a, const u8 * b, const u8 * m, size_t n);

int hdcp_verify_ksv(hdmi_tx_dev_t *dev, const u8 * data, size_t size);

int hdcp_verify_srm(hdmi_tx_dev_t *dev, const u8 * data, size_t size);

#endif	/* HDCPVERIFY_H_ */
