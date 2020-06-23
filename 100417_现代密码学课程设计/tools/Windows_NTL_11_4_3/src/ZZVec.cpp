
#include <NTL/ZZVec.h>


NTL_START_IMPL

void ZZVec::SetSize(long n, long d)
{
   if (n < 0 || d <= 0) LogicError("bad args to ZZVec::SetSize()");

   if (v)
      LogicError("illegal ZZVec initialization");

   if (n == 0) {
      len = n;
      bsize = d;
      return;
   }

   ZZVec tmp;
   tmp.len = 0;
   tmp.bsize = d;

   tmp.v = (ZZ*) NTL_SNS_MALLOC(n, sizeof(ZZ), 0);
   if (!tmp.v) MemoryError();

   long i = 0;
   long m;
   long j;

   while (i < n) {
      m = ZZ_BlockConstructAlloc(tmp.v[i], d, n-i);
      for (j = 1; j < m; j++)
         ZZ_BlockConstructSet(tmp.v[i], tmp.v[i+j], j);
      i += m;
      tmp.len = i;
   }

   tmp.swap(*this);
}

void ZZVec::kill()
{
   long n = len;
   long i = 0;
   while (i < n) {
      long m = ZZ_BlockDestroy(v[i]);
      i += m;
   }

   len = 0; 
   bsize = 0;
   if (v) {
      free(v);
      v = 0; 
   }
}


ZZVec& ZZVec::operator=(const ZZVec& a) 
{
   if (this == &a) return *this;
   ZZVec tmp(a);
   tmp.swap(*this);
   return *this;
}
   
ZZVec::ZZVec(const ZZVec& a) : v(0), len(0), bsize(0)
{
   SetSize(a.len, a.bsize);

   long i;
   for (i = 0; i < a.len; i++)
      v[i] = (a.v)[i];
}


NTL_END_IMPL
