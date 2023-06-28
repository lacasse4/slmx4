#ifndef _PEAK_H
#define _PEAK_H


typedef struct peak {
    int   position;         // peak index in signal array (-1 if peak was not found)
    float value;            // signal value where its peak was found
    float precise_position; // peak index with increased precision
    float precise_value;    // signal value with increased precision
} peak_t;

void   erase_peak(peak_t* peak);
peak_t find_peak(float* signal, int num_samples, int start_index, int stop_index);
peak_t find_peak_precise(float* signal, int num_samples, int start_index, int stop_index);
peak_t find_peak_with_unit(float* signal, int num_samples, float from, float to, float samples_per_unit);

// int    distance_to_index(float frame_start, float frame_end, int num_samples, float distance);
// float  index_to_distance(float frame_start, float frame_end, int num_samples, float index);
// peak_t find_first_peak_above(float* signal, int num_samples, float frame_start, float frame_end, float from, float to, float above);

#endif