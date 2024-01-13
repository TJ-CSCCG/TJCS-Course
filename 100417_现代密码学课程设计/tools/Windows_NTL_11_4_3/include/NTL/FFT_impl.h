
#ifndef NTL_FFT_impl__H
#define NTL_FFT_impl__H

#include <NTL/tools.h>

NTL_OPEN_NNS

#ifdef NTL_ENABLE_AVX_FFT

#if (!defined(NTL_HAVE_AVX512F) && !(defined(NTL_HAVE_AVX2) && defined(NTL_HAVE_FMA)))
#error "NTL_ENABLE_AVX_FFT: not supported on this platform"
#endif

#if (defined(NTL_HAVE_AVX512F) && !defined(NTL_AVOID_AVX512))
#define NTL_LG2_PDSZ (3)
#else
#define NTL_LG2_PDSZ (2)
#endif

#define NTL_FFT_RDUP (NTL_LG2_PDSZ+3)
#define NTL_PDSZ (1 << NTL_LG2_PDSZ)

#else

#define NTL_FFT_RDUP (4)
// Currently, this should be at least 2 to support
// loop unrolling in the FFT implementation

#endif

inline
long FFTRoundUp(long xn, long k)
// Assumes k >= 0.
// Returns an integer m such that 1 <= m <= n = 2^k and 
// m divsisible my 2^NTL_FFT_RDUP.
// Also, if xn <= n, then m >= xn.
{
   long n = 1L << k;
   if (xn <= 0) xn = 1;

   xn = ((xn+((1L << NTL_FFT_RDUP)-1)) >> NTL_FFT_RDUP) << NTL_FFT_RDUP; 

   if (k >= 10) {
      if (xn > n - (n >> 4)) xn = n;
   }
   else {
      if (xn > n - (n >> 3)) xn = n;
   }
   // truncation just a bit below n does not really help
   // at all, and can sometimes slow things down slightly, so round up 
   // to n.  This also takes care of cases where xn > n.
   // Actually, for smallish n, we should round up sooner,
   // at n-n/8, and for larger n, we should round up later,
   // at n-m/16.  At least, experimentally, this is what I see.

   return xn;
}


NTL_CLOSE_NNS

#endif
