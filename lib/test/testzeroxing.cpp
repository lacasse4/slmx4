#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <math.h>

#include "zeroxing.h"

#define MAX_FRAME_SIZE 188
#define NUM_FRAMES 200


int main(int argc, char* argv[])
{
    int num_frames;
     float frame[NUM_FRAMES];
    zeroxing_t _zeroxing_mem;
    zeroxing_t* zeroxing; 
    float frequency = 0.0;
    float sampling_rate = 7.0;

    FILE* fd = fopen("BREATH23", "r");
    fscanf(fd, "%d", &num_frames);
    if (num_frames != NUM_FRAMES) {
        fprintf(stderr, "Invalid file\n");
        exit(1);
    }

    for (int i = 0; i < num_frames; i ++) {
        fscanf(fd, "%f", &frame[i]);    
    }
    fclose(fd);

    zeroxing = &_zeroxing_mem;
    zeroxing_init(zeroxing);

    printf("num_frames = %d\n", num_frames);
    printf("sampling_rate = %f\n", sampling_rate);
   	for (int i = 0; i < MAX_FRAME_SIZE; i++) {
        int is_valid = zeroxing_put(zeroxing, frame[i]);
        if (is_valid) {
            frequency = zeroxing_get(zeroxing, sampling_rate);
            printf("index = %3d, freq = %f\n", i, frequency);
        }
	}

	return EXIT_SUCCESS;
}