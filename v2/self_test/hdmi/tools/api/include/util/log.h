/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef LOG_H_
#define LOG_H_

#include "bsp/system.h"
#include "hdmitx_dev.h"

/**
 * @file
 *	The log  offers a logging utility .
 *	It masks the I/O and logging of the OS, and enables verbose logging in debug mode.
 */
#ifdef __cplusplus
extern "C" {
#endif


#if 0
#define LOG_TRACE() \
	do { if (snps_functions && snps_functions->logger) snps_functions->logger(SNPS_TRACE, "%s", __func__); } while (0)
#define LOG_TRACE1(a) \
	do { if (snps_functions && snps_functions->logger) snps_functions->logger(SNPS_TRACE, "* %s:%d", __func__, a); } while (0)
#define LOG_TRACE2(a,b) \
	do { if (snps_functions && snps_functions->logger) snps_functions->logger(SNPS_TRACE, "** %s: %d %d", __func__, a, b); } while (0)
#define LOG_TRACE3(a,b,c) \
	do { if (snps_functions && snps_functions->logger) snps_functions->logger(SNPS_TRACE, "*** %s: %d %d %d", __func__, a, b, c); } while (0)
#else
#define LOG_TRACE()
#define LOG_TRACE1(a)
#define LOG_TRACE2(a,b)
#define LOG_TRACE3(a,b,c)
#endif

#define LOGGER(level, format, ...)  \
	do { if (snps_functions && snps_functions->logger) snps_functions->logger(level, format, ##__VA_ARGS__); } while (0)


#ifdef __cplusplus
}
#endif
#endif	/* LOG_H_ */
