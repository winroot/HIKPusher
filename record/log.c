/*******************************
@@Author     :  Charles
@@Date       :  2017-08-15
@@Mail       :  pu17rui@sina.com
@@Description: 	logs print
*******************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <stdarg.h>
#include "log.h"
#include "config.h"

static pthread_mutex_t gLOG_out_file_mutex;
static FILE* gLOG_out_fd = NULL;

int LOG_Init(void)
{
	pthread_mutex_init(&gLOG_out_file_mutex, NULL);

	if (access(LOG_OUT_PATH, F_OK) == -1)
	{
		mkdir(LOG_OUT_PATH, S_IRWXU | S_IRWXG | S_IRWXO);
	}	

	return 0;
}
int LOG_Save(char* data, int len)
{
	char file_name[256] = {0};

	time_t now;
	struct tm  *timenow;
	time(&now);
	timenow = localtime(&now);
	// printf("Local time is %s", asctime(timenow));
	/* the file name */
	sprintf(file_name, "%sLog_%d%02d%02d", LOG_OUT_PATH, 
				timenow->tm_year + 1900, timenow->tm_mon + 1, timenow->tm_mday);
	// printf("%s\n", file_name);
	pthread_mutex_lock(&gLOG_out_file_mutex);
	gLOG_out_fd = fopen(file_name, "a+");
	fwrite(data, len, 1, gLOG_out_fd);
	fflush(gLOG_out_fd);//write immediately from sys buf
	pthread_mutex_unlock(&gLOG_out_file_mutex);

	return 0;
}
int LOG_Print(unsigned int err_n, const char* format, ...)
{
	char buf[LOG_CONTENT_MAX_SIZE * 2] = {0};
	char data[LOG_CONTENT_MAX_SIZE] = {0};
	/*get all the params in ...*/
	va_list args_tmp;
	va_start(args_tmp, format);
	vsnprintf(data, sizeof(data), format, args_tmp);
	va_end(args_tmp);

	time_t now;
	struct tm *timenow;
	time(&now);
	timenow = localtime(&now);
	/*add the header of the data*/
	int len = sprintf(buf, "[LOG %d-%02d-%02d %02d:%02d:%02d] ", 
				timenow->tm_year + 1900, timenow->tm_mon + 1, timenow->tm_mday, 
				timenow->tm_hour, timenow->tm_min, timenow->tm_sec);
	len += sprintf(buf + len, "%s", data);
	// printf("buf_len:%d,buf: %s", len, buf);
	printf("%s", buf);
	fflush(stdout);//write immediately from sys buf
#ifdef LOG_SAVE
	LOG_Save(buf, len);
#endif
	return 0;
}
int LOG_Free(void)
{
	pthread_mutex_destroy(&gLOG_out_file_mutex);
	
	if (gLOG_out_fd != NULL)
	{
		fclose(gLOG_out_fd);
		gLOG_out_fd = NULL;
	}


	return 0;
}
