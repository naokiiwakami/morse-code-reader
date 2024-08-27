#ifndef FFT_C_
#define FFT_C_

#ifdef __cplusplus
extern "C" {
#endif

typedef float real;

typedef struct {
  real Re;
  real Im;
} complex;

extern void fft(complex *v, int n, complex *tmp);

#ifdef __cplusplus
}
#endif

#endif // FFT_C_