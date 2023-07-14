#include "timer_us.h"

struct timeval timer_us_init()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv;
}

unsigned long int timer_us_elapsed(struct timeval previous_time)
{
    struct timeval current_time;
    int sec;
    int usec;

    // Get current time
    gettimeofday(&current_time, NULL);

    // Compute the number of seconds and microseconds elapsed since last call
    sec = current_time.tv_sec - previous_time.tv_sec;
    usec = current_time.tv_usec - previous_time.tv_usec;

    // If the previous usec is higher than the current one
    if (usec < 0)
    {
        // Recompute the microseonds and substract one second
        usec = 1000000 - previous_time.tv_usec + current_time.tv_usec;
        sec--;
    }

    // Return the elapsed time in microseconds
    return sec * 1000000 + usec;
}
