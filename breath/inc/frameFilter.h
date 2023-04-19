#ifndef FRAMEFILTER_H_
#define FRAMEFILTER_H_

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 7 Hz

* 0 Hz - 0.2 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 3.9627724820004313 dB

* 0.35 Hz - 3.5 Hz
  gain = 0
  desired attenuation = -20 dB
  actual attenuation = -20.438909072505655 dB

*/

#define FRAMEFILTER_TAP_NUM 33

typedef struct {
  float history[FRAMEFILTER_TAP_NUM];
  unsigned int last_index;
} frameFilter;

void frameFilter_init(frameFilter* f);
void frameFilter_put(frameFilter* f, float input);
float frameFilter_get(frameFilter* f);

#endif