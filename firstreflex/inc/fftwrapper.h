#ifndef FFTWRAPPER_H_
#define FFTWRAPPER_H_

/*
 * C wrapper for fftw
 */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct fft_wrapper fft_wrapper_t;

fft_wrapper_t* fft_init(int size);
void fft_spectrum(fft_wrapper_t* fft_wrapper, float* signal, float* spectrum);
void fft_release(fft_wrapper_t* fft_wrapper);
void fft_cleanup();

#ifdef __cplusplus
}
#endif

#endif