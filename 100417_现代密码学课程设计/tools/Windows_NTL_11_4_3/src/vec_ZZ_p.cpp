

#include <NTL/vec_ZZ_p.h>
#include <NTL/BasicThreadPool.h>


NTL_START_IMPL

#define PAR_THRESH (4000.0)

static inline bool BelowThresh(long n)
{
   return double(n)*double(ZZ_p::ModulusSize()) < PAR_THRESH;
}

static 
void BasicBlockConstruct(ZZ_p* x, long n, long d)
{
   long m, j;

   long i = 0;

   NTL_SCOPE(guard) { BlockDestroy(x, i); };

   while (i < n) {
      m = ZZ_BlockConstructAlloc(x[i]._ZZ_p__rep, d, n-i);
      for (j = 1; j < m; j++)
         ZZ_BlockConstructSet(x[i]._ZZ_p__rep, x[i+j]._ZZ_p__rep, j);
      i += m;
   }

   guard.relax();
}

void BlockConstruct(ZZ_p* x, long n)
{
   if (n <= 0) return; 

   if (!ZZ_pInfo)
      LogicError("ZZ_p constructor called while modulus undefined");

   long d = ZZ_p::ModulusSize();

   BasicBlockConstruct(x, n, d);
}

void BlockConstructFromVec(ZZ_p* x, long n, const ZZ_p* y)
{
   if (n <= 0) return;

   long d = y->_ZZ_p__rep.MaxAlloc() - 1;
   BasicBlockConstruct(x, n, d);

   NTL_SCOPE(guard) { BlockDestroy(x, n); };

   long i;
   for (i = 0; i < n; i++) x[i] = y[i];

   guard.relax();
}

void BlockConstructFromObj(ZZ_p* x, long n, const ZZ_p& y)
{
   if (n <= 0) return;


   if (!ZZ_pInfo)
      LogicError("ZZ_p constructor called while modulus undefined");

   long d = ZZ_p::ModulusSize();

   BasicBlockConstruct(x, n, d);

   NTL_SCOPE(guard) { BlockDestroy(x, n); };

   long i;
   for (i = 0; i < n; i++) x[i] = y;

   guard.relax();
}


void BlockDestroy(ZZ_p* x, long n)
{
   if (n <= 0) return;

   long i = 0;
   long m;

   while (i < n) {
      m = ZZ_BlockDestroy(x[i]._ZZ_p__rep);
      i += m;
   }
}



void InnerProduct(ZZ_p& x, const vec_ZZ_p& a, const vec_ZZ_p& b)
{
   long n = min(a.length(), b.length());
   long i;
   NTL_ZZRegister(accum);
   NTL_ZZRegister(t);

   clear(accum);
   for (i = 0; i < n; i++) {
      mul(t, rep(a[i]), rep(b[i]));
      add(accum, accum, t);
   }

   conv(x, accum);
}

void InnerProduct(ZZ_p& x, const vec_ZZ_p& a, const vec_ZZ_p& b,
                  long offset)
{
   if (offset < 0) LogicError("InnerProduct: negative offset");
   if (NTL_OVERFLOW(offset, 1, 0)) 
      ResourceError("InnerProduct: offset too big");

   long n = min(a.length(), b.length()+offset);
   long i;
   NTL_ZZRegister(accum);
   NTL_ZZRegister(t);

   clear(accum);
   for (i = offset; i < n; i++) {
      mul(t, rep(a[i]), rep(b[i-offset]));
      add(accum, accum, t);
   }

   conv(x, accum);
}

void mul(vec_ZZ_p& x, const vec_ZZ_p& a, const ZZ_p& b_in)
{
   NTL_ZZ_pRegister(b);
   b = b_in;
   long n = a.length();
   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      mul(x[i], a[i], b);
}

void mul(vec_ZZ_p& x, const vec_ZZ_p& a, long b_in)
{
   NTL_ZZ_pRegister(b);
   b = b_in;
   long n = a.length();
   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      mul(x[i], a[i], b);
}


void add(vec_ZZ_p& x, const vec_ZZ_p& a, const vec_ZZ_p& b)
{
   long n = a.length();
   if (b.length() != n) LogicError("vector add: dimension mismatch");

   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      add(x[i], a[i], b[i]);
}

void sub(vec_ZZ_p& x, const vec_ZZ_p& a, const vec_ZZ_p& b)
{
   long n = a.length();
   if (b.length() != n) LogicError("vector sub: dimension mismatch");
   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      sub(x[i], a[i], b[i]);
}

void clear(vec_ZZ_p& x)
{
   long n = x.length();
   long i;
   for (i = 0; i < n; i++)
      clear(x[i]);
}

void negate(vec_ZZ_p& x, const vec_ZZ_p& a)
{
   long n = a.length();
   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      negate(x[i], a[i]);
}

long IsZero(const vec_ZZ_p& a)
{
   long n = a.length();
   long i;

   for (i = 0; i < n; i++)
      if (!IsZero(a[i]))
         return 0;

   return 1;
}

vec_ZZ_p operator+(const vec_ZZ_p& a, const vec_ZZ_p& b)
{
   vec_ZZ_p res;
   add(res, a, b);
   NTL_OPT_RETURN(vec_ZZ_p, res);
}

vec_ZZ_p operator-(const vec_ZZ_p& a, const vec_ZZ_p& b)
{
   vec_ZZ_p res;
   sub(res, a, b);
   NTL_OPT_RETURN(vec_ZZ_p, res);
}


vec_ZZ_p operator-(const vec_ZZ_p& a)
{
   vec_ZZ_p res;
   negate(res, a);
   NTL_OPT_RETURN(vec_ZZ_p, res);
}


ZZ_p operator*(const vec_ZZ_p& a, const vec_ZZ_p& b)
{
   ZZ_p res;
   InnerProduct(res, a, b);
   NTL_OPT_RETURN(ZZ_p, res);
}


void VectorCopy(vec_ZZ_p& x, const vec_ZZ_p& a, long n)
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

void random(vec_ZZ_p& x, long n)
{
   x.SetLength(n);
   for (long i = 0; i < n; i++) random(x[i]);
}

// thread-boosted conversion.
// This is used in the implementation of ZZ_pX multiplication in terms
// of ZZX SSMul.
void conv(vec_ZZ_p& x, const vec_ZZ& a)
{
   long n = a.length();
   x.SetLength(n);

   if (n == 0) return;

   const ZZ *ap = a.elts();
   ZZ_p *xp = x.elts();

   ZZ_pContext context;
   context.save();

   bool seq = BelowThresh(n);

   NTL_GEXEC_RANGE(seq, n, first, last)
   NTL_IMPORT(ap)
   NTL_IMPORT(xp)

      context.restore();

      for (long i = first; i < last; i++)
         conv(xp[i], ap[i]);

   NTL_GEXEC_RANGE_END

}

NTL_END_IMPL
