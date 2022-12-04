#include <math.h>
#include "rms.h"

void rms_init(rms_t* r) {
  int i;
  for(i = 0; i < RMS_TAP_NUM; ++i)
    r->history[i] = 0;
  r->last_index = 0;
}

void rms_put(rms_t* r, float input) {
  r->history[r->last_index++] = input;
  if(r->last_index == RMS_TAP_NUM)
    r->last_index = 0;
}

float rms_get(rms_t* r) {
  float acc = 0;
  int index = r->last_index, i;
  for(i = 0; i < RMS_TAP_NUM; ++i) {
    index = index != 0 ? index-1 : RMS_TAP_NUM-1;
    acc += r->history[index] * r->history[index];
  };
  return sqrtf(acc/RMS_TAP_NUM);
}
