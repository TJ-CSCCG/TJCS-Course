
#ifndef NTL_pd_FFT__H
#define NTL_pd_FFT__H


#include <NTL/tools.h>

NTL_OPEN_NNS


// Sets control register so that rounding mode
// is "down".  Destructor restores control regsiter.
struct CSRPush {
   unsigned int reg;
   CSRPush();
   ~CSRPush();
};


struct pd_mod_t {
   double q;
   const double **wtab;
   const double **wqinvtab;
   const double **wtab1;
   const double **wqinvtab1;
};


void
pd_LazyPrepMulModPrecon_impl(double *bninv, const double *b, double n, long len);

void
pd_fft_trunc_impl(long* A, const long* a, double* xp, long lgN, const pd_mod_t& mod,
                  long yn, long xn);

void
pd_fft_trunc_impl(long* A, const long* a, double* xp, long lgN, const pd_mod_t& mod,
                  long yn, long xn, double fac);

void
pd_ifft_trunc_impl(long* A, const long* a, double* xp, long lgN, 
                   const pd_mod_t& mod, long yn);

void
pd_ifft_trunc_impl(long* A, const long* a, double* xp, long lgN, 
                   const pd_mod_t& mod, long yn, double fac);

NTL_CLOSE_NNS

#endif
