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
#if defined(linux) || defined(__linux) || defined(ANDROID)

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#ifdef    _KERNEL_
#include <linux/delay.h>
#endif
#include <signal.h>        /* SIGIO */
#include <fcntl.h>        /* fcntl */
#include <pthread.h>
#include <sys/mman.h>        /* mmap */
#include <sys/ioctl.h>        /* fopen/fread */
#include <sys/errno.h>        /* fopen/fread */
#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>

#include "driver/jpu.h"
#include "../jdi.h"
#include "jpulog.h"
#include "jputypes.h"
#include "regdefine.h"


#define JPU_DEVICE_NAME "/dev/jpu0"
#define JDI_INSTANCE_POOL_SIZE          sizeof(jpu_instance_pool_t)
#define JDI_INSTANCE_POOL_TOTAL_SIZE    (JDI_INSTANCE_POOL_SIZE + sizeof(MUTEX_HANDLE)*JDI_NUM_LOCK_HANDLES)
typedef pthread_mutex_t    MUTEX_HANDLE;

#endif /* CNM_FPGA_PLATFORM */

#ifdef CNM_SIM_PLATFORM
static unsigned long io_mutex_val;
static void* s_io_mutex = (void *)&io_mutex_val;

#ifdef CNM_SIM_DPI_INTERFACE
static int dpi_init(unsigned long dram_base);
static void dpi_release(void);
static void dpi_write_register(unsigned int addr, unsigned int data);
static unsigned int dpi_read_register(unsigned int addr);
static size_t dpi_write_memory(unsigned int addr, unsigned char *data, size_t len, int endian);
static size_t dpi_read_memory(unsigned int addr, unsigned char *data, size_t len, int endian);
static int dpi_hw_reset();
static int dpi_set_clock_freg(int Device, int OutFreqMHz);
static void dpi_set_event(jpu_sim_context_t *ctx);
static unsigned int dpi_get_time();
#endif /* CNM_SIM_DPI_INTERFACE */
#endif /* CNM_SIM_PLATFORM */

#if defined(CNM_FPGA_PLATFORM) || defined(CNM_SIM_PLATFORM)
static int io_lock(void);
static int io_unlock(void);
#endif

/***********************************************************************************
*
***********************************************************************************/

#if   defined(CNM_SIM_PLATFORM)
#   define JPU_REG_BASE                    (0x00)
#   define JDI_DRAM_PHYSICAL_BASE          0x00
#   define JDI_DRAM_PHYSICAL_SIZE          (4LL*1024*1024*1024)
#   define JDI_SYSTEM_ENDIAN               JDI_LITTLE_ENDIAN
#else /* CNM_SIM_PLATFORM */
#define JDI_DRAM_PHYSICAL_BASE          0x00
#define JDI_DRAM_PHYSICAL_SIZE          (4*1024*1024*1024)
#define JDI_SYSTEM_ENDIAN               JDI_LITTLE_ENDIAN
#endif /* CNM_FPGA_PLATFORM */
#define JPU_REG_SIZE                    0x300
#define JDI_NUM_LOCK_HANDLES            4

typedef struct jpudrv_buffer_pool_t
{
    jpudrv_buffer_t jdb;
    BOOL            inuse;
} jpudrv_buffer_pool_t;

typedef struct  {
    Int32                   jpu_fd;
    jpu_instance_pool_t*    pjip;
    Int32                   task_num;
    Int32                   clock_state;
#ifdef CNM_SIM_PLATFORM
#else
    jpudrv_buffer_t         jdb_register;
#endif
    jpudrv_buffer_pool_t    jpu_buffer_pool[MAX_JPU_BUFFER_POOL];
    Int32                   jpu_buffer_pool_count;
    void*                   jpu_mutex;
} jdi_info_t;

static jdi_info_t s_jdi_info = {0};

static Int32 swap_endian(BYTE* data, size_t len, Uint32 endian);


int jdi_probe()
{
    int ret;

    ret = jdi_init();
    jdi_release();

    return ret;
}

/* @return number of tasks.
 */
int jdi_get_task_num()
{
    jdi_info_t *jdi;

    jdi = &s_jdi_info;

    if (jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) {
        return 0;
    }

    return jdi->task_num;
}

void jdi_delay_us(unsigned int us)
{
#ifdef CNM_SIM_PLATFORM
    sv_delay_ns((long int)us*1000);
#else
    usleep(us);
#endif
}
int jdi_init(Uint32 coreIdx)
{
    jdi_info_t *jdi;
    int i;
    char name[128] = {0};

    jdi = &s_jdi_info;

    if (jdi->jpu_fd != -1 && jdi->jpu_fd != 0x00)
    {
        jdi_lock();
        jdi->task_num++;
        jdi_unlock();
        return 0;
    }

#ifdef ANDROID
    system("/system/lib/modules/load_android.sh");
#endif /* ANDROID */

#ifdef CNM_SIM_PLATFORM
    jdi->jpu_fd = 1; //open("/dev/null", O_RDWR);
#else
    snprintf(name, sizeof(name), "/dev/jpu%d", coreIdx);
    jdi->jpu_fd = open(name, O_RDWR);
#endif
    if (jdi->jpu_fd < 0) {
        JLOG(ERR, "[JDI] Can't open %s[error=%s]\n",name,  strerror(errno));
        return -1;
    }

    memset(jdi->jpu_buffer_pool, 0x00, sizeof(jpudrv_buffer_pool_t)*MAX_JPU_BUFFER_POOL);

    if (!jdi_get_instance_pool()) {
        JLOG(ERR, "[JDI] fail to create instance pool for saving context \n");
        goto ERR_JDI_INIT;
    }

    if (jdi->pjip->instance_pool_inited == FALSE) {
        Uint32* pCodecInst;
#ifdef CNM_SIM_PLATFORM
        pthread_mutex_init((MUTEX_HANDLE *)jdi->jpu_mutex, NULL);
#else
        pthread_mutexattr_t mutexattr;
        pthread_mutexattr_init(&mutexattr);
        pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
#if defined(ANDROID) || !defined(PTHREAD_MUTEX_ROBUST_NP)
#else
         /* If a process or a thread is terminated abnormally,
         * pthread_mutexattr_setrobust_np(attr, PTHREAD_MUTEX_ROBUST_NP) makes
         * next onwer call pthread_mutex_lock() without deadlock.
         */
        pthread_mutexattr_setrobust_np(&mutexattr, PTHREAD_MUTEX_ROBUST_NP);
#endif
        pthread_mutex_init((MUTEX_HANDLE *)jdi->jpu_mutex, &mutexattr);
#endif

        for( i = 0; i < MAX_NUM_INSTANCE; i++) {
            pCodecInst    = (Uint32*)jdi->pjip->jpgInstPool[i];
            pCodecInst[1] = i;    // indicate instIndex of CodecInst
            pCodecInst[0] = 0;    // indicate inUse of CodecInst
        }
#ifdef CNM_SIM_PLATFORM
        memset(&jdi->pjip->vmem, 0x00, sizeof(jpeg_mm_t));
        if (jmem_init(&jdi->pjip->vmem, (unsigned long)JDI_DRAM_PHYSICAL_BASE, JDI_DRAM_PHYSICAL_SIZE) < 0)
        {
            JLOG(ERR, "[JDI] fail to init jpu memory management logic\n");
            goto ERR_JDI_INIT;
        }
        JLOG(INFO, "[jmem_init] vmem:%p, phys_addr: 0x%x, size: 0x%lx\n", &jdi->pjip->vmem, (unsigned long)JDI_DRAM_PHYSICAL_BASE, JDI_DRAM_PHYSICAL_SIZE);
#endif
        jdi->pjip->instance_pool_inited = TRUE;
    }
#ifdef CNM_SIM_PLATFORM
#else
    if (ioctl(jdi->jpu_fd, JDI_IOCTL_GET_REGISTER_INFO, &jdi->jdb_register) < 0)
    {
        JLOG(ERR, "[JDI] fail to get host interface register\n");
        goto ERR_JDI_INIT;
    }
    jdi->jdb_register.virt_addr = (unsigned long)mmap(NULL, jdi->jdb_register.size, PROT_READ | PROT_WRITE, MAP_SHARED, jdi->jpu_fd, jdi->jdb_register.phys_addr);
    if (jdi->jdb_register.virt_addr == (unsigned long)MAP_FAILED) {
        JLOG(ERR, "[JDI] fail to map jpu registers \n");
        goto ERR_JDI_INIT;
    }
    JLOG(INFO, "[JDI] map jdb_register virtaddr=0x%lx, size=%d\n", jdi->jdb_register.virt_addr, jdi->jdb_register.size);
#endif
    jdi_set_clock_gate(1);


#ifdef CNM_SIM_PLATFORM
    memset(s_io_mutex, 0x00, sizeof(void *));
#ifdef CNM_SIM_DPI_INTERFACE
    dpi_init(JDI_DRAM_PHYSICAL_BASE);
#endif /* CNM_SIM_DPI_INTERFACE */
#endif /* CNM_SIM_PLATFORM */

    if (jdi_lock() < 0)
    {
        JLOG(ERR, "[JDI] fail to handle lock function\n");
        goto ERR_JDI_INIT;
    }

    jdi->task_num++;
    jdi_unlock();

    JLOG(INFO, "[JDI] success to init driver \n");
    return 0;

ERR_JDI_INIT:
    jdi_unlock();
    jdi_release();
    return -1;
}

int jdi_release()
{
    jdi_info_t *jdi;

    jdi = &s_jdi_info;
    if (!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) {
        return 0;
    }

    if (jdi_lock() < 0) {
        JLOG(ERR, "[JDI] fail to handle lock function\n");
        return -1;
    }

    if (jdi->task_num == 0) {
        JLOG(ERR, "[JDI] %s:%d task_num is 0\n", __FUNCTION__, __LINE__);
        jdi_unlock();
        return 0;
    }

    jdi->task_num--;
    if (jdi->task_num > 0) {// means that the opened instance remains
        jdi_unlock();
        return 0;
    }
#ifdef CNM_SIM_PLATFORM
    jmem_exit(&jdi->pjip->vmem);
#endif
#ifdef CNM_SIM_PLATFORM
#else
    if (jdi->jdb_register.virt_addr) {
        if (munmap((void *)jdi->jdb_register.virt_addr, jdi->jdb_register.size) < 0) { //lint !e511
            JLOG(ERR, "%s:%d failed to munmap\n", __FUNCTION__, __LINE__);
        }
    }

    memset(&jdi->jdb_register, 0x00, sizeof(jpudrv_buffer_t));
#endif
    jdi_unlock();
    if (jdi->jpu_fd != -1 && jdi->jpu_fd != 0x00) {
        if (jdi->pjip != NULL) {
#ifdef CNM_SIM_PLATFORM
            free(jdi->pjip);
            jdi->pjip = NULL;
#else
            if (munmap((void*)jdi->pjip, JDI_INSTANCE_POOL_TOTAL_SIZE) < 0) {
                JLOG(ERR, "%s:%d failed to munmap\n", __FUNCTION__, __LINE__);
            }
#endif
        }
#ifdef CNM_SIM_PLATFORM
#   ifdef CNM_SIM_DPI_INTERFACE
        dpi_release();
#   endif /* CNM_SIM_DPI_INTERFACE */
#endif /* CNM_SIM_PLATFORM */
#ifdef CNM_SIM_PLATFORM
#else
        close(jdi->jpu_fd);
#endif
    }

    memset(jdi, 0x00, sizeof(jdi_info_t));

    return 0;
}

jpu_instance_pool_t *jdi_get_instance_pool()
{
    jdi_info_t *jdi;
    jpudrv_buffer_t jdb = {0};

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00 )
        return NULL;

    memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));
    if (!jdi->pjip) {
        jdb.size = JDI_INSTANCE_POOL_TOTAL_SIZE;
#ifdef CNM_SIM_PLATFORM
        jdb.virt_addr = (unsigned long)(void *)malloc(jdb.size);
        if (!jdb.virt_addr) {
            JLOG(ERR, "[JDI] fail to allocate get instance pool physical space=%d\n", (int)jdb.size);
        }
        memset((void *)jdb.virt_addr, 0x00, jdb.size);
        //jdb.phys_addr = jdb.virt_addr; // don't need to phys_addr
#else
        if (ioctl(jdi->jpu_fd, JDI_IOCTL_GET_INSTANCE_POOL, &jdb) < 0) {
            JLOG(ERR, "[JDI] fail to allocate get instance pool physical space=%d\n", (int)jdb.size);
            return NULL;
        }

        jdb.virt_addr = (unsigned long)mmap(NULL, jdb.size, PROT_READ | PROT_WRITE, MAP_SHARED, jdi->jpu_fd, 0);
        if (jdb.virt_addr == (unsigned long)MAP_FAILED) {
            JLOG(ERR, "[JDI] fail to map instance pool phyaddr=0x%lx, size = %d\n", (int)jdb.phys_addr, (int)jdb.size);
            return NULL;
        }
#endif
        jdi->pjip      = (jpu_instance_pool_t *)jdb.virt_addr;//lint !e511
        //change the pointer of jpu_mutex to at end pointer of jpu_instance_pool_t to assign at allocated position.
        jdi->jpu_mutex = (void *)((unsigned long)jdi->pjip + JDI_INSTANCE_POOL_SIZE); //lint !e511

        JLOG(INFO, "[JDI] instance pool physaddr=%p, virtaddr=%p, base=%p, size=%d\n", jdb.phys_addr, jdb.virt_addr, jdb.base, jdb.size);
    }

    return (jpu_instance_pool_t *)jdi->pjip;
}

int jdi_open_instance(unsigned long inst_idx)
{
    jdi_info_t *jdi;
#ifdef CNM_SIM_PLATFORM
#else
    jpudrv_inst_info_t inst_info;
#endif

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
        return -1;

#ifdef CNM_SIM_PLATFORM
    jdi->pjip->jpu_instance_num++;
#else
    inst_info.inst_idx = inst_idx;
    if (ioctl(jdi->jpu_fd, JDI_IOCTL_OPEN_INSTANCE, &inst_info) < 0)
    {
        JLOG(ERR, "[JDI] fail to deliver open instance num inst_idx=%d\n", (int)inst_idx);
        return -1;
    }
    jdi->pjip->jpu_instance_num = inst_info.inst_open_count;
#endif

    return 0;
}

int jdi_close_instance(unsigned long inst_idx)
{
    jdi_info_t *jdi;
#ifdef CNM_SIM_PLATFORM
#else
    jpudrv_inst_info_t inst_info;
#endif

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
        return -1;

#ifdef CNM_SIM_PLATFORM
    jdi->pjip->jpu_instance_num--;
#else
    inst_info.inst_idx = inst_idx;
    if (ioctl(jdi->jpu_fd, JDI_IOCTL_CLOSE_INSTANCE, &inst_info) < 0)
    {
        JLOG(ERR, "[JDI] fail to deliver open instance num inst_idx=%d\n", (int)inst_idx);
        return -1;
    }
    jdi->pjip->jpu_instance_num = inst_info.inst_open_count;
#endif

    return 0;
}
int jdi_get_instance_num()
{
    jdi_info_t *jdi;
    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
        return -1;

    return jdi->pjip->jpu_instance_num;
}

int jdi_hw_reset()
{
    jdi_info_t *jdi;
    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
        return -1;

#if   CNM_SIM_PLATFORM
#   ifdef CNM_SIM_DPI_INTERFACE
    return dpi_hw_reset();
#   endif /* CNM_SIM_DPI_INTERFACE */
#else
    return ioctl(jdi->jpu_fd, JDI_IOCTL_RESET, 0);
#endif

}

static void restore_mutex_in_dead(MUTEX_HANDLE *mutex)
{
    int mutex_value;

    if (!mutex)
        return;
#if defined(ANDROID)
    mutex_value = mutex->value;
#else
    memcpy(&mutex_value, mutex, sizeof(mutex_value));
#endif
    if (mutex_value == (int)0xdead10cc) // destroy by device driver
    {
        pthread_mutexattr_t mutexattr;
        pthread_mutexattr_init(&mutexattr);
        pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(mutex, &mutexattr);
    }
}

int jdi_lock()
{
    jdi_info_t *jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) {
        JLOG(ERR, "%s:%d JDI handle isn't initialized\n", __FUNCTION__, __LINE__);
        return -1;
    }


#if defined(ANDROID) || !defined(PTHREAD_MUTEX_ROBUST_NP)
    restore_mutex_in_dead((MUTEX_HANDLE *)jdi->jpu_mutex);
    pthread_mutex_lock((MUTEX_HANDLE*)jdi->jpu_mutex);
#else
    if (pthread_mutex_lock(&jdi->jpu_mutex) != 0) {
        JLOG(ERR, "%s:%d failed to pthread_mutex_locK\n", __FUNCTION__, __LINE__);
        return -1;
    }
#endif

    return 0;//lint !e454
}

void jdi_unlock()
{
    jdi_info_t *jdi;

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd <= 0)
        return;

    pthread_mutex_unlock((MUTEX_HANDLE *)jdi->jpu_mutex);//lint !e455
}

void jdi_write_register(unsigned long addr, unsigned int data)
{
    jdi_info_t *jdi = &s_jdi_info;
#if defined(CNM_FPGA_PLATFORM) || defined(CNM_SIM_PLATFORM)
#else
    unsigned long *reg_addr;
#endif

    if(!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
        return;

#if defined(CNM_FPGA_PLATFORM) || defined(CNM_SIM_PLATFORM)
    if(!jdi->clock_state)
    {
        JLOG(ERR, "[JDI] jdi_write_register clock is in off. enter infinite loop\n");
        while (1);
    }
#endif

#if   CNM_SIM_PLATFORM
#   ifdef CNM_SIM_DPI_INTERFACE
    dpi_write_register(((JPU_REG_BASE+addr)), data);
#   endif /* CNM_SIM_DPI_INTERFACE */
#else
    reg_addr = (unsigned long *)(addr + (unsigned long)jdi->jdb_register.virt_addr);
    *(volatile unsigned int *)reg_addr = data;
#endif
}

unsigned long jdi_read_register(unsigned long addr)
{
    jdi_info_t *jdi;
#if defined(CNM_FPGA_PLATFORM) || defined(CNM_SIM_PLATFORM)
#else
    unsigned long *reg_addr;
#endif

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
        return (unsigned int)-1;

#if defined(CNM_FPGA_PLATFORM) || defined(CNM_SIM_PLATFORM)
    if(!jdi->clock_state)
    {
        JLOG(ERR, "[JDI] jdi_read_register clock is in off. enter infinite loop\n");
        while (1);
    }
#endif

#if   CNM_SIM_PLATFORM
#   ifdef CNM_SIM_DPI_INTERFACE
    return dpi_read_register((JPU_REG_BASE+addr));
#   endif /* CNM_SIM_DPI_INTERFACE */
#else
    reg_addr = (unsigned long *)(addr + (unsigned long)jdi->jdb_register.virt_addr);
    return *(volatile unsigned int *)reg_addr;
#endif
}

size_t jdi_write_memory(unsigned long addr, unsigned char *data, size_t len, int endian)
{
    jdi_info_t *jdi;
    jpudrv_buffer_t jdb;
    Uint32          offset;
    Uint32          i;

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd==-1 || jdi->jpu_fd == 0x00)
        return 0;

    memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));

	addr |= 0x100000000;
    for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
    {
        if (jdi->jpu_buffer_pool[i].inuse == 1)
        {
            jdb = jdi->jpu_buffer_pool[i].jdb;
            if (addr >= jdb.phys_addr && addr < (jdb.phys_addr + jdb.size)) {
                break;
            }
        }
    }

    if (i == MAX_JPU_BUFFER_POOL) {
        JLOG(ERR, "%s NOT FOUND ADDRESS: %08x\n", __FUNCTION__, addr);
        return 0;
    }

    if (!jdb.size) {
        JLOG(ERR, "address 0x%08x is not mapped address!!!\n", (int)addr);
        return 0;
    }

#if   CNM_SIM_PLATFORM
#   ifdef CNM_SIM_DPI_INTERFACE
    dpi_write_memory(addr, data, len, endian);
#   endif /* CNM_SIM_DPI_INTERFACE */
    offset = addr - (unsigned long)jdb.phys_addr;
    memcpy((void *)((unsigned long)jdb.virt_addr+offset), data, len);//lint !e511
#else
    offset = addr - (unsigned long)jdb.phys_addr;
    swap_endian(data, len, endian);
    memcpy((void *)((unsigned long)jdb.virt_addr+offset), data, len);
#endif

    return len;
}

size_t jdi_read_memory(unsigned long addr, unsigned char *data, size_t len, int endian)
{
    jdi_info_t *jdi;
    jpudrv_buffer_t jdb;
#if defined(CNM_FPGA_PLATFORM) || defined(CNM_SIM_PLATFORM)
#else
    unsigned long offset;
#endif
    int i;

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd==-1 || jdi->jpu_fd == 0x00)
        return -1;

    memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));

	addr |= 0x100000000;
    for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
    {
        if (jdi->jpu_buffer_pool[i].inuse == 1)
        {
            jdb = jdi->jpu_buffer_pool[i].jdb;
            if (addr >= jdb.phys_addr && addr < (jdb.phys_addr + jdb.size))
                break;
        }
    }

    if (len == 0) {
        return 0;
    }

    if (!jdb.size)
        return -1;



#if   CNM_SIM_PLATFORM
#   ifdef CNM_SIM_DPI_INTERFACE
    dpi_read_memory(addr, data, len, endian);
#   endif /* CNM_SIM_DPI_INTERFACE */
#else
    offset = addr - (unsigned long)jdb.phys_addr;
    memcpy(data, (const void *)((unsigned long)jdb.virt_addr+offset), len);
    swap_endian(data, len,  endian);
#endif

    return len;
}

int jdi_allocate_dma_memory(jpu_buffer_t *vb)
{
    jdi_info_t *jdi;
    int i;
    jpudrv_buffer_t jdb;

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd==-1 || jdi->jpu_fd == 0x00)
        return -1;

    memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));

    jdb.size = vb->size;
#ifdef CNM_SIM_PLATFORM
    jdi_lock();
    jdb.phys_addr = (unsigned long)jmem_alloc(&jdi->pjip->vmem, jdb.size, 0);
    jdi_unlock();
    sv_mem_alloc(jdb.phys_addr, jdb.size);
    
    if (jdb.phys_addr == (unsigned long)-1)
    {
        JLOG(ERR, "[JDI] fail to jdi_allocate_dma_memory size=%ld\n", vb->size);
        return -1; // not enough memory
    }
    JLOG(INFO, "[JDI] jdi_allocate_dma_memory addr:0x%x size=0x%lx\n", jdb.phys_addr, jdb.size);
#else
    if (ioctl(jdi->jpu_fd, JDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY, &jdb) < 0)
    {
        JLOG(ERR, "[JDI] fail to jdi_allocate_dma_memory size=%d\n", vb->size);
        return -1;
    }
#endif

    vb->phys_addr = (unsigned long)jdb.phys_addr;
    vb->base = (unsigned long)jdb.base;

#if defined(CNM_FPGA_PLATFORM) || defined(CNM_SIM_PLATFORM)
    jdb.virt_addr = (unsigned long)malloc(jdb.size);
    vb->virt_addr = (unsigned long)jdb.virt_addr;
#else
    //map to virtual address
    jdb.virt_addr = (unsigned long)mmap(NULL, jdb.size, PROT_READ | PROT_WRITE, MAP_SHARED, jdi->jpu_fd, jdb.phys_addr);
    if (jdb.virt_addr == (unsigned long)MAP_FAILED) {
        memset(vb, 0x00, sizeof(jpu_buffer_t));
        return -1;
    }
    vb->virt_addr = jdb.virt_addr;
#endif

    jdi_lock();
    for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
    {
        if (jdi->jpu_buffer_pool[i].inuse == 0)
        {
            jdi->jpu_buffer_pool[i].jdb = jdb;
            jdi->jpu_buffer_pool_count++;
            jdi->jpu_buffer_pool[i].inuse = 1;
            break;
        }
    }
    jdi_unlock();
#ifdef CNM_SIM_PLATFORM
    {
        unsigned char *p0;
        p0 = (unsigned char *)malloc(vb->size);
        if (p0)
        {
            memset(p0, 0x00, vb->size);
            jdi_write_memory(vb->phys_addr, p0, vb->size, 0);
            free(p0);
        }
    }
#endif
    JLOG(INFO, "[JDI] jdi_allocate_dma_memory, physaddr=%p, virtaddr=%p~%p, size=0x%lx\n",
         vb->phys_addr, vb->virt_addr, vb->virt_addr + vb->size, vb->size);
    return 0;
}

void jdi_free_dma_memory(jpu_buffer_t *vb)
{
    jdi_info_t *jdi;
    int i;
    jpudrv_buffer_t jdb;


    jdi = &s_jdi_info;

    if(!vb || !jdi || jdi->jpu_fd==-1 || jdi->jpu_fd == 0x00)
        return;

    if (vb->size == 0)
        return ;

    memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));

    jdi_lock();
    for (i=0; i<MAX_JPU_BUFFER_POOL; i++) {
        if (jdi->jpu_buffer_pool[i].jdb.phys_addr == vb->phys_addr) {
            jdi->jpu_buffer_pool[i].inuse = 0;
            jdi->jpu_buffer_pool_count--;
            jdb = jdi->jpu_buffer_pool[i].jdb;
            break;
        }
    }
    jdi_unlock();

    if (!jdb.size)
    {
        JLOG(ERR, "[JDI] invalid buffer to free address = 0x%lx\n", (int)jdb.virt_addr);
        return ;
    }
#ifdef CNM_SIM_PLATFORM
    jdi_lock();
    jmem_free(&jdi->pjip->vmem, (unsigned long)jdb.phys_addr, 0);
    jdi_unlock();
#else
    ioctl(jdi->jpu_fd, JDI_IOCTL_FREE_PHYSICALMEMORY, &jdb);
#endif

#if defined(CNM_FPGA_PLATFORM) || defined(CNM_SIM_PLATFORM)
    free((void*)vb->virt_addr);//lint !e511
#else
    if (munmap((void *)jdb.virt_addr, jdb.size) != 0) {
        JLOG(ERR, "[JDI] fail to jdi_free_dma_memory virtial address = 0x%lx\n", (int)jdb.virt_addr);
    }
#endif
    memset(vb, 0, sizeof(jpu_buffer_t));
}



int jdi_set_clock_gate(int enable)
{
    jdi_info_t *jdi = NULL;
    int ret;

    jdi = &s_jdi_info;
    if(!jdi || jdi->jpu_fd==-1 || jdi->jpu_fd == 0x00)
        return -1;

#if defined(CNM_FPGA_PLATFORM) || defined(CNM_SIM_PLATFORM)
    io_lock();
#endif /* CNM_FPGA_PLATFORM */
    jdi->clock_state = enable;
#ifdef CNM_SIM_PLATFORM
    ret = 0;
#else
    ret = ioctl(jdi->jpu_fd, JDI_IOCTL_SET_CLOCK_GATE, &enable);
#endif
#if defined(CNM_FPGA_PLATFORM) || defined(CNM_SIM_PLATFORM)
    io_unlock();
#endif /* CNM_FPGA_PLATFORM */

    return ret;
}

int jdi_get_clock_gate()
{
    jdi_info_t *jdi;
    int ret;

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd==-1 || jdi->jpu_fd == 0x00)
        return -1;

    ret = jdi->clock_state;

    return ret;
}

#ifdef CNM_SIM_PLATFORM
void jdi_set_event_to_sim(jpu_sim_context_t *ctx)
{
    if (!ctx)
        return;
    dpi_set_event(ctx);

}
#endif



int jdi_wait_inst_ctrl_busy(int timeout, unsigned int addr_flag_reg, unsigned int flag)
{
    Int64 elapse, cur;
    unsigned int data_flag_reg;
    int retry_count;
#ifdef CNM_SIM_PLATFORM
    elapse = dpi_get_time()/1000000;
#else
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    gettimeofday(&tv, NULL);
    elapse = tv.tv_sec*1000 + tv.tv_usec/1000;
#endif

    while(1)
    {
        data_flag_reg = jdi_read_register(addr_flag_reg);

        if (((data_flag_reg >> 4)&0xf) == flag) {
            break;
        }

#ifdef CNM_SIM_PLATFORM
        cur = dpi_get_time()/1000000;
#else
        gettimeofday(&tv, NULL);
        cur = tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif

        if (timeout > 0 && (cur - elapse) > timeout)
        {

            for (retry_count=0; retry_count<10; retry_count++) {
                JLOG(ERR, "[JDI] jdi_wait_inst_ctrl_busy timeout, 0x%x=0x%lx\n", addr_flag_reg, jdi_read_register(addr_flag_reg));
            }
            return -1;
        }
    }
    return 0;

}

#if defined(SUPPORT_WRESP_DONE_CHECKING) || defined(SUPPORT_RRESP_DONE_CHECKING)
int jdi_wait_bus_busy(int timeout, unsigned long instIdx, unsigned long reg)
{
    Int64 elapse, cur;
    Uint32 respFlag;
    struct timeval tv;
#ifdef CNM_SIM_PLATFORM
    elapse = dpi_get_time()/1000000;
#else
    int retry_count;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    gettimeofday(&tv, NULL);
    elapse = tv.tv_sec*1000 + tv.tv_usec/1000;
#endif

    while(1)
    {
        respFlag = JpuReadInstReg(instIdx, reg);

        if (respFlag == 0)
            break;
            
#ifdef CNM_SIM_PLATFORM
        cur = dpi_get_time()/1000000;
#else
        gettimeofday(&tv, NULL);
        cur = tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif

        if (timeout > 0 && (cur - elapse) > timeout)
        {
            for (retry_count=0; retry_count<10; retry_count++) {
                JLOG(ERR, "[JDI] %s timeout, inst=%d, 0x%x=0x%lx\n", __FUNCTION__, instIdx, MJPEG_WRESP_CHECK_REG, JpuReadInstReg(instIdx, MJPEG_WRESP_CHECK_REG));
            }
            return -1;
        }
    }
    return 0;
}
#endif
int jdi_wait_interrupt(int timeout, unsigned long instIdx)
{
    int intr_reason = 0;
    jdi_info_t *jdi;
#ifdef CNM_SIM_PLATFORM
    unsigned int  elapse, cur;
    Uint32 intrFlag;
    int i;
#else
    int ret;
    jpudrv_intr_info_t intr_info;
#endif


    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd <= 0)
        return -1;

#ifdef CNM_SIM_PLATFORM
    elapse = dpi_get_time()/1000000;

    while(1)
    {
        intrFlag = 0;
        for (i=0; i<MAX_NUM_REGISTER_SET; i++) {
            intrFlag = JpuReadInstReg(i, MJPEG_PIC_STATUS_REG);
            if (intrFlag != 0) {
                break;
            }
        }
        if (intrFlag != 0) {
            if (i == instIdx) {
                intr_reason = intrFlag;
                break;
            }
        }

        cur = dpi_get_time()/1000000;

        if ((cur - elapse) > timeout)
        {         
            return -1;
        }
    }
#else
    intr_info.timeout     = timeout;
    intr_info.intr_reason = 0;
    intr_info.inst_idx    = instIdx;
    ret = ioctl(jdi->jpu_fd, JDI_IOCTL_WAIT_INTERRUPT, (void*)&intr_info);
    if (ret != 0)
        return -1;
    intr_reason = intr_info.intr_reason;
#endif
    return intr_reason;
}


void jdi_log(int cmd, int step, int inst)
{
    Int32   i;

    switch(cmd)
    {
    case JDI_LOG_CMD_PICRUN:
        if (step == 1)    //
            JLOG(INFO, "\n**PIC_RUN INST=%d start\n", inst);
        else
            JLOG(INFO, "\n**PIC_RUN INST=%d  end \n", inst);
        break;
    case JDI_LOG_CMD_INIT:
        if (step == 1)    //
            JLOG(INFO, "\n**INIT INST=%d  start\n", inst);
        else
            JLOG(INFO, "\n**INIT INST=%d  end \n", inst);
        break;
    case JDI_LOG_CMD_RESET:
        if (step == 1)    //
            JLOG(INFO, "\n**RESET INST=%d  start\n", inst);
        else
            JLOG(INFO, "\n**RESET INST=%d  end \n", inst);
        break;
    case JDI_LOG_CMD_PAUSE_INST_CTRL:
        if (step == 1)    //
            JLOG(INFO, "\n**PAUSE INST_CTRL  INST=%d start\n", inst);
        else
            JLOG(INFO, "\n**PAUSE INST_CTRL  INST=%d end\n", inst);
        break;
    }

    for (i=(inst*NPT_REG_SIZE); i<=((inst*NPT_REG_SIZE)+0x250); i=i+16)
    {
        JLOG(INFO, "0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", i,
            jdi_read_register(i), jdi_read_register(i+4),
            jdi_read_register(i+8), jdi_read_register(i+0xc));
    }

    JLOG(INFO, "0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", NPT_PROC_BASE,
        jdi_read_register(NPT_PROC_BASE+0x00), jdi_read_register(NPT_PROC_BASE+4),
        jdi_read_register(NPT_PROC_BASE+8), jdi_read_register(NPT_PROC_BASE+0xc));
}


static void SwapByte(Uint8* data, size_t len)
{
    Uint8   temp;
    size_t  i;

    for (i=0; i<len; i+=2) {
        temp      = data[i];
        data[i]   = data[i+1];
        data[i+1] = temp;
    }
}

static void SwapWord(Uint8* data, size_t len)
{
    Uint16  temp;
    Uint16* ptr = (Uint16*)data;
    size_t  i, size = len/sizeof(Uint16);

    for (i=0; i<size; i+=2) {
        temp      = ptr[i];
        ptr[i]   = ptr[i+1];
        ptr[i+1] = temp;
    }
}

static void SwapDword(Uint8* data, size_t len)
{
    Uint32  temp;
    Uint32* ptr = (Uint32*)data;
    size_t  i, size = len/sizeof(Uint32);

    for (i=0; i<size; i+=2) {
        temp      = ptr[i];
        ptr[i]   = ptr[i+1];
        ptr[i+1] = temp;
    }
}

Int32 swap_endian(BYTE* data, size_t len, Uint32 endian)
{
    Uint8   endianMask[8] = {   // endianMask : [2] - 4byte unit swap
        0x00, 0x07, 0x04, 0x03, //              [1] - 2byte unit swap
        0x06, 0x05, 0x02, 0x01  //              [0] - 1byte unit swap
    };
    Uint8   targetEndian;
    Uint8   systemEndian;
    Uint8   changes;
    BOOL    byteSwap=FALSE, wordSwap=FALSE, dwordSwap=FALSE;

    if (endian > 7) {
        JLOG(ERR, "Invalid endian mode: %d, expected value: 0~7\n", endian);
        return -1;
    }

    targetEndian = endianMask[endian];
    systemEndian = endianMask[JDI_SYSTEM_ENDIAN];
    changes      = targetEndian ^ systemEndian;
    byteSwap     = changes & 0x01 ? TRUE : FALSE;
    wordSwap     = changes & 0x02 ? TRUE : FALSE;
    dwordSwap    = changes & 0x04 ? TRUE : FALSE;

    if (byteSwap == TRUE)  SwapByte(data, len);
    if (wordSwap == TRUE)  SwapWord(data, len);
    if (dwordSwap == TRUE) SwapDword(data, len);

    return changes == 0 ? 0 : 1;
}


#if defined(CNM_FPGA_PLATFORM) || defined(CNM_SIM_PLATFORM)

static unsigned long s_dram_base;
static int io_lock(
    void
    )
{
    jdi_info_t* jdi;
#ifdef CNM_SIM_PLATFORM
    int sync_ret;
    int sync_val = getpid();
    volatile int *sync_lock_ptr = NULL;
#endif
    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd <= 0)
        return -1;
#ifdef CNM_SIM_PLATFORM
    sync_lock_ptr = (volatile int *)s_io_mutex;
    // JLOG(INFO, "+%s, lock_val=%d, sync_val=%d\n", __FUNCTION__, *sync_lock_ptr, sync_val);
    while((sync_ret = __sync_val_compare_and_swap(sync_lock_ptr, 0, sync_val)) != 0)
    {

    }
    // JLOG(INFO, "-%s, lock_val=%d\n", __FUNCTION__, *sync_lock_ptr);
    return 0;
#else
    return ioctl(jdi->jpu_fd, JDI_IOCTL_IO_LOCK, NULL);
#endif
}

static int io_unlock(
    void
    )
{
    jdi_info_t* jdi;
    int         ret;
#ifdef CNM_SIM_PLATFORM
    volatile int *sync_lock_ptr = NULL;
#endif
    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd==-1 || jdi->jpu_fd == 0x00) {
        return -1;
    }
#ifdef CNM_SIM_PLATFORM
    sync_lock_ptr = (volatile int *)s_io_mutex;
    __sync_lock_release(sync_lock_ptr);
    ret = 0;
#else
    ret = ioctl(jdi->jpu_fd, JDI_IOCTL_IO_UNLOCK, NULL);
#endif
    return ret;
}

int jdi_set_clock_freg(int Device, int OutFreqMHz, int InFreqMHz )
{
    jdi_info_t *jdi;

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd==-1 || jdi->jpu_fd == 0x00)
        return -1;
#ifdef CNM_SIM_PLATFORM
#ifdef CNM_SIM_DPI_INTERFACE
    return dpi_set_clock_freg(Device, OutFreqMHz);
#endif /* CNM_SIM_DPI_INTERFACE */
#endif /* CNM_SIM_PLATFORM */
}

#endif


#ifdef CNM_SIM_PLATFORM
#ifdef CNM_SIM_DPI_INTERFACE

#define DPI_BUS_LENGTH      8

static int sv_read_memory(unsigned int addr, unsigned char *buf, size_t size)
{
    size_t i;
    int val;

    //assert (address%DPI_BUS_LENGTH == 0 && len%DPI_BUS_LENGTH == 0);  // Limited to 64-bit aligned address, 64-bit data

    for (i=0; i<size; i++)
    {
        sv_mem_read_byte(addr++, &val, 0);
        *buf++ = val & 0xff;
    }


    return 1;
}
int  sv_write_memory(unsigned int addr, unsigned char *buf, size_t size)
{
    size_t i;

    //assert (address%DPI_BUS_LENGTH == 0 && len%DPI_BUS_LENGTH == 0);  // Limited to 64-bit aligned address, 64-bit data

    for (i=0; i<size; i++)
        sv_mem_write_byte(addr++, *buf++, 0);

    return 1;
}

int dpi_init(unsigned long dram_base)
{
    s_dram_base = dram_base;
    return 1;
}
 void dpi_release(void)
{

}
unsigned int dpi_get_time()
{
    long long time_ns;
    if (io_lock() < 0)
        return -1;
    sv_get_stime(&time_ns); // this return the simulation time($stime) 1ns unit
    // JLOG(INFO, "%s ns=%d, ms=%d\n", __FUNCTION__, time_ns, (time_ns/1000000));

    io_unlock();
    return (unsigned int)time_ns;
}
void dpi_set_event(jpu_sim_context_t *ctx)
{
    if (io_lock() < 0)
        return;

    sv_set_event(ctx->frameIdx);

    io_unlock();
}
void dpi_write_register(unsigned int addr, unsigned int data)
{
    if (io_lock() < 0)
        return;

    sv_reg_write((int)addr, (int)data);

    io_unlock();
}
unsigned int dpi_read_register(unsigned int addr)
{
    int data;

    if (io_lock() < 0)
        return -1;

    sv_reg_read(&data, (int)addr);

    io_unlock();
    return data;
}
size_t dpi_write_memory(unsigned int addr, unsigned char *data, size_t len, int endian)
{

    unsigned int    next4Kaddr;
    size_t          sizeToWrite, remainSize;
    unsigned char*  pBuf;
    unsigned char   lsBuf[DPI_BUS_LENGTH];
    unsigned int     alignSize = DPI_BUS_LENGTH;
    size_t           alignMask = DPI_BUS_LENGTH-1, alignedAddr;
    unsigned int     offset;

    if (addr < s_dram_base) {
        JLOG(ERR, "[DPI-w] invalid address base address is 0x%08x\n", addr);
        return 0;
    }

    if (io_lock() < 0)
        return 0;

    if (len==0) {
        io_unlock();
        return 0;
    }

    addr = addr - s_dram_base;

    alignedAddr = addr&~alignMask;
    offset      = addr - alignedAddr;
    pBuf        = (BYTE*)malloc((len+offset+alignMask)&~alignMask);
    if (offset) {
        sv_read_memory(alignedAddr, lsBuf, (offset+alignMask)&~alignMask);
        swap_endian(lsBuf, alignSize, endian);
        memcpy(pBuf, lsBuf, offset);
    }
    addr       = alignedAddr;
    remainSize = len;
    next4Kaddr = (addr+4095)&~4095;
    if (addr != next4Kaddr && (addr+len) > next4Kaddr) {
        sizeToWrite = next4Kaddr - addr - offset;
        memcpy(pBuf+offset, data, sizeToWrite);
        swap_endian(pBuf, (sizeToWrite+offset+alignMask)&~alignMask, endian);
        //sv_mem_alloc(addr, (sizeToWrite+offset+alignMask)&~alignMask);
        sv_write_memory(addr, (unsigned char *)pBuf, (sizeToWrite+offset+alignMask)&~alignMask);

        data       += sizeToWrite;
        remainSize -= sizeToWrite;
        addr        = next4Kaddr;
        offset      = 0;
    }

    sizeToWrite = remainSize+offset;
    memcpy(pBuf+offset, data, remainSize);
    swap_endian(pBuf, (sizeToWrite+alignMask)&~alignMask, endian);

    //sv_mem_alloc(addr, (sizeToWrite+alignMask)&~alignMask);
    sv_write_memory(addr, pBuf, (sizeToWrite+alignMask)&~alignMask);

    free(pBuf);

    io_unlock();

    return len;
}
size_t dpi_read_memory(unsigned int addr, unsigned char *data, size_t len, int endian)
{
    size_t           numberOf4KBlocks;
    size_t           next4Kaddr;
    size_t           sizeToRead, remainSize;
    unsigned char*   pBuf;
    size_t           alignSize = DPI_BUS_LENGTH;
    size_t           alignMask = DPI_BUS_LENGTH-1, alignedAddr;
    size_t           offset;

    UNREFERENCED_PARAMETER(alignSize);

    if (addr < s_dram_base) {
        JLOG(ERR, "[DPI-r] invalid address base address is 0x%08x\n", addr);
        return 0;
    }

    if (io_lock() < 0)
        return 0;

    if (len==0) {
        io_unlock();
        return 0;
    }

    addr = addr - s_dram_base;

    numberOf4KBlocks = ((len+4095)&~4095)>>12;

    alignedAddr = addr&~alignMask;
    offset      = addr - alignedAddr;
    pBuf        = (BYTE*)malloc((len+offset+alignMask)&~alignMask);
    addr        = alignedAddr;
    remainSize  = len+offset;
    next4Kaddr  = (addr+4095)&~4095;
    if (addr != next4Kaddr && (addr+len) > next4Kaddr) {
        sizeToRead  = next4Kaddr - addr;
        sv_read_memory(addr, pBuf, (sizeToRead+alignMask)&~alignMask);
        swap_endian(pBuf, (sizeToRead+offset+alignMask)&~alignMask, endian);
        memcpy(data, pBuf+offset, sizeToRead-offset);

        data       += sizeToRead-offset;
        remainSize -= sizeToRead;
        addr        = next4Kaddr;
        offset      = 0;
        //numberOf4KBlocks--;
    }

    if (numberOf4KBlocks > 0) {
        sv_read_memory(addr, pBuf, (remainSize+alignMask)&~alignMask);
        swap_endian(pBuf, (remainSize+alignMask)&~alignMask, endian);
        memcpy(data, pBuf+offset, remainSize-offset);
    }

    free(pBuf);

    io_unlock();

    return len;
}
int dpi_hw_reset()
{
    return 0;
}

int dpi_set_clock_freg(int Device, int OutFreqMHz)
{
    return 0;
}
#endif
#endif /* CNM_SIM_PLATFORM */
