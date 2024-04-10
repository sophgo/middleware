//-----------------------------------------------------------------------------
// COPYRIGHT (C) 2020   CHIPS&MEDIA INC. ALL RIGHTS RESERVED
// 
// This file is distributed under BSD 3 clause and LGPL2.1 (dual license)
// SPDX License Identifier: BSD-3-Clause
// SPDX License Identifier: LGPL-2.1-only
// 
// The entire notice above must be reproduced on all authorized copies.
// 
// Description  : 
//-----------------------------------------------------------------------------
#include "cnm_fpga.h"
#include "jdi.h"
#include "jpulog.h"
#include "regdefine.h"

BOOL CNM_InitTestDev(
    TestDevConfig  config
    )
{
#if defined(CNM_FPGA_PLATFORM) || defined(CNM_SIM_PLATFORM)
    Uint32 val;

    if (jdi_init() < 0) {
        return FALSE;
    }

    if (config.reset == TRUE) {
        jdi_hw_reset();
    }

    if (config.aclk < ACLK_MIN || config.aclk > ACLK_MAX) {
        JLOG(ERR, "Invalid ACLK(%d) valid range(%d ~ %d)\n", config.aclk, ACLK_MIN, ACLK_MAX);
        return FALSE;
    }

    if (config.cclk < CCLK_MIN || config.cclk > CCLK_MAX) {
        JLOG(ERR, "Invalid CCLK(%d) valid range(%d ~ %d)\n", config.cclk, CCLK_MIN, CCLK_MAX);
        return FALSE;
    }

    if (jdi_set_clock_freg(0, config.aclk, 0) < 0) {     /* third parameter will be ignored */
        return FALSE;
    }
    JLOG(INFO, "INITIALIZED ACLK WITH %dMHz\n", config.aclk);

    if (jdi_set_clock_freg(1, config.cclk, 0) < 0) {     /* third parameter will be ignored */
        return FALSE;
    }
    JLOG(INFO, "INITIALIZED CCLK WITH %dMHz\n", config.cclk);

    JpuWriteReg(CLK_SEL_RST, 1);
    JpuWriteReg(CLK_SEL_ACLK, config.aclk_div);
    JpuWriteReg(CLK_SEL_CCLK, config.cclk_div);
    JpuWriteReg(CLK_SEL_RST, 0);


    if (config.readDelay) val = (1<<16) | (config.readDelay&0x3fff);
    else                  val = 0;
    JpuWriteReg(MJPEG_READ_DELAY, val);
    JLOG(INFO, "AXI READ DELAY     : %d cycles(%08x)\n", config.readDelay, val);
    if (config.writeDelay) val = (1<<16) | (config.writeDelay&0x3fff);
    else                   val = 0;
    JpuWriteReg(MJPEG_WRITE_DELAY, val);
    JLOG(INFO, "AXI WRITE DELAY    : %d cycles(%08x)\n", config.writeDelay, val);

    jdi_release();
#endif

    return TRUE;
}


