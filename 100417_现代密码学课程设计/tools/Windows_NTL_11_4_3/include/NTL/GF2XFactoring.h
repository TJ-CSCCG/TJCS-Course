#ifndef NTL_GF2XFactoring__H
#define NTL_GF2XFactoring__H

#include <NTL/GF2X.h>
#include <NTL/pair_GF2X_long.h>

NTL_OPEN_NNS

long IterIrredTest(const GF2X& f);

void SquareFreeDecomp(vec_pair_GF2X_long& u, const GF2X& ff);
inline vec_pair_GF2X_long SquareFreeDecomp(const GF2X& f)
   { vec_pair_GF2X_long x; SquareFreeDecomp(x, f); return x; }


void DDF(vec_pair_GF2X_long& factors, const GF2X& ff, long verbose=0);
inline vec_pair_GF2X_long DDF(const GF2X& f,
         long verbose=0)
   { vec_pair_GF2X_long x; DDF(x, f, verbose); return x; }


void EDF(vec_GF2X& factors, const GF2X& ff, long d, long verbose=0);
inline vec_GF2X EDF(const GF2X& f, 
         long d, long verbose=0)
   { vec_GF2X x; EDF(x, f, d, verbose); return x; }


void SFCanZass(vec_GF2X& factors, const GF2X& ff, long verbose=0);
inline vec_GF2X SFCanZass(const GF2X& f, long verbose=0)
   { vec_GF2X x; SFCanZass(x, f, verbose); return x; }


void CanZass(vec_pair_GF2X_long& factors, const GF2X& f, long verbose=0);
inline vec_pair_GF2X_long CanZass(const GF2X& f, long verbose=0)
   { vec_pair_GF2X_long x; CanZass(x, f, verbose); return x; }


void mul(GF2X& f, const vec_pair_GF2X_long& v);
inline GF2X mul(const vec_pair_GF2X_long& v)
   { GF2X x; mul(x, v); return x; }


void BuildIrred(GF2X& f, long n);
inline GF2X BuildIrred_GF2X(long n)
   { GF2X x; BuildIrred(x, n); NTL_OPT_RETURN(GF2X, x); }


void BuildRandomIrred(GF2X& f, const GF2X& g);
inline GF2X BuildRandomIrred(const GF2X& g)
   { GF2X x; BuildRandomIrred(x, g); NTL_OPT_RETURN(GF2X, x); }


void BuildSparseIrred(GF2X& f, long n);
inline GF2X BuildSparseIrred_GF2X(long n)
   { GF2X x; BuildSparseIrred(x, n); NTL_OPT_RETURN(GF2X, x); }

NTL_CLOSE_NNS


#endif
