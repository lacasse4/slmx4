#ifndef FRAMEFILTER_H_
#define FRAMEFILTER_H_

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 7 Hz

* 0 Hz - 0.1 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 2.773164851238787 dB

* 0.2 Hz - 3.5 Hz
  gain = 0
  desired attenuation = -20 dB
  actual attenuation = -23.4638667316325 dB

*/

#define FRAMEFILTER_TAP_NUM 63

typedef struct {
  float history[FRAMEFILTER_TAP_NUM];
  unsigned int last_index;
} frameFilter;

void frameFilter_init(frameFilter* f);
void frameFilter_put(frameFilter* f, float input);
float frameFilter_get(frameFilter* f);

#endif