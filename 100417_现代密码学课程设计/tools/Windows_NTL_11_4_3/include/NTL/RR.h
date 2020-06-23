#ifndef NTL_RR__H
#define NTL_RR__H

#include <NTL/ZZ.h>
#include <NTL/xdouble.h>
#include <NTL/quad_float.h>

NTL_OPEN_NNS


class RR {

public:

ZZ x;
long e;

RR() {  e = 0; }

explicit RR(double a) : e(0) { *this = a; }

inline RR(INIT_VAL_TYPE, const ZZ& a);
inline RR(INIT_VAL_TYPE, int a);
inline RR(INIT_VAL_TYPE, long a);
inline RR(INIT_VAL_TYPE, unsigned int a);
inline RR(INIT_VAL_TYPE, unsigned long a);
inline RR(INIT_VAL_TYPE, float a);
inline RR(INIT_VAL_TYPE, double a);
inline RR(INIT_VAL_TYPE, const xdouble& a);
inline RR(INIT_VAL_TYPE, const quad_float& a);
inline RR(INIT_VAL_TYPE, const char *a);  // read from string
inline RR(INIT_VAL_TYPE, const RR& a);


inline RR& operator=(double a);

RR(RR& z, INIT_TRANS_TYPE) : x(z.x, INIT_TRANS), e(z.e) { } 

void swap(RR& z) { x.swap(z.x); _ntl_swap(e, z.e); }



const ZZ& mantissa() const { return x; }
long exponent() const { return e; }

static NTL_CHEAP_THREAD_LOCAL long prec;
static void SetPrecision(long p);
static long precision() { return prec; }

static NTL_CHEAP_THREAD_LOCAL long oprec;
static void SetOutputPrecision(long p);
static long OutputPrecision() { return oprec; }

#ifdef NTL_TRANSITION
private:
RR& operator=(const RR&);
RR(const RR&);
#endif

};



NTL_DECLARE_RELOCATABLE((RR*))


inline void swap(RR& a, RR& b) { a.swap(b); }


class RRPush {
private: 
   long old_p;

   RRPush(const RRPush&); // disable
   void operator=(const RRPush&); // disable

public:
   RRPush() : old_p(RR::prec) { }
   ~RRPush() { RR::prec = old_p; } 

};

// RAII for saving/restoring output precision
// FIXME: document. 

class RROutputPush {
private: 
   long old_p;

   RROutputPush(const RROutputPush&); // disable
   void operator=(const RROutputPush&); // disable

public:
   RROutputPush() : old_p(RR::oprec) { }
   ~RROutputPush() { RR::oprec = old_p; } 

};


long IsZero(const RR& a);
long IsOne(const RR& a);
long sign(const RR& a);
void clear(RR& z);
void set(RR& z);

void add(RR& z, const RR& a, const RR& b);

void add(RR& z, const RR& a, double b);
inline void add(RR& z, double a, const RR& b) { add(z, b, a); }



void sub(RR& z, const RR& a, const RR& b);

void sub(RR& z, const RR& a, double b);
void sub(RR& z, double a, const RR& b);

void negate(RR& z, const RR& a);

void abs(RR& z, const RR& a);
inline RR abs(const RR& a)
   { RR z; abs(z, a); NTL_OPT_RETURN(RR, z); }
inline RR fabs(const RR& a)
   { RR z; abs(z, a); NTL_OPT_RETURN(RR, z); }

void mul(RR& z, const RR& a, const RR& b);

void mul(RR& z, const RR& a, double b);
inline void mul(RR& z, double a, const RR& b) { mul(z, b, a); }

void sqr(RR& z, const RR& a);
inline RR sqr(const RR& a)
   { RR z; sqr(z, a); NTL_OPT_RETURN(RR, z); }

void div(RR& z, const RR& a, const RR& b);

void div(RR& z, const RR& a, double b);
void div(RR& z, double a, const RR& b);

void inv(RR& z, const RR& a);
inline RR inv(const RR& a)
   { RR z; inv(z, a); NTL_OPT_RETURN(RR, z); }

// operator notation:

inline RR operator+(const RR& a, const RR& b)
   { RR x; add(x, a, b); NTL_OPT_RETURN(RR, x); }

inline RR operator+(const RR& a, double b)
   { RR x; add(x, a, b); NTL_OPT_RETURN(RR, x); }

inline RR operator+(double a, const RR& b)
   { RR x; add(x, a, b); NTL_OPT_RETURN(RR, x); }

inline RR& operator+=(RR& x, const RR& b)
   { add(x, x, b); return x; } 

inline RR& operator+=(RR& x, double b)
   { add(x, x, b); return x; } 



inline RR operator-(const RR& a, const RR& b)
   { RR x; sub(x, a, b); NTL_OPT_RETURN(RR, x); }

inline RR operator-(const RR& a, double b)
   { RR x; sub(x, a, b); NTL_OPT_RETURN(RR, x); }

inline RR operator-(double a, const RR& b)
   { RR x; sub(x, a, b); NTL_OPT_RETURN(RR, x); }

inline RR& operator-=(RR& x, const RR& b)
   { sub(x, x, b); return x; } 

inline RR& operator-=(RR& x, double b)
   { sub(x, x, b); return x; } 



inline RR operator*(const RR& a, const RR& b)
   { RR x; mul(x, a, b); NTL_OPT_RETURN(RR, x); }

inline RR operator*(const RR& a, double b)
   { RR x; mul(x, a, b); NTL_OPT_RETURN(RR, x); }

inline RR operator*(double a, const RR& b)
   { RR x; mul(x, a, b); NTL_OPT_RETURN(RR, x); }

inline RR& operator*=(RR& x, const RR& b)
   { mul(x, x, b); return x; } 

inline RR& operator*=(RR& x, double b)
   { mul(x, x, b); return x; } 


inline RR operator/(const RR& a, const RR& b)
   { RR x; div(x, a, b); NTL_OPT_RETURN(RR, x); }

inline RR operator/(const RR& a, double b)
   { RR x; div(x, a, b); NTL_OPT_RETURN(RR, x); }

inline RR operator/(double a, const RR& b)
   { RR x; div(x, a, b); NTL_OPT_RETURN(RR, x); }

inline RR& operator/=(RR& x, const RR& b)
   { div(x, x, b); return x; } 

inline RR& operator/=(RR& x, double b)
   { div(x, x, b); return x; } 


inline RR operator-(const RR& a)
   { RR x; negate(x, a); NTL_OPT_RETURN(RR, x); }


inline RR& operator++(RR& x) { add(x, x, 1); return x; }
inline void operator++(RR& x, int) { add(x, x, 1); }
inline RR& operator--(RR& x) { sub(x, x, 1); return x; }
inline void operator--(RR& x, int) { sub(x, x, 1); }



long compare(const RR& a, const RR& b);

long compare(const RR& a, double b);
inline long compare(double a, const RR& b) { return -compare(b, a); }


long operator==(const RR& a, const RR& b);
inline long operator!=(const RR& a, const RR& b) { return !(a == b); }
inline long operator<=(const RR& a, const RR& b) { return compare(a, b) <= 0; }
inline long operator>=(const RR& a, const RR& b) { return compare(a, b) >= 0; }
inline long operator <(const RR& a, const RR& b) { return compare(a, b)  < 0; }
inline long operator >(const RR& a, const RR& b) { return compare(a, b)  > 0; }

long operator==(const RR& a, double b);
inline long operator!=(const RR& a, double b) { return !(a == b); }
inline long operator<=(const RR& a, double b) { return compare(a, b) <= 0; }
inline long operator>=(const RR& a, double b) { return compare(a, b) >= 0; }
inline long operator <(const RR& a, double b) { return compare(a, b)  < 0; }
inline long operator >(const RR& a, double b) { return compare(a, b)  > 0; }

inline long operator==(double a, const RR& b) { return (b == a); }
inline long operator!=(double a, const RR& b) { return !(a == b); }
inline long operator<=(double a, const RR& b) { return compare(a, b) <= 0; }
inline long operator>=(double a, const RR& b) { return compare(a, b) >= 0; }
inline long operator <(double a, const RR& b) { return compare(a, b)  < 0; }
inline long operator >(double a, const RR& b) { return compare(a, b)  > 0; }

void ceil(RR& z, const RR& a);
inline RR ceil(const RR& a)
   { RR z; ceil(z, a); NTL_OPT_RETURN(RR, z); }

void floor(RR& z, const RR& a);
inline RR floor(const RR& a)
   { RR z; floor(z, a); NTL_OPT_RETURN(RR, z); }

void trunc(RR& z, const RR& a);
inline RR trunc(const RR& a)
   { RR z; trunc(z, a); NTL_OPT_RETURN(RR, z); }

void round(RR& z, const RR& a);
inline RR round(const RR& a)
   { RR z; round(z, a); NTL_OPT_RETURN(RR, z); }

void RoundToPrecision(RR& z, const RR& a, long p);
inline RR RoundToPrecision(const RR& a, long p)
   { RR z; RoundToPrecision(z, a, p); NTL_OPT_RETURN(RR, z); }


// routines with a precision parameter

void ConvPrec(RR& z, const RR& a, long p);
inline RR ConvPrec(const RR& a, long p)
   { RR z; ConvPrec(z, a, p); NTL_OPT_RETURN(RR, z); }

void AddPrec(RR& z, const RR& a, const RR& b, long p);
inline RR AddPrec(const RR& a, const RR& b, long p)
   { RR z; AddPrec(z, a, b, p); NTL_OPT_RETURN(RR, z); }

void SubPrec(RR& z, const RR& a, const RR& b, long p);
inline RR SubPrec(const RR& a, const RR& b, long p)
   { RR z; SubPrec(z, a, b, p); NTL_OPT_RETURN(RR, z); }

void NegatePrec(RR& z, const RR& a, long p);
inline RR NegatePrec(const RR& a, long p)
   { RR z; NegatePrec(z, a, p); NTL_OPT_RETURN(RR, z); }

void AbsPrec(RR& z, const RR& a, long p);
inline RR AbsPrec(const RR& a, long p)
   { RR z; AbsPrec(z, a, p); NTL_OPT_RETURN(RR, z); }

void MulPrec(RR& z, const RR& a, const RR& b, long p);
inline RR MulPrec(const RR& a, const RR& b, long p)
   { RR z; MulPrec(z, a, b, p); NTL_OPT_RETURN(RR, z); }

void SqrPrec(RR& z, const RR& a, long p);
inline RR SqrPrec(const RR& a, long p)
   { RR z; SqrPrec(z, a, p); NTL_OPT_RETURN(RR, z); }

void DivPrec(RR& z, const RR& a, const RR& b, long p);
inline RR DivPrec(const RR& a, const RR& b, long p)
   { RR z; DivPrec(z, a, b, p); NTL_OPT_RETURN(RR, z); }

void InvPrec(RR& z, const RR& a, long p);
inline RR InvPrec(const RR& a, long p)
   { RR z; InvPrec(z, a, p); NTL_OPT_RETURN(RR, z); }

void SqrRootPrec(RR& z, const RR& a, long p);
inline RR SqrRootPrec(const RR& a, long p)
   { RR z; SqrRootPrec(z, a, p); NTL_OPT_RETURN(RR, z); }

void TruncPrec(RR& z, const RR& a, long p);
inline RR TruncPrec(const RR& a, long p)
   { RR z; TruncPrec(z, a, p); NTL_OPT_RETURN(RR, z); }

void FloorPrec(RR& z, const RR& a, long p);
inline RR FloorPrec(const RR& a, long p)
   { RR z; FloorPrec(z, a, p); NTL_OPT_RETURN(RR, z); }

void CeilPrec(RR& z, const RR& a, long p);
inline RR CeilPrec(const RR& a, long p)
   { RR z; CeilPrec(z, a, p); NTL_OPT_RETURN(RR, z); }

void RoundPrec(RR& z, const RR& a, long p);
inline RR RoundPrec(const RR& a, long p)
   { RR z; RoundPrec(z, a, p); NTL_OPT_RETURN(RR, z); }

void ConvPrec(RR& z, const ZZ& a, long p);
inline RR ConvPrec(const ZZ& a, long p)
   { RR z; ConvPrec(z, a, p); NTL_OPT_RETURN(RR, z); }

void ConvPrec(RR& z, long a, long p);
inline RR ConvPrec(long a, long p)
   { RR z; ConvPrec(z, a, p); NTL_OPT_RETURN(RR, z); }

inline void ConvPrec(RR& z, int a, long p) { ConvPrec(z, long(a), p); }
inline RR ConvPrec(int a, long p)
   { RR z; ConvPrec(z, a, p); NTL_OPT_RETURN(RR, z); }

void ConvPrec(RR& z, unsigned long a, long p);
inline RR ConvPrec(unsigned long a, long p)
   { RR z; ConvPrec(z, a, p); NTL_OPT_RETURN(RR, z); }

inline void ConvPrec(RR& z, unsigned int a, long p) 
   { ConvPrec(z, (unsigned long)(a), p); }
inline RR ConvPrec(unsigned int a, long p)
   { RR z; ConvPrec(z, a, p); NTL_OPT_RETURN(RR, z); }

void ConvPrec(RR& z, double a, long p);
inline RR ConvPrec(double a, long p)
   { RR z; ConvPrec(z, a, p); NTL_OPT_RETURN(RR, z); }

void ConvPrec(RR& z, const xdouble& a, long p);
inline RR ConvPrec(const xdouble& a, long p)
   { RR z; ConvPrec(z, a, p); NTL_OPT_RETURN(RR, z); }

void ConvPrec(RR& z, const quad_float& a, long p);
inline RR ConvPrec(const quad_float& a, long p)
   { RR z; ConvPrec(z, a, p); NTL_OPT_RETURN(RR, z); }

void ConvPrec(RR& z, const char *s, long p);
inline RR ConvPrec(const char *s, long p)
   { RR z; ConvPrec(z, s, p); NTL_OPT_RETURN(RR, z); }

NTL_SNS istream& InputPrec(RR& z, NTL_SNS istream& s, long p);
inline RR InputPrec(NTL_SNS istream& s, long p)
   { RR z; NTL_INPUT_CHECK_ERR(InputPrec(z, s, p)); NTL_OPT_RETURN(RR, z); }

void MakeRRPrec(RR& z, const ZZ& a, long e, long p);
inline RR MakeRRPrec(const ZZ& a, long e, long p)
   { RR z; MakeRRPrec(z, a, e, p); NTL_OPT_RETURN(RR, z); }






void conv(RR& z, const ZZ& a);
void conv(RR& z, long a);
inline void conv(RR& z, int a) { conv(z, long(a)); }
void conv(RR& z, unsigned long a);
inline void conv(RR& z, unsigned int a) { conv(z, (unsigned long)(a)); }

void conv(RR& z, double a);
inline void conv(RR& z, float a) { conv(z, double(a)); }
void conv(RR& z, const xdouble& a);
void conv(RR& z, const quad_float& a);

void conv(RR& z, const RR& a);



inline RR::RR(INIT_VAL_TYPE, int a) { e = 0; conv(*this, a); }
inline RR::RR(INIT_VAL_TYPE, long a) { e = 0; conv(*this, a); }
inline RR::RR(INIT_VAL_TYPE, unsigned int a) { e = 0; conv(*this, a); }
inline RR::RR(INIT_VAL_TYPE, unsigned long a) { e = 0; conv(*this, a); }
inline RR::RR(INIT_VAL_TYPE, float a) { e = 0; conv(*this, a); }
inline RR::RR(INIT_VAL_TYPE, double a) { e = 0; conv(*this, a); }

inline RR::RR(INIT_VAL_TYPE, const RR& a) { e = 0; conv(*this, a); }
inline RR::RR(INIT_VAL_TYPE, const ZZ& a) { e = 0; conv(*this, a); }
inline RR::RR(INIT_VAL_TYPE, const xdouble& a) { e = 0; conv(*this, a); }
inline RR::RR(INIT_VAL_TYPE, const quad_float& a) { e = 0; conv(*this, a); }
inline RR::RR(INIT_VAL_TYPE, const char *a) { e = 0; conv(*this, a); }


inline RR to_RR(int a) { return RR(INIT_VAL, a); }
inline RR to_RR(long a) { return RR(INIT_VAL, a); }
inline RR to_RR(unsigned int a) { return RR(INIT_VAL, a); }
inline RR to_RR(unsigned long a) { return RR(INIT_VAL, a); }
inline RR to_RR(float a) { return RR(INIT_VAL, a); }
inline RR to_RR(double a) { return RR(INIT_VAL, a); }
inline RR to_RR(const ZZ& a) { return RR(INIT_VAL, a); }
inline RR to_RR(const RR& a) { return RR(INIT_VAL, a); }
inline RR to_RR(const xdouble& a) { return RR(INIT_VAL, a); }
inline RR to_RR(const quad_float& a) { return RR(INIT_VAL, a); }
inline RR to_RR(const char *a) { return RR(INIT_VAL, a); }

inline RR& RR::operator=(double a) { conv(*this, a); return *this; }

void conv(ZZ& z, const RR& a);
void conv(long& z, const RR& a);
void conv(double& z, const RR& a);
void conv(xdouble& z, const RR& a);
void conv(quad_float& z, const RR& a);

inline void conv(int& z, const RR& a) 
   { long t; conv(t, a); z = int(t); }

inline void conv(float& z, const RR& a) 
   { double t; conv(t, a); z = float(t); }

inline int to_int(const RR& a) { int z; conv(z, a); return z; }
inline long to_long(const RR& a) { long z; conv(z, a); return z; }
inline float to_float(const RR& a) { float z; conv(z, a); return z; }
inline double to_double(const RR& a) { double z; conv(z, a); return z; }

inline xdouble to_xdouble(const RR& a) 
   { xdouble z; conv(z, a); return z; }
inline quad_float to_quad_float(const RR& a) 
   { quad_float z; conv(z, a); return z; }

inline ZZ to_ZZ(const RR& a)
   { ZZ z; conv(z, a); NTL_OPT_RETURN(ZZ, z); }

void CeilToZZ(ZZ& z, const RR& a);
inline ZZ CeilToZZ(const RR& a)
   { ZZ z; CeilToZZ(z, a); NTL_OPT_RETURN(ZZ, z); }

void TruncToZZ(ZZ& z, const RR& a);
inline ZZ TruncToZZ(const RR& a)
   { ZZ z; TruncToZZ(z, a); NTL_OPT_RETURN(ZZ, z); }

void RoundToZZ(ZZ& z, const RR& a);
inline ZZ RoundToZZ(const RR& a)
   { ZZ z; RoundToZZ(z, a); NTL_OPT_RETURN(ZZ, z); }

inline void FloorToZZ(ZZ& z, const RR& a) { conv(z, a); }
inline ZZ FloorToZZ(const RR& a)
   { ZZ z; conv(z, a); NTL_OPT_RETURN(ZZ, z); }


/* additional legacy conversions for v6 conversion regime */

inline void conv(unsigned int& x, const RR& a)
   { long z; conv(z, a); conv(x, z); }

inline void conv(unsigned long& x, const RR& a)
   { long z; conv(z, a); conv(x, z); }


/* ------------------------------------- */

void MakeRR(RR& z, const ZZ& a,  long e);
inline RR MakeRR(const ZZ& a,  long e)
   { RR z; MakeRR(z, a, e); NTL_OPT_RETURN(RR, z); }

void random(RR& z);
inline RR random_RR() 
   { RR z; random(z); NTL_OPT_RETURN(RR, z); }


void power(RR& z, const RR& a, long e);
inline RR power(const RR& a, long e)
   { RR z; power(z, a, e); NTL_OPT_RETURN(RR, z); }

void power2(RR& z, long e);

inline RR power2_RR(long e)
   { RR z; power2(z, e); NTL_OPT_RETURN(RR, z); }

NTL_SNS ostream& operator<<(NTL_SNS ostream& s, const RR& a);
NTL_SNS istream& operator>>(NTL_SNS istream& s, RR& x);


void SqrRoot(RR& x, const RR& a);
inline RR SqrRoot(const RR& a)
   { RR z; SqrRoot(z, a); NTL_OPT_RETURN(RR, z); }
inline RR sqrt(const RR& a)
   { RR z; SqrRoot(z, a); NTL_OPT_RETURN(RR, z); }

void exp(RR& res, const RR& x);
inline RR exp(const RR& a)
   { RR z; exp(z, a); NTL_OPT_RETURN(RR, z); }

void log(RR& res, const RR& x);
inline RR log(const RR& a)
   { RR z; log(z, a); NTL_OPT_RETURN(RR, z); }

void log10(RR& res, const RR& x);
inline RR log10(const RR& a)
   { RR z; log10(z, a); NTL_OPT_RETURN(RR, z); }

void expm1(RR& res, const RR& x);
inline RR expm1(const RR& a)
   { RR z; expm1(z, a); NTL_OPT_RETURN(RR, z); }

void log1p(RR& res, const RR& x);
inline RR log1p(const RR& a)
   { RR z; log1p(z, a); NTL_OPT_RETURN(RR, z); }

void pow(RR& res, const RR& x, const RR& y);
inline RR pow(const RR& x, const RR& y)
   { RR z; pow(z, x, y); NTL_OPT_RETURN(RR, z); }

void ComputePi(RR& res);
inline RR ComputePi_RR()
   { RR z; ComputePi(z); NTL_OPT_RETURN(RR, z); }

void sin(RR& res, const RR& x);
inline RR sin(const RR& a)
   { RR z; sin(z, a); NTL_OPT_RETURN(RR, z); }

void cos(RR& res, const RR& x);
inline RR cos(const RR& a)
   { RR z; cos(z, a); NTL_OPT_RETURN(RR, z); }




NTL_CLOSE_NNS

#endif
