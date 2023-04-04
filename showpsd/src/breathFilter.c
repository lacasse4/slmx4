#include "breathFilter.h"

static float filter_taps[BREATHFILTER_TAP_NUM] = {
  0.1,
  0.2,
  0.4,
  0.2,
  0.1
};

void breathFilter_init(breathFilter* f) {
  for(int i = 0; i < BREATHFILTER_TAP_NUM; ++i)
    f->history[i] = 0;
  f->last_index = 0;
}

void breathFilter_put(breathFilter* f, float input) {
  f->history[f->last_index++] = input;
  if(f->last_index == BREATHFILTER_TAP_NUM)
    f->last_index = 0;
}

float breathFilter_get(breathFilter* f) {
  float acc = 0;
  int index = f->last_index;
  for(int i = 0; i < BREATHFILTER_TAP_NUM; ++i) {
    index = index != 0 ? index-1 : BREATHFILTER_TAP_NUM-1;
    acc += f->history[index] * filter_taps[i];
  };
  return acc;
}
