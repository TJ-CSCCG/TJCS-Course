
#include <NTL/vec_ZZ_pE.h>


NTL_START_IMPL

void InnerProduct(ZZ_pE& x, const vec_ZZ_pE& a, const vec_ZZ_pE& b)
{
   long n = min(a.length(), b.length());
   long i;
   ZZ_pX accum, t;

   clear(accum);
   for (i = 0; i < n; i++) {
      mul(t, rep(a[i]), rep(b[i]));
      add(accum, accum, t);
   }

   conv(x, accum);
}

void InnerProduct(ZZ_pE& x, const vec_ZZ_pE& a, const vec_ZZ_pE& b,
                  long offset)
{
   if (offset < 0) LogicError("InnerProduct: negative offset");
   if (NTL_OVERFLOW(offset, 1, 0)) ResourceError("InnerProduct: offset too big");

   long n = min(a.length(), b.length()+offset);
   long i;
   ZZ_pX accum, t;

   clear(accum);
   for (i = offset; i < n; i++) {
      mul(t, rep(a[i]), rep(b[i-offset]));
      add(accum, accum, t);
   }

   conv(x, accum);
}

void mul(vec_ZZ_pE& x, const vec_ZZ_pE& a, const ZZ_pE& b_in)
{
   ZZ_pE b = b_in;
   long n = a.length();
   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      mul(x[i], a[i], b);
}

void mul(vec_ZZ_pE& x, const vec_ZZ_pE& a, const ZZ_p& b_in)
{
   NTL_ZZ_pRegister(b);
   b = b_in;
   long n = a.length();
   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      mul(x[i], a[i], b);
}

void mul(vec_ZZ_pE& x, const vec_ZZ_pE& a, long b_in)
{
   NTL_ZZ_pRegister(b);
   b = b_in;
   long n = a.length();
   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      mul(x[i], a[i], b);
}


void add(vec_ZZ_pE& x, const vec_ZZ_pE& a, const vec_ZZ_pE& b)
{
   long n = a.length();
   if (b.length() != n) LogicError("vector add: dimension mismatch");

   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      add(x[i], a[i], b[i]);
}

void sub(vec_ZZ_pE& x, const vec_ZZ_pE& a, const vec_ZZ_pE& b)
{
   long n = a.length();
   if (b.length() != n) LogicError("vector sub: dimension mismatch");

   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      sub(x[i], a[i], b[i]);
}

void negate(vec_ZZ_pE& x, const vec_ZZ_pE& a)
{
   long n = a.length();

   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      negate(x[i], a[i]);
}


void clear(vec_ZZ_pE& x)
{
   long n = x.length();
   long i;
   for (i = 0; i < n; i++)
      clear(x[i]);
}



long IsZero(const vec_ZZ_pE& a)
{
   long n = a.length();
   long i;

   for (i = 0; i < n; i++)
      if (!IsZero(a[i]))
         return 0;

   return 1;
}

vec_ZZ_pE operator+(const vec_ZZ_pE& a, const vec_ZZ_pE& b)
{
   vec_ZZ_pE res;
   add(res, a, b);
   NTL_OPT_RETURN(vec_ZZ_pE, res);
}

vec_ZZ_pE operator-(const vec_ZZ_pE& a, const vec_ZZ_pE& b)
{
   vec_ZZ_pE res;
   sub(res, a, b);
   NTL_OPT_RETURN(vec_ZZ_pE, res);
}


vec_ZZ_pE operator-(const vec_ZZ_pE& a)
{
   vec_ZZ_pE res;
   negate(res, a);
   NTL_OPT_RETURN(vec_ZZ_pE, res);
}


ZZ_pE operator*(const vec_ZZ_pE& a, const vec_ZZ_pE& b)
{
   ZZ_pE res;
   InnerProduct(res, a, b);
   return res;
}

void VectorCopy(vec_ZZ_pE& x, const vec_ZZ_pE& a, long n)
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

void random(vec_ZZ_pE& x, long n)
{
   x.SetLength(n);
   for (long i = 0; i < n; i++) random(x[i]);
}


NTL_END_IMPL
