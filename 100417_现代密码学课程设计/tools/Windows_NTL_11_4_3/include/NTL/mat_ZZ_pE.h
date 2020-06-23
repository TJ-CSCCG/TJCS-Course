
#ifndef NTL_mat_ZZ_pE__H
#define NTL_mat_ZZ_pE__H

#include <NTL/vec_vec_ZZ_pE.h>

NTL_OPEN_NNS

typedef Mat<ZZ_pE> mat_ZZ_pE;

void add(mat_ZZ_pE& X, const mat_ZZ_pE& A, const mat_ZZ_pE& B); 
void sub(mat_ZZ_pE& X, const mat_ZZ_pE& A, const mat_ZZ_pE& B); 
void negate(mat_ZZ_pE& X, const mat_ZZ_pE& A); 
void mul(mat_ZZ_pE& X, const mat_ZZ_pE& A, const mat_ZZ_pE& B); 
void mul(vec_ZZ_pE& x, const mat_ZZ_pE& A, const vec_ZZ_pE& b); 
void mul(vec_ZZ_pE& x, const vec_ZZ_pE& a, const mat_ZZ_pE& B); 

void mul(mat_ZZ_pE& X, const mat_ZZ_pE& A, const ZZ_pE& b);

void mul(mat_ZZ_pE& X, const mat_ZZ_pE& A, const ZZ_p& b);
void mul(mat_ZZ_pE& X, const mat_ZZ_pE& A, long b);

inline void mul(mat_ZZ_pE& X, const ZZ_pE& a, const mat_ZZ_pE& B)
   { mul(X, B, a); }

inline void mul(mat_ZZ_pE& X, const ZZ_p& a, const mat_ZZ_pE& B)
   { mul(X, B, a); }

inline void mul(mat_ZZ_pE& X, long a, const mat_ZZ_pE& B)
   { mul(X, B, a); }


void ident(mat_ZZ_pE& X, long n); 
inline mat_ZZ_pE ident_mat_ZZ_pE(long n)
   { mat_ZZ_pE X; ident(X, n); NTL_OPT_RETURN(mat_ZZ_pE, X); }


void random(mat_ZZ_pE& x, long n, long m);
inline mat_ZZ_pE random_mat_ZZ_pE(long n, long m)
   { mat_ZZ_pE x; random(x, n, m); NTL_OPT_RETURN(mat_ZZ_pE, x); }


void determinant(ZZ_pE& d, const mat_ZZ_pE& A);
inline ZZ_pE determinant(const mat_ZZ_pE& A)
   {  ZZ_pE d; determinant(d, A); NTL_OPT_RETURN(ZZ_pE, d); }

long IsIdent(const mat_ZZ_pE& A, long n);

void transpose(mat_ZZ_pE& X, const mat_ZZ_pE& A);
inline mat_ZZ_pE transpose(const mat_ZZ_pE& A)
   { mat_ZZ_pE X; transpose(X, A); NTL_OPT_RETURN(mat_ZZ_pE, X); }

void solve(ZZ_pE& d, vec_ZZ_pE& x, const mat_ZZ_pE& A, const vec_ZZ_pE& b);
void solve(ZZ_pE& d, const mat_ZZ_pE& A, vec_ZZ_pE& x, const vec_ZZ_pE& b);

void inv(ZZ_pE& d, mat_ZZ_pE& X, const mat_ZZ_pE& A);

inline void sqr(mat_ZZ_pE& X, const mat_ZZ_pE& A)
   { mul(X, A, A); }

inline mat_ZZ_pE sqr(const mat_ZZ_pE& A)
   { mat_ZZ_pE X; sqr(X, A); NTL_OPT_RETURN(mat_ZZ_pE, X); }

void inv(mat_ZZ_pE& X, const mat_ZZ_pE& A);

inline mat_ZZ_pE inv(const mat_ZZ_pE& A)
   { mat_ZZ_pE X; inv(X, A); NTL_OPT_RETURN(mat_ZZ_pE, X); }

void power(mat_ZZ_pE& X, const mat_ZZ_pE& A, const ZZ& e);
inline mat_ZZ_pE power(const mat_ZZ_pE& A, const ZZ& e)
   { mat_ZZ_pE X; power(X, A, e); NTL_OPT_RETURN(mat_ZZ_pE, X); }

inline void power(mat_ZZ_pE& X, const mat_ZZ_pE& A, long e)
   { power(X, A, ZZ_expo(e)); }
inline mat_ZZ_pE power(const mat_ZZ_pE& A, long e)
   { mat_ZZ_pE X; power(X, A, e); NTL_OPT_RETURN(mat_ZZ_pE, X); }


void diag(mat_ZZ_pE& X, long n, const ZZ_pE& d);
inline mat_ZZ_pE diag(long n, const ZZ_pE& d)
   { mat_ZZ_pE X; diag(X, n, d); NTL_OPT_RETURN(mat_ZZ_pE, X); }

long IsDiag(const mat_ZZ_pE& A, long n, const ZZ_pE& d);


long gauss(mat_ZZ_pE& M);
long gauss(mat_ZZ_pE& M, long w);
void image(mat_ZZ_pE& X, const mat_ZZ_pE& A);
void kernel(mat_ZZ_pE& X, const mat_ZZ_pE& A);




void clear(mat_ZZ_pE& a);
// x = 0 (dimension unchanged)

long IsZero(const mat_ZZ_pE& a);
// test if a is the zero matrix (any dimension)


// operator notation:

mat_ZZ_pE operator+(const mat_ZZ_pE& a, const mat_ZZ_pE& b);
mat_ZZ_pE operator-(const mat_ZZ_pE& a, const mat_ZZ_pE& b);
mat_ZZ_pE operator*(const mat_ZZ_pE& a, const mat_ZZ_pE& b);

mat_ZZ_pE operator-(const mat_ZZ_pE& a);


// matrix/scalar multiplication:

inline mat_ZZ_pE operator*(const mat_ZZ_pE& a, const ZZ_pE& b)
   { mat_ZZ_pE x; mul(x, a, b); NTL_OPT_RETURN(mat_ZZ_pE, x); }
inline mat_ZZ_pE operator*(const mat_ZZ_pE& a, const ZZ_p& b)
   { mat_ZZ_pE x; mul(x, a, b); NTL_OPT_RETURN(mat_ZZ_pE, x); }
inline mat_ZZ_pE operator*(const mat_ZZ_pE& a, long b)
   { mat_ZZ_pE x; mul(x, a, b); NTL_OPT_RETURN(mat_ZZ_pE, x); }

inline mat_ZZ_pE operator*(const ZZ_pE& a, const mat_ZZ_pE& b)
   { mat_ZZ_pE x; mul(x, a, b); NTL_OPT_RETURN(mat_ZZ_pE, x); }
inline mat_ZZ_pE operator*(const ZZ_p& a, const mat_ZZ_pE& b)
   { mat_ZZ_pE x; mul(x, a, b); NTL_OPT_RETURN(mat_ZZ_pE, x); }
inline mat_ZZ_pE operator*(long a, const mat_ZZ_pE& b)
   { mat_ZZ_pE x; mul(x, a, b); NTL_OPT_RETURN(mat_ZZ_pE, x); }

// matrix/vector multiplication:

vec_ZZ_pE operator*(const mat_ZZ_pE& a, const vec_ZZ_pE& b);

vec_ZZ_pE operator*(const vec_ZZ_pE& a, const mat_ZZ_pE& b);




// assignment operator notation:

inline mat_ZZ_pE& operator+=(mat_ZZ_pE& x, const mat_ZZ_pE& a)
{
   add(x, x, a);
   return x;
}   

inline mat_ZZ_pE& operator-=(mat_ZZ_pE& x, const mat_ZZ_pE& a)
{
   sub(x, x, a);
   return x;
}   


inline mat_ZZ_pE& operator*=(mat_ZZ_pE& x, const mat_ZZ_pE& a)
{
   mul(x, x, a);
   return x;
}   

inline mat_ZZ_pE& operator*=(mat_ZZ_pE& x, const ZZ_pE& a)
{
   mul(x, x, a);
   return x;
}   

inline mat_ZZ_pE& operator*=(mat_ZZ_pE& x, const ZZ_p& a)
{
   mul(x, x, a);
   return x;
}   

inline mat_ZZ_pE& operator*=(mat_ZZ_pE& x, long a)
{
   mul(x, x, a);
   return x;
}   
   

inline vec_ZZ_pE& operator*=(vec_ZZ_pE& x, const mat_ZZ_pE& a)
{
   mul(x, x, a);
   return x;
}   

NTL_CLOSE_NNS

#endif
