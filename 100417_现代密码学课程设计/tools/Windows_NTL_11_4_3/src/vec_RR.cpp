
#include <NTL/vec_RR.h>


NTL_START_IMPL


void InnerProduct(RR& xx, const vec_RR& a, const vec_RR& b)
{
   RR t1, x;

   long n = min(a.length(), b.length());
   long i;

   clear(x);
   for (i = 1; i <= n; i++) {
      mul(t1, a(i), b(i));
      add(x, x, t1);
   }

   xx = x;
}

void mul(vec_RR& x, const vec_RR& a, const RR& b_in)
{
   RR b = b_in;
   long n = a.length();
   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      mul(x[i], a[i], b);
}

void mul(vec_RR& x, const vec_RR& a, double b_in)
{
   NTL_TLS_LOCAL(RR, b);
   conv(b, b_in);
   long n = a.length();
   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      mul(x[i], a[i], b);
}

void add(vec_RR& x, const vec_RR& a, const vec_RR& b)
{
   long n = a.length();
   if (b.length() != n) LogicError("vector add: dimension mismatch");

   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      add(x[i], a[i], b[i]);
}

void sub(vec_RR& x, const vec_RR& a, const vec_RR& b)
{
   long n = a.length();
   if (b.length() != n) LogicError("vector sub: dimension mismatch");
   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      sub(x[i], a[i], b[i]);
}

void clear(vec_RR& x)
{
   long n = x.length();
   long i;
   for (i = 0; i < n; i++)
      clear(x[i]);
}

void negate(vec_RR& x, const vec_RR& a)
{
   long n = a.length();
   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      negate(x[i], a[i]);
}


long IsZero(const vec_RR& a)
{
   long n = a.length();
   long i;

   for (i = 0; i < n; i++)
      if (!IsZero(a[i]))
         return 0;

   return 1;
}

vec_RR operator+(const vec_RR& a, const vec_RR& b)
{
   vec_RR res;
   add(res, a, b);
   NTL_OPT_RETURN(vec_RR, res);
}

vec_RR operator-(const vec_RR& a, const vec_RR& b)
{
   vec_RR res;
   sub(res, a, b);
   NTL_OPT_RETURN(vec_RR, res);
}


vec_RR operator-(const vec_RR& a)
{
   vec_RR res;
   negate(res, a);
   NTL_OPT_RETURN(vec_RR, res);
}

RR operator*(const vec_RR& a, const vec_RR& b)
{
   RR res;
   InnerProduct(res, a, b);
   return res;
}

void VectorCopy(vec_RR& x, const vec_RR& a, long n)
{
   if (n < 0) LogicError("VectorCopy: negative length");
   if (NTL_OVERFLOW(n, 1, 0)) ResourceError("overflow in VectorCopy");

   long m = min(n, a.length());

   x.SetLength(n);

   long i;

   for (i = 0; i < m; i++)
      x[i] = a[i];

   for (i = m; i < n; i++)
      clear(x[i]);
}

NTL_END_IMPL
