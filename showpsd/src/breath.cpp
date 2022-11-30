/*
 *  "In adults, the normal respiratory rate is roughly 12 to 20 breaths per minute."
 *  from https://www.healthline.com/health/normal-respiratory-rate
 */



#include "breath.h"

breath::breath 
{
    flush();
}

int breath::add(float frequency) 
{
    unsigned long int elapsed_time;
    if (!ready) {
        timer.initTimer();
        elapsed_time = timer.elapsedTime_ms();
    }
}


float breath::get_frequency()
{
    return 0.0;
}

void breath::flush() 
{
    int i;
    ready = 0;
    start = 0;
    count = 0;
    for (i = 0; i < MAX_SAMPLES; i++) {
        dist[i] = 0.0;
    }
}
