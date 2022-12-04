#include "frameFilter.h"

static float filter_taps[FRAMEFILTER_TAP_NUM] = {
  0.0028816321222343328,
  0.0324237629596072,
  0.058928519688106626,
  0.09669427867188454,
  0.13310385629212737,
  0.15981242329792125,
  0.1696230293391069,
  0.15981242329792125,
  0.13310385629212737,
  0.09669427867188454,
  0.058928519688106626,
  0.0324237629596072,
  0.0028816321222343328
};

void frameFilter_init(frameFilter* f) {
  int i;
  for(i = 0; i < FRAMEFILTER_TAP_NUM; ++i)
    f->history[i] = 0;
  f->last_index = 0;
}

void frameFilter_put(frameFilter* f, float input) {
  f->history[f->last_index++] = input;
  if(f->last_index == FRAMEFILTER_TAP_NUM)
    f->last_index = 0;
}

float frameFilter_get(frameFilter* f) {
  float acc = 0;
  int index = f->last_index, i;
  for(i = 0; i < FRAMEFILTER_TAP_NUM; ++i) {
    index = index != 0 ? index-1 : FRAMEFILTER_TAP_NUM-1;
    acc += f->history[index] * filter_taps[i];
  };
  return acc;
}

