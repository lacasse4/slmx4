#include "spatialFilter.h"

static float filter_taps[SPATIALFILTER_TAP_NUM] = {
  0.006854706198272708,
  0.028925752283053465,
  0.05738689745566029,
  0.09768443980644448,
  0.13672265688583932,
  0.1665480233319569,
  0.17736636521450042,
  0.1665480233319569,
  0.13672265688583932,
  0.09768443980644448,
  0.05738689745566029,
  0.028925752283053465,
  0.006854706198272708
};

void spatialFilter_init(spatialFilter* f) {
  int i;
  for(i = 0; i < SPATIALFILTER_TAP_NUM; ++i)
    f->history[i] = 0;
  f->last_index = 0;
}

void spatialFilter_put(spatialFilter* f, float input) {
  f->history[f->last_index++] = input;
  if(f->last_index == SPATIALFILTER_TAP_NUM)
    f->last_index = 0;
}

float spatialFilter_get(spatialFilter* f) {
  float acc = 0;
  int index = f->last_index, i;
  for(i = 0; i < SPATIALFILTER_TAP_NUM; ++i) {
    index = index != 0 ? index-1 : SPATIALFILTER_TAP_NUM-1;
    acc += f->history[index] * filter_taps[i];
  };
  return acc;
}
