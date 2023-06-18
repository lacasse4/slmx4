#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <math.h>

#define MAX_FRAME_SIZE 188
#define NUM_FRAMES     200
#define START_INDEX      9

float frames[NUM_FRAMES][MAX_FRAME_SIZE];

int   read_img_file(const char* filename, float frames[NUM_FRAMES][MAX_FRAME_SIZE]);
float find_precise_peak(float* signal, int index);
int   find_first_peak(float* signal, int start, int len);


int main()
{
    if (read_img_file("RAW_FRAMES", frames) != 0) {
        exit(1);
    }

	printf("%d\n", NUM_FRAMES);
	for (int i = 0; i < NUM_FRAMES; i++) {
		int index;
		float position;
		index = find_first_peak(frames[i], START_INDEX, MAX_FRAME_SIZE);
		position = find_precise_peak(frames[i], index);
		printf("%f\n", position);
	}
	return 0;
}


int read_img_file(const char* filename, float frames[NUM_FRAMES][MAX_FRAME_SIZE])
{
	// Write data to file
	FILE* fd = fopen(filename, "r");
	if(fd == NULL) {
		fprintf(stderr,"ERROR: Can not open %s for writing\n", filename);
		return 1;
	}	

    int dummy_int;
    float dummy_float;
	for (int i = 0; i < NUM_FRAMES; i++) {
		fscanf(fd, "%d\n", &dummy_int);    // sensor->get_num_samples()
		fscanf(fd, "%f\n", &dummy_float);  // sensor->get_frame_start()
		fscanf(fd, "%f\n", &dummy_float);  // sensor->get_frame_end());
		for (int j = 0; j < 188; j++) {
			fscanf(fd,"%f\n", &frames[i][j]);
		}
	}

	fclose(fd);
	return 0;
}

int find_first_peak(float* signal, int start, int len) 
{
	float max = signal[start];
	int  imax = start;
	for (int i = start+1; i < len; i++) {
		if (signal[i] < max) {
			break;
		}
		max = signal[i];
		imax = i;
	}
	return imax;
}

float find_precise_peak(float* signal, int index)
{
  float delta;
  float precise_position;

  delta = (signal[index-1] - signal[index+1]) * 0.5 /
          (signal[index-1] - (2.0 * signal[index]) + signal[index+1]);
  precise_position = index + delta;

  return precise_position;
}
