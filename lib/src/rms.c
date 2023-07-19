#include <math.h>
#include "rms.h"

void rms_init(rms_t* r, float init_value) {
    for(int i = 0; i < RMS_TAP_NUM; ++i)
        r->history[i] = init_value;
    r->last_index = 0;
}

void rms_put(rms_t* r, float input) {
    r->history[r->last_index++] = input;
    if(r->last_index == RMS_TAP_NUM)
        r->last_index = 0;
}

float rms_get(rms_t* r) {
    float est1 = 0.0;
    float est2 = 0.0;
    float tmp;
    int index = r->last_index;

    for(int i = 0; i < RMS_TAP_NUM; ++i) {
        index = index != 0 ? index-1 : RMS_TAP_NUM-1;
        est1 += r->history[index];
    }
    est1 = est1 / RMS_TAP_NUM;

    for(int i = 0; i < RMS_TAP_NUM; ++i) {
        index = index != 0 ? index-1 : RMS_TAP_NUM-1;
        tmp =  r->history[index] - est1;
        est2 += tmp * tmp;
    };
    return sqrtf(est2/RMS_TAP_NUM);
}
