// ------------------------------------------------------------------------
//
//                (C) COPYRIGHT 2013-2016 SYNOPSYS, INC.
//                          ALL RIGHTS RESERVED
//
//  This software and the associated documentation are confidential and
//  proprietary to Synopsys, Inc.  Your use or disclosure of this software
//  is subject to the terms and conditions of a written license agreement
//  between you, or your company, and Synopsys, Inc.
//
//  The entire notice above must be reproduced on all authorized copies.
//
// ------------------------------------------------------------------------
//
//  Project:
//
//   ESM
//
//  Description:
//
//   Decoding tool for ESM log dumps.
//
// ------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define ESM_LOG_TOOL_VERSION 0x00010000

#define START_BYTE       0xFF
#define STOP_BYTE        0xFE
#define MESG_TYPE_LOG    0xFD
#define MESG_TYPE_TABLE  0xFC
#define DATA_PARTITION   0x1B
#define FMT_NUMBER       0xF0
#define FMT_NUMBER_HEX   0xF1

#define GET() get_byte()
#define END   end() //(feof(fptr)!=0)

static const char usage_msg[] =
      "\nLOG VIEWER APPLICATION, VERSION: 0x%.8x\n"\
      "usage: %s [-qh?] --in ...\n"\
      "------------------------------------------------------------\n"\
      "-i  --in      file  :MUST specify LOG input file \n"\
      "------------------------------------------------------------\n"\
      "-h  --help\n";

FILE *fptr;
uint16_t mesg_len;
uint32_t report_log_errors = 0;
uint32_t off;
uint32_t start_pos;
uint8_t rollover;

static uint8_t get_byte()
{
   uint8_t byte;

   if (feof(fptr)!=0)
   {
      /* back to the beginning */
      rewind(fptr);
      off = 0;
      rollover = 1;
   }

   fread(&byte, 1, 1, fptr);
   mesg_len++;
   off++;

   return byte;
}

static uint8_t end()
{
   if (rollover == 1)
   {
      if (off >= start_pos)
      {
         return 1;
      }
   }

   return 0;
}

/* Render log with protocol:
 * [START (1 BYTE)] + [MESSAGE TYPE (1 BYTE]] + [LENGTH (2 BYTES)] + [MESSAGE] + [STOP (1 BYTE)]
 */
static void render_log(void)
{
   unsigned long num, id;
   unsigned line, val, x, lineno;
   char fname[256];
   uint8_t mesg_type;
   uint16_t mesg_len_read;
   uint16_t mesg_len_capture;
   uint32_t i;

   off       = 0;
   start_pos = 0;
   rollover  = 0;

   /* Scan for new log entries until we wrap. */
   for (;;) {
      while (GET() != START_BYTE)
      {
         if (END)
         {
            return;
         }
      }

      if (start_pos == 0)
      {
         /* Capture how many bytes into the starting process */
         start_pos = off;
      }

      mesg_type = GET();

      mesg_len_read = GET();
      mesg_len_read <<= 8;
      mesg_len_read += GET();

      mesg_len = 0;

      id = GET() << 8;
      id |= GET();

      if (mesg_type == MESG_TYPE_LOG)
      {
         printf("LOG ENTRY ID=%05lu, ", id);

         printf("file = [");
         x = 0;
         while ((val = GET()) && (!END)) { putchar(val); fname[x++] = val; }
         fname[x] = 0;
         printf("], ");
         lineno = GET() << 8;
         lineno |= GET();
         printf("LINE = %u, ", lineno);

         while (((val = GET()) != STOP_BYTE) && (!END)) {
            if (val == FMT_NUMBER) {
               num = GET() << 24; num |= GET() << 16; num |= GET() << 8; num |= GET(); printf("%lu", num);
            } else if (val == FMT_NUMBER_HEX) {
               num = GET() << 24; num |= GET() << 16; num |= GET() << 8; num |= GET(); printf("%lx", num);
            } else if (val == '\n') {
               ++line;
               //printf("\nLOG ENTRY ID=%05lu, LINE=%04u, file = [%s], line = %u, ", id, line, fname, lineno);
            } else {
               putchar(val);
            }
         }
         putchar('\n');
      }
      else if (mesg_type == MESG_TYPE_TABLE)
      {
         uint32_t data_size;

         while (((val = GET()) != DATA_PARTITION) && (!END))
         {
            putchar(val);
         }

         printf(":\n");

         data_size = mesg_len_read - mesg_len;

         i = 0;
         while(i < data_size)
         {
            val = GET();

            printf("%.2x ", (uint32_t)val);

            if ((i >0) && (((i + 1)%16) == 0))
            {
               putchar('\n');
            }

            i++;
         }
         putchar('\n');

         if ((val = GET()) != STOP_BYTE)
         {
            fprintf(stderr, "[LOG PARSE ERROR]: Expected STOP_BYTE but read %c\n", val);
            report_log_errors++;
         }
      }

      mesg_len_capture = mesg_len-1;

      if (mesg_len_read != mesg_len_capture)
      {
         fprintf(stderr, "[LOG PARSE ERROR]: message size does not match log capture [%d / %d]\n",
               mesg_len_read, mesg_len_capture);
         report_log_errors++;
      }
   }
}

int main(int argc, char **argv)
{
   char *ip_file;
   int32_t c;

   if (argc < 2)
   {
      fprintf(stderr, usage_msg, ESM_LOG_TOOL_VERSION, argv[0]);
      return EXIT_FAILURE;
   }

   while ((c = getopt(argc, argv, "?hi:"))!= -1)
   {
      switch (c)
      {
         case '?':
         case 'h':
            fprintf(stderr, usage_msg, ESM_LOG_TOOL_VERSION, argv[0]);
            break;
         case 'i':
            ip_file = optarg;
            break;
         default:
            fprintf(stderr, "Bad input command %c\n", c);
            return EXIT_FAILURE;
      }
   }

   if (ip_file == 0)
   {
      fprintf(stderr, "Error: Must supply an INPUT configuration file\n");
      return EXIT_FAILURE;
   }

   if ((fptr = fopen(ip_file, "r")) == NULL)
   {
      fprintf(stderr, "Error: Unable to open file [%s]\n", ip_file);
      return EXIT_FAILURE;
   }

   report_log_errors = 0;
   render_log();

   fclose(fptr);

   printf("REPORTED LOG ERRORS: %d\n", report_log_errors);

   return 0;
}
