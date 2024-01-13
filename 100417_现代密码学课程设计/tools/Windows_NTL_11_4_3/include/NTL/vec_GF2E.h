
#ifndef NTL_vec_GF2E__H
#define NTL_vec_GF2E__H

#include <NTL/GF2E.h>

NTL_OPEN_NNS

typedef Vec<GF2E> vec_GF2E;

void mul(vec_GF2E& x, const vec_GF2E& a, const GF2E& b);
inline void mul(vec_GF2E& x, const GF2E& a, const vec_GF2E& b)
   { mul(x, b, a); }

void mul(vec_GF2E& x, const vec_GF2E& a, GF2 b);
inline void mul(vec_GF2E& x, GF2 a, const vec_GF2E& b)
   { mul(x, b, a); }

inline void mul(vec_GF2E& x, const vec_GF2E& a, long b)
   { mul(x, a, to_GF2(b)); }
inline void mul(vec_GF2E& x, long a, const vec_GF2E& b)
   { mul(x, b, a); }



void add(vec_GF2E& x, const vec_GF2E& a, const vec_GF2E& b);
inline void sub(vec_GF2E& x, const vec_GF2E& a, const vec_GF2E& b)
  { add(x, a, b); }

inline void negate(vec_GF2E& x, const vec_GF2E& a) { x = a; }

void clear(vec_GF2E& x);


void InnerProduct(GF2E& x, const vec_GF2E& a, const vec_GF2E& b);
void InnerProduct(GF2E& x, const vec_GF2E& a, const vec_GF2E& b,
                  long offset);


long IsZero(const vec_GF2E& a);

vec_GF2E 
operator+(const vec_GF2E& a, const vec_GF2E& b);

vec_GF2E 
operator-(const vec_GF2E& a, const vec_GF2E& b);

vec_GF2E operator-(const vec_GF2E& a);
GF2E operator*(const vec_GF2E& a, const vec_GF2E& b);

inline vec_GF2E operator*(const vec_GF2E& a, const GF2E& b)
   { vec_GF2E x; mul(x, a, b); NTL_OPT_RETURN(vec_GF2E, x); }

inline vec_GF2E operator*(const vec_GF2E& a, GF2 b)
   { vec_GF2E x; mul(x, a, b); NTL_OPT_RETURN(vec_GF2E, x); }

inline vec_GF2E operator*(const vec_GF2E& a, long b)
   { vec_GF2E x; mul(x, a, b); NTL_OPT_RETURN(vec_GF2E, x); }

inline vec_GF2E operator*(const GF2E& a, const vec_GF2E& b)
   { vec_GF2E x; mul(x, a, b); NTL_OPT_RETURN(vec_GF2E, x); }

inline vec_GF2E operator*(GF2 a, const vec_GF2E& b)
   { vec_GF2E x; mul(x, a, b); NTL_OPT_RETURN(vec_GF2E, x); }

inline vec_GF2E operator*(long a, const vec_GF2E& b)
   { vec_GF2E x; mul(x, a, b); NTL_OPT_RETURN(vec_GF2E, x); }



// assignment operator notation:

inline vec_GF2E& operator+=(vec_GF2E& x, const vec_GF2E& a)
{ 
   add(x, x, a);
   return x;
}

inline vec_GF2E& operator-=(vec_GF2E& x, const vec_GF2E& a)
{ 
   sub(x, x, a);
   return x;
}

inline vec_GF2E& operator*=(vec_GF2E& x, const GF2E& a)
{ 
   mul(x, x, a);
   return x;
}

inline vec_GF2E& operator*=(vec_GF2E& x, GF2 a)
{ 
   mul(x, x, a);
   return x;
}

inline vec_GF2E& operator*=(vec_GF2E& x, long a)
{ 
   mul(x, x, a);
   return x;
}

void VectorCopy(vec_GF2E& x, const vec_GF2E& a, long n);
inline vec_GF2E VectorCopy(const vec_GF2E& a, long n)
   { vec_GF2E x; VectorCopy(x, a, n); NTL_OPT_RETURN(vec_GF2E, x); }


void random(vec_GF2E& x, long n);
inline vec_GF2E random_vec_GF2E(long n)
   { vec_GF2E x; random(x, n); NTL_OPT_RETURN(vec_GF2E, x); }

NTL_CLOSE_NNS

#endif
