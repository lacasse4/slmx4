#include <complex.h>
#include <fftw3.h>
#define N 1024
int main()
{
    fftwf_complex *in, *out;
    fftwf_plan p;
    
    in = (fftwf_complex*) fftw_malloc(sizeof(fftwf_complex) * N);
    out = (fftwf_complex*) fftw_malloc(sizeof(fftwf_complex) * N);
    p = fftwf_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    
    fftwf_execute(p); /* repeat as needed */
    
    fftwf_destroy_plan(p);
    fftwf_free(in); 
    fftwf_free(out);
}
