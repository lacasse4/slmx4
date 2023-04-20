#ifndef FRAMEFILTER_H_
#define FRAMEFILTER_H_

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 27 Hz

* 0 Hz - 0.2 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 3.7499299072054098 dB

* 0.35 Hz - 13.5 Hz
  gain = 0
  desired attenuation = -20 dB
  actual attenuation = -20.903034313999697 dB

*/

#define FRAMEFILTER_TAP_NUM 129

typedef struct {
  float history[FRAMEFILTER_TAP_NUM];
  unsigned int last_index;
} frameFilter;

void frameFilter_init(frameFilter* f);
void frameFilter_put(frameFilter* f, float input);
float frameFilter_get(frameFilter* f);

#endif