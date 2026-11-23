#ifndef DF_TIMER_H
#define DF_TIMER_H
#ifdef _WIN32
#include <windows.h>
static inline double df_now(void) {
    LARGE_INTEGER f,c;
    if(!QueryPerformanceFrequency(&f) || !QueryPerformanceCounter(&c) || f.QuadPart<=0) return -1;
    return (double)c.QuadPart/(double)f.QuadPart;
}
#define DF_CLOCK "QueryPerformanceCounter"
#else
#include <time.h>
static inline double df_now(void) {
    struct timespec ts;
    if(clock_gettime(CLOCK_MONOTONIC,&ts)) return -1;
    return (double)ts.tv_sec+(double)ts.tv_nsec/1e9;
}
#define DF_CLOCK "clock_gettime(CLOCK_MONOTONIC)"
#endif
#endif
