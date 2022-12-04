#include "frameFilter.h"

static float filter_taps[FRAMEFILTER_TAP_NUM] = {
  -0.005731954213693977,
  -0.0008700314245037347,
  0.004434184209121268,
  0.015020635343170354,
  0.0313650838886926,
  0.05278120891908883,
  0.07731006666332174,
  0.1019172604057543,
  0.12303588504027976,
  0.137312692165662,
  0.1423598670271868,
  0.137312692165662,
  0.12303588504027976,
  0.1019172604057543,
  0.07731006666332174,
  0.05278120891908883,
  0.0313650838886926,
  0.015020635343170354,
  0.004434184209121268,
  -0.0008700314245037347,
  -0.005731954213693977
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
