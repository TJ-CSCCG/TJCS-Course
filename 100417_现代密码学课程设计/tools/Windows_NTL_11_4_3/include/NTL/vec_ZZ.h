
#ifndef NTL_vec_ZZ__H
#define NTL_vec_ZZ__H

#include <NTL/ZZ.h>
#include <NTL/vector.h>

NTL_OPEN_NNS

typedef Vec<ZZ> vec_ZZ;

void mul(vec_ZZ& x, const vec_ZZ& a, const ZZ& b);
inline void mul(vec_ZZ& x, const ZZ& a, const vec_ZZ& b)
   { mul(x, b, a); }

void mul(vec_ZZ& x, const vec_ZZ& a, long b);
inline void mul(vec_ZZ& x, long a, const vec_ZZ& b)
   { mul(x, b, a); }

void add(vec_ZZ& x, const vec_ZZ& a, const vec_ZZ& b);

void sub(vec_ZZ& x, const vec_ZZ& a, const vec_ZZ& b);
void clear(vec_ZZ& x);
void negate(vec_ZZ& x, const vec_ZZ& a);




void InnerProduct(ZZ& x, const vec_ZZ& a, const vec_ZZ& b);

long IsZero(const vec_ZZ& a);

vec_ZZ operator+(const vec_ZZ& a, const vec_ZZ& b);
vec_ZZ operator-(const vec_ZZ& a, const vec_ZZ& b);
vec_ZZ operator-(const vec_ZZ& a);

inline vec_ZZ operator*(const vec_ZZ& a, const ZZ& b)
   { vec_ZZ x; mul(x, a, b); NTL_OPT_RETURN(vec_ZZ, x); }

inline vec_ZZ operator*(const vec_ZZ& a, long b)
   { vec_ZZ x; mul(x, a, b); NTL_OPT_RETURN(vec_ZZ, x); }

inline vec_ZZ operator*(const ZZ& a, const vec_ZZ& b)
   { vec_ZZ x; mul(x, a, b); NTL_OPT_RETURN(vec_ZZ, x); }

inline vec_ZZ operator*(long a, const vec_ZZ& b)
   { vec_ZZ x; mul(x, a, b); NTL_OPT_RETURN(vec_ZZ, x); }


ZZ operator*(const vec_ZZ& a, const vec_ZZ& b);





// assignment operator notation:

inline vec_ZZ& operator+=(vec_ZZ& x, const vec_ZZ& a)
{ 
   add(x, x, a);
   return x;
}

inline vec_ZZ& operator-=(vec_ZZ& x, const vec_ZZ& a)
{ 
   sub(x, x, a);
   return x;
}

inline vec_ZZ& operator*=(vec_ZZ& x, const ZZ& a)
{ 
   mul(x, x, a);
   return x;
}

inline vec_ZZ& operator*=(vec_ZZ& x, long a)
{ 
   mul(x, x, a);
   return x;
}

void VectorCopy(vec_ZZ& x, const vec_ZZ& a, long n);
inline vec_ZZ VectorCopy(const vec_ZZ& a, long n)
   { vec_ZZ x; VectorCopy(x, a, n); NTL_OPT_RETURN(vec_ZZ, x); }


NTL_CLOSE_NNS


#endif
