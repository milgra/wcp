#ifndef _ZC_TIME_H
#define _ZC_TIME_H

void zc_time(char* id);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_log.c"
#include <stdio.h>
#include <sys/time.h>

struct timeval zc_time_stamp;

void zc_time(char* id)
{
    if (id)
    {
	struct timeval ts;
	gettimeofday(&ts, NULL);
	zc_log_info("%s DURATION is %lu us", id, (ts.tv_sec - zc_time_stamp.tv_sec) * 1000000 + ts.tv_usec - zc_time_stamp.tv_usec);
	zc_time_stamp = ts;
    }
    else
    {
	gettimeofday(&zc_time_stamp, NULL);
    }
}

#endif
