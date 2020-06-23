
#ifndef NTL_zz_pEXFactoring__H
#define NTL_zz_pEXFactoring__H

#include <NTL/pair_lzz_pEX_long.h>

NTL_OPEN_NNS


void SquareFreeDecomp(vec_pair_zz_pEX_long& u, const zz_pEX& f);
inline vec_pair_zz_pEX_long SquareFreeDecomp(const zz_pEX& f)
   { vec_pair_zz_pEX_long x; SquareFreeDecomp(x, f); return x; }


// Performs square-free decomposition.
// f must be monic.
// If f = prod_i g_i^i, then u is set to a lest of pairs (g_i, i).
// The list is is increasing order of i, with trivial terms 
// (i.e., g_i = 1) deleted.


void FindRoots(vec_zz_pE& x, const zz_pEX& f);
inline vec_zz_pE FindRoots(const zz_pEX& f)
   { vec_zz_pE x; FindRoots(x, f); return x; }

// f is monic, and has deg(f) distinct roots.
// returns the list of roots


void FindRoot(zz_pE& root, const zz_pEX& f);
inline zz_pE FindRoot(const zz_pEX& f)
   { zz_pE x; FindRoot(x, f); return x; }


// finds a single root of f.
// assumes that f is monic and splits into distinct linear factors


extern 
NTL_CHEAP_THREAD_LOCAL 
long zz_pEX_GCDTableSize; /* = 4 */
// Controls GCD blocking for NewDDF


extern 
NTL_CHEAP_THREAD_LOCAL 
double zz_pEXFileThresh; 
// external files are used for baby/giant steps if size
// of these tables exceeds zz_pEXFileThresh KB.


void NewDDF(vec_pair_zz_pEX_long& factors, 
            const zz_pEX& f, const zz_pEX& h, long verbose=0);
inline vec_pair_zz_pEX_long NewDDF(const zz_pEX& f, const zz_pEX& h,
         long verbose=0)
   { vec_pair_zz_pEX_long x; NewDDF(x, f, h, verbose); return x; }





void EDF(vec_zz_pEX& factors, const zz_pEX& f, const zz_pEX& b,
         long d, long verbose=0);
inline vec_zz_pEX EDF(const zz_pEX& f, const zz_pEX& b,
         long d, long verbose=0)
   { vec_zz_pEX x; EDF(x, f, b, d, verbose); return x; }


// Performs equal-degree factorization.
// f is monic, square-free, and all irreducible factors have same degree.
// b = X^p mod f.
// d = degree of irreducible factors of f
// Space for the trace-map computation can be controlled via ComposeBound.



void RootEDF(vec_zz_pEX& factors, const zz_pEX& f, long verbose=0);
inline vec_zz_pEX RootEDF(const zz_pEX& f, long verbose=0)
   { vec_zz_pEX x; RootEDF(x, f, verbose); return x; }


// EDF for d==1

void SFCanZass(vec_zz_pEX& factors, const zz_pEX& f, long verbose=0);
inline vec_zz_pEX SFCanZass(const zz_pEX& f, long verbose=0)
   { vec_zz_pEX x; SFCanZass(x, f, verbose); return x; }


// Assumes f is monic and square-free.
// returns list of factors of f.
// Uses "Cantor/Zassenhaus" approach.



void CanZass(vec_pair_zz_pEX_long& factors, const zz_pEX& f, 
             long verbose=0);
inline vec_pair_zz_pEX_long CanZass(const zz_pEX& f, long verbose=0)
   { vec_pair_zz_pEX_long x; CanZass(x, f, verbose); return x; }


// returns a list of factors, with multiplicities.
// f must be monic.
// Uses "Cantor/Zassenhaus" approach.


void mul(zz_pEX& f, const vec_pair_zz_pEX_long& v);
inline zz_pEX mul(const vec_pair_zz_pEX_long& v)
   { zz_pEX x; mul(x, v); return x; }


// multiplies polynomials, with multiplicities


/*************************************************************

            irreducible poly's:  tests and constructions

**************************************************************/

long ProbIrredTest(const zz_pEX& f, long iter=1);

// performs a fast, probabilistic irreduciblity test
// the test can err only if f is reducible, and the
// error probability is bounded by p^{-iter}.

long DetIrredTest(const zz_pEX& f);

// performs a recursive deterministic irreducibility test
// fast in the worst-case (when input is irreducible).

long IterIrredTest(const zz_pEX& f);

// performs an iterative deterministic irreducibility test,
// based on DDF.  Fast on average (when f has a small factor).

void BuildIrred(zz_pEX& f, long n);
inline zz_pEX BuildIrred_zz_pEX(long n)
   { zz_pEX x; BuildIrred(x, n); NTL_OPT_RETURN(zz_pEX, x); }


// Build a monic irreducible poly of degree n.

void BuildRandomIrred(zz_pEX& f, const zz_pEX& g);
inline zz_pEX BuildRandomIrred(const zz_pEX& g)
    { zz_pEX x; BuildRandomIrred(x, g); NTL_OPT_RETURN(zz_pEX, x); }


// g is a monic irreducible polynomial.
// constructs a random monic irreducible polynomial f of the same degree.


long RecComputeDegree(const zz_pEX& h, const zz_pEXModulus& F);

// f = F.f is assumed to be an "equal degree" polynomial
// h = X^p mod f
// the common degree of the irreducible factors of f is computed
// This routine is useful in counting points on elliptic curves


long IterComputeDegree(const zz_pEX& h, const zz_pEXModulus& F);


void TraceMap(zz_pEX& w, const zz_pEX& a, long d, const zz_pEXModulus& F,
              const zz_pEX& b);

inline zz_pEX TraceMap(const zz_pEX& a, long d, const zz_pEXModulus& F,
              const zz_pEX& b)
   { zz_pEX x; TraceMap(x, a, d, F, b); return x; }


// w = a+a^q+...+^{q^{d-1}} mod f;
// it is assumed that d >= 0, and b = X^q mod f, q a power of p
// Space allocation can be controlled via ComposeBound (see "zz_pEX.h")



void PowerCompose(zz_pEX& w, const zz_pEX& a, long d, const zz_pEXModulus& F);

inline zz_pEX PowerCompose(const zz_pEX& a, long d, const zz_pEXModulus& F)
   { zz_pEX x; PowerCompose(x, a, d, F); return x; }


// w = X^{q^d} mod f;
// it is assumed that d >= 0, and b = X^q mod f, q a power of p
// Space allocation can be controlled via ComposeBound (see "zz_pEX.h")






NTL_CLOSE_NNS

#endif
