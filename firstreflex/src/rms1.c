/*
 * Compute standard deviation of input file.
 */

#include <stdlib.h>
#include <stdio.h>

#include <time.h>

#include "rms.h"

#define MAX_FILE_LENGTH 20480

int main(int argc, char* argv[])
{
    char buff[100];
    float value;
    rms_t rms;
    int first = 1;

    while (fgets(buff, 100, stdin)) {
        sscanf(buff, "%f", &value);
        if (first) {
            rms_init(&rms, value);
            first = 0;
        }
        rms_put(&rms, value);
        printf("%7.4f\n", rms_get(&rms));
    }
    return 0;
}
