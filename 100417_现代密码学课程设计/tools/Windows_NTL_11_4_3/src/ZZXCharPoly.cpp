#include <NTL/ZZX.h>


NTL_START_IMPL


void CharPolyMod(ZZX& gg, const ZZX& a, const ZZX& f, long deterministic)
{
   if (!IsOne(LeadCoeff(f)) || deg(f) < 1 || deg(a) >= deg(f))
      LogicError("CharPolyMod: bad args");


   if (IsZero(a)) {
      clear(gg);
      SetCoeff(gg, deg(f));
      return;
   }

   long bound = 2 + CharPolyBound(a, f);

   long gp_cnt = 0;

   zz_pBak bak;
   bak.save();

   ZZ_pBak bak1;
   bak1.save();

   ZZX g;
   ZZ prod;

   clear(g);
   set(prod);

   long i;

   long instable = 1;

   for (i = 0; ; i++) {
      if (NumBits(prod) > bound)
         break;

      if (!deterministic &&
          !instable && bound > 1000 && NumBits(prod) < 0.25*bound) {
         long plen = 90 + NumBits(max(bound, MaxBits(g)));

         ZZ P;

         GenPrime(P, plen, 90 + 2*NumBits(gp_cnt++));

         ZZ_p::init(P);
         ZZ_pX G, A, F;
         conv(A, a);
         conv(F, f);
         CharPolyMod(G, A, F);

         if (CRT(g, prod, G))
            instable = 1;
         else
            break;
      }

      zz_p::FFTInit(i);

      zz_pX G, A, F;
      conv(A, a);
      conv(F, f);
      CharPolyMod(G, A, F);
      instable = CRT(g, prod, G);
   }

   gg = g;

   bak.restore();
   bak1.restore();
}

NTL_END_IMPL
