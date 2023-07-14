
#ifndef _TIMER_US_H
#define _TIMER_US_H

#include <stdlib.h>
#include <sys/time.h>

struct timeval timer_us_init();
unsigned long int timer_us_elapsed(struct timeval tv);

#endif
