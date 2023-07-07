/*
 * C wrapper for fftw
 */

#include <stdlib.h>
#include <complex.h>
#include <fftw3.h>

#include "fftwrapper.h"

    fftwf_complex *in;
    fftwf_complex *out;
    fftwf_plan p;

struct fft_wrapper
{
    int size;
    fftwf_complex *in;
    fftwf_complex *out;
    fftwf_plan p;
};


fft_wrapper_t* fft_init(int size) 
{
    fft_wrapper_t* fft_wrapper = (fft_wrapper_t*)malloc(sizeof(fft_wrapper));
    if (fft_wrapper == NULL) {
        return NULL;
    }

    fft_wrapper->size = size;
    in   = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * size);
    out  = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * size);
    p    = fftwf_plan_dft_1d(size, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    // fft_wrapper->in   = in;
    // fft_wrapper->out  = out;
    // fft_wrapper->p    = p;
    // fft_wrapper->in   = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * size);
    // fft_wrapper->out  = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * size);
    // fft_wrapper->p    = fftwf_plan_dft_1d(size, fft_wrapper->in, fft_wrapper->out, FFTW_FORWARD, FFTW_ESTIMATE);
    // if (fft_wrapper->in == NULL || fft_wrapper->out == NULL || fft_wrapper->p == NULL) {
    //     fft_release(fft_wrapper);
    //     return NULL;
    // }

    return fft_wrapper;
}

void fft_release(fft_wrapper_t* fft_wrapper)
{
    // in  = fft_wrapper->in;
    // out = fft_wrapper->out;
    // p   = fft_wrapper->p;
    fftwf_destroy_plan(p);
    fftwf_free(out);
    fftwf_free(in); 
    // fftwf_destroy_plan(fft_wrapper->p);
    // fftwf_free(fft_wrapper->out);
    // fftwf_free(fft_wrapper->in); 
    free(fft_wrapper);
}

void fft_spectrum(fft_wrapper_t* fft_wrapper, float* signal, float* spectrum)
{
    // in  = fft_wrapper->in;
    // out = fft_wrapper->out;
    // p   = fft_wrapper->p;

    for (int i = 0; i < fft_wrapper->size; i++) {
        in[i] = signal[i] + I * 0.0;
    }

    fftwf_execute(p);

    for (int i = 0; i < fft_wrapper->size/2+1; i++) {
        spectrum[i] = cabsf(out[i]);
    }

    // for (int i = 0; i < fft_wrapper->size; i++) {
    //     fft_wrapper->in[i] = signal[i] + I * 0.0;
    // }

    // fftwf_execute(fft_wrapper->p);

    // for (int i = 0; i < fft_wrapper->size/2+1; i++) {
    //     spectrum[i] = cabsf(fft_wrapper->out[i]);
    // }

}
