#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(void)
{
   unsigned char tmp[4132];
   unsigned char *ext_mem;
   int fd;

   fd = open("/dev/mem", O_RDWR);
   if (fd == -1) {
      perror("failed to open /dev/mem");
      return EXIT_FAILURE;
   }

   ext_mem = mmap(NULL, 0x20000, PROT_READ|PROT_WRITE,
                                 MAP_SHARED, fd, 0xD0130000);
   if (ext_mem == MAP_FAILED) {
      perror("failed to map external memory");
      return EXIT_FAILURE;
   }

   memcpy(tmp, ext_mem+668, 4132);
   if (fwrite(tmp, 4132, 1, stdout) != 1) {
      perror("write failed");
      return EXIT_FAILURE;
   }

   if (fflush(stdout) != 0) {
      perror("write failed");
      return EXIT_FAILURE;
   }

   return EXIT_SUCCESS;
}
