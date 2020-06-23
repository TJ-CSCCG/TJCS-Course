

#ifndef NTL_sp_arith__H
#define NTL_sp_arith__H


/****************************************************************

    Single-precision modular arithmetic

*****************************************************************/


/*
these routines implement single-precision modular arithmetic.
If n is the modulus, all inputs should be in the range 0..n-1.
The number n itself should be in the range 1..2^{NTL_SP_NBITS}-1.
*/


// DIRT: undocumented feature: in all of these MulMod routines,
// the first argument, a, need only be in the range
// 0..2^{NTL_SP_NBITS}-1.  This is assumption is used internally
// in some NT routines...I've tried to mark all such uses with a
// DIRT comment.  I may decide to make this feature part
// of the documented interface at some point in the future.

// NOTE: this header file is for internal use only, via the ZZ.h header.
// It is also used in the LIP implementation files c/g_lip_impl.h.


#include <NTL/lip.h>
#include <NTL/tools.h>


NTL_OPEN_NNS


#define NTL_HAVE_MULMOD_T

#if (defined(NTL_SPMM_ULL) && defined(NTL_HAVE_LL_TYPE))
// we only honor SPMM_ULL if we have LL
#define NTL_SPMM_ULL_VIABLE

#elif (defined(NTL_SPMM_ULL) && defined(NTL_WIZARD_HACK))
// raise an error when running the wizard and we cannot honor SPMM_ULL
#error "cannot honor NTL_SPMM_ULL"

#endif



#if 0
// the following code can be used to use new-style clients with old versions
// of NTL


#ifndef NTL_HAVE_MULMOD_T

NTL_OPEN_NNS

typedef double mulmod_t;
typedef double muldivrem_t;


inline double PrepMulMod(long n)
{
   return double(1L)/double(n);
}

inline double PrepMulDivRem(long b, long n, double ninv)
{
   return double(b)*ninv;
}

inline double PrepMulDivRem(long b, long n)
{
   return double(b)/double(n);
}


inline double PrepMulModPrecon(long b, long n)
{
   return PrepMulModPrecon(b, n, PrepMulMod(n));
}

NTL_CLOSE_NNS



#endif



#endif







/*********************************************************


HELPER ROUTINES:

long sp_SignMask(long a) 
long sp_SignMask(unsigned long a)
// if (long(a) < 0) then -1 else 0

bool sp_Negative(unsigned long a)
// long(a) < 0

long sp_CorrectDeficit(long a, long n)
long sp_CorrectDeficit(unsigned long a, long n): 
// if (long(a) >= 0) then a else a+n

// it is assumed that n in (0..B), where B = 2^(NTL_BITS_PER_LONG-1),
// and that long(a) >= -n 

long sp_CorrectExcess(long a, long n) 
long sp_CorrectExcess(unsigned long a, long n): 
// if (a < n) then a else a-n

// For the signed version, it is assumed that a >= 0.
// In either version, it is assumed that 
//   n in (0..B) and a-n in (-B..B).


These are designed to respect the flags NTL_CLEAN_INT,
NTL_ARITH_RIGHT_SHIFT, and NTL_AVOID_BRANCHING.


*********************************************************/


#if (NTL_ARITH_RIGHT_SHIFT && !defined(NTL_CLEAN_INT))
// DIRT: IMPL-DEF: arithmetic right shift and cast unsigned to signed

inline 
long sp_SignMask(long a)
{
   return a >> (NTL_BITS_PER_LONG-1);
}

inline 
long sp_SignMask(unsigned long a)
{
   return cast_signed(a) >> (NTL_BITS_PER_LONG-1);
}
#else
inline 
long sp_SignMask(long a)
{
   return -long(cast_unsigned(a) >> (NTL_BITS_PER_LONG-1));
}

inline 
long sp_SignMask(unsigned long a)
{
   return -long(a >> (NTL_BITS_PER_LONG-1));
}
#endif

inline
bool sp_Negative(unsigned long a)
{
   return cast_signed(a) < 0;
}



#if (!defined(NTL_AVOID_BRANCHING))

// The C++ code is written using branching, but  
// on machines with large branch penalties, this code
// should yield "predicated instructions" (i.e., on x86,
// conditional moves).  The "branching" version of sp_CorrectExcess
// in written in a particular way to get optimal machine code:
// subtract, cmove (tested on clang, gcc, icc).

inline 
long sp_CorrectDeficit(long a, long n)
{
   return a >= 0 ? a : a+n;
}

template<class T> 
long sp_CorrectDeficitQuo(T& q, long a, long n, long amt=1)
{
   return a >= 0 ? a : (q -= amt, a+n);
}



inline 
long sp_CorrectDeficit(unsigned long a, long n)
{
   return !sp_Negative(a) ? a : a+n;
}

template<class T> 
long sp_CorrectDeficitQuo(T& q, unsigned long a, long n, long amt=1)
{
   return !sp_Negative(a) ? a : (q -= amt, a+n);
}



inline 
long sp_CorrectExcess(long a, long n)
{
   return a-n >= 0 ? a-n : a;
}

template<class T> 
long sp_CorrectExcessQuo(T& q, long a, long n, long amt=1)
{
   return a-n >= 0 ? (q += amt, a-n) : a;
}



inline 
long sp_CorrectExcess(unsigned long a, long n)
{
   return !sp_Negative(a-n) ? a-n : a;
}

template<class T> 
long sp_CorrectExcessQuo(T& q, unsigned long a, long n, long amt=1)
{
   return !sp_Negative(a-n) ? (q += amt, a-n) : a;
}

#else


// This C++ code uses traditional masking and adding
// to avoid branching. 

inline 
long sp_CorrectDeficit(long a, long n)
{
   return a + (sp_SignMask(a) & n);
}

template<class T> 
long sp_CorrectDeficitQuo(T& q, long a, long n, long amt=1)
{
   q += sp_SignMask(a)*amt;
   return a + (sp_SignMask(a) & n);
}





inline 
long sp_CorrectDeficit(unsigned long a, long n)
{
   return a + (sp_SignMask(a) & n);
}

template<class T> 
long sp_CorrectDeficitQuo(T& q, unsigned long a, long n, long amt=1)
{
   q += sp_SignMask(a)*amt;
   return a + (sp_SignMask(a) & n);
}




inline 
long sp_CorrectExcess(long a, long n)
{
   return (a-n) + (sp_SignMask(a-n) & n);
}

template<class T> 
long sp_CorrectExcessQuo(T& q, long a, long n, long amt=1)
{
   q += (1L + sp_SignMask(a-n))*amt;
   return (a-n) + (sp_SignMask(a-n) & n);
}




inline 
long sp_CorrectExcess(unsigned long a, long n)
{
   return (a-n) + (sp_SignMask(a-n) & n);
}

template<class T> 
long sp_CorrectExcessQuo(T& q, unsigned long a, long n, long amt=1)
{
   q += (1L + sp_SignMask(a-n))*amt;
   return (a-n) + (sp_SignMask(a-n) & n);
}

#endif


// **********************************************************************





#ifdef NTL_HAVE_BUILTIN_CLZL

inline long 
sp_CountLeadingZeros(unsigned long x)
{
   return __builtin_clzl(x);
}

#else

inline long 
sp_CountLeadingZeros(unsigned long x)
{
   long res = NTL_BITS_PER_LONG-NTL_SP_NBITS;
   x = x << (NTL_BITS_PER_LONG-NTL_SP_NBITS);
   while (x < (1UL << (NTL_BITS_PER_LONG-1))) {
      x <<= 1;
      res++;
   }

   return res;
}


#endif




inline 
long AddMod(long a, long b, long n)
{
   long r = a+b;
   return sp_CorrectExcess(r, n);
}

inline 
long SubMod(long a, long b, long n)
{
   long r = a-b;
   return sp_CorrectDeficit(r, n);
}

inline 
long NegateMod(long a, long n)
{
   return SubMod(0, a, n);
}





#if (!defined(NTL_LONGLONG_SP_MULMOD))



#ifdef NTL_LEGACY_SP_MULMOD

#define NTL_WIDE_DOUBLE_PRECISION NTL_DOUBLE_PRECISION
#define NTL_WIDE_FDOUBLE_PRECISION NTL_WIDE_DOUBLE_DP
typedef double wide_double;


#else


#ifdef NTL_LONGDOUBLE_SP_MULMOD


#define NTL_WIDE_DOUBLE_PRECISION NTL_LONGDOUBLE_PRECISION
#define NTL_WIDE_FDOUBLE_PRECISION NTL_WIDE_DOUBLE_LDP
typedef long double wide_double_impl_t;

#else

#define NTL_WIDE_DOUBLE_PRECISION NTL_DOUBLE_PRECISION
#define NTL_WIDE_FDOUBLE_PRECISION NTL_WIDE_DOUBLE_DP
typedef double wide_double_impl_t;

#endif




class wide_double {
public:
   wide_double_impl_t data;

   wide_double() { }

   wide_double(const wide_double& x) : data(x.data) { }

   template<class T>
   explicit wide_double(const T& x) : data(x) { }

   operator wide_double_impl_t() const { return data; }

};

inline wide_double operator+(wide_double x, wide_double y)
{
   return wide_double(x.data + y.data); 
}

inline wide_double operator-(wide_double x, wide_double y)
{
   return wide_double(x.data - y.data); 
}



inline wide_double operator*(wide_double x, wide_double y)
{
   return wide_double(x.data * y.data); 
}

inline wide_double operator/(wide_double x, wide_double y)
{
   return wide_double(x.data / y.data); 
}

inline wide_double floor(wide_double x)
{
   return wide_double(std::floor(x.data));
}

inline wide_double& operator+=(wide_double& x, wide_double y)
{
   return x = x + y;
}

inline wide_double& operator-=(wide_double& x, wide_double y)
{
   return x = x - y;
}

inline wide_double& operator*=(wide_double& x, wide_double y)
{
   return x = x * y;
}


inline wide_double& operator/=(wide_double& x, wide_double y)
{
   return x = x / y;
}

#endif



// old-style MulMod code using floating point arithmetic

typedef wide_double mulmod_t;
typedef wide_double muldivrem_t;

inline wide_double PrepMulMod(long n)
{
   return wide_double(1L)/wide_double(n);
}

inline wide_double PrepMulDivRem(long b, long n, wide_double ninv)
{
   return wide_double(b)*ninv;
}


inline 
long MulMod(long a, long b, long n, wide_double ninv)
{
   long q = long( wide_double(a) * (wide_double(b) * ninv) );
   unsigned long rr = cast_unsigned(a)*cast_unsigned(b) - 
                      cast_unsigned(q)*cast_unsigned(n);
   long r = sp_CorrectDeficit(rr, n);
   return sp_CorrectExcess(r, n);
}

inline 
long NormalizedMulMod(long a, long b, long n, wide_double ninv)
{
   return MulMod(a, b, n, ninv);
}

inline bool NormalizedModulus(wide_double ninv) { return true; }



inline 
long MulModWithQuo(long& qres, long a, long b, long n, wide_double ninv)
{
   long q  = (long) ((((wide_double) a) * ((wide_double) b)) * ninv); 
   unsigned long rr = cast_unsigned(a)*cast_unsigned(b) - 
                      cast_unsigned(q)*cast_unsigned(n);

   long r = sp_CorrectDeficitQuo(q, rr, n);
   r = sp_CorrectExcessQuo(q, r, n);
   qres = q;
   return r;
}


inline 
long MulMod2_legacy(long a, long b, long n, wide_double bninv)
{
   long q  = (long) (((wide_double) a) * bninv);
   unsigned long rr = cast_unsigned(a)*cast_unsigned(b) - 
                      cast_unsigned(q)*cast_unsigned(n);
   long r = sp_CorrectDeficit(rr, n);
   r = sp_CorrectExcess(r, n);
   return r;
}

inline 
long MulDivRem(long& qres, long a, long b, long n, wide_double bninv)
{
   long q  = (long) (((wide_double) a) * bninv);
   unsigned long rr = cast_unsigned(a)*cast_unsigned(b) - 
                      cast_unsigned(q)*cast_unsigned(n);

   long r = sp_CorrectDeficitQuo(q, rr, n);
   r = sp_CorrectExcessQuo(q, r, n);
   qres = q;
   return r;
}

#else

// new-style MulMod code using ULL arithmetic



struct sp_inverse {
   unsigned long inv;
   long shamt;

   sp_inverse() NTL_DEFAULT
   sp_inverse(unsigned long _inv, long _shamt) : inv(_inv), shamt(_shamt) { }
};

typedef sp_inverse mulmod_t;



#if (NTL_BITS_PER_LONG >= NTL_SP_NBITS+4)

#define NTL_PRE_SHIFT1 (NTL_BITS_PER_LONG-NTL_SP_NBITS-4)
#define NTL_POST_SHIFT (0)

#define NTL_PRE_SHIFT2 (2*NTL_SP_NBITS+2)

#else

// DIRT: This assumes NTL_BITS_PER_LONG == NTL_SP_NBITS+2.
// There are checks in lip.h to verify this.

#define NTL_PRE_SHIFT1 (0)
#define NTL_POST_SHIFT (1)

#define NTL_PRE_SHIFT2 (2*NTL_SP_NBITS+1)

#endif




#if (NTL_SP_NBITS <= 2*NTL_DOUBLE_PRECISION-10)


inline unsigned long
sp_NormalizedPrepMulMod(long n)
{
   double ninv = 1/double(n); 
   unsigned long nn = n;

   // initial approximation to quotient
   unsigned long qq = long((double(1L << (NTL_SP_NBITS-1)) * double(1L << NTL_SP_NBITS)) * ninv);

   // NOTE: the true quotient is <= 2^{NTL_SP_NBITS}

   // compute approximate remainder using ULL arithmetic
   NTL_ULL_TYPE rr = (((NTL_ULL_TYPE)(1)) << (2*NTL_SP_NBITS-1)) -
                     (((NTL_ULL_TYPE)(nn)) * ((NTL_ULL_TYPE)(qq)));
                    

   rr = (rr << (NTL_PRE_SHIFT2-2*NTL_SP_NBITS+1)) - 1;

   // now compute a floating point approximation to r,
   // but avoiding unsigned -> float conversions,
   // as these are not as well supported in hardware as
   // signed -> float conversions
   
   unsigned long rrlo = (unsigned long) rr;
   unsigned long rrhi = ((unsigned long) (rr >> NTL_BITS_PER_LONG)) 
                        + (rrlo >> (NTL_BITS_PER_LONG-1));

   long rlo = cast_signed(rrlo);  // these should be No-Ops
   long rhi = cast_signed(rrhi);

   const double bpl_as_double (double(1L << NTL_SP_NBITS) * double(1L << (NTL_BITS_PER_LONG-NTL_SP_NBITS)));
   double fr = double(rlo) + double(rhi)*bpl_as_double;

   // now convert fr*ninv to a long
   // but we have to be careful: fr may be negative.
   // the result should still give floor(r/n) pm 1,
   // and is computed in a way that avoids branching

   long q1 = long(fr*ninv);
   if (q1 < 0) q1--;  
   // This counteracts the round-to-zero behavior of conversion
   // to long.  It should be compiled into branch-free code.

   unsigned long qq1 = q1;

   unsigned long rr1 = rrlo - qq1*nn;

   qq1 += 1L + sp_SignMask(rr1) + sp_SignMask(rr1-n);

   unsigned long res = (qq << (NTL_PRE_SHIFT2-2*NTL_SP_NBITS+1)) + qq1;

   res = res << NTL_PRE_SHIFT1;
   return res;
}

#else

inline unsigned long 
sp_NormalizedPrepMulMod(long n)
{
   return 
      (((unsigned long) ( ((((NTL_ULL_TYPE) 1) << NTL_PRE_SHIFT2) - 1)/n )) << NTL_PRE_SHIFT1);
}

#endif


inline sp_inverse
PrepMulMod(long n)
{
   long shamt = sp_CountLeadingZeros(n) - (NTL_BITS_PER_LONG-NTL_SP_NBITS);
   unsigned long inv = sp_NormalizedPrepMulMod(n << shamt);
   return sp_inverse(inv, shamt);
}







inline long 
sp_NormalizedMulMod(long a, long b, long n, unsigned long ninv) 
{
   ll_type U;
   ll_imul(U, a, b);
   unsigned long H = ll_rshift_get_lo<NTL_SP_NBITS-2>(U);
   unsigned long Q = ll_mul_hi(H, ninv);
   Q = Q >> NTL_POST_SHIFT;
   unsigned long L = ll_get_lo(U);
   long r = L - Q*cast_unsigned(n);  // r in [0..2*n)

   r = sp_CorrectExcess(r, n);
   return r;
}



inline long 
MulMod(long a, long b, long n, sp_inverse ninv)
{
   return sp_NormalizedMulMod(a, b << ninv.shamt, n << ninv.shamt, ninv.inv) >> ninv.shamt;
}

// if you know what you're doing....
// FIXME: eventually, put this is the documented interface...
// but for now, it's "experimental"
inline long 
NormalizedMulMod(long a, long b, long n, sp_inverse ninv)
{
   return sp_NormalizedMulMod(a, b, n, ninv.inv);
}

inline bool
NormalizedModulus(sp_inverse ninv) { return ninv.shamt == 0; }




inline long 
sp_NormalizedMulModWithQuo(long& qres, long a, long b, long n, unsigned long ninv)
{
   ll_type U;
   ll_imul(U, a, b);
   unsigned long H = ll_rshift_get_lo<NTL_SP_NBITS-2>(U);
   unsigned long Q = ll_mul_hi(H, ninv);
   Q = Q >> NTL_POST_SHIFT;
   unsigned long L = ll_get_lo(U);
   long r = L - Q*cast_unsigned(n);  // r in [0..2*n)

   r = sp_CorrectExcessQuo(Q, r, n);
   qres = Q;
   return r;
}

inline long 
MulModWithQuo(long& qres, long a, long b, long n, sp_inverse ninv)
{
   return sp_NormalizedMulModWithQuo(qres, a, b << ninv.shamt, n << ninv.shamt, ninv.inv) >> ninv.shamt;
}




#endif



#if (defined(NTL_SPMM_ULL_VIABLE) || defined(NTL_LONGLONG_SP_MULMOD))


typedef unsigned long mulmod_precon_t;


#if (!defined(NTL_LONGLONG_SP_MULMOD))

inline unsigned long PrepMulModPrecon(long b, long n, wide_double ninv)
{
   long q  = (long) ( (((wide_double) b) * wide_double(NTL_SP_BOUND)) * ninv ); 
   unsigned long rr = (cast_unsigned(b) << NTL_SP_NBITS) - cast_unsigned(q)*cast_unsigned(n);

   q += sp_SignMask(rr) + sp_SignMask(rr-n) + 1L;

   return cast_unsigned(q) << (NTL_BITS_PER_LONG - NTL_SP_NBITS);
}

#else


inline unsigned long 
sp_NormalizedPrepMulModPrecon(long b, long n, unsigned long ninv)
{
   unsigned long H = cast_unsigned(b) << 2;
   unsigned long Q = ll_mul_hi(H, ninv);
   Q = Q >> NTL_POST_SHIFT;
   unsigned long L = cast_unsigned(b) << NTL_SP_NBITS;
   long r = L - Q*cast_unsigned(n);  // r in [0..2*n)


   Q += 1L + sp_SignMask(r-n);
   return Q;  // NOTE: not shifted
}


inline unsigned long 
PrepMulModPrecon(long b, long n, sp_inverse ninv)
{
   return sp_NormalizedPrepMulModPrecon(b << ninv.shamt, n << ninv.shamt, ninv.inv) << (NTL_BITS_PER_LONG-NTL_SP_NBITS);
}





#endif


   


inline long MulModPrecon(long a, long b, long n, unsigned long bninv)
{
   unsigned long qq = ll_mul_hi(a, bninv);
   unsigned long rr = cast_unsigned(a)*cast_unsigned(b) - qq*cast_unsigned(n);
   return sp_CorrectExcess(long(rr), n);
}



inline long MulModPreconWithQuo(long& qres, long a, long b, long n, unsigned long bninv)
{
   unsigned long qq = ll_mul_hi(a, bninv);
   unsigned long rr = cast_unsigned(a)*cast_unsigned(b) - qq*cast_unsigned(n);
   long r = sp_CorrectExcessQuo(qq, long(rr), n);
   qres = long(qq);
   return r;
}



#else

// default, wide_double version

typedef wide_double mulmod_precon_t;


inline wide_double PrepMulModPrecon(long b, long n, wide_double ninv)
{
   return ((wide_double) b) * ninv;
}

inline long MulModPrecon(long a, long b, long n, wide_double bninv)
{
   return MulMod2_legacy(a, b, n, bninv);
}

inline long MulModPreconWithQuo(long& qq, long a, long b, long n, wide_double bninv)
{
   return MulDivRem(qq, a, b, n, bninv);
}



#endif








#if (defined(NTL_LONGLONG_SP_MULMOD))

// some annoying backward-compatibiliy nonsense

struct sp_muldivrem_struct {
   unsigned long bninv;

   explicit sp_muldivrem_struct(unsigned long _bninv) : bninv(_bninv) { }
   sp_muldivrem_struct() NTL_DEFAULT
};

typedef sp_muldivrem_struct muldivrem_t;

inline sp_muldivrem_struct PrepMulDivRem(long b, long n, sp_inverse ninv)
{
   return sp_muldivrem_struct(PrepMulModPrecon(b, n, ninv));
}


inline
long MulDivRem(long& qres, long a, long b, long n,  sp_muldivrem_struct bninv)
{
   return MulModPreconWithQuo(qres, a, b, n, bninv.bninv);
}

#endif






inline mulmod_precon_t PrepMulModPrecon(long b, long n)
{
   return PrepMulModPrecon(b, n, PrepMulMod(n));
}


inline 
long MulMod(long a, long b, long n)
{
   return MulMod(a, b, n, PrepMulMod(n));
}

inline muldivrem_t PrepMulDivRem(long b, long n)
{
   return PrepMulDivRem(b, n, PrepMulMod(n));
}






#ifdef NTL_LEGACY_SP_MULMOD

inline long MulMod2(long a, long b, long n, wide_double bninv)
{
   return MulMod2_legacy(a, b, n, bninv);
}


#endif




inline
void VectorMulModPrecon(long k, long *x, const long *a, long b, long n, 
                        mulmod_precon_t bninv)
{
   for (long i = 0; i < k; i++)
      x[i] = MulModPrecon(a[i], b, n, bninv);
}

inline
void VectorMulMod(long k, long *x, const long *a, long b, long n, 
                  mulmod_t ninv)
{
   mulmod_precon_t bninv;
   bninv = PrepMulModPrecon(b, n, ninv);
   VectorMulModPrecon(k, x, a, b, n, bninv);
}


inline 
void VectorMulMod(long k, long *x, const long *a, long b, long n)
{
   mulmod_t ninv = PrepMulMod(n);
   VectorMulMod(k, x, a, b, n, ninv);
}

#ifdef NTL_HAVE_LL_TYPE


struct sp_reduce_struct {
   unsigned long ninv;
   long sgn;

   sp_reduce_struct(unsigned long _ninv, long _sgn) : 
      ninv(_ninv), sgn(_sgn)  { }

   sp_reduce_struct() NTL_DEFAULT
};

inline
sp_reduce_struct sp_PrepRem(long n)
{
   unsigned long q = (1UL << (NTL_BITS_PER_LONG-1))/cast_unsigned(n);
   long r = (1UL << (NTL_BITS_PER_LONG-1)) - q*cast_unsigned(n);

   long r1 = 2*r;
   q = 2*q;
   r1 = sp_CorrectExcessQuo(q, r1, n);
   
   return sp_reduce_struct(q, r);
}



inline
long rem(unsigned long a, long n, sp_reduce_struct red) 
{
   unsigned long Q = ll_mul_hi(a, red.ninv);
   long r = a - Q*cast_unsigned(n);
   r = sp_CorrectExcess(r, n);
   return r;
}


inline
long rem(long a, long n, sp_reduce_struct red) 
{
   unsigned long a0 = cast_unsigned(a) & ((1UL << (NTL_BITS_PER_LONG-1))-1);
   long r = rem(a0, n, red);
   long s = sp_SignMask(a) & red.sgn;
   return SubMod(r, s, n);
}
#else

struct sp_reduce_struct { };


inline
sp_reduce_struct sp_PrepRem(long n) 
{
   return sp_reduce_struct();
}


inline
long rem(unsigned long a, long n, sp_reduce_struct red) 
{
   return a % cast_unsigned(n);
}

inline
long rem(long a, long n, sp_reduce_struct red)
{
   long r = a % n;
   return sp_CorrectDeficit(r, n);
}


#endif


#ifdef NTL_HAVE_LL_TYPE

#define NTL_HAVE_SP_LL_ROUTINES


// some routines that are currently not part of the documented
// interface.  They currently are only defined when we have appropriate
// LL type.


struct sp_ll_reduce_struct {
   unsigned long inv;
   long nbits;

   sp_ll_reduce_struct() NTL_DEFAULT

   sp_ll_reduce_struct(unsigned long _inv, long _nbits) : inv(_inv), nbits(_nbits) { }

};


inline sp_ll_reduce_struct
make_sp_ll_reduce_struct(long n)
{
   long nbits = NTL_BITS_PER_LONG - sp_CountLeadingZeros(n);
   unsigned long inv =
       (unsigned long) ( ((((NTL_ULL_TYPE) 1) << (nbits+NTL_BITS_PER_LONG))-1UL) / ((NTL_ULL_TYPE) n) );

   return sp_ll_reduce_struct(inv, nbits);
}


// computes remainder (hi, lo) mod d, assumes hi < d
inline long  
sp_ll_red_21(unsigned long hi, unsigned long lo, long d, 
            sp_ll_reduce_struct dinv)
{
   unsigned long H = 
      (hi << (NTL_BITS_PER_LONG-dinv.nbits)) | (lo >> dinv.nbits);
   unsigned long Q = ll_mul_hi(H, dinv.inv) + H;
   unsigned long rr = lo - Q*cast_unsigned(d); // rr in [0..4*d)
   long r = sp_CorrectExcess(rr, 2*d); // r in [0..2*d)
   r = sp_CorrectExcess(r, d);
   return r;
}

// computes remainder (x[n-1], ..., x[0]) mod d
inline long 
sp_ll_red_n1(const unsigned long *x, long n, long d, sp_ll_reduce_struct dinv)
{
   long carry = 0;
   long i;
   for (i = n-1; i >= 0; i--) 
      carry = sp_ll_red_21(carry, x[i], d, dinv);
   return carry;
} 

// computes remainder (x2, x1, x0) mod d, assumes x2 < d
inline long 
sp_ll_red_31(unsigned long x2, unsigned long x1, unsigned long x0,
           long d, sp_ll_reduce_struct dinv)
{
   long carry = sp_ll_red_21(x2, x1, d, dinv);
   return sp_ll_red_21(carry, x0, d, dinv);
}


// normalized versions of the above: assume NumBits(d) == NTL_SP_NBITS

// computes remainder (hi, lo) mod d, assumes hi < d
inline long  
sp_ll_red_21_normalized(unsigned long hi, unsigned long lo, long d, 
            sp_ll_reduce_struct dinv)
{
   unsigned long H = 
      (hi << (NTL_BITS_PER_LONG-NTL_SP_NBITS)) | (lo >> NTL_SP_NBITS);
   unsigned long Q = ll_mul_hi(H, dinv.inv) + H;
   unsigned long rr = lo - Q*cast_unsigned(d); // rr in [0..4*d)
   long r = sp_CorrectExcess(rr, 2*d); // r in [0..2*d)
   r = sp_CorrectExcess(r, d);
   return r;
}

// computes remainder (x[n-1], ..., x[0]) mod d
inline long 
sp_ll_red_n1_normalized(const unsigned long *x, long n, long d, sp_ll_reduce_struct dinv)
{
   long carry = 0;
   long i;
   for (i = n-1; i >= 0; i--) 
      carry = sp_ll_red_21_normalized(carry, x[i], d, dinv);
   return carry;
} 

// computes remainder (x2, x1, x0) mod d, assumes x2 < d
inline long 
sp_ll_red_31_normalized(unsigned long x2, unsigned long x1, unsigned long x0,
           long d, sp_ll_reduce_struct dinv)
{
   long carry = sp_ll_red_21_normalized(x2, x1, d, dinv);
   return sp_ll_red_21_normalized(carry, x0, d, dinv);
}


#else

// provided to streamline some code


struct sp_ll_reduce_struct { };


inline sp_ll_reduce_struct
make_sp_ll_reduce_struct(long n)
{
   return sp_ll_reduce_struct();
}

#endif


NTL_CLOSE_NNS

#endif





/**************************************************************************

Implementation notes -- the LONGLONG MulMod implementation

I started out basing this on Granlund-Moeller multiplication,
but it evolved into something a little bit different.

We assume that modulus n has w bits, so 2^{w-1} <= n < 2^w.
We also assume that 2 <= w <= BPL-2.

As a precomputation step, we compute X = floor((2^v-1)/n), i.e.,
   2^v - 1 = X n + Y,    where 0 <= Y < n

Now, we are given U to reduce mod n.
Write 
   U  = H 2^s + L
  H X = Q 2^t + R
where s + t = v.

Some simple calculations yield:
   H <= U/2^s
   2^{v-w} <= X <  2^v/n <= 2^{v-w+1}
 H X <  2^t U / n

Also:
   U - Qn   <   n U / 2^v  + L + n

For the case where BPL >= 64, we generally work with w = BPL-4.
In this case, we set v = 2w+2, s = w-2, t = w+4.
Then we have:
   U - Qn < n/4 + n/2 + n < 2n
This choice of parameters allows us to do a MulMod with just a single
correction.  It also allows us to do the LazyPrepMulModPrecon
step with just a single correction.

If w = BPL-2, we set v = 2w+1, s = w-2, t = w+3.
Then we have:
   U - Qn   <   n/2 + n/2 + n = 2n 
So again, we we do a MulMod with just a single correction,
although the LazyPrepMulModPrecon now takes two corrections.

For the Lazy stuff, we are computing floor(2^{w+2}b/n), so 
U = 2^{w+2} b.
For the case w = BPL-4, we are setting s = w+2 and t = w.
L = 0 in this case, and we obtain U - Qn < n + n = 2n.
In the case w = BPL-2, we set s = w, t = w+1.
We obtain U - Qn < 2n + n = 3n.
Also, we need to write X = 2^{w+1} + X_0 to perform the 
HX computation correctly.

***************************************************************************/

