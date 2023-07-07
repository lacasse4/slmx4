/*
 * Processus breath signal (from file) with fftw
 * This program was created to test fftwrapper.c
 */

#include <stdlib.h>
#include <stdio.h>

#include <time.h>

#include "fftwrapper.h"

#define N 512
#define MAX_FILE_LENGTH 2048

int   length;
int   offset = 0;
float signal[MAX_FILE_LENGTH];
float spectrum[MAX_FILE_LENGTH];

int main(int argc, char* argv[])
{
    if (argc < 2 && argc > 3) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "       fft <signal_file>\n");
        fprintf(stderr, "    or fft <signal_file> <offset>\n");
        return 1;
    }

    if (argc == 3) {
        offset = atoi(argv[2]);
    }

    // Prepare fft
    fft_wrapper_t* fft_wrapper = fft_init(N);
 
    // read input file

    char buff[100];
    FILE* fd = NULL;
    fd = fopen(argv[1], "r");
    if(fd == NULL) {
        fprintf(stderr, "Error opening file %s\n", argv[1]);
        exit(1);
    }

    length = 0;
    while (fgets(buff, 100, fd)) {
        sscanf(buff, "%f", &signal[length]);
        length++;
        if (length >= MAX_FILE_LENGTH) break;
    }

    fclose(fd);

    // process signal

    clock_t start, stop;
    start = clock();

    fft_spectrum(fft_wrapper, signal, spectrum);

    stop = clock();

    fprintf(stderr, "Time = %f s\n", ((float)(stop-start))/(CLOCKS_PER_SEC));

    for (int i = 0; i < N/2+1; i++) {
        printf("%7.4f\n", spectrum[i]);
    }
 
    fft_release(fft_wrapper);
}
