#include <NTL/quad_float.h>
#include <NTL/RR.h>

#include <cfloat>

NTL_START_IMPL


#if (NTL_BITS_PER_LONG >= NTL_DOUBLE_PRECISION)


quad_float to_quad_float(long n)
{
   double xhi, xlo;

   xhi = TrueDouble(n);

   // Because we are assuming 2's compliment integer
   // arithmetic, the following prevents long(xhi) from overflowing.

   if (n > 0)
      xlo = TrueDouble(n+long(-xhi));
   else
      xlo = TrueDouble(n-long(xhi));

   // renormalize...just to be safe

   quad_float z;
   quad_float_normalize(z, xhi, xlo);
   return z;
}

quad_float to_quad_float(unsigned long n)
{
   double xhi, xlo, t;

   const double bnd = double(1L << (NTL_BITS_PER_LONG-2))*4.0;

   xhi = TrueDouble(n);
   
   if (xhi >= bnd)
      t = xhi - bnd;
   else
      t = xhi;

   // we use the "to_long" function here to be as portable as possible.
   long llo = to_long(n - (unsigned long)(t));
   xlo = TrueDouble(llo);

   quad_float z;
   quad_float_normalize(z, xhi, xlo);
   return z;
}
#endif


NTL_CHEAP_THREAD_LOCAL
long quad_float::oprec = 10;

void quad_float::SetOutputPrecision(long p)
{
   if (p < 1) p = 1;

   if (NTL_OVERFLOW(p, 1, 0)) 
      ResourceError("quad_float: output precision too big");

   oprec = p;
}



void power(quad_float& z, const quad_float& a, long e)
{
   quad_float res, u;
   unsigned long k;

   if (e < 0)
      k = -((unsigned long) e);
   else
      k = e;

   res = 1.0;
   u = a;

   while (k) {
      if (k & 1)
         res = res * u;

      k = k >> 1;
      if (k)
         u = u * u;
   }

   if (e < 0)
      z = 1.0/res;
   else
      z = res;
}


void power2(quad_float& z, long e)
{
   z.hi = _ntl_ldexp(1.0, e);
   z.lo = 0;
}


long to_long(const quad_float& x)
{
   double fhi, flo;

   fhi = floor(x.hi);

   if (fhi == x.hi) 
      flo = floor(x.lo);
   else
      flo = 0;

   // the following code helps to prevent unnecessary integer overflow,
   // and guarantees that to_long(to_quad_float(a)) == a, for all long a,
   // provided long's are not too wide.

   if (fhi > 0)
      return long(flo) - long(-fhi);
   else
      return long(fhi) + long(flo);
}



// This version of ZZ to quad_float coversion relies on the
// precise rounding rules implemented by the ZZ to double conversion.


void conv(quad_float& z, const ZZ& a)
{
   double xhi, xlo;

   conv(xhi, a);

   if (!IsFinite(&xhi)) {
      z.hi = xhi;
      z.lo = 0;
      return;
   }

   NTL_ZZRegister(t);

   conv(t, xhi);
   sub(t, a, t);

   conv(xlo, t);

   quad_float_normalize(z, xhi, xlo);
} 

void conv(ZZ& z, const quad_float& x)
{ 
   NTL_ZZRegister(t1);
   NTL_ZZRegister(t2);
   NTL_ZZRegister(t3);

   double fhi, flo;

   fhi = floor(x.hi);

   if (fhi == x.hi) {
      flo = floor(x.lo);

      conv(t1, fhi);
      conv(t2, flo);

      add(z, t1, t2);
   }
   else
      conv(z, fhi);
}



ostream& operator<<(ostream& s, const quad_float& a)
{
   quad_float aa = a;

   if (!IsFinite(&aa)) {
      s << "NaN";
      return s;
   }

   RRPush push;
   RROutputPush opush;

   RR::SetPrecision(long(3.33*quad_float::oprec) + 10);
   RR::SetOutputPrecision(quad_float::oprec);

   NTL_TLS_LOCAL(RR, t);

   conv(t, a);
   s << t;

   return s;
}

istream& operator>>(istream& s, quad_float& x)
{
   RRPush push;
   RR::SetPrecision(4*NTL_DOUBLE_PRECISION);

   NTL_TLS_LOCAL(RR, t);
   NTL_INPUT_CHECK_RET(s, s >> t);
   conv(x, t);

   return s;
}

void random(quad_float& x)
{
   RRPush push;
   RR::SetPrecision(4*NTL_DOUBLE_PRECISION);

   NTL_TLS_LOCAL(RR, t);
   random(t);
   conv(x, t);
}

quad_float random_quad_float()
{
   quad_float x;
   random(x);
   return x;
}
      
long IsFinite(quad_float *x)
{
   return IsFinite(&x->hi) && IsFinite(&x->lo);
}


quad_float floor(const quad_float& x)
{
   double fhi = floor(x.hi);

   if (fhi != x.hi)
      return quad_float(fhi, 0.0);
   else {
      double flo = floor(x.lo);
      quad_float z;
      quad_float_normalize(z, fhi, flo);
      return z;
   }
}


quad_float ceil(const quad_float& x) { 
  return -floor(-x);
}

quad_float trunc(const quad_float& x) { 
  if (x>=0.0) return floor(x); else return -floor(-x);
}



long compare(const quad_float& x, const quad_float& y)
{
   if (x.hi > y.hi) 
      return 1;
   else if (x.hi < y.hi)
      return -1;
   else if (x.lo > y.lo)
      return 1;
   else if (x.lo < y.lo) 
      return -1;
   else
      return 0;
}


quad_float fabs(const quad_float& x) 
{ if (x.hi>=0.0) return x; else return -x; }


quad_float ldexp(const quad_float& x, long exp) { // x*2^exp
   double xhi, xlo;
   quad_float z;

   xhi = _ntl_ldexp(x.hi, exp);
   xlo = _ntl_ldexp(x.lo, exp);

   quad_float_normalize(z, xhi, xlo);
   return z;
}


quad_float exp(const quad_float& x) { // New version 97 Aug 05
/*
!  Calculate a quadruple-precision exponential
!  Method:
!   x    x.log2(e)    nint[x.log2(e)] + frac[x.log2(e)]
!  e  = 2          = 2
!
!                     iy    fy
!                  = 2   . 2
!  Then
!   fy    y.loge(2)
!  2   = e
!
!  Now y.loge(2) will be less than 0.3466 in absolute value.
!  This is halved and a Pade aproximation is used to approximate e^x over
!  the region (-0.1733, +0.1733).   This approximation is then squared.
*/
  if (x.hi<DBL_MIN_10_EXP*2.302585092994045684017991) 
    return to_quad_float(0.0);
  if (x.hi>DBL_MAX_10_EXP*2.302585092994045684017991) {
    ResourceError("exp(quad_float): overflow");
  }

  static const quad_float Log2 = to_quad_float("0.6931471805599453094172321214581765680755");
  // GLOBAL (assumes C++11 thread-safe init)

  quad_float y,temp,ysq,sum1,sum2;
  long iy;
  y=x/Log2;
  temp = floor(y+0.5);
  iy = to_long(temp);
  y=(y-temp)*Log2;
  y=ldexp(y,-1L);
  ysq=y*y;
  sum1=y*((((ysq+3960.0)*ysq+2162160.0)*ysq+302702400.0)*ysq+8821612800.0);
  sum2=(((90.0*ysq+110880.0)*ysq+30270240.0)*ysq+2075673600.0)*ysq+17643225600.0;
/*
!                     sum2 + sum1         2.sum1
! Now approximation = ----------- = 1 + ----------- = 1 + 2.temp
!                     sum2 - sum1       sum2 - sum1
!
! Then (1 + 2.temp)^2 = 4.temp.(1 + temp) + 1
*/
  temp=sum1/(sum2-sum1);
  y=temp*(temp+1);
  y=ldexp(y,2L);
  return ldexp(y+1,iy);
}

quad_float log(const quad_float& t) { // Newton method. See Bailey, MPFUN
  if (t.hi <= 0.0) {
    ArithmeticError("log(quad_float): argument must be positive");
  }

  quad_float s = to_quad_float(log(t.hi));
  // NOTE: in case log yields excess precision, this assumes
  // that to_quad_float removes it

  quad_float e = exp(s);
  return s+(t-e)/e;  // Newton step
}

quad_float sqrt(const quad_float& y)
{
  if (y.hi < 0.0)
    ArithmeticError("quad_float: square root of negative number");
  if (y.hi == 0.0) return quad_float(0.0,0.0);

  double c = TrueDouble(sqrt(y.hi));
  // NOTE: we call TrueDouble, just in case sqrt yields excess precision

  quad_float yy = y;
  quad_float_in_place_sqrt(yy, c);
  return yy;
}


long operator> (const quad_float& x, const quad_float& y) {
   return (x.hi> y.hi) || (x.hi==y.hi && x.lo> y.lo); }
long operator>=(const quad_float& x, const quad_float& y) {
   return (x.hi>y.hi) || (x.hi==y.hi && x.lo>=y.lo); }
long operator< (const quad_float& x, const quad_float& y) {
   return (x.hi< y.hi) || (x.hi==y.hi && x.lo< y.lo); }
long operator<=(const quad_float& x, const quad_float& y) {
   return (x.hi<y.hi) || (x.hi==y.hi && x.lo<=y.lo); }
long operator==(const quad_float& x, const quad_float& y)
   { return x.hi==y.hi && x.lo==y.lo; }
long operator!=(const quad_float& x, const quad_float& y)
   { return x.hi!=y.hi || x.lo!=y.lo; }


NTL_END_IMPL
