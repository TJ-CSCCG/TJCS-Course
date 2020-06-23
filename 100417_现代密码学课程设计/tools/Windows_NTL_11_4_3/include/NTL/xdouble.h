#ifndef NTL_xdouble__H
#define NTL_xdouble__H

#include <NTL/ZZ.h>

NTL_OPEN_NNS

// NTL_XD_HBOUND = 2^{max(NTL_DOUBLE_PRECISION,NTL_BITS_PER_LONG)+4}

#if (NTL_DOUBLE_PRECISION > NTL_BITS_PER_LONG)

#define NTL_XD_HBOUND (NTL_FDOUBLE_PRECISION*32.0)
#define NTL_XD_HBOUND_LOG (NTL_DOUBLE_PRECISION+4)

#else

#define NTL_XD_HBOUND (double(1L << (NTL_BITS_PER_LONG - 2))*64.0)
#define NTL_XD_HBOUND_LOG (NTL_BITS_PER_LONG+4)

#endif

#define NTL_XD_HBOUND_INV (double(1)/NTL_XD_HBOUND)

#define NTL_XD_BOUND (NTL_XD_HBOUND*NTL_XD_HBOUND)
#define NTL_XD_BOUND_INV (double(1)/NTL_XD_BOUND)


class xdouble {

public:

double x;
long e;

xdouble() : x(0), e(0) { }
explicit xdouble(double a) : x(0), e(0) { *this = a; }

inline xdouble& operator=(double a);



void normalize();

static 
NTL_CHEAP_THREAD_LOCAL 
long oprec;

static void SetOutputPrecision(long p);
static long OutputPrecision() { return oprec; }

double mantissa() const { return x; }
long exponent() const { return e; }

xdouble(double xx, long ee) : x(xx), e(ee) { } // internal use only

};


NTL_DECLARE_RELOCATABLE((xdouble*))

inline xdouble to_xdouble(int a) { return xdouble(a, 0); }
inline xdouble to_xdouble(long a) { return xdouble(a, 0); }
inline xdouble to_xdouble(unsigned int a) { return xdouble(a, 0); }
inline xdouble to_xdouble(unsigned long a) { return xdouble(a, 0); }

xdouble to_xdouble(double a);
inline xdouble to_xdouble(float a) { return to_xdouble(double(a)); }
inline xdouble to_xdouble(const char *a) 
{ xdouble res; conv(res, a); return res; }


inline xdouble& xdouble::operator=(double a)
   { *this = to_xdouble(a); return *this; }

xdouble operator+(const xdouble& a, const xdouble& b);
inline xdouble operator+(const xdouble& a, double b)
   { return a + to_xdouble(b); }
inline xdouble operator+(double a, const xdouble& b)
   { return to_xdouble(a) + b; }

xdouble operator-(const xdouble& a, const xdouble& b);
inline xdouble operator-(const xdouble& a, double b)
   { return a - to_xdouble(b); }
inline xdouble operator-(double a, const xdouble& b)
   { return to_xdouble(a) - b; }

xdouble operator*(const xdouble& a, const xdouble& b);
inline xdouble operator*(const xdouble& a, double b)
   { return a * to_xdouble(b); }
inline xdouble operator*(double a, const xdouble& b)
   { return to_xdouble(a) * b; }

xdouble operator/(const xdouble& a, const xdouble& b);
inline xdouble operator/(const xdouble& a, double b)
   { return a / to_xdouble(b); }
inline xdouble operator/(double a, const xdouble& b)
   { return to_xdouble(a) / b; }

xdouble operator-(const xdouble& a);



inline xdouble& operator+=(xdouble& a, const xdouble& b) 
   { a = a + b; return a; }
inline xdouble& operator+=(xdouble& a, double b)
   { a = a + b; return a; }

inline xdouble& operator-=(xdouble& a, const xdouble& b) 
   { a = a - b; return a; }
inline xdouble& operator-=(xdouble& a, double b) 
   { a = a - b; return a; }

inline xdouble& operator*=(xdouble& a, const xdouble& b) 
   { a = a * b; return a; }
inline xdouble& operator*=(xdouble& a, double b) 
   { a = a * b; return a; }

inline xdouble& operator/=(xdouble& a, const xdouble& b) 
   { a = a / b; return a; }
inline xdouble& operator/=(xdouble& a, double b) 
   { a = a / b; return a; }

inline xdouble& operator++(xdouble& a) { a = a + to_xdouble(1); return a; }
inline xdouble& operator--(xdouble& a) { a = a - to_xdouble(1); return a; }

inline void operator++(xdouble& a, int) { a = a + to_xdouble(1); }
inline void operator--(xdouble& a, int) { a = a - to_xdouble(1); }

long compare(const xdouble& a, const xdouble& b);
inline long compare(const xdouble& a, double b)
   { return compare(a, to_xdouble(b)); }
inline long compare(double a, const xdouble& b)
   { return compare(to_xdouble(a), b); }

long sign(const xdouble& a);

inline long operator==(const xdouble& a, const xdouble& b) 
   { return compare(a, b) == 0; }
inline long operator!=(const xdouble& a, const xdouble& b) 
   { return compare(a, b) != 0; }
inline long operator<=(const xdouble& a, const xdouble& b) 
   { return compare(a, b) <= 0; }
inline long operator>=(const xdouble& a, const xdouble& b) 
   { return compare(a, b) >= 0; }
inline long operator <(const xdouble& a, const xdouble& b) 
   { return compare(a, b)  < 0; }
inline long operator >(const xdouble& a, const xdouble& b) 
   { return compare(a, b)  > 0; }

inline long operator==(const xdouble& a, double b) 
   { return compare(a, b) == 0; }
inline long operator!=(const xdouble& a, double b) 
   { return compare(a, b) != 0; }
inline long operator<=(const xdouble& a, double b) 
   { return compare(a, b) <= 0; }
inline long operator>=(const xdouble& a, double b) 
   { return compare(a, b) >= 0; }
inline long operator <(const xdouble& a, double b) 
   { return compare(a, b)  < 0; }
inline long operator >(const xdouble& a, double b) 
   { return compare(a, b)  > 0; }

inline long operator==(double a, const xdouble& b) 
   { return compare(a, b) == 0; }
inline long operator!=(double a, const xdouble& b) 
   { return compare(a, b) != 0; }
inline long operator<=(double a, const xdouble& b) 
   { return compare(a, b) <= 0; }
inline long operator>=(double a, const xdouble& b) 
   { return compare(a, b) >= 0; }
inline long operator <(double a, const xdouble& b) 
   { return compare(a, b)  < 0; }
inline long operator >(double a, const xdouble& b) 
   { return compare(a, b)  > 0; }


void conv(ZZ& x, const xdouble& a);
// x = floor(a);

inline ZZ to_ZZ(const xdouble& a)
   { ZZ x; conv(x, a); NTL_OPT_RETURN(ZZ, x); }



xdouble to_xdouble(const ZZ& a);
inline void conv(xdouble& z, const ZZ& a)
   { z = to_xdouble(a); }

void conv(double& x, const xdouble& a);
inline double to_double(const xdouble& a)
   { double z; conv(z, a); return z; }

inline void conv(float& x, const xdouble& a) 
   { double t; conv(t, a); x = float(t); }
inline float to_float(const xdouble& a)
   { float z; conv(z, a); return z; }


inline void conv(long& x, const xdouble& a)
   { double z; conv(z, a); x = long(NTL_SNS floor(z)); }
inline long to_long(const xdouble& a)
   { long z; conv(z, a); return z; }




inline void conv(int& x, const xdouble& a)
   { double z; conv(z, a); x = int(NTL_SNS floor(z)); }
inline int to_int(const xdouble& a)
   { int z; conv(z, a); return z; }

inline void conv(xdouble& x, const xdouble& a)
   { x = a; }
inline xdouble to_xdouble(const xdouble& a)
   { return a; }


inline void conv(xdouble& x, int a) { x = to_xdouble(a); }
inline void conv(xdouble& x, long a) { x = to_xdouble(a); }

inline void conv(xdouble& x, unsigned int a) { x = to_xdouble(a); }
inline void conv(xdouble& x, unsigned long a) { x = to_xdouble(a); }

inline void conv(xdouble& x, float a) { x = to_xdouble(a); }
inline void conv(xdouble& x, double a) { x = to_xdouble(a); }



/* additional legacy conversions for v6 conversion regime */


inline void conv(unsigned int& x, const xdouble& a)
   { long z; conv(z, a); conv(x, z); }

inline void conv(unsigned long& x, const xdouble& a)
   { long z; conv(z, a); conv(x, z); }

/* ------------------------------------- */

NTL_SNS ostream& operator<<(NTL_SNS ostream& s, const xdouble& a);

NTL_SNS istream& operator>>(NTL_SNS istream& s, xdouble& x);

xdouble trunc(const xdouble& a);
xdouble floor(const xdouble& a);
xdouble ceil(const xdouble& a);
xdouble fabs(const xdouble& a);
xdouble sqrt(const xdouble& a);

void power(xdouble& z, const xdouble& a, const ZZ& e);
inline xdouble power(const xdouble& a, const ZZ& e)
   { xdouble z; power(z, a, e); return z; }

void power(xdouble& z, const xdouble& a, long e);
inline xdouble power(const xdouble& a, long e)
   { xdouble z; power(z, a, e); return z; }

void power2(xdouble& z, long e);
inline xdouble power2_xdouble(long e)
   { xdouble z; power2(z, e); return z; }


void MulAdd(xdouble& z, const xdouble& a, const xdouble& b, const xdouble& c);
inline xdouble MulAdd(const xdouble& a, const xdouble& b, 
                            const xdouble& c)
   { xdouble z; MulAdd(z, a, b, c); return z; }


void MulSub(xdouble& z, const xdouble& a, const xdouble& b, const xdouble& c);
inline xdouble MulSub(const xdouble& a, const xdouble& b, 
                            const xdouble& c)
   { xdouble z; MulSub(z, a, b, c); return z; }

double log(const xdouble& a);
xdouble xexp(double x);


inline xdouble exp(const xdouble& x) { return xexp(to_double(x)); }


NTL_CLOSE_NNS

#endif
