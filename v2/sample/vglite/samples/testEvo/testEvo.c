/****************************************************************************
*
*    Copyright 2012 - 2019 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


#include <stdio.h>
#include <string.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "Elm.h"

// Size of fb that render into.
static int   render_width = 1920, render_height = 1080;
//char *source = NULL, *pic = NULL;
static ElmBuffer buffer;
int frames = 1; //Frames to render

void cleanup(void)
{
#if !DDRLESS
    ElmDestroyBuffer(buffer);
#endif
    ElmTerminate();
}

//int parse_args(int argc, const char * argv[])
//{
//    int i;
//    int result = 1;
//
//    for (i = 1; i < argc; i++)
//    {
//        if (i == argc - 1)
//        {
//            result = 0;
//            break;
//        }
//
//        if (argv[i][0] == '-')
//        {
//            switch (argv[i][1]) {
//                case 'w':
//                    render_width = atoi(argv[++i]);
//                    break;
//
//                case 'h':
//                    render_height = atoi(argv[++i]);
//                    break;
//
//                case 'f':
//                    frames = atoi(argv[++i]);
//                    break;
//
//                case 's':
//                    source = (char *)(argv[++i]);
//                    break;
//
//                case 'p':
//                    pic = (char *)(argv[++i]);
//                    break;
//
//                default:
//                    result = 0;
//                    break;
//            }
//
//            // Invalid input parameters.
//            if (result == 0)
//            {
//                break;
//            }
//        }
//        else
//        {
//            // Invalid input parameters.
//            result = 0;
//            break;
//        }
//    }
//
//    return result;
//}

int main(int argc, const char * argv[])
{
    uint32_t feature_check = 0;
    char casename[20];
    int status = 0;
    int fcount = 0;
    ElmHandle evo_handle = ELM_NULL_HANDLE;

    //printf("Usage: -w <width> -h <height> -f <count> -s <resource file> -p <picture name>\n");

    //if (0 == parse_args(argc, argv)) {
    //    printf("Invalid arguments.\n");
    //}


#if DDRLESS
    status = ElmInitialize(render_width, render_height);
#else
    status = ElmInitialize(256,256);
#endif
    if (!status)
    {
        printf("Elm init failed:");
        cleanup();
        return -1;
    }

#if DDRLESS_FPGA
    feature_check = vg_lite_query_feature(gcFEATURE_BIT_VG_RENDER_BY_MESH);
    if (feature_check == 0) {
        printf("render by mesh is not supported.\n");
        return 0;
    }
#endif

    printf("Render size: %d x %d\n", render_width, render_height);

#if !DDRLESS
    buffer = ElmCreateBuffer(640, 380, VG_LITE_RGBA8888);
    ElmClear(buffer, 0xFFFFFFFF, 0, 0, 0, 0, 1);
#else
   buffer = NULL;
#endif

    evo_handle = ElmCreateObjectFromFile(ELM_OBJECT_TYPE_EGO, "./svg/5.evo");

    while (frames > 0)
    {
        status = ElmDraw(buffer, evo_handle);
        if (!status) {
            printf("ElmDraw() failed:");
            cleanup();
            return -1;
        }
        ElmFinish();
        frames--;
        printf("frame %d done\n", fcount++);
    }


#if !DDRLESS
    // Save PNG file.
    sprintf(casename, "./svg/5.png");
    ElmSaveBuffer(buffer, casename);
    ElmDestroyBuffer(buffer);
#endif
    ElmDestroyObject(evo_handle);
    ElmTerminate();
    return 0;
}
