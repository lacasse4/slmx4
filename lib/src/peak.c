#include <math.h>
#include "peak.h"

peak_t find_highest_peak(float* signal, int num_samples, 
	float frame_start, float frame_end, float from, float to)
{  
	int   low_index;
	int   high_index;
	int   index;
	float power;
	int   is_max;

	// to be returned if a peak is not found
	peak_t not_found;
	erase_peak(&not_found);

	// 'from' and 'to' must be within the frame_start -> frame_end range
	low_index = distance_to_index(frame_start, frame_end, num_samples, from);
	if (low_index <= 0) return not_found;

	high_index = distance_to_index(frame_start, frame_end, num_samples, to);
	if (high_index >= num_samples-1) return not_found;

	// find max peak in specified range
	index = low_index;
	power = signal[low_index];
	for (int i = low_index; i <= high_index; i++) {
		if (power < signal[i]) {
			power = signal[i]; 
			index = i;
		}
	}

	// there is no peak if max_index is a boundary value
	if (index == low_index) return not_found;
	if (index == high_index) return not_found;

	// value at max_index must be surrounded by lower values
	is_max = signal[index] > signal[index-1] &&
			 signal[index] > signal[index+1];
	if (!is_max) return not_found;

	// find peak accuratly
	peak_t precise_peak;
  	float delta;
  	float interpolation;

  	delta = (signal[index-1] - signal[index+1]) * 0.5 /
            (signal[index-1] - (2.0 * signal[index]) + signal[index+1]);

  	interpolation = index_to_distance(frame_start, frame_end, num_samples, index + delta);

  	precise_peak.index = index;
  	precise_peak.distance = interpolation;
  	precise_peak.power = (signal[index-1] + (2.0 * signal[index]) + signal[index+1]) / 4;

  	return precise_peak;
}

peak_t find_first_peak_above(float* signal, int num_samples, 
	float frame_start, float frame_end, float from, float to, float above)
{  
	int   low_index;
	int   high_index;
	int   index;
	float power;
	int   is_max;

	// to be returned if a peak is not found
	peak_t not_found;
	erase_peak(&not_found);

	// 'from' and 'to' must be within the frame_start -> frame_end range
	low_index = distance_to_index(frame_start, frame_end, num_samples, from);
	if (low_index <= 0) return not_found;

	high_index = distance_to_index(frame_start, frame_end, num_samples, to);
	if (high_index >= num_samples-1) return not_found;

	// find first peak in specified range
	index = low_index;
	power = signal[low_index];
	for (int i = low_index+1; i <= high_index; i++) {
		if (power > signal[i]) break;
		power = signal[i]; 
		index = i;
	}

	// There is no peak if max_index is a boundary value
	if (index == low_index) return not_found;
	if (index == high_index) return not_found;

	// value at max_index must be surrounded by lower values
	is_max = signal[index] > signal[index-1] &&
			 signal[index] > signal[index+1];
	if (!is_max) return not_found;

	// find peak accuratly
	peak_t precise_peak;
  	float delta;
  	float interpolation;

  	delta = (signal[index-1] - signal[index+1]) * 0.5 /
            (signal[index-1] - (2.0 * signal[index]) + signal[index+1]);

  	interpolation = index_to_distance(frame_start, frame_end, num_samples, index + delta);

  	precise_peak.index = index;
  	precise_peak.distance = interpolation;
  	precise_peak.power = (signal[index-1] + (2.0 * signal[index]) + signal[index+1]) / 4;

  	return precise_peak;
}


int distance_to_index(float frame_start, float frame_end, int num_samples, float distance) 
{
  return roundf(num_samples * (distance - frame_start) / (frame_end - frame_start));
}


float index_to_distance(float frame_start, float frame_end, int num_samples, float index) 
{
  return frame_start + index * (frame_end - frame_start) / num_samples;
}


void erase_peak(peak_t* peak) {
    peak->index = -1;
    peak->distance = 0.0;
    peak->power = 0.0;
};
