
#ifndef SRC_INCLUDES_H_
#define SRC_INCLUDES_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdbool.h>
#include <getopt.h>
#include <errno.h>

#ifdef __XMK__
#include "xmk.h"
#include "xutil.h"
#include "pthread.h"
typedef pthread_mutex_t mutex_t;

#else
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef void *mutex_t;
#endif

#define TRUE  1
#define FALSE 0

typedef int hdmi_tx_dev_t;

typedef enum {
	SNPS_ERROR = 0,
	SNPS_WARN,
	SNPS_NOTICE,
	SNPS_DEBUG,
	SNPS_TRACE,
	SNPS_ASSERT
} log_t;


#define LOGGER(level, format, ...)  \
do { printf(format, ##__VA_ARGS__); } while (0)

#endif /* SRC_INCLUDES_H_ */
