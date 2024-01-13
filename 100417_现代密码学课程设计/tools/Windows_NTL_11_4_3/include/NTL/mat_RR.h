
#ifndef NTL_mat_RR__H
#define NTL_mat_RR__H

#include <NTL/matrix.h>
#include <NTL/vec_vec_RR.h>

NTL_OPEN_NNS

typedef Mat<RR> mat_RR;

void add(mat_RR& X, const mat_RR& A, const mat_RR& B); 
void sub(mat_RR& X, const mat_RR& A, const mat_RR& B); 
void negate(mat_RR& X, const mat_RR& A);
void mul(mat_RR& X, const mat_RR& A, const mat_RR& B); 
void mul(vec_RR& x, const mat_RR& A, const vec_RR& b); 
void mul(vec_RR& x, const vec_RR& a, const mat_RR& B); 

void mul(mat_RR& X, const mat_RR& A, const RR& b);
void mul(mat_RR& X, const mat_RR& A, double b);

inline void mul(mat_RR& X, const RR& a, const mat_RR& B)
   { mul(X, B, a); }

inline void mul(mat_RR& X, double a, const mat_RR& B)
   { mul(X, B, a); }

void ident(mat_RR& X, long n); 
inline mat_RR ident_mat_RR(long n)
   { mat_RR X; ident(X, n); NTL_OPT_RETURN(mat_RR, X); }

void determinant(RR& d, const mat_RR& A);
long IsIdent(const mat_RR& A, long n);
void transpose(mat_RR& X, const mat_RR& A);
void solve(RR& d, vec_RR& X,
           const mat_RR& A, const vec_RR& b);
void inv(RR& d, mat_RR& X, const mat_RR& A);

inline void sqr(mat_RR& X, const mat_RR& A)
   { mul(X, A, A); }

inline mat_RR sqr(const mat_RR& A)
   { mat_RR X; sqr(X, A); NTL_OPT_RETURN(mat_RR, X); }

void inv(mat_RR& X, const mat_RR& A);

inline mat_RR inv(const mat_RR& A)
   { mat_RR X; inv(X, A); NTL_OPT_RETURN(mat_RR, X); }

void power(mat_RR& X, const mat_RR& A, const ZZ& e);
inline mat_RR power(const mat_RR& A, const ZZ& e)
   { mat_RR X; power(X, A, e); NTL_OPT_RETURN(mat_RR, X); }

inline void power(mat_RR& X, const mat_RR& A, long e)
   { power(X, A, ZZ_expo(e)); }
inline mat_RR power(const mat_RR& A, long e)
   { mat_RR X; power(X, A, e); NTL_OPT_RETURN(mat_RR, X); }



void diag(mat_RR& X, long n, const RR& d);
inline mat_RR diag(long n, const RR& d)
   { mat_RR X; diag(X, n, d); NTL_OPT_RETURN(mat_RR, X); }

long IsDiag(const mat_RR& A, long n, const RR& d);


// miscellaneous:

RR determinant(const mat_RR& a);
// functional variant of determinant

inline mat_RR transpose(const mat_RR & a)
   { mat_RR x; transpose(x, a); NTL_OPT_RETURN(mat_RR, x); }


void clear(mat_RR& a);
// x = 0 (dimension unchanged)

long IsZero(const mat_RR& a);
// test if a is the zero matrix (any dimension)


// operator notation:

mat_RR operator+(const mat_RR& a, const mat_RR& b);
mat_RR operator-(const mat_RR& a, const mat_RR& b);
mat_RR operator*(const mat_RR& a, const mat_RR& b);

mat_RR operator-(const mat_RR& a);


// matrix/vector multiplication:

vec_RR operator*(const mat_RR& a, const vec_RR& b);

vec_RR operator*(const vec_RR& a, const mat_RR& b);



// matrix/scalar multiplication:

inline mat_RR operator*(const mat_RR& a, const RR& b)
   { mat_RR x; mul(x, a, b); NTL_OPT_RETURN(mat_RR, x); }

inline mat_RR operator*(const mat_RR& a, double b)
   { mat_RR x; mul(x, a, b); NTL_OPT_RETURN(mat_RR, x); }

inline mat_RR operator*(const RR& a, const mat_RR& b)
   { mat_RR x; mul(x, a, b); NTL_OPT_RETURN(mat_RR, x); }

inline mat_RR operator*(double a, const mat_RR& b)
   { mat_RR x; mul(x, a, b); NTL_OPT_RETURN(mat_RR, x); }




// assignment operator notation:

inline mat_RR& operator+=(mat_RR& x, const mat_RR& a)
{
   add(x, x, a);
   return x;
}   

inline mat_RR& operator-=(mat_RR& x, const mat_RR& a)
{
   sub(x, x, a);
   return x;
}   


inline mat_RR& operator*=(mat_RR& x, const mat_RR& a)
{
   mul(x, x, a);
   return x;
}   

inline mat_RR& operator*=(mat_RR& x, const RR& a)
{
   mul(x, x, a);
   return x;
}   

inline mat_RR& operator*=(mat_RR& x, double a)
{
   mul(x, x, a);
   return x;
}   
   

inline vec_RR& operator*=(vec_RR& x, const mat_RR& a)
{
   mul(x, x, a);
   return x;
}   


NTL_CLOSE_NNS


#endif
