#include <NTL/mat_poly_lzz_p.h>


NTL_START_IMPL


void CharPoly(zz_pX& f, const mat_zz_p& M)
{
   long n = M.NumRows();
   if (M.NumCols() != n)
      LogicError("CharPoly: nonsquare matrix");

   if (n == 0) {
      set(f);
      return;
   }

   zz_p t;

   if (n == 1) {
      SetX(f);
      negate(t, M(1, 1));
      SetCoeff(f, 0, t);
      return;
   }

   mat_zz_p H;

   H = M;

   long i, j, m;
   zz_p u, t1;

   for (m = 2; m <= n-1; m++) {
      i = m;
      while (i <= n && IsZero(H(i, m-1)))
         i++;

      if (i <= n) {
         t = H(i, m-1);
         if (i > m) {
            swap(H(i), H(m));
            // swap columns i and m
            for (j = 1; j <= n; j++) 
               swap(H(j, i), H(j, m));
         }

         for (i = m+1; i <= n; i++) {
            div(u, H(i, m-1), t);
            for (j = m; j <= n; j++) {
               mul(t1, u, H(m, j));
               sub(H(i, j), H(i, j), t1);
            }

            for (j = 1; j <= n; j++) {
               mul(t1, u, H(j, i));
               add(H(j, m), H(j, m), t1);
            }
         }
      }
   }

   vec_zz_pX F;
   F.SetLength(n+1);
   zz_pX T;
   T.SetMaxLength(n);

   set(F[0]);
   for (m = 1; m <= n; m++) {
      LeftShift(F[m], F[m-1], 1);
      mul(T, F[m-1], H(m, m));
      sub(F[m], F[m], T);

      set(t);
      for (i = 1; i <= m-1; i++) {
         mul(t, t, H(m-i+1, m-i));
         mul(t1, t, H(m-i, m));
         mul(T, F[m-i-1], t1);
         sub(F[m], F[m], T);
      }
   }

   f = F[n];
}

NTL_END_IMPL
