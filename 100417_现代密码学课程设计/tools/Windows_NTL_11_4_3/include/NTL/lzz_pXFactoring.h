

#ifndef NTL_zz_pXFactoring__H
#define NTL_zz_pXFactoring__H

#include <NTL/lzz_p.h>
#include <NTL/lzz_pX.h>
#include <NTL/pair_lzz_pX_long.h>

NTL_OPEN_NNS



/************************************************************

                      factorization routines 

************************************************************/





void SquareFreeDecomp(vec_pair_zz_pX_long& u, const zz_pX& f);
inline vec_pair_zz_pX_long SquareFreeDecomp(const zz_pX& f)
   { vec_pair_zz_pX_long x; SquareFreeDecomp(x, f); return x; }


// Performs square-free decomposition.
// f must be monic.
// If f = prod_i g_i^i, then u is set to a lest of pairs (g_i, i).
// The list is is increasing order of i, with trivial terms
// (i.e., g_i = 1) deleted.



void FindRoots(vec_zz_p& x, const zz_pX& f);
inline vec_zz_p FindRoots(const zz_pX& f)
   { vec_zz_p x; FindRoots(x, f); return x; }


// f is monic, and has deg(f) distinct roots.
// returns the list of roots

void FindRoot(zz_p& root, const zz_pX& f);
inline zz_p FindRoot(const zz_pX& f)
   { zz_p x; FindRoot(x, f); return x; }


// finds a single root of ff.
// assumes that f is monic and splits into distinct linear factors


void SFBerlekamp(vec_zz_pX& factors, const zz_pX& f, long verbose=0);
inline vec_zz_pX SFBerlekamp(const zz_pX& f, long verbose=0)
   { vec_zz_pX x; SFBerlekamp(x, f, verbose); return x; }


// Assumes f is square-free and monic.
// returns list of factors of f.
// Uses "Berlekamp" appraoch.


void berlekamp(vec_pair_zz_pX_long& factors, const zz_pX& f, long verbose=0);
inline vec_pair_zz_pX_long berlekamp(const zz_pX& f, long verbose=0)
   { vec_pair_zz_pX_long x; berlekamp(x, f, verbose); return x; }


// returns a list of factors, with multiplicities.
// f must be monic.
// Uses "Berlekamp" appraoch.





extern 
NTL_CHEAP_THREAD_LOCAL 
long zz_pX_BlockingFactor;
// Controls GCD blocking for DDF.


void DDF(vec_pair_zz_pX_long& factors, const zz_pX& f, const zz_pX& h,
         long verbose=0);
inline vec_pair_zz_pX_long DDF(const zz_pX& f, const zz_pX& h,
         long verbose=0)
   { vec_pair_zz_pX_long x; DDF(x, f, h, verbose); return x; }


// Performs distinct-degree factorization.
// Assumes f is monic and square-free,  and h  = X^p mod f
// Obsolete: see NewDDF, below.


extern 
NTL_CHEAP_THREAD_LOCAL 
long zz_pX_GCDTableSize; /* = 4 */
// Controls GCD blocking for NewDDF


void NewDDF(vec_pair_zz_pX_long& factors, const zz_pX& f, const zz_pX& h,
         long verbose=0);
inline vec_pair_zz_pX_long NewDDF(const zz_pX& f, const zz_pX& h,
         long verbose=0)
   { vec_pair_zz_pX_long x; NewDDF(x, f, h, verbose); return x; }


// same as above, but uses baby-step/giant-step method


void EDF(vec_zz_pX& factors, const zz_pX& f, const zz_pX& b,
         long d, long verbose=0);

inline vec_zz_pX EDF(const zz_pX& f, const zz_pX& b,
         long d, long verbose=0)
   { vec_zz_pX x; EDF(x, f, b, d, verbose); return x; }


// Performs equal-degree factorization.
// f is monic, square-free, and all irreducible factors have same degree.
// b = X^p mod f.
// d = degree of irreducible factors of f
// Space for the trace-map computation can be controlled via ComposeBound.



void RootEDF(vec_zz_pX& factors, const zz_pX& f, long verbose=0);
inline vec_zz_pX RootEDF(const zz_pX& f, long verbose=0)
   { vec_zz_pX x; RootEDF(x, f, verbose); return x; }


// EDF for d==1

void SFCanZass(vec_zz_pX& factors, const zz_pX& f, long verbose=0);
inline vec_zz_pX SFCanZass(const zz_pX& f, long verbose=0)
   { vec_zz_pX x; SFCanZass(x, f, verbose); return x; }


// Assumes f is square-free.
// returns list of factors of f.
// Uses "Cantor/Zassenhaus" approach.



void SFCanZass1(vec_pair_zz_pX_long& u, zz_pX& h, const zz_pX& f, 
                long verbose=0);

// Not intended for general use.

void SFCanZass2(vec_zz_pX& factors, const vec_pair_zz_pX_long& u,
                const zz_pX& h, long verbose=0);

// Not intended for general use.


void CanZass(vec_pair_zz_pX_long& factors, const zz_pX& f, long verbose=0);

inline vec_pair_zz_pX_long CanZass(const zz_pX& f, long verbose=0)
   { vec_pair_zz_pX_long x; CanZass(x, f, verbose); return x; }


// returns a list of factors, with multiplicities.
// f must be monic.
// Uses "Cantor/Zassenhaus" approach.




void mul(zz_pX& f, const vec_pair_zz_pX_long& v);
inline zz_pX mul(const vec_pair_zz_pX_long& v)
   { zz_pX x; mul(x, v); return x; }


// multiplies polynomials, with multiplicities


/*************************************************************

            irreducible poly's:  tests and constructions

**************************************************************/

long ProbIrredTest(const zz_pX& f, long iter=1);

// performs a fast, probabilistic irreduciblity test
// the test can err only if f is reducible, and the
// error probability is bounded by p^{-iter}.

long DetIrredTest(const zz_pX& f);

// performs a recursive deterministic irreducibility test
// fast in the worst-case (when input is irreducible).

long IterIrredTest(const zz_pX& f);

// performs an iterative deterministic irreducibility test,
// based on DDF.  Fast on average (when f has a small factor).

void BuildIrred(zz_pX& f, long n);
inline zz_pX BuildIrred_zz_pX(long n)
   { zz_pX x; BuildIrred(x, n); NTL_OPT_RETURN(zz_pX, x); }


// Build a monic irreducible poly of degree n.

void BuildRandomIrred(zz_pX& f, const zz_pX& g);
inline zz_pX BuildRandomIrred(const zz_pX& g)
   { zz_pX x; BuildRandomIrred(x, g); NTL_OPT_RETURN(zz_pX, x); }


// g is a monic irreducible polynomial.
// constructs a random monic irreducible polynomial f of the same degree.


long ComputeDegree(const zz_pX& h, const zz_pXModulus& F);

// f = F.f is assumed to be an "equal degree" polynomial
// h = X^p mod f
// the common degree of the irreducible factors of f is computed
// This routine is useful in counting points on elliptic curves

long ProbComputeDegree(const zz_pX& h, const zz_pXModulus& F);

// same as above, but uses a slightly faster probabilistic algorithm
// the return value may be 0 or may be too big, but for large p
// (relative to n), this happens with very low probability.



void TraceMap(zz_pX& w, const zz_pX& a, long d, const zz_pXModulus& F,
              const zz_pX& b);

inline zz_pX TraceMap(const zz_pX& a, long d, const zz_pXModulus& F,
              const zz_pX& b)
   { zz_pX x; TraceMap(x, a, d, F, b); return x; }


// w = a+a^q+...+^{q^{d-1}} mod f;
// it is assumed that d >= 0, and b = X^q mod f, q a power of p
// Space allocation can be controlled via ComposeBound (see "zz_pX.h")



void PowerCompose(zz_pX& w, const zz_pX& a, long d, const zz_pXModulus& F);
inline zz_pX PowerCompose(const zz_pX& a, long d, const zz_pXModulus& F)
   { zz_pX x; PowerCompose(x, a, d, F); return x; }


// w = X^{q^d} mod f;
// it is assumed that d >= 0, and b = X^q mod f, q a power of p
// Space allocation can be controlled via ComposeBound (see "zz_pX.h")

NTL_CLOSE_NNS

#endif
