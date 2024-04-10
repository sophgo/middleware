/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef INCLUDE_APP_SYSTEM_H_
#define INCLUDE_APP_SYSTEM_H_

#if 0
#define LOG_TRACE() \
	do { if (1) hdmitx_logger(SNPS_TRACE, "%s", __func__); } while (0)
#define LOG_TRACE1(a) \
	do { if (1) hdmitx_logger(SNPS_TRACE, "* %s:%d", __func__, a); } while (0)
#define LOG_TRACE2(a,b) \
	do { if (1) hdmitx_logger(SNPS_TRACE, "** %s: %d %d", __func__, a, b); } while (0)
#define LOG_TRACE3(a,b,c) \
	do { if (1) hdmitx_logger(SNPS_TRACE, "*** %s: %d %d %d", __func__, a, b, c); } while (0)
#else
#define LOG_TRACE()
#define LOG_TRACE1(a)
#define LOG_TRACE2(a,b)
#define LOG_TRACE3(a,b,c)
#endif

#define LOGGER(level, format, ...)  \
	do { if (1) hdmitx_logger(level, format, ##__VA_ARGS__); } while (0)

/**
 * From system.h we need:
 *  - A logger function
 *  - A print function
 *  - A sleep function
 */
int  hdmitx_logger(int level, const char *format, ...);
int  hdmitx_log(int filedes, int level, const char *format, ...);
int  hdmitx_print(char const *str, ...);
void hdmitx_sleep(int ms);

#endif /* INCLUDE_APP_SYSTEM_H_ */
