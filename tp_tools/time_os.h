#ifndef _TIME_OS_H_
#define _TIME_OS_H_



#ifdef __MACH__
#include <mach/mach_time.h>


#define CLOCK_REALTIME   0
#define CLOCK_MONOTONIC  0


static int clock_gettime(int clk_id, struct timespec *t)
{
    mach_timebase_info_data_t timebase;
    uint64_t  time;
    uint64_t  nseconds, seconds;

    mach_timebase_info(&timebase);
    time = mach_absolute_time();

    nseconds = (time * timebase.numer) / timebase.denom;
    seconds  = nseconds / 1000000000;
    nseconds = nseconds - seconds * 1000000000;

    t->tv_sec  = seconds;
    t->tv_nsec = nseconds;

#if 0
    printf("clock_gettime()\n");
    printf("  mach_absolute_time: %lld\n", time);
    printf("  timebase.numer: %d, timebase.denom: %d\n", timebase.numer, timebase.denom);
    printf("  tv_sec: %ld, tv_nsec: %ld\n", t->tv_sec, t->tv_nsec);
#endif

    return 0;
}

#else   // __MACH__

#include <time.h>

#endif  // __MACH__



#endif //  _TIME_OS_H_
