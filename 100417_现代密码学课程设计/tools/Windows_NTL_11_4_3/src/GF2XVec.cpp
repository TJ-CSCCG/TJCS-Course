
#include <NTL/GF2XVec.h>


NTL_START_IMPL


void GF2XVec::SetSize(long n, long d)
{
   if (n < 0 || d <= 0) LogicError("bad args to GF2XVec::SetSize()");

   if (v)
      LogicError("illegal GF2XVec initialization");


   if (n == 0) {
      len = n;
      bsize = d;
      return;
   }


   GF2XVec tmp;
   tmp.len = 0;
   tmp.bsize = d;


   tmp.v = (GF2X*) NTL_SNS_MALLOC(n, sizeof(GF2X), 0);
   if (!tmp.v) MemoryError();

   long i = 0;
   long m;
   long j;

   while (i < n) {
      m = WV_BlockConstructAlloc(tmp.v[i].xrep, d, n-i);
      for (j = 1; j < m; j++)
         WV_BlockConstructSet(tmp.v[i].xrep, tmp.v[i+j].xrep, j);
      i += m;
      tmp.len = i;
   }

   tmp.swap(*this);
}


void GF2XVec::kill()
{
   long n = len;
   long i = 0;
   while (i < n) {
      long m = WV_BlockDestroy(v[i].xrep);
      i += m;
   }

   len = 0;
   bsize = 0;
   if (v) {
      free(v);
      v = 0; 
   }
}


GF2XVec& GF2XVec::operator=(const GF2XVec& a) 
{
   if (this == &a) return *this;
   GF2XVec tmp(a);
   tmp.swap(*this);
   return *this;
}
   
GF2XVec::GF2XVec(const GF2XVec& a) : v(0), len(0), bsize(0)

{
   SetSize(a.len, a.bsize);

   long i;
   for (i = 0; i < a.len; i++)
      v[i] = (a.v)[i];
}




NTL_END_IMPL
