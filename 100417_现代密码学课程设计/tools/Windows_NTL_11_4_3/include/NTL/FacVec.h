
#ifndef NTL_FacVec__H
#define NTL_FacVec__H

#include <NTL/vector.h>

NTL_OPEN_NNS

struct IntFactor {
   long q;
   long a;
   long val;
   long link;
};


typedef Vec<IntFactor> vec_IntFactor;
typedef vec_IntFactor FacVec;

void FactorInt(FacVec& fvec, long n);

NTL_CLOSE_NNS

#endif
