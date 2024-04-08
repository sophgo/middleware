#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

static inline void write32(void *ptr, uint32_t val)
{
   *(volatile uint32_t *)ptr = val;
}

static inline uint32_t read32(void *ptr)
{
   return *(volatile uint32_t *)ptr;
}

void dumpreg(void *regs, unsigned byte_offset, const char *name)
{
   unsigned char *p = regs;
   unsigned long val = read32(&p[byte_offset]);

   if (name) {
      printf("[%.2x] = %.8lx (%s)\n", byte_offset, val, name);
   } else {
      printf("[%.2x] = %.8lx\n", byte_offset, val);
   }
}

int main(void)
{
   uint32_t *hpi_regs;
   int fd;

   fd = open("/dev/mem", O_RDWR);
   if (fd == -1) {
      perror("failed to open /dev/mem");
      return EXIT_FAILURE;
   }

   hpi_regs = mmap(NULL, 0x1000, PROT_READ|PROT_WRITE,
                                 MAP_SHARED, fd, 0xD0072000);
   if (hpi_regs == MAP_FAILED) {
      perror("failed to map HPI registers");
      return EXIT_FAILURE;
   }

   dumpreg(hpi_regs, 0x00, "DESIGN_ID");
   dumpreg(hpi_regs, 0x04, "PRODUCT_ID");
   dumpreg(hpi_regs, 0x08, "FW_INFO");
   dumpreg(hpi_regs, 0x18, "AE_2HP_MB");
   dumpreg(hpi_regs, 0x1c, "AE_2HP_MB_STAT");
   dumpreg(hpi_regs, 0x20, "HP_IRQ_EN");
   dumpreg(hpi_regs, 0x24, "HP_IRQ_STAT");
   dumpreg(hpi_regs, 0x28, "HP_2AE_MB");
   dumpreg(hpi_regs, 0x2c, "HP_2AE_MB_STAT");
   dumpreg(hpi_regs, 0x40, "HP_FW_BASE");
   dumpreg(hpi_regs, 0x48, "HP_DATA_BASE");
   dumpreg(hpi_regs, 0x50, "HP_2AE_MB_P0");
   dumpreg(hpi_regs, 0x54, "HP_2AE_MB_P1");
   dumpreg(hpi_regs, 0x58, "AE_2HP_MB_P0");
   dumpreg(hpi_regs, 0x60, "AE_ERR_STAT");
   dumpreg(hpi_regs, 0x70, "CFG_INFO0");
   dumpreg(hpi_regs, 0x74, "CFG_INFO1");

   return 0;
}
