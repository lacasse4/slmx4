#ifndef FILTERS_H
#define FILTERS_H

#include <stdio.h> 
#include <stdlib.h> 

#define NONE 0
#define LOWPASS 1
#define HIGHPASS 2

typedef  struct{
    float output;
    float input[2];
    float gain;
    int type;
}filter_data_t;

typedef  struct{
    int size;
    filter_data_t* filter_array;
}filter_data_2d_t;

float updateFilter(float input, filter_data_t* filter_data);

filter_data_t* init_filter(int type, float gain);

filter_data_2d_t* init_2d_filter(int size);
filter_data_2d_t* init_2d_filter(int size,int type, float gain);

int destroy_2d_filter(filter_data_2d_t* filter_data);

int set2dFilterParam(int type, float gain, filter_data_2d_t* filter_data);

int update2dFilter(float* input, float* output, filter_data_2d_t* filter_data);


#endif