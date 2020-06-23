
#ifndef NTL_zz_pX__H
#define NTL_zz_pX__H

#include <NTL/vector.h>
#include <NTL/lzz_p.h>
#include <NTL/vec_lzz_p.h>
#include <NTL/Lazy.h>
#include <NTL/SmartPtr.h>
#include <NTL/mat_lzz_p.h>

NTL_OPEN_NNS

// some cross-over points

#define NTL_zz_pX_MOD_CROSSOVER (zz_pX_mod_crossover[zz_pInfo->PrimeCnt])
#define NTL_zz_pX_MUL_CROSSOVER (zz_pX_mul_crossover[zz_pInfo->PrimeCnt])
#define NTL_zz_pX_NEWTON_CROSSOVER (zz_pX_newton_crossover[zz_pInfo->PrimeCnt])
#define NTL_zz_pX_DIV_CROSSOVER (zz_pX_div_crossover[zz_pInfo->PrimeCnt])
#define NTL_zz_pX_HalfGCD_CROSSOVER (zz_pX_halfgcd_crossover[zz_pInfo->PrimeCnt])
#define NTL_zz_pX_GCD_CROSSOVER (zz_pX_gcd_crossover[zz_pInfo->PrimeCnt])
#define NTL_zz_pX_BERMASS_CROSSOVER (zz_pX_bermass_crossover[zz_pInfo->PrimeCnt])
#define NTL_zz_pX_TRACE_CROSSOVER (zz_pX_trace_crossover[zz_pInfo->PrimeCnt])

extern const long zz_pX_mod_crossover[];
extern const long zz_pX_mul_crossover[];
extern const long zz_pX_newton_crossover[];
extern const long zz_pX_div_crossover[];
extern const long zz_pX_halfgcd_crossover[];
extern const long zz_pX_gcd_crossover[];
extern const long zz_pX_bermass_crossover[];
extern const long zz_pX_trace_crossover[];



/************************************************************

                         zz_pX

The class zz_pX implements polynomial arithmetic modulo p.
Polynomials are represented as vec_zz_p's.
If f is a zz_pX, then f.rep is a vec_zz_p.
The zero polynomial is represented as a zero length vector.
Otherwise. f.rep[0] is the constant-term, and f.rep[f.rep.length()-1]
is the leading coefficient, which is always non-zero.
The member f.rep is public, so the vector representation is fully
accessible.
Use the member function normalize() to strip leading zeros.

**************************************************************/


class zz_pE; // forward declaration
class zz_pXModulus;
class fftRep;
class zz_pXMultiplier;


class zz_pX {
public:
typedef zz_p coeff_type;
typedef zz_pE residue_type;
typedef zz_pXModulus modulus_type;
typedef zz_pXMultiplier multiplier_type;
typedef fftRep fft_type;


vec_zz_p rep;

typedef vec_zz_p VectorBaseType;



/***************************************************************

          Constructors, Destructors, and Assignment

****************************************************************/


zz_pX() {}
//  initial value 0

explicit zz_pX(long a) { *this = a; }
explicit zz_pX(zz_p a) { *this = a; }


zz_pX(INIT_SIZE_TYPE, long n) { rep.SetMaxLength(n); }


inline zz_pX(long i, zz_p c);
inline zz_pX(long i, long c);

inline zz_pX(INIT_MONO_TYPE, long i, zz_p c);
inline zz_pX(INIT_MONO_TYPE, long i, long c);
inline zz_pX(INIT_MONO_TYPE, long i);


inline zz_pX& operator=(long a);
inline zz_pX& operator=(zz_p a);

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
zz_p& operator[](long i) { return rep[i]; }
const zz_p& operator[](long i) const { return rep[i]; }


void swap(zz_pX& x)
{
   rep.swap(x.rep);
}


static const zz_pX& zero();

zz_pX(zz_pX& x, INIT_TRANS_TYPE) : rep(x.rep, INIT_TRANS) { }

};


NTL_DECLARE_RELOCATABLE((zz_pX*))




/********************************************************************

                           input and output

I/O format:

   [a_0 a_1 ... a_n],

represents the polynomial a_0 + a_1*X + ... + a_n*X^n.

On output, all coefficients will be integers between 0 and p-1,
amd a_n not zero (the zero polynomial is [ ]).
On input, the coefficients are arbitrary integers which are
then reduced modulo p, and leading zeros stripped.

*********************************************************************/


NTL_SNS istream& operator>>(NTL_SNS istream& s, zz_pX& x);
NTL_SNS ostream& operator<<(NTL_SNS ostream& s, const zz_pX& a);




/**********************************************************

                   Some utility routines

***********************************************************/


inline long deg(const zz_pX& a) { return a.rep.length() - 1; }
// degree of a polynomial.
// note that the zero polynomial has degree -1.

const zz_p coeff(const zz_pX& a, long i);
// zero if i not in range

void GetCoeff(zz_p& x, const zz_pX& a, long i);
// x = a[i], or zero if i not in range

const zz_p LeadCoeff(const zz_pX& a);
// zero if a == 0

const zz_p ConstTerm(const zz_pX& a);
// zero if a == 0

void SetCoeff(zz_pX& x, long i, zz_p a);
// x[i] = a, error is raised if i < 0

void SetCoeff(zz_pX& x, long i, long a);
// x[i] = a, error is raised if i < 0

void SetCoeff(zz_pX& x, long i);
// x[i] = 1, error is raised if i < 0

inline zz_pX::zz_pX(long i, zz_p a) { SetCoeff(*this, i, a); }
inline zz_pX::zz_pX(long i, long a) { SetCoeff(*this, i, a); }

inline zz_pX::zz_pX(INIT_MONO_TYPE, long i, zz_p a) { SetCoeff(*this, i, a); }
inline zz_pX::zz_pX(INIT_MONO_TYPE, long i, long a) { SetCoeff(*this, i, a); }
inline zz_pX::zz_pX(INIT_MONO_TYPE, long i) { SetCoeff(*this, i); }

void SetX(zz_pX& x);
// x is set to the monomial X

long IsX(const zz_pX& a);
// test if x = X

inline void clear(zz_pX& x) 
// x = 0

   { x.rep.SetLength(0); }

inline void set(zz_pX& x)
// x = 1

   { x.rep.SetLength(1); set(x.rep[0]); }

inline void swap(zz_pX& x, zz_pX& y)
// swap x & y (only pointers are swapped)

   { x.swap(y); }

void random(zz_pX& x, long n);
inline zz_pX random_zz_pX(long n)
   { zz_pX x; random(x, n); NTL_OPT_RETURN(zz_pX, x); }

// generate a random polynomial of degree < n 

void trunc(zz_pX& x, const zz_pX& a, long m);
// x = a % X^m

inline zz_pX trunc(const zz_pX& a, long m)
   { zz_pX x; trunc(x, a, m); NTL_OPT_RETURN(zz_pX, x); }

void RightShift(zz_pX& x, const zz_pX& a, long n);
// x = a/X^n

inline zz_pX RightShift(const zz_pX& a, long n)
   { zz_pX x; RightShift(x, a, n); NTL_OPT_RETURN(zz_pX, x); }

void LeftShift(zz_pX& x, const zz_pX& a, long n);
// x = a*X^n

inline zz_pX LeftShift(const zz_pX& a, long n)
   { zz_pX x; LeftShift(x, a, n); NTL_OPT_RETURN(zz_pX, x); }


#ifndef NTL_TRANSITION

inline zz_pX operator>>(const zz_pX& a, long n)
   { zz_pX x; RightShift(x, a, n); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX operator<<(const zz_pX& a, long n)
   { zz_pX x; LeftShift(x, a, n); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX& operator<<=(zz_pX& x, long n)
   { LeftShift(x, x, n); return x; }

inline zz_pX& operator>>=(zz_pX& x, long n)
   { RightShift(x, x, n); return x; }

#endif



void diff(zz_pX& x, const zz_pX& a);
// x = derivative of a

inline zz_pX diff(const zz_pX& a)
   { zz_pX x; diff(x, a); NTL_OPT_RETURN(zz_pX, x); }

void MakeMonic(zz_pX& x);
// makes x monic

void reverse(zz_pX& c, const zz_pX& a, long hi);

inline zz_pX reverse(const zz_pX& a, long hi)
   { zz_pX x; reverse(x, a, hi); NTL_OPT_RETURN(zz_pX, x); }

inline void reverse(zz_pX& c, const zz_pX& a)
{  reverse(c, a, deg(a)); }

inline zz_pX reverse(const zz_pX& a)
   { zz_pX x; reverse(x, a); NTL_OPT_RETURN(zz_pX, x); }


inline void VectorCopy(vec_zz_p& x, const zz_pX& a, long n)
   { VectorCopy(x, a.rep, n); }

inline vec_zz_p VectorCopy(const zz_pX& a, long n)
   { return VectorCopy(a.rep, n); }




/*******************************************************************

                        conversion routines

********************************************************************/



void conv(zz_pX& x, long a);

inline zz_pX to_zz_pX(long a)
   { zz_pX x; conv(x, a); NTL_OPT_RETURN(zz_pX, x); }


void conv(zz_pX& x, const ZZ& a);

inline zz_pX to_zz_pX(const ZZ& a)
   { zz_pX x; conv(x, a); NTL_OPT_RETURN(zz_pX, x); }

void conv(zz_pX& x, zz_p a);

inline zz_pX to_zz_pX(zz_p a)
   { zz_pX x; conv(x, a); NTL_OPT_RETURN(zz_pX, x); }


void conv(zz_pX& x, const vec_zz_p& a);

inline zz_pX to_zz_pX(const vec_zz_p& a)
   { zz_pX x; conv(x, a); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX& zz_pX::operator=(zz_p a)
   { conv(*this, a); return *this; }

inline zz_pX& zz_pX::operator=(long a)
   { conv(*this, a); return *this; }


/* additional legacy conversions for v6 conversion regime */

inline void conv(zz_pX& x, const zz_pX& a)
   { x = a; }

inline void conv(vec_zz_p& x, const zz_pX& a)
   { x = a.rep; }


/* ------------------------------------- */



/*************************************************************

                        Comparison

**************************************************************/

long IsZero(const zz_pX& a); 

long IsOne(const zz_pX& a);

inline long operator==(const zz_pX& a, const zz_pX& b)
{
   return a.rep == b.rep;
}

inline long operator!=(const zz_pX& a, const zz_pX& b)
   { return !(a == b); }

long operator==(const zz_pX& a, long b);
long operator==(const zz_pX& a, zz_p b);

inline long operator==(long a, const zz_pX& b) { return b == a; }
inline long operator==(zz_p a, const zz_pX& b) { return b == a; }
   
inline long operator!=(const zz_pX& a, long b) { return !(a == b); }
inline long operator!=(const zz_pX& a, zz_p b) { return !(a == b); }
inline long operator!=(long a, const zz_pX& b) { return !(a == b); }
inline long operator!=(zz_p a, const zz_pX& b) { return !(a == b); }



/***************************************************************

                         Addition

****************************************************************/

void add(zz_pX& x, const zz_pX& a, const zz_pX& b);
// x = a + b

void sub(zz_pX& x, const zz_pX& a, const zz_pX& b);
// x = a - b

void negate(zz_pX& x, const zz_pX& a);
// x = -a

// scalar versions

void add(zz_pX & x, const zz_pX& a, zz_p b); // x = a + b
inline void add(zz_pX& x, const zz_pX& a, long b) { add(x, a, to_zz_p(b)); }

inline void add(zz_pX& x, zz_p a, const zz_pX& b) { add(x, b, a); }
inline void add(zz_pX& x, long a, const zz_pX& b) { add(x, b, a); }

void sub(zz_pX & x, const zz_pX& a, zz_p b); // x = a - b
inline void sub(zz_pX& x, const zz_pX& a, long b) { sub(x, a, to_zz_p(b)); }

void sub(zz_pX& x, zz_p a, const zz_pX& b);
inline void sub(zz_pX& x, long a, const zz_pX& b) { sub(x, to_zz_p(a), b); }

inline zz_pX operator+(const zz_pX& a, const zz_pX& b)
   { zz_pX x; add(x, a, b); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX operator+(const zz_pX& a, zz_p b)
   { zz_pX x; add(x, a, b); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX operator+(const zz_pX& a, long b)
   { zz_pX x; add(x, a, b); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX operator+(zz_p a, const zz_pX& b)
   { zz_pX x; add(x, a, b); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX operator+(long a, const zz_pX& b)
   { zz_pX x; add(x, a, b); NTL_OPT_RETURN(zz_pX, x); }


inline zz_pX operator-(const zz_pX& a, const zz_pX& b)
   { zz_pX x; sub(x, a, b); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX operator-(const zz_pX& a, zz_p b)
   { zz_pX x; sub(x, a, b); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX operator-(const zz_pX& a, long b)
   { zz_pX x; sub(x, a, b); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX operator-(zz_p a, const zz_pX& b)
   { zz_pX x; sub(x, a, b); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX operator-(long a, const zz_pX& b)
   { zz_pX x; sub(x, a, b); NTL_OPT_RETURN(zz_pX, x); }


inline zz_pX& operator+=(zz_pX& x, const zz_pX& b)
   { add(x, x, b); return x; }

inline zz_pX& operator+=(zz_pX& x, zz_p b)
   { add(x, x, b); return x; }

inline zz_pX& operator+=(zz_pX& x, long b)
   { add(x, x, b); return x; }

inline zz_pX& operator-=(zz_pX& x, const zz_pX& b)
   { sub(x, x, b); return x; }

inline zz_pX& operator-=(zz_pX& x, zz_p b)
   { sub(x, x, b); return x; }

inline zz_pX& operator-=(zz_pX& x, long b)
   { sub(x, x, b); return x; }


inline zz_pX operator-(const zz_pX& a)
   { zz_pX x; negate(x, a); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX& operator++(zz_pX& x) { add(x, x, 1); return x; }
inline void operator++(zz_pX& x, int) { add(x, x, 1); }
inline zz_pX& operator--(zz_pX& x) { sub(x, x, 1); return x; }
inline void operator--(zz_pX& x, int) { sub(x, x, 1); }



/*****************************************************************

                        Multiplication

******************************************************************/


void mul(zz_pX& x, const zz_pX& a, const zz_pX& b);
// x = a * b

void sqr(zz_pX& x, const zz_pX& a);
inline zz_pX sqr(const zz_pX& a)
   { zz_pX x; sqr(x, a); NTL_OPT_RETURN(zz_pX, x); }
// x = a^2

void mul(zz_pX& x, const zz_pX& a, zz_p b);
inline void mul(zz_pX& x, const zz_pX& a, long b) { mul(x, a, to_zz_p(b)); }

inline void mul(zz_pX& x, zz_p a, const zz_pX& b) { mul(x, b, a); }
inline void mul(zz_pX& x, long a, const zz_pX& b) { mul(x, b, a); }


inline zz_pX operator*(const zz_pX& a, const zz_pX& b)
   { zz_pX x; mul(x, a, b); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX operator*(const zz_pX& a, zz_p b)
   { zz_pX x; mul(x, a, b); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX operator*(const zz_pX& a, long b)
   { zz_pX x; mul(x, a, b); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX operator*(zz_p a, const zz_pX& b)
   { zz_pX x; mul(x, a, b); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX operator*(long a, const zz_pX& b)
   { zz_pX x; mul(x, a, b); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX& operator*=(zz_pX& x, const zz_pX& b)
   { mul(x, x, b); return x; }

inline zz_pX& operator*=(zz_pX& x, zz_p b)
   { mul(x, x, b); return x; }

inline zz_pX& operator*=(zz_pX& x, long b)
   { mul(x, x, b); return x; }


void PlainMul(zz_pX& x, const zz_pX& a, const zz_pX& b);
// always uses the "classical" algorithm

void PlainSqr(zz_pX& x, const zz_pX& a);
// always uses the "classical" algorithm


void FFTMul(zz_pX& x, const zz_pX& a, const zz_pX& b);
// always uses the FFT

void FFTSqr(zz_pX& x, const zz_pX& a);
// always uses the FFT

void MulTrunc(zz_pX& x, const zz_pX& a, const zz_pX& b, long n);
// x = a * b % X^n

inline zz_pX MulTrunc(const zz_pX& a, const zz_pX& b, long n)
   { zz_pX x; MulTrunc(x, a, b, n); NTL_OPT_RETURN(zz_pX, x); }

void PlainMulTrunc(zz_pX& x, const zz_pX& a, const zz_pX& b, long n);
void FFTMulTrunc(zz_pX& x, const zz_pX& a, const zz_pX& b, long n);

void SqrTrunc(zz_pX& x, const zz_pX& a, long n);
// x = a^2 % X^n

inline zz_pX SqrTrunc(const zz_pX& a, long n)
   { zz_pX x; SqrTrunc(x, a, n); NTL_OPT_RETURN(zz_pX, x); }

void PlainSqrTrunc(zz_pX& x, const zz_pX& a, long n);
void FFTSqrTrunc(zz_pX& x, const zz_pX& a, long n);

void power(zz_pX& x, const zz_pX& a, long e);
inline zz_pX power(const zz_pX& a, long e)
   { zz_pX x; power(x, a, e); NTL_OPT_RETURN(zz_pX, x); }






// The following data structures and routines allow one
// to hand-craft various algorithms, using the FFT convolution
// algorithms directly.
// Look in the file zz_pX.c for examples.




// FFT representation of polynomials

class fftRep {

public:
   long k;                // a 2^k point representation
   long MaxK;             // maximum space allocated
   long len;              // length of truncated FFT
   long NumPrimes;
   UniqueArray<long> tbl[4];

   fftRep() : k(-1), MaxK(-1), len(0), NumPrimes(0) { }

   fftRep(const fftRep& R) : k(-1), MaxK(-1), len(0), NumPrimes(0)
   { *this = R; }

   fftRep(INIT_SIZE_TYPE, long InitK) : k(-1), MaxK(-1), len(0), NumPrimes(0)
   { SetSize(InitK); }

   fftRep& operator=(const fftRep&); 
   void SetSize(long NewK);
   void DoSetSize(long NewK, long NewNumPrimes);
};




void TofftRep_trunc(fftRep& y, const zz_pX& x, long k, long len,
                    long lo, long hi);

inline void TofftRep_trunc(fftRep& y, const zz_pX& x, long k, long len)
{ TofftRep_trunc(y, x, k, len, 0, deg(x)); }

inline
void TofftRep(fftRep& y, const zz_pX& x, long k, long lo, long hi)
// computes an n = 2^k point convolution of x[lo..hi].
{ TofftRep_trunc(y, x, k, 1L << k, lo, hi); }

inline void TofftRep(fftRep& y, const zz_pX& x, long k)

   { TofftRep(y, x, k, 0, deg(x)); }

void RevTofftRep(fftRep& y, const vec_zz_p& x,
                 long k, long lo, long hi, long offset);
// computes an n = 2^k point convolution of X^offset*x[lo..hi] mod X^n-1
// using "inverted" evaluation points.



void FromfftRep(zz_pX& x, fftRep& y, long lo, long hi);
// converts from FFT-representation to coefficient representation
// only the coefficients lo..hi are computed
// NOTE: this version destroys the data in y

// non-destructive versions of the above

void NDFromfftRep(zz_pX& x, const fftRep& y, long lo, long hi, fftRep& temp);
void NDFromfftRep(zz_pX& x, const fftRep& y, long lo, long hi);

void RevFromfftRep(vec_zz_p& x, fftRep& y, long lo, long hi);

   // converts from FFT-representation to coefficient representation
   // using "inverted" evaluation points.
   // only the coefficients lo..hi are computed




void FromfftRep(zz_p* x, fftRep& y, long lo, long hi);
// convert out coefficients lo..hi of y, store result in x.
// no normalization is done.


// direct manipulation of FFT reps

void mul(fftRep& z, const fftRep& x, const fftRep& y);
void sub(fftRep& z, const fftRep& x, const fftRep& y);
void add(fftRep& z, const fftRep& x, const fftRep& y);

void reduce(fftRep& x, const fftRep& a, long k);
// reduces a 2^l point FFT-rep to a 2^k point FFT-rep

void AddExpand(fftRep& x, const fftRep& a);
//  x = x + (an "expanded" version of a)







/*************************************************************

                      Division

**************************************************************/

void DivRem(zz_pX& q, zz_pX& r, const zz_pX& a, const zz_pX& b);
// q = a/b, r = a%b

void div(zz_pX& q, const zz_pX& a, const zz_pX& b);
// q = a/b


void div(zz_pX& q, const zz_pX& a, zz_p b);
inline void div(zz_pX& q, const zz_pX& a, long b)
   { div(q, a, to_zz_p(b)); }

void rem(zz_pX& r, const zz_pX& a, const zz_pX& b);
// r = a%b

long divide(zz_pX& q, const zz_pX& a, const zz_pX& b);
// if b | a, sets q = a/b and returns 1; otherwise returns 0

long divide(const zz_pX& a, const zz_pX& b);
// if b | a, sets q = a/b and returns 1; otherwise returns 0


void InvTrunc(zz_pX& x, const zz_pX& a, long m);
// computes x = a^{-1} % X^m 
// constant term must be non-zero

inline zz_pX InvTrunc(const zz_pX& a, long m)
   { zz_pX x; InvTrunc(x, a, m); NTL_OPT_RETURN(zz_pX, x); }




// These always use "classical" arithmetic
void PlainDivRem(zz_pX& q, zz_pX& r, const zz_pX& a, const zz_pX& b);
void PlainDiv(zz_pX& q, const zz_pX& a, const zz_pX& b);
void PlainRem(zz_pX& r, const zz_pX& a, const zz_pX& b);


// These always use FFT arithmetic
void FFTDivRem(zz_pX& q, zz_pX& r, const zz_pX& a, const zz_pX& b);
void FFTDiv(zz_pX& q, const zz_pX& a, const zz_pX& b);
void FFTRem(zz_pX& r, const zz_pX& a, const zz_pX& b);

void PlainInvTrunc(zz_pX& x, const zz_pX& a, long m);
// always uses "classical" algorithm
// ALIAS RESTRICTION: input may not alias output

void NewtonInvTrunc(zz_pX& x, const zz_pX& a, long m);
// uses a Newton Iteration with the FFT.
// ALIAS RESTRICTION: input may not alias output


inline zz_pX operator/(const zz_pX& a, const zz_pX& b)
   { zz_pX x; div(x, a, b); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX operator/(const zz_pX& a, zz_p b)
   { zz_pX x; div(x, a, b); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX operator/(const zz_pX& a, long b)
   { zz_pX x; div(x, a, b); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX& operator/=(zz_pX& x, zz_p b)
   { div(x, x, b); return x; }

inline zz_pX& operator/=(zz_pX& x, long b)
   { div(x, x, b); return x; }

inline zz_pX& operator/=(zz_pX& x, const zz_pX& b)
   { div(x, x, b); return x; }


inline zz_pX operator%(const zz_pX& a, const zz_pX& b)
   { zz_pX x; rem(x, a, b); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX& operator%=(zz_pX& x, const zz_pX& b)
   { rem(x, x, b); return x; }




/***********************************************************

                         GCD's

************************************************************/


void GCD(zz_pX& x, const zz_pX& a, const zz_pX& b);
// x = GCD(a, b),  x is always monic (or zero if a==b==0).

inline zz_pX GCD(const zz_pX& a, const zz_pX& b)
   { zz_pX x; GCD(x, a, b); NTL_OPT_RETURN(zz_pX, x); }


void XGCD(zz_pX& d, zz_pX& s, zz_pX& t, const zz_pX& a, const zz_pX& b);
// d = gcd(a,b), a s + b t = d 

void PlainXGCD(zz_pX& d, zz_pX& s, zz_pX& t, const zz_pX& a, const zz_pX& b);
// same as above, but uses classical algorithm


void PlainGCD(zz_pX& x, const zz_pX& a, const zz_pX& b);
// always uses "cdlassical" arithmetic


class zz_pXMatrix {
private:

   zz_pXMatrix(const zz_pXMatrix&);  // disable
   zz_pX elts[2][2];

public:

   zz_pXMatrix() { }

   void operator=(const zz_pXMatrix&);
   zz_pX& operator() (long i, long j) { return elts[i][j]; }
   const zz_pX& operator() (long i, long j) const { return elts[i][j]; }
};


void HalfGCD(zz_pXMatrix& M_out, const zz_pX& U, const zz_pX& V, long d_red);
// deg(U) > deg(V),   1 <= d_red <= deg(U)+1.
//
// This computes a 2 x 2 polynomial matrix M_out such that
//    M_out * (U, V)^T = (U', V')^T,
// where U', V' are consecutive polynomials in the Euclidean remainder
// sequence of U, V, and V' is the polynomial of highest degree
// satisfying deg(V') <= deg(U) - d_red.

void XHalfGCD(zz_pXMatrix& M_out, zz_pX& U, zz_pX& V, long d_red);

// same as above, except that U is replaced by U', and V by V'


/*************************************************************

             Modular Arithmetic without pre-conditioning

**************************************************************/

// arithmetic mod f.
// all inputs and outputs are polynomials of degree less than deg(f).
// ASSUMPTION: f is assumed monic, and deg(f) > 0.
// NOTE: if you want to do many computations with a fixed f,
//       use the zz_pXModulus data structure and associated routines below.



void MulMod(zz_pX& x, const zz_pX& a, const zz_pX& b, const zz_pX& f);
// x = (a * b) % f

inline zz_pX MulMod(const zz_pX& a, const zz_pX& b, const zz_pX& f)
   { zz_pX x; MulMod(x, a, b, f); NTL_OPT_RETURN(zz_pX, x); }

void SqrMod(zz_pX& x, const zz_pX& a, const zz_pX& f);
// x = a^2 % f

inline zz_pX SqrMod(const zz_pX& a, const zz_pX& f)
   { zz_pX x; SqrMod(x, a, f); NTL_OPT_RETURN(zz_pX, x); }

void MulByXMod(zz_pX& x, const zz_pX& a, const zz_pX& f);
// x = (a * X) mod f

inline zz_pX MulByXMod(const zz_pX& a, const zz_pX& f)
   { zz_pX x; MulByXMod(x, a, f); NTL_OPT_RETURN(zz_pX, x); }

void InvMod(zz_pX& x, const zz_pX& a, const zz_pX& f);
// x = a^{-1} % f, error is a is not invertible

inline zz_pX InvMod(const zz_pX& a, const zz_pX& f)
   { zz_pX x; InvMod(x, a, f); NTL_OPT_RETURN(zz_pX, x); }

long InvModStatus(zz_pX& x, const zz_pX& a, const zz_pX& f);
// if (a, f) = 1, returns 0 and sets x = a^{-1} % f
// otherwise, returns 1 and sets x = (a, f)



/******************************************************************

        Modular Arithmetic with Pre-conditioning

*******************************************************************/


// If you need to do a lot of arithmetic modulo a fixed f,
// build zz_pXModulus F for f.  This pre-computes information about f
// that speeds up the computation a great deal.


class zz_pXModulus {
public:
   zz_pXModulus() : UseFFT(0), n(-1)  { }

   zz_pX f;   // the modulus
   long UseFFT;// flag indicating whether FFT should be used.
   long n;     // n = deg(f)
   long k;     // least k s/t 2^k >= n
   long l;     // least l s/t 2^l >= 2n-3
   fftRep FRep; // 2^k point rep of f
                // H = rev((rev(f))^{-1} rem X^{n-1})
   fftRep HRep; // 2^l point rep of H

   OptionalVal< Lazy<vec_zz_p> > tracevec;  
   // extra level of indirection to ensure relocatability

   zz_pXModulus(const zz_pX& ff);

   operator const zz_pX& () const { return f; }
   const zz_pX& val() const { return f; }

};


NTL_DECLARE_RELOCATABLE((zz_pXModulus*))


inline long deg(const zz_pXModulus& F) { return F.n; }

void build(zz_pXModulus& F, const zz_pX& f);
// deg(f) > 0


void rem21(zz_pX& x, const zz_pX& a, const zz_pXModulus& F);
// x = a % f
// deg(a) <= 2(n-1), where n = F.n = deg(f)

void rem(zz_pX& x, const zz_pX& a, const zz_pXModulus& F);
// x = a % f, no restrictions on deg(a);  makes repeated calls to rem21

inline zz_pX operator%(const zz_pX& a, const zz_pXModulus& F)
   { zz_pX x; rem(x, a, F); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX& operator%=(zz_pX& x, const zz_pXModulus& F)
   { rem(x, x, F); return x; }


void DivRem(zz_pX& q, zz_pX& r, const zz_pX& a, const zz_pXModulus& F);

void div(zz_pX& q, const zz_pX& a, const zz_pXModulus& F);

inline zz_pX operator/(const zz_pX& a, const zz_pXModulus& F)
   { zz_pX x; div(x, a, F); NTL_OPT_RETURN(zz_pX, x); }

inline zz_pX& operator/=(zz_pX& x, const zz_pXModulus& F)
   { div(x, x, F); return x; }



void MulMod(zz_pX& x, const zz_pX& a, const zz_pX& b, const zz_pXModulus& F);
// x = (a * b) % f
// deg(a), deg(b) < n

inline zz_pX MulMod(const zz_pX& a, const zz_pX& b, const zz_pXModulus& F)
   { zz_pX x; MulMod(x, a, b, F); NTL_OPT_RETURN(zz_pX, x); }


void SqrMod(zz_pX& x, const zz_pX& a, const zz_pXModulus& F);
// x = a^2 % f
// deg(a) < n


inline zz_pX SqrMod(const zz_pX& a, const zz_pXModulus& F)
   { zz_pX x; SqrMod(x, a, F); NTL_OPT_RETURN(zz_pX, x); }


void PowerMod(zz_pX& x, const zz_pX& a, const ZZ& e, const zz_pXModulus& F);
// x = a^e % f, e >= 0

inline zz_pX PowerMod(const zz_pX& a, const ZZ& e, const zz_pXModulus& F)
   { zz_pX x; PowerMod(x, a, e, F); NTL_OPT_RETURN(zz_pX, x); }

inline void PowerMod(zz_pX& x, const zz_pX& a, long e, const zz_pXModulus& F)
   { PowerMod(x, a, ZZ_expo(e), F); }

inline zz_pX PowerMod(const zz_pX& a, long e, const zz_pXModulus& F)
   { zz_pX x; PowerMod(x, a, e, F); NTL_OPT_RETURN(zz_pX, x); }



void PowerXMod(zz_pX& x, const ZZ& e, const zz_pXModulus& F);
// x = X^e % f, e >= 0

inline zz_pX PowerXMod(const ZZ& e, const zz_pXModulus& F)
   { zz_pX x; PowerXMod(x, e, F); NTL_OPT_RETURN(zz_pX, x); }

inline void PowerXMod(zz_pX& x, long e, const zz_pXModulus& F)
   { PowerXMod(x, ZZ_expo(e), F); }

inline zz_pX PowerXMod(long e, const zz_pXModulus& F)
   { zz_pX x; PowerXMod(x, e, F); NTL_OPT_RETURN(zz_pX, x); }

void PowerXPlusAMod(zz_pX& x, zz_p a, const ZZ& e, const zz_pXModulus& F);
// x = (X + a)^e % f, e >= 0

inline zz_pX PowerXPlusAMod(zz_p a, const ZZ& e, const zz_pXModulus& F)
   { zz_pX x; PowerXPlusAMod(x, a, e, F); NTL_OPT_RETURN(zz_pX, x); }

inline void PowerXPlusAMod(zz_pX& x, zz_p a, long e, const zz_pXModulus& F)
   { PowerXPlusAMod(x, a, ZZ_expo(e), F); }


inline zz_pX PowerXPlusAMod(zz_p a, long e, const zz_pXModulus& F)
   { zz_pX x; PowerXPlusAMod(x, a, e, F); NTL_OPT_RETURN(zz_pX, x); }

// If you need to compute a * b % f for a fixed b, but for many a's
// (for example, computing powers of b modulo f), it is
// much more efficient to first build a zz_pXMultiplier B for b,
// and then use the routine below.

class zz_pXMultiplier {
public:
   zz_pXMultiplier() : UseFFT(0)  { }
   zz_pXMultiplier(const zz_pX& b, const zz_pXModulus& F);


   zz_pX b;   
   long UseFFT;
   fftRep B1; 
   fftRep B2; 

   const zz_pX& val() const { return b; }

};

void build(zz_pXMultiplier& B, const zz_pX& b, const zz_pXModulus& F);



void MulMod(zz_pX& x, const zz_pX& a, const zz_pXMultiplier& B,
                                      const zz_pXModulus& F);

// x = (a * b) % f

inline zz_pX MulMod(const zz_pX& a, const zz_pXMultiplier& B,
                                          const zz_pXModulus& F)
   { zz_pX x; MulMod(x, a, B, F); NTL_OPT_RETURN(zz_pX, x); }




/*******************************************************

              Evaluation and related problems

********************************************************/


void BuildFromRoots(zz_pX& x, const vec_zz_p& a);
// computes the polynomial (X-a[0]) ... (X-a[n-1]), where n = a.length()

inline zz_pX BuildFromRoots(const vec_zz_p& a)
   { zz_pX x; BuildFromRoots(x, a); NTL_OPT_RETURN(zz_pX, x); }



void eval(zz_p& b, const zz_pX& f, zz_p a);
// b = f(a)


inline zz_p eval(const zz_pX& f, zz_p a)
   { zz_p x; eval(x, f, a); return x; }


void eval(vec_zz_p& b, const zz_pX& f, const vec_zz_p& a);
//  b[i] = f(a[i])

inline vec_zz_p eval(const zz_pX& f, const vec_zz_p& a)
   { vec_zz_p x; eval(x, f, a); NTL_OPT_RETURN(vec_zz_p, x); }


void interpolate(zz_pX& f, const vec_zz_p& a, const vec_zz_p& b);
// computes f such that f(a[i]) = b[i]

inline zz_pX interpolate(const vec_zz_p& a, const vec_zz_p& b)
   { zz_pX x; interpolate(x, a, b); NTL_OPT_RETURN(zz_pX, x); }



/*****************************************************************

                       vectors of zz_pX's

*****************************************************************/

typedef Vec<zz_pX> vec_zz_pX;


/**********************************************************

         Modular Composition and Minimal Polynomials

***********************************************************/


// algorithms for computing g(h) mod f




void CompMod(zz_pX& x, const zz_pX& g, const zz_pX& h, const zz_pXModulus& F);
// x = g(h) mod f

inline zz_pX CompMod(const zz_pX& g, const zz_pX& h,
                           const zz_pXModulus& F)
   { zz_pX x; CompMod(x, g, h, F); NTL_OPT_RETURN(zz_pX, x); }


void Comp2Mod(zz_pX& x1, zz_pX& x2, const zz_pX& g1, const zz_pX& g2,
              const zz_pX& h, const zz_pXModulus& F);
// xi = gi(h) mod f (i=1,2)

void Comp3Mod(zz_pX& x1, zz_pX& x2, zz_pX& x3, 
              const zz_pX& g1, const zz_pX& g2, const zz_pX& g3,
              const zz_pX& h, const zz_pXModulus& F);
// xi = gi(h) mod f (i=1..3)



// The routine build (see below) which is implicitly called
// by the various compose and UpdateMap routines builds a table
// of polynomials.  
// If zz_pXArgBound > 0, then the table is limited in
// size to approximamtely that many KB.
// If zz_pXArgBound <= 0, then it is ignored, and space is allocated
// so as to maximize speed.
// Initially, zz_pXArgBound = 0.


// If a single h is going to be used with many g's
// then you should build a zz_pXArgument for h,
// and then use the compose routine below.
// build computes and stores h, h^2, ..., h^m mod f.
// After this pre-computation, composing a polynomial of degree 
// roughly n with h takes n/m multiplies mod f, plus n^2
// scalar multiplies.
// Thus, increasing m increases the space requirement and the pre-computation
// time, but reduces the composition time.
// If zz_pXArgBound > 0, a table of size less than m may be built.

struct zz_pXArgument {
   vec_zz_pX H;
};

extern 
NTL_CHEAP_THREAD_LOCAL 
long zz_pXArgBound;


void build(zz_pXArgument& H, const zz_pX& h, const zz_pXModulus& F, long m);

// m must be > 0, otherwise an error is raised

void CompMod(zz_pX& x, const zz_pX& g, const zz_pXArgument& H, 
             const zz_pXModulus& F);

inline zz_pX
CompMod(const zz_pX& g, const zz_pXArgument& H, const zz_pXModulus& F)
   { zz_pX x; CompMod(x, g, H, F); NTL_OPT_RETURN(zz_pX, x); }


// New alternative CompMod strategy that just reduces to 
// matrix multiplication ... 


struct zz_pXNewArgument {
   Mat<zz_p> mat;
   zz_pX     poly;
};

void build(zz_pXNewArgument& H, const zz_pX& h, const zz_pXModulus& F, long m);
void CompMod(zz_pX& x, const zz_pX& g, const zz_pXNewArgument& H,
             const zz_pXModulus& F);
void reduce(zz_pXNewArgument& H, const zz_pXModulus& F);

void ProjectPowers(vec_zz_p& x, const vec_zz_p& a, long k,
                   const zz_pXNewArgument& H, const zz_pXModulus& F);







#ifndef NTL_TRANSITION

void UpdateMap(vec_zz_p& x, const vec_zz_p& a,
               const zz_pXMultiplier& B, const zz_pXModulus& F);

inline vec_zz_p
UpdateMap(const vec_zz_p& a,
          const zz_pXMultiplier& B, const zz_pXModulus& F)
   { vec_zz_p x; UpdateMap(x, a, B, F);
     NTL_OPT_RETURN(vec_zz_p, x); }

#endif


/* computes (a, b), (a, (b*X)%f), ..., (a, (b*X^{n-1})%f),
   where ( , ) denotes the vector inner product.

   This is really a "transposed" MulMod by B.
*/

void PlainUpdateMap(vec_zz_p& x, const vec_zz_p& a,
                    const zz_pX& b, const zz_pX& f);


// same as above, but uses only classical arithmetic


void ProjectPowers(vec_zz_p& x, const vec_zz_p& a, long k,
                   const zz_pX& h, const zz_pXModulus& F);

// computes (a, 1), (a, h), ..., (a, h^{k-1} % f)
// this is really a "transposed" compose.

inline vec_zz_p ProjectPowers(const vec_zz_p& a, long k,
                   const zz_pX& h, const zz_pXModulus& F)
{
   vec_zz_p x;
   ProjectPowers(x, a, k, h, F);
   NTL_OPT_RETURN(vec_zz_p, x);
}


void ProjectPowers(vec_zz_p& x, const vec_zz_p& a, long k,
                   const zz_pXArgument& H, const zz_pXModulus& F);

inline vec_zz_p ProjectPowers(const vec_zz_p& a, long k,
                   const zz_pXArgument& H, const zz_pXModulus& F)
{
   vec_zz_p x;
   ProjectPowers(x, a, k, H, F);
   NTL_OPT_RETURN(vec_zz_p, x);
}

// same as above, but uses a pre-computed zz_pXArgument

inline void project(zz_p& x, const vec_zz_p& a, const zz_pX& b)
   { InnerProduct(x, a, b.rep); }

inline zz_p project(const vec_zz_p& a, const zz_pX& b)
   {  zz_p x; project(x, a, b); return x; }


void MinPolySeq(zz_pX& h, const vec_zz_p& a, long m);
// computes the minimum polynomial of a linealy generated sequence;
// m is a bound on the degree of the polynomial;
// required: a.length() >= 2*m

inline zz_pX MinPolySeq(const vec_zz_p& a, long m)
   { zz_pX x; MinPolySeq(x, a, m); NTL_OPT_RETURN(zz_pX, x); }

void ProbMinPolyMod(zz_pX& h, const zz_pX& g, const zz_pXModulus& F, long m);

inline zz_pX ProbMinPolyMod(const zz_pX& g, const zz_pXModulus& F, long m)
   { zz_pX x; ProbMinPolyMod(x, g, F, m); NTL_OPT_RETURN(zz_pX, x); }


inline void ProbMinPolyMod(zz_pX& h, const zz_pX& g, const zz_pXModulus& F)
   { ProbMinPolyMod(h, g, F, F.n); }

inline zz_pX ProbMinPolyMod(const zz_pX& g, const zz_pXModulus& F)
   { zz_pX x; ProbMinPolyMod(x, g, F); NTL_OPT_RETURN(zz_pX, x); }


// computes the monic minimal polynomial if (g mod f). 
// m = a bound on the degree of the minimal polynomial.
// If this argument is not supplied, it defaults to deg(f).
// The algorithm is probabilistic, always returns a divisor of
// the minimal polynomial, and returns a proper divisor with
// probability at most m/p.

void MinPolyMod(zz_pX& h, const zz_pX& g, const zz_pXModulus& F, long m);

inline zz_pX MinPolyMod(const zz_pX& g, const zz_pXModulus& F, long m)
   { zz_pX x; MinPolyMod(x, g, F, m); NTL_OPT_RETURN(zz_pX, x); }


inline void MinPolyMod(zz_pX& h, const zz_pX& g, const zz_pXModulus& F)
   { MinPolyMod(h, g, F, F.n); }

inline zz_pX MinPolyMod(const zz_pX& g, const zz_pXModulus& F)
   { zz_pX x; MinPolyMod(x, g, F); NTL_OPT_RETURN(zz_pX, x); }


// same as above, but guarantees that result is correct

void IrredPolyMod(zz_pX& h, const zz_pX& g, const zz_pXModulus& F, long m);

inline zz_pX IrredPolyMod(const zz_pX& g, const zz_pXModulus& F, long m)
   { zz_pX x; IrredPolyMod(x, g, F, m); NTL_OPT_RETURN(zz_pX, x); }


inline void IrredPolyMod(zz_pX& h, const zz_pX& g, const zz_pXModulus& F)
   { IrredPolyMod(h, g, F, F.n); }

inline zz_pX IrredPolyMod(const zz_pX& g, const zz_pXModulus& F)
   { zz_pX x; IrredPolyMod(x, g, F); NTL_OPT_RETURN(zz_pX, x); }


// same as above, but assumes that f is irreducible, 
// or at least that the minimal poly of g is itself irreducible.
// The algorithm is deterministic (and is always correct).



/*****************************************************************

                   Traces, norms, resultants

******************************************************************/

void TraceVec(vec_zz_p& S, const zz_pX& f);

inline vec_zz_p TraceVec(const zz_pX& f)
   { vec_zz_p x; TraceVec(x, f); NTL_OPT_RETURN(vec_zz_p, x); }


void FastTraceVec(vec_zz_p& S, const zz_pX& f);
void PlainTraceVec(vec_zz_p& S, const zz_pX& f);

void TraceMod(zz_p& x, const zz_pX& a, const zz_pXModulus& F);

inline zz_p TraceMod(const zz_pX& a, const zz_pXModulus& F)
   { zz_p x; TraceMod(x, a, F); return x; }

void TraceMod(zz_p& x, const zz_pX& a, const zz_pX& f);

inline zz_p TraceMod(const zz_pX& a, const zz_pX& f)
   { zz_p x; TraceMod(x, a, f); return x; }




void ComputeTraceVec(const zz_pXModulus& F);

void NormMod(zz_p& x, const zz_pX& a, const zz_pX& f);


inline zz_p NormMod(const zz_pX& a, const zz_pX& f)
   { zz_p x; NormMod(x, a, f); return x; }

void resultant(zz_p& rres, const zz_pX& a, const zz_pX& b);

inline zz_p resultant(const zz_pX& a, const zz_pX& b)
   { zz_p x; resultant(x, a, b); return x; }

void CharPolyMod(zz_pX& g, const zz_pX& a, const zz_pX& f);
// g = char poly of (a mod f)
// only implemented for p >= deg(f)+1

inline zz_pX CharPolyMod(const zz_pX& a, const zz_pX& f)
   { zz_pX x; CharPolyMod(x, a, f); NTL_OPT_RETURN(zz_pX, x); }





NTL_CLOSE_NNS

#endif
