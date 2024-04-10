/*
 * Copyright (c) 2018, Chips&Media
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpulog.h>




#ifdef CNM_SIM_PLATFORM
extern int _main_multi(int argc, char** argv);
extern int _main_dec(int argc, char** argv);
extern int _main_enc(int argc, char** argv);
void main_sim(char* argv_file, int *suc)
{
#define MAX_TOKEN_COUNT 128
#define MAX_TOKEN_LEN 1024 
    FILE *fp;
    int ret;
    char str_line[1024];
    char *main_argv[MAX_TOKEN_COUNT];
    char *str;
    char *token;
    int argc;
    int i;
    char str_pwd[256];
    int multi_instance_test=0;

    getcwd(str_pwd, 256);

    fp = fopen(argv_file, "r");
    if (fp)
    {
        fgets(str_line, 1024, fp);
        ret = strlen(str_line);
        str_line[ret-1] = '\0';     // remove \n at the end
        fclose(fp);
    }

    JLOG(1, "main_sim called from simv. \n");
    JLOG(1, "argv = %s \n", str_line);
    JLOG(1, "pwd = %s \n", str_pwd);


    for (i=0; i <MAX_TOKEN_COUNT; i++)
    {
        main_argv[i] = (char *)malloc(MAX_TOKEN_LEN);
        if (!main_argv[i])
            goto ERROR_MAIN_SIM;
    }


    argc = 0;
    ret = 0;
    str = strdup(str_line);
    do
    {
        token = strsep(&str, " ");
        if (token && (strlen(token) > 0))
        {
            strcpy(main_argv[argc], token);
            if ( strstr(main_argv[argc], "instance-num") )
                multi_instance_test = 1;
            argc++;
        }

        if (argc > MAX_TOKEN_COUNT)
            break;
    }
    while(token != NULL);

    free(str);

    if (multi_instance_test == 1)
    {
#ifdef SUPPORT_MULTI_SIM
        ret = _main_multi(argc, main_argv);
#endif
    }
    else
    {
        if (strcmp(main_argv[0], "JPG_DEC") == 0)
        {
#ifdef SUPPORT_DECODER_SIM
            ret = _main_dec(argc, main_argv);
#endif
        }
        else
        {
#ifdef SUPPORT_ENCODER_SIM
            ret = _main_enc(argc, main_argv);
#endif
        }
    }

ERROR_MAIN_SIM:
    for (i=0; i <MAX_TOKEN_COUNT; i++)
    {
        if (main_argv[i])
            free(main_argv[i]);
    }

    if (suc)
    {
        *suc = (ret==1 ? 0 : 1);
    }
}
#endif
