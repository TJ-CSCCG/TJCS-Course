
#ifndef NTL_vec_ZZ_pE__H
#define NTL_vec_ZZ_pE__H

#include <NTL/ZZ_pE.h>

NTL_OPEN_NNS

typedef Vec<ZZ_pE> vec_ZZ_pE;


void mul(vec_ZZ_pE& x, const vec_ZZ_pE& a, const ZZ_pE& b);
inline void mul(vec_ZZ_pE& x, const ZZ_pE& a, const vec_ZZ_pE& b)
   { mul(x, b, a); }

void mul(vec_ZZ_pE& x, const vec_ZZ_pE& a, const ZZ_p& b);
inline void mul(vec_ZZ_pE& x, const ZZ_p& a, const vec_ZZ_pE& b)
   { mul(x, b, a); }

void mul(vec_ZZ_pE& x, const vec_ZZ_pE& a, long b);
inline void mul(vec_ZZ_pE& x, long a, const vec_ZZ_pE& b)
   { mul(x, b, a); }

void add(vec_ZZ_pE& x, const vec_ZZ_pE& a, const vec_ZZ_pE& b);
void sub(vec_ZZ_pE& x, const vec_ZZ_pE& a, const vec_ZZ_pE& b);

void negate(vec_ZZ_pE& x, const vec_ZZ_pE& a);

void clear(vec_ZZ_pE& x);


void InnerProduct(ZZ_pE& x, const vec_ZZ_pE& a, const vec_ZZ_pE& b);
void InnerProduct(ZZ_pE& x, const vec_ZZ_pE& a, const vec_ZZ_pE& b,
                  long offset);


long IsZero(const vec_ZZ_pE& a);

void VectorCopy(vec_ZZ_pE& x, const vec_ZZ_pE& a, long n);
inline vec_ZZ_pE VectorCopy(const vec_ZZ_pE& a, long n)
   { vec_ZZ_pE x; VectorCopy(x, a, n); NTL_OPT_RETURN(vec_ZZ_pE, x); }



vec_ZZ_pE operator+(const vec_ZZ_pE& a, const vec_ZZ_pE& b);
vec_ZZ_pE operator-(const vec_ZZ_pE& a, const vec_ZZ_pE& b);
vec_ZZ_pE operator-(const vec_ZZ_pE& a);

inline vec_ZZ_pE operator*(const vec_ZZ_pE& a, const ZZ_pE& b)
   { vec_ZZ_pE x; mul(x, a, b); NTL_OPT_RETURN(vec_ZZ_pE, x); }

inline vec_ZZ_pE operator*(const vec_ZZ_pE& a, const ZZ_p& b)
   { vec_ZZ_pE x; mul(x, a, b); NTL_OPT_RETURN(vec_ZZ_pE, x); }

inline vec_ZZ_pE operator*(const vec_ZZ_pE& a, long b)
   { vec_ZZ_pE x; mul(x, a, b); NTL_OPT_RETURN(vec_ZZ_pE, x); }

inline vec_ZZ_pE operator*(const ZZ_pE& a, const vec_ZZ_pE& b)
   { vec_ZZ_pE x; mul(x, a, b); NTL_OPT_RETURN(vec_ZZ_pE, x); }

inline vec_ZZ_pE operator*(const ZZ_p& a, const vec_ZZ_pE& b)
   { vec_ZZ_pE x; mul(x, a, b); NTL_OPT_RETURN(vec_ZZ_pE, x); }

inline vec_ZZ_pE operator*(long a, const vec_ZZ_pE& b)
   { vec_ZZ_pE x; mul(x, a, b); NTL_OPT_RETURN(vec_ZZ_pE, x); }



ZZ_pE operator*(const vec_ZZ_pE& a, const vec_ZZ_pE& b);



// assignment operator notation:

inline vec_ZZ_pE& operator+=(vec_ZZ_pE& x, const vec_ZZ_pE& a)
{ 
   add(x, x, a);
   return x;
}

inline vec_ZZ_pE& operator-=(vec_ZZ_pE& x, const vec_ZZ_pE& a)
{ 
   sub(x, x, a);
   return x;
}

inline vec_ZZ_pE& operator*=(vec_ZZ_pE& x, const ZZ_pE& a)
{ 
   mul(x, x, a);
   return x;
}

inline vec_ZZ_pE& operator*=(vec_ZZ_pE& x, const ZZ_p& a)
{ 
   mul(x, x, a);
   return x;
}

inline vec_ZZ_pE& operator*=(vec_ZZ_pE& x, long a)
{ 
   mul(x, x, a);
   return x;
}

void random(vec_ZZ_pE& x, long n);
inline vec_ZZ_pE random_vec_ZZ_pE(long n)
   { vec_ZZ_pE x; random(x, n); NTL_OPT_RETURN(vec_ZZ_pE, x); }

NTL_CLOSE_NNS

#endif
