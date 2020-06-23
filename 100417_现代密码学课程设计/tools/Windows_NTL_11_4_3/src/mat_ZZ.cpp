
#include <NTL/mat_ZZ.h>


NTL_START_IMPL


void add(mat_ZZ& X, const mat_ZZ& A, const mat_ZZ& B)  
{  
   long n = A.NumRows();  
   long m = A.NumCols();  
  
   if (B.NumRows() != n || B.NumCols() != m)   
      LogicError("matrix add: dimension mismatch");  
  
   X.SetDims(n, m);  
  
   long i, j;  
   for (i = 1; i <= n; i++)   
      for (j = 1; j <= m; j++)  
         add(X(i,j), A(i,j), B(i,j));  
}  
  
void sub(mat_ZZ& X, const mat_ZZ& A, const mat_ZZ& B)  
{  
   long n = A.NumRows();  
   long m = A.NumCols();  
  
   if (B.NumRows() != n || B.NumCols() != m)  
      LogicError("matrix sub: dimension mismatch");  
  
   X.SetDims(n, m);  
  
   long i, j;  
   for (i = 1; i <= n; i++)  
      for (j = 1; j <= m; j++)  
         sub(X(i,j), A(i,j), B(i,j));  
}  
  
void mul_aux(mat_ZZ& X, const mat_ZZ& A, const mat_ZZ& B)  
{  
   long n = A.NumRows();  
   long l = A.NumCols();  
   long m = B.NumCols();  
  
   if (l != B.NumRows())  
      LogicError("matrix mul: dimension mismatch");  
  
   X.SetDims(n, m);  
  
   long i, j, k;  
   ZZ acc, tmp;  
  
   for (i = 1; i <= n; i++) {  
      for (j = 1; j <= m; j++) {  
         clear(acc);  
         for(k = 1; k <= l; k++) {  
            mul(tmp, A(i,k), B(k,j));  
            add(acc, acc, tmp);  
         }  
         X(i,j) = acc;  
      }  
   }  
}  
  
  
void mul(mat_ZZ& X, const mat_ZZ& A, const mat_ZZ& B)  
{  
   if (&X == &A || &X == &B) {  
      mat_ZZ tmp;  
      mul_aux(tmp, A, B);  
      X = tmp;  
   }  
   else  
      mul_aux(X, A, B);  
}  
  
  
static
void mul_aux(vec_ZZ& x, const mat_ZZ& A, const vec_ZZ& b)  
{  
   long n = A.NumRows();  
   long l = A.NumCols();  
  
   if (l != b.length())  
      LogicError("matrix mul: dimension mismatch");  
  
   x.SetLength(n);  
  
   long i, k;  
   ZZ acc, tmp;  
  
   for (i = 1; i <= n; i++) {  
      clear(acc);  
      for (k = 1; k <= l; k++) {  
         mul(tmp, A(i,k), b(k));  
         add(acc, acc, tmp);  
      }  
      x(i) = acc;  
   }  
}  
  
  
void mul(vec_ZZ& x, const mat_ZZ& A, const vec_ZZ& b)  
{  
   if (&b == &x || A.alias(x)) {
      vec_ZZ tmp;
      mul_aux(tmp, A, b);
      x = tmp;
   }
   else
      mul_aux(x, A, b);
}  

static
void mul_aux(vec_ZZ& x, const vec_ZZ& a, const mat_ZZ& B)  
{  
   long n = B.NumRows();  
   long l = B.NumCols();  
  
   if (n != a.length())  
      LogicError("matrix mul: dimension mismatch");  
  
   x.SetLength(l);  
  
   long i, k;  
   ZZ acc, tmp;  
  
   for (i = 1; i <= l; i++) {  
      clear(acc);  
      for (k = 1; k <= n; k++) {  
         mul(tmp, a(k), B(k,i));
         add(acc, acc, tmp);  
      }  
      x(i) = acc;  
   }  
}  

void mul(vec_ZZ& x, const vec_ZZ& a, const mat_ZZ& B)
{
   if (&a == &x) { 
      vec_ZZ tmp;
      mul_aux(tmp, a, B);
      x = tmp;
   }
   else
      mul_aux(x, a, B);
}

     
  
void ident(mat_ZZ& X, long n)  
{  
   X.SetDims(n, n);  
   long i, j;  
  
   for (i = 1; i <= n; i++)  
      for (j = 1; j <= n; j++)  
         if (i == j)  
            set(X(i, j));  
         else  
            clear(X(i, j));  
} 

static
long DetBound(const mat_ZZ& a)
{
   long n = a.NumRows();
   long i;
   ZZ res, t1;

   set(res);

   for (i = 0; i < n; i++) {
      InnerProduct(t1, a[i], a[i]);
      if (t1 > 1) {
         SqrRoot(t1, t1);
         add(t1, t1, 1);
      }
      mul(res, res, t1);
   }

   return NumBits(res);
}



   

void determinant(ZZ& rres, const mat_ZZ& a, long deterministic)
{
   long n = a.NumRows();
   if (a.NumCols() != n)
      LogicError("determinant: nonsquare matrix");

   if (n == 0) {
      set(rres);
      return;
   }

   zz_pBak zbak;
   zbak.save();

   ZZ_pBak Zbak;
   Zbak.save();

   long instable = 1;

   long gp_cnt = 0;

   long bound = 2+DetBound(a);

   ZZ res, prod;

   clear(res);
   set(prod);


   long i;
   for (i = 0; ; i++) {
      if (NumBits(prod) > bound)
         break;

      if (!deterministic &&
          !instable && bound > 1000 && NumBits(prod) < 0.25*bound) {
         ZZ P;


         long plen = 90 + NumBits(max(bound, NumBits(res)));
         GenPrime(P, plen, 90 + 2*NumBits(gp_cnt++));

         ZZ_p::init(P);

         mat_ZZ_p A;
         conv(A, a);

         ZZ_p t;
         determinant(t, A);

         if (CRT(res, prod, rep(t), P))
            instable = 1;
         else
            break;
      }


      zz_p::FFTInit(i);
      long p = zz_p::modulus();

      mat_zz_p A;
      conv(A, a);

      zz_p t;
      determinant(t, A);

      instable = CRT(res, prod, rep(t), p);
   }

   rres = res;

   zbak.restore();
   Zbak.restore();
}




void conv(mat_zz_p& x, const mat_ZZ& a)
{
   long n = a.NumRows();
   long m = a.NumCols();
   long i;

   x.SetDims(n, m);
   for (i = 0; i < n; i++)
      conv(x[i], a[i]);
}

void conv(mat_ZZ_p& x, const mat_ZZ& a)
{
   long n = a.NumRows();
   long m = a.NumCols();
   long i;

   x.SetDims(n, m);
   for (i = 0; i < n; i++)
      conv(x[i], a[i]);
}

long IsIdent(const mat_ZZ& A, long n)
{
   if (A.NumRows() != n || A.NumCols() != n)
      return 0;

   long i, j;

   for (i = 1; i <= n; i++)
      for (j = 1; j <= n; j++)
         if (i != j) {
            if (!IsZero(A(i, j))) return 0;
         }
         else {
            if (!IsOne(A(i, j))) return 0;
         }

   return 1;
}


void transpose(mat_ZZ& X, const mat_ZZ& A)
{
   long n = A.NumRows();
   long m = A.NumCols();

   long i, j;

   if (&X == & A) {
      if (n == m)
         for (i = 1; i <= n; i++)
            for (j = i+1; j <= n; j++)
               swap(X(i, j), X(j, i));
      else {
         mat_ZZ tmp;
         tmp.SetDims(m, n);
         for (i = 1; i <= n; i++)
            for (j = 1; j <= m; j++)
               tmp(j, i) = A(i, j);
         X.kill();
         X = tmp;
      }
   }
   else {
      X.SetDims(m, n);
      for (i = 1; i <= n; i++)
         for (j = 1; j <= m; j++)
            X(j, i) = A(i, j);
   }
}

long CRT(mat_ZZ& gg, ZZ& a, const mat_zz_p& G)
{
   long n = gg.NumRows();
   long m = gg.NumCols();

   if (G.NumRows() != n || G.NumCols() != m)
      LogicError("CRT: dimension mismatch");

   long p = zz_p::modulus();

   ZZ new_a;
   mul(new_a, a, p);

   long a_inv;
   a_inv = rem(a, p);
   a_inv = InvMod(a_inv, p);

   long p1;
   p1 = p >> 1;

   ZZ a1;
   RightShift(a1, a, 1);

   long p_odd = (p & 1);

   long modified = 0;

   long h;

   ZZ g;
   long i, j;

   for (i = 0; i < n; i++) {
      for (j = 0; j < m; j++) {
         if (!CRTInRange(gg[i][j], a)) {
            modified = 1;
            rem(g, gg[i][j], a);
            if (g > a1) sub(g, g, a);
         }
         else
            g = gg[i][j];
      
         h = rem(g, p);
         h = SubMod(rep(G[i][j]), h, p);
         h = MulMod(h, a_inv, p);
         if (h > p1)
            h = h - p;
      
         if (h != 0) {
            modified = 1;

            if (!p_odd && g > 0 && (h == p1))
               MulSubFrom(g, a, h);
            else
               MulAddTo(g, a, h);

         }
   
         gg[i][j] = g;
      }
   }

   a = new_a;

   return modified;

}


void mul(mat_ZZ& X, const mat_ZZ& A, const ZZ& b_in)
{
   ZZ b = b_in;
   long n = A.NumRows();
   long m = A.NumCols();

   X.SetDims(n, m);

   long i, j;
   for (i = 0; i < n; i++)
      for (j = 0; j < m; j++)
         mul(X[i][j], A[i][j], b);
}

void mul(mat_ZZ& X, const mat_ZZ& A, long b)
{
   long n = A.NumRows();
   long m = A.NumCols();

   X.SetDims(n, m);

   long i, j;
   for (i = 0; i < n; i++)
      for (j = 0; j < m; j++)
         mul(X[i][j], A[i][j], b);
}


static
void ExactDiv(vec_ZZ& x, const ZZ& d)
{
   long n = x.length();
   long i;

   for (i = 0; i < n; i++)
      if (!divide(x[i], x[i], d))
         ArithmeticError("inexact division");
}

static
void ExactDiv(mat_ZZ& x, const ZZ& d)
{
   long n = x.NumRows();
   long m = x.NumCols();
   
   long i, j;

   for (i = 0; i < n; i++)
      for (j = 0; j < m; j++)
         if (!divide(x[i][j], x[i][j], d))
            ArithmeticError("inexact division");
}

void diag(mat_ZZ& X, long n, const ZZ& d_in)  
{  
   ZZ d = d_in;
   X.SetDims(n, n);  
   long i, j;  
  
   for (i = 1; i <= n; i++)  
      for (j = 1; j <= n; j++)  
         if (i == j)  
            X(i, j) = d;  
         else  
            clear(X(i, j));  
} 

long IsDiag(const mat_ZZ& A, long n, const ZZ& d)
{
   if (A.NumRows() != n || A.NumCols() != n)
      return 0;

   long i, j;

   for (i = 1; i <= n; i++)
      for (j = 1; j <= n; j++)
         if (i != j) {
            if (!IsZero(A(i, j))) return 0;
         }
         else {
            if (A(i, j) != d) return 0;
         }

   return 1;
}




void solve(ZZ& d_out, vec_ZZ& x_out,
           const mat_ZZ& A, const vec_ZZ& b,
           long deterministic)
{
   long n = A.NumRows();
   
   if (A.NumCols() != n)
      LogicError("solve: nonsquare matrix");

   if (b.length() != n)
      LogicError("solve: dimension mismatch");

   if (n == 0) {
      set(d_out);
      x_out.SetLength(0);
      return;
   }

   zz_pBak zbak;
   zbak.save();

   ZZ_pBak Zbak;
   Zbak.save();

   vec_ZZ x(INIT_SIZE, n);
   ZZ d, d1;

   ZZ d_prod, x_prod;
   set(d_prod);
   set(x_prod);

   long d_instable = 1;
   long x_instable = 1;

   long check = 0;

   long gp_cnt = 0;

   vec_ZZ y, b1;

   long i;
   long bound = 2+DetBound(A);

   for (i = 0; ; i++) {
      if ((check || IsZero(d)) && !d_instable) {
         if (NumBits(d_prod) > bound) {
            break;
         }
         else if (!deterministic &&
                  bound > 1000 && NumBits(d_prod) < 0.25*bound) {

            ZZ P;
   
            long plen = 90 + NumBits(max(bound, NumBits(d)));
            GenPrime(P, plen, 90 + 2*NumBits(gp_cnt++));
   
            ZZ_p::init(P);
   
            mat_ZZ_p AA;
            conv(AA, A);
   
            ZZ_p dd;
            determinant(dd, AA);
   
            if (CRT(d, d_prod, rep(dd), P))
               d_instable = 1;
            else 
               break;
         }
      }


      zz_p::FFTInit(i);
      long p = zz_p::modulus();

      mat_zz_p AA;
      conv(AA, A);

      if (!check) {
         vec_zz_p bb, xx;
         conv(bb, b);

         zz_p dd; 

         solve(dd, xx, AA, bb);

         d_instable = CRT(d, d_prod, rep(dd), p);
         if (!IsZero(dd)) {
            mul(xx, xx, dd);
            x_instable = CRT(x, x_prod, xx);
         }
         else
            x_instable = 1;

         if (!d_instable && !x_instable) {
            mul(y, x, A);
            mul(b1, b, d);
            if (y == b1) {
               d1 = d;
               check = 1;
            }
         }
      }
      else {
         zz_p dd;
         determinant(dd, AA);
         d_instable = CRT(d, d_prod, rep(dd), p);
      }
   }

   if (check && d1 != d) {
      mul(x, x, d);
      ExactDiv(x, d1);
   }

   d_out = d;
   if (check) x_out = x;

   zbak.restore();
   Zbak.restore();
}

void inv(ZZ& d_out, mat_ZZ& x_out, const mat_ZZ& A, long deterministic)
{
   long n = A.NumRows();
   
   if (A.NumCols() != n)
      LogicError("solve: nonsquare matrix");

   if (n == 0) {
      set(d_out);
      x_out.SetDims(0, 0);
      return;
   }

   zz_pBak zbak;
   zbak.save();

   ZZ_pBak Zbak;
   Zbak.save();

   mat_ZZ x(INIT_SIZE, n, n);
   ZZ d, d1;

   ZZ d_prod, x_prod;
   set(d_prod);
   set(x_prod);

   long d_instable = 1;
   long x_instable = 1;

   long gp_cnt = 0;

   long check = 0;


   mat_ZZ y;

   long i;
   long bound = 2+DetBound(A);

   for (i = 0; ; i++) {
      if ((check || IsZero(d)) && !d_instable) {
         if (NumBits(d_prod) > bound) {
            break;
         }
         else if (!deterministic &&
                  bound > 1000 && NumBits(d_prod) < 0.25*bound) {

            ZZ P;
   
            long plen = 90 + NumBits(max(bound, NumBits(d)));
            GenPrime(P, plen, 90 + 2*NumBits(gp_cnt++));
   
            ZZ_p::init(P);
   
            mat_ZZ_p AA;
            conv(AA, A);
   
            ZZ_p dd;
            determinant(dd, AA);
   
            if (CRT(d, d_prod, rep(dd), P))
               d_instable = 1;
            else 
               break;
         }
      }


      zz_p::FFTInit(i);
      long p = zz_p::modulus();

      mat_zz_p AA;
      conv(AA, A);

      if (!check) {
         mat_zz_p xx;

         zz_p dd; 

         inv(dd, xx, AA);

         d_instable = CRT(d, d_prod, rep(dd), p);
         if (!IsZero(dd)) {
            mul(xx, xx, dd);
            x_instable = CRT(x, x_prod, xx);
         }
         else
            x_instable = 1;

         if (!d_instable && !x_instable) {
            mul(y, x, A);
            if (IsDiag(y, n, d)) {
               d1 = d;
               check = 1;
            }
         }
      }
      else {
         zz_p dd;
         determinant(dd, AA);
         d_instable = CRT(d, d_prod, rep(dd), p);
      }
   }

   if (check && d1 != d) {
      mul(x, x, d);
      ExactDiv(x, d1);
   }

   d_out = d;
   if (check) x_out = x;

   zbak.restore();
   Zbak.restore();
}

void negate(mat_ZZ& X, const mat_ZZ& A)
{
   long n = A.NumRows();
   long m = A.NumCols();


   X.SetDims(n, m);

   long i, j;
   for (i = 1; i <= n; i++)
      for (j = 1; j <= m; j++)
         negate(X(i,j), A(i,j));
}



long IsZero(const mat_ZZ& a)
{
   long n = a.NumRows();
   long i;

   for (i = 0; i < n; i++)
      if (!IsZero(a[i]))
         return 0;

   return 1;
}

void clear(mat_ZZ& x)
{
   long n = x.NumRows();
   long i;
   for (i = 0; i < n; i++)
      clear(x[i]);
}


mat_ZZ operator+(const mat_ZZ& a, const mat_ZZ& b)
{
   mat_ZZ res;
   add(res, a, b);
   NTL_OPT_RETURN(mat_ZZ, res);
}

mat_ZZ operator*(const mat_ZZ& a, const mat_ZZ& b)
{
   mat_ZZ res;
   mul_aux(res, a, b);
   NTL_OPT_RETURN(mat_ZZ, res);
}

mat_ZZ operator-(const mat_ZZ& a, const mat_ZZ& b)
{
   mat_ZZ res;
   sub(res, a, b);
   NTL_OPT_RETURN(mat_ZZ, res);
}


mat_ZZ operator-(const mat_ZZ& a)
{
   mat_ZZ res;
   negate(res, a);
   NTL_OPT_RETURN(mat_ZZ, res);
}

vec_ZZ operator*(const mat_ZZ& a, const vec_ZZ& b)
{
   vec_ZZ res;
   mul_aux(res, a, b);
   NTL_OPT_RETURN(vec_ZZ, res);
}

vec_ZZ operator*(const vec_ZZ& a, const mat_ZZ& b)
{
   vec_ZZ res;
   mul_aux(res, a, b);
   NTL_OPT_RETURN(vec_ZZ, res);
}




void inv(mat_ZZ& X, const mat_ZZ& A)
{
   ZZ d;
   inv(d, X, A);
   if (d == -1)
      negate(X, X);
   else if (d != 1)
      ArithmeticError("inv: non-invertible matrix");
}

void power(mat_ZZ& X, const mat_ZZ& A, const ZZ& e)
{
   if (A.NumRows() != A.NumCols()) LogicError("power: non-square matrix");

   if (e == 0) {
      ident(X, A.NumRows());
      return;
   }

   mat_ZZ T1, T2;
   long i, k;

   k = NumBits(e);
   T1 = A;

   for (i = k-2; i >= 0; i--) {
      sqr(T2, T1);
      if (bit(e, i))
         mul(T1, T2, A);
      else
         T1 = T2;
   }

   if (e < 0)
      inv(X, T1);
   else
      X = T1;
}



/***********************************************************

   routines for solving a linear system via Hensel lifting

************************************************************/


static
long MaxBits(const mat_ZZ& A)
{
   long m = 0;
   long i, j;
   for (i = 0; i < A.NumRows(); i++)
      for (j = 0; j < A.NumCols(); j++)
         m = max(m, NumBits(A[i][j]));

   return m;
}




// Computes an upper bound on the numerators and denominators
// to the solution x*A = b using Hadamard's bound and Cramer's rule. 
// If A contains a zero row, then sets both bounds to zero.

static
void hadamard(ZZ& num_bound, ZZ& den_bound, 
              const mat_ZZ& A, const vec_ZZ& b)
{
   long n = A.NumRows();

   if (n == 0) LogicError("internal error: hadamard with n = 0");

   ZZ b_len, min_A_len, prod, t1;

   InnerProduct(min_A_len, A[0], A[0]);

   prod = min_A_len;

   long i;
   for (i = 1; i < n; i++) {
      InnerProduct(t1, A[i], A[i]);
      if (t1 < min_A_len)
         min_A_len = t1;
      mul(prod, prod, t1);
   }

   if (min_A_len == 0) {
      num_bound = 0;
      den_bound = 0;
      return;
   }

   InnerProduct(b_len, b, b);

   div(t1, prod, min_A_len);
   mul(t1, t1, b_len);

   SqrRoot(num_bound, t1);
   SqrRoot(den_bound, prod);
}


static
void MixedMul(vec_ZZ& x, const vec_zz_p& a, const mat_ZZ& B)
{
   long n = B.NumRows();
   long l = B.NumCols();

   if (n != a.length())
      LogicError("matrix mul: dimension mismatch");

   x.SetLength(l);

   long i, k;
   ZZ acc, tmp;

   for (i = 1; i <= l; i++) {
      clear(acc);
      for (k = 1; k <= n; k++) {
         mul(tmp, B(k, i), rep(a(k)));
         add(acc, acc, tmp);
      }
      x(i) = acc;
    }
} 

static
void SubDiv(vec_ZZ& e, const vec_ZZ& t, long p)
{
   long n = e.length();
   if (t.length() != n) LogicError("SubDiv: dimension mismatch");

   ZZ s;
   long i;

   for (i = 0; i < n; i++) {
      sub(s, e[i], t[i]);
      div(e[i], s, p);
   }
}

static
void MulAdd(vec_ZZ& x, const ZZ& prod, const vec_zz_p& h)
{
   long n = x.length();
   if (h.length() != n) LogicError("MulAdd: dimension mismatch");

   ZZ t;
   long i;

   for (i = 0; i < n; i++) {
      mul(t, prod, rep(h[i]));
      add(x[i], x[i], t);
   }
}


static
void double_MixedMul1(vec_ZZ& x, double *a, double **B, long n)
{
   long i, k;
   double acc;

   for (i = 0; i < n; i++) {
      double *bp = B[i];
      acc = 0;
      for (k = 0; k < n; k++) {
         acc += bp[k] * a[k];
      }
      conv(x[i], acc);
    }
} 


static
void double_MixedMul2(vec_ZZ& x, double *a, double **B, long n, long limit)
{
   long i, k;
   double acc;
   ZZ acc1, t;
   long j;

   for (i = 0; i < n; i++) {
      double *bp = B[i];

      clear(acc1);
      acc = 0;
      j = 0;

      for (k = 0; k < n; k++) {
         acc += bp[k] * a[k];
         j++;
         if (j == limit) {
            conv(t, acc);
            add(acc1, acc1, t);
            acc = 0;
            j = 0;
         }
      }

      if (j > 0) {
         conv(t, acc);
         add(acc1, acc1, t);
      }

      x[i] = acc1;
    }
} 


static
void long_MixedMul1(vec_ZZ& x, long *a, long **B, long n)
{
   long i, k;
   long acc;

   for (i = 0; i < n; i++) {
      long *bp = B[i];
      acc = 0;
      for (k = 0; k < n; k++) {
         acc += bp[k] * a[k];
      }
      conv(x[i], acc);
    }
} 


static
void long_MixedMul2(vec_ZZ& x, long *a, long **B, long n, long limit)
{
   long i, k;
   long acc;
   ZZ acc1, t;
   long j;

   for (i = 0; i < n; i++) {
      long *bp = B[i];

      clear(acc1);
      acc = 0;
      j = 0;

      for (k = 0; k < n; k++) {
         acc += bp[k] * a[k];
         j++;
         if (j == limit) {
            conv(t, acc);
            add(acc1, acc1, t);
            acc = 0;
            j = 0;
         }
      }

      if (j > 0) {
         conv(t, acc);
         add(acc1, acc1, t);
      }

      x[i] = acc1;
    }
} 


void solve1(ZZ& d_out, vec_ZZ& x_out, const mat_ZZ& A, const vec_ZZ& b)
{
   long n = A.NumRows();

   if (A.NumCols() != n)
      LogicError("solve1: nonsquare matrix");

   if (b.length() != n)
      LogicError("solve1: dimension mismatch");

   if (n == 0) {
      set(d_out);
      x_out.SetLength(0);
      return;
   }

   ZZ num_bound, den_bound;

   hadamard(num_bound, den_bound, A, b);

   if (den_bound == 0) {
      clear(d_out);
      return;
   }

   zz_pBak zbak;
   zbak.save();

   long i;
   long j;

   ZZ prod;
   prod = 1;

   mat_zz_p B;


   for (i = 0; ; i++) {
      zz_p::FFTInit(i);

      mat_zz_p AA, BB;
      zz_p dd;

      conv(AA, A);
      inv(dd, BB, AA);

      if (dd != 0) {
         transpose(B, BB);
         break;
      }

      mul(prod, prod, zz_p::modulus());
      
      if (prod > den_bound) {
         d_out = 0;
         return;
      }
   }

   long max_A_len = MaxBits(A);

   long use_double_mul1 = 0;
   long use_double_mul2 = 0;
   long double_limit = 0;

   if (max_A_len + NTL_SP_NBITS + NumBits(n) <= NTL_DOUBLE_PRECISION-1)
      use_double_mul1 = 1;

   if (!use_double_mul1 && max_A_len+NTL_SP_NBITS+2 <= NTL_DOUBLE_PRECISION-1) {
      use_double_mul2 = 1;
      double_limit = (1L << (NTL_DOUBLE_PRECISION-1-max_A_len-NTL_SP_NBITS));
   }

   long use_long_mul1 = 0;
   long use_long_mul2 = 0;
   long long_limit = 0;

   if (max_A_len + NTL_SP_NBITS + NumBits(n) <= NTL_BITS_PER_LONG-1)
      use_long_mul1 = 1;

   if (!use_long_mul1 && max_A_len+NTL_SP_NBITS+2 <= NTL_BITS_PER_LONG-1) {
      use_long_mul2 = 1;
      long_limit = (1L << (NTL_BITS_PER_LONG-1-max_A_len-NTL_SP_NBITS));
   }



   if (use_double_mul1 && use_long_mul1)
      use_long_mul1 = 0;
   else if (use_double_mul1 && use_long_mul2)
      use_long_mul2 = 0;
   else if (use_double_mul2 && use_long_mul1)
      use_double_mul2 = 0;
   else if (use_double_mul2 && use_long_mul2) {
      if (long_limit > double_limit)
         use_double_mul2 = 0;
      else
         use_long_mul2 = 0;
   }


   double **double_A=0;
   double *double_h=0;

   Unique2DArray<double> double_A_store;
   UniqueArray<double> double_h_store;


   if (use_double_mul1 || use_double_mul2) {
      double_h_store.SetLength(n);
      double_h = double_h_store.get();

      double_A_store.SetDims(n, n);
      double_A = double_A_store.get();

      for (i = 0; i < n; i++)
         for (j = 0; j < n; j++)
            double_A[j][i] = to_double(A[i][j]);
   }

   long **long_A=0;
   long *long_h=0;

   Unique2DArray<long> long_A_store;
   UniqueArray<long> long_h_store;


   if (use_long_mul1 || use_long_mul2) {
      long_h_store.SetLength(n);
      long_h = long_h_store.get();

      long_A_store.SetDims(n, n);
      long_A = long_A_store.get();

      for (i = 0; i < n; i++)
         for (j = 0; j < n; j++)
            long_A[j][i] = to_long(A[i][j]);
   }


   vec_ZZ x;
   x.SetLength(n);

   vec_zz_p h;
   h.SetLength(n);

   vec_ZZ e;
   e = b;

   vec_zz_p ee;

   vec_ZZ t;
   t.SetLength(n);

   prod = 1;

   ZZ bound1;
   mul(bound1, num_bound, den_bound);
   mul(bound1, bound1, 2);

   while (prod <= bound1) {
      conv(ee, e);

      mul(h, B, ee);

      if (use_double_mul1) {
         for (i = 0; i < n; i++)
            double_h[i] = to_double(rep(h[i]));

         double_MixedMul1(t, double_h, double_A, n);
      }
      else if (use_double_mul2) {
         for (i = 0; i < n; i++)
            double_h[i] = to_double(rep(h[i]));

         double_MixedMul2(t, double_h, double_A, n, double_limit);
      }
      else if (use_long_mul1) {
         for (i = 0; i < n; i++)
            long_h[i] = to_long(rep(h[i]));

         long_MixedMul1(t, long_h, long_A, n);
      }
      else if (use_long_mul2) {
         for (i = 0; i < n; i++)
            long_h[i] = to_long(rep(h[i]));

         long_MixedMul2(t, long_h, long_A, n, long_limit);
      }
      else
         MixedMul(t, h, A); // t = h*A

      SubDiv(e, t, zz_p::modulus()); // e = (e-t)/p
      MulAdd(x, prod, h);  // x = x + prod*h

      mul(prod, prod, zz_p::modulus());
   }

   vec_ZZ num, denom;
   ZZ d, d_mod_prod, tmp1;

   num.SetLength(n);
   denom.SetLength(n);
 
   d = 1;
   d_mod_prod = 1;

   for (i = 0; i < n; i++) {
      rem(x[i], x[i], prod);
      MulMod(x[i], x[i], d_mod_prod, prod);

      if (!ReconstructRational(num[i], denom[i], x[i], prod, 
           num_bound, den_bound))
          LogicError("solve1 internal error: rat recon failed!");

      mul(d, d, denom[i]);

      if (i != n-1) {
         if (denom[i] != 1) {
            div(den_bound, den_bound, denom[i]); 
            mul(bound1, num_bound, den_bound);
            mul(bound1, bound1, 2);

            div(tmp1, prod, zz_p::modulus());
            while (tmp1 > bound1) {
               prod = tmp1;
               div(tmp1, prod, zz_p::modulus());
            }

            rem(tmp1, denom[i], prod);
            rem(d_mod_prod, d_mod_prod, prod);
            MulMod(d_mod_prod, d_mod_prod, tmp1, prod);
         }
      }
   }

   tmp1 = 1;
   for (i = n-1; i >= 0; i--) {
      mul(num[i], num[i], tmp1);
      mul(tmp1, tmp1, denom[i]);
   }
   
   x_out.SetLength(n);

   for (i = 0; i < n; i++) {
      x_out[i] = num[i];
   }

   d_out = d;
}

NTL_END_IMPL
