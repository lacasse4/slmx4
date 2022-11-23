#include "filters.h"

float updateFilter(float input, filter_data_t* filter_data){
    filter_data->input[1] = filter_data->input[0];
    filter_data->input[0] = input;
    switch(filter_data->type){
        case NONE:
            filter_data->output = filter_data->input[0];
            break;

        case LOWPASS:
            filter_data->output = filter_data->gain*(filter_data->input[0] - filter_data->output) + filter_data->output;
            break;

        case HIGHPASS:
            filter_data->output = (1-filter_data->gain)*(filter_data->input[0] - filter_data->input[1]) + filter_data->gain*filter_data->output;
            break;

        default:
            fprintf(stderr, "ERROR : Undefined filter Type %s (%d)\n", __FILE__, __LINE__);
        break;
    }
    return(filter_data->output);
}

filter_data_t* init_filter(int type, float gain){
    filter_data_t* filt_dat = (filter_data_t*)malloc(sizeof(filter_data_t));
    filt_dat->gain = gain;
    filt_dat->type = type;
    return(filt_dat);
}

filter_data_2d_t* init_2d_filter(int size){
    filter_data_2d_t* filter_data = (filter_data_2d_t*)malloc(sizeof(filter_data_2d_t));
    filter_data->filter_array = (filter_data_t*)malloc(size*sizeof(filter_data_t));
    filter_data->size = size;
    return(filter_data);
}

filter_data_2d_t* init_2d_filter(int size,int type, float gain){
    filter_data_2d_t* filter_data = init_2d_filter(size);
    set2dFilterParam(type, gain, filter_data);
    return(filter_data);
}

int destroy_2d_filter(filter_data_2d_t* filter_data){
    if(filter_data != NULL){
        free(filter_data->filter_array);
        free(filter_data);
        return(EXIT_SUCCESS);
    }
    else{
        return(EXIT_FAILURE);
    }
}

int set2dFilterParam(int type, float gain, filter_data_2d_t* filter_data){
    for(int i = 0; i < filter_data->size; i++){
        filter_data->filter_array[i].output = 0;
        filter_data->filter_array[i].type = type;
        filter_data->filter_array[i].gain = gain;
    }
    return(1);
}

int update2dFilter(float* input, float* output, filter_data_2d_t* filter_data){
    for(int i = 0; i < filter_data->size; i++){
        output[i] = updateFilter(input[i], &(filter_data->filter_array[i]));
    }
    return(1);
}