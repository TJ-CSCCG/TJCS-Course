

#ifndef NTL_ZZ_pXFactoring__H
#define NTL_ZZ_pXFactoring__H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pX.h>
#include <NTL/pair_ZZ_pX_long.h>

NTL_OPEN_NNS




/************************************************************

                      factorization routines 

************************************************************/





void SquareFreeDecomp(vec_pair_ZZ_pX_long& u, const ZZ_pX& f);
inline vec_pair_ZZ_pX_long SquareFreeDecomp(const ZZ_pX& f)
   { vec_pair_ZZ_pX_long x; SquareFreeDecomp(x, f); return x; }

// Performs square-free decomposition.
// f must be monic.
// If f = prod_i g_i^i, then u is set to a lest of pairs (g_i, i).
// The list is is increasing order of i, with trivial terms 
// (i.e., g_i = 1) deleted.


void FindRoots(vec_ZZ_p& x, const ZZ_pX& f);
inline vec_ZZ_p FindRoots(const ZZ_pX& f)
   { vec_ZZ_p x; FindRoots(x, f); return x; }

// f is monic, and has deg(f) distinct roots.
// returns the list of roots

void FindRoot(ZZ_p& root, const ZZ_pX& f);
inline ZZ_p FindRoot(const ZZ_pX& f)
   { ZZ_p x; FindRoot(x, f); return x; }

// finds a single root of f.
// assumes that f is monic and splits into distinct linear factors


void SFBerlekamp(vec_ZZ_pX& factors, const ZZ_pX& f, long verbose=0);
inline vec_ZZ_pX SFBerlekamp(const ZZ_pX& f, long verbose=0)
   { vec_ZZ_pX x; SFBerlekamp(x, f, verbose); return x; }

// Assumes f is square-free and monic.
// returns list of factors of f.
// Uses "Berlekamp" appraoch.


void berlekamp(vec_pair_ZZ_pX_long& factors, const ZZ_pX& f, long verbose=0);
inline vec_pair_ZZ_pX_long 
berlekamp(const ZZ_pX& f, long verbose=0)
   { vec_pair_ZZ_pX_long x; berlekamp(x, f, verbose); return x; }

// returns a list of factors, with multiplicities.
// f must be monic.
// Uses "Berlekamp" appraoch.


extern NTL_CHEAP_THREAD_LOCAL long ZZ_pX_BlockingFactor;
// Controls GCD blocking for DDF.

void DDF(vec_pair_ZZ_pX_long& factors, const ZZ_pX& f, const ZZ_pX& h,
         long verbose=0);

inline vec_pair_ZZ_pX_long DDF(const ZZ_pX& f, const ZZ_pX& h,
         long verbose=0)
   { vec_pair_ZZ_pX_long x; DDF(x, f, h, verbose); return x; }

// Performs distinct-degree factorization.
// Assumes f is monic and square-free,  and h  = X^p mod f
// Obsolete: see NewDDF, below.

extern NTL_CHEAP_THREAD_LOCAL long ZZ_pX_GCDTableSize; /* = 4 */
// Controls GCD blocking for NewDDF


extern NTL_CHEAP_THREAD_LOCAL double ZZ_pXFileThresh; 
// external files are used for baby/giant steps if size
// of these tables exceeds ZZ_pXFileThresh KB.

void NewDDF(vec_pair_ZZ_pX_long& factors, const ZZ_pX& f, const ZZ_pX& h,
         long verbose=0);

inline vec_pair_ZZ_pX_long NewDDF(const ZZ_pX& f, const ZZ_pX& h,
         long verbose=0)
   { vec_pair_ZZ_pX_long x; NewDDF(x, f, h, verbose); return x; }

// same as above, but uses baby-step/giant-step method


void EDF(vec_ZZ_pX& factors, const ZZ_pX& f, const ZZ_pX& b,
         long d, long verbose=0);

inline vec_ZZ_pX EDF(const ZZ_pX& f, const ZZ_pX& b,
         long d, long verbose=0)
   { vec_ZZ_pX x; EDF(x, f, b, d, verbose); return x; }

// Performs equal-degree factorization.
// f is monic, square-free, and all irreducible factors have same degree.
// b = X^p mod f.
// d = degree of irreducible factors of f
// Space for the trace-map computation can be controlled via ComposeBound.



void RootEDF(vec_ZZ_pX& factors, const ZZ_pX& f, long verbose=0);
inline vec_ZZ_pX RootEDF(const ZZ_pX& f, long verbose=0)
   { vec_ZZ_pX x; RootEDF(x, f, verbose); return x; }

// EDF for d==1

void SFCanZass(vec_ZZ_pX& factors, const ZZ_pX& f, long verbose=0);
inline vec_ZZ_pX SFCanZass(const ZZ_pX& f, long verbose=0)
   { vec_ZZ_pX x; SFCanZass(x, f, verbose); return x; }

// Assumes f is monic and square-free.
// returns list of factors of f.
// Uses "Cantor/Zassenhaus" approach.



void CanZass(vec_pair_ZZ_pX_long& factors, const ZZ_pX& f, 
      long verbose=0);

inline vec_pair_ZZ_pX_long CanZass(const ZZ_pX& f, long verbose=0)
   { vec_pair_ZZ_pX_long x; CanZass(x, f, verbose); return x; }

// returns a list of factors, with multiplicities.
// f must be monic.
// Uses "Cantor/Zassenhaus" approach.


void mul(ZZ_pX& f, const vec_pair_ZZ_pX_long& v);
inline ZZ_pX mul(const vec_pair_ZZ_pX_long& v)
   { ZZ_pX x; mul(x, v); return x; }

// multiplies polynomials, with multiplicities


/*************************************************************

            irreducible poly's:  tests and constructions

**************************************************************/

long ProbIrredTest(const ZZ_pX& f, long iter=1);

// performs a fast, probabilistic irreduciblity test
// the test can err only if f is reducible, and the
// error probability is bounded by p^{-iter}.

long DetIrredTest(const ZZ_pX& f);

// performs a recursive deterministic irreducibility test
// fast in the worst-case (when input is irreducible).

long IterIrredTest(const ZZ_pX& f);

// performs an iterative deterministic irreducibility test,
// based on DDF.  Fast on average (when f has a small factor).

void BuildIrred(ZZ_pX& f, long n);
inline ZZ_pX BuildIrred_ZZ_pX(long n)
   { ZZ_pX x; BuildIrred(x, n); NTL_OPT_RETURN(ZZ_pX, x); }

// Build a monic irreducible poly of degree n.

void BuildRandomIrred(ZZ_pX& f, const ZZ_pX& g);
inline ZZ_pX BuildRandomIrred(const ZZ_pX& g)
   { ZZ_pX x; BuildRandomIrred(x, g); NTL_OPT_RETURN(ZZ_pX, x); }

// g is a monic irreducible polynomial.
// constructs a random monic irreducible polynomial f of the same degree.


long ComputeDegree(const ZZ_pX& h, const ZZ_pXModulus& F);

// f = F.f is assumed to be an "equal degree" polynomial
// h = X^p mod f
// the common degree of the irreducible factors of f is computed
// This routine is useful in counting points on elliptic curves

long ProbComputeDegree(const ZZ_pX& h, const ZZ_pXModulus& F);

// same as above, but uses a slightly faster probabilistic algorithm
// the return value may be 0 or may be too big, but for large p
// (relative to n), this happens with very low probability.



void TraceMap(ZZ_pX& w, const ZZ_pX& a, long d, const ZZ_pXModulus& F,
              const ZZ_pX& b);

inline ZZ_pX TraceMap(const ZZ_pX& a, long d, const ZZ_pXModulus& F,
              const ZZ_pX& b)
   { ZZ_pX x; TraceMap(x, a, d, F, b); return x; }

// w = a+a^q+...+^{q^{d-1}} mod f;
// it is assumed that d >= 0, and b = X^q mod f, q a power of p
// Space allocation can be controlled via ComposeBound (see <NTL/ZZ_pX.h>)



void PowerCompose(ZZ_pX& w, const ZZ_pX& a, long d, const ZZ_pXModulus& F);
inline ZZ_pX PowerCompose(const ZZ_pX& a, long d, const ZZ_pXModulus& F)
   { ZZ_pX x; PowerCompose(x, a, d, F); return x; }

// w = X^{q^d} mod f;
// it is assumed that d >= 0, and b = X^q mod f, q a power of p
// Space allocation can be controlled via ComposeBound (see <NTL/ZZ_pX.h>)


NTL_CLOSE_NNS

#endif
