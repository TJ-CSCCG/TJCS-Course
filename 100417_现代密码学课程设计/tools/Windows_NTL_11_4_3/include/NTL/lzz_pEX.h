
#ifndef NTL_zz_pEX__H
#define NTL_zz_pEX__H

#include <NTL/vec_lzz_pE.h>
#include <NTL/Lazy.h>

NTL_OPEN_NNS

class zz_pEXModulus; // forward declaration

class zz_pEX {
public:
typedef zz_pE coeff_type;
typedef zz_pEXModulus modulus_type;


vec_zz_pE rep;


/***************************************************************

          Constructors, Destructors, and Assignment

****************************************************************/


zz_pEX() { }
//  initial value 0

// default copy constructor and assignment
// default destructor

explicit zz_pEX(long a) { *this = a; }
explicit zz_pEX(const zz_p& a) { *this = a; }
explicit zz_pEX(const zz_pE& a) { *this = a; }

zz_pEX(INIT_SIZE_TYPE, long n) { rep.SetMaxLength(n); }


void normalize();
// strip leading zeros

void SetMaxLength(long n) 
// pre-allocate space for n coefficients.
// Value is unchanged

   { rep.SetMaxLength(n); }


void kill() 
// free space held by this polynomial.  Value becomes 0.

   { rep.kill(); }

void swap(zz_pEX& x) { rep.swap(x.rep); }



void SetLength(long n) { rep.SetLength(n); }
zz_pE& operator[](long i) { return rep[i]; }
const zz_pE& operator[](long i) const { return rep[i]; }




static const zz_pEX& zero();

inline zz_pEX(long i, const zz_pE& c);
inline zz_pEX(long i, const zz_p& c);
inline zz_pEX(long i, long c);

inline zz_pEX(INIT_MONO_TYPE, long i, const zz_pE& c);
inline zz_pEX(INIT_MONO_TYPE, long i, const zz_p& c);
inline zz_pEX(INIT_MONO_TYPE, long i, long c);
inline zz_pEX(INIT_MONO_TYPE, long i);


inline zz_pEX& operator=(long a);
inline zz_pEX& operator=(const zz_p& a);
inline zz_pEX& operator=(const zz_pE& a);

zz_pEX(zz_pEX& x, INIT_TRANS_TYPE) : rep(x.rep, INIT_TRANS) { }


};


NTL_DECLARE_RELOCATABLE((zz_pEX*))


NTL_SNS istream& operator>>(NTL_SNS istream& s, zz_pEX& x);
NTL_SNS ostream& operator<<(NTL_SNS ostream& s, const zz_pEX& a);




/**********************************************************

                   Some utility routines

***********************************************************/


inline long deg(const zz_pEX& a) { return a.rep.length() - 1; }
// degree of a polynomial.
// note that the zero polynomial has degree -1.

const zz_pE& coeff(const zz_pEX& a, long i);
// zero if i not in range

const zz_pE& LeadCoeff(const zz_pEX& a);
// zero if a == 0

const zz_pE& ConstTerm(const zz_pEX& a);
// zero if a == 0

void SetCoeff(zz_pEX& x, long i, const zz_pE& a);
void SetCoeff(zz_pEX& x, long i, const zz_p& a);
void SetCoeff(zz_pEX& x, long i, long a);
// x[i] = a, error is raised if i < 0

void SetCoeff(zz_pEX& x, long i);
// x[i] = 1, error is raised if i < 0

inline zz_pEX::zz_pEX(long i, const zz_pE& a) { SetCoeff(*this, i, a); }
inline zz_pEX::zz_pEX(long i, const zz_p& a) { SetCoeff(*this, i, a); }
inline zz_pEX::zz_pEX(long i, long a) { SetCoeff(*this, i, a); }

inline zz_pEX::zz_pEX(INIT_MONO_TYPE, long i, const zz_pE& a) { SetCoeff(*this, i, a); }
inline zz_pEX::zz_pEX(INIT_MONO_TYPE, long i, const zz_p& a) { SetCoeff(*this, i, a); }
inline zz_pEX::zz_pEX(INIT_MONO_TYPE, long i, long a) { SetCoeff(*this, i, a); }
inline zz_pEX::zz_pEX(INIT_MONO_TYPE, long i) { SetCoeff(*this, i); }

void SetX(zz_pEX& x);
// x is set to the monomial X

long IsX(const zz_pEX& a);
// test if x = X

inline void clear(zz_pEX& x) 
// x = 0

   { x.rep.SetLength(0); }

inline void set(zz_pEX& x)
// x = 1

   { x.rep.SetLength(1); set(x.rep[0]); }

inline void swap(zz_pEX& x, zz_pEX& y)
// swap x & y (only pointers are swapped)

   { x.swap(y); }

void random(zz_pEX& x, long n);
inline zz_pEX random_zz_pEX(long n)
   { zz_pEX x; random(x, n); NTL_OPT_RETURN(zz_pEX, x); }
// generate a random polynomial of degree < n 

void trunc(zz_pEX& x, const zz_pEX& a, long m);
inline zz_pEX trunc(const zz_pEX& a, long m)
   { zz_pEX x; trunc(x, a, m);  NTL_OPT_RETURN(zz_pEX, x); }
// x = a % X^m

void RightShift(zz_pEX& x, const zz_pEX& a, long n);
inline zz_pEX RightShift(const zz_pEX& a, long n)
   { zz_pEX x; RightShift(x, a, n);  NTL_OPT_RETURN(zz_pEX, x); }
// x = a/X^n

void LeftShift(zz_pEX& x, const zz_pEX& a, long n);
inline zz_pEX LeftShift(const zz_pEX& a, long n)
   { zz_pEX x; LeftShift(x, a, n);  NTL_OPT_RETURN(zz_pEX, x); }
// x = a*X^n

#ifndef NTL_TRANSITION

inline zz_pEX operator>>(const zz_pEX& a, long n)
   { zz_pEX x; RightShift(x, a, n); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator<<(const zz_pEX& a, long n)
   { zz_pEX x; LeftShift(x, a, n); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX& operator<<=(zz_pEX& x, long n)
   { LeftShift(x, x, n); return x; }

inline zz_pEX& operator>>=(zz_pEX& x, long n)
   { RightShift(x, x, n); return x; }

#endif



void diff(zz_pEX& x, const zz_pEX& a);
inline zz_pEX diff(const zz_pEX& a)
   { zz_pEX x; diff(x, a);  NTL_OPT_RETURN(zz_pEX, x); }
// x = derivative of a



void MakeMonic(zz_pEX& x);

void reverse(zz_pEX& c, const zz_pEX& a, long hi);

inline zz_pEX reverse(const zz_pEX& a, long hi)
   { zz_pEX x; reverse(x, a, hi); NTL_OPT_RETURN(zz_pEX, x); }

inline void reverse(zz_pEX& c, const zz_pEX& a)
{  reverse(c, a, deg(a)); }

inline zz_pEX reverse(const zz_pEX& a)
   { zz_pEX x; reverse(x, a); NTL_OPT_RETURN(zz_pEX, x); }

inline void VectorCopy(vec_zz_pE& x, const zz_pEX& a, long n)
   { VectorCopy(x, a.rep, n); }

inline vec_zz_pE VectorCopy(const zz_pEX& a, long n)
   { return VectorCopy(a.rep, n); }






/*******************************************************************

                        conversion routines

********************************************************************/



void conv(zz_pEX& x, long a);

void conv(zz_pEX& x, const ZZ& a);

void conv(zz_pEX& x, const zz_p& a);
void conv(zz_pEX& x, const zz_pX& a);
void conv(zz_pEX& x, const zz_pE& a);


void conv(zz_pEX& x, const vec_zz_pE& a);

inline zz_pEX to_zz_pEX(long a)
   { zz_pEX x; conv(x, a); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX to_zz_pEX(const ZZ& a)
   { zz_pEX x; conv(x, a); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX to_zz_pEX(const zz_p& a)
   { zz_pEX x; conv(x, a); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX to_zz_pEX(const zz_pX& a)
   { zz_pEX x; conv(x, a); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX to_zz_pEX(const zz_pE& a)
   { zz_pEX x; conv(x, a); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX to_zz_pEX(const vec_zz_pE& a)
   { zz_pEX x; conv(x, a); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX& zz_pEX::operator=(long a)
   { conv(*this, a); return *this; }

inline zz_pEX& zz_pEX::operator=(const zz_p& a)
   { conv(*this, a); return *this; }

inline zz_pEX& zz_pEX::operator=(const zz_pE& a)
   { conv(*this, a); return *this; }




/* additional legacy conversions for v6 conversion regime */

inline void conv(zz_pEX& x, const zz_pEX& a)
   { x = a; }

inline void conv(vec_zz_pE& x, const zz_pEX& a)
   { x = a.rep; }

class ZZX;
void conv(zz_pEX& x, const ZZX& a);


/* ------------------------------------- */



/*************************************************************

                        Comparison

**************************************************************/

long IsZero(const zz_pEX& a); 

long IsOne(const zz_pEX& a);

inline long operator==(const zz_pEX& a, const zz_pEX& b)
{ return a.rep == b.rep; }

long operator==(const zz_pEX& a, long b);
long operator==(const zz_pEX& a, const zz_p& b);
long operator==(const zz_pEX& a, const zz_pE& b);

inline long operator==(long a, const zz_pEX& b)
   { return (b == a); }
inline long operator==(const zz_p& a, const zz_pEX& b)
   { return (b == a); }
inline long operator==(const zz_pE& a, const zz_pEX& b)
   { return (b == a); }

inline long operator!=(const zz_pEX& a, const zz_pEX& b)
   { return !(a == b); }
inline long operator!=(const zz_pEX& a, long b)
   { return !(a == b); }
inline long operator!=(const zz_pEX& a, const zz_p& b)
   { return !(a == b); }
inline long operator!=(const zz_pEX& a, const zz_pE& b)
   { return !(a == b); }
inline long operator!=(const long a, const zz_pEX& b)
   { return !(a == b); }
inline long operator!=(const zz_p& a, const zz_pEX& b)
   { return !(a == b); }
inline long operator!=(const zz_pE& a, const zz_pEX& b)
   { return !(a == b); }


/***************************************************************

                         Addition

****************************************************************/

void add(zz_pEX& x, const zz_pEX& a, const zz_pEX& b);

void sub(zz_pEX& x, const zz_pEX& a, const zz_pEX& b);

void negate(zz_pEX& x, const zz_pEX& a);

// scalar versions

void add(zz_pEX & x, const zz_pEX& a, long b); 
void add(zz_pEX & x, const zz_pEX& a, const zz_p& b); 
void add(zz_pEX & x, const zz_pEX& a, const zz_pE& b); 

inline void add(zz_pEX& x, const zz_pE& a, const zz_pEX& b)
   { add(x, b, a); }
inline void add(zz_pEX& x, const zz_p& a, const zz_pEX& b)
   { add(x, b, a); }
inline void add(zz_pEX& x, long a, const zz_pEX& b)
   { add(x, b, a); }

void sub(zz_pEX & x, const zz_pEX& a, long b); 
void sub(zz_pEX & x, const zz_pEX& a, const zz_p& b); 
void sub(zz_pEX & x, const zz_pEX& a, const zz_pE& b); 

void sub(zz_pEX& x, const zz_pE& a, const zz_pEX& b);
void sub(zz_pEX& x, const zz_p& a, const zz_pEX& b);
void sub(zz_pEX& x, long a, const zz_pEX& b);



inline zz_pEX operator+(const zz_pEX& a, const zz_pEX& b)
   { zz_pEX x; add(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator+(const zz_pEX& a, const zz_pE& b)
   { zz_pEX x; add(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator+(const zz_pEX& a, const zz_p& b)
   { zz_pEX x; add(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator+(const zz_pEX& a, long b)
   { zz_pEX x; add(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator+(const zz_pE& a, const zz_pEX& b)
   { zz_pEX x; add(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator+(const zz_p& a, const zz_pEX& b)
   { zz_pEX x; add(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator+(long a, const zz_pEX& b)
   { zz_pEX x; add(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }


inline zz_pEX operator-(const zz_pEX& a, const zz_pEX& b)
   { zz_pEX x; sub(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator-(const zz_pEX& a, const zz_pE& b)
   { zz_pEX x; sub(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator-(const zz_pEX& a, const zz_p& b)
   { zz_pEX x; sub(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator-(const zz_pEX& a, long b)
   { zz_pEX x; sub(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator-(const zz_pE& a, const zz_pEX& b)
   { zz_pEX x; sub(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator-(const zz_p& a, const zz_pEX& b)
   { zz_pEX x; sub(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator-(long a, const zz_pEX& b)
   { zz_pEX x; sub(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }


inline zz_pEX& operator+=(zz_pEX& x, const zz_pEX& b)
   { add(x, x, b); return x; }

inline zz_pEX& operator+=(zz_pEX& x, const zz_pE& b)
   { add(x, x, b); return x; }

inline zz_pEX& operator+=(zz_pEX& x, const zz_p& b)
   { add(x, x, b); return x; }

inline zz_pEX& operator+=(zz_pEX& x, long b)
   { add(x, x, b); return x; }

inline zz_pEX& operator-=(zz_pEX& x, const zz_pEX& b)
   { sub(x, x, b); return x; }

inline zz_pEX& operator-=(zz_pEX& x, const zz_pE& b)
   { sub(x, x, b); return x; }

inline zz_pEX& operator-=(zz_pEX& x, const zz_p& b)
   { sub(x, x, b); return x; }

inline zz_pEX& operator-=(zz_pEX& x, long b)
   { sub(x, x, b); return x; }


inline zz_pEX operator-(const zz_pEX& a) 
   { zz_pEX x; negate(x, a); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX& operator++(zz_pEX& x) { add(x, x, 1); return x; }
inline void operator++(zz_pEX& x, int) { add(x, x, 1); }
inline zz_pEX& operator--(zz_pEX& x) { sub(x, x, 1); return x; }
inline void operator--(zz_pEX& x, int) { sub(x, x, 1); }



/*****************************************************************

                        Multiplication

******************************************************************/


void mul(zz_pEX& x, const zz_pEX& a, const zz_pEX& b);
// x = a * b

void sqr(zz_pEX& x, const zz_pEX& a);
inline zz_pEX sqr(const zz_pEX& a) 
   { zz_pEX x; sqr(x, a); NTL_OPT_RETURN(zz_pEX, x); }
// x = a^2


void mul(zz_pEX & x, const zz_pEX& a, long b); 
void mul(zz_pEX & x, const zz_pEX& a, const zz_p& b); 
void mul(zz_pEX & x, const zz_pEX& a, const zz_pE& b); 

inline void mul(zz_pEX& x, long a, const zz_pEX& b)
   { mul(x, b, a); }
inline void mul(zz_pEX& x, const zz_p& a, const zz_pEX& b)
   { mul(x, b, a); }
inline void mul(zz_pEX& x, const zz_pE& a, const zz_pEX& b)
   { mul(x, b, a); }

void MulTrunc(zz_pEX& x, const zz_pEX& a, const zz_pEX& b, long n);
inline zz_pEX MulTrunc(const zz_pEX& a, const zz_pEX& b, long n)
   { zz_pEX x; MulTrunc(x, a, b, n); NTL_OPT_RETURN(zz_pEX, x); }
// x = a * b % X^n

void SqrTrunc(zz_pEX& x, const zz_pEX& a, long n);
inline zz_pEX SqrTrunc(const zz_pEX& a, long n)
   { zz_pEX x; SqrTrunc(x, a, n); NTL_OPT_RETURN(zz_pEX, x); }
// x = a*a % X^n


inline zz_pEX operator*(const zz_pEX& a, const zz_pEX& b)
   { zz_pEX x; mul(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator*(const zz_pEX& a, const zz_pE& b)
   { zz_pEX x; mul(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator*(const zz_pEX& a, const zz_p& b)
   { zz_pEX x; mul(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator*(const zz_pEX& a, long b)
   { zz_pEX x; mul(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator*(const zz_pE& a, const zz_pEX& b)
   { zz_pEX x; mul(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator*(const zz_p& a, const zz_pEX& b)
   { zz_pEX x; mul(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator*(long a, const zz_pEX& b)
   { zz_pEX x; mul(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX& operator*=(zz_pEX& x, const zz_pEX& b)
   { mul(x, x, b); return x; }

inline zz_pEX& operator*=(zz_pEX& x, const zz_pE& b)
   { mul(x, x, b); return x; }

inline zz_pEX& operator*=(zz_pEX& x, const zz_p& b)
   { mul(x, x, b); return x; }

inline zz_pEX& operator*=(zz_pEX& x, long b)
   { mul(x, x, b); return x; }


void power(zz_pEX& x, const zz_pEX& a, long e);
inline zz_pEX power(const zz_pEX& a, long e)
   { zz_pEX x; power(x, a, e); NTL_OPT_RETURN(zz_pEX, x); }





/*************************************************************

                      Division

**************************************************************/

void DivRem(zz_pEX& q, zz_pEX& r, const zz_pEX& a, const zz_pEX& b);
// q = a/b, r = a%b

void div(zz_pEX& q, const zz_pEX& a, const zz_pEX& b);
void div(zz_pEX& q, const zz_pEX& a, const zz_pE& b);
void div(zz_pEX& q, const zz_pEX& a, const zz_p& b);
void div(zz_pEX& q, const zz_pEX& a, long b);
// q = a/b

void rem(zz_pEX& r, const zz_pEX& a, const zz_pEX& b);
// r = a%b

long divide(zz_pEX& q, const zz_pEX& a, const zz_pEX& b);
// if b | a, sets q = a/b and returns 1; otherwise returns 0

long divide(const zz_pEX& a, const zz_pEX& b);
// if b | a, sets q = a/b and returns 1; otherwise returns 0

void InvTrunc(zz_pEX& x, const zz_pEX& a, long m);
inline zz_pEX InvTrunc(const zz_pEX& a, long m)
   { zz_pEX x; InvTrunc(x, a, m); NTL_OPT_RETURN(zz_pEX, x); }
// computes x = a^{-1} % X^m 
// constant term must be invertible


inline zz_pEX operator/(const zz_pEX& a, const zz_pEX& b)
   { zz_pEX x; div(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator/(const zz_pEX& a, const zz_pE& b)
   { zz_pEX x; div(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator/(const zz_pEX& a, const zz_p& b)
   { zz_pEX x; div(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX operator/(const zz_pEX& a, long b)
   { zz_pEX x; div(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX& operator/=(zz_pEX& x, const zz_pEX& b)
   { div(x, x, b); return x; }

inline zz_pEX& operator/=(zz_pEX& x, const zz_pE& b)
   { div(x, x, b); return x; }

inline zz_pEX& operator/=(zz_pEX& x, const zz_p& b)
   { div(x, x, b); return x; }

inline zz_pEX& operator/=(zz_pEX& x, long b)
   { div(x, x, b); return x; }


inline zz_pEX operator%(const zz_pEX& a, const zz_pEX& b)
   { zz_pEX x; rem(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX& operator%=(zz_pEX& x, const zz_pEX& b)
   { rem(x, x, b); return x; }



/***********************************************************

                         GCD's

************************************************************/


void GCD(zz_pEX& x, const zz_pEX& a, const zz_pEX& b);
inline zz_pEX GCD(const zz_pEX& a, const zz_pEX& b)
   { zz_pEX x; GCD(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }
// x = GCD(a, b),  x is always monic (or zero if a==b==0).

void XGCD(zz_pEX& d, zz_pEX& s, zz_pEX& t, const zz_pEX& a, const zz_pEX& b);
// d = gcd(a,b), a s + b t = d 


/*************************************************************

             Modular Arithmetic without pre-conditioning

**************************************************************/

// arithmetic mod f.
// all inputs and outputs are polynomials of degree less than deg(f).
// ASSUMPTION: f is assumed monic, and deg(f) > 0.
// NOTE: if you want to do many computations with a fixed f,
//       use the zz_pEXModulus data structure and associated routines below.



void MulMod(zz_pEX& x, const zz_pEX& a, const zz_pEX& b, const zz_pEX& f);
inline zz_pEX MulMod(const zz_pEX& a, const zz_pEX& b, const zz_pEX& f)
   { zz_pEX x; MulMod(x, a, b, f); NTL_OPT_RETURN(zz_pEX, x); }
// x = (a * b) % f

void SqrMod(zz_pEX& x, const zz_pEX& a, const zz_pEX& f);
inline zz_pEX SqrMod(const zz_pEX& a, const zz_pEX& f)
   { zz_pEX x; SqrMod(x, a, f); NTL_OPT_RETURN(zz_pEX, x); }
// x = a^2 % f

void MulByXMod(zz_pEX& x, const zz_pEX& a, const zz_pEX& f);
inline zz_pEX MulByXMod(const zz_pEX& a, const zz_pEX& f)
   { zz_pEX x; MulByXMod(x, a, f); NTL_OPT_RETURN(zz_pEX, x); }
// x = (a * X) mod f

void InvMod(zz_pEX& x, const zz_pEX& a, const zz_pEX& f);
inline zz_pEX InvMod(const zz_pEX& a, const zz_pEX& f)
   { zz_pEX x; InvMod(x, a, f); NTL_OPT_RETURN(zz_pEX, x); }
// x = a^{-1} % f, error is a is not invertible

long InvModStatus(zz_pEX& x, const zz_pEX& a, const zz_pEX& f);
// if (a, f) = 1, returns 0 and sets x = a^{-1} % f
// otherwise, returns 1 and sets x = (a, f)





/******************************************************************

        Modular Arithmetic with Pre-conditioning

*******************************************************************/


// If you need to do a lot of arithmetic modulo a fixed f,
// build zz_pEXModulus F for f.  This pre-computes information about f
// that speeds up the computation a great deal.

class zz_pEXModulus {
public:
   zz_pEXModulus();
   ~zz_pEXModulus();

   zz_pEXModulus(const zz_pEX& ff);

   zz_pEX f;   // the modulus

   operator const zz_pEX& () const { return f; }
   const zz_pEX& val() const { return f; }

   long n; //  deg(f)

   long method;

   zz_pEX h0;
   zz_pE hlc;
   zz_pEX f0;

   OptionalVal< Lazy<vec_zz_pE> > tracevec; 
   // extra level of indirection to ensure relocatability

}; 


NTL_DECLARE_RELOCATABLE((zz_pEXModulus*))



inline long deg(const zz_pEXModulus& F) { return F.n; }


void build(zz_pEXModulus& F, const zz_pEX& f);

void rem(zz_pEX& r, const zz_pEX& a, const zz_pEXModulus& F);
   
void DivRem(zz_pEX& q, zz_pEX& r, const zz_pEX& a, const zz_pEXModulus& F);

void div(zz_pEX& q, const zz_pEX& a, const zz_pEXModulus& F);

void MulMod(zz_pEX& c, const zz_pEX& a, const zz_pEX& b, 
            const zz_pEXModulus& F);
inline zz_pEX MulMod(const zz_pEX& a, const zz_pEX& b, 
            const zz_pEXModulus& F)
   { zz_pEX x; MulMod(x, a, b, F); NTL_OPT_RETURN(zz_pEX, x); }

void SqrMod(zz_pEX& c, const zz_pEX& a, const zz_pEXModulus& F);
inline zz_pEX SqrMod(const zz_pEX& a, const zz_pEXModulus& F)
   { zz_pEX x; SqrMod(x, a, F); NTL_OPT_RETURN(zz_pEX, x); }


void PowerMod(zz_pEX& h, const zz_pEX& g, const ZZ& e, const zz_pEXModulus& F);

inline void PowerMod(zz_pEX& h, const zz_pEX& g, long e, 
                     const zz_pEXModulus& F)
   { PowerMod(h, g, ZZ_expo(e), F); }

inline zz_pEX PowerMod(const zz_pEX& g, const ZZ& e, 
                             const zz_pEXModulus& F)
   { zz_pEX x; PowerMod(x, g, e, F);  NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX PowerMod(const zz_pEX& g, long e, const zz_pEXModulus& F)
   { zz_pEX x; PowerMod(x, g, e, F);  NTL_OPT_RETURN(zz_pEX, x); }

void PowerXMod(zz_pEX& hh, const ZZ& e, const zz_pEXModulus& F);

inline void PowerXMod(zz_pEX& h, long e, const zz_pEXModulus& F)
   { PowerXMod(h, ZZ_expo(e), F); }


inline zz_pEX PowerXMod(const ZZ& e, const zz_pEXModulus& F)
   { zz_pEX x; PowerXMod(x, e, F);  NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX PowerXMod(long e, const zz_pEXModulus& F)
   { zz_pEX x; PowerXMod(x, e, F);  NTL_OPT_RETURN(zz_pEX, x); }


inline zz_pEX operator%(const zz_pEX& a, const zz_pEXModulus& F)
   { zz_pEX x; rem(x, a, F); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX& operator%=(zz_pEX& x, const zz_pEXModulus& F)
   { rem(x, x, F); return x; }

inline zz_pEX operator/(const zz_pEX& a, const zz_pEXModulus& F)
   { zz_pEX x; div(x, a, F); NTL_OPT_RETURN(zz_pEX, x); }

inline zz_pEX& operator/=(zz_pEX& x, const zz_pEXModulus& F)
   { div(x, x, F); return x; }



/*****************************************************************

                       vectors of zz_pEX's

*****************************************************************/



typedef Vec<zz_pEX> vec_zz_pEX;



/*******************************************************

              Evaluation and related problems

********************************************************/




void BuildFromRoots(zz_pEX& x, const vec_zz_pE& a);
inline zz_pEX BuildFromRoots(const vec_zz_pE& a)
   { zz_pEX x; BuildFromRoots(x, a); NTL_OPT_RETURN(zz_pEX, x); }
// computes the polynomial (X-a[0]) ... (X-a[n-1]), where n = a.length()


void eval(zz_pE& b, const zz_pEX& f, const zz_pE& a);
inline zz_pE eval(const zz_pEX& f, const zz_pE& a)
   { zz_pE x; eval(x, f, a); NTL_OPT_RETURN(zz_pE, x); }
// b = f(a)

void eval(vec_zz_pE& b, const zz_pEX& f, const vec_zz_pE& a);
inline vec_zz_pE eval(const zz_pEX& f, const vec_zz_pE& a)
   { vec_zz_pE x; eval(x, f, a); NTL_OPT_RETURN(vec_zz_pE, x); }
//  b[i] = f(a[i])

inline void eval(zz_pE& b, const zz_pX& f, const zz_pE& a)
   { conv(b, CompMod(f, rep(a), zz_pE::modulus())); }
   
inline zz_pE eval(const zz_pX& f, const zz_pE& a)
   { zz_pE x; eval(x, f, a); NTL_OPT_RETURN(zz_pE, x); }
// b = f(a)


void interpolate(zz_pEX& f, const vec_zz_pE& a, const vec_zz_pE& b);
inline zz_pEX interpolate(const vec_zz_pE& a, const vec_zz_pE& b)
   { zz_pEX x; interpolate(x, a, b); NTL_OPT_RETURN(zz_pEX, x); }
// computes f such that f(a[i]) = b[i]





/**********************************************************

         Modular Composition and Minimal Polynomials

***********************************************************/



void CompMod(zz_pEX& x, const zz_pEX& g, const zz_pEX& h, const zz_pEXModulus& F);
inline zz_pEX 
CompMod(const zz_pEX& g, const zz_pEX& h, const zz_pEXModulus& F)
   { zz_pEX x; CompMod(x, g, h, F); NTL_OPT_RETURN(zz_pEX, x); }
// x = g(h) mod f

void Comp2Mod(zz_pEX& x1, zz_pEX& x2, const zz_pEX& g1, const zz_pEX& g2,
              const zz_pEX& h, const zz_pEXModulus& F);
// xi = gi(h) mod f (i=1,2)

void Comp3Mod(zz_pEX& x1, zz_pEX& x2, zz_pEX& x3, 
              const zz_pEX& g1, const zz_pEX& g2, const zz_pEX& g3,
              const zz_pEX& h, const zz_pEXModulus& F);
// xi = gi(h) mod f (i=1..3)



// The routine build (see below) which is implicitly called
// by the various compose and UpdateMap routines builds a table
// of polynomials.  
// If zz_pEXArgBound > 0, then the table is limited in
// size to approximamtely that many KB.
// If zz_pEXArgBound <= 0, then it is ignored, and space is allocated
// so as to maximize speed.
// Initially, zz_pEXArgBound = 0.


// If a single h is going to be used with many g's
// then you should build a zz_pEXArgument for h,
// and then use the compose routine below.
// build computes and stores h, h^2, ..., h^m mod f.
// After this pre-computation, composing a polynomial of degree 
// roughly n with h takes n/m multiplies mod f, plus n^2
// scalar multiplies.
// Thus, increasing m increases the space requirement and the pre-computation
// time, but reduces the composition time.
// If zz_pEXArgBound > 0, a table of size less than m may be built.

struct zz_pEXArgument {
   vec_zz_pEX H;
};

extern 
NTL_CHEAP_THREAD_LOCAL 
long zz_pEXArgBound;


void build(zz_pEXArgument& H, const zz_pEX& h, const zz_pEXModulus& F, long m);

// m must be > 0, otherwise an error is raised

void CompMod(zz_pEX& x, const zz_pEX& g, const zz_pEXArgument& H, 
             const zz_pEXModulus& F);

inline zz_pEX 
CompMod(const zz_pEX& g, const zz_pEXArgument& H, const zz_pEXModulus& F)
   { zz_pEX x; CompMod(x, g, H, F); NTL_OPT_RETURN(zz_pEX, x); }
   



void MinPolySeq(zz_pEX& h, const vec_zz_pE& a, long m);
inline zz_pEX MinPolySeq(const vec_zz_pE& a, long m)
   { zz_pEX x; MinPolySeq(x, a, m); NTL_OPT_RETURN(zz_pEX, x); }


void MinPolyMod(zz_pEX& hh, const zz_pEX& g, const zz_pEXModulus& F);
inline zz_pEX MinPolyMod(const zz_pEX& g, const zz_pEXModulus& F)
   { zz_pEX x; MinPolyMod(x, g, F); NTL_OPT_RETURN(zz_pEX, x); }


void MinPolyMod(zz_pEX& hh, const zz_pEX& g, const zz_pEXModulus& F, long m);
inline zz_pEX MinPolyMod(const zz_pEX& g, const zz_pEXModulus& F, long m)
   { zz_pEX x; MinPolyMod(x, g, F, m); NTL_OPT_RETURN(zz_pEX, x); }

void ProbMinPolyMod(zz_pEX& hh, const zz_pEX& g, const zz_pEXModulus& F);
inline zz_pEX ProbMinPolyMod(const zz_pEX& g, const zz_pEXModulus& F)
   { zz_pEX x; ProbMinPolyMod(x, g, F); NTL_OPT_RETURN(zz_pEX, x); }

void ProbMinPolyMod(zz_pEX& hh, const zz_pEX& g, const zz_pEXModulus& F, long m);
inline zz_pEX ProbMinPolyMod(const zz_pEX& g, const zz_pEXModulus& F, long m)
   { zz_pEX x; ProbMinPolyMod(x, g, F, m); NTL_OPT_RETURN(zz_pEX, x); }

void IrredPolyMod(zz_pEX& h, const zz_pEX& g, const zz_pEXModulus& F);
inline zz_pEX IrredPolyMod(const zz_pEX& g, const zz_pEXModulus& F)
   { zz_pEX x; IrredPolyMod(x, g, F); NTL_OPT_RETURN(zz_pEX, x); }

void IrredPolyMod(zz_pEX& h, const zz_pEX& g, const zz_pEXModulus& F, long m);
inline zz_pEX IrredPolyMod(const zz_pEX& g, const zz_pEXModulus& F, long m)
   { zz_pEX x; IrredPolyMod(x, g, F, m); NTL_OPT_RETURN(zz_pEX, x); }


struct zz_pEXTransMultiplier {
   zz_pEX f0, fbi, b;
   long shamt, shamt_fbi, shamt_b;
};

void build(zz_pEXTransMultiplier& B, const zz_pEX& b, const zz_pEXModulus& F);

void TransMulMod(zz_pEX& x, const zz_pEX& a, const zz_pEXTransMultiplier& B,
               const zz_pEXModulus& F);

void UpdateMap(vec_zz_pE& x, const vec_zz_pE& a, 
         const zz_pEXTransMultiplier& B, const zz_pEXModulus& F);

inline vec_zz_pE UpdateMap(const vec_zz_pE& a,
         const zz_pEXTransMultiplier& B, const zz_pEXModulus& F)
   { vec_zz_pE x; UpdateMap(x, a, B, F); NTL_OPT_RETURN(vec_zz_pE, x); }

void ProjectPowers(vec_zz_pE& x, const vec_zz_pE& a, long k, 
                   const zz_pEXArgument& H, const zz_pEXModulus& F);
inline vec_zz_pE ProjectPowers(const vec_zz_pE& a, long k, 
                   const zz_pEXArgument& H, const zz_pEXModulus& F)
   { vec_zz_pE x; ProjectPowers(x, a, k, H, F); NTL_OPT_RETURN(vec_zz_pE, x); }

void ProjectPowers(vec_zz_pE& x, const vec_zz_pE& a, long k, const zz_pEX& h, 
                   const zz_pEXModulus& F);
inline vec_zz_pE ProjectPowers(const vec_zz_pE& a, long k, 
                   const zz_pEX& H, const zz_pEXModulus& F)
   { vec_zz_pE x; ProjectPowers(x, a, k, H, F); NTL_OPT_RETURN(vec_zz_pE, x); }

inline void project(zz_pE& x, const vec_zz_pE& a, const zz_pEX& b)
   { InnerProduct(x, a, b.rep); }

inline zz_pE project(const vec_zz_pE& a, const zz_pEX& b)
   { zz_pE x; InnerProduct(x, a, b.rep); NTL_OPT_RETURN(zz_pE, x); }



/*****************************************************************

          modular composition and minimal polynonomials
                         in towers

******************************************************************/


// composition

void CompTower(zz_pEX& x, const zz_pX& g, const zz_pEXArgument& A,
             const zz_pEXModulus& F);

inline zz_pEX CompTower(const zz_pX& g, const zz_pEXArgument& A,
             const zz_pEXModulus& F)
   { zz_pEX x; CompTower(x, g, A, F); NTL_OPT_RETURN(zz_pEX, x); }

void CompTower(zz_pEX& x, const zz_pX& g, const zz_pEX& h,
             const zz_pEXModulus& F);

inline zz_pEX CompTower(const zz_pX& g, const zz_pEX& h,
             const zz_pEXModulus& F)
   { zz_pEX x; CompTower(x, g, h, F); NTL_OPT_RETURN(zz_pEX, x); }

// prob min poly

void ProbMinPolyTower(zz_pX& h, const zz_pEX& g, const zz_pEXModulus& F,
                      long m);

inline zz_pX ProbMinPolyTower(const zz_pEX& g, const zz_pEXModulus& F,
                      long m)
   { zz_pX x; ProbMinPolyTower(x, g, F, m); NTL_OPT_RETURN(zz_pX, x); }

inline void ProbMinPolyTower(zz_pX& h, const zz_pEX& g, 
                             const zz_pEXModulus& F)
   { ProbMinPolyTower(h, g, F, deg(F)*zz_pE::degree()); }

inline zz_pX ProbMinPolyTower(const zz_pEX& g, const zz_pEXModulus& F)
   { zz_pX x; ProbMinPolyTower(x, g, F); NTL_OPT_RETURN(zz_pX, x); }


// min poly


void MinPolyTower(zz_pX& h, const zz_pEX& g, const zz_pEXModulus& F,
                      long m);

inline zz_pX MinPolyTower(const zz_pEX& g, const zz_pEXModulus& F,
                      long m)
   { zz_pX x; MinPolyTower(x, g, F, m); NTL_OPT_RETURN(zz_pX, x); }

inline void MinPolyTower(zz_pX& h, const zz_pEX& g, 
                             const zz_pEXModulus& F)
   { MinPolyTower(h, g, F, deg(F)*zz_pE::degree()); }

inline zz_pX MinPolyTower(const zz_pEX& g, const zz_pEXModulus& F)
   { zz_pX x; MinPolyTower(x, g, F); NTL_OPT_RETURN(zz_pX, x); }

// irred poly


void IrredPolyTower(zz_pX& h, const zz_pEX& g, const zz_pEXModulus& F,
                      long m);

inline zz_pX IrredPolyTower(const zz_pEX& g, const zz_pEXModulus& F,
                      long m)
   { zz_pX x; IrredPolyTower(x, g, F, m); NTL_OPT_RETURN(zz_pX, x); }

inline void IrredPolyTower(zz_pX& h, const zz_pEX& g, 
                             const zz_pEXModulus& F)
   { IrredPolyTower(h, g, F, deg(F)*zz_pE::degree()); }

inline zz_pX IrredPolyTower(const zz_pEX& g, const zz_pEXModulus& F)
   { zz_pX x; IrredPolyTower(x, g, F); NTL_OPT_RETURN(zz_pX, x); }

/*****************************************************************

                   Traces, norms, resultants

******************************************************************/

void TraceVec(vec_zz_pE& S, const zz_pEX& f);

inline vec_zz_pE TraceVec(const zz_pEX& f)
   { vec_zz_pE x; TraceVec(x, f); NTL_OPT_RETURN(vec_zz_pE, x); }


void TraceMod(zz_pE& x, const zz_pEX& a, const zz_pEXModulus& F);

inline zz_pE TraceMod(const zz_pEX& a, const zz_pEXModulus& F)
   { zz_pE x; TraceMod(x, a, F); NTL_OPT_RETURN(zz_pE, x); }

void TraceMod(zz_pE& x, const zz_pEX& a, const zz_pEX& f);

inline zz_pE TraceMod(const zz_pEX& a, const zz_pEX& f)
   { zz_pE x; TraceMod(x, a, f); NTL_OPT_RETURN(zz_pE, x); }





void NormMod(zz_pE& x, const zz_pEX& a, const zz_pEX& f);

inline zz_pE NormMod(const zz_pEX& a, const zz_pEX& f)
   { zz_pE x; NormMod(x, a, f); NTL_OPT_RETURN(zz_pE, x); }

void resultant(zz_pE& rres, const zz_pEX& a, const zz_pEX& b);

inline zz_pE resultant(const zz_pEX& a, const zz_pEX& b)
   { zz_pE x; resultant(x, a, b); NTL_OPT_RETURN(zz_pE, x); }

NTL_CLOSE_NNS

#endif
