#ifndef __VO_UT_H__
#define __VO_UT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>

#include "sample_comm.h"
#include "vo_ioctl.h"

CVI_S32 vo_ioctl_test(int fd, int vodev);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __VO_UT_H__ */
