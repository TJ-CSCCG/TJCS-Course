

#ifndef NTL_GF2EX__H
#define NTL_GF2EX__H

#include <NTL/vector.h>
#include <NTL/GF2E.h>
#include <NTL/vec_GF2E.h>
#include <NTL/vec_long.h>
#include <NTL/GF2XVec.h>
#include <NTL/Lazy.h>


NTL_OPEN_NNS

class GF2EXModulus; // forward declaration

class GF2EX {
public:
typedef GF2E coeff_type;
typedef GF2EXModulus modulus_type;


vec_GF2E rep;


/***************************************************************

          Constructors, Destructors, and Assignment

****************************************************************/


GF2EX() { }

explicit GF2EX(long a) { *this = a; }
explicit GF2EX(GF2 a) { *this = a; }
explicit GF2EX(const GF2& a) { *this = a; }


GF2EX(INIT_SIZE_TYPE, long n) { rep.SetMaxLength(n); }

// default copy constructor and assignment
// default destructor

void normalize();
// strip leading zeros

void SetMaxLength(long n) 
// pre-allocate space for n coefficients.
// Value is unchanged

   { rep.SetMaxLength(n); }


void kill() 
// free space held by this polynomial.  Value becomes 0.

   { rep.kill(); }



void SetLength(long n) { rep.SetLength(n); }
GF2E& operator[](long i) { return rep[i]; }
const GF2E& operator[](long i) const { return rep[i]; }




static const GF2EX& zero();



inline GF2EX& operator=(long a);
inline GF2EX& operator=(GF2 a);
inline GF2EX& operator=(const GF2E& a);

inline GF2EX(long i, long a);
inline GF2EX(long i, GF2 a);
inline GF2EX(long i, const GF2E& a);


inline GF2EX(INIT_MONO_TYPE, long i, long a);
inline GF2EX(INIT_MONO_TYPE, long i, GF2 a);
inline GF2EX(INIT_MONO_TYPE, long i, const GF2E& a);
inline GF2EX(INIT_MONO_TYPE, long i);


GF2EX(GF2EX& x, INIT_TRANS_TYPE) : rep(x.rep, INIT_TRANS) { }

void swap(GF2EX& x) { rep.swap(x.rep); }


};



NTL_DECLARE_RELOCATABLE((GF2EX*))




/********************************************************************

                           input and output

*********************************************************************/


NTL_SNS istream& operator>>(NTL_SNS istream& s, GF2EX& x);
NTL_SNS ostream& operator<<(NTL_SNS ostream& s, const GF2EX& a);




/**********************************************************

                   Some utility routines

***********************************************************/


inline long deg(const GF2EX& a) { return a.rep.length() - 1; }

const GF2E& coeff(const GF2EX& a, long i);
// zero if i not in range

void GetCoeff(GF2E& x, const GF2EX& a, long i);
// x = a[i], or zero if i not in range

const GF2E& LeadCoeff(const GF2EX& a);
// zero if a == 0

const GF2E& ConstTerm(const GF2EX& a);
// zero if a == 0

void SetCoeff(GF2EX& x, long i, const GF2E& a);
void SetCoeff(GF2EX& x, long i, GF2 a);
void SetCoeff(GF2EX& x, long i, long a);
// x[i] = a, error is raised if i < 0

void SetCoeff(GF2EX& x, long i);
// x[i] = 1, error is raised if i < 0

inline GF2EX::GF2EX(long i, const GF2E& a) { SetCoeff(*this, i, a); }
inline GF2EX::GF2EX(long i, GF2 a) { SetCoeff(*this, i, a); }
inline GF2EX::GF2EX(long i, long a) { SetCoeff(*this, i, a); }


inline GF2EX::GF2EX(INIT_MONO_TYPE, long i, const GF2E& a) { SetCoeff(*this, i, a); }
inline GF2EX::GF2EX(INIT_MONO_TYPE, long i, GF2 a) { SetCoeff(*this, i, a); }
inline GF2EX::GF2EX(INIT_MONO_TYPE, long i, long a) { SetCoeff(*this, i, a); }
inline GF2EX::GF2EX(INIT_MONO_TYPE, long i) { SetCoeff(*this, i); }


void SetX(GF2EX& x);
// x is set to the monomial X

long IsX(const GF2EX& a);
// test if x = X

inline void clear(GF2EX& x) 
// x = 0

   { x.rep.SetLength(0); }

inline void set(GF2EX& x)
// x = 1

   { x.rep.SetLength(1); set(x.rep[0]); }

inline void swap(GF2EX& x, GF2EX& y)
// swap x & y (only pointers are swapped)

   { x.swap(y); }

void random(GF2EX& x, long n);
inline GF2EX random_GF2EX(long n)
   { GF2EX x; random(x, n); NTL_OPT_RETURN(GF2EX, x); }
// generate a random polynomial of degree < n 

void trunc(GF2EX& x, const GF2EX& a, long m);
inline GF2EX trunc(const GF2EX& a, long m)
   { GF2EX x; trunc(x, a, m); NTL_OPT_RETURN(GF2EX, x); }
// x = a % X^m

void RightShift(GF2EX& x, const GF2EX& a, long n);
inline GF2EX RightShift(const GF2EX& a, long n)
   { GF2EX x; RightShift(x, a, n); NTL_OPT_RETURN(GF2EX, x); }
// x = a/X^n

void LeftShift(GF2EX& x, const GF2EX& a, long n);
inline GF2EX LeftShift(const GF2EX& a, long n)
   { GF2EX x; LeftShift(x, a, n); NTL_OPT_RETURN(GF2EX, x); }
// x = a*X^n

#ifndef NTL_TRANSITION

inline GF2EX operator>>(const GF2EX& a, long n)
   { GF2EX x; RightShift(x, a, n); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator<<(const GF2EX& a, long n)
   { GF2EX x; LeftShift(x, a, n); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX& operator<<=(GF2EX& x, long n)
   { LeftShift(x, x, n); return x; }

inline GF2EX& operator>>=(GF2EX& x, long n)
   { RightShift(x, x, n); return x; }

#endif



void diff(GF2EX& x, const GF2EX& a);
inline GF2EX diff(const GF2EX& a)
   { GF2EX x; diff(x, a); NTL_OPT_RETURN(GF2EX, x); }
// x = derivative of a



void MakeMonic(GF2EX& x);

void reverse(GF2EX& c, const GF2EX& a, long hi);

inline GF2EX reverse(const GF2EX& a, long hi)
   { GF2EX x; reverse(x, a, hi); NTL_OPT_RETURN(GF2EX, x); }

inline void reverse(GF2EX& c, const GF2EX& a)
{  reverse(c, a, deg(a)); }

inline GF2EX reverse(const GF2EX& a)
   { GF2EX x; reverse(x, a); NTL_OPT_RETURN(GF2EX, x); }

inline void VectorCopy(vec_GF2E& x, const GF2EX& a, long n)
   { VectorCopy(x, a.rep, n); }

inline vec_GF2E VectorCopy(const GF2EX& a, long n)
   { return VectorCopy(a.rep, n); }




/*******************************************************************

                        conversion routines

********************************************************************/



void conv(GF2EX& x, long a);
void conv(GF2EX& x, GF2 a);
void conv(GF2EX& x, const GF2E& a);
void conv(GF2EX& x, const ZZ& a);

#ifndef NTL_TRANSITION
void conv(GF2EX& x, const GF2X& a);
#endif

void conv(GF2EX& x, const vec_GF2E& a);

inline GF2EX to_GF2EX(long a)
   { GF2EX x; conv(x, a); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX to_GF2EX(GF2 a)
   { GF2EX x; conv(x, a); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX to_GF2EX(const GF2E& a)
   { GF2EX x; conv(x, a); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX to_GF2EX(const ZZ& a)
   { GF2EX x; conv(x, a); NTL_OPT_RETURN(GF2EX, x); }

#ifndef NTL_TRANSITION
inline GF2EX to_GF2EX(const GF2X& a)
   { GF2EX x; conv(x, a); NTL_OPT_RETURN(GF2EX, x); }
#endif

inline GF2EX to_GF2EX(const vec_GF2E& a)
   { GF2EX x; conv(x, a); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX& GF2EX::operator=(const GF2E& a) { conv(*this, a); return *this; }
inline GF2EX& GF2EX::operator=(GF2 a) { conv(*this, a); return *this; }
inline GF2EX& GF2EX::operator=(long a) { conv(*this, a); return *this; }




/* additional legacy conversions for v6 conversion regime */

inline void conv(GF2EX& x, const GF2EX& a)
   { x = a; }

inline void conv(vec_GF2E& x, const GF2EX& a)
   { x = a.rep; }

class ZZX;
void conv(GF2EX& x, const ZZX& a);


/* ------------------------------------- */





/*************************************************************

                        Comparison

**************************************************************/

long IsZero(const GF2EX& a); 

long IsOne(const GF2EX& a);

inline long operator==(const GF2EX& a, const GF2EX& b)
   { return a.rep == b.rep; }

long operator==(const GF2EX& a, const GF2E& b);
long operator==(const GF2EX& a, GF2 b);
long operator==(const GF2EX& a, long b);

inline long operator==(const GF2E& a, const GF2EX& b) { return b == a; }
inline long operator==(GF2 a, const GF2EX& b) { return b == a; }
inline long operator==(long a, const GF2EX& b) { return b == a; }

inline long operator!=(const GF2EX& a, const GF2EX& b) { return !(a == b); }

inline long operator!=(const GF2EX& a, const GF2E& b) { return !(a == b); }
inline long operator!=(const GF2EX& a, GF2 b) { return !(a == b); }
inline long operator!=(const GF2EX& a, long b) { return !(a == b); }

inline long operator!=(const GF2E& a, const GF2EX& b) { return !(a == b); }
inline long operator!=(GF2 a, const GF2EX& b) { return !(a == b); }
inline long operator!=(long a, const GF2EX& b) { return !(a == b); }


/***************************************************************

                         Addition

****************************************************************/

void add(GF2EX& x, const GF2EX& a, const GF2EX& b);
// x = a + b

void add(GF2EX& x, const GF2EX& a, const GF2E& b);
void add(GF2EX& x, const GF2EX& a, GF2 b);
void add(GF2EX& x, const GF2EX& a, long);

inline void add(GF2EX& x, const GF2E& a, const GF2EX& b) { add(x, b, a); }
inline void add(GF2EX& x, GF2 a, const GF2EX& b) { add(x, b, a); }
inline void add(GF2EX& x, long a, const GF2EX& b) { add(x, b, a); }

inline void sub(GF2EX& x, const GF2EX& a, const GF2EX& b) { add(x, a, b); }

inline void sub(GF2EX& x, const GF2EX& a, const GF2E& b) { add(x, a, b); }
inline void sub(GF2EX& x, const GF2EX& a, GF2 b) { add(x, a, b); }
inline void sub(GF2EX& x, const GF2EX& a, long b) { add(x, a, b); }

inline void sub(GF2EX& x, const GF2E& a, const GF2EX& b) { add(x, a, b); }
inline void sub(GF2EX& x, GF2 a, const GF2EX& b) { add(x, a, b); }
inline void sub(GF2EX& x, long a, const GF2EX& b) { add(x, a, b); }

inline void negate(GF2EX& x, const GF2EX& a) { x = a; }




inline GF2EX operator+(const GF2EX& a, const GF2EX& b)
   { GF2EX x; add(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator+(const GF2EX& a, const GF2E& b)
   { GF2EX x; add(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator+(const GF2EX& a, GF2 b)
   { GF2EX x; add(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator+(const GF2EX& a, long b)
   { GF2EX x; add(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator+(const GF2E& a, const GF2EX& b)
   { GF2EX x; add(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator+(GF2 a, const GF2EX& b)
   { GF2EX x; add(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator+(long a, const GF2EX& b)
   { GF2EX x; add(x, a, b); NTL_OPT_RETURN(GF2EX, x); }


inline GF2EX operator-(const GF2EX& a, const GF2EX& b)
   { GF2EX x; sub(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator-(const GF2EX& a, const GF2E& b)
   { GF2EX x; sub(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator-(const GF2EX& a, GF2 b)
   { GF2EX x; sub(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator-(const GF2EX& a, long b)
   { GF2EX x; sub(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator-(const GF2E& a, const GF2EX& b)
   { GF2EX x; sub(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator-(GF2 a, const GF2EX& b)
   { GF2EX x; sub(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator-(long a, const GF2EX& b)
   { GF2EX x; sub(x, a, b); NTL_OPT_RETURN(GF2EX, x); }


inline GF2EX& operator+=(GF2EX& x, const GF2EX& b)
   { add(x, x, b); return x; }

inline GF2EX& operator+=(GF2EX& x, const GF2E& b)
   { add(x, x, b); return x; }

inline GF2EX& operator+=(GF2EX& x, GF2 b)
   { add(x, x, b); return x; }

inline GF2EX& operator+=(GF2EX& x, long b)
   { add(x, x, b); return x; }

inline GF2EX& operator-=(GF2EX& x, const GF2EX& b)
   { sub(x, x, b); return x; }

inline GF2EX& operator-=(GF2EX& x, const GF2E& b)
   { sub(x, x, b); return x; }

inline GF2EX& operator-=(GF2EX& x, GF2 b)
   { sub(x, x, b); return x; }

inline GF2EX& operator-=(GF2EX& x, long b)
   { sub(x, x, b); return x; }


inline GF2EX operator-(const GF2EX& a) 
   { GF2EX x; negate(x, a); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX& operator++(GF2EX& x) { add(x, x, 1); return x; }
inline void operator++(GF2EX& x, int) { add(x, x, 1); }
inline GF2EX& operator--(GF2EX& x) { sub(x, x, 1); return x; }
inline void operator--(GF2EX& x, int) { sub(x, x, 1); }


/*****************************************************************

                        Multiplication

******************************************************************/


void mul(GF2EX& x, const GF2EX& a, const GF2EX& b);
// x = a * b

void sqr(GF2EX& x, const GF2EX& a);
inline GF2EX sqr(const GF2EX& a)
   { GF2EX x; sqr(x, a); NTL_OPT_RETURN(GF2EX, x); }
// x = a^2

void mul(GF2EX & x, const GF2EX& a, const GF2E& b);
void mul(GF2EX & x, const GF2EX& a, GF2 b);
void mul(GF2EX & x, const GF2EX& a, long b);

inline void mul(GF2EX& x, const GF2E& a, const GF2EX& b) { mul(x, b, a); }
inline void mul(GF2EX& x, GF2 a, const GF2EX& b) { mul(x, b, a); }
inline void mul(GF2EX& x, long a, const GF2EX& b) { mul(x, b, a); }

void MulTrunc(GF2EX& x, const GF2EX& a, const GF2EX& b, long n);
inline GF2EX MulTrunc(const GF2EX& a, const GF2EX& b, long n)
   { GF2EX x; MulTrunc(x, a, b, n); NTL_OPT_RETURN(GF2EX, x); }
// x = a * b % X^n

void SqrTrunc(GF2EX& x, const GF2EX& a, long n);
inline GF2EX SqrTrunc(const GF2EX& a, long n)
   { GF2EX x; SqrTrunc(x, a, n); NTL_OPT_RETURN(GF2EX, x); }
// x = a*a % X^n


inline GF2EX operator*(const GF2EX& a, const GF2EX& b)
   { GF2EX x; mul(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator*(const GF2EX& a, const GF2E& b)
   { GF2EX x; mul(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator*(const GF2EX& a, GF2 b)
   { GF2EX x; mul(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator*(const GF2EX& a, long b)
   { GF2EX x; mul(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator*(const GF2E& a, const GF2EX& b)
   { GF2EX x; mul(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator*(GF2 a, const GF2EX& b)
   { GF2EX x; mul(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator*(long a, const GF2EX& b)
   { GF2EX x; mul(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX& operator*=(GF2EX& x, const GF2EX& b)
   { mul(x, x, b); return x; }

inline GF2EX& operator*=(GF2EX& x, const GF2E& b)
   { mul(x, x, b); return x; }

inline GF2EX& operator*=(GF2EX& x, GF2 b)
   { mul(x, x, b); return x; }

inline GF2EX& operator*=(GF2EX& x, long b)
   { mul(x, x, b); return x; }


void power(GF2EX& x, const GF2EX& a, long e);
inline GF2EX power(const GF2EX& a, long e)
   { GF2EX x; power(x, a, e); NTL_OPT_RETURN(GF2EX, x); }




/*************************************************************

                      Division

**************************************************************/

void DivRem(GF2EX& q, GF2EX& r, const GF2EX& a, const GF2EX& b);
// q = a/b, r = a%b

void div(GF2EX& q, const GF2EX& a, const GF2EX& b);
void div(GF2EX& q, const GF2EX& a, const GF2E& b);
void div(GF2EX& q, const GF2EX& a, GF2 b);
void div(GF2EX& q, const GF2EX& a, long b);
// q = a/b

void rem(GF2EX& r, const GF2EX& a, const GF2EX& b);
// r = a%b

long divide(GF2EX& q, const GF2EX& a, const GF2EX& b);
// if b | a, sets q = a/b and returns 1; otherwise returns 0

long divide(const GF2EX& a, const GF2EX& b);
// if b | a, sets q = a/b and returns 1; otherwise returns 0

void InvTrunc(GF2EX& x, const GF2EX& a, long m);
inline GF2EX InvTrunc(const GF2EX& a, long m)
   { GF2EX x; InvTrunc(x, a, m); NTL_OPT_RETURN(GF2EX, x); }

// computes x = a^{-1} % X^m 
// constant term must be non-zero



inline GF2EX operator/(const GF2EX& a, const GF2EX& b)
   { GF2EX x; div(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator/(const GF2EX& a, const GF2E& b)
   { GF2EX x; div(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator/(const GF2EX& a, GF2 b)
   { GF2EX x; div(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator/(const GF2EX& a, long b)
   { GF2EX x; div(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX& operator/=(GF2EX& x, const GF2EX& b)
   { div(x, x, b); return x; }

inline GF2EX& operator/=(GF2EX& x, const GF2E& b)
   { div(x, x, b); return x; }

inline GF2EX& operator/=(GF2EX& x, GF2 b)
   { div(x, x, b); return x; }

inline GF2EX& operator/=(GF2EX& x, long b)
   { div(x, x, b); return x; }


inline GF2EX operator%(const GF2EX& a, const GF2EX& b)
   { GF2EX x; rem(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX& operator%=(GF2EX& x, const GF2EX& b)
   { rem(x, x, b); return x; }




/***********************************************************

                         GCD's

************************************************************/


void GCD(GF2EX& x, const GF2EX& a, const GF2EX& b);
inline GF2EX GCD(const GF2EX& a, const GF2EX& b)
   { GF2EX x; GCD(x, a, b); NTL_OPT_RETURN(GF2EX, x); }

// x = GCD(a, b),  x is always monic (or zero if a==b==0).

void XGCD(GF2EX& d, GF2EX& s, GF2EX& t, const GF2EX& a, const GF2EX& b);
// d = gcd(a,b), a s + b t = d 


/*************************************************************

             Modular Arithmetic without pre-conditioning

**************************************************************/

// arithmetic mod f.
// all inputs and outputs are polynomials of degree less than deg(f).
// ASSUMPTION: f is assumed monic, and deg(f) > 0.
// NOTE: if you want to do many computations with a fixed f,
//       use the GF2EXModulus data structure and associated routines below.



void MulMod(GF2EX& x, const GF2EX& a, const GF2EX& b, const GF2EX& f);
inline GF2EX MulMod(const GF2EX& a, const GF2EX& b, const GF2EX& f)
   { GF2EX x; MulMod(x, a, b, f); NTL_OPT_RETURN(GF2EX, x); }
// x = (a * b) % f

void SqrMod(GF2EX& x, const GF2EX& a, const GF2EX& f);
inline GF2EX SqrMod(const GF2EX& a, const GF2EX& f)
   { GF2EX x; SqrMod(x, a, f); NTL_OPT_RETURN(GF2EX, x); }
// x = a^2 % f

void MulByXMod(GF2EX& x, const GF2EX& a, const GF2EX& f);
inline GF2EX MulByXMod(const GF2EX& a, const GF2EX& f)
   { GF2EX x; MulByXMod(x, a, f); NTL_OPT_RETURN(GF2EX, x); }
// x = (a * X) mod f

void InvMod(GF2EX& x, const GF2EX& a, const GF2EX& f);
inline GF2EX InvMod(const GF2EX& a, const GF2EX& f)
   { GF2EX x; InvMod(x, a, f); NTL_OPT_RETURN(GF2EX, x); }
// x = a^{-1} % f, error is a is not invertible

long InvModStatus(GF2EX& x, const GF2EX& a, const GF2EX& f);
// if (a, f) = 1, returns 0 and sets x = a^{-1} % f
// otherwise, returns 1 and sets x = (a, f)





/******************************************************************

        Modular Arithmetic with Pre-conditioning

*******************************************************************/


// If you need to do a lot of arithmetic modulo a fixed f,
// build GF2EXModulus F for f.  This pre-computes information about f
// that speeds up the computation a great deal.

class GF2EXModulus {
public:
   GF2EXModulus();

   GF2EXModulus(const GF2EX& ff);

   GF2EX f;   // the modulus

   operator const GF2EX& () const { return f; }
   const GF2EX& val() const { return f; }

   long n; //  deg(f)

   long method; // GF2EX_MOD_PLAIN or GF2EX_MOD_MUL 

   GF2EX h0;
   GF2E hlc;
   GF2EX f0;

   OptionalVal< Lazy<vec_GF2E> > tracevec;
   // extra level of indirection to ensure class is relocatable

}; 


NTL_DECLARE_RELOCATABLE((GF2EXModulus*))
inline long deg(const GF2EXModulus& F) { return F.n; }

void build(GF2EXModulus& F, const GF2EX& f);



void rem(GF2EX& r, const GF2EX& a, const GF2EXModulus& F);
   
void DivRem(GF2EX& q, GF2EX& r, const GF2EX& a, const GF2EXModulus& F);

void div(GF2EX& q, const GF2EX& a, const GF2EXModulus& F);

void MulMod(GF2EX& c, const GF2EX& a, const GF2EX& b, const GF2EXModulus& F);
inline GF2EX MulMod(const GF2EX& a, const GF2EX& b, const GF2EXModulus& F)
   { GF2EX x; MulMod(x, a, b, F); NTL_OPT_RETURN(GF2EX, x); }

void SqrMod(GF2EX& c, const GF2EX& a, const GF2EXModulus& F);
inline GF2EX SqrMod(const GF2EX& a, const GF2EXModulus& F)
   { GF2EX x; SqrMod(x, a, F); NTL_OPT_RETURN(GF2EX, x); }


void PowerMod(GF2EX& h, const GF2EX& g, const ZZ& e, const GF2EXModulus& F);

inline void PowerMod(GF2EX& h, const GF2EX& g, long e, const GF2EXModulus& F)
   { PowerMod(h, g, ZZ_expo(e), F); }

inline GF2EX PowerMod(const GF2EX& g, const ZZ& e, const GF2EXModulus& F)
   { GF2EX x; PowerMod(x, g, e, F);  NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX PowerMod(const GF2EX& g, long e, const GF2EXModulus& F)
   { GF2EX x; PowerMod(x, g, e, F);  NTL_OPT_RETURN(GF2EX, x); }

void PowerXMod(GF2EX& hh, const ZZ& e, const GF2EXModulus& F);

inline void PowerXMod(GF2EX& h, long e, const GF2EXModulus& F)
   { PowerXMod(h, ZZ_expo(e), F); }


inline GF2EX PowerXMod(const ZZ& e, const GF2EXModulus& F)
   { GF2EX x; PowerXMod(x, e, F);  NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX PowerXMod(long e, const GF2EXModulus& F)
   { GF2EX x; PowerXMod(x, e, F);  NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX operator%(const GF2EX& a, const GF2EXModulus& F)
   { GF2EX x; rem(x, a, F); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX& operator%=(GF2EX& x, const GF2EXModulus& F)
   { rem(x, x, F); return x; }

inline GF2EX operator/(const GF2EX& a, const GF2EXModulus& F)
   { GF2EX x; div(x, a, F); NTL_OPT_RETURN(GF2EX, x); }

inline GF2EX& operator/=(GF2EX& x, const GF2EXModulus& F)
   { div(x, x, F); return x; }



/*****************************************************************

                       vectors of GF2EX's

*****************************************************************/



typedef Vec<GF2EX> vec_GF2EX;



/*******************************************************

              Evaluation and related problems

********************************************************/


void BuildFromRoots(GF2EX& x, const vec_GF2E& a);
inline GF2EX BuildFromRoots(const vec_GF2E& a)
   { GF2EX x; BuildFromRoots(x, a); NTL_OPT_RETURN(GF2EX, x); }
// computes the polynomial (X-a[0]) ... (X-a[n-1]), where n = a.length()


void eval(GF2E& b, const GF2EX& f, const GF2E& a);
inline GF2E eval(const GF2EX& f, const GF2E& a)
   { GF2E x; eval(x, f, a); NTL_OPT_RETURN(GF2E, x); }
// b = f(a)

void eval(vec_GF2E& b, const GF2EX& f, const vec_GF2E& a);
inline vec_GF2E eval(const GF2EX& f, const vec_GF2E& a)
   { vec_GF2E x; eval(x, f, a); NTL_OPT_RETURN(vec_GF2E, x); }
//  b[i] = f(a[i])

inline void eval(GF2E& b, const GF2X& f, const GF2E& a)
   { conv(b, CompMod(f, rep(a), GF2E::modulus())); }
   
inline GF2E eval(const GF2X& f, const GF2E& a)
   { GF2E x; eval(x, f, a); NTL_OPT_RETURN(GF2E, x); }
// b = f(a)


void interpolate(GF2EX& f, const vec_GF2E& a, const vec_GF2E& b);
inline GF2EX interpolate(const vec_GF2E& a, const vec_GF2E& b)
   { GF2EX x; interpolate(x, a, b); NTL_OPT_RETURN(GF2EX, x); }
// computes f such that f(a[i]) = b[i]




/**********************************************************

         Modular Composition and Minimal Polynomials

***********************************************************/


// algorithms for computing g(h) mod f




void CompMod(GF2EX& x, const GF2EX& g, const GF2EX& h, const GF2EXModulus& F);
inline GF2EX 
CompMod(const GF2EX& g, const GF2EX& h, const GF2EXModulus& F)
   { GF2EX x; CompMod(x, g, h, F); NTL_OPT_RETURN(GF2EX, x); }
// x = g(h) mod f

void Comp2Mod(GF2EX& x1, GF2EX& x2, const GF2EX& g1, const GF2EX& g2,
              const GF2EX& h, const GF2EXModulus& F);
// xi = gi(h) mod f (i=1,2)

void Comp3Mod(GF2EX& x1, GF2EX& x2, GF2EX& x3, 
              const GF2EX& g1, const GF2EX& g2, const GF2EX& g3,
              const GF2EX& h, const GF2EXModulus& F);
// xi = gi(h) mod f (i=1..3)



// The routine build (see below) which is implicitly called
// by the various compose and UpdateMap routines builds a table
// of polynomials.  
// If GF2EXArgBound > 0, then the table is limited in
// size to approximamtely that many KB.
// If GF2EXArgBound <= 0, then it is ignored, and space is allocated
// so as to maximize speed.
// Initially, GF2EXArgBound = 0.


// If a single h is going to be used with many g's
// then you should build a GF2EXArgument for h,
// and then use the compose routine below.
// build computes and stores h, h^2, ..., h^m mod f.
// After this pre-computation, composing a polynomial of degree 
// roughly n with h takes n/m multiplies mod f, plus n^2
// scalar multiplies.
// Thus, increasing m increases the space requirement and the pre-computation
// time, but reduces the composition time.
// If GF2EXArgBound > 0, a table of size less than m may be built.

struct GF2EXArgument {
   vec_GF2EX H;
};

extern 
NTL_CHEAP_THREAD_LOCAL
long GF2EXArgBound;


void build(GF2EXArgument& H, const GF2EX& h, const GF2EXModulus& F, long m);

// m must be > 0, otherwise an error is raised

void CompMod(GF2EX& x, const GF2EX& g, const GF2EXArgument& H, 
             const GF2EXModulus& F);

inline GF2EX 
CompMod(const GF2EX& g, const GF2EXArgument& H, const GF2EXModulus& F)
   { GF2EX x; CompMod(x, g, H, F); NTL_OPT_RETURN(GF2EX, x); }
   



void MinPolySeq(GF2EX& h, const vec_GF2E& a, long m);
inline GF2EX MinPolySeq(const vec_GF2E& a, long m)
   { GF2EX x; MinPolySeq(x, a, m); NTL_OPT_RETURN(GF2EX, x); }


void MinPolyMod(GF2EX& hh, const GF2EX& g, const GF2EXModulus& F);
inline GF2EX MinPolyMod(const GF2EX& g, const GF2EXModulus& F)
   { GF2EX x; MinPolyMod(x, g, F); NTL_OPT_RETURN(GF2EX, x); }


void MinPolyMod(GF2EX& hh, const GF2EX& g, const GF2EXModulus& F, long m);
inline GF2EX MinPolyMod(const GF2EX& g, const GF2EXModulus& F, long m)
   { GF2EX x; MinPolyMod(x, g, F, m); NTL_OPT_RETURN(GF2EX, x); }

void ProbMinPolyMod(GF2EX& hh, const GF2EX& g, const GF2EXModulus& F);
inline GF2EX ProbMinPolyMod(const GF2EX& g, const GF2EXModulus& F)
   { GF2EX x; ProbMinPolyMod(x, g, F); NTL_OPT_RETURN(GF2EX, x); }

void ProbMinPolyMod(GF2EX& hh, const GF2EX& g, const GF2EXModulus& F, long m);
inline GF2EX ProbMinPolyMod(const GF2EX& g, const GF2EXModulus& F, long m)
   { GF2EX x; ProbMinPolyMod(x, g, F, m); NTL_OPT_RETURN(GF2EX, x); }

void IrredPolyMod(GF2EX& h, const GF2EX& g, const GF2EXModulus& F);
inline GF2EX IrredPolyMod(const GF2EX& g, const GF2EXModulus& F)
   { GF2EX x; IrredPolyMod(x, g, F); NTL_OPT_RETURN(GF2EX, x); }

void IrredPolyMod(GF2EX& h, const GF2EX& g, const GF2EXModulus& F, long m);
inline GF2EX IrredPolyMod(const GF2EX& g, const GF2EXModulus& F, long m)
   { GF2EX x; IrredPolyMod(x, g, F, m); NTL_OPT_RETURN(GF2EX, x); }


struct GF2EXTransMultiplier {
   GF2EX f0, fbi, b;
   long shamt, shamt_fbi, shamt_b;
};

void build(GF2EXTransMultiplier& B, const GF2EX& b, const GF2EXModulus& F);

void TransMulMod(GF2EX& x, const GF2EX& a, const GF2EXTransMultiplier& B,
               const GF2EXModulus& F);

void UpdateMap(vec_GF2E& x, const vec_GF2E& a, 
         const GF2EXTransMultiplier& B, const GF2EXModulus& F);

inline vec_GF2E UpdateMap(const vec_GF2E& a,
         const GF2EXTransMultiplier& B, const GF2EXModulus& F)
   { vec_GF2E x; UpdateMap(x, a, B, F); NTL_OPT_RETURN(vec_GF2E, x); }

void ProjectPowers(vec_GF2E& x, const vec_GF2E& a, long k, 
                   const GF2EXArgument& H, const GF2EXModulus& F);
inline vec_GF2E ProjectPowers(const vec_GF2E& a, long k, 
                   const GF2EXArgument& H, const GF2EXModulus& F)
   { vec_GF2E x; ProjectPowers(x, a, k, H, F); NTL_OPT_RETURN(vec_GF2E, x); }

void ProjectPowers(vec_GF2E& x, const vec_GF2E& a, long k, const GF2EX& h, 
                   const GF2EXModulus& F);
inline vec_GF2E ProjectPowers(const vec_GF2E& a, long k, 
                   const GF2EX& H, const GF2EXModulus& F)
   { vec_GF2E x; ProjectPowers(x, a, k, H, F); NTL_OPT_RETURN(vec_GF2E, x); }

inline void project(GF2E& x, const vec_GF2E& a, const GF2EX& b)
   { InnerProduct(x, a, b.rep); }

inline GF2E project(const vec_GF2E& a, const GF2EX& b)
   { GF2E x; InnerProduct(x, a, b.rep); NTL_OPT_RETURN(GF2E, x); }

/**********************************************************

         Modular Composition and Minimal Polynomials
                         in towers

***********************************************************/

// composition

void CompTower(GF2EX& x, const GF2X& g, const GF2EXArgument& A,
             const GF2EXModulus& F);

inline GF2EX CompTower(const GF2X& g, const GF2EXArgument& A,
             const GF2EXModulus& F)
   { GF2EX x; CompTower(x, g, A, F); NTL_OPT_RETURN(GF2EX, x); }

void CompTower(GF2EX& x, const GF2X& g, const GF2EX& h,
             const GF2EXModulus& F);

inline GF2EX CompTower(const GF2X& g, const GF2EX& h,
             const GF2EXModulus& F)
   { GF2EX x; CompTower(x, g, h, F); NTL_OPT_RETURN(GF2EX, x); }

// prob min poly

void ProbMinPolyTower(GF2X& h, const GF2EX& g, const GF2EXModulus& F,
                      long m);

inline GF2X ProbMinPolyTower(const GF2EX& g, const GF2EXModulus& F,
                      long m)
   { GF2X x; ProbMinPolyTower(x, g, F, m); NTL_OPT_RETURN(GF2X, x); }

inline void ProbMinPolyTower(GF2X& h, const GF2EX& g, 
                             const GF2EXModulus& F)
   { ProbMinPolyTower(h, g, F, deg(F)*GF2E::degree()); }

inline GF2X ProbMinPolyTower(const GF2EX& g, const GF2EXModulus& F)
   { GF2X x; ProbMinPolyTower(x, g, F); NTL_OPT_RETURN(GF2X, x); }


// min poly


void MinPolyTower(GF2X& h, const GF2EX& g, const GF2EXModulus& F,
                      long m);

inline GF2X MinPolyTower(const GF2EX& g, const GF2EXModulus& F,
                      long m)
   { GF2X x; MinPolyTower(x, g, F, m); NTL_OPT_RETURN(GF2X, x); }

inline void MinPolyTower(GF2X& h, const GF2EX& g, 
                             const GF2EXModulus& F)
   { MinPolyTower(h, g, F, deg(F)*GF2E::degree()); }

inline GF2X MinPolyTower(const GF2EX& g, const GF2EXModulus& F)
   { GF2X x; MinPolyTower(x, g, F); NTL_OPT_RETURN(GF2X, x); }

// irred poly


void IrredPolyTower(GF2X& h, const GF2EX& g, const GF2EXModulus& F,
                      long m);

inline GF2X IrredPolyTower(const GF2EX& g, const GF2EXModulus& F,
                      long m)
   { GF2X x; IrredPolyTower(x, g, F, m); NTL_OPT_RETURN(GF2X, x); }

inline void IrredPolyTower(GF2X& h, const GF2EX& g, 
                             const GF2EXModulus& F)
   { IrredPolyTower(h, g, F, deg(F)*GF2E::degree()); }

inline GF2X IrredPolyTower(const GF2EX& g, const GF2EXModulus& F)
   { GF2X x; IrredPolyTower(x, g, F); NTL_OPT_RETURN(GF2X, x); }



/*****************************************************************

                   Traces, norms, resultants

******************************************************************/

void TraceVec(vec_GF2E& S, const GF2EX& f);

inline vec_GF2E TraceVec(const GF2EX& f)
   { vec_GF2E x; TraceVec(x, f); NTL_OPT_RETURN(vec_GF2E, x); }


void TraceMod(GF2E& x, const GF2EX& a, const GF2EXModulus& F);

inline GF2E TraceMod(const GF2EX& a, const GF2EXModulus& F)
   { GF2E x; TraceMod(x, a, F); NTL_OPT_RETURN(GF2E, x); }

void TraceMod(GF2E& x, const GF2EX& a, const GF2EX& f);

inline GF2E TraceMod(const GF2EX& a, const GF2EX& f)
   { GF2E x; TraceMod(x, a, f); NTL_OPT_RETURN(GF2E, x); }





void NormMod(GF2E& x, const GF2EX& a, const GF2EX& f);

inline GF2E NormMod(const GF2EX& a, const GF2EX& f)
   { GF2E x; NormMod(x, a, f); NTL_OPT_RETURN(GF2E, x); }

void resultant(GF2E& rres, const GF2EX& a, const GF2EX& b);

inline GF2E resultant(const GF2EX& a, const GF2EX& b)
   { GF2E x; resultant(x, a, b); NTL_OPT_RETURN(GF2E, x); }


NTL_CLOSE_NNS 

#endif
