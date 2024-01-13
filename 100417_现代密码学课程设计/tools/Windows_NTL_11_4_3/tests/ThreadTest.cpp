#include <NTL/config.h>

#ifdef NTL_THREADS


#include <NTL/ZZX.h>
#include <NTL/ZZ_pXFactoring.h>
#include <NTL/BasicThreadPool.h>
#include <cstdio>

NTL_CLIENT

#if 1


long mobius(long n)
{
  long p,e,arity=0;
  PrimeSeq s;
  while (n!=1)
    { p=s.next();
      e=0;
      while ((n%p==0)) { n=n/p; e++; }
      if (e>1) { return 0; }
      if (e!=0) { arity^=1; }
    }     
  if (arity==0) { return 1; }
  return -1;
}


ZZX Cyclotomic(long N)
{
  ZZX Num,Den,G,F;
  set(Num); set(Den);
  long m,d;
  for (d=1; d<=N; d++)
    { if ((N%d)==0)
         { clear(G);
           SetCoeff(G,N/d,1); SetCoeff(G,0,-1);
           m=mobius(d);
           if (m==1)       { Num*=G; }
           else if (m==-1) { Den*=G; }
         }
    } 
  F=Num/Den;
  return F;
}

long multOrd(const ZZ& p, long m)
{
  long pp = rem(p, m);
  if (GCD(pp, m) != 1) return 0;

  long ord = 1;
  long val = pp; 
  while (val != 1) {
    ord++;
    val = MulMod(val, pp, m);
  }
  return ord;
}

#endif





int main()
{
   SetSeed(ZZ(0));

   long NumContexts = 3;
   long NumPolys = 6;
   long n = 2000;

   Vec<ZZ_pContext> context_vec;
   context_vec.SetLength(NumContexts);

   for (long i = 0; i < NumContexts; i++) { 
      ZZ p;
      GenPrime(p, 150 + i*20);
      context_vec[i] = ZZ_pContext(p);
   }

   Vec<ZZ_pX> poly_vec;
   Vec<vec_pair_ZZ_pX_long> res_vec;

   poly_vec.SetLength(NumPolys);
   res_vec.SetLength(NumPolys);


   for (long i = 0; i < NumPolys; i++) {
      context_vec[i % NumContexts].restore();
      ZZX f = Cyclotomic(n+i);
      conv(poly_vec[i], f);
   }


   cerr << "START\n";

   BasicThreadPool pool(NumPolys);

   pool.exec_index(NumPolys,
      [&](long i) {
         fprintf(stderr, "starting %ld: %s\n", i, CurrentThreadID().c_str());
         context_vec[i % NumContexts].restore();
         CanZass(res_vec[i], poly_vec[i]);
         fprintf(stderr, "stopping %ld: %s\n", i, CurrentThreadID().c_str());
      });

   cerr << "checking results...\n";


   for (long i = 0; i < NumPolys; i++) {
      context_vec[i % NumContexts].restore();
      if (res_vec[i].length() == deg(poly_vec[i])/multOrd(ZZ_p::modulus(), n+i))
         cerr << i << " GOOD\n";
      else
         cerr << i << " BAD\n";
   }
}

#else

#include <NTL/tools.h>

NTL_CLIENT

int main()
{
   cerr << "threads not enabled\n";
}


#endif


