
#ifndef NTL_mat_GF2E__H
#define NTL_mat_GF2E__H

#include <NTL/matrix.h>
#include <NTL/vec_vec_GF2E.h>

NTL_OPEN_NNS

typedef Mat<GF2E> mat_GF2E;

void add(mat_GF2E& X, const mat_GF2E& A, const mat_GF2E& B); 
inline void sub(mat_GF2E& X, const mat_GF2E& A, const mat_GF2E& B)
   { add(X, A, B); }
inline void negate(mat_GF2E& X, const mat_GF2E& A)
   { X = A; }
void mul(mat_GF2E& X, const mat_GF2E& A, const mat_GF2E& B); 
void mul(vec_GF2E& x, const mat_GF2E& A, const vec_GF2E& b); 
void mul(vec_GF2E& x, const vec_GF2E& a, const mat_GF2E& B); 

void mul(mat_GF2E& X, const mat_GF2E& A, const GF2E& b);
inline void mul(mat_GF2E& X, const GF2E& a, const mat_GF2E& B)
   { mul(X, B, a); }

void mul(mat_GF2E& X, const mat_GF2E& A, GF2 b);
inline void mul(mat_GF2E& X, GF2 a, const mat_GF2E& B)
   { mul(X, B, a); }

inline void mul(mat_GF2E& X, const mat_GF2E& A, long b)
   { mul(X, A, to_GF2(b)); }
inline void mul(mat_GF2E& X, long a, const mat_GF2E& B)
   { mul(X, B, a); }

void ident(mat_GF2E& X, long n); 
inline mat_GF2E ident_mat_GF2E(long n)
   { mat_GF2E X; ident(X, n); NTL_OPT_RETURN(mat_GF2E, X); }

void random(mat_GF2E& x, long n, long m);
inline mat_GF2E random_mat_GF2E(long n, long m)
   { mat_GF2E x; random(x, n, m); NTL_OPT_RETURN(mat_GF2E, x); }


void determinant(GF2E& d, const mat_GF2E& A);
long IsIdent(const mat_GF2E& A, long n);
void transpose(mat_GF2E& X, const mat_GF2E& A);
void solve(GF2E& d, vec_GF2E& x, const mat_GF2E& A, const vec_GF2E& b);
void solve(GF2E& d, const mat_GF2E& A, vec_GF2E& x, const vec_GF2E& b);
void inv(GF2E& d, mat_GF2E& X, const mat_GF2E& A);

inline void sqr(mat_GF2E& X, const mat_GF2E& A)
   { mul(X, A, A); }

inline mat_GF2E sqr(const mat_GF2E& A)
   { mat_GF2E X; sqr(X, A); NTL_OPT_RETURN(mat_GF2E, X); }

void inv(mat_GF2E& X, const mat_GF2E& A);

inline mat_GF2E inv(const mat_GF2E& A)
   { mat_GF2E X; inv(X, A); NTL_OPT_RETURN(mat_GF2E, X); }

void power(mat_GF2E& X, const mat_GF2E& A, const ZZ& e);
inline mat_GF2E power(const mat_GF2E& A, const ZZ& e)
   { mat_GF2E X; power(X, A, e); NTL_OPT_RETURN(mat_GF2E, X); }

inline void power(mat_GF2E& X, const mat_GF2E& A, long e)
   { power(X, A, ZZ_expo(e)); }
inline mat_GF2E power(const mat_GF2E& A, long e)
   { mat_GF2E X; power(X, A, e); NTL_OPT_RETURN(mat_GF2E, X); }


void diag(mat_GF2E& X, long n, const GF2E& d);
inline mat_GF2E diag(long n, const GF2E& d)
   { mat_GF2E X; diag(X, n, d); NTL_OPT_RETURN(mat_GF2E, X); }


long IsDiag(const mat_GF2E& A, long n, const GF2E& d);


long gauss(mat_GF2E& M);
long gauss(mat_GF2E& M, long w);
void image(mat_GF2E& X, const mat_GF2E& A);
void kernel(mat_GF2E& X, const mat_GF2E& A);



// miscellaneous:

inline GF2E determinant(const mat_GF2E& a)
   { GF2E x; determinant(x, a); return x; }
// functional variant of determinant

inline mat_GF2E transpose(const mat_GF2E& a)
   { mat_GF2E x; transpose(x, a); NTL_OPT_RETURN(mat_GF2E, x); }

void clear(mat_GF2E& a);
// x = 0 (dimension unchanged)

long IsZero(const mat_GF2E& a);
// test if a is the zero matrix (any dimension)


// operator notation:

mat_GF2E operator+(const mat_GF2E& a, const mat_GF2E& b);
mat_GF2E operator-(const mat_GF2E& a, const mat_GF2E& b);
mat_GF2E operator*(const mat_GF2E& a, const mat_GF2E& b);

mat_GF2E operator-(const mat_GF2E& a);


// matrix/scalar multiplication:

inline mat_GF2E operator*(const mat_GF2E& a, const GF2E& b)
   { mat_GF2E x; mul(x, a, b); NTL_OPT_RETURN(mat_GF2E, x); }

inline mat_GF2E operator*(const mat_GF2E& a, GF2 b)
   { mat_GF2E x; mul(x, a, b); NTL_OPT_RETURN(mat_GF2E, x); }

inline mat_GF2E operator*(const mat_GF2E& a, long b)
   { mat_GF2E x; mul(x, a, b); NTL_OPT_RETURN(mat_GF2E, x); }

inline mat_GF2E operator*(const GF2E& a, const mat_GF2E& b)
   { mat_GF2E x; mul(x, a, b); NTL_OPT_RETURN(mat_GF2E, x); }

inline mat_GF2E operator*(GF2 a, const mat_GF2E& b)
   { mat_GF2E x; mul(x, a, b); NTL_OPT_RETURN(mat_GF2E, x); }

inline mat_GF2E operator*(long a, const mat_GF2E& b)
   { mat_GF2E x; mul(x, a, b); NTL_OPT_RETURN(mat_GF2E, x); }


// matrix/vector multiplication:

vec_GF2E operator*(const mat_GF2E& a, const vec_GF2E& b);

vec_GF2E operator*(const vec_GF2E& a, const mat_GF2E& b);




// assignment operator notation:

inline mat_GF2E& operator+=(mat_GF2E& x, const mat_GF2E& a)
{
   add(x, x, a);
   return x;
}   

inline mat_GF2E& operator-=(mat_GF2E& x, const mat_GF2E& a)
{
   sub(x, x, a);
   return x;
}   


inline mat_GF2E& operator*=(mat_GF2E& x, const mat_GF2E& a)
{
   mul(x, x, a);
   return x;
}   

inline mat_GF2E& operator*=(mat_GF2E& x, const GF2E& a)
{
   mul(x, x, a);
   return x;
}   

inline mat_GF2E& operator*=(mat_GF2E& x, GF2 a)
{
   mul(x, x, a);
   return x;
}   

inline mat_GF2E& operator*=(mat_GF2E& x, long a)
{
   mul(x, x, a);
   return x;
}   
   

inline vec_GF2E& operator*=(vec_GF2E& x, const mat_GF2E& a)
{
   mul(x, x, a);
   return x;
}   


NTL_CLOSE_NNS



#endif
