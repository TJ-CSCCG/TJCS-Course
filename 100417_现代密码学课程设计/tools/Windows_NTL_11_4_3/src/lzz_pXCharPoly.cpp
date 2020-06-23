#include <NTL/mat_poly_lzz_p.h>


NTL_START_IMPL

static
void HessCharPoly(zz_pX& g, const zz_pX& a, const zz_pX& f)
{
   long n = deg(f);
   if (n <= 0 || deg(a) >= n)
      LogicError("HessCharPoly: bad args");

   mat_zz_p M;
   M.SetDims(n, n);

   long i, j;

   zz_pX t;
   t = a;

   for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) 
         M[i][j] = coeff(t, j);

      if (i < n-1) 
         MulByXMod(t, t, f);
   }

   CharPoly(g, M);
}

void CharPolyMod(zz_pX& g, const zz_pX& a, const zz_pX& ff)
{
   zz_pX f = ff;
   MakeMonic(f);
   long n = deg(f);

   if (n <= 0 || deg(a) >= n) 
      LogicError("CharPoly: bad args");

   if (IsZero(a)) {
      clear(g);
      SetCoeff(g, n);
      return;
   }

   if (n > 90 || (zz_p::PrimeCnt() <= 1 && n > 45)) {
      zz_pX h;
      MinPolyMod(h, a, f);
      if (deg(h) == n) {
         g = h;
         return;
      }
   }

   if (zz_p::modulus() < n+1) {
      HessCharPoly(g, a, f);
      return;
   }

   vec_zz_p u(INIT_SIZE, n+1), v(INIT_SIZE, n+1);

   zz_pX h, h1;
   negate(h, a);
   long i;

   for (i = 0; i <= n; i++) {
      u[i] = i;
      add(h1, h, u[i]);
      resultant(v[i], f, h1);
   }

   interpolate(g, u, v);
}

NTL_END_IMPL
