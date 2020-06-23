
#ifndef NTL_vec_ZZ_p__H
#define NTL_vec_ZZ_p__H

#include <NTL/ZZ_p.h>
#include <NTL/vec_ZZ.h>

NTL_OPEN_NNS

typedef Vec<ZZ_p> vec_ZZ_p;


// legacy conversion notation
inline vec_ZZ_p to_vec_ZZ_p(const vec_ZZ& a)
   { vec_ZZ_p x; conv(x, a); NTL_OPT_RETURN(vec_ZZ_p, x); }
inline vec_ZZ to_vec_ZZ(const vec_ZZ_p& a)
   { vec_ZZ x; conv(x, a); NTL_OPT_RETURN(vec_ZZ, x); }


void mul(vec_ZZ_p& x, const vec_ZZ_p& a, const ZZ_p& b);
inline void mul(vec_ZZ_p& x, const ZZ_p& a, const vec_ZZ_p& b)
   { mul(x, b, a); }

void mul(vec_ZZ_p& x, const vec_ZZ_p& a, long b);
inline void mul(vec_ZZ_p& x, long a, const vec_ZZ_p& b)
   { mul(x, b, a); }

void add(vec_ZZ_p& x, const vec_ZZ_p& a, const vec_ZZ_p& b);

void sub(vec_ZZ_p& x, const vec_ZZ_p& a, const vec_ZZ_p& b);
void clear(vec_ZZ_p& x);
void negate(vec_ZZ_p& x, const vec_ZZ_p& a);




void InnerProduct(ZZ_p& x, const vec_ZZ_p& a, const vec_ZZ_p& b);
void InnerProduct(ZZ_p& x, const vec_ZZ_p& a, const vec_ZZ_p& b,
                  long offset);

long IsZero(const vec_ZZ_p& a);

void VectorCopy(vec_ZZ_p& x, const vec_ZZ_p& a, long n);
inline vec_ZZ_p VectorCopy(const vec_ZZ_p& a, long n)
   { vec_ZZ_p x; VectorCopy(x, a, n); NTL_OPT_RETURN(vec_ZZ_p, x); }

vec_ZZ_p operator+(const vec_ZZ_p& a, const vec_ZZ_p& b);
vec_ZZ_p operator-(const vec_ZZ_p& a, const vec_ZZ_p& b);
vec_ZZ_p operator-(const vec_ZZ_p& a);

inline vec_ZZ_p operator*(const vec_ZZ_p& a, const ZZ_p& b)
   { vec_ZZ_p x; mul(x, a, b); NTL_OPT_RETURN(vec_ZZ_p, x); }

inline vec_ZZ_p operator*(const vec_ZZ_p& a, long b)
   { vec_ZZ_p x; mul(x, a, b); NTL_OPT_RETURN(vec_ZZ_p, x); }

inline vec_ZZ_p operator*(const ZZ_p& a, const vec_ZZ_p& b)
   { vec_ZZ_p x; mul(x, a, b); NTL_OPT_RETURN(vec_ZZ_p, x); }

inline vec_ZZ_p operator*(long a, const vec_ZZ_p& b)
   { vec_ZZ_p x; mul(x, a, b); NTL_OPT_RETURN(vec_ZZ_p, x); }

ZZ_p operator*(const vec_ZZ_p& a, const vec_ZZ_p& b);



// assignment operator notation:

inline vec_ZZ_p& operator+=(vec_ZZ_p& x, const vec_ZZ_p& a)
{ 
   add(x, x, a);
   return x;
}

inline vec_ZZ_p& operator-=(vec_ZZ_p& x, const vec_ZZ_p& a)
{ 
   sub(x, x, a);
   return x;
}

inline vec_ZZ_p& operator*=(vec_ZZ_p& x, const ZZ_p& a)
{ 
   mul(x, x, a);
   return x;
}

inline vec_ZZ_p& operator*=(vec_ZZ_p& x, long a)
{ 
   mul(x, x, a);
   return x;
}


void random(vec_ZZ_p& x, long n);
inline vec_ZZ_p random_vec_ZZ_p(long n)
   { vec_ZZ_p x; random(x, n); NTL_OPT_RETURN(vec_ZZ_p, x); }

NTL_CLOSE_NNS


#endif
