#include <math.h>
#include <float.h>
#include "peak.h"

static peak_t get_accurate_peak(peak_t peak, float* signal)
{
	// find peak accuratly
	int i = peak.position;
  	float delta;

  	delta = (signal[i-1] - signal[i+1]) * 0.5 /
            (signal[i-1] - (2.0 * signal[i]) + signal[i+1]);

  	peak.precise_position = i + delta;
  	peak.precise_value = (signal[i-1] + (2.0 * signal[i]) + signal[i+1]) / 4;

  	return peak;
}

void erase_peak(peak_t* peak) {
    peak->position = -1;
	peak->value = 0.0f;
    peak->precise_position = 0.0f;
    peak->precise_value = 0.0f;
	peak->scaled_position = 0.0f;
}

peak_t find_peak(float* signal, int num_samples, int start_index, int stop_index)
{  
	int   position;
	float value;
	peak_t not_found;
	peak_t peak;

	// to be returned if a peak is not found
	erase_peak(&not_found);

	// check input parameters
	if (start_index < 0)            return not_found;
	if (stop_index  >= num_samples) return not_found;

	// find max peak in specified range
	position = start_index;
	value = signal[start_index];
	for (int i = start_index; i <= stop_index; i++) {
		if (value < signal[i]) {
			value = signal[i]; 
			position = i;
		}
	}

	// a peak can not be at a boundary value
	if (position == start_index) return not_found;
	if (position == stop_index)  return not_found;

	// return result
	erase_peak(&peak);
	peak.position = position;
	peak.value = value;

  	return peak;
}

peak_t find_peak_precise(float* signal, int num_samples, int start_index, int stop_index)
{
	peak_t peak;

	// find peak within range from-to
	peak = find_peak(signal, num_samples, start_index, stop_index);
	if (peak.position == -1) return peak;
  	return get_accurate_peak(peak, signal);
}

peak_t find_peak_with_unit(float* signal, int num_samples, float from, float to, float samples_per_unit)
{
	int start_index = roundf(from * samples_per_unit);
	int stop_index  = roundf(to   * samples_per_unit);
	peak_t peak = find_peak_precise(signal, num_samples, start_index, stop_index);
	peak.scaled_position = peak.precise_position / samples_per_unit;
	return peak;
}

peak_t find_first_peak(float* signal, int num_samples, int start_index, int stop_index)
{  
	peak_t peak;

	// peak is reset. peak.position == -1 means no peak was found.
	erase_peak(&peak);

	// check input parameters
	if (start_index < 0) return peak;
	if (stop_index >= num_samples-1) return peak;

	// search for the first peak in specified range
	for (int i = start_index+1; i < stop_index; i++) {
		if (signal[i-1] < signal[i] && signal[i+1] < signal[i]) {
			peak.position = i;
			peak.value = signal[i];
			break;
		}
	}

  	return peak;
}

peak_t find_first_peak_precise(float* signal, int num_samples, int start_index, int stop_index)
{
	peak_t peak;

	// find peak within range from-to
	peak = find_first_peak(signal, num_samples, start_index, stop_index);
	if (peak.position == -1) return peak;
  	return get_accurate_peak(peak, signal);
}

peak_t find_first_peak_with_unit(float* signal, int num_samples, 
	float from, float to, float samples_per_unit)
{
	int start_index = roundf(from * samples_per_unit);
	int stop_index  = roundf(to   * samples_per_unit);
	peak_t peak = find_first_peak_precise(signal, num_samples, start_index, stop_index);
	peak.scaled_position = peak.precise_position / samples_per_unit;
	return peak;
}

peak_t find_second_peak(float* signal, int num_samples, int start_index, int stop_index)
{  
	peak_t first_peak;
	peak_t second_peak;

	// Reset second_peak.
	erase_peak(&second_peak);

	// find the first peak;
	first_peak = find_first_peak(signal, num_samples, start_index, stop_index);
	if (first_peak.position == -1) return second_peak;

	// search for the second peak in specified range
	for (int i = first_peak.position+1; i < stop_index; i++) {
		if (signal[i-1] < signal[i] && signal[i+1] < signal[i]) {
			second_peak.position = i;
			second_peak.value = signal[i];
			break;
		}
	}

  	return second_peak;
}

peak_t find_second_peak_precise(float* signal, int num_samples, int start_index, int stop_index)
{
	peak_t peak;

	// find peak within range from-to
	peak = find_second_peak(signal, num_samples, start_index, stop_index);
	if (peak.position == -1) return peak;
  	return get_accurate_peak(peak, signal);
}

peak_t find_second_peak_with_unit(float* signal, int num_samples, 
	float from, float to, float samples_per_unit)
{
	int start_index = roundf(from * samples_per_unit);
	int stop_index  = roundf(to   * samples_per_unit);
	peak_t peak = find_second_peak_precise(signal, num_samples, start_index, stop_index);
	peak.scaled_position = peak.precise_position / samples_per_unit;
	return peak;
}


float  find_peak_raising_edge(float* signal, int num_sample, int start_index, int stop_index)
{
	int    i;
	int    found = 0;
	float  edge = -1.0f;
	float  edge_level;
	peak_t peak;

	peak = find_peak(signal, num_sample, start_index, stop_index);
	if (peak.position == -1) {
		return edge;
	}

	edge_level = peak.value * 0.5;

	// looking backward from peak position, search signal value under edge_level
	for (i = peak.position; i >= start_index && !found; i--) {
		if (signal[i] < edge_level) {
			found = 1;
			break;
		}
	}

	// use linear interpolation to get a precise edge position at edge_level. 
	if (found) {
		float m = signal[i+1] - signal[i];
		if (fabsf(m) >= FLT_EPSILON) {
			float b = signal[i] - m * i;
			edge = (edge_level - b) / m;
		}
	}
	return edge;
}

float  find_peak_raising_edge_with_unit(float* signal, int num_samples, float from, float to, float samples_per_unit)
{
	float edge;

	int start_index = roundf(from * samples_per_unit);
	int stop_index  = roundf(to   * samples_per_unit);
	edge = find_peak_raising_edge(signal, num_samples, start_index, stop_index);
	if (edge != -1.0f) {
		edge = edge / samples_per_unit;
	}

	return edge;
}
