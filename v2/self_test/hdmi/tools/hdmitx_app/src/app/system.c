// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "app/system.h"
#include "platform.h"

char * error_level_string(log_t level)
{
	switch(level){
		case SNPS_EMERG:  return "SNPS_EMERG";
		case SNPS_ALERT:  return "SNPS_ALERT";
		case SNPS_CRITICAL: return "SNPS_CRITICAL";
		case SNPS_ERROR:  return  "\033[1;31mSNPS_ERROR\033[0m";
		case SNPS_WARN:   return "SNPS_WARN";
		case SNPS_NOTICE: return "SNPS_NOTICE";
		case SNPS_INFO:   return "SNPS_INFO";
		case SNPS_DEBUG:  return "SNPS_DEBUG";
		case SNPS_TRACE:  return "SNPS_TRACE";
		case SNPS_REPORT:  return "";
	}
	return "SNPS_UNKWN";
}


/**
 * System Functions
 */
int hdmitx_logger(int level, const char *format, ...)
{
	struct hdmi_tx_app *p_app = get_platform();
	va_list arg;
	char temp[500];
	time_t ltime;
	ltime=time(NULL);
	int str_end = 0;

	if(p_app->verbose < level)
		return 0;

	// Write time stamp
	temp[0] = '\0';
	snprintf(temp, 256, "[%s", asctime(localtime(&ltime)));
	temp[strlen(temp)-1] = ']';
	strcat(temp, " - ");

	//Log Time only to file
	if(p_app->hdmi_tx_log_file)
		write(p_app->hdmi_tx_log_file, temp, strlen(temp));

	temp[0] = '\0';

	strcat(temp, error_level_string((log_t) level));
	strcat(temp, " > ");


	if((level < SNPS_WARN) && (level <= p_app->verbose))
		printf("%s", temp);

	if(p_app->hdmi_tx_log_file)
		write(p_app->hdmi_tx_log_file, temp, strlen(temp));

	// Write message
	temp[0] = '\0';
	va_start(arg, format);
	vsnprintf(temp, 256, format, arg);
	va_end(arg);
	str_end = strlen(temp);
	temp[str_end] = '\n';
	temp[str_end+1] = '\0';

	if(p_app->hdmi_tx_log_file)
		write(p_app->hdmi_tx_log_file, temp, strlen(temp));

	if(level <= p_app->verbose)
		printf("%s", temp);
	return 0;
}


int hdmitx_log(int filedes, int level, const char *format, ...)
{
	struct hdmi_tx_app *p_app = get_platform();
	va_list arg;
	char temp[500] = {0,0,};
	time_t ltime;
	ltime=time(NULL);
	int str_end = 0;

	if(filedes < 0)
		return -1;

	if(p_app->verbose < level ) // Check this - file 0 is valid
		return 0; //nothing to log

	// Write time stamp
	snprintf(temp, 256, "[%s", asctime(localtime(&ltime)));
	temp[strlen(temp)-1] = ']';
	strcat(temp, " ");
	//Log Time only to file
	write(filedes, temp, strlen(temp));

	temp[0] = '\0';

	strcat(temp, error_level_string((log_t) level));

	write(filedes, temp, strlen(temp));

	// Write message
	temp[0] = '\0';
	va_start(arg, format);
	vsnprintf(temp, 256, format, arg);
	va_end(arg);

	str_end = strlen(temp);
	temp[str_end] = '\n';
	temp[str_end+1] = '\0';

	write(filedes, temp, strlen(temp));

	return 0;
}

void hdmitx_sleep(int ms){
	usleep(ms);
}
