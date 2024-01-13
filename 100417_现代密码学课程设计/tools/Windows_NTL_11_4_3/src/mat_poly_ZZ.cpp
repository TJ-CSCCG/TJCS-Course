#include <NTL/mat_poly_ZZ.h>
#include <NTL/mat_poly_ZZ_p.h>
#include <NTL/mat_poly_lzz_p.h>


NTL_START_IMPL

static
long CharPolyBound(const mat_ZZ& a)
// This bound is computed via interpolation
// through complex roots of unity.

{
   long n = a.NumRows();
   long i;
   ZZ res, t1, t2;

   set(res);

   for (i = 0; i < n; i++) {
      InnerProduct(t1, a[i], a[i]);
      abs(t2, a[i][i]);
      mul(t2, t2, 2);
      add(t2, t2, 1);
      add(t1, t1, t2);
      if (t1 > 1) {
         SqrRoot(t1, t1);
         add(t1, t1, 1);
      }
      mul(res, res, t1);
   }

   return NumBits(res);
}

void CharPoly(ZZX& gg, const mat_ZZ& a, long deterministic)
{
   long n = a.NumRows();
   if (a.NumCols() != n)
      LogicError("CharPoly: nonsquare matrix");

   if (n == 0) {
      set(gg);
      return;
   }


   if (n == 1) {
      ZZ t;
      SetX(gg);
      negate(t, a(1, 1));
      SetCoeff(gg, 0, t);
      return;
   }

   long bound = 2 + CharPolyBound(a);

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

   long gp_cnt = 0;

   for (i = 0; ; i++) {
      if (NumBits(prod) > bound)
         break;

      if (!deterministic &&
          !instable && bound > 1000 && NumBits(prod) < 0.25*bound) {
         long plen = 90 + NumBits(max(bound, MaxBits(g)));

         ZZ P;

         GenPrime(P, plen, 90 + 2*NumBits(gp_cnt++));

         ZZ_p::init(P);
         mat_ZZ_p A;
         ZZ_pX G;
         conv(A, a);
         CharPoly(G, A);

         if (CRT(g, prod, G))
            instable = 1;
         else
            break;
      }

      zz_p::FFTInit(i);

      mat_zz_p A;
      zz_pX G;
      conv(A, a);
      CharPoly(G, A);
      instable = CRT(g, prod, G);
   }

   gg = g;

   bak.restore();
   bak1.restore();
}

NTL_END_IMPL
