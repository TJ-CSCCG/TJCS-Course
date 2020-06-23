#include <NTL/mat_poly_ZZ_p.h>


NTL_START_IMPL

static
void HessCharPoly(ZZ_pX& g, const ZZ_pX& a, const ZZ_pX& f)
{
   long n = deg(f);
   if (n <= 0 || deg(a) >= n)
      LogicError("HessCharPoly: bad args");

   mat_ZZ_p M;
   M.SetDims(n, n);

   long i, j;

   ZZ_pX t;
   t = a;

   for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) 
         M[i][j] = coeff(t, j);

      if (i < n-1) 
         MulByXMod(t, t, f);
   }

   CharPoly(g, M);
}

void CharPolyMod(ZZ_pX& g, const ZZ_pX& a, const ZZ_pX& ff)
{
   ZZ_pX f = ff;
   MakeMonic(f);
   long n = deg(f);

   if (n <= 0 || deg(a) >= n) 
      LogicError("CharPoly: bad args");

   if (IsZero(a)) {
      clear(g);
      SetCoeff(g, n);
      return;
   }

   if (n > 25) {
      ZZ_pX h;
      MinPolyMod(h, a, f);
      if (deg(h) == n) {
         g = h;
         return;
      }
   }

   if (ZZ_p::modulus() < n+1) {
      HessCharPoly(g, a, f);
      return;
   }

   vec_ZZ_p u(INIT_SIZE, n+1), v(INIT_SIZE, n+1);

   ZZ_pX h, h1;
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
