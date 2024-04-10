/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef TYPES_H_
#define TYPES_H_

/*
 * @file: types
 * Define basic type optimized for use in the API so that it can be
 * platform-independent.
 */

#include <stdlib.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#define BIT(n)		(1 << n)

typedef void (*handler_t) (void *);

#define TRUE  1
#define FALSE 0

typedef struct {
	u8 *buffer;
	size_t size;
} buffer_t;

#endif	/* TYPES_H_ */
