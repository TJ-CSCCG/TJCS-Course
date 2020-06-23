
#include <NTL/FacVec.h>
#include <NTL/ZZ.h>


NTL_START_IMPL


static
void swap(IntFactor& x, IntFactor& y)
{
   IntFactor t;

   t = x;  x = y;  y = t;
}

static
void FindMin(FacVec& v, long lo, long hi)
{
   long minv = 0;
   long minp = -1;
   long i;

   for (i = lo; i <= hi; i++) {
      if (minv == 0 || v[i].val < minv) {
         minv = v[i].val;
         minp = i;
      }
   }

   swap(v[lo], v[minp]);
}


void FactorInt(FacVec& fvec, long n)
{
   if (n <= 1) LogicError("internal error: FactorInt(FacVec,long n) with n<=1");

   if (NTL_OVERFLOW(n, 1, 0))
      ResourceError("internal error: FactorInt(FacVec,long n) with n too large");

   long NumFactors;
   long q;

   fvec.SetLength(2*NextPowerOfTwo(n));

   NumFactors = 0;
   q = 2;

   while (n != 1) {
      if (n%q == 0) {
         fvec[NumFactors].q = q;
         n = n/q;
         fvec[NumFactors].a = 1;
         fvec[NumFactors].val = q;
         while (n%q == 0) {
            n = n/q;
            (fvec[NumFactors].a)++;
            fvec[NumFactors].val *= q;
         }         
         fvec[NumFactors].link = -1;
         NumFactors++;
      }

      q++;
   }

   fvec.SetLength(2*NumFactors-1);

   long lo = 0;
   long hi = NumFactors - 1;

   while (lo < hi) {
      FindMin(fvec, lo, hi);
      FindMin(fvec, lo+1, hi);
      hi++;
      fvec[hi].link = lo;
      fvec[hi].val = fvec[lo].val * fvec[lo+1].val;
      lo += 2;
   }
}



NTL_END_IMPL
