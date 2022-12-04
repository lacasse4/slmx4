#ifndef FRAMEFILTER_H_
#define FRAMEFILTER_H_

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 7 Hz

* 0 Hz - 0.333 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 2.400516853635107 dB

* 1 Hz - 3.5 Hz
  gain = 0
  desired attenuation = -30 dB
  actual attenuation = -34.69897881070135 dB

*/

#define FRAMEFILTER_TAP_NUM 13

typedef struct {
  float history[FRAMEFILTER_TAP_NUM];
  unsigned int last_index;
} frameFilter;

void frameFilter_init(frameFilter* f);
void frameFilter_put(frameFilter* f, float input);
float frameFilter_get(frameFilter* f);

#endif