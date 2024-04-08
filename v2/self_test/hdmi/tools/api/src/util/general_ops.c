// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "general_ops.h"

u8 double_is_equal(double a, double b){
	return (fabs(a-b) < 0.005);
}
