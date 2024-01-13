
#include <NTL/vec_ZZ.h>

NTL_START_IMPL


void InnerProduct(ZZ& xx, const vec_ZZ& a, const vec_ZZ& b)
{
   ZZ t1, x;

   long n = min(a.length(), b.length());
   long i;

   clear(x);
   for (i = 1; i <= n; i++) {
      mul(t1, a(i), b(i));
      add(x, x, t1);
   }

   xx = x;
}

void mul(vec_ZZ& x, const vec_ZZ& a, const ZZ& b_in)
{
   ZZ b = b_in;
   long n = a.length();
   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      mul(x[i], a[i], b);
}

void mul(vec_ZZ& x, const vec_ZZ& a, long b)
{
   long n = a.length();
   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      mul(x[i], a[i], b);
}

void add(vec_ZZ& x, const vec_ZZ& a, const vec_ZZ& b)
{
   long n = a.length();
   if (b.length() != n) LogicError("vector add: dimension mismatch");

   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      add(x[i], a[i], b[i]);
}

void sub(vec_ZZ& x, const vec_ZZ& a, const vec_ZZ& b)
{
   long n = a.length();
   if (b.length() != n) LogicError("vector sub: dimension mismatch");
   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      sub(x[i], a[i], b[i]);
}

void clear(vec_ZZ& x)
{
   long n = x.length();
   long i;
   for (i = 0; i < n; i++)
      clear(x[i]);
}

void negate(vec_ZZ& x, const vec_ZZ& a)
{
   long n = a.length();
   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      negate(x[i], a[i]);
}




long IsZero(const vec_ZZ& a)
{
   long n = a.length();
   long i;

   for (i = 0; i < n; i++)
      if (!IsZero(a[i]))
         return 0;

   return 1;
}

vec_ZZ operator+(const vec_ZZ& a, const vec_ZZ& b)
{
   vec_ZZ res;
   add(res, a, b);
   NTL_OPT_RETURN(vec_ZZ, res);
}

vec_ZZ operator-(const vec_ZZ& a, const vec_ZZ& b)
{
   vec_ZZ res;
   sub(res, a, b);
   NTL_OPT_RETURN(vec_ZZ, res);
}


vec_ZZ operator-(const vec_ZZ& a)
{
   vec_ZZ res;
   negate(res, a);
   NTL_OPT_RETURN(vec_ZZ, res);
}


ZZ operator*(const vec_ZZ& a, const vec_ZZ& b)
{
   ZZ res;
   InnerProduct(res, a, b);
   NTL_OPT_RETURN(ZZ, res);
}

void VectorCopy(vec_ZZ& x, const vec_ZZ& a, long n)
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
