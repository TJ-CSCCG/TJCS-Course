
#include <NTL/LLL.h>


NTL_START_IMPL


static void ExactDiv(ZZ& qq, const ZZ& a, const ZZ& b)
{
   NTL_ZZRegister(q);
   NTL_ZZRegister(r);

   DivRem(q, r, a, b);
   if (!IsZero(r)) {
      cerr << "a = " << a << "\n";
      cerr << "b = " << b << "\n";
      LogicError("ExactDiv: nonzero remainder");
   }
   qq = q;
}


static void BalDiv(ZZ& q, const ZZ& a, const ZZ& d)

//  rounds a/d to nearest integer, breaking ties
//    by rounding towards zero.  Assumes d > 0.

{
   NTL_ZZRegister(r);
   DivRem(q, r, a, d);


   add(r, r, r);

   long cmp = compare(r, d);
   if (cmp > 0 || (cmp == 0 && q < 0))
      add(q, q, 1);
}



static void MulAddDiv(ZZ& c, const ZZ& c1, const ZZ& c2, 
                      const ZZ& x, const ZZ& y, const ZZ& z)

// c = (x*c1 + y*c2)/z

{
   NTL_ZZRegister(t1);
   NTL_ZZRegister(t2);

   mul(t1, x, c1);
   mul(t2, y, c2);
   add(t1, t1, t2);
   ExactDiv(c, t1, z);
}


static void MulSubDiv(ZZ& c, const ZZ& c1, const ZZ& c2, 
                      const ZZ& x, const ZZ& y, const ZZ& z)

// c = (x*c1 - y*c2)/z

{
   NTL_ZZRegister(t1);
   NTL_ZZRegister(t2);

   mul(t1, x, c1);
   mul(t2, y, c2);
   sub(t1, t1, t2);
   ExactDiv(c, t1, z);
}
   




#if 0

static void MulSubDiv(vec_ZZ& c, const vec_ZZ& c1, const vec_ZZ& c2,
                      const ZZ& x, const ZZ& y, const ZZ& z)

// c = (x*c1 + y*c2)/z

{
   long n = c1.length();
   if (c2.length() != n) LogicError("MulSubDiv: length mismatch");
   c.SetLength(n);

   long i;
   for (i = 1; i <= n; i++) 
      MulSubDiv(c(i), c1(i), c2(i), x, y, z);
}

#endif

static void RowTransform(vec_ZZ& c1, vec_ZZ& c2,
                         const ZZ& x, const ZZ& y, const ZZ& u, const ZZ& v)

// (c1, c2) = (x*c1 + y*c2, u*c1 + v*c2)

{
   long n = c1.length();
   if (c2.length() != n) LogicError("MulSubDiv: length mismatch");
   NTL_ZZRegister(t1);
   NTL_ZZRegister(t2);
   NTL_ZZRegister(t3);
   NTL_ZZRegister(t4);

   long i;
   for (i = 1; i <= n; i++) {
      mul(t1, x, c1(i));
      mul(t2, y, c2(i));
      add(t1, t1, t2);

      mul(t3, u, c1(i));
      mul(t4, v, c2(i));
      add(t3, t3, t4);

      c1(i) = t1;
      c2(i) = t3;
   }
}

static void RowTransform(ZZ& c1, ZZ& c2,
                         const ZZ& x, const ZZ& y, const ZZ& u, const ZZ& v)

// (c1, c2) = (x*c1 + y*c2, u*c1 + v*c2)

{
   NTL_ZZRegister(t1);
   NTL_ZZRegister(t2);
   NTL_ZZRegister(t3);
   NTL_ZZRegister(t4);

   mul(t1, x, c1);
   mul(t2, y, c2);
   add(t1, t1, t2);

   mul(t3, u, c1);
   mul(t4, v, c2);
   add(t3, t3, t4);

   c1 = t1;
   c2 = t3;
}



static void MulSubFrom(vec_ZZ& c, const vec_ZZ& c2, const ZZ& x)

// c = c - x*c2

{
   long n = c.length();
   if (c2.length() != n) LogicError("MulSubFrom: length mismatch");

   long i;
   for (i = 1; i <= n; i++)
      MulSubFrom(c(i), c2(i), x);
}

static void MulSubFrom(vec_ZZ& c, const vec_ZZ& c2, long x)

// c = c - x*c2

{
   long n = c.length();
   if (c2.length() != n) LogicError("MulSubFrom: length mismatch");

   long i;
   for (i = 1; i <= n; i++)
      MulSubFrom(c(i), c2(i), x);
}


      
      
   
static long SwapTest(const ZZ& d0, const ZZ& d1, const ZZ& d2, const ZZ& lam,
                     long a, long b)

// test if a*d1^2 > b*(d0*d2 + lam^2)

{
   NTL_ZZRegister(t1);
   NTL_ZZRegister(t2);

   mul(t1, d0, d2);
   sqr(t2, lam);
   add(t1, t1, t2);
   mul(t1, t1, b);

   sqr(t2, d1);
   mul(t2, t2, a);

   return t2 > t1;
}






static
void reduce(long k, long l, 
            mat_ZZ& B, vec_long& P, vec_ZZ& D, 
            vec_vec_ZZ& lam, mat_ZZ* U)
{
   NTL_ZZRegister(t1);
   NTL_ZZRegister(r);

   if (P(l) == 0) return;
   add(t1, lam(k)(P(l)), lam(k)(P(l)));
   abs(t1, t1);
   if (t1 <= D[P(l)]) return;

   long j;
   long rr, small_r;

   BalDiv(r, lam(k)(P(l)), D[P(l)]);

   if (r.WideSinglePrecision()) {
      small_r = 1;
      rr = to_long(r);
   }
   else {
      small_r = 0;
   }
      
   if (small_r) {
      MulSubFrom(B(k), B(l), rr);

      if (U) MulSubFrom((*U)(k), (*U)(l), rr);

      for (j = 1; j <= l-1; j++)
         if (P(j) != 0)
            MulSubFrom(lam(k)(P(j)), lam(l)(P(j)), rr);
      MulSubFrom(lam(k)(P(l)), D[P(l)], rr);
   }
   else {
      MulSubFrom(B(k), B(l), r);

      if (U) MulSubFrom((*U)(k), (*U)(l), r);

      for (j = 1; j <= l-1; j++)
         if (P(j) != 0)
            MulSubFrom(lam(k)(P(j)), lam(l)(P(j)), r);
      MulSubFrom(lam(k)(P(l)), D[P(l)], r);
   }


}


static
long swap(long k, mat_ZZ& B, vec_long& P, vec_ZZ& D, 
          vec_vec_ZZ& lam, mat_ZZ* U, long m, long verbose)

// swaps vectors k-1 and k;  assumes P(k-1) != 0
// returns 1 if vector k-1 need to be reduced after the swap...
//    this only occurs in 'case 2' when there are linear dependencies

{
   long i, j;
   NTL_ZZRegister(t1);
   NTL_ZZRegister(t2);
   NTL_ZZRegister(t3);
   NTL_ZZRegister(e);
   NTL_ZZRegister(x);
   NTL_ZZRegister(y);


   if (P(k) != 0) {
      if (verbose) cerr << "swap case 1: " << k << "\n";

      swap(B(k-1), B(k));
      if (U) swap((*U)(k-1), (*U)(k));
   
      for (j = 1; j <= k-2; j++)
         if (P(j) != 0)
            swap(lam(k-1)(P(j)), lam(k)(P(j)));

      for (i = k+1; i <= m; i++) {
         MulAddDiv(t1, lam(i)(P(k)-1), lam(i)(P(k)), 
                   lam(k)(P(k)-1), D[P(k)-2], D[P(k)-1]); 
         MulSubDiv(t2, lam(i)(P(k)-1), lam(i)(P(k)), 
                   D[P(k)], lam(k)(P(k)-1), D[P(k)-1]);
         lam(i)(P(k)-1) = t1;
         lam(i)(P(k)) = t2;
      }

      MulAddDiv(D[P(k)-1], D[P(k)], lam(k)(P(k)-1),
                D[P(k)-2], lam(k)(P(k)-1), D[P(k)-1]);

      return 0;
   }
   else if (!IsZero(lam(k)(P(k-1)))) {
      if (verbose) cerr << "swap case 2: " << k << "\n";
      XGCD(e, x, y, lam(k)(P(k-1)), D[P(k-1)]);

      ExactDiv(t1, lam(k)(P(k-1)), e);
      ExactDiv(t2, D[P(k-1)], e);

      t3 = t2;
      negate(t2, t2);
      RowTransform(B(k-1), B(k), t1, t2, y, x);
      if (U) RowTransform((*U)(k-1), (*U)(k), t1, t2, y, x);
      for (j = 1; j <= k-2; j++)
         if (P(j) != 0)
            RowTransform(lam(k-1)(P(j)), lam(k)(P(j)), t1, t2, y, x);

      sqr(t2, t2);
      ExactDiv(D[P(k-1)], D[P(k-1)], t2);

      for (i = k+1; i <= m; i++)
         if (P(i) != 0) {
            ExactDiv(D[P(i)], D[P(i)], t2);
            for (j = i+1; j <= m; j++) {
               ExactDiv(lam(j)(P(i)), lam(j)(P(i)), t2);
            }
         }

      for (i = k+1; i <= m; i++) {
         ExactDiv(lam(i)(P(k-1)), lam(i)(P(k-1)), t3);
      }

      swap(P(k-1), P(k));

      return 1;
   }
   else {
      if (verbose) cerr << "swap case 3: " << k << "\n";

      swap(B(k-1), B(k));
      if (U) swap((*U)(k-1), (*U)(k));
   
      for (j = 1; j <= k-2; j++)
         if (P(j) != 0)
            swap(lam(k-1)(P(j)), lam(k)(P(j)));

      swap(P(k-1), P(k));

      return 0;
   }
}

   


static
void IncrementalGS(mat_ZZ& B, vec_long& P, vec_ZZ& D, vec_vec_ZZ& lam, 
                   long& s, long k)
{
   long n = B.NumCols();
   long m = B.NumRows();

   NTL_ZZRegister(u);
   NTL_ZZRegister(t1);
   NTL_ZZRegister(t2);

   long i, j;

   for (j = 1; j <= k-1; j++) {
      long posj = P(j);
      if (posj == 0) continue;

      InnerProduct(u, B(k), B(j));
      for (i = 1; i <= posj-1; i++) {
         mul(t1, D[i], u);
         mul(t2, lam(k)(i), lam(j)(i));
         sub(t1, t1, t2);
         div(t1, t1, D[i-1]);
         u = t1;
      }

      lam(k)(posj) = u;
   }

   InnerProduct(u, B(k), B(k));
   for (i = 1; i <= s; i++) {
      mul(t1, D[i], u);
      mul(t2, lam(k)(i), lam(k)(i));
      sub(t1, t1, t2);
      div(t1, t1, D[i-1]);
      u = t1;
   }

   if (u == 0) {
      P(k) = 0;
   }
   else {
      s++;
      P(k) = s;
      D[s] = u;
   }
}


static
long LLL(vec_ZZ& D, mat_ZZ& B, mat_ZZ* U, long a, long b, long verbose)
{
   long m = B.NumRows();
   long n = B.NumCols();

   long force_reduce = 1;

   vec_long P;
   P.SetLength(m);

   D.SetLength(m+1);
   D[0] = 1;

   vec_vec_ZZ lam;

   lam.SetLength(m);

   long j;
   for (j = 1; j <= m; j++)
      lam(j).SetLength(m);

   if (U) ident(*U, m);

   long s = 0;

   long k = 1;
   long max_k = 0;


   while (k <= m) {
      if (k > max_k) {
         IncrementalGS(B, P, D, lam, s, k);
         max_k = k;
      }

      if (k == 1) {
         force_reduce = 1; 
         k++;
         continue;
      }

      if (force_reduce)
         for (j = k-1; j >= 1; j--)
            reduce(k, j, B, P, D, lam, U);

      if (P(k-1) != 0 && 
          (P(k) == 0 || 
           SwapTest(D[P(k)], D[P(k)-1], D[P(k)-2], lam(k)(P(k)-1), a, b))) {
         force_reduce = swap(k, B, P, D, lam, U, max_k, verbose);
         k--;
      }
      else {
         force_reduce = 1;
         k++;
      }
   }

   D.SetLength(s+1);
   return s;
}



static
long image(ZZ& det, mat_ZZ& B, mat_ZZ* U, long verbose)
{
   long m = B.NumRows();
   long n = B.NumCols();

   long force_reduce = 1;

   vec_long P;
   P.SetLength(m);

   vec_ZZ D;
   D.SetLength(m+1);
   D[0] = 1;

   vec_vec_ZZ lam;

   lam.SetLength(m);

   long j;
   for (j = 1; j <= m; j++)
      lam(j).SetLength(m);

   if (U) ident(*U, m);

   long s = 0;

   long k = 1;
   long max_k = 0;


   while (k <= m) {
      if (k > max_k) {
         IncrementalGS(B, P, D, lam, s, k);
         max_k = k;
      }

      if (k == 1) {
         force_reduce = 1; 
         k++;
         continue;
      }

      if (force_reduce)
         for (j = k-1; j >= 1; j--) 
            reduce(k, j, B, P, D, lam, U);

      if (P(k-1) != 0 && P(k) == 0) {
         force_reduce = swap(k, B, P, D, lam, U, max_k, verbose);
         k--;
      }
      else {
         force_reduce = 1;
         k++;
      }
   }

   det = D[s];
   return s;
}

long LLL(ZZ& det, mat_ZZ& B, mat_ZZ& U, long verbose)
{
   vec_ZZ D;
   long s;
   s = LLL(D, B, &U, 3, 4, verbose);
   det = D[s];
   return s;
}

long LLL(ZZ& det, mat_ZZ& B, long verbose)
{
   vec_ZZ D;
   long s;
   s = LLL(D, B, 0, 3, 4, verbose);
   det = D[s];
   return s;
}

long LLL(ZZ& det, mat_ZZ& B, mat_ZZ& U, long a, long b, long verbose)
{
   if (a <= 0 || b <= 0 || a > b || b/4 >= a) LogicError("LLL: bad args");

   vec_ZZ D;
   long s;
   s = LLL(D, B, &U, a, b, verbose);
   det = D[s];
   return s;
}

long LLL(ZZ& det, mat_ZZ& B, long a, long b, long verbose)
{
   if (a <= 0 || b <= 0 || a > b || b/4 >= a) LogicError("LLL: bad args");

   vec_ZZ D;
   long s;
   s = LLL(D, B, 0, a, b, verbose);
   det = D[s];
   return s;
}


long LLL_plus(vec_ZZ& D_out, mat_ZZ& B, mat_ZZ& U, long verbose)
{
   vec_ZZ D;
   long s;
   s = LLL(D, B, &U, 3, 4, verbose);
   D_out = D;
   return s;
}

long LLL_plus(vec_ZZ& D_out, mat_ZZ& B, long verbose)
{
   vec_ZZ D;
   long s;
   s = LLL(D, B, 0, 3, 4, verbose);
   D_out = D;
   return s;
}

long LLL_plus(vec_ZZ& D_out, mat_ZZ& B, mat_ZZ& U, long a, long b, long verbose)
{
   if (a <= 0 || b <= 0 || a > b || b/4 >= a) LogicError("LLL_plus: bad args");

   vec_ZZ D;
   long s;
   s = LLL(D, B, &U, a, b, verbose);
   D_out = D;
   return s;
}

long LLL_plus(vec_ZZ& D_out, mat_ZZ& B, long a, long b, long verbose)
{
   if (a <= 0 || b <= 0 || a > b || b/4 >= a) LogicError("LLL_plus: bad args");

   vec_ZZ D;
   long s;
   s = LLL(D, B, 0, a, b, verbose);
   D_out = D;
   return s;
}


long image(ZZ& det, mat_ZZ& B, mat_ZZ& U, long verbose)
{
   return image(det, B, &U, verbose);
}

long image(ZZ& det, mat_ZZ& B, long verbose)
{
   return image(det, B, 0, verbose);
}

long LatticeSolve(vec_ZZ& x, const mat_ZZ& A, const vec_ZZ& y, long reduce)
{
   long n = A.NumRows();
   long m = A.NumCols();

   if (y.length() != m)
      LogicError("LatticeSolve: dimension mismatch");

   if (reduce < 0 || reduce > 2)
      LogicError("LatticeSolve: bad reduce parameter");

   if (IsZero(y)) {
      x.SetLength(n);
      clear(x);
      return 1;
   }

   mat_ZZ A1, U1;
   ZZ det2;
   long im_rank, ker_rank;

   A1 = A;

   im_rank = image(det2, A1, U1);
   ker_rank = n - im_rank;

   mat_ZZ A2, U2;
   long new_rank;
   long i;

   A2.SetDims(im_rank + 1, m);
   for (i = 1; i <= im_rank; i++)
      A2(i) = A1(ker_rank + i);

   A2(im_rank + 1) = y;

   new_rank = image(det2, A2, U2);

   if (new_rank != im_rank || 
      (U2(1)(im_rank+1) != 1  && U2(1)(im_rank+1) != -1))
      return 0;

   vec_ZZ x1;
   x1.SetLength(im_rank);

   for (i = 1; i <= im_rank; i++)
      x1(i) = U2(1)(i);

   if (U2(1)(im_rank+1) == 1)
      negate(x1, x1);

   vec_ZZ x2, tmp;
   x2.SetLength(n);
   clear(x2);
   tmp.SetLength(n);

   for (i = 1; i <= im_rank; i++) {
      mul(tmp, U1(ker_rank+i), x1(i));
      add(x2, x2, tmp);
   }

   if (reduce == 0) {
      x = x2;
      return 1;
   }
   else if (reduce == 1) {
      U1.SetDims(ker_rank+1, n);
      U1(ker_rank+1) = x2;
      image(det2, U1);

      x = U1(ker_rank + 1);
      return 1;
   }
   else if (reduce == 2) {
      U1.SetDims(ker_rank, n);
      LLL(det2, U1);
      U1.SetDims(ker_rank+1, n);
      U1(ker_rank+1) = x2;
      image(det2, U1);

      x = U1(ker_rank + 1);
      return 1;
   }

   return 0;
} 



NTL_END_IMPL
