

#ifndef NTL_GF2EXFactoring__H
#define NTL_GF2EXFactoring__H

#include <NTL/GF2EX.h>
#include <NTL/pair_GF2EX_long.h>

NTL_OPEN_NNS


/************************************************************

                      factorization routines 

************************************************************/





void SquareFreeDecomp(vec_pair_GF2EX_long& u, const GF2EX& f);
inline vec_pair_GF2EX_long SquareFreeDecomp(const GF2EX& f)
   { vec_pair_GF2EX_long x; SquareFreeDecomp(x, f); return x; }


// Performs square-free decomposition.
// f must be monic.
// If f = prod_i g_i^i, then u is set to a lest of pairs (g_i, i).
// The list is is increasing order of i, with trivial terms 
// (i.e., g_i = 1) deleted.


void FindRoots(vec_GF2E& x, const GF2EX& f);
inline vec_GF2E FindRoots(const GF2EX& f)
   { vec_GF2E x; FindRoots(x, f); return x; }


// f is monic, and has deg(f) distinct roots.
// returns the list of roots

void FindRoot(GF2E& root, const GF2EX& f);
inline GF2E FindRoot(const GF2EX& f)
   { GF2E x; FindRoot(x, f); return x; }


// finds a single root of f.
// assumes that f is monic and splits into distinct linear factors


void SFBerlekamp(vec_GF2EX& factors, const GF2EX& f, long verbose=0);
inline vec_GF2EX SFBerlekamp(const GF2EX& f, long verbose=0)
   { vec_GF2EX x; SFBerlekamp(x, f, verbose); return x; }


// Assumes f is square-free and monic.
// returns list of factors of f.
// Uses "Berlekamp" appraoch.


void berlekamp(vec_pair_GF2EX_long& factors, const GF2EX& f, long verbose=0);
inline vec_pair_GF2EX_long
berlekamp(const GF2EX& f, long verbose=0)
   { vec_pair_GF2EX_long x; berlekamp(x, f, verbose); return x; }


// returns a list of factors, with multiplicities.
// f must be monic.
// Uses "Berlekamp" appraoch.


extern 
NTL_CHEAP_THREAD_LOCAL
long GF2EX_BlockingFactor;
// Controls GCD blocking for DDF.

void DDF(vec_pair_GF2EX_long& factors, const GF2EX& f, const GF2EX& h,
         long verbose=0);

inline vec_pair_GF2EX_long DDF(const GF2EX& f, const GF2EX& h,
         long verbose=0)
   { vec_pair_GF2EX_long x; DDF(x, f, h, verbose); return x; }


// Performs distinct-degree factorization.
// Assumes f is monic and square-free,  and h  = X^p mod f
// Obsolete: see NewDDF, below.

extern 
NTL_CHEAP_THREAD_LOCAL
long GF2EX_GCDTableSize; /* = 4 */
// Controls GCD blocking for NewDDF


extern 
NTL_CHEAP_THREAD_LOCAL
double GF2EXFileThresh; 
// external files are used for baby/giant steps if size
// of these tables exceeds GF2EXFileThresh KB.


void NewDDF(vec_pair_GF2EX_long& factors, const GF2EX& f, const GF2EX& h,
         long verbose=0);
inline vec_pair_GF2EX_long NewDDF(const GF2EX& f, const GF2EX& h,
         long verbose=0)
   { vec_pair_GF2EX_long x; NewDDF(x, f, h, verbose); return x; }


// same as above, but uses baby-step/giant-step method


void EDF(vec_GF2EX& factors, const GF2EX& f, const GF2EX& b,
         long d, long verbose=0);
inline vec_GF2EX EDF(const GF2EX& f, const GF2EX& b,
         long d, long verbose=0)
   { vec_GF2EX x; EDF(x, f, b, d, verbose); return x; }


// Performs equal-degree factorization.
// f is monic, square-free, and all irreducible factors have same degree.
// b = X^p mod f.
// d = degree of irreducible factors of f
// Space for the trace-map computation can be controlled via ComposeBound.



void RootEDF(vec_GF2EX& factors, const GF2EX& f, long verbose=0);
inline vec_GF2EX RootEDF(const GF2EX& f, long verbose=0)
   { vec_GF2EX x; RootEDF(x, f, verbose); return x; }


// EDF for d==1

void SFCanZass(vec_GF2EX& factors, const GF2EX& f, long verbose=0);
inline vec_GF2EX SFCanZass(const GF2EX& f, long verbose=0)
   { vec_GF2EX x; SFCanZass(x, f, verbose); return x; }


// Assumes f is monic and square-free.
// returns list of factors of f.
// Uses "Cantor/Zassenhaus" approach.



void CanZass(vec_pair_GF2EX_long& factors, const GF2EX& f, long verbose=0);
inline vec_pair_GF2EX_long CanZass(const GF2EX& f, long verbose=0)
   { vec_pair_GF2EX_long x; CanZass(x, f, verbose); return x; }


// returns a list of factors, with multiplicities.
// f must be monic.
// Uses "Cantor/Zassenhaus" approach.


void mul(GF2EX& f, const vec_pair_GF2EX_long& v);
inline GF2EX mul(const vec_pair_GF2EX_long& v)
   { GF2EX x; mul(x, v); return x; }


// multiplies polynomials, with multiplicities


/*************************************************************

            irreducible poly's:  tests and constructions

**************************************************************/

long ProbIrredTest(const GF2EX& f, long iter=1);

// performs a fast, probabilistic irreduciblity test
// the test can err only if f is reducible, and the
// error probability is bounded by p^{-iter}.

long DetIrredTest(const GF2EX& f);

// performs a recursive deterministic irreducibility test
// fast in the worst-case (when input is irreducible).

long IterIrredTest(const GF2EX& f);

// performs an iterative deterministic irreducibility test,
// based on DDF.  Fast on average (when f has a small factor).

void BuildIrred(GF2EX& f, long n);
inline GF2EX BuildIrred_GF2EX(long n)
   { GF2EX x; BuildIrred(x, n); NTL_OPT_RETURN(GF2EX, x); }



// Build a monic irreducible poly of degree n.

void BuildRandomIrred(GF2EX& f, const GF2EX& g);
inline GF2EX BuildRandomIrred(const GF2EX& g)
   { GF2EX x; BuildRandomIrred(x, g); NTL_OPT_RETURN(GF2EX, x); }


// g is a monic irreducible polynomial.
// constructs a random monic irreducible polynomial f of the same degree.


long RecComputeDegree(const GF2EX& h, const GF2EXModulus& F);

// f = F.f is assumed to be an "equal degree" polynomial
// h = X^p mod f
// the common degree of the irreducible factors of f is computed
// This routine is useful in counting points on elliptic curves


long IterComputeDegree(const GF2EX& h, const GF2EXModulus& F);


void TraceMap(GF2EX& w, const GF2EX& a, long d, const GF2EXModulus& F,
              const GF2EX& b);
inline GF2EX TraceMap(const GF2EX& a, long d, const GF2EXModulus& F,
              const GF2EX& b)
   { GF2EX x; TraceMap(x, a, d, F, b); return x; }


// w = a+a^q+...+^{q^{d-1}} mod f;
// it is assumed that d >= 0, and b = X^q mod f, q a power of p
// Space allocation can be controlled via ComposeBound (see <NTL/GF2EX.h>)



void PowerCompose(GF2EX& w, const GF2EX& a, long d, const GF2EXModulus& F);
inline GF2EX PowerCompose(const GF2EX& a, long d, const GF2EXModulus& F)
   { GF2EX x; PowerCompose(x, a, d, F); return x; }


// w = X^{q^d} mod f;
// it is assumed that d >= 0, and b = X^q mod f, q a power of p
// Space allocation can be controlled via ComposeBound (see <NTL/GF2EX.h>)

void PlainFrobeniusMap(GF2EX& h, const GF2EXModulus& F);
void ComposeFrobeniusMap(GF2EX& y, const GF2EXModulus& F);
void FrobeniusMap(GF2EX& h, const GF2EXModulus& F);
inline GF2EX FrobeniusMap(const GF2EXModulus& F)
   { GF2EX x; FrobeniusMap(x, F); return x; }
long UseComposeFrobenius(long d, long n);


NTL_CLOSE_NNS

#endif
