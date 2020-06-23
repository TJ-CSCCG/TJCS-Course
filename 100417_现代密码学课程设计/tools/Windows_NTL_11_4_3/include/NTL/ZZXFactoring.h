#ifndef NTL_ZZXFactoring__H
#define NTL_ZZXFactoring__H


#include <NTL/ZZX.h>
#include <NTL/pair_ZZX_long.h>

NTL_OPEN_NNS

void mul(ZZX& x, const vec_pair_ZZX_long& a);
inline ZZX mul(const vec_pair_ZZX_long& v)
   { ZZX x; mul(x, v); return x; }

void SquareFreeDecomp(vec_pair_ZZX_long& u, const ZZX& f);
inline vec_pair_ZZX_long SquareFreeDecomp(const ZZX& f)
   { vec_pair_ZZX_long x; SquareFreeDecomp(x, f); return x; }

// input is primitive, with positive leading coefficient

void MultiLift(vec_ZZX& A, const vec_zz_pX& a, const ZZX& f, long e,
               long verbose=0);

// Using current value p of zz_p::modulus(), this lifts
// the square-free factorization a mod p of f to a factorization
// A mod p^e of f.
// It is required that f and all the polynomials in a are monic.



void SFFactor(vec_ZZX& factors, const ZZX& f,
              long verbose=0,
              long bnd=0);

inline vec_ZZX SFFactor(const ZZX& f, long verbose=0, long bnd=0)
   { vec_ZZX x; SFFactor(x, f, verbose, bnd); return x; }

// input f is primitive and square-free, with positive leading
// coefficient.  

// bnd, if not zero, indicates that
// f divides a polynomial h whose Euclidean norm
// is bounded by 2^{bnd} in absolute value.

extern NTL_CHEAP_THREAD_LOCAL long ZZXFac_MaxPrune;
extern NTL_CHEAP_THREAD_LOCAL long ZZXFac_InitNumPrimes;
extern NTL_CHEAP_THREAD_LOCAL long ZZXFac_MaxNumPrimes;
extern NTL_CHEAP_THREAD_LOCAL long ZZXFac_PowerHack;
extern NTL_CHEAP_THREAD_LOCAL long ZZXFac_van_Hoeij;


void factor(ZZ& c,
            vec_pair_ZZX_long& factors,
            const ZZX& f,
            long verbose=0,
            long bnd=0);

// input f is is an arbitrary polynomial.
// c is the content of f, and factors is the factorization
// of its primitive part.

// bnd is as in SFFactor.

NTL_CLOSE_NNS

#endif
