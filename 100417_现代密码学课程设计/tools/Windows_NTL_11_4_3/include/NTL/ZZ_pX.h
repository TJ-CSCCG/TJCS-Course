

#ifndef NTL_ZZ_pX__H
#define NTL_ZZ_pX__H

#include <NTL/vector.h>
#include <NTL/ZZ_p.h>
#include <NTL/vec_ZZ.h>
#include <NTL/vec_ZZ_p.h>
#include <NTL/FFT.h>
#include <NTL/Lazy.h>
#include <NTL/SmartPtr.h>

#ifndef NTL_WIZARD_HACK
#include <NTL/mat_ZZ_p.h>
#endif


NTL_OPEN_NNS




// some cross-over points
// macros are used so as to be consistent with zz_pX 

#define NTL_ZZ_pX_FFT_CROSSOVER (20)  
#define NTL_ZZ_pX_NEWTON_CROSSOVER (45)
#define NTL_ZZ_pX_DIV_CROSSOVER (90)
#define NTL_ZZ_pX_HalfGCD_CROSSOVER (25)
#define NTL_ZZ_pX_GCD_CROSSOVER (180)
#define NTL_ZZ_pX_BERMASS_CROSSOVER (90)
#define NTL_ZZ_pX_TRACE_CROSSOVER (90)



/************************************************************

                         ZZ_pX

The class ZZ_pX implements polynomial arithmetic modulo p.
Polynomials are represented as vec_ZZ_p's.
If f is a ZZ_pX, then f.rep is a vec_ZZ_p.
The zero polynomial is represented as a zero length vector.
Otherwise. f.rep[0] is the constant-term, and f.rep[f.rep.length()-1]
is the leading coefficient, which is always non-zero.
The member f.rep is public, so the vector representation is fully
accessible.
Use the member function normalize() to strip leading zeros.

**************************************************************/

class ZZ_pE; // forward declaration
class ZZ_pXModulus;
class FFTRep;
class ZZ_pXMultiplier;

class ZZ_pX {
public:
typedef ZZ_p coeff_type;
typedef ZZ_pE residue_type;
typedef ZZ_pXModulus modulus_type;
typedef ZZ_pXMultiplier multiplier_type;
typedef FFTRep fft_type;

typedef vec_ZZ_p VectorBaseType; 


vec_ZZ_p rep;


/***************************************************************

          Constructors, Destructors, and Assignment

****************************************************************/


ZZ_pX() { }
//  initial value 0


explicit ZZ_pX(long a) { *this = a; }
explicit ZZ_pX(const ZZ_p& a) { *this = a; }


ZZ_pX(INIT_SIZE_TYPE, long n) { rep.SetMaxLength(n); }

// default copy constrctor and assignment

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
ZZ_p& operator[](long i) { return rep[i]; }
const ZZ_p& operator[](long i) const { return rep[i]; }


static const ZZ_pX& zero();


ZZ_pX(ZZ_pX& x, INIT_TRANS_TYPE) : rep(x.rep, INIT_TRANS) { }

inline ZZ_pX(long i, const ZZ_p& c);
inline ZZ_pX(long i, long c);

inline ZZ_pX(INIT_MONO_TYPE, long i, const ZZ_p& c);
inline ZZ_pX(INIT_MONO_TYPE, long i, long c);
inline ZZ_pX(INIT_MONO_TYPE, long i);

ZZ_pX& operator=(long a);
ZZ_pX& operator=(const ZZ_p& a);


void swap(ZZ_pX& x)
{
   rep.swap(x.rep);
}


};



NTL_DECLARE_RELOCATABLE((ZZ_pX*))




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


NTL_SNS istream& operator>>(NTL_SNS istream& s, ZZ_pX& x);
NTL_SNS ostream& operator<<(NTL_SNS ostream& s, const ZZ_pX& a);




/**********************************************************

                   Some utility routines

***********************************************************/


inline long deg(const ZZ_pX& a) { return a.rep.length() - 1; }
// degree of a polynomial.
// note that the zero polynomial has degree -1.

const ZZ_p& coeff(const ZZ_pX& a, long i);
// zero if i not in range

void GetCoeff(ZZ_p& x, const ZZ_pX& a, long i);
// x = a[i], or zero if i not in range

const ZZ_p& LeadCoeff(const ZZ_pX& a);
// zero if a == 0

const ZZ_p& ConstTerm(const ZZ_pX& a);
// zero if a == 0

void SetCoeff(ZZ_pX& x, long i, const ZZ_p& a);
// x[i] = a, error is raised if i < 0

void SetCoeff(ZZ_pX& x, long i, long a);

void SetCoeff(ZZ_pX& x, long i);
// x[i] = 1, error is raised if i < 0

inline ZZ_pX::ZZ_pX(long i, const ZZ_p& a) { SetCoeff(*this, i, a); } 
inline ZZ_pX::ZZ_pX(long i, long a) { SetCoeff(*this, i, a); } 

inline ZZ_pX::ZZ_pX(INIT_MONO_TYPE, long i, const ZZ_p& a) { SetCoeff(*this, i, a); } 
inline ZZ_pX::ZZ_pX(INIT_MONO_TYPE, long i, long a) { SetCoeff(*this, i, a); } 
inline ZZ_pX::ZZ_pX(INIT_MONO_TYPE, long i) { SetCoeff(*this, i); } 

void SetX(ZZ_pX& x);
// x is set to the monomial X

long IsX(const ZZ_pX& a);
// test if a = X

inline void clear(ZZ_pX& x) 
// x = 0

   { x.rep.SetLength(0); }

inline void set(ZZ_pX& x)
// x = 1

   { x.rep.SetLength(1); set(x.rep[0]); }

inline void swap(ZZ_pX& x, ZZ_pX& y)
// swap x & y (only pointers are swapped)

   { x.swap(y); }

void random(ZZ_pX& x, long n);
inline ZZ_pX random_ZZ_pX(long n)
   { ZZ_pX x; random(x, n); NTL_OPT_RETURN(ZZ_pX, x); }
// generate a random polynomial of degree < n 

void trunc(ZZ_pX& x, const ZZ_pX& a, long m);
// x = a % X^m

inline ZZ_pX trunc(const ZZ_pX& a, long m)
   { ZZ_pX x; trunc(x, a, m); NTL_OPT_RETURN(ZZ_pX, x); }

void RightShift(ZZ_pX& x, const ZZ_pX& a, long n);
// x = a/X^n

inline ZZ_pX RightShift(const ZZ_pX& a, long n)
   { ZZ_pX x; RightShift(x, a, n); NTL_OPT_RETURN(ZZ_pX, x); }

void LeftShift(ZZ_pX& x, const ZZ_pX& a, long n);
// x = a*X^n

inline ZZ_pX LeftShift(const ZZ_pX& a, long n)
   { ZZ_pX x; LeftShift(x, a, n); NTL_OPT_RETURN(ZZ_pX, x); }

#ifndef NTL_TRANSITION

inline ZZ_pX operator>>(const ZZ_pX& a, long n)
   { ZZ_pX x; RightShift(x, a, n); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX operator<<(const ZZ_pX& a, long n)
   { ZZ_pX x; LeftShift(x, a, n); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX& operator<<=(ZZ_pX& x, long n)
   { LeftShift(x, x, n); return x; }

inline ZZ_pX& operator>>=(ZZ_pX& x, long n)
   { RightShift(x, x, n); return x; }

#endif



void diff(ZZ_pX& x, const ZZ_pX& a);
// x = derivative of a

inline ZZ_pX diff(const ZZ_pX& a)
   { ZZ_pX x; diff(x, a); NTL_OPT_RETURN(ZZ_pX, x); }


void MakeMonic(ZZ_pX& x);

void reverse(ZZ_pX& c, const ZZ_pX& a, long hi);

inline ZZ_pX reverse(const ZZ_pX& a, long hi)
   { ZZ_pX x; reverse(x, a, hi); NTL_OPT_RETURN(ZZ_pX, x); }

inline void reverse(ZZ_pX& c, const ZZ_pX& a)
{  reverse(c, a, deg(a)); }

inline ZZ_pX reverse(const ZZ_pX& a)
   { ZZ_pX x; reverse(x, a); NTL_OPT_RETURN(ZZ_pX, x); }

inline void VectorCopy(vec_ZZ_p& x, const ZZ_pX& a, long n)
   { VectorCopy(x, a.rep, n); }

inline vec_ZZ_p VectorCopy(const ZZ_pX& a, long n)
   { return VectorCopy(a.rep, n); }




/*******************************************************************

                        conversion routines

********************************************************************/



void conv(ZZ_pX& x, long a);
void conv(ZZ_pX& x, const ZZ& a);
void conv(ZZ_pX& x, const ZZ_p& a);
void conv(ZZ_pX& x, const vec_ZZ_p& a);

inline ZZ_pX to_ZZ_pX(long a)
   { ZZ_pX x; conv(x, a); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX to_ZZ_pX(const ZZ& a)
   { ZZ_pX x; conv(x, a); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX to_ZZ_pX(const ZZ_p& a)
   { ZZ_pX x; conv(x, a); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX to_ZZ_pX(const vec_ZZ_p& a)
   { ZZ_pX x; conv(x, a); NTL_OPT_RETURN(ZZ_pX, x); }





/* additional legacy conversions for v6 conversion regime */

inline void conv(ZZ_pX& x, const ZZ_pX& a)
   { x = a; }

inline void conv(vec_ZZ_p& x, const ZZ_pX& a)
   { x = a.rep; }


/* ------------------------------------- */



/*************************************************************

                        Comparison

**************************************************************/

long IsZero(const ZZ_pX& a); 

long IsOne(const ZZ_pX& a);

inline long operator==(const ZZ_pX& a, const ZZ_pX& b)
{
   return a.rep == b.rep;
}

inline long operator!=(const ZZ_pX& a, const ZZ_pX& b)
{
   return !(a == b);
}

long operator==(const ZZ_pX& a, long b);
long operator==(const ZZ_pX& a, const ZZ_p& b);

inline long operator==(long a, const ZZ_pX& b) { return b == a; }
inline long operator==(const ZZ_p& a, const ZZ_pX& b) { return b == a; }

inline long operator!=(const ZZ_pX& a, long b) { return !(a == b); }
inline long operator!=(const ZZ_pX& a, const ZZ_p& b) { return !(a == b); }

inline long operator!=(long a, const ZZ_pX& b) { return !(a == b); }
inline long operator!=(const ZZ_p& a, const ZZ_pX& b) { return !(a == b); }


/***************************************************************

                         Addition

****************************************************************/

void add(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b);
// x = a + b

void sub(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b);
// x = a - b

void negate(ZZ_pX& x, const ZZ_pX& a);
// x = -a

// scalar versions

void add(ZZ_pX& x, const ZZ_pX& a, const ZZ_p& b); // x = a + b
void add(ZZ_pX& x, const ZZ_pX& a, long b);

inline void add(ZZ_pX& x, const ZZ_p& a, const ZZ_pX& b) { add(x, b, a); }
inline void add(ZZ_pX& x, long a, const ZZ_pX& b) { add(x, b, a); }


void sub(ZZ_pX & x, const ZZ_pX& a, const ZZ_p& b); // x = a - b

void sub(ZZ_pX& x, const ZZ_pX& a, long b);
void sub(ZZ_pX& x, const ZZ_pX& a, const ZZ_p& b);

void sub(ZZ_pX& x, long a, const ZZ_pX& b);
void sub(ZZ_pX& x, const ZZ_p& a, const ZZ_pX& b);

inline ZZ_pX operator+(const ZZ_pX& a, const ZZ_pX& b)
   { ZZ_pX x; add(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX operator+(const ZZ_pX& a, const ZZ_p& b)
   { ZZ_pX x; add(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX operator+(const ZZ_pX& a, long b)
   { ZZ_pX x; add(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX operator+(const ZZ_p& a, const ZZ_pX& b)
   { ZZ_pX x; add(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX operator+(long a, const ZZ_pX& b)
   { ZZ_pX x; add(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }


inline ZZ_pX operator-(const ZZ_pX& a, const ZZ_pX& b)
   { ZZ_pX x; sub(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX operator-(const ZZ_pX& a, const ZZ_p& b)
   { ZZ_pX x; sub(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX operator-(const ZZ_pX& a, long b)
   { ZZ_pX x; sub(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX operator-(const ZZ_p& a, const ZZ_pX& b)
   { ZZ_pX x; sub(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX operator-(long a, const ZZ_pX& b)
   { ZZ_pX x; sub(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }


inline ZZ_pX& operator+=(ZZ_pX& x, const ZZ_pX& b)
   { add(x, x, b); return x; }

inline ZZ_pX& operator+=(ZZ_pX& x, const ZZ_p& b)
   { add(x, x, b); return x; }

inline ZZ_pX& operator+=(ZZ_pX& x, long b)
   { add(x, x, b); return x; }

inline ZZ_pX& operator-=(ZZ_pX& x, const ZZ_pX& b)
   { sub(x, x, b); return x; }

inline ZZ_pX& operator-=(ZZ_pX& x, const ZZ_p& b)
   { sub(x, x, b); return x; }

inline ZZ_pX& operator-=(ZZ_pX& x, long b)
   { sub(x, x, b); return x; }


inline ZZ_pX operator-(const ZZ_pX& a) 
   { ZZ_pX x; negate(x, a); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX& operator++(ZZ_pX& x) { add(x, x, 1); return x; }
inline void operator++(ZZ_pX& x, int) { add(x, x, 1); }
inline ZZ_pX& operator--(ZZ_pX& x) { sub(x, x, 1); return x; }
inline void operator--(ZZ_pX& x, int) { sub(x, x, 1); }

/*****************************************************************

                        Multiplication

******************************************************************/


void mul(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b);
// x = a * b

void sqr(ZZ_pX& x, const ZZ_pX& a);
inline ZZ_pX sqr(const ZZ_pX& a)
   { ZZ_pX x; sqr(x, a); NTL_OPT_RETURN(ZZ_pX, x); }
// x = a^2

void mul(ZZ_pX & x, const ZZ_pX& a, const ZZ_p& b); 
void mul(ZZ_pX& x, const ZZ_pX& a, long b);


inline void mul(ZZ_pX& x, const ZZ_p& a, const ZZ_pX& b)
   { mul(x, b, a); }

inline void mul(ZZ_pX& x, long a, const ZZ_pX& b)
   { mul(x, b, a); }

inline ZZ_pX operator*(const ZZ_pX& a, const ZZ_pX& b)
   { ZZ_pX x; mul(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX operator*(const ZZ_pX& a, const ZZ_p& b)
   { ZZ_pX x; mul(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX operator*(const ZZ_pX& a, long b)
   { ZZ_pX x; mul(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX operator*(const ZZ_p& a, const ZZ_pX& b)
   { ZZ_pX x; mul(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX operator*(long a, const ZZ_pX& b)
   { ZZ_pX x; mul(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX& operator*=(ZZ_pX& x, const ZZ_pX& b)
   { mul(x, x, b); return x; }

inline ZZ_pX& operator*=(ZZ_pX& x, const ZZ_p& b)
   { mul(x, x, b); return x; }

inline ZZ_pX& operator*=(ZZ_pX& x, long b)
   { mul(x, x, b); return x; }


void PlainMul(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b);
// always uses the "classical" algorithm

void PlainSqr(ZZ_pX& x, const ZZ_pX& a);
// always uses the "classical" algorithm


void FFTMul(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b);
// always uses the FFT

void FFTSqr(ZZ_pX& x, const ZZ_pX& a);
// always uses the FFT

void MulTrunc(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b, long n);
// x = a * b % X^n

inline ZZ_pX MulTrunc(const ZZ_pX& a, const ZZ_pX& b, long n)
   { ZZ_pX x; MulTrunc(x, a, b, n); NTL_OPT_RETURN(ZZ_pX, x); }

void PlainMulTrunc(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b, long n);
void FFTMulTrunc(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b, long n);

void SqrTrunc(ZZ_pX& x, const ZZ_pX& a, long n);
// x = a^2 % X^n

inline ZZ_pX SqrTrunc(const ZZ_pX& a, long n)
   { ZZ_pX x; SqrTrunc(x, a, n); NTL_OPT_RETURN(ZZ_pX, x); }

void PlainSqrTrunc(ZZ_pX& x, const ZZ_pX& a, long n);
void FFTSqrTrunc(ZZ_pX& x, const ZZ_pX& a, long n);


void power(ZZ_pX& x, const ZZ_pX& a, long e);
inline ZZ_pX power(const ZZ_pX& a, long e)
   { ZZ_pX x; power(x, a, e); NTL_OPT_RETURN(ZZ_pX, x); }


// The following data structures and routines allow one
// to hand-craft various algorithms, using the FFT convolution
// algorithms directly.
// Look in the file ZZ_pX.c for examples.




// FFT representation of polynomials

class FFTRep {
public:
   long k;                // a 2^k point representation
   long MaxK;             // maximum space allocated
   long len;              // length of truncated FFT
   long NumPrimes; 
   Unique2DArray<long> tbl;

   FFTRep() : k(-1), MaxK(-1), len(0), NumPrimes(0) { }

   FFTRep(const FFTRep& R) : k(-1), MaxK(-1), len(0), NumPrimes(0)
   { *this = R; }

   FFTRep(INIT_SIZE_TYPE, long InitK) : k(-1), MaxK(-1), len(0), NumPrimes(0)
   {  SetSize(InitK); }

   FFTRep& operator=(const FFTRep& R);
   void SetSize(long NewK);
   void DoSetSize(long NewK, long NewNumPrimes);
};


void ToFFTRep_trunc(FFTRep& y, const ZZ_pX& x, long k, long len, 
                    long lo, long hi);

inline void ToFFTRep_trunc(FFTRep& y, const ZZ_pX& x, long k, long len)
{ ToFFTRep_trunc(y, x, k, len, 0, deg(x)); }

inline
void ToFFTRep(FFTRep& y, const ZZ_pX& x, long k, long lo, long hi)
// computes an n = 2^k point convolution of x[lo..hi].
{ ToFFTRep_trunc(y, x, k, 1L << k, lo, hi); }

inline void ToFFTRep(FFTRep& y, const ZZ_pX& x, long k)

   { ToFFTRep(y, x, k, 0, deg(x)); }

void RevToFFTRep(FFTRep& y, const vec_ZZ_p& x,
                 long k, long lo, long hi, long offset);
// computes an n = 2^k point convolution of X^offset*x[lo..hi] mod X^n-1
// using "inverted" evaluation points.


void FromFFTRep(ZZ_pX& x, FFTRep& y, long lo, long hi);
// converts from FFT-representation to coefficient representation
// only the coefficients lo..hi are computed
// NOTE: this version destroys the data in y

// non-destructive versions of the above

void NDFromFFTRep(ZZ_pX& x, const FFTRep& y, long lo, long hi, FFTRep& temp);
void NDFromFFTRep(ZZ_pX& x, const FFTRep& y, long lo, long hi);

void RevFromFFTRep(vec_ZZ_p& x, FFTRep& y, long lo, long hi);

   // converts from FFT-representation to coefficient representation
   // using "inverted" evaluation points.
   // only the coefficients lo..hi are computed




void FromFFTRep(ZZ_p* x, FFTRep& y, long lo, long hi);
// convert out coefficients lo..hi of y, store result in x.
// no normalization is done.


// direct manipulation of FFT reps

void mul(FFTRep& z, const FFTRep& x, const FFTRep& y);
void sub(FFTRep& z, const FFTRep& x, const FFTRep& y);
void add(FFTRep& z, const FFTRep& x, const FFTRep& y);

void reduce(FFTRep& x, const FFTRep& a, long k);
// reduces a 2^l point FFT-rep to a 2^k point FFT-rep

void AddExpand(FFTRep& x, const FFTRep& a);
//  x = x + (an "expanded" version of a)





// This data structure holds unconvoluted modular representations
// of polynomials

class ZZ_pXModRep {
private:
   ZZ_pXModRep(const ZZ_pXModRep&); // disabled
   void operator=(const ZZ_pXModRep&); // disabled

public:
   long n;
   long MaxN;
   long NumPrimes; 
   Unique2DArray<long> tbl;

   void SetSize(long NewN);

   ZZ_pXModRep() : n(0), MaxN(0), NumPrimes(0) {  }
   ZZ_pXModRep(INIT_SIZE_TYPE, long NewN) : n(0), MaxN(0), NumPrimes(0)
   {  SetSize(NewN); }
};


void ToZZ_pXModRep(ZZ_pXModRep& x, const ZZ_pX& a, long lo, long hi);

void ToFFTRep(FFTRep& x, const ZZ_pXModRep& a, long k, long lo, long hi);
// converts coefficients lo..hi to a 2^k-point FFTRep.
// must have hi-lo+1 < 2^k


void FromFFTRep(ZZ_pXModRep& x, const FFTRep& a);
// for testing and timing purposes only -- converts from FFTRep

void FromZZ_pXModRep(ZZ_pX& x, const ZZ_pXModRep& a, long lo, long hi);
// for testing and timing purposes only -- converts from ZZ_pXModRep




/*************************************************************

                      Division

**************************************************************/

void DivRem(ZZ_pX& q, ZZ_pX& r, const ZZ_pX& a, const ZZ_pX& b);
// q = a/b, r = a%b

void div(ZZ_pX& q, const ZZ_pX& a, const ZZ_pX& b);
// q = a/b

void div(ZZ_pX& q, const ZZ_pX& a, const ZZ_p& b);
void div(ZZ_pX& q, const ZZ_pX& a, long b);


void rem(ZZ_pX& r, const ZZ_pX& a, const ZZ_pX& b);
// r = a%b

long divide(ZZ_pX& q, const ZZ_pX& a, const ZZ_pX& b);
// if b | a, sets q = a/b and returns 1; otherwise returns 0

long divide(const ZZ_pX& a, const ZZ_pX& b);
// if b | a, sets q = a/b and returns 1; otherwise returns 0

void InvTrunc(ZZ_pX& x, const ZZ_pX& a, long m);
// computes x = a^{-1} % X^m 
// constant term must be non-zero

inline ZZ_pX InvTrunc(const ZZ_pX& a, long m)
   { ZZ_pX x; InvTrunc(x, a, m); NTL_OPT_RETURN(ZZ_pX, x); }



// These always use "classical" arithmetic
void PlainDivRem(ZZ_pX& q, ZZ_pX& r, const ZZ_pX& a, const ZZ_pX& b);
void PlainDiv(ZZ_pX& q, const ZZ_pX& a, const ZZ_pX& b);
void PlainRem(ZZ_pX& r, const ZZ_pX& a, const ZZ_pX& b);

void PlainRem(ZZ_pX& r, const ZZ_pX& a, const ZZ_pX& b, ZZVec& tmp);
void PlainDivRem(ZZ_pX& q, ZZ_pX& r, const ZZ_pX& a, const ZZ_pX& b,
                 ZZVec& tmp);


// These always use FFT arithmetic
void FFTDivRem(ZZ_pX& q, ZZ_pX& r, const ZZ_pX& a, const ZZ_pX& b);
void FFTDiv(ZZ_pX& q, const ZZ_pX& a, const ZZ_pX& b);
void FFTRem(ZZ_pX& r, const ZZ_pX& a, const ZZ_pX& b);

void PlainInvTrunc(ZZ_pX& x, const ZZ_pX& a, long m);
// always uses "classical" algorithm
// ALIAS RESTRICTION: input may not alias output

void NewtonInvTrunc(ZZ_pX& x, const ZZ_pX& a, long m);
// uses a Newton Iteration with the FFT.
// ALIAS RESTRICTION: input may not alias output


inline ZZ_pX operator/(const ZZ_pX& a, const ZZ_pX& b)
   { ZZ_pX x; div(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX operator/(const ZZ_pX& a, const ZZ_p& b)
   { ZZ_pX x; div(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX operator/(const ZZ_pX& a, long b)
   { ZZ_pX x; div(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX& operator/=(ZZ_pX& x, const ZZ_p& b)
   { div(x, x, b); return x; }

inline ZZ_pX& operator/=(ZZ_pX& x, long b)
   { div(x, x, b); return x; }

inline ZZ_pX& operator/=(ZZ_pX& x, const ZZ_pX& b)
   { div(x, x, b); return x; }


inline ZZ_pX operator%(const ZZ_pX& a, const ZZ_pX& b)
   { ZZ_pX x; rem(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX& operator%=(ZZ_pX& x, const ZZ_pX& b)
   { rem(x, x, b); return x; }


/***********************************************************

                         GCD's

************************************************************/


void GCD(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b);
// x = GCD(a, b),  x is always monic (or zero if a==b==0).

inline ZZ_pX GCD(const ZZ_pX& a, const ZZ_pX& b)
   { ZZ_pX x; GCD(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }

void XGCD(ZZ_pX& d, ZZ_pX& s, ZZ_pX& t, const ZZ_pX& a, const ZZ_pX& b);
// d = gcd(a,b), a s + b t = d 

void PlainXGCD(ZZ_pX& d, ZZ_pX& s, ZZ_pX& t, const ZZ_pX& a, const ZZ_pX& b);
// same as above, but uses classical algorithm


void PlainGCD(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b);
// always uses "cdlassical" arithmetic


class ZZ_pXMatrix {
private:

   ZZ_pXMatrix(const ZZ_pXMatrix&);  // disable
   ZZ_pX elts[2][2];

public:

   ZZ_pXMatrix() { }

   void operator=(const ZZ_pXMatrix&);
   ZZ_pX& operator() (long i, long j) { return elts[i][j]; }
   const ZZ_pX& operator() (long i, long j) const { return elts[i][j]; }
};


void HalfGCD(ZZ_pXMatrix& M_out, const ZZ_pX& U, const ZZ_pX& V, long d_red);
// deg(U) > deg(V),   1 <= d_red <= deg(U)+1.
//
// This computes a 2 x 2 polynomial matrix M_out such that
//    M_out * (U, V)^T = (U', V')^T,
// where U', V' are consecutive polynomials in the Euclidean remainder
// sequence of U, V, and V' is the polynomial of highest degree
// satisfying deg(V') <= deg(U) - d_red.

void XHalfGCD(ZZ_pXMatrix& M_out, ZZ_pX& U, ZZ_pX& V, long d_red);

// same as above, except that U is replaced by U', and V by V'


/*************************************************************

             Modular Arithmetic without pre-conditioning

**************************************************************/

// arithmetic mod f.
// all inputs and outputs are polynomials of degree less than deg(f).
// ASSUMPTION: f is assumed monic, and deg(f) > 0.
// NOTE: if you want to do many computations with a fixed f,
//       use the ZZ_pXModulus data structure and associated routines below.



void MulMod(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b, const ZZ_pX& f);
// x = (a * b) % f

inline ZZ_pX MulMod(const ZZ_pX& a, const ZZ_pX& b, const ZZ_pX& f)
   { ZZ_pX x; MulMod(x, a, b, f); NTL_OPT_RETURN(ZZ_pX, x); }

void SqrMod(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& f);
// x = a^2 % f

inline ZZ_pX SqrMod(const ZZ_pX& a,  const ZZ_pX& f)
   { ZZ_pX x; SqrMod(x, a, f); NTL_OPT_RETURN(ZZ_pX, x); }

void MulByXMod(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& f);
// x = (a * X) mod f

inline ZZ_pX MulByXMod(const ZZ_pX& a,  const ZZ_pX& f)
   { ZZ_pX x; MulByXMod(x, a, f); NTL_OPT_RETURN(ZZ_pX, x); }



void InvMod(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& f);
// x = a^{-1} % f, error is a is not invertible

inline ZZ_pX InvMod(const ZZ_pX& a,  const ZZ_pX& f)
   { ZZ_pX x; InvMod(x, a, f); NTL_OPT_RETURN(ZZ_pX, x); }

long InvModStatus(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& f);
// if (a, f) = 1, returns 0 and sets x = a^{-1} % f
// otherwise, returns 1 and sets x = (a, f)




/******************************************************************

        Modular Arithmetic with Pre-conditioning

*******************************************************************/


// If you need to do a lot of arithmetic modulo a fixed f,
// build ZZ_pXModulus F for f.  This pre-computes information about f
// that speeds up the computation a great deal.


class ZZ_pXModulus {
public:
   ZZ_pXModulus() : UseFFT(0), n(-1)  { }
   

   // the following members may become private in future
   ZZ_pX f;   // the modulus
   long UseFFT;// flag indicating whether FFT should be used.
   long n;     // n = deg(f)
   long k;     // least k s/t 2^k >= n
   long l;     // least l s/t 2^l >= 2n-3
   FFTRep FRep; // 2^k point rep of f
                // H = rev((rev(f))^{-1} rem X^{n-1})
   FFTRep HRep; // 2^l point rep of H

   OptionalVal< Lazy<vec_ZZ_p> > tracevec;
   // an extra level of indirection to ensure the class
   // can be used in a Vec (there may be a mutex in the Lazy object)

   // but these will remain public
   ZZ_pXModulus(const ZZ_pX& ff);

   const ZZ_pX& val() const { return f; }
   operator const ZZ_pX& () const { return f; }

};


NTL_DECLARE_RELOCATABLE((ZZ_pXModulus*))

inline long deg(const ZZ_pXModulus& F) { return F.n; }

void build(ZZ_pXModulus& F, const ZZ_pX& f);
// deg(f) > 0.


void rem21(ZZ_pX& x, const ZZ_pX& a, const ZZ_pXModulus& F);
// x = a % f
// deg(a) <= 2(n-1), where n = F.n = deg(f)

void rem(ZZ_pX& x, const ZZ_pX& a, const ZZ_pXModulus& F);
// x = a % f, no restrictions on deg(a);  makes repeated calls to rem21

inline ZZ_pX operator%(const ZZ_pX& a, const ZZ_pXModulus& F)
   { ZZ_pX x; rem(x, a, F); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX& operator%=(ZZ_pX& x, const ZZ_pXModulus& F)
   { rem(x, x, F); return x; } 

void DivRem(ZZ_pX& q, ZZ_pX& r, const ZZ_pX& a, const ZZ_pXModulus& F);

void div(ZZ_pX& q, const ZZ_pX& a, const ZZ_pXModulus& F);

inline ZZ_pX operator/(const ZZ_pX& a, const ZZ_pXModulus& F)
   { ZZ_pX x; div(x, a, F); NTL_OPT_RETURN(ZZ_pX, x); }

inline ZZ_pX& operator/=(ZZ_pX& x, const ZZ_pXModulus& F)
   { div(x, x, F); return x; } 

void MulMod(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b, const ZZ_pXModulus& F);
// x = (a * b) % f
// deg(a), deg(b) < n

inline ZZ_pX MulMod(const ZZ_pX& a, const ZZ_pX& b, const ZZ_pXModulus& F)
   { ZZ_pX x; MulMod(x, a, b, F); NTL_OPT_RETURN(ZZ_pX, x); }

void SqrMod(ZZ_pX& x, const ZZ_pX& a, const ZZ_pXModulus& F);
// x = a^2 % f
// deg(a) < n

inline ZZ_pX SqrMod(const ZZ_pX& a, const ZZ_pXModulus& F)
   { ZZ_pX x; SqrMod(x, a, F); NTL_OPT_RETURN(ZZ_pX, x); }

void PowerMod(ZZ_pX& x, const ZZ_pX& a, const ZZ& e, const ZZ_pXModulus& F);
// x = a^e % f, e >= 0

inline ZZ_pX PowerMod(const ZZ_pX& a, const ZZ& e, const ZZ_pXModulus& F)
   { ZZ_pX x; PowerMod(x, a, e, F); NTL_OPT_RETURN(ZZ_pX, x); }

inline void PowerMod(ZZ_pX& x, const ZZ_pX& a, long e, const ZZ_pXModulus& F)
   { PowerMod(x, a, ZZ_expo(e), F); }

inline ZZ_pX PowerMod(const ZZ_pX& a, long e, const ZZ_pXModulus& F)
   { ZZ_pX x; PowerMod(x, a, e, F); NTL_OPT_RETURN(ZZ_pX, x); }



void PowerXMod(ZZ_pX& x, const ZZ& e, const ZZ_pXModulus& F);
// x = X^e % f, e >= 0

inline ZZ_pX PowerXMod(const ZZ& e, const ZZ_pXModulus& F)
   { ZZ_pX x; PowerXMod(x, e, F); NTL_OPT_RETURN(ZZ_pX, x); }

inline void PowerXMod(ZZ_pX& x, long e, const ZZ_pXModulus& F)
   { PowerXMod(x, ZZ_expo(e), F); }

inline ZZ_pX PowerXMod(long e, const ZZ_pXModulus& F)
   { ZZ_pX x; PowerXMod(x, e, F); NTL_OPT_RETURN(ZZ_pX, x); }

void PowerXPlusAMod(ZZ_pX& x, const ZZ_p& a, const ZZ& e, const ZZ_pXModulus& F);
// x = (X + a)^e % f, e >= 0

inline ZZ_pX PowerXPlusAMod(const ZZ_p& a, const ZZ& e, const ZZ_pXModulus& F)
   { ZZ_pX x; PowerXPlusAMod(x, a, e, F); NTL_OPT_RETURN(ZZ_pX, x); }

inline void PowerXPlusAMod(ZZ_pX& x, const ZZ_p& a, long e, const ZZ_pXModulus& F)
   { PowerXPlusAMod(x, a, ZZ_expo(e), F); }


inline ZZ_pX PowerXPlusAMod(const ZZ_p& a, long e, const ZZ_pXModulus& F)
   { ZZ_pX x; PowerXPlusAMod(x, a, e, F); NTL_OPT_RETURN(ZZ_pX, x); }

// If you need to compute a * b % f for a fixed b, but for many a's
// (for example, computing powers of b modulo f), it is
// much more efficient to first build a ZZ_pXMultiplier B for b,
// and then use the routine below.

class ZZ_pXMultiplier {
public:
   ZZ_pXMultiplier() : UseFFT(0)  { }
   ZZ_pXMultiplier(const ZZ_pX& b, const ZZ_pXModulus& F);



   // the following members may become private in the future
   ZZ_pX b;   
   long UseFFT;
   FFTRep B1; 
   FFTRep B2; 

   // but this will remain public
   const ZZ_pX& val() const { return b; }

};

void build(ZZ_pXMultiplier& B, const ZZ_pX& b, const ZZ_pXModulus& F);

void MulMod(ZZ_pX& x, const ZZ_pX& a, const ZZ_pXMultiplier& B,
                                      const ZZ_pXModulus& F);

inline ZZ_pX MulMod(const ZZ_pX& a, const ZZ_pXMultiplier& B,
                                          const ZZ_pXModulus& F)
   { ZZ_pX x; MulMod(x, a, B, F); NTL_OPT_RETURN(ZZ_pX, x); }

// x = (a * b) % f


/*******************************************************

              Evaluation and related problems

********************************************************/


void BuildFromRoots(ZZ_pX& x, const vec_ZZ_p& a);
// computes the polynomial (X-a[0]) ... (X-a[n-1]), where n = a.length()

inline ZZ_pX BuildFromRoots(const vec_ZZ_p& a)
   { ZZ_pX x; BuildFromRoots(x, a); NTL_OPT_RETURN(ZZ_pX, x); }


void eval(ZZ_p& b, const ZZ_pX& f, const ZZ_p& a);
// b = f(a)

inline ZZ_p eval(const ZZ_pX& f, const ZZ_p& a)
   { ZZ_p x; eval(x, f, a); NTL_OPT_RETURN(ZZ_p, x); }

void eval(vec_ZZ_p& b, const ZZ_pX& f, const vec_ZZ_p& a);
//  b[i] = f(a[i])

inline vec_ZZ_p eval(const ZZ_pX& f, const vec_ZZ_p& a)
   { vec_ZZ_p x; eval(x, f, a); NTL_OPT_RETURN(vec_ZZ_p, x); }


void interpolate(ZZ_pX& f, const vec_ZZ_p& a, const vec_ZZ_p& b);
// computes f such that f(a[i]) = b[i]

inline ZZ_pX interpolate(const vec_ZZ_p& a, const vec_ZZ_p& b)
   { ZZ_pX x; interpolate(x, a, b); NTL_OPT_RETURN(ZZ_pX, x); }


/*****************************************************************

                       vectors of ZZ_pX's

*****************************************************************/

typedef Vec<ZZ_pX> vec_ZZ_pX;



/**********************************************************

         Modular Composition and Minimal Polynomials

***********************************************************/


// algorithms for computing g(h) mod f



void CompMod(ZZ_pX& x, const ZZ_pX& g, const ZZ_pX& h, const ZZ_pXModulus& F);
// x = g(h) mod f

inline ZZ_pX CompMod(const ZZ_pX& g, const ZZ_pX& h, 
                           const ZZ_pXModulus& F)
   { ZZ_pX x; CompMod(x, g, h, F); NTL_OPT_RETURN(ZZ_pX, x); }

void Comp2Mod(ZZ_pX& x1, ZZ_pX& x2, const ZZ_pX& g1, const ZZ_pX& g2,
              const ZZ_pX& h, const ZZ_pXModulus& F);
// xi = gi(h) mod f (i=1,2)

void Comp3Mod(ZZ_pX& x1, ZZ_pX& x2, ZZ_pX& x3, 
              const ZZ_pX& g1, const ZZ_pX& g2, const ZZ_pX& g3,
              const ZZ_pX& h, const ZZ_pXModulus& F);
// xi = gi(h) mod f (i=1..3)



// The routine build (see below) which is implicitly called
// by the various compose and UpdateMap routines builds a table
// of polynomials.  
// If ZZ_pXArgBound > 0, then the table is limited in
// size to approximamtely that many KB.
// If ZZ_pXArgBound <= 0, then it is ignored, and space is allocated
// so as to maximize speed.
// Initially, ZZ_pXArgBound = 0.


// If a single h is going to be used with many g's
// then you should build a ZZ_pXArgument for h,
// and then use the compose routine below.
// build computes and stores h, h^2, ..., h^m mod f.
// After this pre-computation, composing a polynomial of degree 
// roughly n with h takes n/m multiplies mod f, plus n^2
// scalar multiplies.
// Thus, increasing m increases the space requirement and the pre-computation
// time, but reduces the composition time.
// If ZZ_pXArgBound > 0, a table of size less than m may be built.

struct ZZ_pXArgument {
   vec_ZZ_pX H;
};

extern NTL_CHEAP_THREAD_LOCAL long ZZ_pXArgBound;


void build(ZZ_pXArgument& H, const ZZ_pX& h, const ZZ_pXModulus& F, long m);

// m must be > 0, otherwise an error is raised

void CompMod(ZZ_pX& x, const ZZ_pX& g, const ZZ_pXArgument& H, 
             const ZZ_pXModulus& F);

inline ZZ_pX 
CompMod(const ZZ_pX& g, const ZZ_pXArgument& H, const ZZ_pXModulus& F)
   { ZZ_pX x; CompMod(x, g, H, F); NTL_OPT_RETURN(ZZ_pX, x); }


// New alternative CompMod strategy that just reduces to 
// matrix multiplication ... 

#if (!defined(NTL_WIZARD_HACK) && defined(NTL_HAVE_LL_TYPE))
// NOTE: currently, we only ise the multi-modular matmul NTL_HAVE_LL_TYPE
// if NTL_HAVE_LL_TYPE is defined.

#define NTL_USE_ZZ_pXNewArgument

#endif

#ifdef NTL_USE_ZZ_pXNewArgument

struct ZZ_pXNewArgument {
   mat_ZZ_p_opaque mat;
   ZZ_pX poly;
};

void build(ZZ_pXNewArgument& H, const ZZ_pX& h, const ZZ_pXModulus& F, long m);
void CompMod(ZZ_pX& x, const ZZ_pX& g, const ZZ_pXNewArgument& H,
             const ZZ_pXModulus& F);

void ProjectPowers(vec_ZZ_p& x, const vec_ZZ_p& a, long k,
                   const ZZ_pXNewArgument& H, const ZZ_pXModulus& F);

#else

typedef ZZ_pXArgument ZZ_pXNewArgument;

#endif




#ifndef NTL_TRANSITION

void UpdateMap(vec_ZZ_p& x, const vec_ZZ_p& a,
               const ZZ_pXMultiplier& B, const ZZ_pXModulus& F);

inline vec_ZZ_p
UpdateMap(const vec_ZZ_p& a, 
          const ZZ_pXMultiplier& B, const ZZ_pXModulus& F)
   { vec_ZZ_p x; UpdateMap(x, a, B, F); 
     NTL_OPT_RETURN(vec_ZZ_p, x); }

#endif


/* computes (a, b), (a, (b*X)%f), ..., (a, (b*X^{n-1})%f),
   where ( , ) denotes the vector inner product.

   This is really a "transposed" MulMod by B.
*/

void PlainUpdateMap(vec_ZZ_p& x, const vec_ZZ_p& a,
                    const ZZ_pX& b, const ZZ_pX& f);

// same as above, but uses only classical arithmetic


void ProjectPowers(vec_ZZ_p& x, const vec_ZZ_p& a, long k,
                   const ZZ_pX& h, const ZZ_pXModulus& F);

inline vec_ZZ_p ProjectPowers(const vec_ZZ_p& a, long k,
                   const ZZ_pX& h, const ZZ_pXModulus& F)
{
   vec_ZZ_p x;
   ProjectPowers(x, a, k, h, F);
   NTL_OPT_RETURN(vec_ZZ_p, x);
}


// computes (a, 1), (a, h), ..., (a, h^{k-1} % f)
// this is really a "transposed" compose.

void ProjectPowers(vec_ZZ_p& x, const vec_ZZ_p& a, long k,
                   const ZZ_pXArgument& H, const ZZ_pXModulus& F);

inline vec_ZZ_p ProjectPowers(const vec_ZZ_p& a, long k,
                   const ZZ_pXArgument& H, const ZZ_pXModulus& F)
{
   vec_ZZ_p x;
   ProjectPowers(x, a, k, H, F);
   NTL_OPT_RETURN(vec_ZZ_p, x);
}

// same as above, but uses a pre-computed ZZ_pXArgument

inline void project(ZZ_p& x, const vec_ZZ_p& a, const ZZ_pX& b)
   { InnerProduct(x, a, b.rep); }

inline ZZ_p project(const vec_ZZ_p& a, const ZZ_pX& b)
   {  ZZ_p x; project(x, a, b); NTL_OPT_RETURN(ZZ_p, x); }

void MinPolySeq(ZZ_pX& h, const vec_ZZ_p& a, long m);
// computes the minimum polynomial of a linealy generated sequence;
// m is a bound on the degree of the polynomial;
// required: a.length() >= 2*m

inline ZZ_pX MinPolySeq(const vec_ZZ_p& a, long m)
   { ZZ_pX x; MinPolySeq(x, a, m); NTL_OPT_RETURN(ZZ_pX, x); }

void ProbMinPolyMod(ZZ_pX& h, const ZZ_pX& g, const ZZ_pXModulus& F, long m);

inline ZZ_pX ProbMinPolyMod(const ZZ_pX& g, const ZZ_pXModulus& F, long m)
   { ZZ_pX x; ProbMinPolyMod(x, g, F, m); NTL_OPT_RETURN(ZZ_pX, x); }


inline void ProbMinPolyMod(ZZ_pX& h, const ZZ_pX& g, const ZZ_pXModulus& F)
   { ProbMinPolyMod(h, g, F, F.n); }

inline ZZ_pX ProbMinPolyMod(const ZZ_pX& g, const ZZ_pXModulus& F)
   { ZZ_pX x; ProbMinPolyMod(x, g, F); NTL_OPT_RETURN(ZZ_pX, x); }


// computes the monic minimal polynomial if (g mod f).
// m = a bound on the degree of the minimal polynomial.
// If this argument is not supplied, it defaults to deg(f).
// The algorithm is probabilistic, always returns a divisor of
// the minimal polynomial, and returns a proper divisor with
// probability at most m/p.

void MinPolyMod(ZZ_pX& h, const ZZ_pX& g, const ZZ_pXModulus& F, long m);

inline ZZ_pX MinPolyMod(const ZZ_pX& g, const ZZ_pXModulus& F, long m)
   { ZZ_pX x; MinPolyMod(x, g, F, m); NTL_OPT_RETURN(ZZ_pX, x); }

inline void MinPolyMod(ZZ_pX& h, const ZZ_pX& g, const ZZ_pXModulus& F)
   { MinPolyMod(h, g, F, F.n); }

inline ZZ_pX MinPolyMod(const ZZ_pX& g, const ZZ_pXModulus& F)
   { ZZ_pX x; MinPolyMod(x, g, F); NTL_OPT_RETURN(ZZ_pX, x); }

// same as above, but guarantees that result is correct

void IrredPolyMod(ZZ_pX& h, const ZZ_pX& g, const ZZ_pXModulus& F, long m);

inline ZZ_pX IrredPolyMod(const ZZ_pX& g, const ZZ_pXModulus& F, long m)
   { ZZ_pX x; IrredPolyMod(x, g, F, m); NTL_OPT_RETURN(ZZ_pX, x); }


inline void IrredPolyMod(ZZ_pX& h, const ZZ_pX& g, const ZZ_pXModulus& F)
   { IrredPolyMod(h, g, F, F.n); }

inline ZZ_pX IrredPolyMod(const ZZ_pX& g, const ZZ_pXModulus& F)
   { ZZ_pX x; IrredPolyMod(x, g, F); NTL_OPT_RETURN(ZZ_pX, x); }

// same as above, but assumes that f is irreducible, 
// or at least that the minimal poly of g is itself irreducible.
// The algorithm is deterministic (and is always correct).

/*****************************************************************

                   Traces, norms, resultants

******************************************************************/

void TraceVec(vec_ZZ_p& S, const ZZ_pX& f);

inline vec_ZZ_p TraceVec(const ZZ_pX& f)
   { vec_ZZ_p x; TraceVec(x, f); NTL_OPT_RETURN(vec_ZZ_p, x); }

void FastTraceVec(vec_ZZ_p& S, const ZZ_pX& f);
void PlainTraceVec(vec_ZZ_p& S, const ZZ_pX& f);

void TraceMod(ZZ_p& x, const ZZ_pX& a, const ZZ_pXModulus& F);

inline ZZ_p TraceMod(const ZZ_pX& a, const ZZ_pXModulus& F)
   { ZZ_p x; TraceMod(x, a, F); NTL_OPT_RETURN(ZZ_p, x); }

void TraceMod(ZZ_p& x, const ZZ_pX& a, const ZZ_pX& f);

inline ZZ_p TraceMod(const ZZ_pX& a, const ZZ_pX& f)
   { ZZ_p x; TraceMod(x, a, f); NTL_OPT_RETURN(ZZ_p, x); }



void ComputeTraceVec(const ZZ_pXModulus& F);


void NormMod(ZZ_p& x, const ZZ_pX& a, const ZZ_pX& f);

inline ZZ_p NormMod(const ZZ_pX& a, const ZZ_pX& f)
   { ZZ_p x; NormMod(x, a, f); NTL_OPT_RETURN(ZZ_p, x); }

void resultant(ZZ_p& rres, const ZZ_pX& a, const ZZ_pX& b);

inline ZZ_p resultant(const ZZ_pX& a, const ZZ_pX& b)
   { ZZ_p x; resultant(x, a, b); NTL_OPT_RETURN(ZZ_p, x); }

void CharPolyMod(ZZ_pX& g, const ZZ_pX& a, const ZZ_pX& f);
// g = char poly of (a mod f)

inline ZZ_pX CharPolyMod(const ZZ_pX& a, const ZZ_pX& f)
   { ZZ_pX x; CharPolyMod(x, a, f); NTL_OPT_RETURN(ZZ_pX, x); }



NTL_CLOSE_NNS

#endif
