
#ifndef NTL_ZZ_pEXFactoring__H
#define NTL_ZZ_pEXFactoring__H

#include <NTL/pair_ZZ_pEX_long.h>

NTL_OPEN_NNS


void SquareFreeDecomp(vec_pair_ZZ_pEX_long& u, const ZZ_pEX& f);
inline vec_pair_ZZ_pEX_long SquareFreeDecomp(const ZZ_pEX& f)
   { vec_pair_ZZ_pEX_long x; SquareFreeDecomp(x, f); return x; }


// Performs square-free decomposition.
// f must be monic.
// If f = prod_i g_i^i, then u is set to a lest of pairs (g_i, i).
// The list is is increasing order of i, with trivial terms 
// (i.e., g_i = 1) deleted.


void FindRoots(vec_ZZ_pE& x, const ZZ_pEX& f);
inline vec_ZZ_pE FindRoots(const ZZ_pEX& f)
   { vec_ZZ_pE x; FindRoots(x, f); return x; }

// f is monic, and has deg(f) distinct roots.
// returns the list of roots


void FindRoot(ZZ_pE& root, const ZZ_pEX& f);
inline ZZ_pE FindRoot(const ZZ_pEX& f)
   { ZZ_pE x; FindRoot(x, f); return x; }


// finds a single root of f.
// assumes that f is monic and splits into distinct linear factors


extern 
NTL_CHEAP_THREAD_LOCAL 
long ZZ_pEX_GCDTableSize; /* = 4 */
// Controls GCD blocking for NewDDF

extern 
NTL_CHEAP_THREAD_LOCAL 
double ZZ_pEXFileThresh; 
// of these tables exceeds ZZ_pEXFileThresh KB.



void NewDDF(vec_pair_ZZ_pEX_long& factors, 
            const ZZ_pEX& f, const ZZ_pEX& h, long verbose=0);
inline vec_pair_ZZ_pEX_long NewDDF(const ZZ_pEX& f, const ZZ_pEX& h,
         long verbose=0)
   { vec_pair_ZZ_pEX_long x; NewDDF(x, f, h, verbose); return x; }





void EDF(vec_ZZ_pEX& factors, const ZZ_pEX& f, const ZZ_pEX& b,
         long d, long verbose=0);
inline vec_ZZ_pEX EDF(const ZZ_pEX& f, const ZZ_pEX& b,
         long d, long verbose=0)
   { vec_ZZ_pEX x; EDF(x, f, b, d, verbose); return x; }


// Performs equal-degree factorization.
// f is monic, square-free, and all irreducible factors have same degree.
// b = X^p mod f.
// d = degree of irreducible factors of f
// Space for the trace-map computation can be controlled via ComposeBound.



void RootEDF(vec_ZZ_pEX& factors, const ZZ_pEX& f, long verbose=0);
inline vec_ZZ_pEX RootEDF(const ZZ_pEX& f, long verbose=0)
   { vec_ZZ_pEX x; RootEDF(x, f, verbose); return x; }


// EDF for d==1

void SFCanZass(vec_ZZ_pEX& factors, const ZZ_pEX& f, long verbose=0);
inline vec_ZZ_pEX SFCanZass(const ZZ_pEX& f, long verbose=0)
   { vec_ZZ_pEX x; SFCanZass(x, f, verbose); return x; }


// Assumes f is monic and square-free.
// returns list of factors of f.
// Uses "Cantor/Zassenhaus" approach.



void CanZass(vec_pair_ZZ_pEX_long& factors, const ZZ_pEX& f, 
             long verbose=0);
inline vec_pair_ZZ_pEX_long CanZass(const ZZ_pEX& f, long verbose=0)
   { vec_pair_ZZ_pEX_long x; CanZass(x, f, verbose); return x; }


// returns a list of factors, with multiplicities.
// f must be monic.
// Uses "Cantor/Zassenhaus" approach.


void mul(ZZ_pEX& f, const vec_pair_ZZ_pEX_long& v);
inline ZZ_pEX mul(const vec_pair_ZZ_pEX_long& v)
   { ZZ_pEX x; mul(x, v); return x; }


// multiplies polynomials, with multiplicities


/*************************************************************

            irreducible poly's:  tests and constructions

**************************************************************/

long ProbIrredTest(const ZZ_pEX& f, long iter=1);

// performs a fast, probabilistic irreduciblity test
// the test can err only if f is reducible, and the
// error probability is bounded by p^{-iter}.

long DetIrredTest(const ZZ_pEX& f);

// performs a recursive deterministic irreducibility test
// fast in the worst-case (when input is irreducible).

long IterIrredTest(const ZZ_pEX& f);

// performs an iterative deterministic irreducibility test,
// based on DDF.  Fast on average (when f has a small factor).

void BuildIrred(ZZ_pEX& f, long n);
inline ZZ_pEX BuildIrred_ZZ_pEX(long n)
   { ZZ_pEX x; BuildIrred(x, n); NTL_OPT_RETURN(ZZ_pEX, x); }


// Build a monic irreducible poly of degree n.

void BuildRandomIrred(ZZ_pEX& f, const ZZ_pEX& g);
inline ZZ_pEX BuildRandomIrred(const ZZ_pEX& g)
    { ZZ_pEX x; BuildRandomIrred(x, g); NTL_OPT_RETURN(ZZ_pEX, x); }


// g is a monic irreducible polynomial.
// constructs a random monic irreducible polynomial f of the same degree.


long RecComputeDegree(const ZZ_pEX& h, const ZZ_pEXModulus& F);

// f = F.f is assumed to be an "equal degree" polynomial
// h = X^p mod f
// the common degree of the irreducible factors of f is computed
// This routine is useful in counting points on elliptic curves


long IterComputeDegree(const ZZ_pEX& h, const ZZ_pEXModulus& F);


void TraceMap(ZZ_pEX& w, const ZZ_pEX& a, long d, const ZZ_pEXModulus& F,
              const ZZ_pEX& b);

inline ZZ_pEX TraceMap(const ZZ_pEX& a, long d, const ZZ_pEXModulus& F,
              const ZZ_pEX& b)
   { ZZ_pEX x; TraceMap(x, a, d, F, b); return x; }


// w = a+a^q+...+^{q^{d-1}} mod f;
// it is assumed that d >= 0, and b = X^q mod f, q a power of p
// Space allocation can be controlled via ComposeBound (see <NTL/ZZ_pEX.h>)



void PowerCompose(ZZ_pEX& w, const ZZ_pEX& a, long d, const ZZ_pEXModulus& F);

inline ZZ_pEX PowerCompose(const ZZ_pEX& a, long d, const ZZ_pEXModulus& F)
   { ZZ_pEX x; PowerCompose(x, a, d, F); return x; }


// w = X^{q^d} mod f;
// it is assumed that d >= 0, and b = X^q mod f, q a power of p
// Space allocation can be controlled via ComposeBound (see <NTL/ZZ_pEX.h>)





NTL_CLOSE_NNS

#endif
