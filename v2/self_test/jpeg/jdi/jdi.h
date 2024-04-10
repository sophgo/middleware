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
#ifndef _JDI_HPI_H_
#define _JDI_HPI_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "../jpuapi/jpuconfig.h"
#include "mm.h"
#include "jputypes.h"

#define MAX_JPU_BUFFER_POOL 32
#define JpuWriteInstReg( INST_IDX, ADDR, DATA )		jdi_write_register( ((unsigned long)INST_IDX*NPT_REG_SIZE)+ADDR, DATA ) // system register write 	with instance index
#define JpuReadInstReg( INST_IDX, ADDR )		    jdi_read_register( ((unsigned long)INST_IDX*NPT_REG_SIZE)+ADDR ) // system register write 	with instance index
#define JpuWriteReg( ADDR, DATA )                   jdi_write_register( ADDR, DATA ) // system register write
#define JpuReadReg( ADDR )                          jdi_read_register( ADDR )           // system register write
#define JpuWriteMem( ADDR, DATA, LEN, ENDIAN )      jdi_write_memory( ADDR, DATA, LEN, ENDIAN ) // system memory write
#define JpuReadMem( ADDR, DATA, LEN, ENDIAN )       jdi_read_memory( ADDR, DATA, LEN, ENDIAN ) // system memory write

typedef Int32 HANDLE;

typedef struct jpu_buffer_t {
    //unsigned int  size;
    size_t  size;
    unsigned long phys_addr;
    unsigned long base;
    unsigned long virt_addr;
} jpu_buffer_t;

typedef struct jpu_instance_pool_t {
    unsigned char   jpgInstPool[MAX_NUM_INSTANCE][MAX_INST_HANDLE_SIZE];
    Int32           jpu_instance_num;
    BOOL            instance_pool_inited;
	void*           instPendingInst[MAX_NUM_INSTANCE];
    jpeg_mm_t       vmem;
} jpu_instance_pool_t;

#ifdef CNM_SIM_PLATFORM
typedef struct jpu_sim_context_t {
	Uint32           frameIdx;
	Uint32           instIdx;
} jpu_sim_context_t;
#endif

typedef enum {
    JDI_LITTLE_ENDIAN = 0,
    JDI_BIG_ENDIAN,
    JDI_32BIT_LITTLE_ENDIAN,
    JDI_32BIT_BIG_ENDIAN,
} EndianMode;


typedef enum {
    JDI_LOG_CMD_PICRUN  = 0,
    JDI_LOG_CMD_INIT  = 1,
    JDI_LOG_CMD_RESET  = 2,
    JDI_LOG_CMD_PAUSE_INST_CTRL = 3,
    JDI_LOG_CMD_MAX
} jdi_log_cmd;


#if defined (__cplusplus)
extern "C" {
#endif
    int jdi_probe();
    /* @brief It returns the number of task using JDI.
     */
    int jdi_get_task_num();
    int jdi_init();
    int jdi_release();    //this function may be called only at system off.
    jpu_instance_pool_t *jdi_get_instance_pool();
    int jdi_allocate_dma_memory(jpu_buffer_t *vb);
    void jdi_free_dma_memory(jpu_buffer_t *vb);
    int jdi_wait_interrupt(int timeout, unsigned long instIdx);
    int jdi_hw_reset();
    int jdi_wait_inst_ctrl_busy(int timeout, unsigned int addr_flag_reg, unsigned int flag);
    int jdi_set_clock_gate(int enable);
    int jdi_get_clock_gate();
    int jdi_open_instance(unsigned long instIdx);
    int jdi_close_instance(unsigned long instIdx);
    int jdi_get_instance_num();


    void jdi_write_register(unsigned long addr, unsigned int data);
    unsigned long jdi_read_register(unsigned long addr);

    size_t jdi_write_memory(unsigned long addr, unsigned char *data, size_t len, int endian);
    size_t jdi_read_memory(unsigned long addr, unsigned char *data, size_t len, int endian);

#ifdef CNM_SIM_PLATFORM
	void jdi_set_event_to_sim(jpu_sim_context_t *ctx);
#endif
    int jdi_lock();
    void jdi_unlock();
    void jdi_log(int cmd, int step, int inst);
    void jdi_delay_us(unsigned int us);
#if defined(CNM_FPGA_PLATFORM) || defined(CNM_SIM_PLATFORM)
    int jdi_set_clock_freg(int Device, int OutFreqMHz, int InFreqMHz);

#if defined(CNM_SIM_PLATFORM)
#define ACLK_MAX                    300
#define ACLK_MIN                    16
#define CCLK_MAX                    300
#define CCLK_MIN                    16
#endif
#else
#define ACLK_MAX                    300
#define ACLK_MIN                    16
#define CCLK_MAX                    300
#define CCLK_MIN                    16
#endif




#if defined (__cplusplus)
}
#endif

#endif //#ifndef _JDI_HPI_H_
