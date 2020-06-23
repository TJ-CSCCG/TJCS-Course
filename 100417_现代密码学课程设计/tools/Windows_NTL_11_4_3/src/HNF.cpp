
#include <NTL/HNF.h>


NTL_START_IMPL


// This implements a variation of an algorithm in
// [P. Domich, R. Kannan and L. Trotter, Math. Oper. Research 12:50-59, 1987].
// I started with the description in Henri Cohen's book, but had to modify
// that because Cohen does not actually keep the numbers reduced modulo
// the determinant, which leads to larger than necessary numbers.
// This modifiaction was put in place in v3.9b.

static
void EuclUpdate(vec_ZZ& u, vec_ZZ& v, 
                const ZZ& a, const ZZ& b, const ZZ& c, const ZZ& d,
                const ZZ& M)

{
   long m = u.length(); 
   long i;

   ZZ M1;
   RightShift(M1, M, 1);

   ZZ t1, t2, t3;

   for (i = 1; i <= m; i++) {
      mul(t1, u(i), a);
      mul(t2, v(i), b);
      add(t1, t1, t2);
      rem(t1, t1, M);
      if (t1 > M1)
         sub(t1, t1, M);

      t3 = t1;

      mul(t1, u(i), c);
      mul(t2, v(i), d);
      add(t1, t1, t2);
      rem(t1, t1, M);
      if (t1 > M1)
         sub(t1, t1, M);

      u(i) = t3;
      v(i) = t1;
   }
}


static
void FixDiag(vec_ZZ& u, const ZZ& a, const vec_ZZ& v, const ZZ& M, long m)
{
   long i;
   ZZ t1;

   for (i = 1; i <= m; i++) {
      mul(t1, a, v(i));
      rem(u(i), t1, M);
   }
}


static
void ReduceW(vec_ZZ& u, const ZZ& a, const vec_ZZ& v, const ZZ& M, long m)
{
   long i;
   ZZ t1, t2;

   for (i = 1; i <= m; i++) {
      mul(t1, a, v(i));
      sub(t2, u(i), t1);
      rem(u(i), t2, M);
   }
}
   


void HNF(mat_ZZ& W, const mat_ZZ& A_in, const ZZ& D_in)
{
   mat_ZZ A = A_in;

   long n = A.NumRows();
   long m = A.NumCols();

   ZZ D = D_in;
   if (D < 0)
      negate(D, D);

   if (n == 0 || m == 0 || D == 0)
      LogicError("HNF: bad input");

   W.SetDims(m, m);
   clear(W);

   long i, j, k;
   ZZ d, u, v, c1, c2;

   k = n;

   for (i = m; i >= 1; i--) {
      for (j = k-1; j >= 1; j--) {
         if (A(j, i) != 0) {
            XGCD(d, u, v, A(k, i), A(j, i));
            div(c1, A(k, i), d);
            div(c2, A(j, i), d);
            negate(c2, c2);
            EuclUpdate(A(j), A(k), c1, c2, v, u, D);
         }
      }

      XGCD(d, u, v, A(k, i), D);
      FixDiag(W(i), u, A(k), D, i);
      if (W(i, i) == 0) W(i, i) = D;

      for (j = i+1; j <= m; j++) {
         div(c1, W(j, i), W(i, i));
         ReduceW(W(j), c1, W(i), D, i);
      }

      div(D, D, d);
      k--;
   }
}

NTL_END_IMPL
