
#ifndef NTL_vec_zz_pE__H
#define NTL_vec_zz_pE__H

#include <NTL/lzz_pE.h>

NTL_OPEN_NNS


typedef Vec<zz_pE> vec_zz_pE;

void mul(vec_zz_pE& x, const vec_zz_pE& a, const zz_pE& b);
inline void mul(vec_zz_pE& x, const zz_pE& a, const vec_zz_pE& b)
   { mul(x, b, a); }

void mul(vec_zz_pE& x, const vec_zz_pE& a, const zz_p& b);
inline void mul(vec_zz_pE& x, const zz_p& a, const vec_zz_pE& b)
   { mul(x, b, a); }

void mul(vec_zz_pE& x, const vec_zz_pE& a, long b);
inline void mul(vec_zz_pE& x, long a, const vec_zz_pE& b)
   { mul(x, b, a); }

void add(vec_zz_pE& x, const vec_zz_pE& a, const vec_zz_pE& b);
void sub(vec_zz_pE& x, const vec_zz_pE& a, const vec_zz_pE& b);

void negate(vec_zz_pE& x, const vec_zz_pE& a);

void clear(vec_zz_pE& x);


void InnerProduct(zz_pE& x, const vec_zz_pE& a, const vec_zz_pE& b);
void InnerProduct(zz_pE& x, const vec_zz_pE& a, const vec_zz_pE& b,
                  long offset);


long IsZero(const vec_zz_pE& a);

void VectorCopy(vec_zz_pE& x, const vec_zz_pE& a, long n);
inline vec_zz_pE VectorCopy(const vec_zz_pE& a, long n)
   { vec_zz_pE x; VectorCopy(x, a, n); NTL_OPT_RETURN(vec_zz_pE, x); }



vec_zz_pE operator+(const vec_zz_pE& a, const vec_zz_pE& b);
vec_zz_pE operator-(const vec_zz_pE& a, const vec_zz_pE& b);
vec_zz_pE operator-(const vec_zz_pE& a);

inline vec_zz_pE operator*(const vec_zz_pE& a, const zz_pE& b)
   { vec_zz_pE x; mul(x, a, b); NTL_OPT_RETURN(vec_zz_pE, x); }

inline vec_zz_pE operator*(const vec_zz_pE& a, const zz_p& b)
   { vec_zz_pE x; mul(x, a, b); NTL_OPT_RETURN(vec_zz_pE, x); }

inline vec_zz_pE operator*(const vec_zz_pE& a, long b)
   { vec_zz_pE x; mul(x, a, b); NTL_OPT_RETURN(vec_zz_pE, x); }

inline vec_zz_pE operator*(const zz_pE& a, const vec_zz_pE& b)
   { vec_zz_pE x; mul(x, a, b); NTL_OPT_RETURN(vec_zz_pE, x); }

inline vec_zz_pE operator*(const zz_p& a, const vec_zz_pE& b)
   { vec_zz_pE x; mul(x, a, b); NTL_OPT_RETURN(vec_zz_pE, x); }

inline vec_zz_pE operator*(long a, const vec_zz_pE& b)
   { vec_zz_pE x; mul(x, a, b); NTL_OPT_RETURN(vec_zz_pE, x); }



zz_pE operator*(const vec_zz_pE& a, const vec_zz_pE& b);



// assignment operator notation:

inline vec_zz_pE& operator+=(vec_zz_pE& x, const vec_zz_pE& a)
{ 
   add(x, x, a);
   return x;
}

inline vec_zz_pE& operator-=(vec_zz_pE& x, const vec_zz_pE& a)
{ 
   sub(x, x, a);
   return x;
}

inline vec_zz_pE& operator*=(vec_zz_pE& x, const zz_pE& a)
{ 
   mul(x, x, a);
   return x;
}

inline vec_zz_pE& operator*=(vec_zz_pE& x, const zz_p& a)
{ 
   mul(x, x, a);
   return x;
}

inline vec_zz_pE& operator*=(vec_zz_pE& x, long a)
{ 
   mul(x, x, a);
   return x;
}

void random(vec_zz_pE& x, long n);
inline vec_zz_pE random_vec_zz_pE(long n)
   { vec_zz_pE x; random(x, n); NTL_OPT_RETURN(vec_zz_pE, x); }

NTL_CLOSE_NNS

#endif
