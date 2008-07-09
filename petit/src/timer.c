/* $Id: timer.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#if !defined(OMIT_GETRUSAGE)

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

namespace omega {

static struct rusage start_time;

void start_clock( void )
    {
    getrusage(RUSAGE_SELF, &start_time);
    }


long clock_diff( void )
    {
    struct rusage current_time;
    getrusage(RUSAGE_SELF, &current_time);
    return (current_time.ru_utime.tv_sec -start_time.ru_utime.tv_sec)*1000000 +
           (current_time.ru_utime.tv_usec-start_time.ru_utime.tv_usec);
    }

} 

#endif
