#ifndef _H_TIMER
#define _H_TIMER

#include <stdio.h>
#include <sys/time.h>

/* ‡@OS‚ªŠÇ—‚·‚éŽž‚ðŽæ“¾ */
inline
unsigned long long gettimeval(void) {
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return ((unsigned long long)tv.tv_sec)*1000000+tv.tv_usec;
}

/* ‡AŽžŠÔŒv‘ª‚ÌŠJŽn */
inline
void start_timer(unsigned int *startt) {
    *startt = (unsigned int)gettimeval();
    return;
}

/* ‡BŽžŠÔŒv‘ª‚ÌI—¹ */
inline
unsigned int stop_timer(unsigned int *startt) {
    unsigned int stopt = (unsigned int)gettimeval();
    return (stopt>=*startt)?(stopt-*startt):(stopt);
}

/* ‡CŽžŠÔŒv‘ªŒ‹‰Ê‚Ì•\Ž¦ */
#define print_timer(te) {printf("time of %s:%f[msec]\n", #te, te*1.0e-3);}

#endif /*_H_TIMER*/
