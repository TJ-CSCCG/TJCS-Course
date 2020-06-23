
#ifndef NTL_mat_ZZ__H
#define NTL_mat_ZZ__H

#include <NTL/matrix.h>
#include <NTL/vec_vec_ZZ.h>
#include <NTL/mat_lzz_p.h>
#include <NTL/mat_ZZ_p.h>

NTL_OPEN_NNS

typedef Mat<ZZ> mat_ZZ;


void add(mat_ZZ& X, const mat_ZZ& A, const mat_ZZ& B); 
void sub(mat_ZZ& X, const mat_ZZ& A, const mat_ZZ& B); 
void negate(mat_ZZ& X, const mat_ZZ& A);
void mul(mat_ZZ& X, const mat_ZZ& A, const mat_ZZ& B); 
void mul(vec_ZZ& x, const mat_ZZ& A, const vec_ZZ& b); 
void mul(vec_ZZ& x, const vec_ZZ& a, const mat_ZZ& B); 

void mul(mat_ZZ& X, const mat_ZZ& A, const ZZ& b);
inline void mul(mat_ZZ& X, const ZZ& a, const mat_ZZ& B)
   { mul(X, B, a); }

void mul(mat_ZZ& X, const mat_ZZ& A, long b);
inline void mul(mat_ZZ& X, long a, const mat_ZZ& B)
   { mul(X, B, a); }

void ident(mat_ZZ& X, long n); 
inline mat_ZZ ident_mat_ZZ(long n)
   { mat_ZZ X; ident(X, n); NTL_OPT_RETURN(mat_ZZ, X); }

long IsIdent(const mat_ZZ& A, long n);
void diag(mat_ZZ& X, long n, const ZZ& d);
inline mat_ZZ diag(long n, const ZZ& d)
   { mat_ZZ X; diag(X, n, d); NTL_OPT_RETURN(mat_ZZ, X); }

long IsDiag(const mat_ZZ& A, long n, const ZZ& d);

void determinant(ZZ& d, const mat_ZZ& A, long deterministic=0);
void solve(ZZ& d, vec_ZZ& x,
           const mat_ZZ& A, const vec_ZZ& b,
           long deterministic=0);

void solve1(ZZ& d_out, vec_ZZ& x_out, const mat_ZZ& A, const vec_ZZ& b);


inline
void HenselSolve1(ZZ& d_out, vec_ZZ& x_out, const mat_ZZ& A, const vec_ZZ& b)
   { solve1(d_out, x_out, A, b); }
// for backward compatability only


void inv(ZZ& d, mat_ZZ& X, const mat_ZZ& A, long deterministic=0);

inline void sqr(mat_ZZ& X, const mat_ZZ& A)
   { mul(X, A, A); }

inline mat_ZZ sqr(const mat_ZZ& A)
   { mat_ZZ X; sqr(X, A); NTL_OPT_RETURN(mat_ZZ, X); }

void inv(mat_ZZ& X, const mat_ZZ& A);

inline mat_ZZ inv(const mat_ZZ& A)
   { mat_ZZ X; inv(X, A); NTL_OPT_RETURN(mat_ZZ, X); }

void power(mat_ZZ& X, const mat_ZZ& A, const ZZ& e);
inline mat_ZZ power(const mat_ZZ& A, const ZZ& e)
   { mat_ZZ X; power(X, A, e); NTL_OPT_RETURN(mat_ZZ, X); }

inline void power(mat_ZZ& X, const mat_ZZ& A, long e)
   { power(X, A, ZZ_expo(e)); }
inline mat_ZZ power(const mat_ZZ& A, long e)
   { mat_ZZ X; power(X, A, e); NTL_OPT_RETURN(mat_ZZ, X); }



void transpose(mat_ZZ& X, const mat_ZZ& A);
inline mat_ZZ transpose(const mat_ZZ& A)
   { mat_ZZ x; transpose(x, A); NTL_OPT_RETURN(mat_ZZ, x); }

void conv(mat_zz_p& x, const mat_ZZ& a);
inline mat_zz_p to_mat_zz_p(const mat_ZZ& a)
   { mat_zz_p x; conv(x, a); NTL_OPT_RETURN(mat_zz_p, x); }

void conv(mat_ZZ_p& x, const mat_ZZ& a);
inline mat_ZZ_p to_mat_ZZ_p(const mat_ZZ& a)
   { mat_ZZ_p x; conv(x, a); NTL_OPT_RETURN(mat_ZZ_p, x); }

long CRT(mat_ZZ& g, ZZ& a, const mat_zz_p& G);


// miscellaneous:

inline ZZ determinant(const mat_ZZ& a, long deterministic=0)
   { ZZ x; determinant(x, a, deterministic); return x; }

// functional variant of determinant

void clear(mat_ZZ& a);
// x = 0 (dimension unchanged)

long IsZero(const mat_ZZ& a);
// test if a is the zero matrix (any dimension)


// operator notation:

mat_ZZ operator+(const mat_ZZ& a, const mat_ZZ& b);
mat_ZZ operator-(const mat_ZZ& a, const mat_ZZ& b);
mat_ZZ operator*(const mat_ZZ& a, const mat_ZZ& b);

mat_ZZ operator-(const mat_ZZ& a);


// matrix/scalar multiplication:

inline mat_ZZ operator*(const mat_ZZ& a, const ZZ& b)
   { mat_ZZ x; mul(x, a, b); NTL_OPT_RETURN(mat_ZZ, x); }

inline mat_ZZ operator*(const mat_ZZ& a, long b)
   { mat_ZZ x; mul(x, a, b); NTL_OPT_RETURN(mat_ZZ, x); }

inline mat_ZZ operator*(const ZZ& a, const mat_ZZ& b)
   { mat_ZZ x; mul(x, a, b); NTL_OPT_RETURN(mat_ZZ, x); }

inline mat_ZZ operator*(long a, const mat_ZZ& b)
   { mat_ZZ x; mul(x, a, b); NTL_OPT_RETURN(mat_ZZ, x); }


// matrix/vector multiplication:

vec_ZZ operator*(const mat_ZZ& a, const vec_ZZ& b);

vec_ZZ operator*(const vec_ZZ& a, const mat_ZZ& b);



// assignment operator notation:

inline mat_ZZ& operator+=(mat_ZZ& x, const mat_ZZ& a)
{
   add(x, x, a);
   return x;
}   

inline mat_ZZ& operator-=(mat_ZZ& x, const mat_ZZ& a)
{
   sub(x, x, a);
   return x;
}   


inline mat_ZZ& operator*=(mat_ZZ& x, const mat_ZZ& a)
{
   mul(x, x, a);
   return x;
}   

inline mat_ZZ& operator*=(mat_ZZ& x, const ZZ& a)
{
   mul(x, x, a);
   return x;
}   
   
inline mat_ZZ& operator*=(mat_ZZ& x, long a)
{
   mul(x, x, a);
   return x;
}   

inline vec_ZZ& operator*=(vec_ZZ& x, const mat_ZZ& a)
{
   mul(x, x, a);
   return x;
}   

NTL_CLOSE_NNS




#endif
