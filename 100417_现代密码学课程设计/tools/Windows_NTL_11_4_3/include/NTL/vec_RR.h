
#ifndef NTL_vec_RR__H
#define NTL_vec_RR__H

#include <NTL/RR.h>
#include <NTL/vector.h>

NTL_OPEN_NNS

typedef Vec<RR> vec_RR;

void mul(vec_RR& x, const vec_RR& a, const RR& b);
inline void mul(vec_RR& x, const RR& a, const vec_RR& b)
   { mul(x, b, a); }

void mul(vec_RR& x, const vec_RR& a, double b);
inline void mul(vec_RR& x, double a, const vec_RR& b)
   { mul(x, b, a); }




void add(vec_RR& x, const vec_RR& a, const vec_RR& b);

void sub(vec_RR& x, const vec_RR& a, const vec_RR& b);
void clear(vec_RR& x);
void negate(vec_RR& x, const vec_RR& a);


void InnerProduct(RR& x, const vec_RR& a, const vec_RR& b);

long IsZero(const vec_RR& a);

void VectorCopy(vec_RR& x, const vec_RR& a, long n);
inline vec_RR VectorCopy(const vec_RR& a, long n)
   { vec_RR x; VectorCopy(x, a, n); NTL_OPT_RETURN(vec_RR, x); }
 


vec_RR operator+(const vec_RR& a, const vec_RR& b);
vec_RR operator-(const vec_RR& a, const vec_RR& b);
vec_RR operator-(const vec_RR& a);

inline vec_RR operator*(const vec_RR& a, const RR& b)
   { vec_RR x; mul(x, a, b); NTL_OPT_RETURN(vec_RR, x); }

inline vec_RR operator*(const vec_RR& a, double b)
   { vec_RR x; mul(x, a, b); NTL_OPT_RETURN(vec_RR, x); }

inline vec_RR operator*(const RR& a, const vec_RR& b)
   { vec_RR x; mul(x, a, b); NTL_OPT_RETURN(vec_RR, x); }

inline vec_RR operator*(double a, const vec_RR& b)
   { vec_RR x; mul(x, a, b); NTL_OPT_RETURN(vec_RR, x); }

RR operator*(const vec_RR& a, const vec_RR& b);


// assignment operator notation:

inline vec_RR& operator+=(vec_RR& x, const vec_RR& a)
{ 
   add(x, x, a);
   return x;
}

inline vec_RR& operator-=(vec_RR& x, const vec_RR& a)
{ 
   sub(x, x, a);
   return x;
}

inline vec_RR& operator*=(vec_RR& x, const RR& a)
{ 
   mul(x, x, a);
   return x;
}

inline vec_RR& operator*=(vec_RR& x, double a)
{ 
   mul(x, x, a);
   return x;
}


NTL_CLOSE_NNS

#endif
