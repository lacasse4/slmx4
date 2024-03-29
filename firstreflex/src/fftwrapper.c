/*
 * C wrapper for fftw
 */

#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <fftw3.h>

#include "fftwrapper.h"

struct fft_wrapper
{
    int size;
    fftwf_complex *in;
    fftwf_complex *out;
    fftwf_plan plan;
};


fft_wrapper_t* fft_init(int size) 
{
    fft_wrapper_t* fft_wrapper = (fft_wrapper_t*)malloc(sizeof(fft_wrapper_t));
    if (fft_wrapper == NULL) {
        return NULL;
    }
    memset(fft_wrapper, 0, sizeof(fft_wrapper_t));

    fft_wrapper->size = size;
    fft_wrapper->in   = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * size);
    fft_wrapper->out  = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * size);
    fft_wrapper->plan = fftwf_plan_dft_1d(size, fft_wrapper->in, fft_wrapper->out, FFTW_FORWARD, FFTW_ESTIMATE);

    if (fft_wrapper->in == NULL || fft_wrapper->out == NULL || fft_wrapper->plan == NULL) {
        fft_release(fft_wrapper);
        return NULL;
    }

    return fft_wrapper;
}

void fft_release(fft_wrapper_t* fft_wrapper)
{
    fftwf_destroy_plan(fft_wrapper->plan);
    fftwf_free(fft_wrapper->out);
    fftwf_free(fft_wrapper->in); 
    free(fft_wrapper);
}

void fft_cleanup()
{
    fftwf_cleanup();
}

void fft_spectrum(fft_wrapper_t* fft_wrapper, float* signal, float* spectrum)
{
    for (int i = 0; i < fft_wrapper->size; i++) {
        fft_wrapper->in[i] = signal[i] + I * 0.0;
    }

    fftwf_execute(fft_wrapper->plan);

    for (int i = 0; i < fft_wrapper->size/2+1; i++) {
        spectrum[i] = cabsf(fft_wrapper->out[i]);
    }
}
