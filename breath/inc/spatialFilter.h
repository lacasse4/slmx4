#ifndef SPATIALFILTER_H_
#define SPATIALFILTER_H_

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 19.0476 Hz

* 0 Hz - 1 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 2.903707009482916 dB

* 3 Hz - 9.5238 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -43.071346887700734 dB

  ADDED by VL, 2023-04-03:
    19.0476 is in _1/meter_, that is 1 divided by 5.25 cm.
    5.25 cm is the spatial resolution of the SLMX4 sensor.

*/

#define SPATIALFILTER_TAP_NUM 13

typedef struct {
  float history[SPATIALFILTER_TAP_NUM];
  unsigned int last_index;
} spatialFilter;

void spatialFilter_init(spatialFilter* f);
void spatialFilter_put(spatialFilter* f, float input);
float spatialFilter_get(spatialFilter* f);

#endif