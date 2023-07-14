#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <math.h>

#include "peak.h"

int main(int argc, char* argv[])
{
    float frame[188];
    peak_t peak;

    FILE* fd = fopen("FRAME", "r");
    if(fd == NULL) {
        fprintf(stderr, "Unable to open FRAME\n");
        exit(1);
    }

    for (int i = 0; i < 188; i ++) {
        fscanf(fd, "%f", &frame[i]);    
    }
    fclose(fd);


    // peak = find_second_peak(frame, 10, 0, 8);
    // printf("position = %d\n", peak.position);
    // printf("value    = %f\n", peak.value);

    // peak = find_second_peak_precise(frame, 10, 0, 8);
    // printf("position = %d\n", peak.position);
    // printf("value    = %f\n", peak.value);
    // printf("precise_position = %f\n", peak.precise_position);
    // printf("precise_value    = %f\n", peak.precise_value);

    peak = find_second_peak_with_unit(frame, 188, 0.45f, 2.0f, 188/9.85f);
    printf("position = %d\n", peak.position);
    printf("value    = %f\n", peak.value);
    printf("precise_position = %f\n", peak.precise_position);
    printf("precise_value    = %f\n", peak.precise_value);
    printf("scaled_position  = %f\n", peak.scaled_position);

	return EXIT_SUCCESS;
}