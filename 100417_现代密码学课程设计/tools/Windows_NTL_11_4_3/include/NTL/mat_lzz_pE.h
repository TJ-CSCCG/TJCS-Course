
#ifndef NTL_mat_zz_pE__H
#define NTL_mat_zz_pE__H

#include <NTL/vec_vec_lzz_pE.h>

NTL_OPEN_NNS

typedef Mat<zz_pE> mat_zz_pE;

void add(mat_zz_pE& X, const mat_zz_pE& A, const mat_zz_pE& B); 
void sub(mat_zz_pE& X, const mat_zz_pE& A, const mat_zz_pE& B); 
void negate(mat_zz_pE& X, const mat_zz_pE& A); 
void mul(mat_zz_pE& X, const mat_zz_pE& A, const mat_zz_pE& B); 
void mul(vec_zz_pE& x, const mat_zz_pE& A, const vec_zz_pE& b); 
void mul(vec_zz_pE& x, const vec_zz_pE& a, const mat_zz_pE& B); 

void mul(mat_zz_pE& X, const mat_zz_pE& A, const zz_pE& b);

void mul(mat_zz_pE& X, const mat_zz_pE& A, const zz_p& b);
void mul(mat_zz_pE& X, const mat_zz_pE& A, long b);

inline void mul(mat_zz_pE& X, const zz_pE& a, const mat_zz_pE& B)
   { mul(X, B, a); }

inline void mul(mat_zz_pE& X, const zz_p& a, const mat_zz_pE& B)
   { mul(X, B, a); }

inline void mul(mat_zz_pE& X, long a, const mat_zz_pE& B)
   { mul(X, B, a); }


void ident(mat_zz_pE& X, long n); 
inline mat_zz_pE ident_mat_zz_pE(long n)
   { mat_zz_pE X; ident(X, n); NTL_OPT_RETURN(mat_zz_pE, X); }

void random(mat_zz_pE& x, long n, long m);
inline mat_zz_pE random_mat_zz_pE(long n, long m)
   { mat_zz_pE x; random(x, n, m); NTL_OPT_RETURN(mat_zz_pE, x); }



void determinant(zz_pE& d, const mat_zz_pE& A);
inline zz_pE determinant(const mat_zz_pE& A)
   {  zz_pE d; determinant(d, A); NTL_OPT_RETURN(zz_pE, d); }

long IsIdent(const mat_zz_pE& A, long n);

void transpose(mat_zz_pE& X, const mat_zz_pE& A);
inline mat_zz_pE transpose(const mat_zz_pE& A)
   { mat_zz_pE X; transpose(X, A); NTL_OPT_RETURN(mat_zz_pE, X); }

void solve(zz_pE& d, vec_zz_pE& x, const mat_zz_pE& A, const vec_zz_pE& b);
void solve(zz_pE& d, const mat_zz_pE& A, vec_zz_pE& x, const vec_zz_pE& b);

void inv(zz_pE& d, mat_zz_pE& X, const mat_zz_pE& A);

inline void sqr(mat_zz_pE& X, const mat_zz_pE& A)
   { mul(X, A, A); }

inline mat_zz_pE sqr(const mat_zz_pE& A)
   { mat_zz_pE X; sqr(X, A); NTL_OPT_RETURN(mat_zz_pE, X); }

void inv(mat_zz_pE& X, const mat_zz_pE& A);

inline mat_zz_pE inv(const mat_zz_pE& A)
   { mat_zz_pE X; inv(X, A); NTL_OPT_RETURN(mat_zz_pE, X); }

void power(mat_zz_pE& X, const mat_zz_pE& A, const ZZ& e);
inline mat_zz_pE power(const mat_zz_pE& A, const ZZ& e)
   { mat_zz_pE X; power(X, A, e); NTL_OPT_RETURN(mat_zz_pE, X); }

inline void power(mat_zz_pE& X, const mat_zz_pE& A, long e)
   { power(X, A, ZZ_expo(e)); }
inline mat_zz_pE power(const mat_zz_pE& A, long e)
   { mat_zz_pE X; power(X, A, e); NTL_OPT_RETURN(mat_zz_pE, X); }


void diag(mat_zz_pE& X, long n, const zz_pE& d);
inline mat_zz_pE diag(long n, const zz_pE& d)
   { mat_zz_pE X; diag(X, n, d); NTL_OPT_RETURN(mat_zz_pE, X); }

long IsDiag(const mat_zz_pE& A, long n, const zz_pE& d);


long gauss(mat_zz_pE& M);
long gauss(mat_zz_pE& M, long w);
void image(mat_zz_pE& X, const mat_zz_pE& A);
void kernel(mat_zz_pE& X, const mat_zz_pE& A);




void clear(mat_zz_pE& a);
// x = 0 (dimension unchanged)

long IsZero(const mat_zz_pE& a);
// test if a is the zero matrix (any dimension)


// operator notation:

mat_zz_pE operator+(const mat_zz_pE& a, const mat_zz_pE& b);
mat_zz_pE operator-(const mat_zz_pE& a, const mat_zz_pE& b);
mat_zz_pE operator*(const mat_zz_pE& a, const mat_zz_pE& b);

mat_zz_pE operator-(const mat_zz_pE& a);


// matrix/scalar multiplication:

inline mat_zz_pE operator*(const mat_zz_pE& a, const zz_pE& b)
   { mat_zz_pE x; mul(x, a, b); NTL_OPT_RETURN(mat_zz_pE, x); }
inline mat_zz_pE operator*(const mat_zz_pE& a, const zz_p& b)
   { mat_zz_pE x; mul(x, a, b); NTL_OPT_RETURN(mat_zz_pE, x); }
inline mat_zz_pE operator*(const mat_zz_pE& a, long b)
   { mat_zz_pE x; mul(x, a, b); NTL_OPT_RETURN(mat_zz_pE, x); }

inline mat_zz_pE operator*(const zz_pE& a, const mat_zz_pE& b)
   { mat_zz_pE x; mul(x, a, b); NTL_OPT_RETURN(mat_zz_pE, x); }
inline mat_zz_pE operator*(const zz_p& a, const mat_zz_pE& b)
   { mat_zz_pE x; mul(x, a, b); NTL_OPT_RETURN(mat_zz_pE, x); }
inline mat_zz_pE operator*(long a, const mat_zz_pE& b)
   { mat_zz_pE x; mul(x, a, b); NTL_OPT_RETURN(mat_zz_pE, x); }

// matrix/vector multiplication:

vec_zz_pE operator*(const mat_zz_pE& a, const vec_zz_pE& b);

vec_zz_pE operator*(const vec_zz_pE& a, const mat_zz_pE& b);




// assignment operator notation:

inline mat_zz_pE& operator+=(mat_zz_pE& x, const mat_zz_pE& a)
{
   add(x, x, a);
   return x;
}   

inline mat_zz_pE& operator-=(mat_zz_pE& x, const mat_zz_pE& a)
{
   sub(x, x, a);
   return x;
}   


inline mat_zz_pE& operator*=(mat_zz_pE& x, const mat_zz_pE& a)
{
   mul(x, x, a);
   return x;
}   

inline mat_zz_pE& operator*=(mat_zz_pE& x, const zz_pE& a)
{
   mul(x, x, a);
   return x;
}   

inline mat_zz_pE& operator*=(mat_zz_pE& x, const zz_p& a)
{
   mul(x, x, a);
   return x;
}   

inline mat_zz_pE& operator*=(mat_zz_pE& x, long a)
{
   mul(x, x, a);
   return x;
}   
   

inline vec_zz_pE& operator*=(vec_zz_pE& x, const mat_zz_pE& a)
{
   mul(x, x, a);
   return x;
}   

NTL_CLOSE_NNS

#endif
