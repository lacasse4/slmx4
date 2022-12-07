#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <math.h>

#include "zeroxing.h"

#define MAX_FRAME_SIZE 188
#define NUM_FRAMES 200


int main(int argc, char* argv[])
{
    int num_samples;
    float frame_start;
    float frame_end;
    float frame[MAX_FRAME_SIZE];
    zeroxing_t _zeroxing_mem;
    zeroxing_t* zeroxing; 
    float frequency = 0.0;
    float sampling_rate = 7.0;

    FILE* fd = fopen("SCAN22", "r");
    fscanf(fd, "%d", &num_samples);
    fscanf(fd, "%f", &frame_start);
    fscanf(fd, "%f", &frame_end);
    for (int i = 0; i < num_samples; i ++) {
        fscanf(fd, "%f", &frame[i]);    
    }
    fclose(fd);

    zeroxing = &_zeroxing_mem;
    zeroxing_init(zeroxing);

   	for (int i = 0; i < MAX_FRAME_SIZE; i++) {
        int is_valid = zeroxing_put(zeroxing, frame[i]);
        if (is_valid) {
            frequency = zeroxing_get(zeroxing, sampling_rate);
            printf("index = %3d, freq = %f\n", i, frequency);
        }
	}

	return EXIT_SUCCESS;
}