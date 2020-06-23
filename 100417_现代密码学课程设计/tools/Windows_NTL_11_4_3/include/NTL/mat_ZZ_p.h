
#ifndef NTL_mat_ZZ_p__H
#define NTL_mat_ZZ_p__H

#include <NTL/tools.h>
#include <NTL/matrix.h>
#include <NTL/vec_vec_ZZ_p.h>
#include <NTL/SmartPtr.h>

NTL_OPEN_NNS

typedef Mat<ZZ_p> mat_ZZ_p;



// ****************** opaque stuff **************

struct mat_ZZ_p_opaque_body {
   virtual ~mat_ZZ_p_opaque_body() { }
   virtual mat_ZZ_p_opaque_body* clone() const = 0;
   virtual long NumRows() const = 0; 
   virtual long NumCols() const = 0; 
   virtual void mul(mat_ZZ_p& X, const mat_ZZ_p& A) const = 0;
   virtual void mul_transpose(mat_ZZ_p& X, const mat_ZZ_p& A) const = 0;
};

mat_ZZ_p_opaque_body *mat_ZZ_p_opaque_body_move(mat_ZZ_p& A);

struct mat_ZZ_p_opaque {

   CopiedPtr<mat_ZZ_p_opaque_body,CloningCopiedPtrPolicy> ptr;

   void move(mat_ZZ_p& A)
   {
      this->ptr.reset(mat_ZZ_p_opaque_body_move(A));
   }

   long NumRows() const 
   {
      return ptr ? ptr->NumRows() : 0;
   }

   long NumCols() const 
   {
      return ptr ? ptr->NumCols() : 0;
   }

   bool initialized() const 
   {
      return ptr != 0;
   }

};

inline
void mul(mat_ZZ_p& X, const mat_ZZ_p& A, const mat_ZZ_p_opaque& B)
{
   if (!B.ptr) 
      LogicError("mul: uninitialzed mat_ZZ_p_opaque");
   else
      B.ptr->mul(X, A);
}

inline
void mul_transpose(mat_ZZ_p& X, const mat_ZZ_p& A, const mat_ZZ_p_opaque& B)
{
   if (!B.ptr) 
      LogicError("mul: uninitialzed mat_ZZ_p_opaque");
   else
      B.ptr->mul_transpose(X, A);
}





void add(mat_ZZ_p& X, const mat_ZZ_p& A, const mat_ZZ_p& B); 
void sub(mat_ZZ_p& X, const mat_ZZ_p& A, const mat_ZZ_p& B); 
void negate(mat_ZZ_p& X, const mat_ZZ_p& A); 
void mul(mat_ZZ_p& X, const mat_ZZ_p& A, const mat_ZZ_p& B); 
void mul(vec_ZZ_p& x, const mat_ZZ_p& A, const vec_ZZ_p& b); 
void mul(vec_ZZ_p& x, const vec_ZZ_p& a, const mat_ZZ_p& B); 

void mul(mat_ZZ_p& X, const mat_ZZ_p& A, const ZZ_p& b);
void mul(mat_ZZ_p& X, const mat_ZZ_p& A, long b);

inline void mul(mat_ZZ_p& X, const ZZ_p& a, const mat_ZZ_p& B)
   { mul(X, B, a); }

inline void mul(mat_ZZ_p& X, long a, const mat_ZZ_p& B)
   { mul(X, B, a); }

void ident(mat_ZZ_p& X, long n); 
inline mat_ZZ_p ident_mat_ZZ_p(long n)
   { mat_ZZ_p X; ident(X, n); NTL_OPT_RETURN(mat_ZZ_p, X); }


void random(mat_ZZ_p& x, long n, long m);
inline mat_ZZ_p random_mat_ZZ_p(long n, long m)
   { mat_ZZ_p x; random(x, n, m); NTL_OPT_RETURN(mat_ZZ_p, x); }



void determinant(ZZ_p& d, const mat_ZZ_p& A);
long IsIdent(const mat_ZZ_p& A, long n);
void transpose(mat_ZZ_p& X, const mat_ZZ_p& A);
void solve(ZZ_p& d, vec_ZZ_p& X, const mat_ZZ_p& A, const vec_ZZ_p& b);
void solve(ZZ_p& d, const mat_ZZ_p& A, vec_ZZ_p& x, const vec_ZZ_p& b);
void inv(ZZ_p& d, mat_ZZ_p& X, const mat_ZZ_p& A);

inline void sqr(mat_ZZ_p& X, const mat_ZZ_p& A)
   { mul(X, A, A); }

inline mat_ZZ_p sqr(const mat_ZZ_p& A)
   { mat_ZZ_p X; sqr(X, A); NTL_OPT_RETURN(mat_ZZ_p, X); }

void inv(mat_ZZ_p& X, const mat_ZZ_p& A);

inline mat_ZZ_p inv(const mat_ZZ_p& A)
   { mat_ZZ_p X; inv(X, A); NTL_OPT_RETURN(mat_ZZ_p, X); }

void power(mat_ZZ_p& X, const mat_ZZ_p& A, const ZZ& e);
inline mat_ZZ_p power(const mat_ZZ_p& A, const ZZ& e)
   { mat_ZZ_p X; power(X, A, e); NTL_OPT_RETURN(mat_ZZ_p, X); }

inline void power(mat_ZZ_p& X, const mat_ZZ_p& A, long e)
   { power(X, A, ZZ_expo(e)); }
inline mat_ZZ_p power(const mat_ZZ_p& A, long e)
   { mat_ZZ_p X; power(X, A, e); NTL_OPT_RETURN(mat_ZZ_p, X); }


void diag(mat_ZZ_p& X, long n, const ZZ_p& d);
inline mat_ZZ_p diag(long n, const ZZ_p& d)
   { mat_ZZ_p X; diag(X, n, d); NTL_OPT_RETURN(mat_ZZ_p, X); }

long IsDiag(const mat_ZZ_p& A, long n, const ZZ_p& d);


long gauss(mat_ZZ_p& M);
long gauss(mat_ZZ_p& M, long w);
void image(mat_ZZ_p& X, const mat_ZZ_p& A);
void kernel(mat_ZZ_p& X, const mat_ZZ_p& A);




inline ZZ_p determinant(const mat_ZZ_p& a)
   { ZZ_p x; determinant(x, a); return x; }
// functional variant of determinant

inline mat_ZZ_p transpose(const mat_ZZ_p & a)
   { mat_ZZ_p x; transpose(x, a); NTL_OPT_RETURN(mat_ZZ_p, x); }

void clear(mat_ZZ_p& a);
// x = 0 (dimension unchanged)

long IsZero(const mat_ZZ_p& a);
// test if a is the zero matrix (any dimension)


// operator notation:

mat_ZZ_p operator+(const mat_ZZ_p& a, const mat_ZZ_p& b);
mat_ZZ_p operator-(const mat_ZZ_p& a, const mat_ZZ_p& b);
mat_ZZ_p operator*(const mat_ZZ_p& a, const mat_ZZ_p& b);

mat_ZZ_p operator-(const mat_ZZ_p& a);


// matrix/scalar multiplication:

inline mat_ZZ_p operator*(const mat_ZZ_p& a, const ZZ_p& b)
   { mat_ZZ_p x; mul(x, a, b); NTL_OPT_RETURN(mat_ZZ_p, x); }

inline mat_ZZ_p operator*(const mat_ZZ_p& a, long b)
   { mat_ZZ_p x; mul(x, a, b); NTL_OPT_RETURN(mat_ZZ_p, x); }

inline mat_ZZ_p operator*(const ZZ_p& a, const mat_ZZ_p& b)
   { mat_ZZ_p x; mul(x, a, b); NTL_OPT_RETURN(mat_ZZ_p, x); }

inline mat_ZZ_p operator*(long a, const mat_ZZ_p& b)
   { mat_ZZ_p x; mul(x, a, b); NTL_OPT_RETURN(mat_ZZ_p, x); }

// matrix/vector multiplication:

vec_ZZ_p operator*(const mat_ZZ_p& a, const vec_ZZ_p& b);

vec_ZZ_p operator*(const vec_ZZ_p& a, const mat_ZZ_p& b);




// assignment operator notation:

inline mat_ZZ_p& operator+=(mat_ZZ_p& x, const mat_ZZ_p& a)
{
   add(x, x, a);
   return x;
}   

inline mat_ZZ_p& operator-=(mat_ZZ_p& x, const mat_ZZ_p& a)
{
   sub(x, x, a);
   return x;
}   


inline mat_ZZ_p& operator*=(mat_ZZ_p& x, const mat_ZZ_p& a)
{
   mul(x, x, a);
   return x;
}   

inline mat_ZZ_p& operator*=(mat_ZZ_p& x, const ZZ_p& a)
{
   mul(x, x, a);
   return x;
}   

inline mat_ZZ_p& operator*=(mat_ZZ_p& x, long a)
{
   mul(x, x, a);
   return x;
}   
   

inline vec_ZZ_p& operator*=(vec_ZZ_p& x, const mat_ZZ_p& a)
{
   mul(x, x, a);
   return x;
}   

NTL_CLOSE_NNS



#endif
