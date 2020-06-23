
#ifndef NTL_quad_float__H
#define NTL_quad_float__H


// The quad_float module is derived from the doubledouble
// library originally developed by Keith Briggs:
//    http://keithbriggs.info/doubledouble.html
// I attach the original copyright notice.


/*

Copyright (C) 1997 Keith Martin Briggs

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/



#include <NTL/ZZ.h>

NTL_OPEN_NNS


class quad_float {
public:
  double hi, lo;

  // Constructors
  quad_float() : hi(0), lo(0)  {}

  explicit quad_float(double a) : hi(0), lo(0) { *this = a; }


  inline quad_float& operator=(double x);

  static 
  NTL_CHEAP_THREAD_LOCAL 
  long oprec;

  static void SetOutputPrecision(long p);
  static long OutputPrecision() { return oprec; }

  quad_float(double x, double y) : hi(x), lo(y) { } // internal use only
  // FIXME: add a special argument to this to make it more "internal"


};  // end class quad_float


NTL_DECLARE_RELOCATABLE((quad_float*))




#if (NTL_BITS_PER_LONG < NTL_DOUBLE_PRECISION)

// FIXME: we could make this <=, and even BPL <= DP+1 for 
// conversions from signed long...but this is mainly academic

inline quad_float to_quad_float(long n) { return quad_float(n, 0); }
inline quad_float to_quad_float(unsigned long n) { return quad_float(n, 0); }

#else


quad_float to_quad_float(long n);
quad_float to_quad_float(unsigned long n);

#endif



void quad_float_normalize(quad_float& z, const double& xhi, const double& xlo);

void quad_float_in_place_add(quad_float& x, const quad_float& y);
void quad_float_in_place_sub(quad_float& x, const quad_float& y);
void quad_float_in_place_mul(quad_float& x, const quad_float& y);
void quad_float_in_place_div(quad_float& x, const quad_float& y);

void quad_float_in_place_negate(quad_float& x);
void quad_float_in_place_sqrt(quad_float& y, double& c_ref);

void quad_float_PrecisionOK(long&, const double&);




#if (NTL_BITS_PER_INT < NTL_DOUBLE_PRECISION)

inline quad_float to_quad_float(int n) { return quad_float(n, 0); }
inline quad_float to_quad_float(unsigned int n) { return quad_float(n, 0); }
 
#else

inline quad_float to_quad_float(int n) 
   { return to_quad_float(long(n)); }
inline quad_float to_quad_float(unsigned int n) 
   { return to_quad_float((unsigned long) n); }

#endif





// NOTE: for extended precision platforms, the call to TrueDouble
// should remove it
inline quad_float to_quad_float(double x) 
   { return quad_float(TrueDouble(x), 0); }

inline quad_float to_quad_float(float x) 
   { return to_quad_float(double(x)); }

inline quad_float& quad_float::operator=(double x) 
   { *this = to_quad_float(x); return *this; }





inline quad_float& operator+= (quad_float& x, const quad_float& y)
   { quad_float_in_place_add(x, y); return x; }
inline quad_float& operator-= (quad_float& x, const quad_float& y)
   { quad_float_in_place_sub(x, y); return x; }
inline quad_float& operator*= (quad_float& x, const quad_float& y)
   { quad_float_in_place_mul(x, y); return x; }
inline quad_float& operator/= (quad_float& x, const quad_float& y)
   { quad_float_in_place_div(x, y); return x; }

inline quad_float operator-(const quad_float& x)
   { quad_float xx = x; quad_float_in_place_negate(xx); return xx; }



inline quad_float operator+(const quad_float& x, const quad_float& y)
   { quad_float xx = x; xx += y; return xx; }

inline quad_float operator-(const quad_float& x, const quad_float& y)
   { quad_float xx = x; xx -= y; return xx; }

inline quad_float operator*(const quad_float& x, const quad_float& y)
   { quad_float xx = x; xx *= y; return xx; }

inline quad_float operator/(const quad_float& x, const quad_float& y)
   { quad_float xx = x; xx /= y; return xx; }







inline quad_float operator+(const quad_float& x, double y )
   { return x + to_quad_float(y); }

inline quad_float operator+(double x, const quad_float& y)
   { return to_quad_float(x) + y; }

inline quad_float operator-(const quad_float& x, double y )
   { return x - to_quad_float(y); }

inline quad_float operator-(double x, const quad_float& y)
   { return to_quad_float(x) - y; }

inline quad_float operator*(const quad_float& x, double y )
   { return x * to_quad_float(y); }

inline quad_float operator*(double x, const quad_float& y)
   { return to_quad_float(x) * y; }

inline quad_float operator/(const quad_float& x, double y )
   { return x / to_quad_float(y); }

inline quad_float operator/(double x, const quad_float& y)
   { return to_quad_float(x) / y; }



inline quad_float& operator += (quad_float& x, double y)
   { x += to_quad_float(y); return x; }

inline quad_float& operator-= (quad_float& x, double y)
   { x -= to_quad_float(y); return x; }

inline quad_float& operator*= (quad_float& x, double y)
   { x *= to_quad_float(y); return x; }

inline quad_float& operator/= (quad_float& x, double y)
   { x /= to_quad_float(y); return x; }

inline quad_float& operator++(quad_float& a) { a += 1.0; return a; }
inline void operator++(quad_float& a, int) { a += 1.0; }

inline quad_float& operator--(quad_float& a) { a -= 1.0; return a; }
inline void operator--(quad_float& a, int) { a -= 1.0; }


long operator> (const quad_float& x, const quad_float& y);
long operator>=(const quad_float& x, const quad_float& y);
long operator< (const quad_float& x, const quad_float& y);
long operator<=(const quad_float& x, const quad_float& y);
long operator==(const quad_float& x, const quad_float& y);
long operator!=(const quad_float& x, const quad_float& y);

inline long operator> (const quad_float& x, double y) 
   { return x > to_quad_float(y); }
inline long operator> (double x, const quad_float& y)
   { return to_quad_float(x) > y; }

inline long operator>=(const quad_float& x, double y) 
   { return x >= to_quad_float(y); }
inline long operator>=(double x, const quad_float& y)
   { return to_quad_float(x) >= y; }

inline long operator< (const quad_float& x, double y) 
   { return x < to_quad_float(y); }
inline long operator< (double x, const quad_float& y)
   { return to_quad_float(x) < y; }

inline long operator<=(const quad_float& x, double y) 
   { return x <= to_quad_float(y); }
inline long operator<=(double x, const quad_float& y)
   { return to_quad_float(x) <= y; }

inline long operator!=(const quad_float& x, double y) 
   { return x != to_quad_float(y); }
inline long operator!=(double x, const quad_float& y)
   { return to_quad_float(x) != y; }

inline long operator==(const quad_float& x, double y) 
   { return x == to_quad_float(y); }
inline long operator==(double x, const quad_float& y)
   { return to_quad_float(x) == y; }


inline long sign(const quad_float& x){
  if (x.hi>0.0) return 1; else if (x.hi<0.0) return -1; else return 0;
}

long compare(const quad_float&, const quad_float&);

inline long compare(const quad_float& x, double y)
   { return compare(x, to_quad_float(y)); }

inline long compare(double x, const quad_float& y)
   { return compare(to_quad_float(x), y); }



NTL_SNS istream& operator >> (NTL_SNS istream&, quad_float&);
NTL_SNS ostream& operator << (NTL_SNS ostream&, const quad_float&);


quad_float sqrt(const quad_float&);
quad_float floor(const quad_float&);
quad_float ceil(const quad_float&);
quad_float trunc(const quad_float&);
quad_float fabs(const quad_float&);

void power(quad_float&, const quad_float&, long);
inline quad_float power(const quad_float& x, long e)
   { quad_float z; power(z, x, e); return z; }

void power2(quad_float&, long);
inline quad_float power2_quad_float(long e)
   { quad_float z; power2(z, e); return z; }


long to_long(const quad_float&);
inline int to_int(const quad_float& x) { return to_int(to_long(x)); }

inline double to_double(const quad_float& x) { return x.hi; }

inline float to_float(const quad_float& x) { return float(x.hi); }


inline void conv(quad_float& x, int a) { x = to_quad_float(a); }
inline void conv(quad_float& x, long a) { x = to_quad_float(a); }

inline void conv(quad_float& x, unsigned int a) { x = to_quad_float(a); }
inline void conv(quad_float& x, unsigned long a) { x = to_quad_float(a); }

inline void conv(quad_float& x, float a) { x = to_quad_float(a); }
inline void conv(quad_float& x, double a) { x = to_quad_float(a); }


inline void conv(long& x, const quad_float& a) { x = to_long(a); }
inline void conv(int& x, const quad_float& a) { x = to_int(a); }
inline void conv(double& x, const quad_float& a) { x = to_double(a); }
inline void conv(float& x, const quad_float& a) { x = to_float(a); }

void conv(quad_float&, const ZZ&);
inline quad_float to_quad_float(const ZZ& x)
   { quad_float z; conv(z, x); return z; }

void conv(ZZ&, const quad_float&);
inline ZZ to_ZZ(const quad_float& a)
   { ZZ x; conv(x, a); NTL_OPT_RETURN(ZZ, x); }

inline void conv(quad_float& x, const quad_float& a)
   { x = a; }
inline quad_float to_quad_float(const quad_float& a)
   { return a; }

inline quad_float to_quad_float(const char *s)
{ quad_float res; conv(res, s); return res; }



/* additional legacy conversions for v6 conversion regime */

inline void conv(unsigned int& x, const quad_float& a)
   { long z; conv(z, a); conv(x, z); }

inline void conv(unsigned long& x, const quad_float& a)
   { long z; conv(z, a); conv(x, z); }


/* ------------------------------------- */

long IsFinite(quad_float *x);


quad_float ldexp(const quad_float& x, long exp);

quad_float exp(const quad_float& x);
quad_float log(const quad_float& x);

void random(quad_float& x);
quad_float random_quad_float();


NTL_CLOSE_NNS

#endif
