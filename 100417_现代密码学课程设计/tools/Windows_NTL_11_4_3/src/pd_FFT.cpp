
// The configure script should define NTL_FP_CONTRACT_OFF
// for icc via the NOCONTRACT variable
#ifdef NTL_FP_CONTRACT_OFF
#pragma fp_contract(off)
#endif


#include <NTL/tools.h>

#ifdef NTL_ENABLE_AVX_FFT

// The configure script tries to prevent this, but we
// double check here.  Note that while it is strongly 
// discouraged, other parts of NTL probably work even with 
// "fast math"; however, quad_float will definitely break.

#if (defined(__GNUC__) && __FAST_MATH__)
#error "do not compile pd_FFT.cpp with -ffast-math!!"
#endif



#include <NTL/PD.h>
#include <NTL/pd_FFT.h>
#include <NTL/FFT_impl.h>

#if (defined(__GNUC__) && __FAST_MATH__)
#error "do not compile pd_FFT.cpp with -ffast-math!!"
#endif

#if (NTL_FMA_DETECTED && !defined(NTL_CONTRACTION_FIXED))
#error "contraction not fixed"
#endif


NTL_START_IMPL

#define NTL_CSR_NEAREST (0x00000000)
#define NTL_CSR_DOWN    (0x00002000)
#define NTL_CSR_UP      (0x00004000)
#define NTL_CSR_TRUNC   (0x00006000)
#define NTL_CSR_MASK    (0x00006000)

CSRPush::CSRPush()
{
   // save current register value
   reg = _mm_getcsr();
   // set rounding mode to "down"
   _mm_setcsr((reg & ~NTL_CSR_MASK) | NTL_CSR_DOWN);
}

CSRPush::~CSRPush()
{
   _mm_setcsr(reg);
}



void
pd_LazyPrepMulModPrecon_impl(double *bninv, const double *b, double n, long len)
{
   for (long i = 0; i < len; i++) bninv[i] = b[i]/n;
}



template<class pd> pd
pd_LazyReduce1(pd a, double q)
{
   return correct_excess(a, q);
}

template<class pd> pd 
pd_LazyReduce2(pd a, double q)
{
   return correct_excess(a, 2*q);
}

// inputs in [0, 2*n), output in [0, 4*n)
template<class pd> pd
pd_LazyAddMod(pd a, pd b, double n)
{
   return a+b;
}

// inputs in [0, 2*n), output in [0, 4*n)
template<class pd> pd
pd_LazySubMod(pd a, pd b, double n)
{
   return a-b+2*n;
}

// inputs in [0, 2*n), output in [0, 2*n)
template<class pd> pd
pd_LazyAddMod2(pd a, pd b, double n)
{
   pd r = a+b;
   return correct_excess(r, 2*n);
}

// inputs in [0, 2*n), output in [0, 2*n)
template<class pd> pd 
pd_LazySubMod2(pd a, pd b, double n)
{
   pd r = a-b;
   return correct_deficit(r, 2*n);
}

// inputs in [0, 4*n), output in [0, 4*n)
template<class pd> pd
pd_LazyAddMod4(pd a, pd b, double n)
{
   pd r = a+b;
   return correct_excess(r, 4*n);
}

// inputs in [0, 4*n), output in [0, 4*n)
template<class pd> pd 
pd_LazySubMod4(pd a, pd b, double n)
{
   pd r = a-b;
   return correct_deficit(r, 4*n);
}


// Input and output in [0, 4*n)
template<class pd> pd
pd_LazyDoubleMod4(pd a, double n)
{
   return 2 * pd_LazyReduce2(a, n);
}

// Input and output in [0, 2*n)
template<class pd> pd
pd_LazyDoubleMod2(pd a, double n)
{
   return 2 * pd_LazyReduce1(a, n);
}



// n in [0,2^50), b in [0,n), a in [0,4*n), bninv = RoundDown(b/n)
// returns a*b mod n in [0, 2*n)
template<class pd> pd
pd_LazyMulModPrecon(pd a, pd b, double n, pd bninv)
{
   pd hi = a*b;
   pd lo = fused_mulsub(a, b, hi);  // hi+lo == a*b (exactly)
   pd q =  fused_muladd(a, bninv, 1L << 52);
   q -= (1L << 52);             // q is the correct quotient, or one too small
   pd d = fused_negmuladd(q, n, hi);   // d == hi - q*n (exactly)
   pd r = d + lo;           // r is the remainder, or the remainder plus n

   return r;
}

// return (a[0] + a[1], a[0] - a[1], a[2] + a[3], a[2] - a[3], ...)
// all inputs and outputs in [0, 2*n)
template<class pd> pd
pd_fwd_butterfly_packed2(pd a, double n)
{
   pd b = swap2(a);
   pd sum = pd_LazyAddMod(a, b, n);
   pd diff = pd_LazySubMod(b, a, n);
   pd res = blend2(sum, diff);
   res = pd_LazyReduce2(res, n);
   return res;
}

// return (a[0] + a[2], a[1] + a[3], (a[0] - a[2]), (a[1] - a[3])*root, ...) 
// all inputs and outputs in [0, 2*n)
// it is also assumed that w = (1,1,1,root,...) and wninv = RoundDown(w/n)
template<class pd> pd
pd_fwd_butterfly_packed4(pd a, pd w, double n, pd wninv)
{
   pd b = swap4(a);
   pd sum = pd_LazyAddMod(a, b, n);
   pd diff = pd_LazySubMod(b, a, n);
   pd res = blend4(sum, diff);
   res = pd_LazyMulModPrecon(res, w, n, wninv);
   return res;
}


static double 
pd_LazyPrepMulModPrecon(long b, long n)
{
   return double(b)/double(n);
}


//===================================


#define NTL_PD_FFT_THRESH (11)

#define PDLGSZ NTL_LG2_PDSZ
#define PDSZ NTL_PDSZ

#if (PDSZ == 8)
typedef PD<8> pd_full;
typedef PD<4> pd_half;
typedef PD<2> pd_qrtr;
#else
typedef PD<4> pd_full;
typedef PD<2> pd_half;
#endif

#define PDLD pd_full::load


// this assumes xx0, xx1, w, qinv are pd_half's
#define fwd_butterfly_half(xx0, xx1, w, q, wqinv)  \
do \
{ \
   pd_half x0_ = xx0; \
   pd_half x1_ = xx1; \
   pd_half t_  = pd_LazySubMod(x0_, x1_, q); \
   xx0 = pd_LazyAddMod2(x0_, x1_, q); \
   xx1 = pd_LazyMulModPrecon(t_, w, q, wqinv); \
}  \
while (0)

// this assumes xx0, xx1, w, qinv are pd_full's
#define fwd_butterfly_full(xx0, xx1, w, q, wqinv)  \
do \
{ \
   pd_full x0_ = xx0; \
   pd_full x1_ = xx1; \
   pd_full t_  = pd_LazySubMod(x0_, x1_, q); \
   xx0 = pd_LazyAddMod2(x0_, x1_, q); \
   xx1 = pd_LazyMulModPrecon(t_, w, q, wqinv); \
}  \
while (0)

// this assumes xx0_ptr, xx1_ptr, w_ptr, wqinv_ptr are double pointers
// which are read/written as pd_full's.  
// In gcc, restrict keyword will help code gen.
#define fwd_butterfly(xx0_ptr, xx1_ptr, w_ptr, q, wqinv_ptr)  \
do \
{ \
   pd_full x0_     = PDLD(xx0_ptr); \
   pd_full x1_     = PDLD(xx1_ptr); \
   pd_full w_      = PDLD(w_ptr); \
   pd_full wqinv_  = PDLD(wqinv_ptr); \
   pd_full t_      = pd_LazySubMod(x0_, x1_, q); \
   store(xx0_ptr, pd_LazyAddMod2(x0_, x1_, q)); \
   store(xx1_ptr, pd_LazyMulModPrecon(t_, w_, q, wqinv_)); \
}  \
while (0)


#if 0
#define fwd_butterfly_x4(xx0_ptr, xx1_ptr, w_ptr, q, wqinv_ptr)  \
do  \
{  \
   pd_full xx0_0_ = PDLD(xx0_ptr+0*PDSZ);  pd_full xx1_0_ = PDLD(xx1_ptr+0*PDSZ);  \
   pd_full xx0_1_ = PDLD(xx0_ptr+1*PDSZ);  pd_full xx1_1_ = PDLD(xx1_ptr+1*PDSZ);  \
   pd_full xx0_2_ = PDLD(xx0_ptr+2*PDSZ);  pd_full xx1_2_ = PDLD(xx1_ptr+2*PDSZ);  \
   pd_full xx0_3_ = PDLD(xx0_ptr+3*PDSZ);  pd_full xx1_3_ = PDLD(xx1_ptr+3*PDSZ);  \
   fwd_butterfly_full(xx0_0_, xx1_0_, PDLD(w_ptr+0*PDSZ), q, PDLD(wqinv_ptr+0*PDSZ));  \
   fwd_butterfly_full(xx0_1_, xx1_1_, PDLD(w_ptr+1*PDSZ), q, PDLD(wqinv_ptr+1*PDSZ));  \
   fwd_butterfly_full(xx0_2_, xx1_2_, PDLD(w_ptr+2*PDSZ), q, PDLD(wqinv_ptr+2*PDSZ));  \
   fwd_butterfly_full(xx0_3_, xx1_3_, PDLD(w_ptr+3*PDSZ), q, PDLD(wqinv_ptr+3*PDSZ));  \
   store(xx0_ptr+0*PDSZ, xx0_0_);  store(xx1_ptr+0*PDSZ, xx1_0_);  \
   store(xx0_ptr+1*PDSZ, xx0_1_);  store(xx1_ptr+1*PDSZ, xx1_1_);  \
   store(xx0_ptr+2*PDSZ, xx0_2_);  store(xx1_ptr+2*PDSZ, xx1_2_);  \
   store(xx0_ptr+3*PDSZ, xx0_3_);  store(xx1_ptr+3*PDSZ, xx1_3_);  \
}  \
while(0)
#else
#define fwd_butterfly_x4(xx0_ptr, xx1_ptr, w_ptr, q, wqinv_ptr)  \
do  \
{  \
   fwd_butterfly(xx0_ptr+0*PDSZ, xx1_ptr+0*PDSZ, w_ptr+0*PDSZ, q, wqinv_ptr+0*PDSZ);  \
   fwd_butterfly(xx0_ptr+1*PDSZ, xx1_ptr+1*PDSZ, w_ptr+1*PDSZ, q, wqinv_ptr+1*PDSZ);  \
   fwd_butterfly(xx0_ptr+2*PDSZ, xx1_ptr+2*PDSZ, w_ptr+2*PDSZ, q, wqinv_ptr+2*PDSZ);  \
   fwd_butterfly(xx0_ptr+3*PDSZ, xx1_ptr+3*PDSZ, w_ptr+3*PDSZ, q, wqinv_ptr+3*PDSZ);  \
}  \
while(0)
#endif




static inline NTL_ALWAYS_INLINE void
pd_fft_layer_inner_loop(double* NTL_RESTRICT xp0, 
                        double* NTL_RESTRICT xp1,
                        long size, 
                        const double* NTL_RESTRICT wtab, 
                        const double* NTL_RESTRICT wqinvtab, 
                        double q)

{
   long j = 0;
   do {
     fwd_butterfly_x4(xp0+j, xp1+j, wtab+j, q, wqinvtab+j);
     j += 4*PDSZ;
   } while (j < size);
}

// assumes size >= 8*PDSZ
static inline NTL_ALWAYS_INLINE void 
pd_fft_layer(double* xp, long blocks, long size,
	     const double* wtab, 
	     const double* wqinvtab, 
	     double q)
{
   size /= 2;

   do {
      pd_fft_layer_inner_loop(xp, xp+size, size, wtab, wqinvtab, q);
      xp += 2 * size;
   } while (--blocks != 0);
}


// size == 8*PDSZ
static inline NTL_ALWAYS_INLINE void
pd_fft_layer_size8(double* NTL_RESTRICT xp, long blocks,
		  const double* NTL_RESTRICT wtab, 
		  const double* NTL_RESTRICT wqinvtab, 
		  double q)
{
   do {
      fwd_butterfly_x4(xp+0*PDSZ, xp+4*PDSZ, wtab, q, wqinvtab);
      xp += 8*PDSZ;
   } while (--blocks != 0);
}

// size == 4*PDSZ
static inline NTL_ALWAYS_INLINE void
pd_fft_layer_size4(double* NTL_RESTRICT xp, long blocks,
		   const double* NTL_RESTRICT wtab, 
		   const double* NTL_RESTRICT wqinvtab, 
		   double q)
{
   do {
      fwd_butterfly(xp+0*PDSZ, xp+2*PDSZ, wtab+0*PDSZ, q, wqinvtab+0*PDSZ);
      fwd_butterfly(xp+1*PDSZ, xp+3*PDSZ, wtab+1*PDSZ, q, wqinvtab+1*PDSZ);

      fwd_butterfly(xp+4*PDSZ, xp+6*PDSZ, wtab+0*PDSZ, q, wqinvtab+0*PDSZ);
      fwd_butterfly(xp+5*PDSZ, xp+7*PDSZ, wtab+1*PDSZ, q, wqinvtab+1*PDSZ);

      xp += 8*PDSZ;
      blocks -= 2;
   } while (blocks != 0);
}

// size == 2*PDSZ
static inline NTL_ALWAYS_INLINE void
pd_fft_layer_size2(double* NTL_RESTRICT xp, long blocks,
		   const double* NTL_RESTRICT wtab, 
		   const double* NTL_RESTRICT wqinvtab, 
		   double q)
{
   do {
      fwd_butterfly(xp+0*PDSZ, xp+1*PDSZ, wtab+0*PDSZ, q, wqinvtab+0*PDSZ);
      fwd_butterfly(xp+2*PDSZ, xp+3*PDSZ, wtab+0*PDSZ, q, wqinvtab+0*PDSZ);
      fwd_butterfly(xp+4*PDSZ, xp+5*PDSZ, wtab+0*PDSZ, q, wqinvtab+0*PDSZ);
      fwd_butterfly(xp+6*PDSZ, xp+7*PDSZ, wtab+0*PDSZ, q, wqinvtab+0*PDSZ);

      xp += 8*PDSZ;
      blocks -= 4;
   } while (blocks != 0);
}

#if (PDSZ == 8)
static inline NTL_ALWAYS_INLINE void
pd_fft_layer_size1_one_block(double* x,
                   pd_half w8, pd_half w8qinv,
                   pd_full w4, pd_full w4qinv,
		   double q)
{
   pd_half x0 = pd_half::load(x);
   pd_half x1 = pd_half::load(x+PDSZ/2);
   fwd_butterfly_half(x0, x1, w8, q, w8qinv);
   pd_full y = join(x0, x1);

   y = pd_fwd_butterfly_packed4(y, w4, q, w4qinv);
   y = pd_fwd_butterfly_packed2(y, q);

   store(x, y);
}

// size == PDSZ == 8
// processes last three levels, of size 8, 4, and 2.
static inline NTL_ALWAYS_INLINE void
pd_fft_layer_size1(double* xp, long blocks,
                   const double **w_pp, const double **wqinv_pp,
		   double q)
{
   const double *w8_ptr = *w_pp;
   const double *w8qinv_ptr = *wqinv_pp;

   const double *w4_ptr = *(w_pp-1);
   const double *w4qinv_ptr = *(wqinv_pp-1);

   pd_half w8 = pd_half::load(w8_ptr);

   pd_half w8qinv = pd_half::load(w8qinv_ptr);

   
   pd_qrtr w4_qrtr = pd_qrtr::load(w4_ptr);
   pd_half w4_half = join(w4_qrtr, w4_qrtr);
   pd_full w4      = join(w4_half, w4_half);
   w4 = blend4(dup2even(w4), w4);

   pd_qrtr w4qinv_qrtr = pd_qrtr::load(w4qinv_ptr);
   pd_half w4qinv_half = join(w4qinv_qrtr, w4qinv_qrtr);
   pd_full w4qinv      = join(w4qinv_half, w4qinv_half);
   w4qinv = blend4(dup2even(w4qinv), w4qinv);

   do {
      pd_fft_layer_size1_one_block(xp+0*PDSZ, w8, w8qinv, w4, w4qinv, q);
      pd_fft_layer_size1_one_block(xp+1*PDSZ, w8, w8qinv, w4, w4qinv, q);
      pd_fft_layer_size1_one_block(xp+2*PDSZ, w8, w8qinv, w4, w4qinv, q);
      pd_fft_layer_size1_one_block(xp+3*PDSZ, w8, w8qinv, w4, w4qinv, q);

      xp += 4*PDSZ;
      blocks -= 4;
   } while (blocks != 0);
}
#else
// PDSZ == 4

static inline NTL_ALWAYS_INLINE void
pd_fft_layer_size1_one_block(double* x,
                   pd_half w4, pd_half w4qinv,
		   double q)
{
   pd_half x0 = pd_half::load(x);
   pd_half x1 = pd_half::load(x+PDSZ/2);
   fwd_butterfly_half(x0, x1, w4, q, w4qinv);
   pd_full y = join(x0, x1);

   y = pd_fwd_butterfly_packed2(y, q);

   store(x, y);
}

// size == PDSZ == 4
// processes last two levels, of size 4 and 2.
static inline NTL_ALWAYS_INLINE void
pd_fft_layer_size1(double* xp, long blocks,
                   const double **w_pp, const double **wqinv_pp,
		   double q)
{
   const double *w4_ptr = *w_pp;
   const double *w4qinv_ptr = *wqinv_pp;


   pd_half w4 = pd_half::load(w4_ptr);
   pd_half w4qinv = pd_half::load(w4qinv_ptr);

   
   do {
      pd_fft_layer_size1_one_block(xp+0*PDSZ, w4, w4qinv, q);
      pd_fft_layer_size1_one_block(xp+1*PDSZ, w4, w4qinv, q);
      pd_fft_layer_size1_one_block(xp+2*PDSZ, w4, w4qinv, q);
      pd_fft_layer_size1_one_block(xp+3*PDSZ, w4, w4qinv, q);

      xp += 4*PDSZ;
      blocks -= 4;
   } while (blocks != 0);
}

#endif
       
      


void 
pd_fft_base(double* xp, long lgN, const pd_mod_t& mod)
{
  double q = mod.q;
  const double** wtab = mod.wtab;
  const double** wqinvtab = mod.wqinvtab;

  long N = 1L << lgN;

  long j, size, blocks;
  for (j = lgN, size = N, blocks = 1; 
       size > 8*PDSZ; j--, blocks <<= 1, size >>= 1)
    pd_fft_layer(xp, blocks, size, wtab[j], wqinvtab[j], q);

  pd_fft_layer_size8(xp, blocks, wtab[j], wqinvtab[j], q);  
  j--, blocks <<= 1, size >>= 1;

  pd_fft_layer_size4(xp, blocks, wtab[j], wqinvtab[j], q);  
  j--, blocks <<= 1, size >>= 1;

  pd_fft_layer_size2(xp, blocks, wtab[j], wqinvtab[j], q);  
  j--, blocks <<= 1, size >>= 1;

  pd_fft_layer_size1(xp, blocks, wtab+j, wqinvtab+j, q);  

}

static inline NTL_ALWAYS_INLINE void 
pd_move(double *x, const long *a)
{
   pd_full r;
   loadu(r, a);
   store(x, r);
}

static inline NTL_ALWAYS_INLINE void 
pd_move(long *x, const double *a)
{
   pd_full r;
   load(r, a);
   storeu(x, r);
}

static inline NTL_ALWAYS_INLINE void 
pd_reduce1_move(long *x, const double *a, double q)
{
   pd_full r;
   load(r, a);
   r = pd_LazyReduce1(r, q);
   storeu(x, r);
}

static inline NTL_ALWAYS_INLINE void 
pd_reduce2_move(long *x, const double *a, double q)
{
   pd_full r;
   load(r, a);
   r = pd_LazyReduce2(r, q);
   r = pd_LazyReduce1(r, q);
   storeu(x, r);
}

static inline NTL_ALWAYS_INLINE void 
pd_mul_move(long *x, const double *a, pd_full b, double q, pd_full bqinv)
{
   pd_full r;
   load(r, a);
   r = pd_LazyMulModPrecon(r, b, q, bqinv);
   r = pd_LazyReduce1(r, q);
   storeu(x, r);
}




static
void pd_fft_short(double* xp, long yn, long xn, long lgN, 
                   const pd_mod_t& mod)
{
  long N = 1L << lgN;

  if (yn == N)
    {
      if (xn == N && lgN <= NTL_PD_FFT_THRESH)
	{
	  // no truncation
	  pd_fft_base(xp, lgN, mod);
	  return;
	}
    }

  // divide-and-conquer algorithm

  long half = N >> 1;
  double q = mod.q;

  if (yn <= half)
    {
      if (xn <= half)
	{
	  pd_fft_short(xp, yn, xn, lgN - 1, mod);
	}
      else
	{
	  xn -= half;

	  // (X, Y) -> X + Y
	  for (long j = 0; j < xn; j+=PDSZ)
	    store(xp+j, pd_LazyAddMod2(PDLD(xp+j), PDLD(xp+j+half), q));

	  pd_fft_short(xp, yn, half, lgN - 1, mod);
	}
    }
  else
    {
      yn -= half;
      
      double* xp0 = xp;
      double* xp1 = xp + half;
      const double* wtab = mod.wtab[lgN];
      const double* wqinvtab = mod.wqinvtab[lgN];

      if (xn <= half)
	{
	  // X -> (X, w*X)
	  for (long j = 0; j < xn; j+=PDSZ)
	    store(xp1+j, pd_LazyMulModPrecon(PDLD(xp0+j), PDLD(wtab+j), q, PDLD(wqinvtab+j)));

	  pd_fft_short(xp0, half, xn, lgN - 1, mod);
	  pd_fft_short(xp1, yn, xn, lgN - 1, mod);
	}
      else
	{
	  xn -= half;

	  // (X, Y) -> (X + Y, w*(X - Y))
          pd_fft_layer_inner_loop(xp0, xp1, xn, wtab, wqinvtab, q);

	  // X -> (X, w*X)
	  for (long j = xn; j < half; j+=PDSZ)
	    store(xp1+j, pd_LazyMulModPrecon(PDLD(xp0+j), PDLD(wtab+j), q, PDLD(wqinvtab+j)));

	  pd_fft_short(xp0, half, half, lgN - 1, mod);
	  pd_fft_short(xp1, yn, half, lgN - 1, mod);
	}
    }
}


void 
pd_fft_trunc_impl(long* A, const long* a, double* xp, long lgN, const pd_mod_t& mod,
                  long yn, long xn)
           
{
   for (long i = 0; i < xn; i += 4*PDSZ) {
      pd_move(xp+i+0*PDSZ, a+i+0*PDSZ);
      pd_move(xp+i+1*PDSZ, a+i+1*PDSZ);
      pd_move(xp+i+2*PDSZ, a+i+2*PDSZ);
      pd_move(xp+i+3*PDSZ, a+i+3*PDSZ);
   }

   pd_fft_short(xp, yn, xn, lgN, mod);

   double q = mod.q;
   for (long i = 0; i < yn; i += 4*PDSZ) {
      pd_reduce1_move(A+i+0*PDSZ, xp+i+0*PDSZ, q);
      pd_reduce1_move(A+i+1*PDSZ, xp+i+1*PDSZ, q);
      pd_reduce1_move(A+i+2*PDSZ, xp+i+2*PDSZ, q);
      pd_reduce1_move(A+i+3*PDSZ, xp+i+3*PDSZ, q);
   }
}


void 
pd_fft_trunc_impl(long* A, const long* a, double* xp, long lgN, const pd_mod_t& mod,
                  long yn, long xn, double fac)
           
{
   for (long i = 0; i < xn; i += 4*PDSZ) {
      pd_move(xp+i+0*PDSZ, a+i+0*PDSZ);
      pd_move(xp+i+1*PDSZ, a+i+1*PDSZ);
      pd_move(xp+i+2*PDSZ, a+i+2*PDSZ);
      pd_move(xp+i+3*PDSZ, a+i+3*PDSZ);
   }

   pd_fft_short(xp, yn, xn, lgN, mod);

   double q = mod.q;
   double facqinv = fac/q; 
   for (long i = 0; i < yn; i += 4*PDSZ) {
      pd_mul_move(A+i+0*PDSZ, xp+i+0*PDSZ, fac, q, facqinv);
      pd_mul_move(A+i+1*PDSZ, xp+i+1*PDSZ, fac, q, facqinv);
      pd_mul_move(A+i+2*PDSZ, xp+i+2*PDSZ, fac, q, facqinv);
      pd_mul_move(A+i+3*PDSZ, xp+i+3*PDSZ, fac, q, facqinv);
   }
}

//================ ifft ==============

// return (a[0] + a[1], a[0] - a[1], a[2] + a[3], a[2] - a[3], ...)
// all inputs and outputs in [0, 4*n)
template<class pd> pd
pd_inv_butterfly_packed2(pd a, double n)
{
   a = pd_LazyReduce2(a, n);
   pd b = swap2(a);
   pd sum = pd_LazyAddMod(a, b, n);
   pd diff = pd_LazySubMod(b, a, n);
   pd res = blend2(sum, diff);
   return res;
}

// return (a[0] + a[2], a[1] + a[3]*root, a[0] - a[2], a[1] - a[3]*root, ...) 
// all inputs and outputs in [0, 4*n)
// it is also assumed that w = (1,1,1,root,...) and wninv = RoundDown(w/n)
template<class pd> pd
pd_inv_butterfly_packed4(pd a, pd w, double n, pd wninv)
{
   a = pd_LazyMulModPrecon(a, w, n, wninv);
   pd b = swap4(a);
   pd sum = pd_LazyAddMod(a, b, n);
   pd diff = pd_LazySubMod(b, a, n);
   pd res = blend4(sum, diff);
   return res;
}

#define inv_butterfly_half(xx0, xx1, w, q, wqinv)  \
do  \
{  \
   pd_half x0_ = pd_LazyReduce2(xx0, q);  \
   pd_half x1_ = xx1;  \
   pd_half t_ = pd_LazyMulModPrecon(x1_, w, q, wqinv);   \
   xx0 = pd_LazyAddMod(x0_, t_, q);    \
   xx1 = pd_LazySubMod(x0_, t_, q);    \
} while (0)


#define inv_butterfly(xx0_ptr, xx1_ptr, w_ptr, q, wqinv_ptr)  \
do  \
{  \
   pd_full x0_ = pd_LazyReduce2(PDLD(xx0_ptr), q);  \
   pd_full x1_ = PDLD(xx1_ptr);  \
   pd_full t_ = pd_LazyMulModPrecon(x1_, PDLD(w_ptr), q, PDLD(wqinv_ptr));   \
   store(xx0_ptr, pd_LazyAddMod(x0_, t_, q));    \
   store(xx1_ptr, pd_LazySubMod(x0_, t_, q));    \
} while (0)

#define inv_butterfly_x4(xx0_ptr, xx1_ptr, w_ptr, q, wqinv_ptr)  \
do  \
{  \
   inv_butterfly(xx0_ptr+0*PDSZ, xx1_ptr+0*PDSZ, w_ptr+0*PDSZ, q, wqinv_ptr+0*PDSZ);  \
   inv_butterfly(xx0_ptr+1*PDSZ, xx1_ptr+1*PDSZ, w_ptr+1*PDSZ, q, wqinv_ptr+1*PDSZ);  \
   inv_butterfly(xx0_ptr+2*PDSZ, xx1_ptr+2*PDSZ, w_ptr+2*PDSZ, q, wqinv_ptr+2*PDSZ);  \
   inv_butterfly(xx0_ptr+3*PDSZ, xx1_ptr+3*PDSZ, w_ptr+3*PDSZ, q, wqinv_ptr+3*PDSZ);  \
}  \
while(0)

static inline NTL_ALWAYS_INLINE void
pd_ifft_layer_inner_loop(double* NTL_RESTRICT xp0, 
                         double* NTL_RESTRICT xp1,
                         long size, 
                         const double* NTL_RESTRICT wtab, 
                         const double* NTL_RESTRICT wqinvtab, 
                         double q)

{
   long j = 0;
   do {
     inv_butterfly_x4(xp0+j, xp1+j, wtab+j, q, wqinvtab+j);
     j += 4*PDSZ;
   } while (j < size);
}

// assumes size >= 8*PDSZ
static inline NTL_ALWAYS_INLINE void 
pd_ifft_layer(double* xp, long blocks, long size,
	      const double* wtab, 
	      const double* wqinvtab, 
	      double q)
{
   size /= 2;

   do {
      pd_ifft_layer_inner_loop(xp, xp+size, size, wtab, wqinvtab, q);
      xp += 2 * size;
   } while (--blocks != 0);
}

// size == 8*PDSZ
static inline NTL_ALWAYS_INLINE void
pd_ifft_layer_size8(double* NTL_RESTRICT xp, long blocks,
		    const double* NTL_RESTRICT wtab, 
		    const double* NTL_RESTRICT wqinvtab, 
		    double q)
{
   do {
      inv_butterfly_x4(xp+0*PDSZ, xp+4*PDSZ, wtab, q, wqinvtab);
      xp += 8*PDSZ;
   } while (--blocks != 0);
}

// size == 4*PDSZ
static inline NTL_ALWAYS_INLINE void
pd_ifft_layer_size4(double* NTL_RESTRICT xp, long blocks,
		    const double* NTL_RESTRICT wtab, 
		    const double* NTL_RESTRICT wqinvtab, 
		    double q)
{
   do {
      inv_butterfly(xp+0*PDSZ, xp+2*PDSZ, wtab+0*PDSZ, q, wqinvtab+0*PDSZ);
      inv_butterfly(xp+1*PDSZ, xp+3*PDSZ, wtab+1*PDSZ, q, wqinvtab+1*PDSZ);

      inv_butterfly(xp+4*PDSZ, xp+6*PDSZ, wtab+0*PDSZ, q, wqinvtab+0*PDSZ);
      inv_butterfly(xp+5*PDSZ, xp+7*PDSZ, wtab+1*PDSZ, q, wqinvtab+1*PDSZ);

      xp += 8*PDSZ;
      blocks -= 2;
   } while (blocks != 0);
}

// size == 2*PDSZ
static inline NTL_ALWAYS_INLINE void
pd_ifft_layer_size2(double* NTL_RESTRICT xp, long blocks,
		    const double* NTL_RESTRICT wtab, 
		    const double* NTL_RESTRICT wqinvtab, 
		    double q)
{
   do {
      inv_butterfly(xp+0*PDSZ, xp+1*PDSZ, wtab+0*PDSZ, q, wqinvtab+0*PDSZ);
      inv_butterfly(xp+2*PDSZ, xp+3*PDSZ, wtab+0*PDSZ, q, wqinvtab+0*PDSZ);
      inv_butterfly(xp+4*PDSZ, xp+5*PDSZ, wtab+0*PDSZ, q, wqinvtab+0*PDSZ);
      inv_butterfly(xp+6*PDSZ, xp+7*PDSZ, wtab+0*PDSZ, q, wqinvtab+0*PDSZ);

      xp += 8*PDSZ;
      blocks -= 4;
   } while (blocks != 0);
}

#if (PDSZ == 8)
static inline NTL_ALWAYS_INLINE void
pd_ifft_layer_size1_one_block(double* x,
                   pd_half w8, pd_half w8qinv,
                   pd_full w4, pd_full w4qinv,
		   double q)
{
   pd_full y = PDLD(x);
   y = pd_inv_butterfly_packed2(y, q);
   y = pd_inv_butterfly_packed4(y, w4, q, w4qinv);

   pd_half x0 = get_lo(y);
   pd_half x1 = get_hi(y);
   inv_butterfly_half(x0, x1, w8, q, w8qinv);
   
   store(x, x0);
   store(x+PDSZ/2, x1);
}

// size == PDSZ == 8
// processes last three levels, of size 8, 4, and 2.
static inline NTL_ALWAYS_INLINE void
pd_ifft_layer_size1(double* xp, long blocks,
                   const double **w_pp, const double **wqinv_pp,
		   double q)
{
   const double *w8_ptr = *w_pp;
   const double *w8qinv_ptr = *wqinv_pp;

   const double *w4_ptr = *(w_pp-1);
   const double *w4qinv_ptr = *(wqinv_pp-1);

   pd_half w8 = pd_half::load(w8_ptr);

   pd_half w8qinv = pd_half::load(w8qinv_ptr);

   
   pd_qrtr w4_qrtr = pd_qrtr::load(w4_ptr);
   pd_half w4_half = join(w4_qrtr, w4_qrtr);
   pd_full w4      = join(w4_half, w4_half);
   w4 = blend4(dup2even(w4), w4);

   pd_qrtr w4qinv_qrtr = pd_qrtr::load(w4qinv_ptr);
   pd_half w4qinv_half = join(w4qinv_qrtr, w4qinv_qrtr);
   pd_full w4qinv      = join(w4qinv_half, w4qinv_half);
   w4qinv = blend4(dup2even(w4qinv), w4qinv);

   do {
      pd_ifft_layer_size1_one_block(xp+0*PDSZ, w8, w8qinv, w4, w4qinv, q);
      pd_ifft_layer_size1_one_block(xp+1*PDSZ, w8, w8qinv, w4, w4qinv, q);
      pd_ifft_layer_size1_one_block(xp+2*PDSZ, w8, w8qinv, w4, w4qinv, q);
      pd_ifft_layer_size1_one_block(xp+3*PDSZ, w8, w8qinv, w4, w4qinv, q);

      xp += 4*PDSZ;
      blocks -= 4;
   } while (blocks != 0);
}
#else
// PDSZ == 4

static inline NTL_ALWAYS_INLINE void
pd_ifft_layer_size1_one_block(double* x,
                   pd_half w4, pd_half w4qinv,
		   double q)
{
   pd_full y = PDLD(x);
   y = pd_inv_butterfly_packed2(y, q);

   pd_half x0 = get_lo(y);
   pd_half x1 = get_hi(y);
   inv_butterfly_half(x0, x1, w4, q, w4qinv);
   
   store(x, x0);
   store(x+PDSZ/2, x1);
}

// size == PDSZ == 4
// processes last two levels, of size 4 and 2.
static inline NTL_ALWAYS_INLINE void
pd_ifft_layer_size1(double* xp, long blocks,
                   const double **w_pp, const double **wqinv_pp,
		   double q)
{
   const double *w4_ptr = *w_pp;
   const double *w4qinv_ptr = *wqinv_pp;


   pd_half w4 = pd_half::load(w4_ptr);
   pd_half w4qinv = pd_half::load(w4qinv_ptr);

   
   do {
      pd_ifft_layer_size1_one_block(xp+0*PDSZ, w4, w4qinv, q);
      pd_ifft_layer_size1_one_block(xp+1*PDSZ, w4, w4qinv, q);
      pd_ifft_layer_size1_one_block(xp+2*PDSZ, w4, w4qinv, q);
      pd_ifft_layer_size1_one_block(xp+3*PDSZ, w4, w4qinv, q);

      xp += 4*PDSZ;
      blocks -= 4;
   } while (blocks != 0);
}

#endif
       
void 
pd_ifft_base(double* xp, long lgN, const pd_mod_t& mod)
{
  double q = mod.q;
  const double** wtab = mod.wtab;
  const double** wqinvtab = mod.wqinvtab;

  long N = 1L << lgN;

  long j=PDLGSZ, size=PDSZ, blocks=N/PDSZ;

  pd_ifft_layer_size1(xp, blocks, wtab+j, wqinvtab+j, q);  
  j++, blocks >>= 1, size <<= 1;

  pd_ifft_layer_size2(xp, blocks, wtab[j], wqinvtab[j], q);  
  j++, blocks >>= 1, size <<= 1;

  pd_ifft_layer_size4(xp, blocks, wtab[j], wqinvtab[j], q);  
  j++, blocks >>= 1, size <<= 1;

  pd_ifft_layer_size8(xp, blocks, wtab[j], wqinvtab[j], q);  
  j++, blocks >>= 1, size <<= 1;

  for (; size <= N; j++, blocks >>= 1, size <<= 1)
    pd_ifft_layer(xp, blocks, size, wtab[j], wqinvtab[j], q);

}

static void 
pd_ifft_short2(double* xp, long yn, long lgN, const pd_mod_t& mod);


static void 
pd_ifft_short1(double* xp, long yn, long lgN, const pd_mod_t& mod)

// Implements truncated inverse FFT interface, but with xn==yn.
// All computations are done in place.

{
  long N = 1L << lgN;

  if (yn == N && lgN <= NTL_PD_FFT_THRESH)
    {
      // no truncation
      pd_ifft_base(xp, lgN, mod);
      return;
    }

  // divide-and-conquer algorithm

  long half = N >> 1;
  double q = mod.q;

  if (yn <= half)
    {
      // X -> 2X
      for (long j = 0; j < yn; j+=PDSZ)
      	store(xp+j, pd_LazyDoubleMod4(PDLD(xp+j), q));

      pd_ifft_short1(xp, yn, lgN - 1, mod);
    }
  else
    {
      double* xp0 = xp;
      double* xp1 = xp + half;

      pd_ifft_short1(xp0, half, lgN - 1, mod);

      yn -= half;

      if (yn < half) {
        const double* wtab1 = mod.wtab1[lgN];
        const double* wqinvtab1 = mod.wqinvtab1[lgN];

	// X -> (2X, w*X)
	for (long j = yn; j < half; j+=PDSZ)
	  {
	    pd_full x0 = PDLD(xp0+j);
	    store(xp0+j, pd_LazyDoubleMod4(x0, q));
	    store(xp1+j, pd_LazyMulModPrecon(x0, PDLD(wtab1+j), q, PDLD(wqinvtab1+j)));
	  }
      }

      pd_ifft_short2(xp1, yn, lgN - 1, mod);

      // (X, Y) -> (X + Y/w, X - Y/w)
      pd_ifft_layer_inner_loop(xp0, xp1, yn, mod.wtab[lgN], mod.wqinvtab[lgN], q); 
    }
}


static void 
pd_ifft_short2(double* xp, long yn, long lgN, const pd_mod_t& mod)

// Implements truncated inverse FFT interface, but with xn==N.
// All computations are done in place.

{
  long N = 1L << lgN;

  if (yn == N && lgN <= NTL_PD_FFT_THRESH)
    {
      // no truncation
      pd_ifft_base(xp, lgN, mod);
      return;
    }

  // divide-and-conquer algorithm

  long half = N >> 1;
  double q = mod.q;

  if (yn <= half)
    {
      // X -> 2X
      for (long j = 0; j < yn; j+=PDSZ)
     	store(xp+j, pd_LazyDoubleMod4(PDLD(xp+j), q));

      // (X, Y) -> X + Y
      for (long j = yn; j < half; j+=PDSZ)
	store(xp+j, pd_LazyAddMod4(PDLD(xp+j), PDLD(xp+j+half), q));

      pd_ifft_short2(xp, yn, lgN - 1, mod);

      // (X, Y) -> X - Y
      for (long j = 0; j < yn; j+=PDSZ)
	store(xp+j, pd_LazySubMod4(PDLD(xp+j), PDLD(xp+j+half), q));
    }
  else
    {
      double* xp0 = xp;
      double* xp1 = xp + half;

      pd_ifft_short1(xp0, half, lgN - 1, mod);

      yn -= half;


      if (yn < half) {
        const double* wtab1 = mod.wtab1[lgN];
        const double* wqinvtab1 = mod.wqinvtab1[lgN];

	// (X, Y) -> (2X - Y, w*(X - Y))
	for (long j = yn; j < half; j+=PDSZ)
	  {
	    pd_full x0 = PDLD(xp0+j);
	    pd_full x1 = PDLD(xp1+j);
	    pd_full u  = pd_LazySubMod4(x0, x1, q);
	    store(xp0+j, pd_LazyAddMod4(x0, u, q));
	    store(xp1+j, pd_LazyMulModPrecon(u, PDLD(wtab1+j), q, PDLD(wqinvtab1+j)));
	  }
      }

      pd_ifft_short2(xp1, yn, lgN - 1, mod);

      // (X, Y) -> (X + Y/w, X - Y/w)
      pd_ifft_layer_inner_loop(xp0, xp1, yn, mod.wtab[lgN], mod.wqinvtab[lgN], q); 
    }
}


void 
pd_ifft_trunc_impl(long* A, const long* a, double* xp, long lgN, const pd_mod_t& mod, 
                   long yn, double fac)
{
   long N = 1L << lgN;

   for (long i = 0; i < yn; i += 4*PDSZ) {
      pd_move(xp+i+0*PDSZ, a+i+0*PDSZ);
      pd_move(xp+i+1*PDSZ, a+i+1*PDSZ);
      pd_move(xp+i+2*PDSZ, a+i+2*PDSZ);
      pd_move(xp+i+3*PDSZ, a+i+3*PDSZ);
   }

   pd_ifft_short1(xp, yn, lgN, mod);

   double q = mod.q;
   double facqinv = fac/q; 
   for (long i = 0; i < yn; i += 4*PDSZ) {
      pd_mul_move(A+i+0*PDSZ, xp+i+0*PDSZ, fac, q, facqinv);
      pd_mul_move(A+i+1*PDSZ, xp+i+1*PDSZ, fac, q, facqinv);
      pd_mul_move(A+i+2*PDSZ, xp+i+2*PDSZ, fac, q, facqinv);
      pd_mul_move(A+i+3*PDSZ, xp+i+3*PDSZ, fac, q, facqinv);
   }
}


void 
pd_ifft_trunc_impl(long* A, const long* a, double* xp, long lgN, const pd_mod_t& mod, 
                   long yn)
{
   long N = 1L << lgN;

   for (long i = 0; i < yn; i += 4*PDSZ) {
      pd_move(xp+i+0*PDSZ, a+i+0*PDSZ);
      pd_move(xp+i+1*PDSZ, a+i+1*PDSZ);
      pd_move(xp+i+2*PDSZ, a+i+2*PDSZ);
      pd_move(xp+i+3*PDSZ, a+i+3*PDSZ);
   }

   pd_ifft_short1(xp, yn, lgN, mod);

   double q = mod.q;
   for (long i = 0; i < yn; i += 4*PDSZ) {
      pd_reduce2_move(A+i+0*PDSZ, xp+i+0*PDSZ, q);
      pd_reduce2_move(A+i+1*PDSZ, xp+i+1*PDSZ, q);
      pd_reduce2_move(A+i+2*PDSZ, xp+i+2*PDSZ, q);
      pd_reduce2_move(A+i+3*PDSZ, xp+i+3*PDSZ, q);
   }
}

NTL_END_IMPL

#else

void _ntl_pd_FFT_dummy() { }

#endif
