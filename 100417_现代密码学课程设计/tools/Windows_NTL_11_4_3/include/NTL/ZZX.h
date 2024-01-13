
#ifndef NTL_ZZX__H
#define NTL_ZZX__H

#include <NTL/vec_ZZ.h>
#include <NTL/lzz_pX.h>
#include <NTL/ZZ_pX.h>

NTL_OPEN_NNS


class ZZX {

public:

vec_ZZ rep;


/***************************************************************

          Constructors, Destructors, and Assignment

****************************************************************/


ZZX() { }
//  initial value 0

explicit ZZX(long a) { *this = a; }
explicit ZZX(const ZZ& a) { *this = a; }

ZZX(INIT_SIZE_TYPE, long n) 
// initial value 0, but space is pre-allocated for n coefficients

   { rep.SetMaxLength(n); }

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



typedef ZZ coeff_type;
void SetLength(long n) { rep.SetLength(n); }
ZZ& operator[](long i) { return rep[i]; }
const ZZ& operator[](long i) const { return rep[i]; }




static const ZZX& zero();

inline ZZX(long i, const ZZ& c);
inline ZZX(long i, long c);

inline ZZX(INIT_MONO_TYPE, long i, const ZZ& c);
inline ZZX(INIT_MONO_TYPE, long i, long c);
inline ZZX(INIT_MONO_TYPE, long i);


inline ZZX& operator=(long a);
inline ZZX& operator=(const ZZ& a);


ZZX(ZZX& x, INIT_TRANS_TYPE) : rep(x.rep, INIT_TRANS) { }

void swap(ZZX& x) { rep.swap(x.rep); }
// swap with x (only pointers are swapped)

};


NTL_DECLARE_RELOCATABLE((ZZX*))




/********************************************************************

                           input and output

I/O format:

   [a_0 a_1 ... a_n],

represents the polynomial a_0 + a_1*X + ... + a_n*X^n.

On output, all coefficients will be integers between 0 and p-1,
amd a_n not zero (the zero polynomial is [ ]).
Leading zeroes are stripped.

*********************************************************************/


NTL_SNS istream& operator>>(NTL_SNS istream& s, ZZX& x);
NTL_SNS ostream& operator<<(NTL_SNS ostream& s, const ZZX& a);




/**********************************************************

                   Some utility routines

***********************************************************/


inline long deg(const ZZX& a) { return a.rep.length() - 1; }
// degree of a polynomial.
// note that the zero polynomial has degree -1.

const ZZ& coeff(const ZZX& a, long i);
// zero if i not in range

void GetCoeff(ZZ& x, const ZZX& a, long i);
// x = a[i], or zero if i not in range

const ZZ& LeadCoeff(const ZZX& a);
// zero if a == 0

const ZZ& ConstTerm(const ZZX& a);
// zero if a == 0

void SetCoeff(ZZX& x, long i, const ZZ& a);
// x[i] = a, error is raised if i < 0

void SetCoeff(ZZX& x, long i, long a);
// x[i] = a, error is raised if i < 0

void SetCoeff(ZZX& x, long i);
// x[i] = 1, error is raised if i < 0

inline ZZX::ZZX(long i, const ZZ& a) { SetCoeff(*this, i, a); }
inline ZZX::ZZX(long i, long a) { SetCoeff(*this, i, a); }

inline ZZX::ZZX(INIT_MONO_TYPE, long i, const ZZ& a) { SetCoeff(*this, i, a); }
inline ZZX::ZZX(INIT_MONO_TYPE, long i, long a) { SetCoeff(*this, i, a); }
inline ZZX::ZZX(INIT_MONO_TYPE, long i) { SetCoeff(*this, i); }


void SetX(ZZX& x);
// x is set to the monomial X

long IsX(const ZZX& a);
// test if x = X

inline void clear(ZZX& x) 
// x = 0

   { x.rep.SetLength(0); }

inline void set(ZZX& x)
// x = 1

   { x.rep.SetLength(1); set(x.rep[0]); }

inline void swap(ZZX& x, ZZX& y)
// swap x & y (only pointers are swapped)

   { x.swap(y); }

void trunc(ZZX& x, const ZZX& a, long m);
// x = a % X^m

inline ZZX trunc(const ZZX& a, long m)
   { ZZX x; trunc(x, a, m); NTL_OPT_RETURN(ZZX, x); }

void RightShift(ZZX& x, const ZZX& a, long n);
// x = a/X^n

inline ZZX RightShift(const ZZX& a, long n)
   { ZZX x; RightShift(x, a, n); NTL_OPT_RETURN(ZZX, x); }

void LeftShift(ZZX& x, const ZZX& a, long n);
// x = a*X^n

inline ZZX LeftShift(const ZZX& a, long n)
   { ZZX x; LeftShift(x, a, n); NTL_OPT_RETURN(ZZX, x); }


#ifndef NTL_TRANSITION

inline ZZX operator>>(const ZZX& a, long n)
   { ZZX x; RightShift(x, a, n); NTL_OPT_RETURN(ZZX, x); }

inline ZZX operator<<(const ZZX& a, long n)
   { ZZX x; LeftShift(x, a, n); NTL_OPT_RETURN(ZZX, x); }

inline ZZX& operator<<=(ZZX& x, long n) 
   { LeftShift(x, x, n); return x; }

inline ZZX& operator>>=(ZZX& x, long n) 
   { RightShift(x, x, n); return x; }

#endif


void diff(ZZX& x, const ZZX& a);
// x = derivative of a

inline ZZX diff(const ZZX& a)
   { ZZX x; diff(x, a); NTL_OPT_RETURN(ZZX, x); }

void InvTrunc(ZZX& x, const ZZX& a, long m);
// computes x = a^{-1} % X^m
// constant term must be non-zero

inline ZZX InvTrunc(const ZZX& a, long m)
   { ZZX x; InvTrunc(x, a, m); NTL_OPT_RETURN(ZZX, x); }

void MulTrunc(ZZX& x, const ZZX& a, const ZZX& b, long n);
// x = a * b % X^n

inline ZZX MulTrunc(const ZZX& a, const ZZX& b, long n)
   { ZZX x; MulTrunc(x, a, b, n); NTL_OPT_RETURN(ZZX, x); }

void SqrTrunc(ZZX& x, const ZZX& a, long n);
// x = a^2 % X^n

inline ZZX SqrTrunc(const ZZX& a, long n)
   { ZZX x; SqrTrunc(x, a, n); NTL_OPT_RETURN(ZZX, x); }

void reverse(ZZX& c, const ZZX& a, long hi);

inline ZZX reverse(const ZZX& a, long hi)
   { ZZX x; reverse(x, a, hi); NTL_OPT_RETURN(ZZX, x); }

inline void reverse(ZZX& c, const ZZX& a)
{  reverse(c, a, deg(a)); }

inline ZZX reverse(const ZZX& a)
   { ZZX x; reverse(x, a); NTL_OPT_RETURN(ZZX, x); }


inline void VectorCopy(vec_ZZ& x, const ZZX& a, long n)
   { VectorCopy(x, a.rep, n); }

inline vec_ZZ VectorCopy(const ZZX& a, long n)
   { return VectorCopy(a.rep, n); }







/*******************************************************************

                        conversion routines

********************************************************************/


void conv(ZZX& x, long a);
inline ZZX to_ZZX(long a)
   { ZZX x; conv(x, a); NTL_OPT_RETURN(ZZX, x); }

inline ZZX& ZZX::operator=(long a)
   { conv(*this, a); return *this; }

void conv(ZZX& x, const ZZ& a);
inline ZZX to_ZZX(const ZZ& a)
   { ZZX x; conv(x, a); NTL_OPT_RETURN(ZZX, x); }

inline ZZX& ZZX::operator=(const ZZ& a)
   { conv(*this, a); return *this; }

void conv(ZZX& x, const vec_ZZ& a);
inline ZZX to_ZZX(const vec_ZZ& a)
   { ZZX x; conv(x, a); NTL_OPT_RETURN(ZZX, x); }

void conv(zz_pX& x, const ZZX& a);
inline zz_pX to_zz_pX(const ZZX& a)
   { zz_pX x; conv(x, a); NTL_OPT_RETURN(zz_pX, x); }

void conv(ZZ_pX& x, const ZZX& a);
inline ZZ_pX to_ZZ_pX(const ZZX& a)
   { ZZ_pX x; conv(x, a); NTL_OPT_RETURN(ZZ_pX, x); }

void conv(ZZX& x, const ZZ_pX& a);
inline ZZX to_ZZX(const ZZ_pX& a)
   { ZZX x; conv(x, a); NTL_OPT_RETURN(ZZX, x); }

void conv(ZZX& x, const zz_pX& a);
inline ZZX to_ZZX(const zz_pX& a)
   { ZZX x; conv(x, a); NTL_OPT_RETURN(ZZX, x); }




/* additional legacy conversions for v6 conversion regime */

inline void conv(ZZX& x, const ZZX& a)
   { x = a; }

inline void conv(vec_ZZ& x, const ZZX& a)
   { x = a.rep; }


/* ------------------------------------- */



/*************************************************************

                        Comparison

**************************************************************/

long IsZero(const ZZX& a); 

long IsOne(const ZZX& a);

long operator==(const ZZX& a, const ZZX& b);

inline long operator!=(const ZZX& a, const ZZX& b) { return !(a == b); }

long operator==(const ZZX& a, const ZZ& b);
long operator==(const ZZX& a, long b);

inline long operator==(const ZZ& a, const ZZX& b) { return b == a; }
inline long operator==(long a, const ZZX& b) { return b == a; }

inline long operator!=(const ZZX& a, const ZZ& b) { return !(a == b); }
inline long operator!=(const ZZX& a, long b) { return !(a == b); }
inline long operator!=(const ZZ& a, const ZZX& b) { return !(a == b); }
inline long operator!=(long a, const ZZX& b) { return !(a == b); }


/***************************************************************

                         Addition

****************************************************************/

void add(ZZX& x, const ZZX& a, const ZZX& b);
// x = a + b

void sub(ZZX& x, const ZZX& a, const ZZX& b);
// x = a - b

void negate(ZZX& x, const ZZX& a);
// x = -a

// scalar versions


void add(ZZX & x, const ZZX& a, const ZZ& b); // x = a + b
void add(ZZX& x, const ZZX& a, long b);

inline void add(ZZX& x, const ZZ& a, const ZZX& b) { add(x, b, a); }
inline void add(ZZX& x, long a, const ZZX& b) { add(x, b, a); }


void sub(ZZX & x, const ZZX& a, const ZZ& b); // x = a - b
void sub(ZZX& x, const ZZX& a, long b);

void sub(ZZX& x, const ZZ& a, const ZZX& b);
void sub(ZZX& x, long a, const ZZX& b);


inline ZZX operator+(const ZZX& a, const ZZX& b)
   { ZZX x; add(x, a, b); NTL_OPT_RETURN(ZZX, x); }

inline ZZX operator+(const ZZX& a, const ZZ& b)
   { ZZX x; add(x, a, b); NTL_OPT_RETURN(ZZX, x); }

inline ZZX operator+(const ZZX& a, long b)
   { ZZX x; add(x, a, b); NTL_OPT_RETURN(ZZX, x); }

inline ZZX operator+(const ZZ& a, const ZZX& b)
   { ZZX x; add(x, a, b); NTL_OPT_RETURN(ZZX, x); }

inline ZZX operator+(long a, const ZZX& b)
   { ZZX x; add(x, a, b); NTL_OPT_RETURN(ZZX, x); }


inline ZZX operator-(const ZZX& a, const ZZX& b)
   { ZZX x; sub(x, a, b); NTL_OPT_RETURN(ZZX, x); }

inline ZZX operator-(const ZZX& a, const ZZ& b)
   { ZZX x; sub(x, a, b); NTL_OPT_RETURN(ZZX, x); }

inline ZZX operator-(const ZZX& a, long b)
   { ZZX x; sub(x, a, b); NTL_OPT_RETURN(ZZX, x); }

inline ZZX operator-(const ZZ& a, const ZZX& b)
   { ZZX x; sub(x, a, b); NTL_OPT_RETURN(ZZX, x); }

inline ZZX operator-(long a, const ZZX& b)
   { ZZX x; sub(x, a, b); NTL_OPT_RETURN(ZZX, x); }


inline ZZX& operator+=(ZZX& x, const ZZX& b)
   { add(x, x, b); return x; }

inline ZZX& operator+=(ZZX& x, const ZZ& b)
   { add(x, x, b); return x; }

inline ZZX& operator+=(ZZX& x, long b)
   { add(x, x, b); return x; }

inline ZZX& operator-=(ZZX& x, const ZZX& b)
   { sub(x, x, b); return x; }

inline ZZX& operator-=(ZZX& x, const ZZ& b)
   { sub(x, x, b); return x; }

inline ZZX& operator-=(ZZX& x, long b)
   { sub(x, x, b); return x; }


inline ZZX operator-(const ZZX& a) 
   { ZZX x; negate(x, a); NTL_OPT_RETURN(ZZX, x); }

inline ZZX& operator++(ZZX& x) { add(x, x, 1); return x; }
inline void operator++(ZZX& x, int) { add(x, x, 1); }
inline ZZX& operator--(ZZX& x) { sub(x, x, 1); return x; }
inline void operator--(ZZX& x, int) { sub(x, x, 1); }


/*****************************************************************

                        Multiplication

******************************************************************/


void mul(ZZX& x, const ZZX& a, const ZZX& b);
// x = a * b


void sqr(ZZX& x, const ZZX& a);
inline ZZX sqr(const ZZX& a)
   { ZZX x; sqr(x, a); NTL_OPT_RETURN(ZZX, x); }
// x = a^2

void PlainMul(ZZX& x, const ZZX& a, const ZZX& b);
void PlainSqr(ZZX& x, const ZZX& a);

void KarMul(ZZX& x, const ZZX& a, const ZZX& b);
void KarSqr(ZZX& x, const ZZX& a);

void HomMul(ZZX& x, const ZZX& a, const ZZX& b);
void HomSqr(ZZX& x, const ZZX& a);

void SSMul(ZZX& x, const ZZX& a, const ZZX& b);
void SSSqr(ZZX& x, const ZZX& a);

void SSMul(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b);
void SSSqr(ZZ_pX& x, const ZZ_pX& a);

void KarMul(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b);
void KarSqr(ZZ_pX& x, const ZZ_pX& a);

double SSRatio(long na, long maxa, long nb, long maxb);


void mul(ZZX & x, const ZZX& a, const ZZ& b);
void mul(ZZX& x, const ZZX& a, long b);

inline void mul(ZZX& x, const ZZ& a, const ZZX& b) { mul(x, b, a); } 
inline void mul(ZZX& x, long a, const ZZX& b) { mul(x, b, a); } 


inline ZZX operator*(const ZZX& a, const ZZX& b)
   { ZZX x; mul(x, a, b); NTL_OPT_RETURN(ZZX, x); }

inline ZZX operator*(const ZZX& a, const ZZ& b)
   { ZZX x; mul(x, a, b); NTL_OPT_RETURN(ZZX, x); }

inline ZZX operator*(const ZZX& a, long b)
   { ZZX x; mul(x, a, b); NTL_OPT_RETURN(ZZX, x); }

inline ZZX operator*(const ZZ& a, const ZZX& b)
   { ZZX x; mul(x, a, b); NTL_OPT_RETURN(ZZX, x); }

inline ZZX operator*(long a, const ZZX& b)
   { ZZX x; mul(x, a, b); NTL_OPT_RETURN(ZZX, x); }

inline ZZX& operator*=(ZZX& x, const ZZX& b)
   { mul(x, x, b); return x; }

inline ZZX& operator*=(ZZX& x, const ZZ& b)
   { mul(x, x, b); return x; }

inline ZZX& operator*=(ZZX& x, long b)
   { mul(x, x, b); return x; }






/*************************************************************

                      Division

**************************************************************/



// "plain" versions
void PlainPseudoDivRem(ZZX& q, ZZX& r, const ZZX& a, const ZZX& b);
void PlainPseudoDiv(ZZX& q, const ZZX& a, const ZZX& b);
void PlainPseudoRem(ZZX& r, const ZZX& a, const ZZX& b);

// "homomorphic imaging" versions
void HomPseudoDivRem(ZZX& q, ZZX& r, const ZZX& a, const ZZX& b);
void HomPseudoDiv(ZZX& q, const ZZX& a, const ZZX& b);
void HomPseudoRem(ZZX& r, const ZZX& a, const ZZX& b);

inline void PseudoDivRem(ZZX& q, ZZX& r, const ZZX& a, const ZZX& b)
// performs pseudo-division:  computes q and r
// with deg(r) < deg(b), and LeadCoeff(b)^(deg(a)-deg(b)+1) a = b q + r.
// current implementation always defaults to "plain"

   { PlainPseudoDivRem(q, r, a, b); }

inline void PseudoDiv(ZZX& q, const ZZX& a, const ZZX& b)

   { PlainPseudoDiv(q, a, b); }

inline void PseudoRem(ZZX& r, const ZZX& a, const ZZX& b)

   { PlainPseudoRem(r, a, b); }

inline ZZX PseudoDiv(const ZZX& a, const ZZX& b)
   { ZZX x; PseudoDiv(x, a, b); NTL_OPT_RETURN(ZZX, x); }

inline ZZX PseudoRem(const ZZX& a, const ZZX& b)
   { ZZX x; PseudoRem(x, a, b); NTL_OPT_RETURN(ZZX, x); }


#ifndef NTL_TRANSITION

void DivRem(ZZX& q, ZZX& r, const ZZX& a, const ZZX& b);

void div(ZZX& q, const ZZX& a, const ZZX& b);
void div(ZZX& q, const ZZX& a, const ZZ& b);
void div(ZZX& q, const ZZX& a, long b);

void rem(ZZX& r, const ZZX& a, const ZZX& b);

inline ZZX operator/(const ZZX& a, const ZZX& b)
   { ZZX x; div(x, a, b); NTL_OPT_RETURN(ZZX, x); }

inline ZZX operator/(const ZZX& a, const ZZ& b)
   { ZZX x; div(x, a, b); NTL_OPT_RETURN(ZZX, x); }

inline ZZX operator/(const ZZX& a, long b)
   { ZZX x; div(x, a, b); NTL_OPT_RETURN(ZZX, x); }

inline ZZX& operator/=(ZZX& x, const ZZ& b)
   { div(x, x, b); return x; }

inline ZZX& operator/=(ZZX& x, long b)
   { div(x, x, b); return x; }

inline ZZX& operator/=(ZZX& x, const ZZX& b)
   { div(x, x, b); return x; }


inline ZZX operator%(const ZZX& a, const ZZX& b)
   { ZZX x; rem(x, a, b); NTL_OPT_RETURN(ZZX, x); }

inline ZZX& operator%=(ZZX& x, const ZZX& b)
   { rem(x, x, b); return x; }

#endif


// Modular arithemtic---f must be monic, and other args
// must have degree less than that of f

void MulMod(ZZX& x, const ZZX& a, const ZZX& b, const ZZX& f);

inline ZZX MulMod(const ZZX& a, const ZZX& b, const ZZX& f)
   { ZZX x; MulMod(x, a, b, f); NTL_OPT_RETURN(ZZX, x); }

void SqrMod(ZZX& x, const ZZX& a, const ZZX& f);

inline ZZX SqrMod(const ZZX& a, const ZZX& f)
   { ZZX x; SqrMod(x, a, f); NTL_OPT_RETURN(ZZX, x); }

void MulByXMod(ZZX& x, const ZZX& a, const ZZX& f);

inline ZZX MulByXMod(const ZZX& a, const ZZX& f)
   { ZZX x; MulByXMod(x, a, f); NTL_OPT_RETURN(ZZX, x); }


// these always use "plain" division
long PlainDivide(ZZX& q, const ZZX& a, const ZZX& b);
long PlainDivide(const ZZX& a, const ZZX& b);

// these always use "homomorphic imaging"
long HomDivide(ZZX& q, const ZZX& a, const ZZX& b);
long HomDivide(const ZZX& a, const ZZX& b);

long divide(ZZX& q, const ZZX& a, const ZZX& b);
// if b | a, sets q = a/b and returns 1; otherwise returns 0

long divide(const ZZX& a, const ZZX& b);


long divide(ZZX& q, const ZZX& a, const ZZ& b);
// if b | a, sets q = a/b and returns 1; otherwise returns 0

long divide(const ZZX& a, const ZZ& b);
// if b | a, returns 1; otherwise returns 0

//single-precision versions
long divide(ZZX& q, const ZZX& a, long b);
long divide(const ZZX& a, long b);



void content(ZZ& d, const ZZX& f);
// d = content of f, sign(d) == sign(LeadCoeff(f))

inline ZZ content(const ZZX& f)
   { ZZ x; content(x, f); NTL_OPT_RETURN(ZZ, x); }



void PrimitivePart(ZZX& pp, const ZZX& f);
// pp = primitive part of f, LeadCoeff(pp) >= 0

inline ZZX PrimitivePart(const ZZX& f)
   { ZZX x; PrimitivePart(x, f); NTL_OPT_RETURN(ZZX, x); }
   

void GCD(ZZX& d, const ZZX& a, const ZZX& b);
// d = gcd(a, b), LeadCoeff(d) >= 0

inline ZZX GCD(const ZZX& a, const ZZX& b)
   { ZZX x; GCD(x, a, b); NTL_OPT_RETURN(ZZX, x); }

long MaxBits(const ZZX& f);
// returns max NumBits of coefficients of f

long CharPolyBound(const ZZX& a, const ZZX& f);



/***************************************************************

                      traces, norms, resultants

****************************************************************/

void TraceVec(vec_ZZ& S, const ZZX& f);
// S[i] = Trace(X^i mod f), for i = 0..deg(f)-1.
// f must be a monic polynomial.

inline vec_ZZ TraceVec(const ZZX& f)
   { vec_ZZ x; TraceVec(x, f); NTL_OPT_RETURN(vec_ZZ, x); }

void TraceMod(ZZ& res, const ZZX& a, const ZZX& f);
inline ZZ TraceMod(const ZZX& a, const ZZX& f)
   { ZZ x; TraceMod(x, a, f); NTL_OPT_RETURN(ZZ, x); }
// res = trace of (a mod f)
// f must be monic


void resultant(ZZ& res, const ZZX& a, const ZZX& b, long deterministic=0);
inline ZZ resultant(const ZZX& a, const ZZX& b, long deterministic=0)
   { ZZ x; resultant(x, a, b, deterministic); NTL_OPT_RETURN(ZZ, x); }

// res = resultant of a and b
// if !deterministic, then it may use a randomized strategy
//    that errs with probability no more than 2^{-80}.

void NormMod(ZZ& res, const ZZX& a, const ZZX& f, long deterministic=0);
inline ZZ NormMod(const ZZX& a, const ZZX& f, long deterministic=0)
   { ZZ x; NormMod(x, a, f, deterministic); NTL_OPT_RETURN(ZZ, x); }
// res = norm of (a mod f)
// f must be monic
// if !deterministic, then it may use a randomized strategy
//    that errs with probability no more than 2^{-80}.


void discriminant(ZZ& d, const ZZX& a, long deterministic=0);
inline ZZ discriminant(const ZZX& a, long deterministic=0)
   { ZZ x; discriminant(x, a, deterministic); NTL_OPT_RETURN(ZZ, x); }
// d = discriminant of a
//   = (-1)^{m(m-1)/2} resultant(a, a')/lc(a),
//     where m = deg(a)
// if !deterministic, then it may use a randomized strategy
//    that errs with probability no more than 2^{-80}.


void CharPolyMod(ZZX& g, const ZZX& a, const ZZX& f, long deterministic=0);
inline ZZX CharPolyMod(const ZZX& a, const ZZX& f, long deterministic=0)
   { ZZX x; CharPolyMod(x, a, f, deterministic); NTL_OPT_RETURN(ZZX, x); }
// g = char poly of (a mod f)
// f must be monic
// if !deterministic, then it may use a randomized strategy
//    that errs with probability no more than 2^{-80}.


void MinPolyMod(ZZX& g, const ZZX& a, const ZZX& f);
inline ZZX MinPolyMod(const ZZX& a, const ZZX& f)
   { ZZX x; MinPolyMod(x, a, f); NTL_OPT_RETURN(ZZX, x); }
// g = min poly of (a mod f)
// f must be monic
// may use a probabilistic strategy that errs with
//   probability no more than 2^{-80}


void XGCD(ZZ& r, ZZX& s, ZZX& t, const ZZX& a, const ZZX& b, 
          long deterministic=0);
// r = resultant of a and b;
// if r != 0, then computes s and t such that:
//   a*s + b*t = r;
// otherwise s and t not affected.
// if !deterministic, then resultant computation may use a randomized strategy
//    that errs with probability no more than 2^{-80}.



/******************************************************

      Incremental Chinese Remaindering

*******************************************************/

long CRT(ZZX& a, ZZ& prod, const zz_pX& A);
long CRT(ZZX& a, ZZ& prod, const ZZ_pX& A);
// If p is the current modulus with (p, prod) = 1;
// Computes b such that b = a mod prod and b = A mod p,
//    with coefficients in the interval (-p*prod/2, p*prod/2];
// Sets a = b, prod = p*prod, and returns 1 if a's value changed.




typedef Vec<ZZX> vec_ZZX;


NTL_CLOSE_NNS

#endif
