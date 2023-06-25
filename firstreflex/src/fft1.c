/*
 * Processus breath signal (from file) with fftw
 */

#include <stdlib.h>
#include <stdio.h>
#include <complex.h>
#include <fftw3.h>

#define N 512
#define MAX_FILE_LENGTH 2048

int   length;
int   offset = 0;
float signal[MAX_FILE_LENGTH];

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

    fftwf_complex *in, *out;
    fftwf_plan p;

    in = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * N);
    out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * N);
    p = fftwf_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
 
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

    for (int i = 0; i < N; i++) {
        in[i] = signal[i+offset] + I * 0.0;
    }

    fftwf_execute(p);

    for (int i = 0; i < N/2+1; i++) {
        printf("%7.4f\n", cabsf(out[i]));
    }
 
    fftwf_destroy_plan(p);
    fftwf_free(in); 
    fftwf_free(out);
}