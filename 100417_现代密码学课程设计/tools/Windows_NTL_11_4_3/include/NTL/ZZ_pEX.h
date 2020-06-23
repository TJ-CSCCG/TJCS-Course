
#ifndef NTL_ZZ_pEX__H
#define NTL_ZZ_pEX__H

#include <NTL/vec_ZZ_pE.h>
#include <NTL/Lazy.h>

NTL_OPEN_NNS

class ZZ_pEXModulus; // forward declaration

class ZZ_pEX {
public:
typedef ZZ_pE coeff_type;
typedef ZZ_pEXModulus modulus_type;


vec_ZZ_pE rep;


/***************************************************************

          Constructors, Destructors, and Assignment

****************************************************************/


ZZ_pEX() { }
//  initial value 0

explicit ZZ_pEX(long a) { *this = a; }
explicit ZZ_pEX(const ZZ_p& a) { *this = a; }
explicit ZZ_pEX(const ZZ_pE& a) { *this = a; }


ZZ_pEX(INIT_SIZE_TYPE, long n) { rep.SetMaxLength(n); }


// default copy constructor and assignment
// defaut destructor

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
ZZ_pE& operator[](long i) { return rep[i]; }
const ZZ_pE& operator[](long i) const { return rep[i]; }





static const ZZ_pEX& zero();

inline ZZ_pEX(long i, const ZZ_pE& c);
inline ZZ_pEX(long i, const ZZ_p& c);
inline ZZ_pEX(long i, long c);

inline ZZ_pEX(INIT_MONO_TYPE, long i, const ZZ_pE& c);
inline ZZ_pEX(INIT_MONO_TYPE, long i, const ZZ_p& c);
inline ZZ_pEX(INIT_MONO_TYPE, long i, long c);
inline ZZ_pEX(INIT_MONO_TYPE, long i);


inline ZZ_pEX& operator=(long a);
inline ZZ_pEX& operator=(const ZZ_p& a);
inline ZZ_pEX& operator=(const ZZ_pE& a);

ZZ_pEX(ZZ_pEX& x, INIT_TRANS_TYPE) : rep(x.rep, INIT_TRANS) { }


void swap(ZZ_pEX& x) { rep.swap(x.rep); }


};



NTL_DECLARE_RELOCATABLE((ZZ_pEX*))


NTL_SNS istream& operator>>(NTL_SNS istream& s, ZZ_pEX& x);
NTL_SNS ostream& operator<<(NTL_SNS ostream& s, const ZZ_pEX& a);




/**********************************************************

                   Some utility routines

***********************************************************/


inline long deg(const ZZ_pEX& a) { return a.rep.length() - 1; }
// degree of a polynomial.
// note that the zero polynomial has degree -1.

const ZZ_pE& coeff(const ZZ_pEX& a, long i);
// zero if i not in range

const ZZ_pE& LeadCoeff(const ZZ_pEX& a);
// zero if a == 0

const ZZ_pE& ConstTerm(const ZZ_pEX& a);
// zero if a == 0

void SetCoeff(ZZ_pEX& x, long i, const ZZ_pE& a);
void SetCoeff(ZZ_pEX& x, long i, const ZZ_p& a);
void SetCoeff(ZZ_pEX& x, long i, long a);
// x[i] = a, error is raised if i < 0

void SetCoeff(ZZ_pEX& x, long i);
// x[i] = 1, error is raised if i < 0

inline ZZ_pEX::ZZ_pEX(long i, const ZZ_pE& a) { SetCoeff(*this, i, a); }
inline ZZ_pEX::ZZ_pEX(long i, const ZZ_p& a) { SetCoeff(*this, i, a); }
inline ZZ_pEX::ZZ_pEX(long i, long a) { SetCoeff(*this, i, a); }

inline ZZ_pEX::ZZ_pEX(INIT_MONO_TYPE, long i, const ZZ_pE& a) { SetCoeff(*this, i, a); }
inline ZZ_pEX::ZZ_pEX(INIT_MONO_TYPE, long i, const ZZ_p& a) { SetCoeff(*this, i, a); }
inline ZZ_pEX::ZZ_pEX(INIT_MONO_TYPE, long i, long a) { SetCoeff(*this, i, a); }
inline ZZ_pEX::ZZ_pEX(INIT_MONO_TYPE, long i) { SetCoeff(*this, i); }

void SetX(ZZ_pEX& x);
// x is set to the monomial X

long IsX(const ZZ_pEX& a);
// test if x = X

inline void clear(ZZ_pEX& x) 
// x = 0

   { x.rep.SetLength(0); }

inline void set(ZZ_pEX& x)
// x = 1

   { x.rep.SetLength(1); set(x.rep[0]); }

inline void swap(ZZ_pEX& x, ZZ_pEX& y)
// swap x & y (only pointers are swapped)

   { x.swap(y); }

void random(ZZ_pEX& x, long n);
inline ZZ_pEX random_ZZ_pEX(long n)
   { ZZ_pEX x; random(x, n); NTL_OPT_RETURN(ZZ_pEX, x); }
// generate a random polynomial of degree < n 

void trunc(ZZ_pEX& x, const ZZ_pEX& a, long m);
inline ZZ_pEX trunc(const ZZ_pEX& a, long m)
   { ZZ_pEX x; trunc(x, a, m);  NTL_OPT_RETURN(ZZ_pEX, x); }
// x = a % X^m

void RightShift(ZZ_pEX& x, const ZZ_pEX& a, long n);
inline ZZ_pEX RightShift(const ZZ_pEX& a, long n)
   { ZZ_pEX x; RightShift(x, a, n);  NTL_OPT_RETURN(ZZ_pEX, x); }
// x = a/X^n

void LeftShift(ZZ_pEX& x, const ZZ_pEX& a, long n);
inline ZZ_pEX LeftShift(const ZZ_pEX& a, long n)
   { ZZ_pEX x; LeftShift(x, a, n);  NTL_OPT_RETURN(ZZ_pEX, x); }
// x = a*X^n

#ifndef NTL_TRANSITION

inline ZZ_pEX operator>>(const ZZ_pEX& a, long n)
   { ZZ_pEX x; RightShift(x, a, n); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator<<(const ZZ_pEX& a, long n)
   { ZZ_pEX x; LeftShift(x, a, n); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX& operator<<=(ZZ_pEX& x, long n)
   { LeftShift(x, x, n); return x; }

inline ZZ_pEX& operator>>=(ZZ_pEX& x, long n)
   { RightShift(x, x, n); return x; }

#endif



void diff(ZZ_pEX& x, const ZZ_pEX& a);
inline ZZ_pEX diff(const ZZ_pEX& a)
   { ZZ_pEX x; diff(x, a);  NTL_OPT_RETURN(ZZ_pEX, x); }
// x = derivative of a



void MakeMonic(ZZ_pEX& x);

void reverse(ZZ_pEX& c, const ZZ_pEX& a, long hi);

inline ZZ_pEX reverse(const ZZ_pEX& a, long hi)
   { ZZ_pEX x; reverse(x, a, hi); NTL_OPT_RETURN(ZZ_pEX, x); }

inline void reverse(ZZ_pEX& c, const ZZ_pEX& a)
{  reverse(c, a, deg(a)); }

inline ZZ_pEX reverse(const ZZ_pEX& a)
   { ZZ_pEX x; reverse(x, a); NTL_OPT_RETURN(ZZ_pEX, x); }

inline void VectorCopy(vec_ZZ_pE& x, const ZZ_pEX& a, long n)
   { VectorCopy(x, a.rep, n); }

inline vec_ZZ_pE VectorCopy(const ZZ_pEX& a, long n)
   { return VectorCopy(a.rep, n); }






/*******************************************************************

                        conversion routines

********************************************************************/



void conv(ZZ_pEX& x, long a);

void conv(ZZ_pEX& x, const ZZ& a);

void conv(ZZ_pEX& x, const ZZ_p& a);
void conv(ZZ_pEX& x, const ZZ_pX& a);
void conv(ZZ_pEX& x, const ZZ_pE& a);


void conv(ZZ_pEX& x, const vec_ZZ_pE& a);

inline ZZ_pEX to_ZZ_pEX(long a)
   { ZZ_pEX x; conv(x, a); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX to_ZZ_pEX(const ZZ& a)
   { ZZ_pEX x; conv(x, a); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX to_ZZ_pEX(const ZZ_p& a)
   { ZZ_pEX x; conv(x, a); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX to_ZZ_pEX(const ZZ_pX& a)
   { ZZ_pEX x; conv(x, a); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX to_ZZ_pEX(const ZZ_pE& a)
   { ZZ_pEX x; conv(x, a); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX to_ZZ_pEX(const vec_ZZ_pE& a)
   { ZZ_pEX x; conv(x, a); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX& ZZ_pEX::operator=(long a)
   { conv(*this, a); return *this; }

inline ZZ_pEX& ZZ_pEX::operator=(const ZZ_p& a)
   { conv(*this, a); return *this; }

inline ZZ_pEX& ZZ_pEX::operator=(const ZZ_pE& a)
   { conv(*this, a); return *this; }




/* additional legacy conversions for v6 conversion regime */

inline void conv(ZZ_pEX& x, const ZZ_pEX& a)
   { x = a; }

inline void conv(vec_ZZ_pE& x, const ZZ_pEX& a)
   { x = a.rep; }

class ZZX;
void conv(ZZ_pEX& x, const ZZX& a);


/* ------------------------------------- */




/*************************************************************

                        Comparison

**************************************************************/

long IsZero(const ZZ_pEX& a); 

long IsOne(const ZZ_pEX& a);

inline long operator==(const ZZ_pEX& a, const ZZ_pEX& b)
{ return a.rep == b.rep; }

long operator==(const ZZ_pEX& a, long b);
long operator==(const ZZ_pEX& a, const ZZ_p& b);
long operator==(const ZZ_pEX& a, const ZZ_pE& b);

inline long operator==(long a, const ZZ_pEX& b)
   { return (b == a); }
inline long operator==(const ZZ_p& a, const ZZ_pEX& b)
   { return (b == a); }
inline long operator==(const ZZ_pE& a, const ZZ_pEX& b)
   { return (b == a); }

inline long operator!=(const ZZ_pEX& a, const ZZ_pEX& b)
   { return !(a == b); }
inline long operator!=(const ZZ_pEX& a, long b)
   { return !(a == b); }
inline long operator!=(const ZZ_pEX& a, const ZZ_p& b)
   { return !(a == b); }
inline long operator!=(const ZZ_pEX& a, const ZZ_pE& b)
   { return !(a == b); }
inline long operator!=(const long a, const ZZ_pEX& b)
   { return !(a == b); }
inline long operator!=(const ZZ_p& a, const ZZ_pEX& b)
   { return !(a == b); }
inline long operator!=(const ZZ_pE& a, const ZZ_pEX& b)
   { return !(a == b); }


/***************************************************************

                         Addition

****************************************************************/

void add(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& b);

void sub(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& b);

void negate(ZZ_pEX& x, const ZZ_pEX& a);

// scalar versions

void add(ZZ_pEX & x, const ZZ_pEX& a, long b); 
void add(ZZ_pEX & x, const ZZ_pEX& a, const ZZ_p& b); 
void add(ZZ_pEX & x, const ZZ_pEX& a, const ZZ_pE& b); 

inline void add(ZZ_pEX& x, const ZZ_pE& a, const ZZ_pEX& b)
   { add(x, b, a); }
inline void add(ZZ_pEX& x, const ZZ_p& a, const ZZ_pEX& b)
   { add(x, b, a); }
inline void add(ZZ_pEX& x, long a, const ZZ_pEX& b)
   { add(x, b, a); }

void sub(ZZ_pEX & x, const ZZ_pEX& a, long b); 
void sub(ZZ_pEX & x, const ZZ_pEX& a, const ZZ_p& b); 
void sub(ZZ_pEX & x, const ZZ_pEX& a, const ZZ_pE& b); 

void sub(ZZ_pEX& x, const ZZ_pE& a, const ZZ_pEX& b);
void sub(ZZ_pEX& x, const ZZ_p& a, const ZZ_pEX& b);
void sub(ZZ_pEX& x, long a, const ZZ_pEX& b);



inline ZZ_pEX operator+(const ZZ_pEX& a, const ZZ_pEX& b)
   { ZZ_pEX x; add(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator+(const ZZ_pEX& a, const ZZ_pE& b)
   { ZZ_pEX x; add(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator+(const ZZ_pEX& a, const ZZ_p& b)
   { ZZ_pEX x; add(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator+(const ZZ_pEX& a, long b)
   { ZZ_pEX x; add(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator+(const ZZ_pE& a, const ZZ_pEX& b)
   { ZZ_pEX x; add(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator+(const ZZ_p& a, const ZZ_pEX& b)
   { ZZ_pEX x; add(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator+(long a, const ZZ_pEX& b)
   { ZZ_pEX x; add(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }


inline ZZ_pEX operator-(const ZZ_pEX& a, const ZZ_pEX& b)
   { ZZ_pEX x; sub(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator-(const ZZ_pEX& a, const ZZ_pE& b)
   { ZZ_pEX x; sub(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator-(const ZZ_pEX& a, const ZZ_p& b)
   { ZZ_pEX x; sub(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator-(const ZZ_pEX& a, long b)
   { ZZ_pEX x; sub(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator-(const ZZ_pE& a, const ZZ_pEX& b)
   { ZZ_pEX x; sub(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator-(const ZZ_p& a, const ZZ_pEX& b)
   { ZZ_pEX x; sub(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator-(long a, const ZZ_pEX& b)
   { ZZ_pEX x; sub(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }


inline ZZ_pEX& operator+=(ZZ_pEX& x, const ZZ_pEX& b)
   { add(x, x, b); return x; }

inline ZZ_pEX& operator+=(ZZ_pEX& x, const ZZ_pE& b)
   { add(x, x, b); return x; }

inline ZZ_pEX& operator+=(ZZ_pEX& x, const ZZ_p& b)
   { add(x, x, b); return x; }

inline ZZ_pEX& operator+=(ZZ_pEX& x, long b)
   { add(x, x, b); return x; }

inline ZZ_pEX& operator-=(ZZ_pEX& x, const ZZ_pEX& b)
   { sub(x, x, b); return x; }

inline ZZ_pEX& operator-=(ZZ_pEX& x, const ZZ_pE& b)
   { sub(x, x, b); return x; }

inline ZZ_pEX& operator-=(ZZ_pEX& x, const ZZ_p& b)
   { sub(x, x, b); return x; }

inline ZZ_pEX& operator-=(ZZ_pEX& x, long b)
   { sub(x, x, b); return x; }


inline ZZ_pEX operator-(const ZZ_pEX& a) 
   { ZZ_pEX x; negate(x, a); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX& operator++(ZZ_pEX& x) { add(x, x, 1); return x; }
inline void operator++(ZZ_pEX& x, int) { add(x, x, 1); }
inline ZZ_pEX& operator--(ZZ_pEX& x) { sub(x, x, 1); return x; }
inline void operator--(ZZ_pEX& x, int) { sub(x, x, 1); }



/*****************************************************************

                        Multiplication

******************************************************************/


void mul(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& b);
// x = a * b

void sqr(ZZ_pEX& x, const ZZ_pEX& a);
inline ZZ_pEX sqr(const ZZ_pEX& a) 
   { ZZ_pEX x; sqr(x, a); NTL_OPT_RETURN(ZZ_pEX, x); }
// x = a^2


void mul(ZZ_pEX & x, const ZZ_pEX& a, long b); 
void mul(ZZ_pEX & x, const ZZ_pEX& a, const ZZ_p& b); 
void mul(ZZ_pEX & x, const ZZ_pEX& a, const ZZ_pE& b); 

inline void mul(ZZ_pEX& x, long a, const ZZ_pEX& b)
   { mul(x, b, a); }
inline void mul(ZZ_pEX& x, const ZZ_p& a, const ZZ_pEX& b)
   { mul(x, b, a); }
inline void mul(ZZ_pEX& x, const ZZ_pE& a, const ZZ_pEX& b)
   { mul(x, b, a); }

void MulTrunc(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& b, long n);
inline ZZ_pEX MulTrunc(const ZZ_pEX& a, const ZZ_pEX& b, long n)
   { ZZ_pEX x; MulTrunc(x, a, b, n); NTL_OPT_RETURN(ZZ_pEX, x); }
// x = a * b % X^n

void SqrTrunc(ZZ_pEX& x, const ZZ_pEX& a, long n);
inline ZZ_pEX SqrTrunc(const ZZ_pEX& a, long n)
   { ZZ_pEX x; SqrTrunc(x, a, n); NTL_OPT_RETURN(ZZ_pEX, x); }
// x = a*a % X^n


inline ZZ_pEX operator*(const ZZ_pEX& a, const ZZ_pEX& b)
   { ZZ_pEX x; mul(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator*(const ZZ_pEX& a, const ZZ_pE& b)
   { ZZ_pEX x; mul(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator*(const ZZ_pEX& a, const ZZ_p& b)
   { ZZ_pEX x; mul(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator*(const ZZ_pEX& a, long b)
   { ZZ_pEX x; mul(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator*(const ZZ_pE& a, const ZZ_pEX& b)
   { ZZ_pEX x; mul(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator*(const ZZ_p& a, const ZZ_pEX& b)
   { ZZ_pEX x; mul(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator*(long a, const ZZ_pEX& b)
   { ZZ_pEX x; mul(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX& operator*=(ZZ_pEX& x, const ZZ_pEX& b)
   { mul(x, x, b); return x; }

inline ZZ_pEX& operator*=(ZZ_pEX& x, const ZZ_pE& b)
   { mul(x, x, b); return x; }

inline ZZ_pEX& operator*=(ZZ_pEX& x, const ZZ_p& b)
   { mul(x, x, b); return x; }

inline ZZ_pEX& operator*=(ZZ_pEX& x, long b)
   { mul(x, x, b); return x; }


void power(ZZ_pEX& x, const ZZ_pEX& a, long e);
inline ZZ_pEX power(const ZZ_pEX& a, long e)
   { ZZ_pEX x; power(x, a, e); NTL_OPT_RETURN(ZZ_pEX, x); }





/*************************************************************

                      Division

**************************************************************/

void DivRem(ZZ_pEX& q, ZZ_pEX& r, const ZZ_pEX& a, const ZZ_pEX& b);
// q = a/b, r = a%b

void div(ZZ_pEX& q, const ZZ_pEX& a, const ZZ_pEX& b);
void div(ZZ_pEX& q, const ZZ_pEX& a, const ZZ_pE& b);
void div(ZZ_pEX& q, const ZZ_pEX& a, const ZZ_p& b);
void div(ZZ_pEX& q, const ZZ_pEX& a, long b);
// q = a/b

void rem(ZZ_pEX& r, const ZZ_pEX& a, const ZZ_pEX& b);
// r = a%b

long divide(ZZ_pEX& q, const ZZ_pEX& a, const ZZ_pEX& b);
// if b | a, sets q = a/b and returns 1; otherwise returns 0

long divide(const ZZ_pEX& a, const ZZ_pEX& b);
// if b | a, sets q = a/b and returns 1; otherwise returns 0

void InvTrunc(ZZ_pEX& x, const ZZ_pEX& a, long m);
inline ZZ_pEX InvTrunc(const ZZ_pEX& a, long m)
   { ZZ_pEX x; InvTrunc(x, a, m); NTL_OPT_RETURN(ZZ_pEX, x); }
// computes x = a^{-1} % X^m 
// constant term must be invertible


inline ZZ_pEX operator/(const ZZ_pEX& a, const ZZ_pEX& b)
   { ZZ_pEX x; div(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator/(const ZZ_pEX& a, const ZZ_pE& b)
   { ZZ_pEX x; div(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator/(const ZZ_pEX& a, const ZZ_p& b)
   { ZZ_pEX x; div(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX operator/(const ZZ_pEX& a, long b)
   { ZZ_pEX x; div(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX& operator/=(ZZ_pEX& x, const ZZ_pEX& b)
   { div(x, x, b); return x; }

inline ZZ_pEX& operator/=(ZZ_pEX& x, const ZZ_pE& b)
   { div(x, x, b); return x; }

inline ZZ_pEX& operator/=(ZZ_pEX& x, const ZZ_p& b)
   { div(x, x, b); return x; }

inline ZZ_pEX& operator/=(ZZ_pEX& x, long b)
   { div(x, x, b); return x; }


inline ZZ_pEX operator%(const ZZ_pEX& a, const ZZ_pEX& b)
   { ZZ_pEX x; rem(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX& operator%=(ZZ_pEX& x, const ZZ_pEX& b)
   { rem(x, x, b); return x; }



/***********************************************************

                         GCD's

************************************************************/


void GCD(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& b);
inline ZZ_pEX GCD(const ZZ_pEX& a, const ZZ_pEX& b)
   { ZZ_pEX x; GCD(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }
// x = GCD(a, b),  x is always monic (or zero if a==b==0).

void XGCD(ZZ_pEX& d, ZZ_pEX& s, ZZ_pEX& t, const ZZ_pEX& a, const ZZ_pEX& b);
// d = gcd(a,b), a s + b t = d 


/*************************************************************

             Modular Arithmetic without pre-conditioning

**************************************************************/

// arithmetic mod f.
// all inputs and outputs are polynomials of degree less than deg(f).
// ASSUMPTION: f is assumed monic, and deg(f) > 0.
// NOTE: if you want to do many computations with a fixed f,
//       use the ZZ_pEXModulus data structure and associated routines below.



void MulMod(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& b, const ZZ_pEX& f);
inline ZZ_pEX MulMod(const ZZ_pEX& a, const ZZ_pEX& b, const ZZ_pEX& f)
   { ZZ_pEX x; MulMod(x, a, b, f); NTL_OPT_RETURN(ZZ_pEX, x); }
// x = (a * b) % f

void SqrMod(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& f);
inline ZZ_pEX SqrMod(const ZZ_pEX& a, const ZZ_pEX& f)
   { ZZ_pEX x; SqrMod(x, a, f); NTL_OPT_RETURN(ZZ_pEX, x); }
// x = a^2 % f

void MulByXMod(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& f);
inline ZZ_pEX MulByXMod(const ZZ_pEX& a, const ZZ_pEX& f)
   { ZZ_pEX x; MulByXMod(x, a, f); NTL_OPT_RETURN(ZZ_pEX, x); }
// x = (a * X) mod f

void InvMod(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& f);
inline ZZ_pEX InvMod(const ZZ_pEX& a, const ZZ_pEX& f)
   { ZZ_pEX x; InvMod(x, a, f); NTL_OPT_RETURN(ZZ_pEX, x); }
// x = a^{-1} % f, error is a is not invertible

long InvModStatus(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& f);
// if (a, f) = 1, returns 0 and sets x = a^{-1} % f
// otherwise, returns 1 and sets x = (a, f)





/******************************************************************

        Modular Arithmetic with Pre-conditioning

*******************************************************************/


// If you need to do a lot of arithmetic modulo a fixed f,
// build ZZ_pEXModulus F for f.  This pre-computes information about f
// that speeds up the computation a great deal.

class ZZ_pEXModulus {
public:
   ZZ_pEXModulus();
   ~ZZ_pEXModulus();

   ZZ_pEXModulus(const ZZ_pEX& ff);

   ZZ_pEX f;   // the modulus

   operator const ZZ_pEX& () const { return f; }
   const ZZ_pEX& val() const { return f; }

   long n; //  deg(f)

   long method;

   ZZ_pEX h0;
   ZZ_pE hlc;
   ZZ_pEX f0;

   OptionalVal< Lazy<vec_ZZ_pE> > tracevec; 
   // extra level of indirection to ensure relocatability

}; 


NTL_DECLARE_RELOCATABLE((ZZ_pEXModulus*))

inline long deg(const ZZ_pEXModulus& F) { return F.n; }


void build(ZZ_pEXModulus& F, const ZZ_pEX& f);

void rem(ZZ_pEX& r, const ZZ_pEX& a, const ZZ_pEXModulus& F);
   
void DivRem(ZZ_pEX& q, ZZ_pEX& r, const ZZ_pEX& a, const ZZ_pEXModulus& F);

void div(ZZ_pEX& q, const ZZ_pEX& a, const ZZ_pEXModulus& F);

void MulMod(ZZ_pEX& c, const ZZ_pEX& a, const ZZ_pEX& b, 
            const ZZ_pEXModulus& F);
inline ZZ_pEX MulMod(const ZZ_pEX& a, const ZZ_pEX& b, 
            const ZZ_pEXModulus& F)
   { ZZ_pEX x; MulMod(x, a, b, F); NTL_OPT_RETURN(ZZ_pEX, x); }

void SqrMod(ZZ_pEX& c, const ZZ_pEX& a, const ZZ_pEXModulus& F);
inline ZZ_pEX SqrMod(const ZZ_pEX& a, const ZZ_pEXModulus& F)
   { ZZ_pEX x; SqrMod(x, a, F); NTL_OPT_RETURN(ZZ_pEX, x); }


void PowerMod(ZZ_pEX& h, const ZZ_pEX& g, const ZZ& e, const ZZ_pEXModulus& F);

inline void PowerMod(ZZ_pEX& h, const ZZ_pEX& g, long e, 
                     const ZZ_pEXModulus& F)
   { PowerMod(h, g, ZZ_expo(e), F); }

inline ZZ_pEX PowerMod(const ZZ_pEX& g, const ZZ& e, 
                             const ZZ_pEXModulus& F)
   { ZZ_pEX x; PowerMod(x, g, e, F);  NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX PowerMod(const ZZ_pEX& g, long e, const ZZ_pEXModulus& F)
   { ZZ_pEX x; PowerMod(x, g, e, F);  NTL_OPT_RETURN(ZZ_pEX, x); }

void PowerXMod(ZZ_pEX& hh, const ZZ& e, const ZZ_pEXModulus& F);

inline void PowerXMod(ZZ_pEX& h, long e, const ZZ_pEXModulus& F)
   { PowerXMod(h, ZZ_expo(e), F); }


inline ZZ_pEX PowerXMod(const ZZ& e, const ZZ_pEXModulus& F)
   { ZZ_pEX x; PowerXMod(x, e, F);  NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX PowerXMod(long e, const ZZ_pEXModulus& F)
   { ZZ_pEX x; PowerXMod(x, e, F);  NTL_OPT_RETURN(ZZ_pEX, x); }


inline ZZ_pEX operator%(const ZZ_pEX& a, const ZZ_pEXModulus& F)
   { ZZ_pEX x; rem(x, a, F); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX& operator%=(ZZ_pEX& x, const ZZ_pEXModulus& F)
   { rem(x, x, F); return x; }

inline ZZ_pEX operator/(const ZZ_pEX& a, const ZZ_pEXModulus& F)
   { ZZ_pEX x; div(x, a, F); NTL_OPT_RETURN(ZZ_pEX, x); }

inline ZZ_pEX& operator/=(ZZ_pEX& x, const ZZ_pEXModulus& F)
   { div(x, x, F); return x; }



/*****************************************************************

                       vectors of ZZ_pEX's

*****************************************************************/



typedef Vec<ZZ_pEX> vec_ZZ_pEX;





/*******************************************************

              Evaluation and related problems

********************************************************/




void BuildFromRoots(ZZ_pEX& x, const vec_ZZ_pE& a);
inline ZZ_pEX BuildFromRoots(const vec_ZZ_pE& a)
   { ZZ_pEX x; BuildFromRoots(x, a); NTL_OPT_RETURN(ZZ_pEX, x); }
// computes the polynomial (X-a[0]) ... (X-a[n-1]), where n = a.length()


void eval(ZZ_pE& b, const ZZ_pEX& f, const ZZ_pE& a);
inline ZZ_pE eval(const ZZ_pEX& f, const ZZ_pE& a)
   { ZZ_pE x; eval(x, f, a); NTL_OPT_RETURN(ZZ_pE, x); }
// b = f(a)

void eval(vec_ZZ_pE& b, const ZZ_pEX& f, const vec_ZZ_pE& a);
inline vec_ZZ_pE eval(const ZZ_pEX& f, const vec_ZZ_pE& a)
   { vec_ZZ_pE x; eval(x, f, a); NTL_OPT_RETURN(vec_ZZ_pE, x); }
//  b[i] = f(a[i])

inline void eval(ZZ_pE& b, const ZZ_pX& f, const ZZ_pE& a)
   { conv(b, CompMod(f, rep(a), ZZ_pE::modulus())); }
   
inline ZZ_pE eval(const ZZ_pX& f, const ZZ_pE& a)
   { ZZ_pE x; eval(x, f, a); NTL_OPT_RETURN(ZZ_pE, x); }
// b = f(a)


void interpolate(ZZ_pEX& f, const vec_ZZ_pE& a, const vec_ZZ_pE& b);
inline ZZ_pEX interpolate(const vec_ZZ_pE& a, const vec_ZZ_pE& b)
   { ZZ_pEX x; interpolate(x, a, b); NTL_OPT_RETURN(ZZ_pEX, x); }
// computes f such that f(a[i]) = b[i]





/**********************************************************

         Modular Composition and Minimal Polynomials

***********************************************************/



void CompMod(ZZ_pEX& x, const ZZ_pEX& g, const ZZ_pEX& h, const ZZ_pEXModulus& F);
inline ZZ_pEX 
CompMod(const ZZ_pEX& g, const ZZ_pEX& h, const ZZ_pEXModulus& F)
   { ZZ_pEX x; CompMod(x, g, h, F); NTL_OPT_RETURN(ZZ_pEX, x); }
// x = g(h) mod f

void Comp2Mod(ZZ_pEX& x1, ZZ_pEX& x2, const ZZ_pEX& g1, const ZZ_pEX& g2,
              const ZZ_pEX& h, const ZZ_pEXModulus& F);
// xi = gi(h) mod f (i=1,2)

void Comp3Mod(ZZ_pEX& x1, ZZ_pEX& x2, ZZ_pEX& x3, 
              const ZZ_pEX& g1, const ZZ_pEX& g2, const ZZ_pEX& g3,
              const ZZ_pEX& h, const ZZ_pEXModulus& F);
// xi = gi(h) mod f (i=1..3)



// The routine build (see below) which is implicitly called
// by the various compose and UpdateMap routines builds a table
// of polynomials.  
// If ZZ_pEXArgBound > 0, then the table is limited in
// size to approximamtely that many KB.
// If ZZ_pEXArgBound <= 0, then it is ignored, and space is allocated
// so as to maximize speed.
// Initially, ZZ_pEXArgBound = 0.


// If a single h is going to be used with many g's
// then you should build a ZZ_pEXArgument for h,
// and then use the compose routine below.
// build computes and stores h, h^2, ..., h^m mod f.
// After this pre-computation, composing a polynomial of degree 
// roughly n with h takes n/m multiplies mod f, plus n^2
// scalar multiplies.
// Thus, increasing m increases the space requirement and the pre-computation
// time, but reduces the composition time.
// If ZZ_pEXArgBound > 0, a table of size less than m may be built.

struct ZZ_pEXArgument {
   vec_ZZ_pEX H;
};

extern NTL_CHEAP_THREAD_LOCAL long ZZ_pEXArgBound;


void build(ZZ_pEXArgument& H, const ZZ_pEX& h, const ZZ_pEXModulus& F, long m);

// m must be > 0, otherwise an error is raised

void CompMod(ZZ_pEX& x, const ZZ_pEX& g, const ZZ_pEXArgument& H, 
             const ZZ_pEXModulus& F);

inline ZZ_pEX 
CompMod(const ZZ_pEX& g, const ZZ_pEXArgument& H, const ZZ_pEXModulus& F)
   { ZZ_pEX x; CompMod(x, g, H, F); NTL_OPT_RETURN(ZZ_pEX, x); }
   



void MinPolySeq(ZZ_pEX& h, const vec_ZZ_pE& a, long m);
inline ZZ_pEX MinPolySeq(const vec_ZZ_pE& a, long m)
   { ZZ_pEX x; MinPolySeq(x, a, m); NTL_OPT_RETURN(ZZ_pEX, x); }


void MinPolyMod(ZZ_pEX& hh, const ZZ_pEX& g, const ZZ_pEXModulus& F);
inline ZZ_pEX MinPolyMod(const ZZ_pEX& g, const ZZ_pEXModulus& F)
   { ZZ_pEX x; MinPolyMod(x, g, F); NTL_OPT_RETURN(ZZ_pEX, x); }


void MinPolyMod(ZZ_pEX& hh, const ZZ_pEX& g, const ZZ_pEXModulus& F, long m);
inline ZZ_pEX MinPolyMod(const ZZ_pEX& g, const ZZ_pEXModulus& F, long m)
   { ZZ_pEX x; MinPolyMod(x, g, F, m); NTL_OPT_RETURN(ZZ_pEX, x); }

void ProbMinPolyMod(ZZ_pEX& hh, const ZZ_pEX& g, const ZZ_pEXModulus& F);
inline ZZ_pEX ProbMinPolyMod(const ZZ_pEX& g, const ZZ_pEXModulus& F)
   { ZZ_pEX x; ProbMinPolyMod(x, g, F); NTL_OPT_RETURN(ZZ_pEX, x); }

void ProbMinPolyMod(ZZ_pEX& hh, const ZZ_pEX& g, const ZZ_pEXModulus& F, long m);
inline ZZ_pEX ProbMinPolyMod(const ZZ_pEX& g, const ZZ_pEXModulus& F, long m)
   { ZZ_pEX x; ProbMinPolyMod(x, g, F, m); NTL_OPT_RETURN(ZZ_pEX, x); }

void IrredPolyMod(ZZ_pEX& h, const ZZ_pEX& g, const ZZ_pEXModulus& F);
inline ZZ_pEX IrredPolyMod(const ZZ_pEX& g, const ZZ_pEXModulus& F)
   { ZZ_pEX x; IrredPolyMod(x, g, F); NTL_OPT_RETURN(ZZ_pEX, x); }

void IrredPolyMod(ZZ_pEX& h, const ZZ_pEX& g, const ZZ_pEXModulus& F, long m);
inline ZZ_pEX IrredPolyMod(const ZZ_pEX& g, const ZZ_pEXModulus& F, long m)
   { ZZ_pEX x; IrredPolyMod(x, g, F, m); NTL_OPT_RETURN(ZZ_pEX, x); }


struct ZZ_pEXTransMultiplier {
   ZZ_pEX f0, fbi, b;
   long shamt, shamt_fbi, shamt_b;
};

void build(ZZ_pEXTransMultiplier& B, const ZZ_pEX& b, const ZZ_pEXModulus& F);

void TransMulMod(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEXTransMultiplier& B,
               const ZZ_pEXModulus& F);

void UpdateMap(vec_ZZ_pE& x, const vec_ZZ_pE& a, 
         const ZZ_pEXTransMultiplier& B, const ZZ_pEXModulus& F);

inline vec_ZZ_pE UpdateMap(const vec_ZZ_pE& a,
         const ZZ_pEXTransMultiplier& B, const ZZ_pEXModulus& F)
   { vec_ZZ_pE x; UpdateMap(x, a, B, F); NTL_OPT_RETURN(vec_ZZ_pE, x); }

void ProjectPowers(vec_ZZ_pE& x, const vec_ZZ_pE& a, long k, 
                   const ZZ_pEXArgument& H, const ZZ_pEXModulus& F);
inline vec_ZZ_pE ProjectPowers(const vec_ZZ_pE& a, long k, 
                   const ZZ_pEXArgument& H, const ZZ_pEXModulus& F)
   { vec_ZZ_pE x; ProjectPowers(x, a, k, H, F); NTL_OPT_RETURN(vec_ZZ_pE, x); }

void ProjectPowers(vec_ZZ_pE& x, const vec_ZZ_pE& a, long k, const ZZ_pEX& h, 
                   const ZZ_pEXModulus& F);
inline vec_ZZ_pE ProjectPowers(const vec_ZZ_pE& a, long k, 
                   const ZZ_pEX& H, const ZZ_pEXModulus& F)
   { vec_ZZ_pE x; ProjectPowers(x, a, k, H, F); NTL_OPT_RETURN(vec_ZZ_pE, x); }

inline void project(ZZ_pE& x, const vec_ZZ_pE& a, const ZZ_pEX& b)
   { InnerProduct(x, a, b.rep); }

inline ZZ_pE project(const vec_ZZ_pE& a, const ZZ_pEX& b)
   { ZZ_pE x; InnerProduct(x, a, b.rep); NTL_OPT_RETURN(ZZ_pE, x); }



/*****************************************************************

          modular composition and minimal polynonomials
                         in towers

******************************************************************/


// composition

void CompTower(ZZ_pEX& x, const ZZ_pX& g, const ZZ_pEXArgument& A,
             const ZZ_pEXModulus& F);

inline ZZ_pEX CompTower(const ZZ_pX& g, const ZZ_pEXArgument& A,
             const ZZ_pEXModulus& F)
   { ZZ_pEX x; CompTower(x, g, A, F); NTL_OPT_RETURN(ZZ_pEX, x); }

void CompTower(ZZ_pEX& x, const ZZ_pX& g, const ZZ_pEX& h,
             const ZZ_pEXModulus& F);

inline ZZ_pEX CompTower(const ZZ_pX& g, const ZZ_pEX& h,
             const ZZ_pEXModulus& F)
   { ZZ_pEX x; CompTower(x, g, h, F); NTL_OPT_RETURN(ZZ_pEX, x); }

// prob min poly

void ProbMinPolyTower(ZZ_pX& h, const ZZ_pEX& g, const ZZ_pEXModulus& F,
                      long m);

inline ZZ_pX ProbMinPolyTower(const ZZ_pEX& g, const ZZ_pEXModulus& F,
                      long m)
   { ZZ_pX x; ProbMinPolyTower(x, g, F, m); NTL_OPT_RETURN(ZZ_pX, x); }

inline void ProbMinPolyTower(ZZ_pX& h, const ZZ_pEX& g, 
                             const ZZ_pEXModulus& F)
   { ProbMinPolyTower(h, g, F, deg(F)*ZZ_pE::degree()); }

inline ZZ_pX ProbMinPolyTower(const ZZ_pEX& g, const ZZ_pEXModulus& F)
   { ZZ_pX x; ProbMinPolyTower(x, g, F); NTL_OPT_RETURN(ZZ_pX, x); }


// min poly


void MinPolyTower(ZZ_pX& h, const ZZ_pEX& g, const ZZ_pEXModulus& F,
                      long m);

inline ZZ_pX MinPolyTower(const ZZ_pEX& g, const ZZ_pEXModulus& F,
                      long m)
   { ZZ_pX x; MinPolyTower(x, g, F, m); NTL_OPT_RETURN(ZZ_pX, x); }

inline void MinPolyTower(ZZ_pX& h, const ZZ_pEX& g, const ZZ_pEXModulus& F)
   { MinPolyTower(h, g, F, deg(F)*ZZ_pE::degree()); }


inline ZZ_pX MinPolyTower(const ZZ_pEX& g, const ZZ_pEXModulus& F)
   { ZZ_pX x; MinPolyTower(x, g, F); NTL_OPT_RETURN(ZZ_pX, x); }

// irred poly


void IrredPolyTower(ZZ_pX& h, const ZZ_pEX& g, const ZZ_pEXModulus& F,
                      long m);

inline ZZ_pX IrredPolyTower(const ZZ_pEX& g, const ZZ_pEXModulus& F,
                      long m)
   { ZZ_pX x; IrredPolyTower(x, g, F, m); NTL_OPT_RETURN(ZZ_pX, x); }

inline void IrredPolyTower(ZZ_pX& h, const ZZ_pEX& g, const ZZ_pEXModulus& F)
   { IrredPolyTower(h, g, F, deg(F)*ZZ_pE::degree()); }


inline ZZ_pX IrredPolyTower(const ZZ_pEX& g, const ZZ_pEXModulus& F)
   { ZZ_pX x; IrredPolyTower(x, g, F); NTL_OPT_RETURN(ZZ_pX, x); }

/*****************************************************************

                   Traces, norms, resultants

******************************************************************/

void TraceVec(vec_ZZ_pE& S, const ZZ_pEX& f);

inline vec_ZZ_pE TraceVec(const ZZ_pEX& f)
   { vec_ZZ_pE x; TraceVec(x, f); NTL_OPT_RETURN(vec_ZZ_pE, x); }


void TraceMod(ZZ_pE& x, const ZZ_pEX& a, const ZZ_pEXModulus& F);

inline ZZ_pE TraceMod(const ZZ_pEX& a, const ZZ_pEXModulus& F)
   { ZZ_pE x; TraceMod(x, a, F); NTL_OPT_RETURN(ZZ_pE, x); }

void TraceMod(ZZ_pE& x, const ZZ_pEX& a, const ZZ_pEX& f);

inline ZZ_pE TraceMod(const ZZ_pEX& a, const ZZ_pEX& f)
   { ZZ_pE x; TraceMod(x, a, f); NTL_OPT_RETURN(ZZ_pE, x); }





void NormMod(ZZ_pE& x, const ZZ_pEX& a, const ZZ_pEX& f);

inline ZZ_pE NormMod(const ZZ_pEX& a, const ZZ_pEX& f)
   { ZZ_pE x; NormMod(x, a, f); NTL_OPT_RETURN(ZZ_pE, x); }

void resultant(ZZ_pE& rres, const ZZ_pEX& a, const ZZ_pEX& b);

inline ZZ_pE resultant(const ZZ_pEX& a, const ZZ_pEX& b)
   { ZZ_pE x; resultant(x, a, b); NTL_OPT_RETURN(ZZ_pE, x); }


NTL_CLOSE_NNS

#endif
