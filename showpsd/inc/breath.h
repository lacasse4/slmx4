#ifndef SLMX4_VCOM_H
#define SLMX4_VCOM_H

#include "serialib.h"

#define MAX_SAMPLES 100

class breath
{
    timeOut timer;
    float frequency;
    int   start;
    int   count;
    int   ready;
    unsigned long int start_time;

    float dist[MAX_SAMPLES];
    unsigned long int 
    void flush();

public:
    int   add(float distance);
    float get_frequency();
};


#endif