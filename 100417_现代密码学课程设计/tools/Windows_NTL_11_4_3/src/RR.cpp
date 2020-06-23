
#include <NTL/RR.h>



NTL_START_IMPL


// FIXME: I just converted all the static RR's to thread local static RR's.
// Perhaps I should at some point make the equivalent of an RR Register.
// But be careful: certain computations, like ComputePi, actually cache
// results, so that will take more work.  In any case, RR is not a high
// priority right now.


NTL_CHEAP_THREAD_LOCAL
long RR::prec = 150;

void RR::SetPrecision(long p)
{
   if (p < 53)
      p = 53;

   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("RR: precision too high");

   prec = p;
}

NTL_CHEAP_THREAD_LOCAL
long RR::oprec = 10;

void RR::SetOutputPrecision(long p)
{
   if (p < 1)
      p = 1;

   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("RR: output precision too high");

   oprec = p;
}



static
void normalize1(RR& z, const ZZ& y_x, long y_e, long prec, long residual)
{
   long len = NumBits(y_x);

   if (len > prec) {
      long correction = ZZ_RoundCorrection(y_x, len - prec, residual);

      RightShift(z.x, y_x, len - prec);

      if (correction) 
         add(z.x, z.x, correction);

      z.e = y_e + len - prec;
   }
   else if (len == 0) {
      clear(z.x);
      z.e = 0;
   }
   else {
      z.x = y_x;
      z.e = y_e;
   }

   if (!IsOdd(z.x))
      z.e += MakeOdd(z.x);

   if (z.e >= NTL_OVFBND)
      ResourceError("RR: overflow");

   if (z.e <= -NTL_OVFBND)
      ResourceError("RR: underflow");
}

void normalize(RR& z, const RR& y, long residual = 0)
{
   normalize1(z, y.x, y.e, RR::prec, residual);
}

void MakeRR(RR& z, const ZZ& a,  long e)
{
   if (e >= NTL_OVFBND)
      ResourceError("MakeRR: e too big");

   if (e <= -NTL_OVFBND)
      ResourceError("MakeRR: e too small");

   normalize1(z, a, e, RR::prec, 0);
}

void MakeRRPrec(RR& x, const ZZ& a, long e, long p)
{
   if (p < 1)
      LogicError("MakeRRPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("MakeRRPrec: precsion too big");

   RRPush push;
   RR::prec = p;
   MakeRR(x, a, e);
}

void random(RR& z)
{
   NTL_TLS_LOCAL(RR, t);
   RandomBits(t.x, RR::prec); 
   t.e = -RR::prec;
   normalize(z, t);
}


static inline 
void xcopy(RR& x, const RR& a)
   { normalize(x, a); }

// xcopy emulates old assignment semantics...
// many routines here implicitly assume assignment normalizes,
// but that is no longer the case as of v3.0.

void ConvPrec(RR& x, const RR& a, long p)
{
   if (p < 1)
      LogicError("ConvPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("ConvPrec: precsion too big");

   RRPush push;
   RR::prec = p;
   normalize(x, a);
}

void RoundToPrecision(RR& x, const RR& a, long p)
{
   ConvPrec(x, a, p);
}
   

void conv(RR& x, const RR& a)
{
   normalize(x, a);
}


long IsZero(const RR& a)
{
   return IsZero(a.x);
}

long IsOne(const RR& a)
{
   return a.e == 0 && IsOne(a.x);
}

long sign(const RR& a)
{
   return sign(a.x);
}

void clear(RR& z)
{
   z.e = 0;
   clear(z.x);
}

void set(RR& z)
{
   z.e = 0;
   set(z.x);
}


void add(RR& z, const RR& a, const RR& b)
{
   NTL_TLS_LOCAL(RR, t);

   if (IsZero(a.x)) {
      xcopy(z, b);
      return;
   }

   if (IsZero(b.x)) {
      xcopy(z, a);
      return;
   }

   if (a.e > b.e) {
      if (a.e-b.e - max(RR::prec-NumBits(a.x),0) >= NumBits(b.x) + 2)
         normalize(z, a, sign(b));
      else {
         LeftShift(t.x, a.x, a.e-b.e);
         add(t.x, t.x, b.x);
         t.e = b.e;
         normalize(z, t);
      }
   }
   else if (a.e < b.e) {
      if (b.e-a.e - max(RR::prec-NumBits(b.x),0) >= NumBits(a.x) + 2)
         normalize(z, b, sign(a));
      else {
         LeftShift(t.x, b.x, b.e-a.e);
         add(t.x, t.x, a.x);
         t.e = a.e;
         normalize(z, t);
      }
   }
   else {
      add(t.x, a.x, b.x);
      t.e = a.e;
      normalize(z, t);
   }
}

void AddPrec(RR& x, const RR& a, const RR& b, long p)
{
   if (p < 1)
      LogicError("AddPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("AddPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   add(x, a, b);
}

void sub(RR& z, const RR& a, const RR& b)
{
   NTL_TLS_LOCAL(RR, t);

   if (IsZero(a.x)) {
      negate(z, b);
      return;
   }

   if (IsZero(b.x)) {
      xcopy(z, a);
      return;
   }

   if (a.e > b.e) {
      if (a.e-b.e - max(RR::prec-NumBits(a.x),0) >= NumBits(b.x) + 2)
         normalize(z, a, -sign(b));
      else {
         LeftShift(t.x, a.x, a.e-b.e);
         sub(t.x, t.x, b.x);
         t.e = b.e;
         xcopy(z, t);
      }
   }
   else if (a.e < b.e) {
      if (b.e-a.e - max(RR::prec-NumBits(b.x),0) >= NumBits(a.x) + 2) {
         normalize(z, b, -sign(a));
         negate(z.x, z.x);
      }
      else {
         LeftShift(t.x, b.x, b.e-a.e);
         sub(t.x, a.x, t.x);
         t.e = a.e;
         xcopy(z, t);
      }
   }
   else {
      sub(t.x, a.x, b.x);
      t.e = a.e;
      normalize(z, t);
   }
}

void SubPrec(RR& x, const RR& a, const RR& b, long p)
{
   if (p < 1)
      LogicError("SubPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("SubPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   sub(x, a, b);
}

void negate(RR& z, const RR& a)
{
   xcopy(z, a);
   negate(z.x, z.x);
}

void NegatePrec(RR& x, const RR& a, long p)
{
   if (p < 1)
      LogicError("NegatePrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("NegatePrec: bad precsion");

   RRPush push;
   RR::prec = p;
   negate(x, a);
}

void abs(RR& z, const RR& a)
{
   xcopy(z, a);
   abs(z.x, z.x);
}

void AbsPrec(RR& x, const RR& a, long p)
{
   if (p < 1)
      LogicError("AbsPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("AbsPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   abs(x, a);
}


void mul(RR& z, const RR& a, const RR& b)
{
   NTL_TLS_LOCAL(RR, t);

   mul(t.x, a.x, b.x);
   t.e = a.e + b.e;
   xcopy(z, t);
}

void MulPrec(RR& x, const RR& a, const RR& b, long p)
{
   if (p < 1)
      LogicError("MulPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("MulPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   mul(x, a, b);
}


void sqr(RR& z, const RR& a)
{
   NTL_TLS_LOCAL(RR, t);

   sqr(t.x, a.x);
   t.e = a.e + a.e;
   xcopy(z, t);
}

void SqrPrec(RR& x, const RR& a, long p)
{
   if (p < 1)
      LogicError("SqrPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("SqrPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   sqr(x, a);
}



void div(RR& z, const RR& a, const RR& b)
{
   if (IsZero(b))
      ArithmeticError("RR: division by zero");

   if (IsZero(a)) {
      clear(z);
      return;
   }

   long la = NumBits(a.x);
   long lb = NumBits(b.x);

   long neg = (sign(a) != sign(b));

   long k = RR::prec - la + lb + 1;
   if (k < 0) k = 0;

   NTL_TLS_LOCAL(RR, t);
   NTL_ZZRegister(A);
   NTL_ZZRegister(B);
   NTL_ZZRegister(R);

   abs(A, a.x);
   LeftShift(A, A, k);

   abs(B, b.x);
   DivRem(t.x, R, A, B);

   t.e = a.e - b.e - k;

   normalize(z, t, !IsZero(R));

   if (neg)
      negate(z.x, z.x);
}

void DivPrec(RR& x, const RR& a, const RR& b, long p)
{
   if (p < 1)
      LogicError("DivPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("DivPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   div(x, a, b);
}


void SqrRoot(RR& z, const RR& a)
{
   if (sign(a) < 0)
      ArithmeticError("RR: attempt to take square root of negative number");

   if (IsZero(a)) {
      clear(z);
      return;
   }

   RR t;
   ZZ T1, T2;
   long k;

   k = 2*RR::prec - NumBits(a.x) + 1;

   if (k < 0) k = 0;

   if ((a.e - k) & 1) k++;

   LeftShift(T1, a.x, k);
   // since k >= 2*prec - bits(a) + 1, T1 has at least 2*prec+1 bits,           
   // thus T1 >= 2^(2*prec)                                                     

   SqrRoot(t.x, T1); // t.x >= 2^prec thus t.x contains the round bit           
   t.e = (a.e - k)/2;
   sqr(T2, t.x);  

   // T1-T2 is the (lower part of the) sticky bit                               
   normalize(z, t, T2 < T1);
}




void SqrRootPrec(RR& x, const RR& a, long p)
{
   if (p < 1)
      LogicError("SqrRootPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("SqrRootPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   SqrRoot(x, a);
}




long compare(const RR& a, const RR& b)
{
   NTL_TLS_LOCAL(RR, t);

   SubPrec(t, a, b, 1);
   return sign(t);
}



long operator==(const RR& a, const RR& b) 
{
   return a.e == b.e && a.x == b.x;
}


void trunc(RR& z, const RR& a)
{
   NTL_TLS_LOCAL(RR, t);

   if (a.e >= 0) 
      xcopy(z, a);
   else {
      RightShift(t.x, a.x, -a.e);
      t.e = 0;
      xcopy(z, t);
   }
}

void TruncPrec(RR& x, const RR& a, long p)
{
   if (p < 1)
      LogicError("TruncPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("TruncPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   trunc(x, a);
}

void floor(RR& z, const RR& a)
{
   NTL_TLS_LOCAL(RR, t);

   if (a.e >= 0) 
      xcopy(z, a);
   else {
      RightShift(t.x, a.x, -a.e);
      if (sign(a.x) < 0)
         add(t.x, t.x, -1);
      t.e = 0;
      xcopy(z, t);
   }
}

void FloorPrec(RR& x, const RR& a, long p)
{
   if (p < 1)
      LogicError("FloorPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("FloorPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   floor(x, a);
}

void ceil(RR& z, const RR& a)
{
   NTL_TLS_LOCAL(RR, t);

   if (a.e >= 0)
      xcopy(z, a);
   else {
      RightShift(t.x, a.x, -a.e);
      if (sign(a.x) > 0)
         add(t.x, t.x, 1);
      t.e = 0;
      xcopy(z, t);
   }
}

void CeilPrec(RR& x, const RR& a, long p)
{
   if (p < 1)
      LogicError("CeilPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("CeilPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   ceil(x, a);
}

void round(RR& z, const RR& a)
{
   if (a.e >= 0) {
      xcopy(z, a);
      return;
   }

   long len = NumBits(a.x);

   if (-a.e > len) {
      z = 0;
      return;
   }

   if (-a.e == len) {
      if (len == 1)
         z = 0;
      else
         z = sign(a.x);

      return;
   }

   NTL_TLS_LOCAL(RR, t);
   ConvPrec(t, a, len+a.e);
   xcopy(z, t);
}

void RoundPrec(RR& x, const RR& a, long p)
{
   if (p < 1)
      LogicError("RoundPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("RoundPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   round(x, a);
}


   

void conv(RR& z, const ZZ& a)
{
   normalize1(z, a, 0, RR::prec, 0);
}

void ConvPrec(RR& x, const ZZ& a, long p)
{
   if (p < 1)
      LogicError("ConvPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("ConvPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   conv(x, a);
}


void conv(RR& z, long a)
{
   if (a == 0) {
      clear(z);
      return;
   }

   if (a == 1) {
      set(z);
      return;
   }

   NTL_ZZRegister(t);
   t = a;
   conv(z, t);
}

void ConvPrec(RR& x, long a, long p)
{
   if (p < 1)
      LogicError("ConvPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("ConvPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   conv(x, a);
}

void conv(RR& z, unsigned long a)
{
   if (a == 0) {
      clear(z);
      return;
   }

   if (a == 1) {
      set(z);
      return;
   }

   NTL_ZZRegister(t);
   conv(t, a);
   conv(z, t);
}

void ConvPrec(RR& x, unsigned long a, long p)
{
   if (p < 1)
      LogicError("ConvPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("ConvPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   conv(x, a);
}


void conv(RR& z, double a)
{
   if (a == 0) {
      clear(z);
      return;
   }

   if (a == 1) {
      set(z);
      return;
   }

   if (!IsFinite(&a))
      ArithmeticError("RR: conversion of a non-finite double");

   int e;
   double f;
   NTL_TLS_LOCAL(RR, t);

   f = frexp(a, &e);

   f = f * NTL_FDOUBLE_PRECISION;
   f = f * 4;

   conv(t.x, f);
   t.e = e - (NTL_DOUBLE_PRECISION + 1);

   xcopy(z, t);
}

void ConvPrec(RR& x, double a, long p)
{
   if (p < 1)
      LogicError("ConvPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("ConvPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   conv(x, a);
}


void conv(ZZ& z, const RR& a)
{
   if (a.e >= 0) 
      LeftShift(z, a.x, a.e);
   else {
      long sgn = sign(a.x);
      RightShift(z, a.x, -a.e);
      if (sgn < 0)
         sub(z, z, 1);
   }
}

void CeilToZZ(ZZ& z, const RR& a)
{
   if (a.e >= 0)
      LeftShift(z, a.x, a.e);
   else {
      long sgn = sign(a.x);
      RightShift(z, a.x, -a.e);
      if (sgn > 0)
         add(z, z, 1);
   }
}

void TruncToZZ(ZZ& z, const RR& a)
{
   if (a.e >= 0)
      LeftShift(z, a.x, a.e);
   else 
      RightShift(z, a.x, -a.e);
}


void RoundToZZ(ZZ& z, const RR& a)
{
   if (a.e >= 0) {
      LeftShift(z, a.x, a.e);
      return;
   }

   long len = NumBits(a.x);

   if (-a.e > len) {
      z = 0;
      return;
   }

   if (-a.e == len) {
      if (len == 1)
         z = 0;
      else
         z = sign(a.x);

      return;
   }

   NTL_TLS_LOCAL(RR, t);

   ConvPrec(t, a, len+a.e);

   LeftShift(z, t.x, t.e);
}


void conv(long& z, const RR& a)
{
   ZZ t;
   if (a.e >= NTL_BITS_PER_LONG)
      z = 0;
   else {
      conv(t, a);
      conv(z, t);
   }
}

void conv(double& z, const RR& aa)
{
   double x;
   NTL_TLS_LOCAL(RR, a);

   ConvPrec(a, aa, NTL_DOUBLE_PRECISION);
   // round to NTL_DOUBLE_PRECISION bits to avoid double overflow

   conv(x, a.x);
   z = _ntl_ldexp(x, a.e);
}




void add(RR& z, const RR& a, double b)
{
   NTL_TLS_LOCAL(RR, B);
   B = b;
   add(z, a, B);
}



void sub(RR& z, const RR& a, double b)
{
   NTL_TLS_LOCAL(RR, B);
   B = b;
   sub(z, a, B);
}

void sub(RR& z, double a, const RR& b)
{
   NTL_TLS_LOCAL(RR, A);
   A = a;
   sub(z, A, b);
}



void mul(RR& z, const RR& a, double b)
{
   NTL_TLS_LOCAL(RR, B);
   B = b;
   mul(z, a, B);
}


void div(RR& z, const RR& a, double b)
{
   NTL_TLS_LOCAL(RR, B);
   B = b;
   div(z, a, B);
}

void div(RR& z, double a, const RR& b)
{
   NTL_TLS_LOCAL(RR, A);
   A = a;
   div(z, A, b);
}


void inv(RR& z, const RR& a)
{
   NTL_TLS_LOCAL_INIT(RR, one, (to_RR(1)));
   div(z, one, a);
}

void InvPrec(RR& x, const RR& a, long p)
{
   if (p < 1)
      LogicError("InvPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("InvPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   inv(x, a);
}


long compare(const RR& a, double b)
{
   if (b == 0) return sign(a);

   NTL_TLS_LOCAL(RR, B);
   B = b;
   return compare(a, B);
}


long operator==(const RR& a, double b) 
{
   if (b == 0) return IsZero(a);
   if (b == 1) return IsOne(a);

   NTL_TLS_LOCAL(RR, B);
   B = b;
   return a == B;
}


void power(RR& z, const RR& a, long e)
{
   RR b, res;

   long n = NumBits(e);

   RRPush push;
   long p = RR::precision();
   RR::SetPrecision(p + n + 10);

   xcopy(b, a);

   set(res);
   long i;

   for (i = n-1; i >= 0; i--) {
      sqr(res, res);
      if (bit(e, i))
         mul(res, res, b);
   }

   RR::SetPrecision(p);

   if (e < 0) 
      inv(z, res);
   else
      xcopy(z, res);
}


istream& operator>>(istream& s, RR& x)
{
   RR v;

   {
      RRPush push;

      long c;
      long cval;
      long sign;
      ZZ a, b;

      if (!s) NTL_INPUT_ERROR(s, "bad RR input");


      c = s.peek();
      while (IsWhiteSpace(c)) {
         s.get();
         c = s.peek();
      }

      if (c == '-') {
         sign = -1;
         s.get();
         c = s.peek();
      }
      else
         sign = 1;

      long got1 = 0;
      long got_dot = 0;
      long got2 = 0;

      a = 0;
      b = 1;

      cval = CharToIntVal(c);

      if (cval >= 0 && cval <= 9) {
         got1 = 1;

         while (cval >= 0 && cval <= 9) {
            mul(a, a, 10);
            add(a, a, cval);
            s.get();
            c = s.peek();
            cval = CharToIntVal(c);
         }
      }

      if (c == '.') {
         got_dot = 1;

         s.get();
         c = s.peek();
         cval = CharToIntVal(c);

         if (cval >= 0 && cval <= 9) {
            got2 = 1;
      
            while (cval >= 0 && cval <= 9) {
               mul(a, a, 10);
               add(a, a, cval);
               mul(b, b, 10);
               s.get();
               c = s.peek();
               cval = CharToIntVal(c);
            }
         }
      }

      if (got_dot && !got1 && !got2)  NTL_INPUT_ERROR(s, "bad RR input");

      ZZ e;

      long got_e = 0;
      long e_sign;

      if (c == 'e' || c == 'E') {
         got_e = 1;

         s.get();
         c = s.peek();

         if (c == '-') {
            e_sign = -1;
            s.get();
            c = s.peek();
         }
         else if (c == '+') {
            e_sign = 1;
            s.get();
            c = s.peek();
         }
         else
            e_sign = 1;

         cval = CharToIntVal(c);

         if (cval < 0 || cval > 9) NTL_INPUT_ERROR(s, "bad RR input");

         e = 0;
         while (cval >= 0 && cval <= 9) {
            mul(e, e, 10);
            add(e, e, cval);
            s.get();
            c = s.peek();
            cval = CharToIntVal(c);
         }
      }

      if (!got1 && !got2 && !got_e) NTL_INPUT_ERROR(s, "bad RR input");

      RR t1, t2;


      long old_p = RR::precision();

      if (got1 || got2) {
         ConvPrec(t1, a, max(NumBits(a), 1));
         ConvPrec(t2, b, NumBits(b));
         if (got_e)
            RR::SetPrecision(old_p + 10);

         div(v, t1, t2);
      }
      else
         set(v);

      if (sign < 0)
         negate(v, v);

      if (got_e) {
         if (e >= NTL_OVFBND) ResourceError("RR input overflow");
         long E;
         conv(E, e);
         if (e_sign < 0) E = -E;
         RR::SetPrecision(old_p + 10);
         power(t1, to_RR(10), E);
         mul(v, v, t1);
      }
   }

   xcopy(x, v);
   return s;
}

istream& InputPrec(RR& x, istream& s, long p)
{
   if (p < 1)
      LogicError("InputPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("InputPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   s >> x;
   return s;
}


void conv(RR& z, const xdouble& a)
{
   conv(z, a.mantissa());

   if (a.exponent() >  ((2*NTL_OVFBND)/(2*NTL_XD_HBOUND_LOG))) 
      ResourceError("RR: overlow");

   if (a.exponent() < -((2*NTL_OVFBND)/(2*NTL_XD_HBOUND_LOG))) 
      ResourceError("RR: underflow");

   z.e += a.exponent()*(2*NTL_XD_HBOUND_LOG);

   if (z.e >= NTL_OVFBND)
      ResourceError("RR: overflow");

   if (z.e <= -NTL_OVFBND)
      ResourceError("RR: underflow");
}

void ConvPrec(RR& x, const xdouble& a, long p)
{
   if (p < 1)
      LogicError("ConvPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("ConvPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   conv(x, a);
}



void conv(xdouble& z, const RR& a)
{
   xdouble x;
   xdouble y;

   conv(x, a.x);
   power2(y, a.e);
   z = x*y;
}
      
void power2(RR& z, long e)
{
   if (e >= NTL_OVFBND)
      ResourceError("RR: overflow");

   if (e <= -NTL_OVFBND)
      ResourceError("RR: underflow");

   set(z.x); 
   z.e = e;
}

void conv(RR& z, const quad_float& a)
{
   NTL_TLS_LOCAL(RR, hi);
   NTL_TLS_LOCAL(RR, lo);
   NTL_TLS_LOCAL(RR, res);

   ConvPrec(hi, a.hi, NTL_DOUBLE_PRECISION);
   ConvPrec(lo, a.lo, NTL_DOUBLE_PRECISION);

   add(res, hi, lo);

   z = res;
}

void ConvPrec(RR& x, const quad_float& a, long p)
{
   if (p < 1)
      LogicError("ConvPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("ConvPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   conv(x, a);
}


void conv(quad_float& z, const RR& a)
{
   NTL_TLS_LOCAL(RR, a_hi);
   NTL_TLS_LOCAL(RR, a_lo);

   ConvPrec(a_hi, a, NTL_DOUBLE_PRECISION);  // high order bits
   SubPrec(a_lo, a, a_hi, NTL_DOUBLE_PRECISION);  // low order bits

   z = to_quad_float(a_hi.x)*power2_quad_float(a_hi.e) +
       to_quad_float(a_lo.x)*power2_quad_float(a_lo.e);
}


void ConvPrec(RR& x, const char *s, long p)
{
   if (p < 1)
      LogicError("ConvPrec: bad precsion");
   if (NTL_OVERFLOW(p, 1, 0))
      ResourceError("ConvPrec: bad precsion");

   RRPush push;
   RR::prec = p;
   conv(x, s);
}


void ReallyComputeE(RR& res)
{
   RRPush push;
   long p = RR::precision();
   RR::SetPrecision(p + NumBits(p) + 10);

   RR s, s1, t;

   s = 1;
   t = 1;

   long i;

   for (i = 2; ; i++) {
      add(s1, s, t);
      if (s == s1) break;
      xcopy(s, s1);
      div(t, t, i);
   }

   RR::SetPrecision(p);
   xcopy(res, s);
}

void ComputeE(RR& res)
{
   static NTL_CHEAP_THREAD_LOCAL long prec = 0;

   NTL_TLS_LOCAL(RR, e);

   RRPush push;
   long p = RR::precision();

   if (prec <= p + 10) {
      prec = p + 20;
      RR::SetPrecision(prec);
      ReallyComputeE(e);
      RR::SetPrecision(p);
   }

   xcopy(res, e);
}


void exp(RR& res, const RR& x)
{
   if (x >= NTL_OVFBND || x <= -NTL_OVFBND)
      ResourceError("RR: overflow");

   RRPush push;
   long p = RR::precision();

   // step 0: write x = n + f, n an integer and |f| <= 1/2
   //    careful -- we want to compute f to > p bits of precision


   RR f, nn;
   RR::SetPrecision(NTL_BITS_PER_LONG);
   round(nn, x);
   RR::SetPrecision(p + 10);
   sub(f, x, nn);
   long n = to_long(nn);

   // step 1: calculate t1 = e^n by repeated squaring

   RR::SetPrecision(p + NumBits(n) + 10);

   RR e;
   ComputeE(e);

   RR::SetPrecision(p + 10);

   RR t1;
   power(t1, e, n); 

   // step 2: calculate t2 = e^f using Taylor series expansion

   RR::SetPrecision(p + NumBits(p) + 10);

   RR t2, s, s1, t;
   long i;

   s = 0;
   t = 1;

   for (i = 1; ; i++) {
      add(s1, s, t);
      if (s == s1) break;
      xcopy(s, s1);
      mul(t, t, f);
      div(t, t, i);
   }

   xcopy(t2, s);

   RR::SetPrecision(p);

   mul(res, t1, t2);
}



void ReallyComputeLn2(RR& res)
{
   RRPush push;
   long p = RR::precision();
   RR::SetPrecision(p + NumBits(p) + 10);

   RR s, s1, t, t1;

   s = 0;
   t = 0.5;
   t1 = 0.5;

   long i;

   for (i = 2; ; i++) {
      add(s1, s, t);
      if (s == s1) break;
      xcopy(s, s1);
      mul(t1, t1, 0.5);
      div(t, t1, i);
   }

   RR::SetPrecision(p);
   xcopy(res, s);
}


void ComputeLn2(RR& res)
{
   static NTL_CHEAP_THREAD_LOCAL long prec = 0;

   NTL_TLS_LOCAL(RR, ln2);

   RRPush push;
   long p = RR::precision();

   if (prec <= p + 10) {
      prec = p + 20;
      RR::SetPrecision(prec);
      ReallyComputeLn2(ln2);
      RR::SetPrecision(p);
   }

   xcopy(res, ln2);
}

long Lg2(const RR& x)
{
   return NumBits(x.mantissa()) + x.exponent();
}

void log(RR& res, const RR& x)
{
   if (x <= 0) ArithmeticError("argument to log must be positive");

   RRPush push;
   long p = RR::precision();

   RR::SetPrecision(p + NumBits(p) + 10);

   RR y;
   long n;

   // re-write x = 2^n * (1 - y), where -1/2 < y < 1/4  (so 3/4 < 1-y < 3/2)

   if (x > 0.75 && x < 1.5) {
      n = 0;
      sub(y, 1, x);
   }
   else {
      n = Lg2(x) - 1;
      RR t;
      power2(t, -n);
      mul(t, t, x);
      while (t > 1.5) {
         mul(t, t, 0.5);
         n++;
      }

      sub(y, 1, t);
   }

   // compute s = - ln(1-y) by power series expansion

   RR s, s1, t, t1;

   s = 0;
   xcopy(t, y);
   xcopy(t1, y);

   long i;

   for (i = 2; ; i++) {
      add(s1, s, t);
      if (s == s1) break;
      xcopy(s, s1);
      mul(t1, t1, y);
      div(t, t1, i);
   }

   if (n == 0) 
      t = 0;
   else {
      ComputeLn2(t);
      mul(t, t, n);
   }

   RR::SetPrecision(p);

   sub(res, t, s);
}


void ComputeLn10(RR& res)
{
   static NTL_CHEAP_THREAD_LOCAL long prec = 0;

   NTL_TLS_LOCAL(RR, ln10);

   RRPush push;
   long p = RR::precision();

   if (prec <= p + 10) {
      prec = p + 20;
      RR::SetPrecision(prec);
      log(ln10, to_RR(10));
      RR::SetPrecision(p);
   }

   xcopy(res, ln10);
}

void log10(RR& res, const RR& x)
{
   RRPush push;
   long p = RR::precision();
   RR::SetPrecision(p + 10);

   RR ln10, t1, t2;
   ComputeLn10(ln10);

   log(t1, x);
   div(t2, t1, ln10);

   RR::SetPrecision(p);

   xcopy(res, t2);
}


void expm1(RR& res, const RR& x)
{
   RRPush push;
   long p = RR::precision();

   if (x < -0.5 || x > 0.5) {
      RR t;
      RR::SetPrecision(p + 10);
      exp(t, x);
      RR::SetPrecision(p);
      sub(res, t, 1);
      return;
   }


   RR::SetPrecision(p + NumBits(p) + 10);

   RR f;

   xcopy(f, x);

   RR s, s1, t;
   long i;

   s = 0;
   xcopy(t, f);

   for (i = 2; ; i++) {
      add(s1, s, t);
      if (s == s1) break;
      xcopy(s, s1);
      mul(t, t, f);
      div(t, t, i);
   }

   RR::SetPrecision(p);

   xcopy(res, s);
}



void log1p(RR& res, const RR& x)
{
   RRPush push;
   long p = RR::precision();
   RR y;

   if (x < -0.5 || x > 0.5) {
      RR::SetPrecision(p + 10);
      log(y, 1 + x);
      RR::SetPrecision(p);
      xcopy(res, y);
      return;
   }


   RR::SetPrecision(p + NumBits(p) + 10);


   negate(y, x);

   // compute s = - ln(1-y) by power series expansion

   RR s, s1, t, t1;

   s = 0;
   xcopy(t, y);
   xcopy(t1, y);

   long i;

   for (i = 2; ; i++) {
      add(s1, s, t);
      if (s == s1) break;
      xcopy(s, s1);
      mul(t1, t1, y);
      div(t, t1, i);
   }

   RR::SetPrecision(p);

   negate(res, s);

}


void pow(RR& res, const RR& x, const RR& y)
{

   if (y == 0) {
      res = 1;
      return;
   }

   if (x == 0) {
      res = 0;
      return;
   }

   if (x == 1) {
      res = 1;
      return;
   }

   if (x < 0) {
      ArithmeticError("pow: sorry...first argument to pow must be nonnegative");
   }

   RRPush push;
   long p = RR::precision();

   // calculate working precison...one could use p + NTL_BITS_PER_LONG + 10,
   // for example, but we want the behaviour to be machine independent.
   // so we calculate instead a rough approximation to log |y log(x)|

   RR t1, t2;
   long k;

   if (x > 0.5 && x < 1.5) { 
      xcopy(t1, x - 1);
      k = Lg2(t1);
   }
   else {
      k = NumBits(Lg2(x)); 
   }

   k += Lg2(y);

   if (k > NTL_BITS_PER_LONG+10) ResourceError("RR: overflow");

   if (k < 0) k = 0;

   
   RR::SetPrecision(p + k + 10);

   t1 = y*log(x);

   RR::SetPrecision(p);

   t2 = exp(t1);

   res = t2;
}


void ReallyComputePi(RR& res)
{
   RRPush push;
   long p = RR::precision();
   RR::SetPrecision(p + NumBits(p) + 10);


   RR sum1;

   RR s, s1, t, t1;

   s = 0;
   t = 0.5;
   t1 = 0.5;

   long i;

   for (i = 3; ; i+=2) {
      add(s1, s, t);
      if (s == s1) break;
      xcopy(s, s1);
      mul(t1, t1, -0.25);
      div(t, t1, i);
   }

   xcopy(sum1, s);


   RR g;

   inv(g, to_RR(3)); // g = 1/3

   s = 0;
   xcopy(t, g);
   xcopy(t1, g);

   sqr(g, g);
   negate(g, g); // g = -1/9

   for (i = 3; ; i+=2) {
      add(s1, s, t);
      if (s == s1) break;
      xcopy(s, s1);
      mul(t1, t1, g);
      div(t, t1, i);
   }

   add(s, s, sum1);
   mul(s, s, 4);

   RR::SetPrecision(p);
   xcopy(res, s);
}

void ComputePi(RR& res)
{
   static NTL_CHEAP_THREAD_LOCAL long prec = 0;

   NTL_TLS_LOCAL(RR, pi);

   RRPush push;
   long p = RR::precision();

   if (prec <= p + 10) {
      prec = p + 20;
      RR::SetPrecision(prec);
      ReallyComputePi(pi);
      RR::SetPrecision(p);
   }

   xcopy(res, pi);
}



void sin(RR& res, const RR& x)
{
   if (x == 0) {
      res = 0;
      return;
   }

   if (Lg2(x) > 1000) 
      ResourceError("sin: sorry...argument too large in absolute value");

   RRPush push;
   long p = RR::precision();

   RR pi, t1, f;
   RR n;


   // we want to make x^2 < 3, so that the series for sin(x)
   // converges nicely, without any nasty cancellations in the
   // first terms of the series.

   RR::SetPrecision(p + NumBits(p) + 10);

   if (x*x < 3) {
      xcopy(f, x);
   }
   else {

      // we want to write x/pi = n + f, |f| < 1/2....
      // but we have to do *this* very carefully, so that f is computed
      // to precision > p.  I know, this is sick!
   
      long p1;
   
      p1 = p + Lg2(x) + 20;
   
   
      for (;;) {
         RR::SetPrecision(p1);
         ComputePi(pi);
         xcopy(t1, x/pi);
         xcopy(n, floor(t1));
         xcopy(f, t1 - n);
         if (f > 0.5) {
            n++;
            xcopy(f, t1 - n);
         }

         if (f == 0 || p1 < p - Lg2(f) + Lg2(n) + 10) {
            // we don't have enough bits of f...increase p1 and continue

            p1 = p1 + max(20, p1/10);
         }
         else
            break;
      }

      RR::SetPrecision(p + NumBits(p) + 10);
      ComputePi(pi);

      xcopy(f, pi * f);
      
      if (n != 0 && n.exponent() == 0) {
         // n is odd, so we negate f, which negates sin(f)

         xcopy(f, -f);
      }

   }

   // Boy, that was painful, but now its over, and we can simply apply
   // the series for sin(f)

   RR t2, s, s1, t;
   long i;

   s = 0;
   xcopy(t, f);

   for (i = 3; ; i=i+2) {
      add(s1, s, t);
      if (s == s1) break;
      xcopy(s, s1);
      mul(t, t, f);
      mul(t, t, f);
      div(t, t, i-1);
      div(t, t, i);
      negate(t, t);
   }

   RR::SetPrecision(p);
   
   xcopy(res, s);

}

void cos(RR& res, const RR& x)
{
   if (x == 0) {
      res = 1;
      return;
   }

   if (Lg2(x) > 1000) 
      ResourceError("cos: sorry...argument too large in absolute value");

   RRPush push;
   long p = RR::precision();

   RR pi, t1, f;
   RR n;

   // we want to write x/pi = (n+1/2) + f, |f| < 1/2....
   // but we have to do *this* very carefully, so that f is computed
   // to precision > p.  I know, this is sick!

   long p1;

   p1 = p + Lg2(x) + 20;


   for (;;) {
      RR::SetPrecision(p1);
      ComputePi(pi);
      xcopy(t1, x/pi);
      xcopy(n, floor(t1));
      xcopy(f, t1 - (n + 0.5));

      if (f == 0 || p1 < p - Lg2(f) + Lg2(n) + 10) {
         // we don't have enough bits of f...increase p1 and continue

         p1 = p1 + max(20, p1/10);
      }
      else
         break;
   }

   RR::SetPrecision(p + NumBits(p) + 10);
   ComputePi(pi);

   xcopy(f, pi * f);
   
   if (n == 0 || n.exponent() != 0) {
      // n is even, so we negate f, which negates sin(f)

      xcopy(f, -f);
   }

   // Boy, that was painful, but now its over, and we can simply apply
   // the series for sin(f)

   RR t2, s, s1, t;
   long i;

   s = 0;
   xcopy(t, f);

   for (i = 3; ; i=i+2) {
      add(s1, s, t);
      if (s == s1) break;
      xcopy(s, s1);
      mul(t, t, f);
      mul(t, t, f);
      div(t, t, i-1);
      div(t, t, i);
      negate(t, t);
   }

   RR::SetPrecision(p);
   
   xcopy(res, s);

}


ostream& operator<<(ostream& s, const RR& a)
{
   if (IsZero(a)) {
      s << "0";
      return s;
   }

   RRPush push;

   // we compute new_p and log_10_a precisely using sufficient
   // precision---this is necessary to achieve accuracy and
   // platform independent behaviour

   long temp_p = max(NumBits(RR::OutputPrecision()), 
                     NumBits(Lg2(a))) + 10; 

   RR::SetPrecision(temp_p);

   RR ln2, ln10, log_2_10;
   ComputeLn2(ln2);
   ComputeLn10(ln10);
   log_2_10 = ln10/ln2;
   long new_p = to_long(RR::OutputPrecision()*log_2_10) + 20;
   long log_10_a = to_long(Lg2(a)/log_2_10);

   RR::SetPrecision(new_p);

   RR b;
   long neg;

   if (a < 0) {
      negate(b, a);
      neg = 1;
   }
   else {
      xcopy(b, a);
      neg = 0;
   }

   long k = RR::OutputPrecision() - log_10_a;

   RR c, d;

   power(c, to_RR(10), RR::OutputPrecision());
   power(d, to_RR(10), log_10_a);

   div(b, b, d);
   mul(b, b, c);

   while (b < c) {
      mul(b, b, 10);
      k++;
   }

   while (b >= c) {
      div(b, b, 10);
      k--;
   }

   add(b, b, 0.5);
   k = -k;

   ZZ B;
   conv(B, b);

   long bp_len = RR::OutputPrecision()+10;

   UniqueArray<char> bp_store;
   bp_store.SetLength(bp_len);
   char *bp = bp_store.get();

   long len, i;

   len = 0;
   do {
      if (len >= bp_len) LogicError("RR output: buffer overflow");
      bp[len] = IntValToChar(DivRem(B, B, 10));
      len++;
   } while (B > 0);

   for (i = 0; i < len/2; i++) {
      char tmp;
      tmp = bp[i];
      bp[i] = bp[len-1-i];
      bp[len-1-i] = tmp;
   }

   i = len-1;
   while (bp[i] == '0') i--;

   k += (len-1-i);
   len = i+1;

   bp[len] = '\0';

   if (k > 3 || k < -len - 3) {
      // use scientific notation

      if (neg) s << "-";
      s << "0." << bp << "e" << (k + len);
   }
   else if (k >= 0) {
      if (neg) s << "-";
      s << bp;
      for (i = 0; i < k; i++) 
         s << "0";
   }
   else if (k <= -len) {
      if (neg) s << "-";
      s << "0.";
      for (i = 0; i < -len-k; i++)
         s << "0";
      s << bp;
   }
   else {
      if (neg) s << "-";
      for (i = 0; i < len+k; i++)
         s << bp[i];

      s << ".";

      for (i = len+k; i < len; i++)
         s << bp[i];
   }

   return s;
}


NTL_END_IMPL
