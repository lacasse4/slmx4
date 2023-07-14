#include <math.h>
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
	int   position;
	int   is_max; 
	float value;
	peak_t not_found;
	peak_t peak;

	// to be returned if a peak is not found
	erase_peak(&not_found);

	// check input parameters
	if (start_index < 0) return not_found;
	if (stop_index >= num_samples-1) return not_found;

	// find first peak in specified range
	position = start_index;
	value = signal[start_index];
	for (int i = start_index+1; i < stop_index; i++) {
		if (value > signal[i]) break;
		value = signal[i]; 
		position = i;
	}

	// a peak can not be at a boundary
	if (position == start_index) return not_found;
	if (position == stop_index) return not_found;

	// value at position must be surrounded by lower values
	is_max = value > signal[position-1] &&
		     value > signal[position+1];
	if (!is_max) return not_found;

	// return result
	erase_peak(&peak);
	peak.position = position;
	peak.value = value;

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
	int   position_first;
	int   position_second;
	int   is_max;
	float value;
	peak_t not_found;
	peak_t peak;

	// to be returned if a peak is not found
	erase_peak(&not_found);

	// check input parameters
	if (start_index < 0) return not_found;
	if (stop_index >= num_samples-1) return not_found;

	// find first peak in specified range
	position_first = start_index;
	value = signal[start_index];
	for (int i = start_index+1; i < stop_index; i++) {
		if (value > signal[i]) break;
		value = signal[i]; 
		position_first = i;
	}

	// a peak can not be at a boundary
	if (position_first == start_index) return not_found;
	if (position_first == stop_index) return not_found;

	// value at position must be surrounded by lower values
	is_max = value > signal[position_first-1] &&
		     value > signal[position_first+1];
	if (!is_max) return not_found;

	// find second peak in specified range
	position_second = position_first + 1;
	value = signal[position_second];
	for (int i = position_second+1; i < stop_index; i++) {
		if (value > signal[i]) break;
		value = signal[i]; 
		position_second = i;
	}

	// a peak can not be at a boundary
	if (position_second == stop_index) return not_found;

	// value at position must be surrounded by lower values
	is_max = value > signal[position_second-1] &&
		     value > signal[position_second+1];
	if (!is_max) return not_found;

	// return result
	erase_peak(&peak);
	peak.position = position_second;
	peak.value = value;

  	return peak;
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
