#ifndef FRAMEFILTER_H_
#define FRAMEFILTER_H_

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 7 Hz

* 0 Hz - 0.33 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 3.8034211906444026 dB

* 0.8 Hz - 3.5 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -40.78380143444543 dB

*/

#define FRAMEFILTER_TAP_NUM 21

typedef struct {
  float history[FRAMEFILTER_TAP_NUM];
  unsigned int last_index;
} frameFilter;

void frameFilter_init(frameFilter* f);
void frameFilter_put(frameFilter* f, float input);
float frameFilter_get(frameFilter* f);

#endif