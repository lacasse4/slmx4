#include "frameFilter.h"

static float filter_taps[FRAMEFILTER_TAP_NUM] = {
  -0.04538991160263083,
  0.005071194866483603,
  0.00707748597271618,
  0.010414163819292428,
  0.014936537465792673,
  0.020360535755248377,
  0.026730994217898655,
  0.03363607921645075,
  0.04087198908746204,
  0.04816495693384592,
  0.05520395519180912,
  0.06176301277761297,
  0.06754596991252412,
  0.07232531977249239,
  0.0758946478262003,
  0.07809268905377964,
  0.07884014875677947,
  0.07809268905377964,
  0.0758946478262003,
  0.07232531977249239,
  0.06754596991252412,
  0.06176301277761297,
  0.05520395519180912,
  0.04816495693384592,
  0.04087198908746204,
  0.03363607921645075,
  0.026730994217898655,
  0.020360535755248377,
  0.014936537465792673,
  0.010414163819292428,
  0.00707748597271618,
  0.005071194866483603,
  -0.04538991160263083
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
