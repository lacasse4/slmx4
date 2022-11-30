#ifndef _PEAK_H
#define _PEAK_H


typedef struct peak {
    int index;           // peak index in signal array
    float distance;      // peak distance
    float power;         // peak power in db
} peak_t;

void   erase_peak(peak_t* peak);
peak_t find_highest_peak(float* signal, int num_samples, float frame_start, float frame_end, float from, float to);
int    distance_to_index(float frame_start, float frame_end, int num_samples, float distance);
float  index_to_distance(float frame_start, float frame_end, int num_samples, float index);

#endif